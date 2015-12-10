/*
 * libmm-streamrecorder
 *
 * Copyright (c) 2000 - 2013 Samsung Electronics Co., Ltd. All rights reserved.
 *
 * Contact: Hyuntae Kim <ht1211.kim@samsung.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/*=======================================================================================
|  INCLUDE FILES									|
=======================================================================================*/
#include <sys/time.h>
#include <unistd.h>

#include "mm_streamrecorder_internal.h"
#include "mm_streamrecorder_gstcommon.h"
#include "mm_streamrecorder_gstdispatch.h"
#include "mm_streamrecorder_util.h"
#include "mm_streamrecorder.h"

/*-----------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/
#define USE_AUDIO_CLOCK_TUNE
#define _MMSTREAMRECORDER_WAIT_EOS_TIME              5.0	/* sec */
#define _MMSTREAMRECORDER_STATE_SET_COUNT			3

/*-----------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:						|
-----------------------------------------------------------------------*/
/* STATIC INTERNAL FUNCTION */

/*=======================================================================================
|  FUNCTION DEFINITIONS									|
=======================================================================================*/
/*-----------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:					|
-----------------------------------------------------------------------*/
gboolean _mmstreamrecorder_gstreamer_init()
{
	static const int max_argc = 10;
	int i = 0;
	gint *argc = NULL;
	gchar **argv = NULL;
	GError *err = NULL;
	gboolean ret = FALSE;
	/* type_string_array *GSTInitOption = NULL; */

	/* mmf_return_val_if_fail(conf, FALSE); */

	_mmstreamrec_dbg_log("");

	/* alloc */
	argc = malloc(sizeof(int));
	argv = malloc(sizeof(gchar *) * max_argc);

	if (!argc || !argv)
		goto ERROR;

	memset(argv, 0, sizeof(gchar *) * max_argc);

	/* add initial */
	*argc = 1;
	argv[0] = g_strdup("mmstreamrecorder");

	_mmstreamrec_dbg_log("initializing gstreamer with following parameter[argc:%d]", *argc);

	for (i = 0; i < *argc; i++)
		_mmstreamrec_dbg_log("argv[%d] : %s", i, argv[i]);

	/* initializing gstreamer */
	ret = gst_init_check(argc, &argv, &err);

	if (!ret) {
		_mmstreamrec_dbg_err("Could not initialize GStreamer: %s ", err ? err->message : "unknown error occurred");
		if (err)
			g_error_free(err);
	}

	/* release */

	for (i = 0; i < *argc; i++) {
		if (argv[i]) {
			g_free(argv[i]);
			argv[i] = NULL;
		}
	}

	if (argv) {
		g_free(argv);
		argv = NULL;
	}

	if (argc) {
		g_free(argc);
		argc = NULL;
	}

	return ret;

 ERROR:
	_mmstreamrec_dbg_err("failed to initialize gstreamer");

	if (argv) {
		g_free(argv);
		argv = NULL;
	}

	if (argc) {
		g_free(argc);
		argc = NULL;
	}

	return FALSE;
}

int _mmstreamrecorder_get_eos_message(MMHandleType handle)
{
	double elapsed = 0.0;

	GstMessage *gMessage = NULL;
	GstBus *bus = NULL;
	GstClockTime timeout = 1 * GST_SECOND;	/* maximum waiting time */
	GTimer *timer = NULL;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);

	_mmstreamrec_dbg_log("");
	if (sc) {
		bus = gst_pipeline_get_bus(GST_PIPELINE(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst));
		timer = g_timer_new();

		if (!(sc->bget_eos)) {
			while (TRUE) {
				elapsed = g_timer_elapsed(timer, NULL);

				/*_mmstreamrec_dbg_log("elapsed:%f sec", elapsed);*/

				if (elapsed > _MMSTREAMRECORDER_WAIT_EOS_TIME) {
					_mmstreamrec_dbg_warn("Timeout. EOS isn't received.");
					g_timer_destroy(timer);
					gst_object_unref(bus);
					return MM_ERROR_STREAMRECORDER_RESPONSE_TIMEOUT;
				}

				gMessage = gst_bus_timed_pop(bus, timeout);
				if (gMessage != NULL) {
					_mmstreamrec_dbg_log("Get message(%x).", GST_MESSAGE_TYPE(gMessage));
					_mmstreamrecorder_pipeline_cb_message(bus, gMessage, (void *)hstreamrecorder);

					if (GST_MESSAGE_TYPE(gMessage) == GST_MESSAGE_EOS || sc->bget_eos) {
						gst_message_unref(gMessage);
						break;
					}
					gst_message_unref(gMessage);
				} else {
					_mmstreamrec_dbg_log("timeout of gst_bus_timed_pop()");
					if (sc->bget_eos) {
						_mmstreamrec_dbg_log("Get EOS in another thread.");
						break;
					}
				}
			}
		}

		g_timer_destroy(timer);
		timer = NULL;
		gst_object_unref(bus);
		bus = NULL;
	} else {
		_mmstreamrec_dbg_err("subcontext error NULL");
	}
	_mmstreamrec_dbg_log("END");

	return MM_ERROR_NONE;
}

