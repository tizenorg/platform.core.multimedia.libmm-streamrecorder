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
#include "mm_streamrecorder_internal.h"
#include "mm_streamrecorder_video.h"
#include "mm_streamrecorder_gstcommon.h"
#include "mm_streamrecorder_recorder.h"
#include "mm_streamrecorder_util.h"
#include "mm_streamrecorder_buffer_manager.h"
#include "mm_streamrecorder_fileinfo.h"
#include "mm_streamrecorder_attribute.h"

#include <gst/gst.h>
#include <gst/base/gstadapter.h>
#include <gst/app/gstappsrc.h>
#include <gst/app/gstappsink.h>


/*---------------------------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:								|
---------------------------------------------------------------------------------------*/
/* STATIC INTERNAL FUNCTION */

/*=======================================================================================
|  FUNCTION DEFINITIONS									|
=======================================================================================*/
/*---------------------------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:							|
---------------------------------------------------------------------------------------*/
int _mmstreamrecorder_create_pipeline(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	GstElement *pipeline = NULL;

	_mmstreamrec_dbg_log("handle : %x", handle);

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	/* ENCODER MODE */
	ret = _mmstreamrecorder_create_recorder_pipeline(handle);

	pipeline = sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst;

	ret = _mmstreamrecorder_gst_set_state(handle, pipeline, GST_STATE_READY);

	_mmstreamrec_dbg_log("ret[%x]", ret);
	return ret;
}

void _mmstreamrecorder_destroy_pipeline(MMHandleType handle)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	gint i = 0;
	int element_num = 0;
	_MMStreamRecorderGstElement *element = NULL;
	GstBus *bus = NULL;

	mmf_return_if_fail(hstreamrecorder);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_if_fail(sc);

	_mmstreamrec_dbg_log("");

	element = sc->encode_element;
	element_num = sc->encode_element_num;
	if (element == NULL) {
		_mmstreamrec_dbg_log("encode element is null!!");
		return;
	}

	if (sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst != NULL) {
		bus = gst_pipeline_get_bus(GST_PIPELINE(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst));

		_mmstreamrec_dbg_log("Pipeline clear!!");

		/* Remove pipeline message callback */
		if (hstreamrecorder->pipeline_cb_event_id != 0) {
			g_source_remove(hstreamrecorder->pipeline_cb_event_id);
			hstreamrecorder->pipeline_cb_event_id = 0;
		}

		/* Remove remained message in bus */
		if (bus) {
			GstMessage *gst_msg = NULL;
			while ((gst_msg = gst_bus_pop(bus)) != NULL) {
				_mmstreamrecorder_pipeline_cb_message(bus, gst_msg, (gpointer) hstreamrecorder);
				gst_message_unref(gst_msg);
				gst_msg = NULL;
			}
			gst_object_unref(bus);
			bus = NULL;
		}

		/* Inside each pipeline destroy function, Set GST_STATE_NULL to Main pipeline */
		_mmstreamrecorder_destroy_recorder_pipeline(handle);
	}

	if (element != NULL) {
		/* checking unreleased element */
		for (i = 0; i < element_num; i++) {
			if (element[i].gst) {
				if (GST_IS_ELEMENT(element[i].gst)) {
					_mmstreamrec_dbg_warn("Still alive element - ID[%d], name [%s], ref count[%d], status[%s]", element[i].id, GST_OBJECT_NAME(element[i].gst), GST_OBJECT_REFCOUNT(element[i].gst), gst_element_state_get_name(GST_STATE(element[i].gst)));
					g_object_weak_unref(G_OBJECT(element[i].gst), (GWeakNotify) _mmstreamrecorder_element_release_noti, sc);
				} else {
					_mmstreamrec_dbg_warn("The element[%d] is still aliving, check it", element[i].id);
				}

				element[i].id = _MMSTREAMRECORDER_ENCODE_NONE;
				element[i].gst = NULL;
			}
		}
	}
	return;
}

int _mmstreamrecorder_create_recorder_pipeline(MMHandleType handle)
{
	int i = 0;
	int err = MM_ERROR_NONE;
	int audio_enable = FALSE;
	GstBus *bus = NULL;
	GstPad *srcpad = NULL;
	GstPad *sinkpad = NULL;

	unsigned int video_codec = MM_VIDEO_CODEC_INVALID;
	unsigned int file_format = MM_FILE_FORMAT_INVALID;
	char *err_name = NULL;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_warn("start");

	err = mm_streamrecorder_get_attributes(handle, &err_name, MMSTR_VIDEO_ENCODER, &video_codec, MMSTR_FILE_FORMAT, &file_format, NULL);
	if (err != MM_ERROR_NONE) {
		_mmstreamrec_dbg_warn("Get attrs fail. (%s:%x)", err_name, err);
		SAFE_FREE(err_name);
		return err;
	}

	err = _mmstreamrecorder_check_videocodec_fileformat_compatibility(video_codec, file_format);
	if (err != MM_ERROR_NONE)
		return err;

	/* Main pipeline */
	_MMSTREAMRECORDER_PIPELINE_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCODE_MAIN_PIPE, "recorder_pipeline", err);

	/* get audio disable */
	mm_streamrecorder_get_attributes(handle, NULL, MMSTR_AUDIO_ENABLE, &audio_enable, NULL);
	sc->audio_enable = audio_enable;

	_mmstreamrec_dbg_log("AUDIO DISABLE : %d", sc->audio_enable);

	if (sc->audio_enable == TRUE) {
		/* create audiosrc bin */
		err = _mmstreamrecorder_create_audiosrc_bin((MMHandleType) hstreamrecorder);
		if (err != MM_ERROR_NONE)
			return err;
	}

	err = _mmstreamrecorder_create_encodesink_bin((MMHandleType) hstreamrecorder, MM_STREAMRECORDER_ENCBIN_PROFILE_VIDEO);
	if (err != MM_ERROR_NONE)
		return err;

	if (sc->audio_enable == TRUE) {
		gst_bin_add(GST_BIN(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst), sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst);
	}

	/* add element and encodesink bin to encode main pipeline */
	gst_bin_add_many(GST_BIN(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst), sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_FILT].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, NULL);

	/* Link each element : appsrc - capsfilter - encodesink bin */
	srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst, "src");
	sinkpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_FILT].gst, "sink");
	_MM_GST_PAD_LINK_UNREF(srcpad, sinkpad, err, pipeline_creation_error);

	srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_FILT].gst, "src");
	sinkpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, "video_sink0");
	_MM_GST_PAD_LINK_UNREF(srcpad, sinkpad, err, pipeline_creation_error);

	if (sc->audio_enable == TRUE) {
		srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, "src");
		sinkpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, "audio_sink0");
		_MM_GST_PAD_LINK_UNREF(srcpad, sinkpad, err, pipeline_creation_error);
	}

	if (sc->audio_enable == TRUE) {
		sinkpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, "sink");
		MMSTREAMRECORDER_ADD_BUFFER_PROBE(sinkpad, _MMSTREAMRECORDER_HANDLER_VIDEOREC, __mmstreamrecorder_audioque_dataprobe, hstreamrecorder);
		gst_object_unref(sinkpad);
		sinkpad = NULL;

		if (sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC_QUE].gst) {
			srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC_QUE].gst, "src");
			MMSTREAMRECORDER_ADD_EVENT_PROBE(srcpad, _MMSTREAMRECORDER_HANDLER_VIDEOREC, __mmstreamrecorder_eventprobe_monitor, hstreamrecorder);
			gst_object_unref(srcpad);
			srcpad = NULL;
		}
	}

	if (sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC_QUE].gst) {
		srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC_QUE].gst, "src");
		MMSTREAMRECORDER_ADD_EVENT_PROBE(srcpad, _MMSTREAMRECORDER_HANDLER_VIDEOREC, __mmstreamrecorder_eventprobe_monitor, hstreamrecorder);
		gst_object_unref(srcpad);
		srcpad = NULL;
	}

	if (sc->audio_enable == FALSE) {
		sinkpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst, "sink");
		MMSTREAMRECORDER_ADD_BUFFER_PROBE(sinkpad, _MMSTREAMRECORDER_HANDLER_VIDEOREC, __mmstreamrecorder_video_dataprobe_audio_disable, hstreamrecorder);
		gst_object_unref(sinkpad);
		sinkpad = NULL;
	}

	if (!strcmp(/*gst_element_rsink_name */"filesink", "filesink")) {
		srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst, "src");
		MMSTREAMRECORDER_ADD_BUFFER_PROBE(srcpad, _MMSTREAMRECORDER_HANDLER_VIDEOREC, __mmstreamrecorder_video_dataprobe_record, hstreamrecorder);
		gst_object_unref(srcpad);
		srcpad = NULL;

		if (sc->audio_enable == TRUE) {
			srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, "src");
			MMSTREAMRECORDER_ADD_BUFFER_PROBE(srcpad, _MMSTREAMRECORDER_HANDLER_VIDEOREC, __mmstreamrecorder_audio_dataprobe_check, hstreamrecorder);
			gst_object_unref(srcpad);
			srcpad = NULL;
		}
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst));

	/* register pipeline message callback */
	hstreamrecorder->encode_pipeline_cb_event_id = gst_bus_add_watch(bus, (GstBusFunc) _mmstreamrecorder_pipeline_cb_message, hstreamrecorder);

	gst_object_unref(bus);
	bus = NULL;

	return MM_ERROR_NONE;

 pipeline_creation_error:
	for (i = _MMSTREAMRECORDER_AUDIOSRC_BIN; i <= _MMSTREAMRECORDER_ENCSINK_SINK; i++) {
		_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, i);
	}
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCODE_MAIN_PIPE);
	return err;
}

