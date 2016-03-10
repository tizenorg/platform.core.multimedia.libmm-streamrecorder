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
#include <stdio.h>
#include <stdarg.h>
#include <sys/vfs.h>			/* struct statfs */

#include "mm_streamrecorder_internal.h"
#include "mm_streamrecorder_gstdispatch.h"
#include "mm_streamrecorder_gstcommon.h"
#include "mm_streamrecorder_util.h"
#include "mm_streamrecorder_recorder.h"

/*-----------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/

#define _MMSTREAMRECORDER_MINIMUM_FRAME              5
#define _MMSTREAMRECORDER_FREE_SPACE_CHECK_INTERVAL  5
#define _MMSTREAMRECORDER_MINIMUM_SPACE		(512*1024)	/* byte */
#define _MMSTREAMRECORDER_MMS_MARGIN_SPACE		(512)	/* byte */
#define _MMSTREAMRECORDER_AUDIO_FREE_SPACE_CHECK_INTERVAL  10
#define _MMSTREAMRECORDER_AUDIO_MINIMUM_SPACE        (100*1024)

/*-----------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:												|
---------------------------------------------------------------------------*/
/* STATIC INTERNAL FUNCTION */

/*===========================================================================================
|																							|
|  FUNCTION DEFINITIONS																		|
========================================================================================== */
/*---------------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:											|
---------------------------------------------------------------------------*/
void _mmstreamrecorder_remove_buffer_probe(MMHandleType handle, _MMStreamRecorderHandlerCategory category)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	GList *list = NULL;
	MMStreamRecorderHandlerItem *item = NULL;

	mmf_return_if_fail(hstreamrecorder);

	if (!hstreamrecorder->buffer_probes) {
		_mmstreamrec_dbg_warn("list for buffer probe is NULL");
		return;
	}

	_mmstreamrec_dbg_log("start - category : 0x%x", category);

	list = hstreamrecorder->buffer_probes;
	while (list) {
		item = list->data;
		if (!item) {
			_mmstreamrec_dbg_err("Remove buffer probe faild, the item is NULL");
			list = g_list_next(list);
			continue;
		}

		if (item->category & category) {
			if (item->object && GST_IS_PAD(item->object)) {
				_mmstreamrec_dbg_log("Remove buffer probe on [%s:%s] - [ID : %lu], [Category : %x]", GST_DEBUG_PAD_NAME(item->object), item->handler_id, item->category);
				gst_pad_remove_probe(GST_PAD(item->object), item->handler_id);
			} else {
				_mmstreamrec_dbg_warn("Remove buffer probe faild, the pad is null or not pad, just remove item from list and free it");
			}

			list = g_list_next(list);
			hstreamrecorder->buffer_probes = g_list_remove(hstreamrecorder->buffer_probes, item);
			SAFE_FREE(item);
		} else {
			_mmstreamrec_dbg_log("Skip item : [ID : %lu], [Category : %x] ", item->handler_id, item->category);
			list = g_list_next(list);
		}
	}

	if (category == _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL) {
		g_list_free(hstreamrecorder->buffer_probes);
		hstreamrecorder->buffer_probes = NULL;
	}

	_mmstreamrec_dbg_log("done");

	return;
}

void _mmstreamrecorder_remove_event_probe(MMHandleType handle, _MMStreamRecorderHandlerCategory category)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	GList *list = NULL;
	MMStreamRecorderHandlerItem *item = NULL;

	mmf_return_if_fail(hstreamrecorder);

	if (!hstreamrecorder->event_probes) {
		_mmstreamrec_dbg_warn("list for event probe is NULL");
		return;
	}

	_mmstreamrec_dbg_log("start - category : 0x%x", category);

	list = hstreamrecorder->event_probes;
	while (list) {
		item = list->data;
		if (!item) {
			_mmstreamrec_dbg_err("Remove event probe faild, the item is NULL");
			list = g_list_next(list);
			continue;
		}

		if (item->category & category) {
			if (item->object && GST_IS_PAD(item->object)) {
				_mmstreamrec_dbg_log("Remove event probe on [%s:%s] - [ID : %lu], [Category : %x]", GST_DEBUG_PAD_NAME(item->object), item->handler_id, item->category);
				gst_pad_remove_probe(GST_PAD(item->object), item->handler_id);
			} else {
				_mmstreamrec_dbg_warn("Remove event probe faild, the pad is null or not pad, just remove item from list and free it");
			}

			list = g_list_next(list);
			hstreamrecorder->event_probes = g_list_remove(hstreamrecorder->event_probes, item);
			SAFE_FREE(item);
		} else {
			_mmstreamrec_dbg_log("Skip item : [ID : %lu], [Category : %x] ", item->handler_id, item->category);
			list = g_list_next(list);
		}
	}

	if (category == _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL) {
		g_list_free(hstreamrecorder->event_probes);
		hstreamrecorder->event_probes = NULL;
	}

	_mmstreamrec_dbg_log("done");

	return;
}

void _mmstreamrecorder_remove_data_probe(MMHandleType handle, _MMStreamRecorderHandlerCategory category)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	GList *list = NULL;
	MMStreamRecorderHandlerItem *item = NULL;

	mmf_return_if_fail(hstreamrecorder);

	if (!hstreamrecorder->data_probes) {
		_mmstreamrec_dbg_warn("list for data probe is NULL");
		return;
	}

	_mmstreamrec_dbg_log("start - category : 0x%x", category);

	list = hstreamrecorder->data_probes;
	while (list) {
		item = list->data;
		if (!item) {
			_mmstreamrec_dbg_err("Remove data probe faild, the item is NULL");
			list = g_list_next(list);
			continue;
		}

		if (item->category & category) {
			if (item->object && GST_IS_PAD(item->object)) {
				_mmstreamrec_dbg_log("Remove data probe on [%s:%s] - [ID : %lu], [Category : %x]", GST_DEBUG_PAD_NAME(item->object), item->handler_id, item->category);
				gst_pad_remove_probe(GST_PAD(item->object), item->handler_id);
			} else {
				_mmstreamrec_dbg_warn("Remove data probe faild, the pad is null or not pad, just remove item from list and free it");
			}

			list = g_list_next(list);
			hstreamrecorder->data_probes = g_list_remove(hstreamrecorder->data_probes, item);
			SAFE_FREE(item);
		} else {
			_mmstreamrec_dbg_log("Skip item : [ID : %lu], [Category : %x] ", item->handler_id, item->category);
			list = g_list_next(list);
		}
	}

	if (category == _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL) {
		g_list_free(hstreamrecorder->data_probes);
		hstreamrecorder->data_probes = NULL;
	}

	_mmstreamrec_dbg_log("done");

	return;
}

