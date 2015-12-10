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

#ifndef __MM_STREAMRECORDER_GSTCOMMON_H__
#define __MM_STREAMRECORDER_GSTCOMMON_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/
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

/* gstreamer element creation macro */
#define _MMSTREAMRECORDER_PIPELINE_MAKE(sub_context, element, eid, name /*char* */, err) \
	if (element[eid].gst != NULL) { \
		_mmstreamrec_dbg_err("The element(Pipeline) is existed. element_id=[%d], name=[%s]", eid, name); \
		gst_object_unref(element[eid].gst); \
	} \
	element[eid].id = eid; \
	element[eid].gst = gst_pipeline_new(name); \
	if (element[eid].gst == NULL) { \
		_mmstreamrec_dbg_err("Pipeline creation fail. element_id=[%d], name=[%s]", eid, name); \
		err = MM_ERROR_STREAMRECORDER_RESOURCE_CREATION; \
		goto pipeline_creation_error; \
	} else { \
		g_object_weak_ref(G_OBJECT(element[eid].gst), (GWeakNotify)_mmstreamrecorder_element_release_noti/*NULL*/, sub_context); \
	}

#define _MMSTREAMRECORDER_BIN_MAKE(sub_context, element, eid, name /*char* */, err) \
	if (element[eid].gst != NULL) { \
		_mmstreamrec_dbg_err("The element(Bin) is existed. element_id=[%d], name=[%s]", eid, name); \
		gst_object_unref(element[eid].gst); \
	} \
	element[eid].id = eid; \
	element[eid].gst = gst_bin_new(name); \
	if (element[eid].gst == NULL) { \
		_mmstreamrec_dbg_err("Bin creation fail. element_id=[%d], name=[%s]\n", eid, name); \
		err = MM_ERROR_STREAMRECORDER_RESOURCE_CREATION; \
		goto pipeline_creation_error; \
	} else { \
		g_object_weak_ref(G_OBJECT(element[eid].gst), (GWeakNotify)_mmstreamrecorder_element_release_noti/*NULL*/, sub_context); \
	}

#define _MMSTREAMRECORDER_ELEMENT_MAKE(sub_context, element, eid, name /*char* */, nickname /*char* */, elist, err) \
	if (element[eid].gst != NULL) { \
		_mmstreamrec_dbg_err("The element is existed. element_id=[%d], name=[%s]", eid, name); \
		gst_object_unref(element[eid].gst); \
	} \
	element[eid].gst = gst_element_factory_make(name, nickname); \
	if (element[eid].gst == NULL) { \
		_mmstreamrec_dbg_err("Element creation fail. element_id=[%d], name=[%s]", eid, name); \
		err = MM_ERROR_STREAMRECORDER_RESOURCE_CREATION; \
		goto pipeline_creation_error; \
	} else { \
		_mmstreamrec_dbg_err("Element creation done. element_id=[%d], name=[%s]", eid, name); \
		element[eid].id = eid; \
		g_object_weak_ref(G_OBJECT(element[eid].gst), (GWeakNotify)_mmstreamrecorder_element_release_noti/*NULL*/, sub_context); \
		err = MM_ERROR_NONE; \
	} \
	elist = g_list_append(elist, &(element[eid]));

#define _MMSTREAMRECORDER_ELEMENT_MAKE_IGNORE_ERROR(sub_context, element, eid, name /*char* */, nickname /*char* */, elist) \
	if (element[eid].gst != NULL) { \
		_mmstreamrec_dbg_err("The element is existed. element_id=[%d], name=[%s]", eid, name); \
		gst_object_unref(element[eid].gst); \
	} \
	element[eid].gst = gst_element_factory_make(name, nickname); \
	if (element[eid].gst == NULL) { \
		_mmstreamrec_dbg_err("Element creation fail. element_id=[%d], name=[%s], but keep going...", eid, name); \
	} else { \
		_mmstreamrec_dbg_err("Element creation done. element_id=[%d], name=[%s]", eid, name); \
		element[eid].id = eid; \
		g_object_weak_ref(G_OBJECT(element[eid].gst), (GWeakNotify)_mmstreamrecorder_element_release_noti/*NULL*/, sub_context); \
		elist = g_list_append(elist, &(element[eid])); \
	}

#define _MMSTREAMRECORDER_ENCODEBIN_ELMGET(sub_context, eid, name /*char* */, err) \
	if (sub_context->encode_element[eid].gst != NULL) { \
		_mmstreamrec_dbg_err("The element is existed. element_id=[%d], name=[%s]", eid, name); \
		gst_object_unref(sub_context->encode_element[eid].gst); \
	} \
	sub_context->encode_element[eid].id = eid; \
	g_object_get(G_OBJECT(sub_context->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst), name, &(sub_context->encode_element[eid].gst), NULL); \
	if (sub_context->encode_element[eid].gst == NULL) { \
		_mmstreamrec_dbg_err("Encode Element get fail. element_id=[%d], name=[%s]", eid, name); \
		err = MM_ERROR_STREAMRECORDER_RESOURCE_CREATION; \
		goto pipeline_creation_error; \
	} else{ \
		gst_object_unref(sub_context->encode_element[eid].gst); \
		g_object_weak_ref(G_OBJECT(sub_context->encode_element[eid].gst), (GWeakNotify)_mmstreamrecorder_element_release_noti/*NULL*/, sub_context); \
	}

