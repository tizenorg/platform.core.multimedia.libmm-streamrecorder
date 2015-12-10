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
========================================================================================*/
#include <stdio.h>
#include <string.h>
#include <mm_types.h>
#include <mm_error.h>

#include "mm_streamrecorder_internal.h"
#include "mm_streamrecorder_recorder.h"
#include "mm_streamrecorder_attribute.h"
#include "mm_streamrecorder_gstdispatch.h"

#include <asm/types.h>

/*---------------------------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/
#define __MMSTREAMRECORDER_CMD_ITERATE_MAX           3

/*---------------------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:								|
---------------------------------------------------------------------------------------*/

/*=======================================================================================
|  FUNCTION DEFINITIONS									|
=======================================================================================*/
/*---------------------------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:							|
---------------------------------------------------------------------------------------*/

/* Internal command functions {*/
int _mmstreamrecorder_create(MMHandleType *handle)
{
	int ret = MM_ERROR_NONE;
	/* char *err_attr_name = NULL; */
	mmf_streamrecorder_t *hstreamrecorder = NULL;

	_mmstreamrec_dbg_log("Entered");
	mmf_return_val_if_fail(handle, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	/* Create mmf_streamrecorder_t handle and initialize every variable */
	hstreamrecorder = (mmf_streamrecorder_t *) malloc(sizeof(mmf_streamrecorder_t));
	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_LOW_MEMORY);
	memset(hstreamrecorder, 0x00, sizeof(mmf_streamrecorder_t));

	/* init values */
	hstreamrecorder->sub_context = NULL;

	pthread_mutex_init(&((hstreamrecorder->mtsafe).lock), NULL);
	pthread_cond_init(&((hstreamrecorder->mtsafe).cond), NULL);

	pthread_mutex_init(&((hstreamrecorder->mtsafe).cmd_lock), NULL);
	pthread_mutex_init(&((hstreamrecorder->mtsafe).state_lock), NULL);
	pthread_mutex_init(&((hstreamrecorder->mtsafe).gst_state_lock), NULL);
	pthread_mutex_init(&((hstreamrecorder->mtsafe).message_cb_lock), NULL);

	hstreamrecorder->attributes = _mmstreamrecorder_alloc_attribute((MMHandleType) hstreamrecorder);

	if (!(hstreamrecorder->attributes)) {
		_mmstreamrec_dbg_err("_mmstreamrecorder_create::alloc attribute error.");

		ret = MM_ERROR_STREAMRECORDER_RESOURCE_CREATION;
		goto _ERR_ALLOC_ATTRIBUTE;
	}

	_mmstreamrecorder_alloc_subcontext((MMHandleType) hstreamrecorder);
	if (!hstreamrecorder->sub_context) {
		ret = MM_ERROR_STREAMRECORDER_RESOURCE_CREATION;
		goto _ERR_ALLOC_SUBCONTEXT;
	}

	/* Initial GSTREAMER */
	ret = _mmstreamrecorder_gstreamer_init();

	if (!ret) {
		_mmstreamrec_dbg_err("Failed to initialize gstreamer!!");
		ret = MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
		goto _ERR_INITIAL_GSTREAMER;
	}

	ret = _mm_streamrecorder_ini_load(&hstreamrecorder->ini);
	if (ret != MM_ERROR_NONE) {
		_mmstreamrec_dbg_warn("failed to load ini file\n");
		goto _ERR_INITIAL_GSTREAMER;
	}

	*handle = (MMHandleType) hstreamrecorder;

	return MM_ERROR_NONE;

 _ERR_INITIAL_GSTREAMER:
	_mmstreamrecorder_dealloc_subcontext((MMHandleType) hstreamrecorder);
 _ERR_ALLOC_ATTRIBUTE:
 _ERR_ALLOC_SUBCONTEXT:
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).lock));
	pthread_cond_destroy(&((hstreamrecorder->mtsafe).cond));
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).cmd_lock));
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).state_lock));
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).gst_state_lock));
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).message_cb_lock));

	return ret;
}