void _mmstreamrecorder_disconnect_signal(MMHandleType handle, _MMStreamRecorderHandlerCategory category)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	GList *list = NULL;
	MMStreamRecorderHandlerItem *item = NULL;

	mmf_return_if_fail(hstreamrecorder);

	if (!hstreamrecorder->signals) {
		_mmstreamrec_dbg_warn("list for signal is NULL");
		return;
	}

	_mmstreamrec_dbg_log("start - category : 0x%x", category);

	list = hstreamrecorder->signals;
	while (list) {
		item = list->data;
		if (!item) {
			_mmstreamrec_dbg_err("Fail to Disconnecting signal, the item is NULL");
			list = g_list_next(list);
			continue;
		}

		if (item->category & category) {
			if (item->object && GST_IS_ELEMENT(item->object)) {
				if (g_signal_handler_is_connected(item->object, item->handler_id)) {
					_mmstreamrec_dbg_log("Disconnect signal from [%s] : [ID : %lu], [Category : %x]", GST_OBJECT_NAME(item->object), item->handler_id, item->category);
					g_signal_handler_disconnect(item->object, item->handler_id);
				} else {
					_mmstreamrec_dbg_warn("Signal was not connected, cannot disconnect it :  [%s]  [ID : %lu], [Category : %x]", GST_OBJECT_NAME(item->object), item->handler_id, item->category);
				}
			} else {
				_mmstreamrec_dbg_err("Fail to Disconnecting signal, the element is null or not element, just remove item from list and free it");
			}

			list = g_list_next(list);
			hstreamrecorder->signals = g_list_remove(hstreamrecorder->signals, item);
			SAFE_FREE(item);
		} else {
			_mmstreamrec_dbg_log("Skip item : [ID : %lu], [Category : %x] ", item->handler_id, item->category);
			list = g_list_next(list);
		}
	}

	if (category == _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL) {
		g_list_free(hstreamrecorder->signals);
		hstreamrecorder->signals = NULL;
	}

	_mmstreamrec_dbg_log("done");

	return;
}

void _mmstreamrecorder_element_release_noti(gpointer data, GObject *where_the_object_was)
{
	int i = 0;
	_MMStreamRecorderSubContext *sc = (_MMStreamRecorderSubContext *) data;

	mmf_return_if_fail(sc);

	mmf_return_if_fail(sc->encode_element);

	for (i = 0; i < _MMSTREAMRECORDER_ENCODE_PIPELINE_ELEMENT_NUM; i++) {
		if (sc->encode_element[i].gst && (G_OBJECT(sc->encode_element[i].gst) == where_the_object_was)) {
			_mmstreamrec_dbg_warn("The encode element[%d][%p] is finalized", sc->encode_element[i].id, sc->encode_element[i].gst);
			sc->encode_element[i].gst = NULL;
			sc->encode_element[i].id = _MMSTREAMRECORDER_ENCODE_NONE;
			return;
		}
	}

	_mmstreamrec_dbg_warn("there is no matching element %p", where_the_object_was);

	return;
}

gboolean _mmstreamrecorder_msg_callback(void *data)
{
	_MMStreamRecorderMsgItem *item = (_MMStreamRecorderMsgItem *) data;
	mmf_streamrecorder_t *hstreamrecorder = NULL;
	mmf_return_val_if_fail(item, FALSE);

	hstreamrecorder = MMF_STREAMRECORDER(item->handle);
	mmf_return_val_if_fail(hstreamrecorder, FALSE);

	/* _mmstreamrec_dbg_log("msg id:%x, msg_cb:%p, msg_data:%p, item:%p", item->id, hstreamrecorder->msg_cb, hstreamrecorder->msg_data, item); */
	_MMSTREAMRECORDER_LOCK_MESSAGE_CALLBACK(hstreamrecorder);

	if ((hstreamrecorder) && (hstreamrecorder->msg_cb))
		hstreamrecorder->msg_cb(item->id, (MMMessageParamType *) (&(item->param)), hstreamrecorder->msg_cb_param);

	_MMSTREAMRECORDER_UNLOCK_MESSAGE_CALLBACK(hstreamrecorder);

	_MMSTREAMRECORDER_LOCK((MMHandleType) hstreamrecorder);
	if (hstreamrecorder->msg_data)
		hstreamrecorder->msg_data = g_list_remove(hstreamrecorder->msg_data, item);

	free(item);
	item = NULL;
	_MMSTREAMRECORDER_UNLOCK((MMHandleType) hstreamrecorder);
	/* For not being called again */
	return FALSE;
}

void _mmstreamrecorder_remove_all_handlers(MMHandleType handle, _MMStreamRecorderHandlerCategory category)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("ENTER");

	if (hstreamrecorder->signals)
		_mmstreamrecorder_disconnect_signal((MMHandleType) hstreamrecorder, category);
	if (hstreamrecorder->data_probes)
		_mmstreamrecorder_remove_data_probe((MMHandleType) hstreamrecorder, category);
	if (hstreamrecorder->event_probes)
		_mmstreamrecorder_remove_event_probe((MMHandleType) hstreamrecorder, category);
	if (hstreamrecorder->buffer_probes)
		_mmstreamrecorder_remove_buffer_probe((MMHandleType) hstreamrecorder, category);

	_mmstreamrec_dbg_log("LEAVE");
}

gboolean _mmstreamrecorder_send_message(MMHandleType handle, _MMStreamRecorderMsgItem *data)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderMsgItem *item = NULL;

	mmf_return_val_if_fail(hstreamrecorder, FALSE);
	mmf_return_val_if_fail(data, FALSE);

	/* _mmstreamrec_dbg_err("ENTER"); */

	/* _mmstreamrec_dbg_err("data->id =  %x",data->id); */

	switch (data->id) {
	case MM_MESSAGE_STREAMRECORDER_STATE_CHANGED:
		data->param.union_type = MM_MSG_UNION_STATE;
		break;
	case MM_MESSAGE_STREAMRECORDER_RECORDING_STATUS:
		data->param.union_type = MM_MSG_UNION_RECORDING_STATUS;
		break;
	case MM_MESSAGE_STREAMRECORDER_CONSUME_COMPLETE:	/* 0x801 */
		data->param.union_type = MM_MSG_UNION_CONSUME_RECORDER_BUFFER;
		break;
	case MM_MESSAGE_STREAMRECORDER_TIME_LIMIT:
	case MM_MESSAGE_STREAMRECORDER_MAX_SIZE:
	case MM_MESSAGE_STREAMRECORDER_NO_FREE_SPACE:
	case MM_MESSAGE_STREAMRECORDER_ERROR:
	case MM_MESSAGE_STREAMRECORDER_VIDEO_CAPTURED:
	case MM_MESSAGE_STREAMRECORDER_AUDIO_CAPTURED:
	case MM_MESSAGE_READY_TO_RESUME:
	default:
		data->param.union_type = MM_MSG_UNION_CODE;
		break;
	}

	item = g_malloc(sizeof(_MMStreamRecorderMsgItem));
	if (!item)
		return FALSE;
	memcpy(item, data, sizeof(_MMStreamRecorderMsgItem));
	item->handle = handle;

	_MMSTREAMRECORDER_LOCK(handle);
	hstreamrecorder->msg_data = g_list_append(hstreamrecorder->msg_data, item);
	/* _mmstreamrec_dbg_log("item[%p]", item); */

	/* Use DEFAULT priority */
	g_idle_add_full(G_PRIORITY_DEFAULT, _mmstreamrecorder_msg_callback, item, NULL);

	_MMSTREAMRECORDER_UNLOCK(handle);

	/* release allocated memory */
	if (data->id == MM_MESSAGE_STREAMRECORDER_VIDEO_CAPTURED ||
		data->id == MM_MESSAGE_STREAMRECORDER_AUDIO_CAPTURED) {
		MMStreamRecordingReport *report = (MMStreamRecordingReport *)data->param.data;
		if (report) {
			SAFE_G_FREE(report->recording_filename);
			data->param.data = NULL;
		}
		SAFE_G_FREE(report);
	}
	return TRUE;
}

