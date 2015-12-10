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

#ifndef __MM_STREAMRECORDER_GSTDISPATCH_H__
#define __MM_STREAMRECORDER_GSTDISPATCH_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/
#include "mm_streamrecorder_util.h"
#include <gst/gst.h>
#include <gst/gstutils.h>
#include <gst/gstpad.h>
#include <mm_types.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| GLOBAL DEFINITIONS AND DECLARATIONS FOR STREAMRECORDER					|
========================================================================================*/

/*=======================================================================================
| MACRO DEFINITIONS									|
========================================================================================*/

#define MMSTREAMRECORDER_ADD_BUFFER_PROBE(x_pad, x_category, x_callback, x_hstreamrecorder) \
do { \
	MMStreamRecorderHandlerItem *item = NULL; \
	item = (MMStreamRecorderHandlerItem *)g_malloc(sizeof(MMStreamRecorderHandlerItem)); \
	if (!item) {\
		_mmstreamrec_dbg_err("Cannot connect buffer probe [malloc fail] \n"); \
	} else if (x_category == 0 || !(x_category & _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL)) { \
		_mmstreamrec_dbg_err("Invalid handler category : %x \n", x_category); \
	} else { \
		item->object = G_OBJECT(x_pad); \
		item->category = x_category; \
		item->handler_id = gst_pad_add_probe(x_pad,GST_PAD_PROBE_TYPE_BUFFER,x_callback, x_hstreamrecorder,NULL); \
		x_hstreamrecorder->buffer_probes = g_list_append(x_hstreamrecorder->buffer_probes, item); \
		_mmstreamrec_dbg_log("Adding buffer probe on [%s:%s] - [ID : %lu], [Category : %x] ", GST_DEBUG_PAD_NAME(item->object), item->handler_id, item->category); \
	} \
} while (0);

#define MMSTREAMRECORDER_ADD_EVENT_PROBE(x_pad, x_category, x_callback, x_hstreamrecorder) \
do { \
	MMStreamRecorderHandlerItem *item = NULL; \
	item = (MMStreamRecorderHandlerItem *) g_malloc(sizeof(MMStreamRecorderHandlerItem)); \
	if (!item) { \
		_mmstreamrec_dbg_err("Cannot connect buffer probe [malloc fail] \n"); \
	} \
	else if (x_category == 0 || !(x_category & _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL)) { \
		_mmstreamrec_dbg_err("Invalid handler category : %x \n", x_category); \
	} else { \
		item->object =G_OBJECT(x_pad); \
		item->category = x_category; \
		item->handler_id = gst_pad_add_probe(x_pad,GST_PAD_PROBE_TYPE_EVENT_BOTH,x_callback, x_hstreamrecorder,NULL); \
		x_hstreamrecorder->event_probes = g_list_append(x_hstreamrecorder->event_probes, item); \
		_mmstreamrec_dbg_log("Adding event probe on [%s:%s] - [ID : %lu], [Category : %x] ", GST_DEBUG_PAD_NAME(item->object), item->handler_id, item->category); \
	} \
} while (0);

#define MMSTREAMRECORDER_ADD_DATA_PROBE(x_pad, x_category, x_callback, x_hstreamrecorder) \
do { \
	MMStreamRecorderHandlerItem *item = NULL; \
	item = (MMStreamRecorderHandlerItem *) g_malloc(sizeof(MMStreamRecorderHandlerItem)); \
	if (!item) { \
		_mmstreamrec_dbg_err("Cannot connect buffer probe [malloc fail] \n"); \
	} else if (x_category == 0 || !(x_category & _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL)) { \
		_mmstreamrec_dbg_err("Invalid handler category : %x \n", x_category); \
	} else { \
		item->object =G_OBJECT(x_pad); \
		item->category = x_category; \
		item->handler_id = gst_pad_add_data_probe(x_pad, G_CALLBACK(x_callback), x_hstreamrecorder); \
		x_hstreamrecorder->data_probes = g_list_append(x_hstreamrecorder->data_probes, item); \
		_mmstreamrec_dbg_log("Adding data probe on [%s:%s] - [ID : %lu], [Category : %x] ", GST_DEBUG_PAD_NAME(item->object), item->handler_id, item->category); \
	} \
} while (0);

#define MMSTREAMRECORDER_SIGNAL_CONNECT( x_object, x_category, x_signal, x_callback, x_hstreamrecorder) \
do { \
	MMStreamRecorderHandlerItem* item = NULL; \
	item = (MMStreamRecorderHandlerItem *) g_malloc(sizeof(MMStreamRecorderHandlerItem)); \
	if (!item) { \
		_mmstreamrec_dbg_err("Cannot connect signal [%s]\n", x_signal ); \
	} else if (x_category == 0 || !(x_category & _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL)) { \
		_mmstreamrec_dbg_err("Invalid handler category : %x \n", x_category); \
	} else { \
		item->object = G_OBJECT(x_object); \
		item->category = x_category; \
		item->handler_id = g_signal_connect(G_OBJECT(x_object), x_signal,\
						    G_CALLBACK(x_callback), x_hstreamrecorder ); \
		x_hstreamrecorder->signals = g_list_append(x_hstreamrecorder->signals, item); \
		_mmstreamrec_dbg_log("Connecting signal on [%s] - [ID : %lu], [Category : %x] ", GST_OBJECT_NAME(item->object), item->handler_id, item->category); \
	} \
} while (0);