int _mmstreamrecorder_destroy_recorder_pipeline(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;

	GstBus *bus = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_log("start");

	if (!sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst) {
		_mmstreamrec_dbg_warn("pipeline is not existed.");
		return MM_ERROR_NONE;
	}

	_mmstreamrecorder_remove_all_handlers((MMHandleType) hstreamrecorder, _MMSTREAMRECORDER_HANDLER_VIDEOREC);

	ret = _mmstreamrecorder_gst_set_state(handle, sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst, GST_STATE_NULL);
	if (ret != MM_ERROR_NONE) {
		_mmstreamrec_dbg_err("Faile to change encode main pipeline [0x%x]", ret);
		return ret;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst));

	/* Remove remained message */
	if (bus) {
		GstMessage *gst_msg = NULL;
		while ((gst_msg = gst_bus_pop(bus)) != NULL) {
			_mmstreamrecorder_pipeline_cb_message(bus, gst_msg, (gpointer) hstreamrecorder);
			gst_message_unref(gst_msg);
			gst_msg = NULL;
		}
		gst_object_unref(bus);
		bus = NULL;
	}

	/* remove audio pipeline first */
	ret = _mmstreamrecorder_destroy_audiosrc_bin(handle);
	if (ret != MM_ERROR_NONE) {
		_mmstreamrec_dbg_err("Fail to remove audio pipeline");
		return ret;
	}

	ret = _mmstreamrecorder_destroy_encodesink_bin(handle);
	if (ret != MM_ERROR_NONE) {
		_mmstreamrec_dbg_err("Fail to remove encoder pipeline");
		return ret;
	}

	/* Remove pipeline message callback */
	if (hstreamrecorder->encode_pipeline_cb_event_id != 0) {
		g_source_remove(hstreamrecorder->encode_pipeline_cb_event_id);
		hstreamrecorder->encode_pipeline_cb_event_id = 0;
	}

	_mmstreamrec_dbg_log("done");

	return ret;
}

int _mmstreamrecorder_create_encodesink_bin(MMHandleType handle, MMStreamRecorderEncodebinProfile profile)
{
	int err = MM_ERROR_NONE;
	int result = 0;
	int channel = 0;
	int audio_enc = 0;
	int video_enc = 0;
	int v_bitrate = 0;
	int a_bitrate = 0;
	int video_width = 0;
	int video_height = 0;
	int video_fps = 0;
	int file_format = 0;
	int audio_src_format = 0;
	int video_src_format = 0;
	int audio_samplerate = 0;
	const char *str_profile = NULL;
	const char *str_aac = NULL;
	const char *str_aar = NULL;
	const char *str_acs = NULL;
	char *err_name = NULL;

	GstCaps *caps = NULL;
	GstPad *pad = NULL;
	GList *element_list = NULL;
	char *temp_filename = NULL;
	int fileformat = 0;
	int size = 0;
	guint imax_size = 0;
	guint imax_time = 0;
	int rec_mode = 0;

	_MMStreamRecorderVideoInfo *info = NULL;
	_MMStreamRecorderFileInfo *finfo = NULL;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	/* type_element *VideoencElement = NULL; */
	/* type_element *AudioencElement = NULL; */
	/* type_element *MuxElement = NULL; */
	/* type_element *RecordsinkElement = NULL; */

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_log("start - profile : %d", profile);

	info = sc->info_video;
	finfo = sc->info_file;

	/* check element availability */
	mm_streamrecorder_get_attributes(handle, &err_name, MMSTR_AUDIO_ENCODER, &audio_enc, MMSTR_AUDIO_CHANNEL, &channel, MMSTR_VIDEO_BITRATE, &v_bitrate, MMSTR_VIDEO_ENCODER, &video_enc, MMSTR_AUDIO_BITRATE, &a_bitrate, MMSTR_VIDEO_RESOLUTION_WIDTH, &video_width, MMSTR_VIDEO_RESOLUTION_HEIGHT, &video_height, MMSTR_VIDEO_FRAMERATE, &video_fps, MMSTR_FILE_FORMAT, &file_format, MMSTR_AUDIO_SAMPLERATE, &audio_samplerate, MMSTR_AUDIO_SOURCE_FORMAT, &audio_src_format, MMSTR_VIDEO_SOURCE_FORMAT, &video_src_format, MMSTR_RECORDER_MODE, &rec_mode, NULL);

	_mmstreamrec_dbg_err("audio encoder - %d , video encoder : %d", audio_enc, video_enc);
	_mmstreamrec_dbg_err("audio channel - %d , video v_bitrate : %d", channel, v_bitrate);
	_mmstreamrec_dbg_err("audio a_bitrate - %d , video video_width : %d ,video video_height : %d ", a_bitrate, video_width, video_height);
	_mmstreamrec_dbg_err("video_fps - %d , video file_format : %d", video_fps, file_format);

	/* Check existence */
	if (sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst) {
		if (((GObject *) sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst)->ref_count > 0)
			gst_object_unref(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst);

		_mmstreamrec_dbg_log("_MMSTREAMRECORDER_ENCSINK_BIN is Already existed.");
	}

	/* Create bin element */
	_MMSTREAMRECORDER_BIN_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCSINK_BIN, "encodesink_bin", err);

	/* Create child element */
	if (hstreamrecorder->ini.encsink_bin_profile != MM_STREAMRECORDER_ENCBIN_PROFILE_AUDIO) {
		/* create appsrc and capsfilter */
		_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCSINK_SRC, hstreamrecorder->ini.name_of_encsink_src, "encodesink_src", element_list, err);
		_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCSINK_FILT, "capsfilter", "encodesink_filter", element_list, err);

		caps = gst_set_videosrcpad_caps(video_src_format, video_width, video_height, video_fps, 1);
		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst, "caps", caps);
		if (caps) {
			gst_caps_unref(caps);
			caps = NULL;
		}

		caps = gst_set_videosrcpad_caps(video_src_format, video_width, video_height, video_fps, 1);
		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_FILT].gst, "caps", caps);
		if (caps) {
			gst_caps_unref(caps);
			caps = NULL;
		}


		/* release element_list, they will be placed out of encodesink bin */
		if (element_list) {
			g_list_free(element_list);
			element_list = NULL;
		}
		if (rec_mode == MM_STREAMRECORDER_MODE_MEDIABUFFER) {
			/* set appsrc as live source */
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst, "is-live", hstreamrecorder->ini.encsink_src_islive);
		}

	}

	_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCSINK_ENCBIN, "encodebin", "encodesink_encbin", element_list, err);

	_mmstreamrec_dbg_log("Profile[%d]", profile);

	/* Set information */
	if (hstreamrecorder->ini.encsink_bin_profile == MM_STREAMRECORDER_ENCBIN_PROFILE_VIDEO) {
		str_profile = "VideoProfile";
		str_aac = "VideoAutoAudioConvert";
		str_aar = "VideoAutoAudioResample";
		str_acs = "VideoAutoColorSpace";
	} else if (hstreamrecorder->ini.encsink_bin_profile == MM_STREAMRECORDER_ENCBIN_PROFILE_AUDIO) {
		str_profile = "AudioProfile";
		str_aac = "AudioAutoAudioConvert";
		str_aar = "AudioAutoAudioResample";
		str_acs = "AudioAutoColorSpace";
	}

	/* TODO : check the last value ( set ) */
	MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "profile", hstreamrecorder->ini.encsink_bin_profile);
	if (rec_mode == MM_STREAMRECORDER_MODE_MEDIABUFFER) {
		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "auto-audio-convert", hstreamrecorder->ini.encsink_bin_auto_audio_convert);
	}
	MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "auto-audio-resample", hstreamrecorder->ini.encsink_bin_auto_audio_resample);
	MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "auto-colorspace", hstreamrecorder->ini.encsink_bin_auto_colorspace);
	MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "use-video-toggle", hstreamrecorder->ini.encsink_bin_use_video_toggle);

	/* Codec */
	if (hstreamrecorder->ini.encsink_bin_profile == MM_STREAMRECORDER_ENCBIN_PROFILE_VIDEO) {
		switch (video_enc) {
		case MM_VIDEO_CODEC_H263:
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "venc-name", hstreamrecorder->ini.h263_video_encoder);
			break;
		case MM_VIDEO_CODEC_H264:
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "venc-name", hstreamrecorder->ini.h264_video_encoder);
			break;
		case MM_VIDEO_CODEC_MPEG4:
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "venc-name", hstreamrecorder->ini.mpeg4_video_encoder);
			break;
		default:
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "venc-name", hstreamrecorder->ini.h264_video_encoder);
			break;
		}
		_MMSTREAMRECORDER_ENCODEBIN_ELMGET(sc, _MMSTREAMRECORDER_ENCSINK_VENC, "video-encode", err);

		/* set color converter size */
		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "vconv-name", hstreamrecorder->ini.name_of_encsink_bin_video_converter);
		_MMSTREAMRECORDER_ENCODEBIN_ELMGET(sc, _MMSTREAMRECORDER_ENCSINK_VCONV, "video-convert", err);

		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "auto-colorspace", hstreamrecorder->ini.encsink_bin_auto_colorspace);

		if (video_src_format == MM_STREAMRECORDER_INPUT_FORMAT_NV12) {
			video_src_format = MM_STREAMRECORDER_INPUT_FORMAT_I420;
		}
		caps = gst_set_videosrcpad_caps(video_src_format, video_width, video_height, video_fps, 1);
		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "vcaps", caps);
		if (video_src_format != MM_STREAMRECORDER_INPUT_FORMAT_NV12) {
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VCONV].gst, "dst-buffer-num", hstreamrecorder->ini.convert_output_buffer_num);
		}
		/* state tuning */
		err = gst_pad_set_caps(gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst, "sink"), caps);
		err = MM_ERROR_NONE;
		/* MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst, "state-tuning", TRUE); */

		if (caps) {
			gst_caps_unref(caps);
			caps = NULL;
		}
		_mmstreamrec_dbg_log("size %dx%d, dst-buffer-num %d", video_width, video_height, hstreamrecorder->ini.convert_output_buffer_num);

		_mmstreamrec_dbg_warn("encoder set caps result : 0x%x", err);

		if (hstreamrecorder->ini.encsink_bin_use_parser[0]) {
			GstElement *parser = gst_element_factory_make(hstreamrecorder->ini.encsink_bin_use_parser, "parse");
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "use-venc-queue", parser);
			_MMSTREAMRECORDER_ENCODEBIN_ELMGET(sc, _MMSTREAMRECORDER_ENCSINK_PARSER, "use-venc-queue", err);
		}
	}

	if (sc->audio_enable == TRUE) {

		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "aenc-name", hstreamrecorder->ini.name_of_encsink_bin_audio_encoder);
		_MMSTREAMRECORDER_ENCODEBIN_ELMGET(sc, _MMSTREAMRECORDER_ENCSINK_AENC, "audio-encode", err);
		_mmstreamrec_dbg_err("audio-encode err = %x ", err);

		/* Set basic infomation */
		if (audio_enc != MM_AUDIO_CODEC_VORBIS) {
			int depth = 0;

			if (audio_src_format == MM_STREAMRECORDER_AUDIO_FORMAT_PCM_S16_LE) {
				depth = 16;
			} else {			/* MM_STREAMRECORDER_AUDIO_FORMAT_PCM_U8 */
				depth = 8;
			}

			/* TODO : set rate , channel , depth */

			caps = gst_set_audiosrcpad_caps(audio_samplerate, 2, depth, 16, 1);
			/* MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "auto-audio-convert", TRUE); */
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "acaps", caps);
			{
				gchar *type = gst_caps_to_string(caps);

				_mmstreamrec_dbg_warn("Set srcpad caps: %s", type);
			}
			gst_caps_unref(caps);
			caps = NULL;
		} else {
			/* what are the audio encoder which should get audio/x-raw-float? */
			caps = gst_caps_new_simple("audio/x-raw", "rate", G_TYPE_INT, audio_samplerate, "channels", G_TYPE_INT, channel, "endianness", G_TYPE_INT, BYTE_ORDER, "width", G_TYPE_INT, 32, NULL);
			_mmstreamrec_dbg_log("caps [x-raw-float, rate:%d, channel:%d, endianness:%d, width:32]", audio_samplerate, channel, BYTE_ORDER);
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "auto-audio-convert", TRUE);
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "acaps", caps);
			gst_caps_unref(caps);
			caps = NULL;
		}