void _mmstreamrecorder_remove_element_handle(MMHandleType handle, void *element, int first_elem, int last_elem)
{
	int i = 0;
	_MMStreamRecorderGstElement *remove_element = (_MMStreamRecorderGstElement *) element;

	mmf_return_if_fail(handle && remove_element);
	mmf_return_if_fail((first_elem >= 0) && (last_elem > 0) && (last_elem > first_elem));

	_mmstreamrec_dbg_log("");

	for (i = first_elem; i <= last_elem; i++) {
		remove_element[i].gst = NULL;
		remove_element[i].id = _MMSTREAMRECORDER_ENCODE_NONE;
	}

	return;
}

gboolean _mmstreamrecorder_add_elements_to_bin(GstBin *bin, GList *element_list)
{
	GList *local_list = element_list;
	_MMStreamRecorderGstElement *element = NULL;

	mmf_return_val_if_fail(bin && local_list, FALSE);

	while (local_list) {
		element = (_MMStreamRecorderGstElement *) local_list->data;
		if (element && element->gst) {
			if (!gst_bin_add(bin, GST_ELEMENT(element->gst))) {
				_mmstreamrec_dbg_err("Add element [%s] to bin [%s] FAILED", GST_ELEMENT_NAME(GST_ELEMENT(element->gst)), GST_ELEMENT_NAME(GST_ELEMENT(bin)));
				return FALSE;
			} else {
				_mmstreamrec_dbg_log("Add element [%s] to bin [%s] OK", GST_ELEMENT_NAME(GST_ELEMENT(element->gst)), GST_ELEMENT_NAME(GST_ELEMENT(bin)));
			}
		}
		local_list = local_list->next;
	}

	return TRUE;
}

gboolean _mmstreamrecorder_link_elements(GList * element_list)
{
	GList *local_list = element_list;
	_MMStreamRecorderGstElement *element = NULL;
	_MMStreamRecorderGstElement *pre_element = NULL;

	mmf_return_val_if_fail(local_list, FALSE);

	pre_element = (_MMStreamRecorderGstElement *) local_list->data;
	local_list = local_list->next;

	while (local_list) {
		element = (_MMStreamRecorderGstElement *) local_list->data;
		if (element && element->gst) {
			if (pre_element != NULL) {
				if (_MM_GST_ELEMENT_LINK(GST_ELEMENT(pre_element->gst), GST_ELEMENT(element->gst))) {
					_mmstreamrec_dbg_log("Link [%s] to [%s] OK", GST_ELEMENT_NAME(GST_ELEMENT(pre_element->gst)), GST_ELEMENT_NAME(GST_ELEMENT(element->gst)));
				} else {
					_mmstreamrec_dbg_err("Link [%s] to [%s] FAILED", GST_ELEMENT_NAME(GST_ELEMENT(pre_element->gst)), GST_ELEMENT_NAME(GST_ELEMENT(element->gst)));
					return FALSE;
				}
			} else {
				_mmstreamrec_dbg_err("pre_element is null");
				return FALSE;
			}
		}

		pre_element = element;
		local_list = local_list->next;
	}

	return TRUE;
}