gboolean _mmstreamrecorder_remove_message_all(MMHandleType handle)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderMsgItem *item = NULL;
	gboolean ret = TRUE;
	GList *list = NULL;

	mmf_return_val_if_fail(hstreamrecorder, FALSE);

	_MMSTREAMRECORDER_LOCK(handle);

	if (hstreamrecorder->msg_data) {
		list = hstreamrecorder->msg_data;

		while (list) {
			item = list->data;
			list = g_list_next(list);

			if (item) {
				ret = g_idle_remove_by_data(item);
				_mmstreamrec_dbg_log("Remove item[%p]. ret[%d]", item, ret);

				hstreamrecorder->msg_data = g_list_remove(hstreamrecorder->msg_data, item);

				SAFE_FREE(item);
			}
		}

		g_list_free(hstreamrecorder->msg_data);
		hstreamrecorder->msg_data = NULL;
	}


	_MMSTREAMRECORDER_UNLOCK(handle);

	return ret;
}

gboolean _mmstreamrecorder_handle_gst_error(MMHandleType handle, GstMessage *message, GError *error)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	/* _MMstreamrecorderMsgItem msg; */
	gchar *msg_src_element;
	_MMStreamRecorderSubContext *sc = NULL;

	return_val_if_fail(hstreamrecorder, FALSE);
	return_val_if_fail(error, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);

	_mmstreamrec_dbg_log("");

	/* filtering filesink related errors */
	if ((error->code == GST_RESOURCE_ERROR_WRITE || error->code == GST_RESOURCE_ERROR_SEEK)) {
		if (sc->ferror_count == 2 && sc->ferror_send == FALSE) {
			sc->ferror_send = TRUE;
			/* msg.param.code = __mmstreamrecorder_gst_handle_resource_error(handle, error->code, message); */
		} else {
			sc->ferror_count++;
			_mmstreamrec_dbg_warn("Skip error");
			return TRUE;
		}
	}

	if (error->domain == GST_CORE_ERROR) {
		/* msg.param.code = __mmstreamrecorder_gst_handle_core_error(handle, error->code, message); */
	} else if (error->domain == GST_LIBRARY_ERROR) {
		/* msg.param.code = __mmstreamrecorder_gst_handle_library_error(handle, error->code, message); */
	} else if (error->domain == GST_RESOURCE_ERROR) {
		/* msg.param.code = __mmstreamrecorder_gst_handle_resource_error(handle, error->code, message); */
	} else if (error->domain == GST_STREAM_ERROR) {
		/* msg.param.code = __mmstreamrecorder_gst_handle_stream_error(handle, error->code, message); */
	} else {
		_mmstreamrec_dbg_warn("This error domain is not defined.");

		/* we treat system error as an internal error */
		/* msg.param.code = MM_ERROR_streamrecorder_INTERNAL; */
	}

	if (message->src) {
		msg_src_element = GST_ELEMENT_NAME(GST_ELEMENT_CAST(message->src));
/*		_mmstreamrec_dbg_err("-Msg src : [%s] Domain : [%s]   Error : [%s]  Code : [%d] is tranlated to error code : [0x%x]",
								msg_src_element, g_quark_to_string (error->domain), error->message, error->code, msg.param.code); */
	} else {
/*		_mmstreamrec_dbg_err("Domain : [%s]   Error : [%s]  Code : [%d] is tranlated to error code : [0x%x]",
								g_quark_to_string (error->domain), error->message, error->code, msg.param.code); */
	}

#ifdef _MMSTREAMRECORDER_SKIP_GST_FLOW_ERROR
	/* Check whether send this error to application */
/*	if (msg.param.code == MM_ERROR_streamrecorder_GST_FLOW_ERROR) {
		_mmstreamrec_dbg_log("We got the error. But skip it.");
		return TRUE;
	} */
#endif							/* _MMstreamrecorder_SKIP_GST_FLOW_ERROR */

	/* post error to application */
	sc->error_occurs = TRUE;
/*	msg.id = MM_MESSAGE_streamrecorder_ERROR;
	_mmstreamrecorder_send_message(handle, &msg); */

	return TRUE;
}

gint _mmstreamrecorder_gst_handle_core_error(MMHandleType handle, int code, GstMessage *message)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	GstElement *element = NULL;

	_mmstreamrec_dbg_log("");

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	/* Specific plugin - video encoder plugin */
	element = GST_ELEMENT_CAST(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst);

	if (GST_ELEMENT_CAST(message->src) == element) {
		if (code == GST_CORE_ERROR_NEGOTIATION)
			return MM_ERROR_STREAMRECORDER_GST_NEGOTIATION;
		else
			return MM_ERROR_STREAMRECORDER_ENCODER;
	}

	/* General */
	switch (code) {
	case GST_CORE_ERROR_STATE_CHANGE:
		return MM_ERROR_STREAMRECORDER_GST_STATECHANGE;
	case GST_CORE_ERROR_NEGOTIATION:
		return MM_ERROR_STREAMRECORDER_GST_NEGOTIATION;
	case GST_CORE_ERROR_MISSING_PLUGIN:
	case GST_CORE_ERROR_SEEK:
	case GST_CORE_ERROR_NOT_IMPLEMENTED:
	case GST_CORE_ERROR_FAILED:
	case GST_CORE_ERROR_TOO_LAZY:
	case GST_CORE_ERROR_PAD:
	case GST_CORE_ERROR_THREAD:
	case GST_CORE_ERROR_EVENT:
	case GST_CORE_ERROR_CAPS:
	case GST_CORE_ERROR_TAG:
	case GST_CORE_ERROR_CLOCK:
	case GST_CORE_ERROR_DISABLED:
	default:
		return MM_ERROR_STREAMRECORDER_GST_CORE;
		break;
	}
}

gint _mmstreamrecorder_gst_handle_library_error(MMHandleType handle, int code, GstMessage *message)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_log("");

	/* Specific plugin - NONE */

	/* General */
	switch (code) {
	case GST_LIBRARY_ERROR_FAILED:
	case GST_LIBRARY_ERROR_TOO_LAZY:
	case GST_LIBRARY_ERROR_INIT:
	case GST_LIBRARY_ERROR_SHUTDOWN:
	case GST_LIBRARY_ERROR_SETTINGS:
	case GST_LIBRARY_ERROR_ENCODE:
	default:
		_mmstreamrec_dbg_err("Library error(%d)", code);
		return MM_ERROR_STREAMRECORDER_GST_LIBRARY;
	}
}