#define MMSTREAMRECORDER_G_OBJECT_GET(obj, name, value) \
do { \
	if (obj) { \
		if(g_object_class_find_property(G_OBJECT_GET_CLASS(G_OBJECT(obj)), name)) { \
			g_object_get(G_OBJECT(obj), name, value, NULL); \
		} else { \
			_mmstreamrec_dbg_warn ("The object doesn't have a property named(%s)", name); \
		} \
	} else { \
		_mmstreamrec_dbg_err("Null object"); \
	} \
} while(0);

#define MMSTREAMRECORDER_G_OBJECT_SET(obj, name, value) \
do { \
	if (obj) { \
		if(g_object_class_find_property(G_OBJECT_GET_CLASS(G_OBJECT(obj)), name)) { \
			g_object_set(G_OBJECT(obj), name, value, NULL); \
		} else { \
			_mmstreamrec_dbg_warn ("The object doesn't have a property named(%s)", name); \
		} \
	} else { \
		_mmstreamrec_dbg_err("Null object"); \
	} \
} while(0);

#define MMSTREAMRECORDER_SEND_MESSAGE(handle, msg_id, msg_code) \
{\
	_MMStreamRecorderMsgItem msg;\
	msg.id = msg_id;\
	msg.param.code = msg_code;\
	_mmstreamrec_dbg_log("msg id : %x, code : %x", msg_id, msg_code);\
	_mmstreamrecorder_send_message((MMHandleType)handle, &msg);\
}

/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/
/**
 *Type define of util.
 */
typedef enum {
	_MMSTREAMRECORDER_HANDLER_VIDEOREC = (1 << 0),
	_MMSTREAMRECORDER_HANDLER_AUDIOREC = (1 << 1),
} _MMStreamRecorderHandlerCategory;

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/

/**
 * Structure of handler item
 */
typedef struct {
	GObject *object;
	_MMStreamRecorderHandlerCategory category;
	gulong handler_id;
} MMStreamRecorderHandlerItem;

/**
 * Structure of message item
 */
typedef struct {
	MMHandleType handle;	/**< handle */
	int id;			/**< message id */
	MMMessageParamType param;
							/**< message parameter */
} _MMStreamRecorderMsgItem;

/*=======================================================================================
| CONSTANT DEFINITIONS									|
========================================================================================*/
#define _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL \
	(_MMSTREAMRECORDER_HANDLER_VIDEOREC | _MMSTREAMRECORDER_HANDLER_AUDIOREC)

/*=======================================================================================
| GLOBAL FUNCTION PROTOTYPES								|
========================================================================================*/
/* GStreamer */
void _mmstreamrecorder_remove_buffer_probe(MMHandleType handle, _MMStreamRecorderHandlerCategory category);
void _mmstreamrecorder_remove_event_probe(MMHandleType handle, _MMStreamRecorderHandlerCategory category);
void _mmstreamrecorder_remove_data_probe(MMHandleType handle, _MMStreamRecorderHandlerCategory category);
void _mmstreamrecorder_disconnect_signal(MMHandleType handle, _MMStreamRecorderHandlerCategory category);
void _mmstreamrecorder_element_release_noti(gpointer data, GObject *where_the_object_was);

/* Message */
gboolean _mmstreamrecorder_msg_callback(void *data);
gboolean _mmstreamrecorder_send_message(MMHandleType handle, _MMStreamRecorderMsgItem *data);
gboolean _mmstreamrecorder_remove_message_all(MMHandleType handle);

gboolean _mmstreamrecorder_handle_gst_error(MMHandleType handle, GstMessage *message, GError *error);
gint _mmstreamrecorder_gst_handle_stream_error(MMHandleType handle, int code, GstMessage *message);
gint _mmstreamrecorder_gst_handle_resource_error(MMHandleType handle, int code, GstMessage *message);
gint _mmstreamrecorder_gst_handle_library_error(MMHandleType handle, int code, GstMessage *message);
gint _mmstreamrecorder_gst_handle_core_error(MMHandleType handle, int code, GstMessage *message);
gboolean _mmstreamrecorder_handle_gst_warning(MMHandleType handle, GstMessage *message, GError *error);
void _mmstreamrecorder_remove_all_handlers(MMHandleType handle, _MMStreamRecorderHandlerCategory category);

/**
 * This function is callback function of main pipeline.
 * Once this function is registered with certain pipeline using gst_bus_add_watch(),
 * this callback will be called every time when there is upcomming message from pipeline.
 * Basically, this function is used as error handling function, now.
 *
 * @param[in]	bus		pointer of buf that called this function.
 * @param[in]	message		callback message from pipeline.
 * @param[in]	data		user data.
 * @return	This function returns true on success, or false value with error
 * @remarks
 * @see		__mmstreamrecorder_create_preview_pipeline()
 *
 */
gboolean _mmstreamrecorder_pipeline_cb_message(GstBus *bus, GstMessage *message, gpointer data);

GstPadProbeReturn __mmstreamrecorder_eventprobe_monitor(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);

GstPadProbeReturn __mmstreamrecorder_video_dataprobe_audio_disable(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);

GstPadProbeReturn __mmstreamrecorder_video_dataprobe_record(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);

GstPadProbeReturn __mmstreamrecorder_audio_dataprobe_check(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);

GstPadProbeReturn __mmstreamrecorder_audioque_dataprobe(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);

GstPadProbeReturn __mmstreamrecorder_audio_dataprobe_record(GstPad *pad, GstPadProbeInfo *info, gpointer u_data);
void __mmstreamrecorder_audiorec_pad_added_cb(GstElement *element, GstPad *pad, MMHandleType handle);

#ifdef __cplusplus
}
#endif
#endif							/* __MM_STREAMRECORDER_GSTDISPATCH_H__ */