MMStreamRecorderStateType _mmstreamrecorder_get_state(MMHandleType handle)
{
	int state;
	mmf_streamrecorder_t *streamrecorder = MMF_STREAMRECORDER(handle);

	mmf_return_val_if_fail(streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_MMSTREAMRECORDER_LOCK_STATE(handle);

	state = streamrecorder->state;

	_MMSTREAMRECORDER_UNLOCK_STATE(handle);

	return state;
}


int _mmstreamrecorder_destroy(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("");

	if (!hstreamrecorder) {
		_mmstreamrec_dbg_err("Not initialized");
		ret = MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
		goto _ERR_STREAMRECORDER_CMD_PRECON;
	}

	if (!_MMSTREAMRECORDER_TRYLOCK_CMD(hstreamrecorder)) {
		_mmstreamrec_dbg_err("Another command is running.");
		ret = MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
		goto _ERR_STREAMRECORDER_CMD_PRECON;
	}

	/* Release SubContext and pipeline */
	if (hstreamrecorder->sub_context) {
		if (hstreamrecorder->sub_context->encode_element)
			_mmstreamrecorder_destroy_pipeline(handle);

		_mmstreamrecorder_dealloc_subcontext(handle);
	}

	/* Remove idle function which is not called yet */
	if (hstreamrecorder->setting_event_id) {
		_mmstreamrec_dbg_log("Remove remaining idle function");
		g_source_remove(hstreamrecorder->setting_event_id);
		hstreamrecorder->setting_event_id = 0;
	}

	/* Remove attributes */
	if (hstreamrecorder->attributes) {
		_mmstreamrecorder_dealloc_attribute(hstreamrecorder->attributes);
		hstreamrecorder->attributes = 0;
	}

	/* Remove messages which are not called yet */
	_mmstreamrecorder_remove_message_all(handle);

	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);

	/* Release lock, cond */
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).lock));
	pthread_cond_destroy(&((hstreamrecorder->mtsafe).cond));
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).cmd_lock));
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).state_lock));
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).gst_state_lock));
	pthread_mutex_destroy(&((hstreamrecorder->mtsafe).message_cb_lock));

	/* Release handle */
	memset(hstreamrecorder, 0x00, sizeof(mmf_streamrecorder_t));
	free(hstreamrecorder);

	return MM_ERROR_NONE;

 _ERR_STREAMRECORDER_CMD_PRECON:

	_mmstreamrec_dbg_err("Destroy fail (ret %x)", ret);

	return ret;
}

int _mmstreamrecorder_realize(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;
/*	int state = MM_STREAMRECORDER_STATE_NONE;
	int state_FROM = MM_STREAMRECORDER_STATE_NULL;
	int state_TO = MM_STREAMRECORDER_STATE_READY;
	int errorcode = MM_ERROR_NONE;
	char *videosink_element_type = NULL;
	char *videosink_name = NULL; */

	_mmstreamrec_dbg_log("");

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	if (!hstreamrecorder) {
		_mmstreamrec_dbg_err("Not initialized");
		ret = MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
		return ret;
	}

	/* Check quick-device-close for emergency */
	if (hstreamrecorder->quick_device_close) {
		_mmstreamrec_dbg_err("_mmstreamrecorder_realize can't be called!!!!");
		ret = MM_ERROR_STREAMRECORDER_DEVICE;
		goto _ERR_STREAMRECORDER_CMD_PRECON;
	}

	if (!_MMSTREAMRECORDER_TRYLOCK_CMD(hstreamrecorder)) {
		_mmstreamrec_dbg_err("Another command is running.");
		ret = MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
		goto _ERR_STREAMRECORDER_CMD_PRECON;
	}

	/* create pipeline */
	ret = _mmstreamrecorder_create_pipeline(handle);
	if (ret != MM_ERROR_NONE) {
		/* check internal error of gstreamer */
		if (hstreamrecorder->sub_context->error_code != MM_ERROR_NONE) {
			ret = hstreamrecorder->sub_context->error_code;
			_mmstreamrec_dbg_log("gstreamer error is occurred. return it %x", ret);
		}
		/* release sub context */
		_mmstreamrecorder_dealloc_subcontext(handle);
		goto _ERR_STREAMRECORDER_CMD;
	}

	/* set command function */
	_mmstreamrecorder_set_functions(handle);

	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);

	return MM_ERROR_NONE;

 _ERR_STREAMRECORDER_CMD:
/* _ERR_STREAMRECORDER_CMD_PRECON_AFTER_LOCK: */
	/* Security thread release */
	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);

 _ERR_STREAMRECORDER_CMD_PRECON:
/*	_mmstreamrec_dbg_err("Realize fail (type %d, state %d, ret %x)", hstreamrecorder->type, state, ret); */

	return ret;
}

int _mmstreamrecorder_unrealize(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("");

	if (!hstreamrecorder) {
		_mmstreamrec_dbg_err("Not initialized");
		ret = MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
		return ret;
	}

	if (!_MMSTREAMRECORDER_TRYLOCK_CMD(hstreamrecorder)) {
		_mmstreamrec_dbg_err("Another command is running.");
		ret = MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
		goto _ERR_STREAMRECORDER_CMD_PRECON;
	}

	/* Release SubContext */
	if (hstreamrecorder->sub_context) {
		/* destroy pipeline */
		_mmstreamrecorder_destroy_pipeline(handle);
		/* Deallocate SubContext */
	}

	/* Deinitialize main context member */
	_mmstreamrecorder_unset_functions(handle);

	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);

	return MM_ERROR_NONE;

 _ERR_STREAMRECORDER_CMD_PRECON:
	/* send message */
	return ret;
}

int _mmstreamrecorder_record(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;

	_MMStreamRecorderSubContext *sc = NULL;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("");

	if (!hstreamrecorder) {
		_mmstreamrec_dbg_err("Not initialized");
		ret = MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
		return ret;
	}

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	/* check quick-device-close for emergency */
	if (hstreamrecorder->quick_device_close) {
		_mmstreamrec_dbg_err("_mmstreamrecorder_start can't be called!!!!");
		ret = MM_ERROR_STREAMRECORDER_DEVICE;
		goto _ERR_STREAMRECORDER_CMD_PRECON;
	}

	/* initialize error code */
	hstreamrecorder->sub_context->error_code = MM_ERROR_NONE;

	ret = hstreamrecorder->command((MMHandleType) hstreamrecorder, _MM_STREAMRECORDER_CMD_RECORD);
	if (ret != MM_ERROR_NONE) {
		_mmstreamrec_dbg_err("_mmstreamrecorder_record does not work!");
		goto _ERR_STREAMRECORDER_CMD;
	}

	return MM_ERROR_NONE;

 _ERR_STREAMRECORDER_CMD:
 _ERR_STREAMRECORDER_CMD_PRECON:
	/* check internal error of gstreamer */
	if (hstreamrecorder->sub_context->error_code != MM_ERROR_NONE) {
		ret = hstreamrecorder->sub_context->error_code;
		hstreamrecorder->sub_context->error_code = MM_ERROR_NONE;

		_mmstreamrec_dbg_log("gstreamer error is occurred. return it %x", ret);
	}
	return ret;
}