gint _mmstreamrecorder_gst_handle_resource_error(MMHandleType handle, int code, GstMessage *message)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	GstElement *element = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_log("");

	/* Specific plugin */

	/* encodebin */
	element = GST_ELEMENT_CAST(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst);
	if (GST_ELEMENT_CAST(message->src) == element) {
		if (code == GST_RESOURCE_ERROR_FAILED) {
			_mmstreamrec_dbg_err("Encoder [Resource error]");
			return MM_ERROR_STREAMRECORDER_ENCODER_BUFFER;
		} else {
			_mmstreamrec_dbg_err("Encoder [General(%d)]", code);
			return MM_ERROR_STREAMRECORDER_ENCODER;
		}
	}

	/* General */
	switch (code) {
	case GST_RESOURCE_ERROR_WRITE:
		_mmstreamrec_dbg_err("File write error");
		return MM_ERROR_FILE_WRITE;
	case GST_RESOURCE_ERROR_NO_SPACE_LEFT:
		_mmstreamrec_dbg_err("No left space");
		return MM_MESSAGE_STREAMRECORDER_NO_FREE_SPACE;
	case GST_RESOURCE_ERROR_OPEN_WRITE:
		_mmstreamrec_dbg_err("Out of storage");
		return MM_ERROR_OUT_OF_STORAGE;
	case GST_RESOURCE_ERROR_SEEK:
		_mmstreamrec_dbg_err("File read(seek)");
		return MM_ERROR_FILE_READ;
	case GST_RESOURCE_ERROR_NOT_FOUND:
	case GST_RESOURCE_ERROR_FAILED:
	case GST_RESOURCE_ERROR_TOO_LAZY:
	case GST_RESOURCE_ERROR_BUSY:
	case GST_RESOURCE_ERROR_OPEN_READ:
	case GST_RESOURCE_ERROR_OPEN_READ_WRITE:
	case GST_RESOURCE_ERROR_CLOSE:
	case GST_RESOURCE_ERROR_READ:
	case GST_RESOURCE_ERROR_SYNC:
	case GST_RESOURCE_ERROR_SETTINGS:
	default:
		_mmstreamrec_dbg_err("Resource error(%d)", code);
		return MM_ERROR_STREAMRECORDER_GST_RESOURCE;
	}
}

gint _mmstreamrecorder_gst_handle_stream_error(MMHandleType handle, int code, GstMessage *message)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	GstElement *element = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_log("");

	/* Specific plugin */
	/* video encoder */
	element = GST_ELEMENT_CAST(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst);
	if (GST_ELEMENT_CAST(message->src) == element) {
		switch (code) {
		case GST_STREAM_ERROR_WRONG_TYPE:
			_mmstreamrec_dbg_err("Video encoder [wrong stream type]");
			return MM_ERROR_STREAMRECORDER_ENCODER_WRONG_TYPE;
		case GST_STREAM_ERROR_ENCODE:
			_mmstreamrec_dbg_err("Video encoder [encode error]");
			return MM_ERROR_STREAMRECORDER_ENCODER_WORKING;
		case GST_STREAM_ERROR_FAILED:
			_mmstreamrec_dbg_err("Video encoder [stream failed]");
			return MM_ERROR_STREAMRECORDER_ENCODER_WORKING;
		default:
			_mmstreamrec_dbg_err("Video encoder [General(%d)]", code);
			return MM_ERROR_STREAMRECORDER_ENCODER;
		}
	}

	/* General plugin */
	switch (code) {
	case GST_STREAM_ERROR_FORMAT:
		_mmstreamrec_dbg_err("General [negotiation error(%d)]", code);
		return MM_ERROR_STREAMRECORDER_GST_NEGOTIATION;
	case GST_STREAM_ERROR_FAILED:
		_mmstreamrec_dbg_err("General [flow error(%d)]", code);
		return MM_ERROR_STREAMRECORDER_GST_FLOW_ERROR;
	case GST_STREAM_ERROR_TYPE_NOT_FOUND:
	case GST_STREAM_ERROR_DECODE:
	case GST_STREAM_ERROR_CODEC_NOT_FOUND:
	case GST_STREAM_ERROR_NOT_IMPLEMENTED:
	case GST_STREAM_ERROR_TOO_LAZY:
	case GST_STREAM_ERROR_ENCODE:
	case GST_STREAM_ERROR_DEMUX:
	case GST_STREAM_ERROR_MUX:
	case GST_STREAM_ERROR_DECRYPT:
	case GST_STREAM_ERROR_DECRYPT_NOKEY:
	case GST_STREAM_ERROR_WRONG_TYPE:
	default:
		_mmstreamrec_dbg_err("General [error(%d)]", code);
		return MM_ERROR_STREAMRECORDER_GST_STREAM;
	}
}

gboolean _mmstreamrecorder_handle_gst_warning(MMHandleType handle, GstMessage *message, GError *error)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	gchar *debug = NULL;
	GError *err = NULL;

	return_val_if_fail(hstreamrecorder, FALSE);
	return_val_if_fail(error, FALSE);

	_mmstreamrec_dbg_log("");

	gst_message_parse_warning(message, &err, &debug);

	if (error->domain == GST_CORE_ERROR) {
		_mmstreamrec_dbg_warn("GST warning: GST_CORE domain");
	} else if (error->domain == GST_LIBRARY_ERROR) {
		_mmstreamrec_dbg_warn("GST warning: GST_LIBRARY domain");
	} else if (error->domain == GST_RESOURCE_ERROR) {
		_mmstreamrec_dbg_warn("GST warning: GST_RESOURCE domain");
		/* _mmstreamrecorder_gst_handle_resource_warning(handle, message, error); */
	} else if (error->domain == GST_STREAM_ERROR) {
		_mmstreamrec_dbg_warn("GST warning: GST_STREAM domain");
	} else {
		_mmstreamrec_dbg_warn("This error domain(%d) is not defined.", error->domain);
	}

	if (err != NULL) {
		g_error_free(err);
		err = NULL;
	}
	if (debug != NULL) {
		_mmstreamrec_dbg_err("Debug: %s", debug);
		g_free(debug);
		debug = NULL;
	}

	return TRUE;
}