#if 0
		if (audio_enc == MM_AUDIO_CODEC_AMR && channel == 2) {
			caps = gst_caps_new_simple("audio/x-raw-int", "channels", G_TYPE_INT, 1, NULL);
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "auto-audio-convert", TRUE);
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "acaps", caps);
			gst_caps_unref(caps);
			caps = NULL;
		}

		if (audio_enc == MM_AUDIO_CODEC_OGG) {
			caps = gst_caps_new_simple("audio/x-raw-int", NULL);
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "auto-audio-convert", TRUE);
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "acaps", caps);
			gst_caps_unref(caps);
			caps = NULL;
		}
#endif

		_MMSTREAMRECORDER_ENCODEBIN_ELMGET(sc, _MMSTREAMRECORDER_ENCSINK_AENC_QUE, "use-aenc-queue", err);
	}

	/* Mux */
	switch (file_format) {
	case MM_FILE_FORMAT_3GP:{
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "mux-name", hstreamrecorder->ini.name_of_encsink_bin_3GPMUXER);
		}
		break;

	case MM_FILE_FORMAT_MP4:{
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "mux-name", hstreamrecorder->ini.name_of_encsink_bin_MP4MUXER);
		}
		break;

	default:{
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "mux-name", hstreamrecorder->ini.name_of_encsink_bin_MP4MUXER);
		}
		break;

	}
	_MMSTREAMRECORDER_ENCODEBIN_ELMGET(sc, _MMSTREAMRECORDER_ENCSINK_MUX, "mux", err);

	/* Sink */
	/* for recording */
	_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCSINK_SINK, "filesink", NULL, element_list, err);
	MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, "async", 0);

	err = mm_streamrecorder_get_attributes(handle, &err_name,
										   MMSTR_FILE_FORMAT, &fileformat, MMSTR_FILENAME, &temp_filename, &size,
										   MMSTR_TARGET_MAX_SIZE, &imax_size,
										   MMSTR_TARGET_TIME_LIMIT, &imax_time,
										   NULL);

	if (err != MM_ERROR_NONE) {
		_mmstreamrec_dbg_warn("Get attrs fail. (%s:%x)", err_name, err);
		SAFE_FREE(err_name);
		return err;
	}

	finfo->fileformat = fileformat;

	/* set max size */
	if (imax_size <= 0)
		info->max_size = 0;		/* do not check */
	else
		info->max_size = ((guint64) imax_size) << 10;	/* to byte */

	/* set max time */
	if (imax_time <= 0)
		info->max_time = 0;		/* do not check */
	else
		info->max_time = ((guint64) imax_time) * 1000;	/* to millisecond */

	finfo->filename = strdup(temp_filename);
	if (!finfo->filename) {
		_mmstreamrec_dbg_err("strdup was failed");
		return err;
	}

	_mmstreamrec_dbg_log("Record start : set file name using attribute - %s ", finfo->filename);

	MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, "location", finfo->filename);

	if (profile == MM_STREAMRECORDER_ENCBIN_PROFILE_VIDEO) {
		/* video encoder attribute setting */
		if (v_bitrate > 0) {
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst, "bitrate", v_bitrate);
		} else {
			_mmstreamrec_dbg_warn("video bitrate is too small[%d], so skip setting. Use DEFAULT value.", v_bitrate);
		}
	}

	if (sc->audio_enable == TRUE) {
		/* audio encoder attribute setting */
		if (a_bitrate > 0) {
			switch (audio_enc) {
			case MM_AUDIO_CODEC_AMR:
				result = _mmstreamrecorder_get_amrnb_bitrate_mode(a_bitrate);
				_mmstreamrec_dbg_log("Set AMR encoder mode [%d]", result);
				MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, "band-mode", result);
				break;
			case MM_AUDIO_CODEC_AAC:
				_mmstreamrec_dbg_log("Set AAC encoder bitrate [%d]", a_bitrate);
				MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, "bitrate", a_bitrate);
				break;
			default:
				_mmstreamrec_dbg_log("Audio codec is not AMR or AAC... you need to implement setting function for audio encoder bit-rate");
				break;
			}
		} else {
			_mmstreamrec_dbg_warn("Setting bitrate is too small, so skip setting. Use DEFAULT value.");
		}
	}

	_mmstreamrec_dbg_log("Element creation complete");

	/* Add element to bin */
	if (!_mmstreamrecorder_add_elements_to_bin(GST_BIN(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst), element_list)) {
		_mmstreamrec_dbg_err("element add error.");
		err = MM_ERROR_STREAMRECORDER_RESOURCE_CREATION;
		goto pipeline_creation_error;
	}

	_mmstreamrec_dbg_log("Element add complete");

	if (profile == MM_STREAMRECORDER_ENCBIN_PROFILE_VIDEO) {
		pad = gst_element_get_request_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "video");
		if (gst_element_add_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, gst_ghost_pad_new("video_sink0", pad)) < 0) {
			gst_object_unref(pad);
			pad = NULL;
			_mmstreamrec_dbg_err("failed to create ghost video_sink0 on _MMSTREAMRECORDER_ENCSINK_BIN.");
			err = MM_ERROR_STREAMRECORDER_GST_LINK;
			goto pipeline_creation_error;
		}
		gst_object_unref(pad);
		pad = NULL;

		if (sc->audio_enable == TRUE) {
			pad = gst_element_get_request_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "audio");
			if (gst_element_add_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, gst_ghost_pad_new("audio_sink0", pad)) < 0) {
				gst_object_unref(pad);
				pad = NULL;
				_mmstreamrec_dbg_err("failed to create ghost audio_sink0 on _MMSTREAMRECORDER_ENCSINK_BIN.");
				err = MM_ERROR_STREAMRECORDER_GST_LINK;
				goto pipeline_creation_error;
			}
			gst_object_unref(pad);
			pad = NULL;
		}
	} else if (profile == MM_STREAMRECORDER_ENCBIN_PROFILE_AUDIO) {
		pad = gst_element_get_request_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "audio");
		if (gst_element_add_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, gst_ghost_pad_new("audio_sink0", pad)) < 0) {
			gst_object_unref(pad);
			pad = NULL;
			_mmstreamrec_dbg_err("failed to create ghost audio_sink0 on _MMSTREAMRECORDER_ENCSINK_BIN.");
			err = MM_ERROR_STREAMRECORDER_GST_LINK;
			goto pipeline_creation_error;
		}
		gst_object_unref(pad);
		pad = NULL;
	} else {
		/* for stillshot */
		pad = gst_element_get_request_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "image");
		if (gst_element_add_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, gst_ghost_pad_new("image_sink0", pad)) < 0) {
			gst_object_unref(pad);
			pad = NULL;
			_mmstreamrec_dbg_err("failed to create ghost image_sink0 on _MMSTREAMRECORDER_ENCSINK_BIN.");
			err = MM_ERROR_STREAMRECORDER_GST_LINK;
			goto pipeline_creation_error;
		}
		gst_object_unref(pad);
		pad = NULL;
	}

	_mmstreamrec_dbg_log("Get pad complete");

	/* Link internal element */
	if (!_mmstreamrecorder_link_elements(element_list)) {
		_mmstreamrec_dbg_err("element link error.");
		err = MM_ERROR_STREAMRECORDER_GST_LINK;
		goto pipeline_creation_error;
	}

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	_mmstreamrec_dbg_log("done");

	return MM_ERROR_NONE;

 pipeline_creation_error:
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_ENCBIN);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_SRC);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_FILT);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_VENC);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_AENC);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_IENC);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_MUX);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_SINK);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_BIN);

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	return err;
}