int _mmstreamrecorder_push_stream_buffer(MMHandleType handle, MMStreamRecorderStreamType streamtype, unsigned long timestamp, void *buffer, int size)
{
	int ret = MM_ERROR_NONE;

	int format;
	GstMapInfo map;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	/* _mmstreamrec_dbg_log(""); */

	if (!hstreamrecorder) {
		_mmstreamrec_dbg_err("Not initialized");
		ret = MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
		return ret;
	}
	mm_streamrecorder_get_attributes(handle, NULL,
					MMSTR_VIDEO_SOURCE_FORMAT, &format,
					NULL);
	GstStreamRecorderBuffer *stream_buffer = NULL;
	stream_buffer = (GstStreamRecorderBuffer *) malloc(sizeof(GstStreamRecorderBuffer));
	if (stream_buffer == NULL) {
		_mmstreamrec_dbg_err("stream buffer allocation fail");
		return MM_ERROR_STREAMRECORDER_LOW_MEMORY;
	}
	stream_buffer->str_handle = handle;
	stream_buffer->buffer = gst_buffer_new();
	if (stream_buffer->buffer == NULL) {
		free(stream_buffer);
		stream_buffer = NULL;
		_mmstreamrec_dbg_err("gst buffer allocation fail");
		return MM_ERROR_STREAMRECORDER_LOW_MEMORY;
	}
	stream_buffer->user_buffer = buffer;
	/* Get Media Packet to Surface to MMVideoBuffer */

	stream_buffer->buffer->pts = timestamp;
	GST_BUFFER_DURATION(stream_buffer->buffer) = GST_CLOCK_TIME_NONE;

	gst_buffer_map(stream_buffer->buffer, &map, GST_MAP_READWRITE);
	if (streamtype == MM_STREAM_TYPE_VIDEO) {
		if (format == MM_STREAMRECORDER_INPUT_FORMAT_NV12 || format == MM_STREAMRECORDER_INPUT_FORMAT_NV21) {

			MMVideoBuffer *video_buf = (MMVideoBuffer *)buffer;
			/* Buffer at 0th position */
			gst_buffer_append_memory(stream_buffer->buffer, gst_memory_new_wrapped(GST_MEMORY_FLAG_READONLY,
								video_buf->handle.paddr[0], size, 0, size, video_buf->handle.paddr[0], NULL));
			/* Buffer at 1st position */
			gst_buffer_append_memory(stream_buffer->buffer, gst_memory_new_wrapped(GST_MEMORY_FLAG_READONLY,
								video_buf, sizeof(MMVideoBuffer), 0, sizeof(MMVideoBuffer), stream_buffer, _mmstreamrecorder_buffer_destroy));
		} else {
				gst_buffer_append_memory(stream_buffer->buffer, gst_memory_new_wrapped(GST_MEMORY_FLAG_READONLY,
								buffer, size, 0, size, stream_buffer, _mmstreamrecorder_buffer_destroy));
		}
		ret = _mmstreamrecorder_push_videostream_buffer(handle, timestamp, stream_buffer->buffer, size);
	} else if (streamtype == MM_STREAM_TYPE_AUDIO) {
		gst_buffer_append_memory(stream_buffer->buffer, gst_memory_new_wrapped(GST_MEMORY_FLAG_READONLY,
								buffer, size, 0, buffer, stream_buffer, _mmstreamrecorder_buffer_destroy));
		ret = _mmstreamrecorder_push_audiostream_buffer(handle, timestamp, stream_buffer->buffer, size);
	} else {
		gst_buffer_unmap(stream_buffer->buffer, &map);
		gst_object_unref(stream_buffer->buffer);
		free(stream_buffer);
		stream_buffer = NULL;
		return MM_ERROR_STREAMRECORDER_INVALID_CONDITION;
	}
	gst_buffer_unmap(stream_buffer->buffer, &map);
	return ret;

}

int _mmstreamrecorder_pause(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("");

	if (!hstreamrecorder) {
		_mmstreamrec_dbg_err("Not initialized");
		ret = MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
		return ret;
	}

	if (!_MMSTREAMRECORDER_TRYLOCK_CMD(hstreamrecorder)) {
		_mmstreamrec_dbg_err("Another command is running.");
		ret = MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
		goto _ERR_STREAMRECORDER_CMD_PRECON;
	}

	ret = hstreamrecorder->command((MMHandleType) hstreamrecorder, _MM_STREAMRECORDER_CMD_PAUSE);
	if (ret != MM_ERROR_NONE)
		goto _ERR_STREAMRECORDER_CMD;

	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);

	return MM_ERROR_NONE;

 _ERR_STREAMRECORDER_CMD:
	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);
 _ERR_STREAMRECORDER_CMD_PRECON:
	/* send message */
	return ret;
}

int _mmstreamrecorder_commit(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("");

	if (!hstreamrecorder) {
		_mmstreamrec_dbg_err("Not initialized");
		ret = MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
		return ret;
	}

	if (!_MMSTREAMRECORDER_TRYLOCK_CMD(hstreamrecorder)) {
		_mmstreamrec_dbg_err("Another command is running.");
		ret = MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
		goto _ERR_STREAMRECORDER_CMD_PRECON;
	}

	ret = hstreamrecorder->command((MMHandleType) hstreamrecorder, _MM_STREAMRECORDER_CMD_COMMIT);
	if (ret != MM_ERROR_NONE)
		goto _ERR_STREAMRECORDER_CMD;

	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);

	return MM_ERROR_NONE;

 _ERR_STREAMRECORDER_CMD:
	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);
 _ERR_STREAMRECORDER_CMD_PRECON:
	/* send message */

	return ret;
}