gboolean _mmstreamrecorder_pipeline_cb_message(GstBus *bus, GstMessage *message, gpointer data)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(data);
	/* _MMstreamrecorderMsgItem msg; */
	_MMStreamRecorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hstreamrecorder, FALSE);
	mmf_return_val_if_fail(message, FALSE);
	/* _mmstreamrec_dbg_log("message type=(%d)", GST_MESSAGE_TYPE(message)); */

	switch (GST_MESSAGE_TYPE(message)) {
	case GST_MESSAGE_UNKNOWN:
		_mmstreamrec_dbg_log("GST_MESSAGE_UNKNOWN");
		break;
	case GST_MESSAGE_EOS:
		{
			_mmstreamrec_dbg_log("Got EOS from element \"%s\".", GST_STR_NULL(GST_ELEMENT_NAME(GST_MESSAGE_SRC(message))));

			sc = MMF_STREAMRECORDER_SUBCONTEXT(hstreamrecorder);
			mmf_return_val_if_fail(sc, TRUE);

			/*if (hstreamrecorder->type != MM_STREAMRECORDER_MODE_AUDIO) { */
			mmf_return_val_if_fail(sc->info_video, TRUE);
			if (sc->info_video->b_commiting) {
				_mmstreamrecorder_video_handle_eos((MMHandleType) hstreamrecorder);
			}
			/*} else {
			   mmf_return_val_if_fail(sc->info_audio, TRUE);
			   if (sc->info_audio->b_commiting) {
			   _mmstreamrecorder_audio_handle_eos((MMHandleType)hstreamrecorder);
			   }
			   } */

			sc->bget_eos = TRUE;

			break;
		}
	case GST_MESSAGE_ERROR:
		{
			GError *err;
			gchar *debug;
			gst_message_parse_error(message, &err, &debug);

			_mmstreamrec_dbg_err("GSTERR: %s", err->message);
			_mmstreamrec_dbg_err("Error Debug: %s", debug);

			_mmstreamrecorder_handle_gst_error((MMHandleType) hstreamrecorder, message, err);

			g_error_free(err);
			g_free(debug);
			break;
		}
	case GST_MESSAGE_WARNING:
		{
			GError *err;
			gchar *debug;
			gst_message_parse_warning(message, &err, &debug);

			_mmstreamrec_dbg_warn("GSTWARN: %s", err->message);

			_mmstreamrecorder_handle_gst_warning((MMHandleType) hstreamrecorder, message, err);

			g_error_free(err);
			g_free(debug);
			break;
		}
	case GST_MESSAGE_INFO:
		_mmstreamrec_dbg_log("GST_MESSAGE_INFO");
		break;
	case GST_MESSAGE_TAG:
		_mmstreamrec_dbg_log("GST_MESSAGE_TAG");
		break;
	case GST_MESSAGE_BUFFERING:
		_mmstreamrec_dbg_log("GST_MESSAGE_BUFFERING");
		break;
	case GST_MESSAGE_STATE_CHANGED:
		{
			const GValue *vnewstate;
			GstState newstate;
			GstElement *pipeline = NULL;

			sc = MMF_STREAMRECORDER_SUBCONTEXT(hstreamrecorder);
			if ((sc) && (sc->encode_element)) {
				if (sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst) {
					pipeline = sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst;
					if (message->src == (GstObject *) pipeline) {
						vnewstate = (GValue *) gst_structure_get_value(gst_message_get_structure(message), "new-state");
						newstate = (GstState) vnewstate->data[0].v_int;
						_mmstreamrec_dbg_log("GST_MESSAGE_STATE_CHANGED[%s]", gst_element_state_get_name(newstate));
					}
				}
			}
			break;
		}
	case GST_MESSAGE_STATE_DIRTY:
		_mmstreamrec_dbg_log("GST_MESSAGE_STATE_DIRTY");
		break;
	case GST_MESSAGE_STEP_DONE:
		_mmstreamrec_dbg_log("GST_MESSAGE_STEP_DONE");
		break;
	case GST_MESSAGE_CLOCK_PROVIDE:
		_mmstreamrec_dbg_log("GST_MESSAGE_CLOCK_PROVIDE");
		break;
	case GST_MESSAGE_CLOCK_LOST:
		_mmstreamrec_dbg_log("GST_MESSAGE_CLOCK_LOST");
		break;
	case GST_MESSAGE_NEW_CLOCK:
		{
			GstClock *l_clock;
			gst_message_parse_new_clock(message, &l_clock);
			_mmstreamrec_dbg_log("GST_MESSAGE_NEW_CLOCK : %s", (l_clock ? GST_OBJECT_NAME(l_clock) : "NULL"));
			break;
		}
	case GST_MESSAGE_STRUCTURE_CHANGE:
		_mmstreamrec_dbg_log("GST_MESSAGE_STRUCTURE_CHANGE");
		break;
	case GST_MESSAGE_STREAM_STATUS:
		_mmstreamrec_dbg_log("GST_MESSAGE_STREAM_STATUS");
		break;
	case GST_MESSAGE_APPLICATION:
		_mmstreamrec_dbg_log("GST_MESSAGE_APPLICATION");
		break;
	case GST_MESSAGE_ELEMENT:
		_mmstreamrec_dbg_log("GST_MESSAGE_ELEMENT");
		break;
	case GST_MESSAGE_SEGMENT_START:
		_mmstreamrec_dbg_log("GST_MESSAGE_SEGMENT_START");
		break;
	case GST_MESSAGE_SEGMENT_DONE:
		_mmstreamrec_dbg_log("GST_MESSAGE_SEGMENT_DONE");
		break;
	case GST_MESSAGE_DURATION:
		_mmstreamrec_dbg_log("GST_MESSAGE_DURATION");
		break;
	case GST_MESSAGE_LATENCY:
		_mmstreamrec_dbg_log("GST_MESSAGE_LATENCY");
		break;
	case GST_MESSAGE_ASYNC_START:
		_mmstreamrec_dbg_log("GST_MESSAGE_ASYNC_START");
		break;
	case GST_MESSAGE_ASYNC_DONE:
		_mmstreamrec_dbg_log("GST_MESSAGE_ASYNC_DONE");
		break;
	case GST_MESSAGE_ANY:
		_mmstreamrec_dbg_log("GST_MESSAGE_ANY");
		break;
	default:
		_mmstreamrec_dbg_log("not handled message type=(%d)", GST_MESSAGE_TYPE(message));
		break;
	}

	return TRUE;
}

/**
 * This function is record video data probing function.
 * If this function is linked with certain pad by gst_pad_add_buffer_probe(),
 * this function will be called when data stream pass through the pad.
 *
 * @param[in]	pad		probing pad which calls this function.
 * @param[in]	buffer		buffer which contains stream data.
 * @param[in]	u_data		user data.
 * @return	This function returns true on success, or false value with error
 * @remarks
 * @see
 */
GstPadProbeReturn __mmstreamrecorder_eventprobe_monitor(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
	GstEvent *event = GST_PAD_PROBE_INFO_EVENT(info);
	switch (GST_EVENT_TYPE(event)) {
	case GST_EVENT_UNKNOWN:
		/* upstream events */
	case GST_EVENT_QOS:
	case GST_EVENT_SEEK:
	case GST_EVENT_NAVIGATION:
	case GST_EVENT_LATENCY:
		/* downstream serialized events */
	case GST_EVENT_SEGMENT:
	case GST_EVENT_TAG:
	case GST_EVENT_BUFFERSIZE:
		_mmstreamrec_dbg_log("[%s:%s] gots %s", GST_DEBUG_PAD_NAME(pad), GST_EVENT_TYPE_NAME(event));
		break;
	case GST_EVENT_EOS:
		_mmstreamrec_dbg_warn("[%s:%s] gots %s", GST_DEBUG_PAD_NAME(pad), GST_EVENT_TYPE_NAME(event));
		break;
		/* bidirectional events */
	case GST_EVENT_FLUSH_START:
	case GST_EVENT_FLUSH_STOP:
		_mmstreamrec_dbg_err("[%s:%s] gots %s", GST_DEBUG_PAD_NAME(pad), GST_EVENT_TYPE_NAME(event));
		break;
	default:
		_mmstreamrec_dbg_log("[%s:%s] gots %s", GST_DEBUG_PAD_NAME(pad), GST_EVENT_TYPE_NAME(event));
		break;
	}

	return GST_PAD_PROBE_OK;
}