/* GStreamer element remove macro */
#define _MMSTREAMRECORDER_ELEMENT_REMOVE(element, eid) \
	if (element[eid].gst != NULL) { \
		gst_object_unref(element[eid].gst); \
	}

#define _MM_GST_ELEMENT_LINK_MANY       gst_element_link_many
#define _MM_GST_ELEMENT_LINK            gst_element_link
#define _MM_GST_PAD_LINK                gst_pad_link

#define _MM_GST_PAD_LINK_UNREF(srcpad, sinkpad, err, if_fail_goto)\
{\
	GstPadLinkReturn ret = _MM_GST_PAD_LINK(srcpad, sinkpad);\
	if (ret != GST_PAD_LINK_OK) {\
		GstObject *src_parent = gst_pad_get_parent(srcpad);\
		GstObject *sink_parent = gst_pad_get_parent(sinkpad);\
		char *src_name = NULL;\
		char *sink_name = NULL;\
		g_object_get((GObject *)src_parent, "name", &src_name, NULL);\
		g_object_get((GObject *)sink_parent, "name", &sink_name, NULL);\
		_mmstreamrec_dbg_err("src[%s] - sink[%s] link failed", src_name, sink_name);\
		gst_object_unref(src_parent); src_parent = NULL;\
		gst_object_unref(sink_parent); sink_parent = NULL;\
		if (src_name) {\
			free(src_name); src_name = NULL;\
		}\
		if (sink_name) {\
			free(sink_name); sink_name = NULL;\
		}\
		gst_object_unref(srcpad); srcpad = NULL;\
		gst_object_unref(sinkpad); sinkpad = NULL;\
		err = MM_ERROR_STREAMRECORDER_GST_LINK;\
		goto if_fail_goto;\
	}\
	gst_object_unref(srcpad); srcpad = NULL;\
	gst_object_unref(sinkpad); sinkpad = NULL;\
}

#define _MM_GST_PAD_UNLINK_UNREF( srcpad, sinkpad) \
	if (srcpad && sinkpad) { \
		gst_pad_unlink(srcpad, sinkpad); \
	} else { \
		_mmstreamrec_dbg_warn("some pad(srcpad:%p,sinkpad:%p) is NULL", srcpad, sinkpad); \
	} \
	if (srcpad) { \
		gst_object_unref(srcpad); srcpad = NULL; \
	} \
	if (sinkpad) { \
		gst_object_unref(sinkpad); sinkpad = NULL; \
	}

/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/

/**
* Encodebin profile
*/
typedef enum _MMStreamRecorderEncodebinProfile {
	MM_STREAMRECORDER_ENCBIN_PROFILE_VIDEO = 0,	 /**< Video recording profile */
	MM_STREAMRECORDER_ENCBIN_PROFILE_AUDIO,		 /**< Audio recording profile */
	MM_STREAMRECORDER_ENCBIN_PROFILE_IMAGE,		 /**< Image capture profile */
	MM_STREAMRECORDER_ENCBIN_PROFILE_NUM
} MMStreamRecorderEncodebinProfile;

#define __MMSTREAMRECORDER_SET_GST_STATE_TIMEOUT     5

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/
/**
 * Element name table.
 * @note if name is NULL, not supported.
 */
typedef struct {
	unsigned int prof_id;	/**< id of mmstreamrecorder_profile_attrs_id */
	unsigned int id;	/**< id of value id */
	char *name;		/**< gstreamer element name*/
} _MMStreamRecorderElementName;

/**
 * MMstreamrecorder Gstreamer Element
 */
typedef struct {
	unsigned int id;	/**< Gstreamer piplinem element name */
	GstElement *gst;	/**< Gstreamer element */
} _MMStreamRecorderGstElement;

/*=======================================================================================
| CONSTANT DEFINITIONS									|
========================================================================================*/

/*=======================================================================================
| STATIC VARIABLES									|
========================================================================================*/

/*=======================================================================================
| EXTERN GLOBAL VARIABLE								|
========================================================================================*/

/*=======================================================================================
| GLOBAL FUNCTION PROTOTYPES								|
========================================================================================*/

gboolean _mmstreamrecorder_gstreamer_init();

/* etc */
int _mmstreamrecorder_get_eos_message(MMHandleType handle);
void _mmstreamrecorder_remove_element_handle(MMHandleType handle, void *element, int first_elem, int last_elem);
gboolean _mmstreamrecorder_add_elements_to_bin(GstBin * bin, GList * element_list);
gboolean _mmstreamrecorder_link_elements(GList * element_list);

/**
 * This function sets gstreamer element status.
 * If the gstreamer fails to set status or returns asynchronous mode,
 * this function waits for state changed until timeout expired.
 *
 * @param[in]	pipeline	Pointer of pipeline
 * @param[in]	target_state	newly setting status
 * @return	This function returns zero on success, or negative value with error code.
 * @remarks
 * @see
 *
 */
int _mmstreamrecorder_gst_set_state(MMHandleType handle, GstElement *pipeline, GstState target_state);
GstCaps *gst_set_videosrcpad_caps(gint srcfmt, gint width, gint height, gint rate, gint scale);
GstCaps *gst_set_audiosrcpad_caps(gint samplerate, gint channel, gint depth, gint width, gint datatype);

#ifdef __cplusplus
}
#endif
#endif /* __MM_STREAMRECORDER_GSTCOMMON_H__ */