int _mmstreamrecorder_cancel(MMHandleType handle)
{
	int ret = MM_ERROR_NONE;

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("");

	if (!hstreamrecorder) {
		_mmstreamrec_dbg_err("Not initialized");
		ret = MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
		return ret;
	}

	if (!_MMSTREAMRECORDER_TRYLOCK_CMD(hstreamrecorder)) {
		_mmstreamrec_dbg_err("Another command is running.");
		ret = MM_ERROR_STREAMRECORDER_CMD_IS_RUNNING;
		goto _ERR_STREAMRECORDER_CMD_PRECON;
	}

	ret = hstreamrecorder->command((MMHandleType) hstreamrecorder, _MM_STREAMRECORDER_CMD_CANCEL);
	if (ret != MM_ERROR_NONE)
		goto _ERR_STREAMRECORDER_CMD;

	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);

	return MM_ERROR_NONE;

 _ERR_STREAMRECORDER_CMD:
	_MMSTREAMRECORDER_UNLOCK_CMD(hstreamrecorder);
 _ERR_STREAMRECORDER_CMD_PRECON:
	/* send message */

	return ret;
}

/* } Internal command functions */

int _mmstreamrecorder_set_message_callback(MMHandleType handle, MMMessageCallback callback, void *user_data)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("%p", hstreamrecorder);

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	if (callback == NULL) {
		_mmstreamrec_dbg_warn("Message Callback is disabled");
		_mmstreamrec_dbg_warn("Application sets callback as NULL");
	}

	if (!_MMSTREAMRECORDER_TRYLOCK_MESSAGE_CALLBACK(hstreamrecorder)) {
		_mmstreamrec_dbg_warn("Application's message callback is running now");
		return MM_ERROR_STREAMRECORDER_INVALID_CONDITION;
	}

	/* set message callback to message handle */
	hstreamrecorder->msg_cb = callback;
	hstreamrecorder->msg_cb_param = user_data;

	_MMSTREAMRECORDER_UNLOCK_MESSAGE_CALLBACK(hstreamrecorder);

	return MM_ERROR_NONE;
}

int _mmstreamrecorder_alloc_subcontext(MMHandleType handle)
{
	int i;
	int ret = MM_ERROR_NONE;
	_MMStreamRecorderSubContext *sc = NULL;

	_mmstreamrec_dbg_log("");

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_RESOURCE_CREATION);

	/* alloc container */
	sc = (_MMStreamRecorderSubContext *) malloc(sizeof(_MMStreamRecorderSubContext));
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_LOW_MEMORY);

	/* init members */
	memset(sc, 0x00, sizeof(_MMStreamRecorderSubContext));

	sc->encode_element_num = _MMSTREAMRECORDER_ENCODE_PIPELINE_ELEMENT_NUM;

	/* alloc element array */
	sc->encode_element = (_MMStreamRecorderGstElement *) malloc(sizeof(_MMStreamRecorderGstElement) * sc->encode_element_num);
	if (!sc->encode_element) {
		_mmstreamrec_dbg_err("Failed to alloc encode element structure");
		goto ALLOC_SUBCONTEXT_FAILED;
	}

	for (i = 0; i < sc->encode_element_num; i++) {
		sc->encode_element[i].id = _MMSTREAMRECORDER_ENCODE_NONE;
		sc->encode_element[i].gst = NULL;
	}

	sc->fourcc = 0x80000000;
	sc->pass_first_vframe = 0;

	hstreamrecorder->sub_context = sc;

	ret = _mmstreamrecorder_alloc_subcontext_fileinfo((MMHandleType) hstreamrecorder);
	if (ret != MM_ERROR_NONE) {
		_mmstreamrec_dbg_err("Failed to allocate subcontext fileinfo");
		goto ALLOC_SUBCONTEXT_FAILED;
	}

	ret = _mmstreamrecorder_alloc_subcontext_videoinfo((MMHandleType) hstreamrecorder);
	if (ret != MM_ERROR_NONE) {
		_mmstreamrec_dbg_err("Failed to allocate subcontext videoinfo");
		goto ALLOC_SUBCONTEXT_FAILED;
	}

	_mmstreamrecorder_alloc_subcontext_audioinfo((MMHandleType) hstreamrecorder);
	if (ret != MM_ERROR_NONE) {
		_mmstreamrec_dbg_err("Failed to allocate subcontext audioinfo");
		goto ALLOC_SUBCONTEXT_FAILED;
	}

	return MM_ERROR_NONE;

 ALLOC_SUBCONTEXT_FAILED:

	if (sc) {
		if (sc->encode_element) {
			free(sc->encode_element);
			sc->encode_element = NULL;
		}
		free(sc);
		sc = NULL;
	}
	if (hstreamrecorder->sub_context != NULL)
		hstreamrecorder->sub_context = NULL;
	return MM_ERROR_STREAMRECORDER_LOW_MEMORY;
}