GstPadProbeReturn __mmstreamrecorder_audio_dataprobe_check(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(u_data);
	GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
	GstMapInfo mapinfo = GST_MAP_INFO_INIT;
	guint64 buffer_size = 0;

	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *audioinfo = NULL;

	mmf_return_val_if_fail(hstreamrecorder, GST_PAD_PROBE_OK);
	mmf_return_val_if_fail(buffer, GST_PAD_PROBE_DROP);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(hstreamrecorder);

	mmf_return_val_if_fail(sc && sc->info_audio, GST_PAD_PROBE_OK);
	memset(&mapinfo, 0x0, sizeof(GstMapInfo));

	audioinfo = sc->info_audio;

	gst_buffer_map(buffer, &mapinfo, GST_MAP_READ);
	buffer_size = mapinfo.size;
	gst_buffer_unmap(buffer, &mapinfo);

	/*_mmstreamrec_dbg_err("[%" GST_TIME_FORMAT "]", GST_TIME_ARGS(GST_BUFFER_TIMESTAMP(buffer)));*/

	if (audioinfo->audio_frame_count == 0) {
		audioinfo->filesize += buffer_size;
		audioinfo->audio_frame_count++;
		return GST_PAD_PROBE_OK;
	}

	if (sc->ferror_send || sc->isMaxsizePausing) {
		_mmstreamrec_dbg_warn("Recording is paused, drop frames");
		return GST_PAD_PROBE_DROP;
	}

	audioinfo->filesize += buffer_size;
	audioinfo->audio_frame_count++;

	return GST_PAD_PROBE_OK;
}

GstPadProbeReturn __mmstreamrecorder_video_dataprobe_record(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
	static int count = 0;
	gint ret = 0;
	guint vq_size = 0;
	guint aq_size = 0;
	guint64 free_space = 0;
	guint64 buffer_size = 0;
	guint64 trailer_size = 0;
	guint64 queued_buffer = 0;
	char *filename = NULL;
	GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
	GstMapInfo mapinfo = GST_MAP_INFO_INIT;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(u_data);
	_MMStreamRecorderMsgItem msg;
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderVideoInfo *videoinfo = NULL;
	_MMStreamRecorderFileInfo *finfo = NULL;

	mmf_return_val_if_fail(hstreamrecorder, GST_PAD_PROBE_OK);
	mmf_return_val_if_fail(buffer, GST_PAD_PROBE_DROP);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(hstreamrecorder);
	mmf_return_val_if_fail(sc && sc->info_video && sc->info_file, TRUE);
	memset(&mapinfo, 0x0, sizeof(GstMapInfo));

	videoinfo = sc->info_video;
	finfo = sc->info_file;

	/*_mmstreamrec_dbg_log("[%" GST_TIME_FORMAT "]", GST_TIME_ARGS(GST_BUFFER_TIMESTAMP(buffer)));*/
	if (sc->ferror_send) {
		_mmstreamrec_dbg_warn("file write error, drop frames");
		return GST_PAD_PROBE_DROP;
	}
	gst_buffer_map(buffer, &mapinfo, GST_MAP_READ);
	buffer_size = mapinfo.size;
	gst_buffer_unmap(buffer, &mapinfo);

	videoinfo->video_frame_count++;
	if (videoinfo->video_frame_count <= (guint64) _MMSTREAMRECORDER_MINIMUM_FRAME) {
		/* _mmstreamrec_dbg_log("Pass minimum frame: info->video_frame_count: %" G_GUINT64_FORMAT " ", info->video_frame_count); */
		videoinfo->filesize += buffer_size;
		return GST_PAD_PROBE_OK;
	}

	/* get trailer size */
	if (finfo->fileformat == MM_FILE_FORMAT_3GP || finfo->fileformat == MM_FILE_FORMAT_MP4) {
		MMSTREAMRECORDER_G_OBJECT_GET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_MUX].gst, "expected-trailer-size", &trailer_size);
	} else {
		trailer_size = 0;
	}

	/* to minimizing free space check overhead */
	count = count % _MMSTREAMRECORDER_FREE_SPACE_CHECK_INTERVAL;
	if (count++ == 0) {
		filename = finfo->filename;
		ret = _mmstreamrecorder_get_freespace(filename, &free_space);

		/*_mmstreamrec_dbg_log("check free space for recording");*/

		switch (ret) {
		case -2:				/* file not exist */
		case -1:				/* failed to get free space */
			_mmstreamrec_dbg_err("Error occured. [%d]", ret);
			if (sc->ferror_count == 2 && sc->ferror_send == FALSE) {
				sc->ferror_send = TRUE;
				msg.id = MM_MESSAGE_STREAMRECORDER_ERROR;
				if (ret == -2) {
					msg.param.code = MM_ERROR_FILE_NOT_FOUND;
				} else {
					msg.param.code = MM_ERROR_FILE_READ;
				}
				_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);
			} else {
				sc->ferror_count++;
			}

			return GST_PAD_PROBE_DROP;	/* skip this buffer */
			break;
		default:				/* succeeded to get free space */
			/* check free space for recording */
			/* get queued buffer size */
			if (sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC_QUE].gst) {
				MMSTREAMRECORDER_G_OBJECT_GET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC_QUE].gst, "current-level-bytes", &aq_size);
			}
			if (sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC_QUE].gst) {
				MMSTREAMRECORDER_G_OBJECT_GET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC_QUE].gst, "current-level-bytes", &vq_size);
			}

			queued_buffer = aq_size + vq_size;

			/* check free space */
			if (free_space < (_MMSTREAMRECORDER_MINIMUM_SPACE + buffer_size + trailer_size + queued_buffer)) {
				_mmstreamrec_dbg_warn("No more space for recording!!! Recording is paused.");
				_mmstreamrec_dbg_warn("Free Space : [%" G_GUINT64_FORMAT "], trailer size : [%" G_GUINT64_FORMAT "]," " buffer size : [%" G_GUINT64_FORMAT "], queued buffer size : [%" G_GUINT64_FORMAT "]", free_space, trailer_size, buffer_size, queued_buffer);

				if (!sc->isMaxsizePausing) {
					MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "block", TRUE);
					sc->isMaxsizePausing = TRUE;

					msg.id = MM_MESSAGE_STREAMRECORDER_NO_FREE_SPACE;
					_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);
				}

				return GST_PAD_PROBE_DROP;
			}
			break;
		}
	}

	videoinfo->filesize += (guint64) buffer_size;

	/*
	   _mmstreamrec_dbg_log("filesize %lld Byte, ", info->filesize);
	 */

	return GST_PAD_PROBE_OK;
}