int _mmstreamrecorder_destroy_encodesink_bin(MMHandleType handle)
{
	GstPad *reqpad = NULL;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_log("");

	if (sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst != NULL) {
		/* release request pad */
		reqpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "audio");
		if (reqpad) {
			gst_element_release_request_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, reqpad);
			gst_object_unref(reqpad);
			reqpad = NULL;
		}

		reqpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "video");
		if (reqpad) {
			gst_element_release_request_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, reqpad);
			gst_object_unref(reqpad);
			reqpad = NULL;
		}

		/* release encode main pipeline */
		gst_object_unref(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst);

		_mmstreamrec_dbg_log("Encoder pipeline removed");
	}

	return MM_ERROR_NONE;
}

int _mmstreamrecorder_create_audiosrc_bin(MMHandleType handle)
{
	int err = MM_ERROR_NONE;
	int val = 0;
	int rate = 0;
	int format = 0;
	int channel = 0;
	unsigned int a_enc = MM_AUDIO_CODEC_INVALID;
	unsigned int file_format = MM_FILE_FORMAT_INVALID;
	char *err_name = NULL;
	int rec_mode = 0;

	GstCaps *caps = NULL;
	GstPad *pad = NULL;
	GList *element_list = NULL;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderGstElement *last_element = NULL;
	/* type_element *AudiosrcElement = NULL; */

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_log("");

	err = mm_streamrecorder_get_attributes(handle, &err_name, MMSTR_AUDIO_ENCODER, &a_enc, MMSTR_AUDIO_BITRATE, &val, MMSTR_AUDIO_SAMPLERATE, &rate, MMSTR_AUDIO_SOURCE_FORMAT, &format, MMSTR_AUDIO_CHANNEL, &channel, MMSTR_FILE_FORMAT, &file_format, MMSTR_RECORDER_MODE, &rec_mode, NULL);

	if (err != MM_ERROR_NONE) {
		_mmstreamrec_dbg_warn("Get attrs fail. (%s:%x)", err_name, err);
		SAFE_FREE(err_name);
		return err;
	}

	err = _mmstreamrecorder_check_audiocodec_fileformat_compatibility(a_enc, file_format);
	if (err != MM_ERROR_NONE) {
		_mmstreamrec_dbg_err("error name :%s , audio format %d , fileformat %d. error : %x)", err_name, a_enc, file_format, err);
		SAFE_FREE(err_name);
		return err;
	}

	/* Check existence */
	if (sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst) {
		if (((GObject *) sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst)->ref_count > 0)
			gst_object_unref(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst);

		_mmstreamrec_dbg_log("_MMSTREAMRECORDER_AUDIOSRC_BIN is Already existed. Unref once...");
	}

	/* Create bin element */
	_MMSTREAMRECORDER_BIN_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_AUDIOSRC_BIN, "audiosource_bin", err);

	_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_AUDIOSRC_SRC, "appsrc", hstreamrecorder->ini.name_of_audio_src, element_list, err);

	_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_AUDIOSRC_FILT, "capsfilter", "audiosrc_capsfilter", element_list, err);

	/* Set basic infomation */
	if (a_enc != MM_AUDIO_CODEC_VORBIS) {
		int depth = 0;

		if (format == MM_STREAMRECORDER_AUDIO_FORMAT_PCM_S16_LE) {
			depth = 16;
		} else {				/* MM_STREAMRECORDER_AUDIO_FORMAT_PCM_U8 */
			depth = 8;
		}

		caps = gst_set_audiosrcpad_caps(rate, channel, depth, 16, 1);

		_mmstreamrec_dbg_log("caps [x-raw-int, rate:%d, channel:%d, depth:%d]", rate, channel, depth);
	} else {
		/* what are the audio encoder which should get audio/x-raw-float? */
		caps = gst_caps_new_simple("audio/x-raw-float", "rate", G_TYPE_INT, rate, "channels", G_TYPE_INT, channel, "endianness", G_TYPE_INT, BYTE_ORDER, "width", G_TYPE_INT, 32, NULL);
		_mmstreamrec_dbg_log("caps [x-raw-float, rate:%d, channel:%d, endianness:%d, width:32]", rate, channel, BYTE_ORDER);
	}

	if (caps) {
		if (rec_mode == MM_STREAMRECORDER_MODE_MEDIABUFFER) {
			MMSTREAMRECORDER_G_OBJECT_SET((sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst), "caps", caps);
		}
		MMSTREAMRECORDER_G_OBJECT_SET((sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_FILT].gst), "caps", caps);
		{
			gchar *type = gst_caps_to_string(caps);

			_mmstreamrec_dbg_err("_MMSTREAMRECORDER_AUDIOSRC_FILT %s", type);

		}
		gst_caps_unref(caps);
		caps = NULL;
	} else {
		_mmstreamrec_dbg_err("create caps error");
		err = MM_ERROR_STREAMRECORDER_RESOURCE_CREATION;
		goto pipeline_creation_error;
	}

	if (rec_mode == MM_STREAMRECORDER_MODE_SCREENRECORD) {
#if 1							/* mic mode */
		MMSTREAMRECORDER_G_OBJECT_SET((sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst), "do-timestamp", TRUE);
#else							/* speaker mode with alsasrc */
		MMSTREAMRECORDER_G_OBJECT_SET((sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst), "device", "hw:0,8");
		MMSTREAMRECORDER_G_OBJECT_SET((sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst), "latency-time", 256000);
		MMSTREAMRECORDER_G_OBJECT_SET((sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst), "buffer-time", 10000);
		MMSTREAMRECORDER_G_OBJECT_SET((sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst), "do-timestamp", FALSE);
#endif
	} else {
		MMSTREAMRECORDER_G_OBJECT_SET((sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst), "do-timestamp", FALSE);
	}

	if (!_mmstreamrecorder_add_elements_to_bin(GST_BIN(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst), element_list)) {
		_mmstreamrec_dbg_err("element add error.");
		err = MM_ERROR_STREAMRECORDER_RESOURCE_CREATION;
		goto pipeline_creation_error;
	}

	if (!_mmstreamrecorder_link_elements(element_list)) {
		_mmstreamrec_dbg_err("element link error.");
		err = MM_ERROR_STREAMRECORDER_GST_LINK;
		goto pipeline_creation_error;
	}

	last_element = (_MMStreamRecorderGstElement *) (g_list_last(element_list)->data);
	pad = gst_element_get_static_pad(last_element->gst, "src");
	if ((gst_element_add_pad(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, gst_ghost_pad_new("src", pad))) < 0) {
		gst_object_unref(pad);
		pad = NULL;
		_mmstreamrec_dbg_err("failed to create ghost pad on _MMSTREAMRECORDER_AUDIOSRC_BIN.");
		err = MM_ERROR_STREAMRECORDER_GST_LINK;
		goto pipeline_creation_error;
	}

	gst_object_unref(pad);
	pad = NULL;

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	return MM_ERROR_NONE;

 pipeline_creation_error:
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_AUDIOSRC_SRC);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_AUDIOSRC_FILT);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_AUDIOSRC_BIN);

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	return err;
}