int _mmstreamrecorder_alloc_subcontext_videoinfo(MMHandleType handle)
{
	_MMStreamRecorderSubContext *sc = NULL;

	_mmstreamrec_dbg_log("");

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	/* alloc info for each mode */

	sc->info_video = malloc(sizeof(_MMStreamRecorderVideoInfo));
	if (!sc->info_video) {
		_mmstreamrec_dbg_err("Failed to alloc info structure");
		return MM_ERROR_STREAMRECORDER_LOW_MEMORY;
	}
	memset(sc->info_video, 0x00, sizeof(_MMStreamRecorderVideoInfo));

	return MM_ERROR_NONE;
}

int _mmstreamrecorder_alloc_subcontext_audioinfo(MMHandleType handle)
{
	_MMStreamRecorderSubContext *sc = NULL;

	_mmstreamrec_dbg_log("");

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	/* alloc info for each mode */
	sc->info_audio = malloc(sizeof(_MMStreamRecorderAudioInfo));
	if (!sc->info_audio) {
		_mmstreamrec_dbg_err("Failed to alloc info structure");
		return MM_ERROR_STREAMRECORDER_LOW_MEMORY;
	}
	memset(sc->info_audio, 0x00, sizeof(_MMStreamRecorderAudioInfo));

	return MM_ERROR_NONE;
}

int _mmstreamrecorder_alloc_subcontext_fileinfo(MMHandleType handle)
{
	_MMStreamRecorderSubContext *sc = NULL;

	_mmstreamrec_dbg_log("");

	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	/* alloc info for each mode */
	sc->info_file = malloc(sizeof(_MMStreamRecorderFileInfo));
	if (!sc->info_file) {
		_mmstreamrec_dbg_err("Failed to alloc info structure");
		return MM_ERROR_STREAMRECORDER_LOW_MEMORY;
	}
	memset(sc->info_file, 0x00, sizeof(_MMStreamRecorderFileInfo));

	return MM_ERROR_NONE;
}

void _mmstreamrecorder_dealloc_subcontext(MMHandleType handle)
{
	_MMStreamRecorderSubContext *sc = NULL;

	_mmstreamrec_dbg_log("");

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	mmf_return_if_fail(hstreamrecorder);
	mmf_return_if_fail(hstreamrecorder->sub_context);

	sc = hstreamrecorder->sub_context;

	if (sc) {

		if (sc->encode_element) {
			_mmstreamrec_dbg_log("release encode_element");
			free(sc->encode_element);
			sc->encode_element = NULL;
		}

		if (sc->info_video) {
			_mmstreamrec_dbg_log("release info_video");
			free(sc->info_video);
			sc->info_video = NULL;
		}

		if (sc->info_audio) {
			_mmstreamrec_dbg_log("release info_audio");
			free(sc->info_audio);
			sc->info_audio = NULL;
		}

		if (sc->info_file) {
			_mmstreamrec_dbg_log("release info_file");
			free(sc->info_file);
			sc->info_file = NULL;
		}

		free(sc);
		sc = NULL;
	}
	if (hstreamrecorder->sub_context != NULL)
		hstreamrecorder->sub_context = NULL;

	return;
}

void _mmstreamrecorder_set_functions(MMHandleType handle)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("");

	/* Now only video type */

	hstreamrecorder->command = _mmstreamrecorder_video_command;

	return;
}

void _mmstreamrecorder_unset_functions(MMHandleType handle)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	_mmstreamrec_dbg_log("");

	hstreamrecorder->command = NULL;

	return;
}