GstPadProbeReturn __mmstreamrecorder_video_dataprobe_audio_disable(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
	guint64 trailer_size = 0;
	guint64 rec_pipe_time = 0;
	unsigned int remained_time = 0;

	GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);
	GstClockTime b_time;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(u_data);
	_MMStreamRecorderMsgItem msg;
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderVideoInfo *videoinfo = NULL;
	_MMStreamRecorderFileInfo *finfo = NULL;

	mmf_return_val_if_fail(buffer, GST_PAD_PROBE_DROP);
	mmf_return_val_if_fail(hstreamrecorder, GST_PAD_PROBE_OK);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(u_data);
	mmf_return_val_if_fail(sc, GST_PAD_PROBE_OK);
	mmf_return_val_if_fail(sc->info_video, GST_PAD_PROBE_OK);
	mmf_return_val_if_fail(sc->info_file, GST_PAD_PROBE_OK);

	videoinfo = sc->info_video;
	finfo = sc->info_file;

	b_time = GST_BUFFER_TIMESTAMP(buffer);

	rec_pipe_time = GST_TIME_AS_MSECONDS(b_time);

	if (finfo->fileformat == MM_FILE_FORMAT_3GP || finfo->fileformat == MM_FILE_FORMAT_MP4) {
		MMSTREAMRECORDER_G_OBJECT_GET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_MUX].gst, "expected-trailer-size", &trailer_size);
	} else {
		trailer_size = 0;
	}

	/* check max time */
	if (videoinfo->max_time > 0 && rec_pipe_time > videoinfo->max_time) {
		_mmstreamrec_dbg_warn("Current time : [%" G_GUINT64_FORMAT "], Maximum time : [%" G_GUINT64_FORMAT "]", rec_pipe_time, videoinfo->max_time);

		if (!sc->isMaxtimePausing) {
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "block", TRUE);

			sc->isMaxtimePausing = TRUE;

			msg.id = MM_MESSAGE_STREAMRECORDER_RECORDING_STATUS;
			msg.param.recording_status.elapsed = (unsigned long long)rec_pipe_time;
			msg.param.recording_status.filesize = (unsigned long long)((videoinfo->filesize + trailer_size) >> 10);
			msg.param.recording_status.remained_time = 0;
			_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);

			msg.id = MM_MESSAGE_STREAMRECORDER_TIME_LIMIT;
			_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);
		}

		return GST_PAD_PROBE_DROP;
	}

	if (videoinfo->max_time > 0 && videoinfo->max_time < (remained_time + rec_pipe_time))
		remained_time = videoinfo->max_time - rec_pipe_time;

	msg.id = MM_MESSAGE_STREAMRECORDER_RECORDING_STATUS;
	msg.param.recording_status.elapsed = (unsigned long long)rec_pipe_time;
	msg.param.recording_status.filesize = (unsigned long long)((videoinfo->filesize + trailer_size) >> 10);
	msg.param.recording_status.remained_time = remained_time;
	_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);

	/*
	   _mmstreamrec_dbg_log("time [%" GST_TIME_FORMAT "], size [%d]",
	   GST_TIME_ARGS(rec_pipe_time), msg.param.recording_status.filesize);
	 */

	return GST_PAD_PROBE_OK;
}

GstPadProbeReturn __mmstreamrecorder_audioque_dataprobe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
	_MMStreamRecorderMsgItem msg;
	guint64 trailer_size = 0;
	guint64 rec_pipe_time = 0;
	_MMStreamRecorderSubContext *sc = NULL;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(u_data);
	_MMStreamRecorderVideoInfo *videoinfo = NULL;
	_MMStreamRecorderFileInfo *finfo = NULL;
	unsigned int remained_time = 0;
	GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);

	mmf_return_val_if_fail(buffer, GST_PAD_PROBE_DROP);
	mmf_return_val_if_fail(hstreamrecorder, GST_PAD_PROBE_OK);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(u_data);

	mmf_return_val_if_fail(sc, GST_PAD_PROBE_OK);
	mmf_return_val_if_fail(sc->info_video, GST_PAD_PROBE_OK);
	mmf_return_val_if_fail(sc->info_file, GST_PAD_PROBE_OK);

	videoinfo = sc->info_video;
	finfo = sc->info_file;

	if (!GST_CLOCK_TIME_IS_VALID(GST_BUFFER_PTS(buffer))) {
		_mmstreamrec_dbg_err("Buffer timestamp is invalid, check it");
		return GST_PAD_PROBE_OK;
	}

	rec_pipe_time = GST_TIME_AS_MSECONDS(GST_BUFFER_TIMESTAMP(buffer));

	if (finfo->fileformat == MM_FILE_FORMAT_3GP || finfo->fileformat == MM_FILE_FORMAT_MP4) {
		MMSTREAMRECORDER_G_OBJECT_GET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_MUX].gst, "expected-trailer-size", &trailer_size);
	} else {
		trailer_size = 0;
	}

	if (videoinfo->max_time > 0 && videoinfo->max_time < (remained_time + rec_pipe_time))
		remained_time = videoinfo->max_time - rec_pipe_time;

	if (videoinfo->max_time > 0 && rec_pipe_time > videoinfo->max_time) {
		_mmstreamrec_dbg_warn("Current time : [%" G_GUINT64_FORMAT "], Maximum time : [%" G_GUINT64_FORMAT "]", rec_pipe_time, videoinfo->max_time);

		if (!sc->isMaxtimePausing) {
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "block", TRUE);

			sc->isMaxtimePausing = TRUE;

			msg.id = MM_MESSAGE_STREAMRECORDER_RECORDING_STATUS;
			msg.param.recording_status.elapsed = (unsigned long long)rec_pipe_time;
			msg.param.recording_status.filesize = (unsigned long long)((videoinfo->filesize + trailer_size) >> 10);
			msg.param.recording_status.remained_time = 0;
			_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);

			msg.id = MM_MESSAGE_STREAMRECORDER_TIME_LIMIT;
			_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);
		}

		return GST_PAD_PROBE_DROP;
	}

	msg.id = MM_MESSAGE_STREAMRECORDER_RECORDING_STATUS;
	msg.param.recording_status.elapsed = (unsigned long long)rec_pipe_time;
	msg.param.recording_status.filesize = (unsigned long long)((videoinfo->filesize + trailer_size) >> 10);
	msg.param.recording_status.remained_time = remained_time;
	_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);

	/*
	   _mmstreamrec_dbg_log("audio data probe :: time [%" GST_TIME_FORMAT "], size [%lld KB]",
	   GST_TIME_ARGS(rec_pipe_time), msg.param.recording_status.filesize);
	 */

	return GST_PAD_PROBE_OK;
}