int _mmstreamrecorder_destroy_audiosrc_bin(MMHandleType handle)
{
	GstPad *srcpad = NULL;
	GstPad *sinkpad = NULL;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	_mmstreamrec_dbg_log("");

	if (sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst != NULL) {
		srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, "src");
		sinkpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, "audio_sink0");
		_MM_GST_PAD_UNLINK_UNREF(srcpad, sinkpad);

		/* release audiosrc bin */
		gst_bin_remove(GST_BIN(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst), sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst);

		_mmstreamrecorder_remove_element_handle(handle, (void *)sc->encode_element, _MMSTREAMRECORDER_AUDIOSRC_BIN, _MMSTREAMRECORDER_AUDIOSRC_FILT);

		_mmstreamrec_dbg_log("Audio pipeline removed");
	}

	return MM_ERROR_NONE;
}

/* COMMAND - VIDEO */
int _mmstreamrecorder_video_command(MMHandleType handle, int command)
{
	int ret = MM_ERROR_NONE;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderVideoInfo *info = NULL;
	_MMStreamRecorderAudioInfo *info_audio = NULL;
	_MMStreamRecorderFileInfo *finfo = NULL;
	_MMStreamRecorderSubContext *sc = NULL;
	GstElement *pipeline = NULL;
	GstPad *pad = NULL;
	guint count = 0;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc && sc->encode_element, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	mmf_return_val_if_fail(sc->info_video, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	if (sc->audio_enable == TRUE) {
		mmf_return_val_if_fail(sc->info_audio, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	}
	mmf_return_val_if_fail(sc->info_file, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	info = sc->info_video;
	if (sc->audio_enable == TRUE)
		info_audio = sc->info_audio;

	finfo = sc->info_file;

	_mmstreamrec_dbg_log("command %d", command);

	pipeline = sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst;

	switch (command) {
	case _MM_STREAMRECORDER_CMD_RECORD:
		{

			/* Recording */
			_mmstreamrec_dbg_log("Record Start");
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "block", FALSE);

			/* Adjust display FPS */
			info->video_frame_count = 0;
			if (info_audio)
				info_audio->audio_frame_count = 0;

			info->filesize = 0;
			sc->ferror_send = FALSE;
			sc->ferror_count = 0;
			sc->error_occurs = FALSE;
			sc->bget_eos = FALSE;

			ret = _mmstreamrecorder_gst_set_state(handle, sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst, GST_STATE_PLAYING);
			if (ret != MM_ERROR_NONE) {
				/* Remove recorder pipeline and recording file which size maybe zero */
				ret = _mmstreamrecorder_destroy_recorder_pipeline(handle);

				if (finfo->filename) {
					_mmstreamrec_dbg_log("file delete(%s)", finfo->filename);
					unlink(finfo->filename);
					g_free(finfo->filename);
					finfo->filename = NULL;
				}
				goto _ERR_STREAMRECORDER_VIDEO_COMMAND;
			}

		}
		break;
	case _MM_STREAMRECORDER_CMD_PAUSE:
		{

			if (info->b_commiting) {
				_mmstreamrec_dbg_warn("now on commiting previous file!!(command : %d)", command);
				return MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
			}

			for (count = 0; count <= hstreamrecorder->ini.retrial_count; count++) {
				if (sc->audio_enable == FALSE) {
					/* check only video frame */
					if (info->video_frame_count >= hstreamrecorder->ini.minimum_frame) {
						break;
					} else if (count == hstreamrecorder->ini.retrial_count) {
						_mmstreamrec_dbg_err("Pause fail, frame count %" G_GUINT64_FORMAT "", info->video_frame_count);
						return MM_ERROR_STREAMRECORDER_INVALID_CONDITION;
					} else {
						_mmstreamrec_dbg_warn("Waiting for enough video frame, retrial[%d], frame %" G_GUINT64_FORMAT "", count, info->video_frame_count);
					}

					usleep(hstreamrecorder->ini.video_frame_wait_time);
				} else {
					/* check both of video and audio frame */
					if (info->video_frame_count >= hstreamrecorder->ini.minimum_frame && info_audio->audio_frame_count) {
						break;
					} else if (count == hstreamrecorder->ini.retrial_count) {
						_mmstreamrec_dbg_err("Pause fail, frame count VIDEO[%" G_GUINT64_FORMAT "], AUDIO [%" G_GUINT64_FORMAT "]", info->video_frame_count, info_audio->audio_frame_count);
						return MM_ERROR_STREAMRECORDER_INVALID_CONDITION;
					} else {
						_mmstreamrec_dbg_warn("Waiting for enough frames, retrial [%d], VIDEO[%" G_GUINT64_FORMAT "], AUDIO [%" G_GUINT64_FORMAT "]", count, info->video_frame_count, info_audio->audio_frame_count);
					}

					usleep(hstreamrecorder->ini.video_frame_wait_time);
				}
			}
			/* tee block */
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "runtime-pause", TRUE);

			break;
		}
		break;
	case _MM_STREAMRECORDER_CMD_CANCEL:
		{
			if (info->b_commiting) {
				_mmstreamrec_dbg_warn("now on commiting previous file!!(command : %d)", command);
				return MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
			}

			ret = _mmstreamrecorder_destroy_recorder_pipeline(handle);
			if (ret != MM_ERROR_NONE)
				goto _ERR_STREAMRECORDER_VIDEO_COMMAND;

			/* remove target file */
			if (finfo->filename) {
				_mmstreamrec_dbg_log("file delete(%s)", finfo->filename);
				unlink(finfo->filename);
				g_free(finfo->filename);
				finfo->filename = NULL;
			}

			sc->isMaxsizePausing = FALSE;
			sc->isMaxtimePausing = FALSE;

			info->video_frame_count = 0;
			if (info_audio)
				info_audio->audio_frame_count = 0;

			info->filesize = 0;
			break;
		}
		break;
	case _MM_STREAMRECORDER_CMD_COMMIT:
		/* video recording command */
		{

			if (info->b_commiting) {
				_mmstreamrec_dbg_err("now on commiting previous file!!(command : %d)", command);
				return MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
			} else {
				_mmstreamrec_dbg_log("_MM_STREAMRECORDER_CMD_COMMIT : start");
				info->b_commiting = TRUE;
			}

			for (count = 0; count <= hstreamrecorder->ini.retrial_count; count++) {
				if (sc->audio_enable == FALSE) {
					/* check only video frame */
					if (info->video_frame_count >= hstreamrecorder->ini.minimum_frame) {
						break;
					} else if (count == hstreamrecorder->ini.retrial_count) {
						_mmstreamrec_dbg_err("Commit fail, frame count is %" G_GUINT64_FORMAT "", info->video_frame_count);
						info->b_commiting = FALSE;
						return MM_ERROR_STREAMRECORDER_INVALID_CONDITION;
					} else {
						_mmstreamrec_dbg_warn("Waiting for enough video frame, retrial [%d], frame %" G_GUINT64_FORMAT "", count, info->video_frame_count);
					}

					usleep(hstreamrecorder->ini.video_frame_wait_time);
				} else {
					/* check both of video and audio frame */
					if (info->video_frame_count >= hstreamrecorder->ini.minimum_frame && info_audio->audio_frame_count) {
						break;
					} else if (count == hstreamrecorder->ini.retrial_count) {
						_mmstreamrec_dbg_err("Commit fail, VIDEO[%" G_GUINT64_FORMAT "], AUDIO [%" G_GUINT64_FORMAT "]", info->video_frame_count, info_audio->audio_frame_count);

						info->b_commiting = FALSE;
						return MM_ERROR_STREAMRECORDER_INVALID_CONDITION;
					} else {
						_mmstreamrec_dbg_warn("Waiting for enough frames, retrial [%d], VIDEO[%" G_GUINT64_FORMAT "], AUDIO [%" G_GUINT64_FORMAT "]", count, info->video_frame_count, info_audio->audio_frame_count);
					}

					usleep(hstreamrecorder->ini.video_frame_wait_time);
				}
			}

			if (sc->error_occurs) {
				GstPad *video = NULL;
				GstPad *audio = NULL;

				_mmstreamrec_dbg_err("Committing Error case");
#if 0
				video = gst_element_get_static_pad(sc->element[_MMSTREAMRECORDER_VIDEOSINK_SINK].gst, "sink");
				ret = gst_pad_send_event(video, gst_event_new_eos());
				_mmstreamrec_dbg_err("Sending EOS video sink  : %d", ret);
				gst_object_unref(video);
#endif
				video = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst, "src");
				gst_pad_push_event(video, gst_event_new_flush_start());
				gst_pad_push_event(video, gst_event_new_flush_stop(TRUE));
				ret = gst_pad_push_event(video, gst_event_new_eos());
				_mmstreamrec_dbg_err("Sending EOS video encoder src pad  : %d", ret);
				gst_object_unref(video);

				if (sc->audio_enable == TRUE) {
					audio = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, "src");
					gst_pad_push_event(audio, gst_event_new_flush_start());
					gst_pad_push_event(audio, gst_event_new_flush_stop(TRUE));
					ret = gst_element_send_event(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst, gst_event_new_eos());
					_mmstreamrec_dbg_err("Sending EOS audio encoder src pad  : %d", ret);
					gst_object_unref(audio);
				}
			} else {
				if (sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst != NULL) {
					ret = gst_element_send_event(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst, gst_event_new_eos());
					_mmstreamrec_dbg_warn("send eos to appsrc result : %d", ret);
				}

				if (sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst != NULL) {
					pad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst, "src");
					ret = gst_element_send_event(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst, gst_event_new_eos());
					gst_object_unref(pad);
					pad = NULL;

					_mmstreamrec_dbg_warn("send eos to audiosrc result : %d", ret);
				}
			}

			/* Wait EOS */
			_mmstreamrec_dbg_log("Start to wait EOS");
			ret = _mmstreamrecorder_get_eos_message(handle);
			if (ret != MM_ERROR_NONE) {
				info->b_commiting = FALSE;
				goto _ERR_STREAMRECORDER_VIDEO_COMMAND;
			}
		}
		break;
	default:
		ret = MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT;
		break;
	}
	return MM_ERROR_NONE;

 _ERR_STREAMRECORDER_VIDEO_COMMAND:
	if (ret != MM_ERROR_NONE)
		_mmstreamrec_dbg_err("Current Videosrc status");

	return ret;
}