int _mmstreamrecorder_gst_set_state(MMHandleType handle, GstElement *pipeline, GstState target_state)
{
	unsigned int k = 0;
	GstState pipeline_state = GST_STATE_VOID_PENDING;
	GstStateChangeReturn setChangeReturn = GST_STATE_CHANGE_FAILURE;
	GstStateChangeReturn getChangeReturn = GST_STATE_CHANGE_FAILURE;
	GstClockTime get_timeout = __MMSTREAMRECORDER_SET_GST_STATE_TIMEOUT * GST_SECOND;

	mmf_return_val_if_fail(handle, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_log("Set state to %d", target_state);

	_MMSTREAMRECORDER_LOCK_GST_STATE(handle);

	for (k = 0; k < _MMSTREAMRECORDER_STATE_SET_COUNT; k++) {
		setChangeReturn = gst_element_set_state(pipeline, target_state);
		_mmstreamrec_dbg_log("gst_element_set_state[%d] return %d", target_state, setChangeReturn);
		if (setChangeReturn != GST_STATE_CHANGE_FAILURE) {
			getChangeReturn = gst_element_get_state(pipeline, &pipeline_state, NULL, get_timeout);
			switch (getChangeReturn) {
			case GST_STATE_CHANGE_NO_PREROLL:
				_mmstreamrec_dbg_log("status=GST_STATE_CHANGE_NO_PREROLL.");
			case GST_STATE_CHANGE_SUCCESS:
				/* if we reached the final target state, exit */
				if (pipeline_state == target_state) {
					_MMSTREAMRECORDER_UNLOCK_GST_STATE(handle);
					return MM_ERROR_NONE;
				}
				break;
			case GST_STATE_CHANGE_ASYNC:
				_mmstreamrec_dbg_log("status=GST_STATE_CHANGE_ASYNC.");
				break;
			default:
				_MMSTREAMRECORDER_UNLOCK_GST_STATE(handle);
				_mmstreamrec_dbg_log("status=GST_STATE_CHANGE_FAILURE.");
				return MM_ERROR_STREAMRECORDER_GST_STATECHANGE;
			}

			_MMSTREAMRECORDER_UNLOCK_GST_STATE(handle);
			_mmstreamrec_dbg_err("timeout of gst_element_get_state()!!");
			return MM_ERROR_STREAMRECORDER_RESPONSE_TIMEOUT;
		}
		usleep(_MMSTREAMRECORDER_STATE_CHECK_INTERVAL);
	}

	_MMSTREAMRECORDER_UNLOCK_GST_STATE(handle);

	_mmstreamrec_dbg_err("Failure. gst_element_set_state timeout!!");

	return MM_ERROR_STREAMRECORDER_RESPONSE_TIMEOUT;
}

GstCaps *gst_set_videosrcpad_caps(gint srcfmt, gint width, gint height, gint rate, gint scale)
{

	GstCaps *caps = NULL;

	if (srcfmt == MM_STREAMRECORDER_INPUT_FORMAT_NV12)
		caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "NV12", "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, "framerate", GST_TYPE_FRACTION, rate, scale, NULL);
	else if (srcfmt == MM_STREAMRECORDER_INPUT_FORMAT_I420)
		caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "I420", "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, "framerate", GST_TYPE_FRACTION, rate, scale, NULL);
	else
		caps = gst_caps_new_simple("video/x-raw", "format", G_TYPE_STRING, "BGRA8888", "bpp", G_TYPE_INT, 32, "depth", G_TYPE_INT, 24, "endianness", G_TYPE_INT, 4321, "red_mask", G_TYPE_INT, 65280, "green_mask", G_TYPE_INT, 16711680, "blue_mask", G_TYPE_INT, -16777216, "width", G_TYPE_INT, width, "height", G_TYPE_INT, height, "framerate", GST_TYPE_FRACTION, rate, scale, NULL);

	if (!caps) {
		_mmstreamrec_dbg_err("failed to alloc caps");
		return NULL;
	}

	/* gchar *type = gst_caps_to_string(caps); */

	/* _mmstreamrec_dbg_warn("Set srcpad caps: %s",type); */

	/* g_free(type); */

	return caps;
}

GstCaps *gst_set_audiosrcpad_caps(gint samplerate, gint channel, gint depth, gint width, gint datatype)
{

	GstCaps *caps = gst_caps_new_simple("audio/x-raw",
										"rate", G_TYPE_INT, samplerate,
										"channels", G_TYPE_INT, 2,
										"depth", G_TYPE_INT, depth,
										"width", G_TYPE_INT, width,
										"signed", G_TYPE_BOOLEAN, TRUE,
										"endianness", G_TYPE_INT, 1234,
										NULL);

	if (!caps) {
		_mmstreamrec_dbg_err("failed to alloc caps");
		return NULL;
	}

	/* gchar *type = gst_caps_to_string(caps); */

	/* _mmstreamrec_dbg_log("Set srcpad caps: %s", type); */

	/* g_free(type); */

	return caps;
}