void __mmstreamrecorder_audiorec_pad_added_cb(GstElement *element, GstPad *pad, MMHandleType handle)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("ENTER(%s)", GST_PAD_NAME(pad));
	/* FIXME : the name of audio sink pad of wavparse, oggmux doesn't have 'audio'. How could I handle the name? */
	if ((strstr(GST_PAD_NAME(pad), "audio")) || (strstr(GST_PAD_NAME(pad), "sink"))) {
		MMSTREAMRECORDER_ADD_BUFFER_PROBE(pad, _MMSTREAMRECORDER_HANDLER_AUDIOREC, __mmstreamrecorder_audio_dataprobe_record, hstreamrecorder);
	} else {
		_mmstreamrec_dbg_warn("Unknow pad is added, check it : [%s]", GST_PAD_NAME(pad));
	}

	return;
}

GstPadProbeReturn __mmstreamrecorder_audio_dataprobe_record(GstPad *pad, GstPadProbeInfo *info, gpointer u_data)
{
	static int count = 0;
	guint64 rec_pipe_time = 0;
	guint64 free_space = 0;
	guint64 buffer_size = 0;
	guint64 trailer_size = 0;
	char *filename = NULL;
	unsigned long long remained_time = 0;
	GstBuffer *buffer = GST_PAD_PROBE_INFO_BUFFER(info);

	_MMStreamRecorderSubContext *sc = NULL;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(u_data);
	_MMStreamRecorderAudioInfo *audioinfo = NULL;
	_MMStreamRecorderFileInfo *finfo = NULL;
	_MMStreamRecorderMsgItem msg;

	mmf_return_val_if_fail(hstreamrecorder, GST_PAD_PROBE_DROP);
	mmf_return_val_if_fail(buffer, GST_PAD_PROBE_DROP);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(hstreamrecorder);
	mmf_return_val_if_fail(sc && sc->info_audio && sc->info_file, FALSE);
	audioinfo = sc->info_audio;
	finfo = sc->info_file;

	if (sc->isMaxtimePausing || sc->isMaxsizePausing) {
		_mmstreamrec_dbg_warn("isMaxtimePausing[%d],isMaxsizePausing[%d]", sc->isMaxtimePausing, sc->isMaxsizePausing);
		return GST_PAD_PROBE_DROP;
	}

	buffer_size = gst_buffer_get_size(buffer);

	if (audioinfo->filesize == 0) {
		if (finfo->fileformat == MM_FILE_FORMAT_WAV) {
			audioinfo->filesize += 44;	/* wave header size */
		} else if (finfo->fileformat == MM_FILE_FORMAT_AMR) {
			audioinfo->filesize += 6;	/* amr header size */
		}

		audioinfo->filesize += buffer_size;
		return GST_PAD_PROBE_OK;
	}

	if (sc->ferror_send) {
		_mmstreamrec_dbg_warn("file write error, drop frames");
		return GST_PAD_PROBE_DROP;
	}

	/* get trailer size */
	if (finfo->fileformat == MM_FILE_FORMAT_3GP || finfo->fileformat == MM_FILE_FORMAT_MP4 || finfo->fileformat == MM_FILE_FORMAT_AAC) {
		MMSTREAMRECORDER_G_OBJECT_GET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_MUX].gst, "expected-trailer-size", &trailer_size);
		/*_mmstreamrec_dbg_log("trailer_size %d", trailer_size);*/
	} else {
		trailer_size = 0;		/* no trailer */
	}

	filename = finfo->filename;

	/* to minimizing free space check overhead */
	count = count % _MMSTREAMRECORDER_AUDIO_FREE_SPACE_CHECK_INTERVAL;
	if (count++ == 0) {
		gint free_space_ret = _mmstreamrecorder_get_freespace(filename, &free_space);

		/*_mmstreamrec_dbg_log("check free space for recording");*/

		switch (free_space_ret) {
		case -2:				/* file not exist */
		case -1:				/* failed to get free space */
			_mmstreamrec_dbg_err("Error occured. [%d]", free_space_ret);
			if (sc->ferror_count == 2 && sc->ferror_send == FALSE) {
				sc->ferror_send = TRUE;
				msg.id = MM_MESSAGE_STREAMRECORDER_ERROR;
				if (free_space_ret == -2) {
					msg.param.code = MM_ERROR_FILE_NOT_FOUND;
				} else {
					msg.param.code = MM_ERROR_FILE_READ;
				}
				_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);
			} else {
				sc->ferror_count++;
			}

			return GST_PAD_PROBE_DROP;	/* skip this buffer */

		default:				/* succeeded to get free space */
			/* check free space for recording */
			if (free_space < (guint64) (_MMSTREAMRECORDER_AUDIO_MINIMUM_SPACE + buffer_size + trailer_size)) {
				_mmstreamrec_dbg_warn("No more space for recording!!!");
				_mmstreamrec_dbg_warn("Free Space : [%" G_GUINT64_FORMAT "], file size : [%" G_GUINT64_FORMAT "]", free_space, audioinfo->filesize);

				if (audioinfo->bMuxing) {
					MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "block", TRUE);
				} else {
					MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, "empty-buffers", TRUE);
				}

				sc->isMaxsizePausing = TRUE;
				msg.id = MM_MESSAGE_STREAMRECORDER_NO_FREE_SPACE;
				_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);

				return FALSE;	/* skip this buffer */
			}
			break;
		}
	}

	if (!GST_CLOCK_TIME_IS_VALID(GST_BUFFER_PTS(buffer))) {
		_mmstreamrec_dbg_err("Buffer timestamp is invalid, check it");
		return GST_PAD_PROBE_DROP;
	}

	rec_pipe_time = GST_TIME_AS_MSECONDS(GST_BUFFER_TIMESTAMP(buffer));

	if (audioinfo->max_time > 0 && audioinfo->max_time < (remained_time + rec_pipe_time))
		remained_time = audioinfo->max_time - rec_pipe_time;

	/*_mmstreamrec_dbg_log("remained time : %u", remained_time);*/

	/* check recording time limit and send recording status message */
	if (audioinfo->max_time > 0 && rec_pipe_time > audioinfo->max_time) {
		_mmstreamrec_dbg_warn("Current time : [%" G_GUINT64_FORMAT "], Maximum time : [%" G_GUINT64_FORMAT "]", rec_pipe_time, audioinfo->max_time);

		if (audioinfo->bMuxing) {
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "block", TRUE);
		} else {
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, "empty-buffers", TRUE);
		}

		sc->isMaxtimePausing = TRUE;
		msg.id = MM_MESSAGE_STREAMRECORDER_TIME_LIMIT;
		_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);

		/* skip this buffer */
		return GST_PAD_PROBE_DROP;
	}

	/* send message for recording time and recorded file size */
	if (audioinfo->b_commiting == FALSE) {
		audioinfo->filesize += buffer_size;

		msg.id = MM_MESSAGE_STREAMRECORDER_RECORDING_STATUS;
		msg.param.recording_status.elapsed = (unsigned long long)rec_pipe_time;
		msg.param.recording_status.filesize = (unsigned long long)((audioinfo->filesize + trailer_size) >> 10);
		msg.param.recording_status.remained_time = remained_time;
		_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);

		return GST_PAD_PROBE_OK;
	} else {
		/* skip this buffer if commit process has been started */
		return GST_PAD_PROBE_DROP;
	}
}