int _mmstreamrecorder_video_handle_eos(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderVideoInfo *info = NULL;
	_MMStreamRecorderAudioInfo *info_audio = NULL;
	_MMStreamRecorderFileInfo *finfo = NULL;
	_MMStreamRecorderMsgItem msg;
	MMStreamRecordingReport *report = NULL;

	mmf_return_val_if_fail(hstreamrecorder, FALSE);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);
	mmf_return_val_if_fail(sc->info_video, FALSE);
	if (sc->audio_enable == TRUE)
		mmf_return_val_if_fail(sc->info_audio, FALSE);

	mmf_return_val_if_fail(sc->info_file, FALSE);

	info = sc->info_video;
	if (sc->audio_enable == TRUE)
		info_audio = sc->info_audio;

	finfo = sc->info_file;

	_mmstreamrec_dbg_err("");

	/* remove blocking part */
	MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "block", FALSE);

	ret = _mmstreamrecorder_destroy_recorder_pipeline(handle);
	if (ret != MM_ERROR_NONE)
		_mmstreamrec_dbg_warn("_mmstreamrecorder_destroy_recorder_pipeline failed. error[%x]", ret);

	/* Send recording report to application */
	msg.id = MM_MESSAGE_STREAMRECORDER_VIDEO_CAPTURED;
	report = (MMStreamRecordingReport *) malloc(sizeof(MMStreamRecordingReport));
	if (!report) {
		_mmstreamrec_dbg_err("Recording report fail(%s). Out of memory.", finfo->filename);
	} else {
		report->recording_filename = strdup(finfo->filename);
		msg.param.data = report;
		msg.param.code = 1;
		_mmstreamrecorder_send_message((MMHandleType) hstreamrecorder, &msg);
	}

	/* Finishing */
	sc->pipeline_time = 0;
	sc->pause_time = 0;
	sc->isMaxsizePausing = FALSE;	/*In async function, this variable should set in callback function. */
	sc->isMaxtimePausing = FALSE;
	sc->error_occurs = FALSE;

	info->video_frame_count = 0;
	if (info_audio)
		info_audio->audio_frame_count = 0;

	info->filesize = 0;
	g_free(finfo->filename);
	finfo->filename = NULL;
	info->b_commiting = FALSE;

	_mmstreamrec_dbg_err("_mmstreamrecorder_video_handle_eos : end");

	return TRUE;
}

/* AUDIO */

int _mmstreamrecorder_create_audio_pipeline(MMHandleType handle)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);

	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	return _mmstreamrecorder_create_audiop_with_encodebin(handle);
}

/**
 * This function destroy audio pipeline.
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @return	void
 * @remarks
 * @see		_mmstreamrecorder_destroy_audio_pipeline()
 *
 */
void _mmstreamrecorder_destroy_audio_pipeline(MMHandleType handle)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;
	mmf_return_if_fail(hstreamrecorder);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);

	mmf_return_if_fail(sc && sc->info_audio);

	info = sc->info_audio;

	_mmstreamrec_dbg_log("start");

	if (sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst) {
		_mmstreamrec_dbg_warn("release audio pipeline");

		_mmstreamrecorder_gst_set_state(handle, sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst, GST_STATE_NULL);

		_mmstreamrecorder_remove_all_handlers((MMHandleType) hstreamrecorder, _MMSTREAMRECORDER_HANDLER_CATEGORY_ALL);

		if (info->bMuxing) {
			GstPad *reqpad = NULL;
			/* FIXME:
			   Release request pad
			   The ref_count of mux is always # of streams in here, i don't know why it happens.
			   So, i unref the mux manually
			 */
			reqpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "audio");
			gst_element_release_request_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, reqpad);
			gst_object_unref(reqpad);

			if (GST_IS_ELEMENT(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_MUX].gst) && GST_OBJECT_REFCOUNT(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_MUX].gst) > 1) {
				gst_object_unref(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_MUX].gst);
			}
		}
		gst_object_unref(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst);
	}

	_mmstreamrec_dbg_log("done");

	return;
}

int _mmstreamrecorder_create_audiop_with_encodebin(MMHandleType handle)
{
	int err = MM_ERROR_NONE;
	char *aenc_name = NULL;
	char *mux_name = NULL;
	char *err_name = NULL;
	int rec_mode = 0;

	GstBus *bus = NULL;
	GstPad *srcpad = NULL;
	GstPad *sinkpad = NULL;
	GList *element_list = NULL;

	_MMStreamRecorderAudioInfo *info = NULL;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	/* type_element *aenc_elem = NULL; */
	/* type_element *mux_elem = NULL; */

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);

	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->info_audio, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	info = (_MMStreamRecorderAudioInfo *) sc->info_audio;

	_mmstreamrec_dbg_log("");

	err = mm_streamrecorder_get_attributes(handle, &err_name, MMSTR_RECORDER_MODE, &rec_mode, NULL);

	if (!mux_name || !strcmp(mux_name, "wavenc")) {
		/* IF MUX in not chosen then record in raw file */
		_mmstreamrec_dbg_log("Record without muxing.");
		info->bMuxing = FALSE;
	} else {
		_mmstreamrec_dbg_log("Record with mux.");
		info->bMuxing = TRUE;
	}

	/* Create GStreamer pipeline */
	_MMSTREAMRECORDER_PIPELINE_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCODE_MAIN_PIPE, "recorder_pipeline", err);

	err = _mmstreamrecorder_create_audiosrc_bin(handle);
	if (err != MM_ERROR_NONE)
		return err;

	if (info->bMuxing) {
		/* Muxing. can use encodebin. */
		err = _mmstreamrecorder_create_encodesink_bin((MMHandleType) hstreamrecorder, MM_STREAMRECORDER_ENCBIN_PROFILE_AUDIO);
		if (err != MM_ERROR_NONE)
			return err;

	} else {
		/* without muxing. can't use encodebin. */

		_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCSINK_AQUE, "queue", NULL, element_list, err);

		if (rec_mode == MM_STREAMRECORDER_MODE_MEDIABUFFER) {
			if (strcmp(hstreamrecorder->ini.name_of_encsink_bin_audio_encoder, "wavenc") != 0) {
				_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCSINK_CONV, "audioconvert", NULL, element_list, err);
			}
		}

		_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCSINK_AENC, aenc_name, NULL, element_list, err);

		_MMSTREAMRECORDER_ELEMENT_MAKE(sc, sc->encode_element, _MMSTREAMRECORDER_ENCSINK_SINK, hstreamrecorder->ini.name_of_encsink_sink, NULL, element_list, err);
	}

	/* Add and link elements */
	if (info->bMuxing) {
		/* IF MUX is indicated create MUX */
		gst_bin_add_many(GST_BIN(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst), sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, NULL);

		srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, "src");
		sinkpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_BIN].gst, "audio_sink0");
		_MM_GST_PAD_LINK_UNREF(srcpad, sinkpad, err, pipeline_creation_error);
	} else {
		/* IF MUX in not chosen then record in raw amr file */
		if (!strcmp(aenc_name, "wavenc")) {
			gst_bin_add_many(GST_BIN(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst), sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, NULL);

			if (!_MM_GST_ELEMENT_LINK_MANY(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, NULL)) {
				err = MM_ERROR_STREAMRECORDER_GST_LINK;
				goto pipeline_creation_error;
			}
		} else {
			if (rec_mode == MM_STREAMRECORDER_MODE_MEDIABUFFER) {
				gst_bin_add_many(GST_BIN(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst), sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_CONV].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, NULL);

				if (!_MM_GST_ELEMENT_LINK_MANY(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_CONV].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, NULL)) {
					err = MM_ERROR_STREAMRECORDER_GST_LINK;
					goto pipeline_creation_error;
				}
			} else {
				gst_bin_add_many(GST_BIN(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst), sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, NULL);

				if (!_MM_GST_ELEMENT_LINK_MANY(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_BIN].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, NULL)) {
					err = MM_ERROR_STREAMRECORDER_GST_LINK;
					goto pipeline_creation_error;
				}
			}
		}
	}

	if (info->bMuxing) {
		MMSTREAMRECORDER_SIGNAL_CONNECT(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_MUX].gst, _MMSTREAMRECORDER_HANDLER_AUDIOREC, "pad-added", __mmstreamrecorder_audiorec_pad_added_cb, hstreamrecorder);
	} else {
		srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AENC].gst, "src");
		MMSTREAMRECORDER_ADD_BUFFER_PROBE(srcpad, _MMSTREAMRECORDER_HANDLER_AUDIOREC, __mmstreamrecorder_audio_dataprobe_record, hstreamrecorder);
		gst_object_unref(srcpad);
		srcpad = NULL;
	}

	bus = gst_pipeline_get_bus(GST_PIPELINE(sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst));

	/* register message callback  */
	hstreamrecorder->pipeline_cb_event_id = gst_bus_add_watch(bus, (GstBusFunc) _mmstreamrecorder_pipeline_cb_message, hstreamrecorder);

	/* set sync callback */
	gst_bus_set_sync_handler(bus, gst_bus_sync_signal_handler, hstreamrecorder, NULL);

	gst_object_unref(bus);
	bus = NULL;

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	return MM_ERROR_NONE;

 pipeline_creation_error:
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCODE_MAIN_PIPE);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_AUDIOSRC_BIN);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_AQUE);
	if (rec_mode == MM_STREAMRECORDER_MODE_MEDIABUFFER) {
		_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_CONV);
	}
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_AENC);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_SINK);
	_MMSTREAMRECORDER_ELEMENT_REMOVE(sc->encode_element, _MMSTREAMRECORDER_ENCSINK_BIN);

	if (element_list) {
		g_list_free(element_list);
		element_list = NULL;
	}

	return err;
}

int _mmstreamrecorder_audio_command(MMHandleType handle, int command)
{
	int cmd = command;
	int ret = MM_ERROR_NONE;
	int err = 0;
	guint64 free_space = 0;
	guint64 cal_space = 0;
	char *dir_name = NULL;
	char *err_attr_name = NULL;
	guint count = 0;
	int size = 0;

	GstElement *pipeline = NULL;
	GstElement *audioSrc = NULL;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;
	_MMStreamRecorderFileInfo *finfo = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);

	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->info_audio, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->info_file, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	pipeline = sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst;
	info = sc->info_audio;
	finfo = sc->info_file;

	_mmstreamrec_dbg_log("");

	pipeline = sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst;
	audioSrc = sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst;
	switch (cmd) {
	case _MM_STREAMRECORDER_CMD_RECORD:
		/* check status for resume case */
		{
			guint imax_size = 0;
			guint imax_time = 0;
			char *temp_filename = NULL;

			if (sc->pipeline_time)
				gst_element_set_start_time((GstElement *) GST_PIPELINE(pipeline), sc->pipeline_time);

			sc->pipeline_time = hstreamrecorder->ini.reset_pause_time;

			ret = mm_streamrecorder_get_attributes(handle, &err_attr_name, MMSTR_TARGET_MAX_SIZE, &imax_size, MMSTR_TARGET_TIME_LIMIT, &imax_time, MMSTR_FILE_FORMAT, &(finfo->fileformat), MMSTR_FILENAME, &temp_filename, &size, NULL);
			if (ret != MM_ERROR_NONE) {
				_mmstreamrec_dbg_warn("failed to get attribute. (%s:%x)", err_attr_name, ret);
				SAFE_FREE(err_attr_name);
				goto _ERR_STREAMRECORDER_AUDIO_COMMAND;
			}

			finfo->filename = strdup(temp_filename);
			if (!finfo->filename) {
				_mmstreamrec_dbg_err("STRDUP was failed");
				goto _ERR_STREAMRECORDER_AUDIO_COMMAND;
			}

			_mmstreamrec_dbg_log("Record start : set file name using attribute - %s\n ", finfo->filename);

			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, "location", finfo->filename);

			sc->ferror_send = FALSE;
			sc->ferror_count = 0;
			sc->bget_eos = FALSE;
			info->filesize = 0;

			/* set max size */
			if (imax_size <= 0)
				info->max_size = 0;	/* do not check */
			else
				info->max_size = ((guint64) imax_size) << 10;	/* to byte */

			/* set max time */
			if (imax_time <= 0)
				info->max_time = 0;	/* do not check */
			else
				info->max_time = ((guint64) imax_time) * 1000;	/* to millisecond */

			/* TODO : check free space before recording start, need to more discussion */
			dir_name = g_path_get_dirname(finfo->filename);
			err = _mmstreamrecorder_get_freespace(dir_name, &free_space);

			_mmstreamrec_dbg_warn("current space for recording - %s : [%" G_GUINT64_FORMAT "]", dir_name, free_space);

			if (dir_name) {
				g_free(dir_name);
				dir_name = NULL;
			}
			cal_space = (guint64)(hstreamrecorder->ini.audio_frame_minimum_space);
			cal_space = cal_space + (5 * 1024);
			if ((err == -1) || free_space <= cal_space) {
				_mmstreamrec_dbg_err("No more space for recording");
				return MM_MESSAGE_STREAMRECORDER_NO_FREE_SPACE;
			}
		}

		ret = _mmstreamrecorder_gst_set_state(handle, pipeline, GST_STATE_PLAYING);
		if (ret != MM_ERROR_NONE)
			goto _ERR_STREAMRECORDER_AUDIO_COMMAND;

		break;

	case _MM_STREAMRECORDER_CMD_PAUSE:
		{
			GstClock *l_clock = NULL;

			if (info->b_commiting) {
				_mmstreamrec_dbg_warn("now on commiting previous file!!(cmd : %d)", cmd);
				return MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
			}

			for (count = 0; count <= hstreamrecorder->ini.retrial_count; count++) {
				if (info->filesize > 0) {
					break;
				} else if (count == hstreamrecorder->ini.retrial_count) {
					_mmstreamrec_dbg_err("Pause fail, wait 200 ms, but file size is %lld", info->filesize);
					return MM_ERROR_STREAMRECORDER_INVALID_CONDITION;
				} else {
					_mmstreamrec_dbg_warn("Wait for enough audio frame, retry count[%d], file size is %lld", count, info->filesize);
				}
				usleep(hstreamrecorder->ini.audio_frame_wait_time);
			}

			ret = _mmstreamrecorder_gst_set_state(handle, pipeline, GST_STATE_PAUSED);
			if (ret != MM_ERROR_NONE)
				goto _ERR_STREAMRECORDER_AUDIO_COMMAND;

			/* FIXME: consider delay. */
			l_clock = gst_pipeline_get_clock(GST_PIPELINE(pipeline));
			sc->pipeline_time = gst_clock_get_time(l_clock) - gst_element_get_base_time(GST_ELEMENT(pipeline));
			break;
		}

	case _MM_STREAMRECORDER_CMD_CANCEL:
		if (info->b_commiting) {
			_mmstreamrec_dbg_warn("now on commiting previous file!!(cmd : %d)", cmd);
			return MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
		}

		ret = _mmstreamrecorder_gst_set_state(handle, pipeline, GST_STATE_READY);
		if (ret != MM_ERROR_NONE)
			goto _ERR_STREAMRECORDER_AUDIO_COMMAND;

		if (info->bMuxing) {
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "block", FALSE);
		} else {
			MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, "empty-buffers", FALSE);
		}

		_mmstreamrecorder_gst_set_state(handle, sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SINK].gst, GST_STATE_NULL);

		sc->pipeline_time = 0;
		sc->pause_time = 0;
		sc->isMaxsizePausing = FALSE;
		sc->isMaxtimePausing = FALSE;

		if (finfo->filename) {
			_mmstreamrec_dbg_log("file delete(%s)", finfo->filename);
			unlink(finfo->filename);
			g_free(finfo->filename);
			finfo->filename = NULL;
		}
		break;

	case _MM_STREAMRECORDER_CMD_COMMIT:
		{

			_mmstreamrec_dbg_log("_MM_STREAMRECORDER_CMD_COMMIT");

			if (info->b_commiting) {
				_mmstreamrec_dbg_warn("now on commiting previous file!!(cmd : %d)", cmd);
				return MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
			} else {
				_mmstreamrec_dbg_log("_MM_STREAMRECORDER_CMD_COMMIT : start");
				info->b_commiting = TRUE;
			}

			for (count = 0; count <= hstreamrecorder->ini.retrial_count; count++) {
				if (info->filesize > 0) {
					break;
				} else if (count == hstreamrecorder->ini.retrial_count) {
					_mmstreamrec_dbg_err("Commit fail, waited 200 ms, but file size is %lld", info->filesize);
					info->b_commiting = FALSE;
					return MM_ERROR_STREAMRECORDER_INVALID_CONDITION;
				} else {
					_mmstreamrec_dbg_warn("Waiting for enough audio frame, re-count[%d], file size is %lld", count, info->filesize);
				}
				usleep(hstreamrecorder->ini.audio_frame_wait_time);
			}

			if (audioSrc) {
				GstPad *pad = gst_element_get_static_pad(audioSrc, "src");
				ret = gst_element_send_event(audioSrc, gst_event_new_eos());
				gst_object_unref(pad);
				pad = NULL;
				/* for pause -> commit case */
				/*if (_mmstreamrecorder_get_state((MMHandleType)hstreamrecorder) == MM_STREAMRECORDER_STATE_PAUSED) {
				   ret = _mmstreamrecorder_gst_set_state(handle, pipeline, GST_STATE_PLAYING);
				   if (ret != MM_ERROR_NONE) {
				   goto _ERR_STREAMRECORDER_AUDIO_COMMAND;
				   }
				   } */
			}

			/* wait until finishing EOS */
			_mmstreamrec_dbg_log("Start to wait EOS");
			if ((ret = _mmstreamrecorder_get_eos_message(handle)) != MM_ERROR_NONE)
				goto _ERR_STREAMRECORDER_AUDIO_COMMAND;

			break;
		}
	default:
		ret = MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT;
		break;
	}

 _ERR_STREAMRECORDER_AUDIO_COMMAND:
	return ret;
}

int _mmstreamrecorder_audio_handle_eos(MMHandleType handle)
{
	int err = MM_ERROR_NONE;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;
	_MMStreamRecorderFileInfo *finfo = NULL;
	GstElement *pipeline = NULL;
	_MMStreamRecorderMsgItem msg;
	MMStreamRecordingReport *report;

	mmf_return_val_if_fail(hstreamrecorder, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);

	mmf_return_val_if_fail(sc, FALSE);
	mmf_return_val_if_fail(sc->info_audio, FALSE);
	mmf_return_val_if_fail(sc->info_file, FALSE);

	_mmstreamrec_dbg_err("");

	info = sc->info_audio;
	finfo = sc->info_file;

	pipeline = sc->encode_element[_MMSTREAMRECORDER_ENCODE_MAIN_PIPE].gst;

	err = _mmstreamrecorder_gst_set_state(handle, pipeline, GST_STATE_READY);

	if (err != MM_ERROR_NONE)
		_mmstreamrec_dbg_warn("Failed:_MM_STREAMRECORDER_CMD_COMMIT:GST_STATE_READY. err[%x]", err);

	/* Send recording report message to application */
	msg.id = MM_MESSAGE_STREAMRECORDER_AUDIO_CAPTURED;
	report = (MMStreamRecordingReport *) malloc(sizeof(MMStreamRecordingReport));
	if (!report) {
		_mmstreamrec_dbg_err("Recording report fail(%s). Out of memory.", finfo->filename);
		return FALSE;
	}

	/* START TAG HERE */
	/* MM_AUDIO_CODEC_AAC + MM_FILE_FORMAT_MP4 */
	if (finfo->fileformat == MM_FILE_FORMAT_3GP || finfo->fileformat == MM_FILE_FORMAT_MP4)
		_mmstreamrecorder_audio_add_metadata_info_m4a(handle);
	/* END TAG HERE */

	report->recording_filename = strdup(finfo->filename);
	msg.param.data = report;

	_mmstreamrecorder_send_message(handle, &msg);

	if (info->bMuxing) {
		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_ENCBIN].gst, "block", FALSE);
	} else {
		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_AQUE].gst, "empty-buffers", FALSE);
	}

	_mmstreamrecorder_gst_set_state(handle, pipeline, GST_STATE_NULL);

	sc->pipeline_time = 0;
	sc->pause_time = 0;
	sc->isMaxsizePausing = FALSE;
	sc->isMaxtimePausing = FALSE;

	g_free(finfo->filename);
	finfo->filename = NULL;

	_mmstreamrec_dbg_err("_MM_STREAMRECORDER_CMD_COMMIT : end");

	info->b_commiting = FALSE;

	return TRUE;
}

int _mmstreamrecorder_push_videostream_buffer(MMHandleType handle, unsigned long timestamp, GstBuffer *buffer, int size)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	GstPad *srcpad = NULL;
	GstCaps *srccaps = NULL;
	char *err_name = NULL;
	int video_fps = 0;
	int video_src = 0;
	int video_width = 0;
	int video_height = 0;
	int ret = MM_ERROR_NONE;
	int video_source_format = 0;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	if (buffer == NULL || size == 0) {
		_mmstreamrec_dbg_err("video : Buffer is %p , size %d, time stamp is %ld", buffer, size, timestamp);
		return MM_ERROR_STREAMRECORDER_RESOURCE_CREATION;
	}

	_mmstreamrec_dbg_log("video : Buffer is %p , size %d, time stamp is %ld", buffer, size, timestamp);

	/* check element availability */
	ret = mm_streamrecorder_get_attributes(handle, &err_name, MMSTR_VIDEO_FRAMERATE, &video_fps, MMSTR_VIDEO_SOURCE_FORMAT, &video_src, MMSTR_VIDEO_RESOLUTION_WIDTH, &video_width, MMSTR_VIDEO_RESOLUTION_HEIGHT, &video_height, MMSTR_VIDEO_SOURCE_FORMAT, &video_source_format, NULL);

	if (sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst) {

		/*_mmstreamrec_dbg_log("Buffer Push start , time stamp %ld",timestamp);*/
		srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst, "src");
		srccaps = gst_pad_get_current_caps(srcpad);
		srccaps = gst_set_videosrcpad_caps(video_src, video_width, video_height, video_fps, 1);
		gst_app_src_set_caps((GstAppSrc *) sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst, srccaps);
		/*_mmstreamrec_dbg_err("newbuf streamrecorder(%p) ",newbuf);*/

		ret = gst_app_src_push_buffer((GstAppSrc *) sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst, buffer);
		if (ret) {
			_mmstreamrec_dbg_err("video gst_app_src_push_buffer %d", ret);
			ret = MM_ERROR_STREAMRECORDER_VIDEOBUFFER_PUSH;
		}

		/* g_signal_emit_by_name(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_SRC].gst, "push-buffer", newbuf, &ret); */
	}

	return ret;
}

int _mmstreamrecorder_push_audiostream_buffer(MMHandleType handle, unsigned long timestamp, GstBuffer *buffer, int size)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;
	GstFlowReturn err = GST_FLOW_OK;
	GstPad *srcpad = NULL;
	GstCaps *srccaps = NULL;
	int rate = 0;
	int channel = 0;
	int depth = 0;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	info = (_MMStreamRecorderAudioInfo *) sc->info_audio;
	mmf_return_val_if_fail(info, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	rate = info->iSamplingRate;
	depth = info->audio_encode_depth;
	channel = info->iChannels;

	/*_mmstreamrec_dbg_log("Audio Buffer Push start , time stamp %ld",timestamp); */

	if (sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst) {

		srcpad = gst_element_get_static_pad(sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst, "src");
		/* TODO : CHANNEL , WIDTH, DATATYPE */
		srccaps = gst_pad_get_current_caps(srcpad);
		srccaps = gst_set_audiosrcpad_caps(rate, channel, depth, 16, 1);
		gst_base_src_set_caps(GST_BASE_SRC(srcpad), srccaps);

		err = gst_app_src_push_buffer((GstAppSrc *) sc->encode_element[_MMSTREAMRECORDER_AUDIOSRC_SRC].gst, buffer);

		if (err) {
			_mmstreamrec_dbg_err("Audio gst_app_src_push_buffer %d", err);
			return MM_ERROR_STREAMRECORDER_AUDIOBUFFER_PUSH;
		}
	}

	return MM_ERROR_NONE;
}

