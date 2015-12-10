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

#ifndef __MM_STREAMRECORDER_RECORDER_H__
#define __MM_STREAMRECORDER_RECORDER_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/
#include <mm_types.h>
#include "mm_streamrecorder_gstdispatch.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| GLOBAL DEFINITIONS AND DECLARATIONS FOR STREAMCORDER					|
========================================================================================*/

/*=======================================================================================
| MACRO DEFINITIONS									|
========================================================================================*/

/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/

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

/**
 * This function create main pipeline according to type.
 *
 * @param[in]	handle		Handle of streamrecorder context.
 * @param[in]	type		Allocation type of streamrecorder context.
 * @return	This function returns zero on success, or negative value with error code.
 * @remarks
 * @see 	_mmstreamrecorder_destroy_pipeline()
 *
 */
int _mmstreamrecorder_create_pipeline(MMHandleType handle);

/**
 * This function release all element of main pipeline according to type.
 *
 * @param[in]	handle		Handle of streamrecorder context.
 * @param[in]	type		Allocation type of streamrecorder context.
 * @return	void
 * @remarks
 * @see		_mmstreamrecorder_create_pipeline()
 *
 */
void _mmstreamrecorder_destroy_pipeline(MMHandleType handle);

/**
 * This function creates recorder pipeline.
 * When application creates initial pipeline, there are only bins for preview.
 * If application wants to record, recorder pipeline should be created.
 *
 * @param[in]	handle		Handle of streamrecorder context.
 * @return	This function returns MM_ERROR_NONE on success, or the other values on error.
 * @remarks
 */
int _mmstreamrecorder_create_recorder_pipeline(MMHandleType handle);

/**
 * This function creates bin of audio source.
 * Basically, main pipeline of streamrecorder is composed of several bin(a bundle
 *  of elements). Each bin operates its own function. This bin has a roll
 * inputting audio data from audio source(such as mike).
 *
 * @param[in]	handle		Handle of streamrecorder context.
 * @return	This function returns MM_ERROR_NONE on success, or the other values on error.
 * @remarks
 */
int _mmstreamrecorder_create_audiosrc_bin(MMHandleType handle);

int _mmstreamrecorder_destroy_audiosrc_bin(MMHandleType handle);

/**
 * This function creates outputsink bin.
 *
 * @param[in]	handle		Handle of streamrecorder context.
 * @param[in]	profile		profile of encodesinkbin.
 * @return	This function returns MM_ERROR_NONE on success, or the other values on error.
 * @remarks
 */
int _mmstreamrecorder_create_encodesink_bin(MMHandleType handle, MMStreamRecorderEncodebinProfile profile);

int _mmstreamrecorder_destroy_encodesink_bin(MMHandleType handle);

/**
 * This function handles EOS(end of stream) when commit video recording.
 *
 * @param[in]	handle		Handle of streamrecorder context.
 * @return	This function returns TRUE on success, or FALSE on failure.
 * @remarks
 * @see		_mmstreamrecorder_set_functions()
 */
int _mmstreamrecorder_video_handle_eos(MMHandleType handle);

int _mmstreamrecorder_destroy_recorder_pipeline(MMHandleType handle);

// COMMAND
int _mmstreamrecorder_video_command(MMHandleType handle, int command);

/**
 * This function creates audio pipeline for audio recording.
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @return	This function returns MM_ERROR_NONE on success, or others on failure.
 * @remarks
 * @see		_mmstreamrecorder_destroy_audio_pipeline()
 *
 */
int _mmstreamrecorder_create_audio_pipeline(MMHandleType handle);

/**
 * This function destroy audio pipeline.
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @return	void
 * @remarks
 * @see		_mmstreamrecorder_destroy_pipeline()
 *
 */
void _mmstreamrecorder_destroy_audio_pipeline(MMHandleType handle);

/**
 * This function runs command for audio recording.
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @param[in]	command		audio recording command.
 * @return	This function returns MM_ERROR_NONE on success, or others on failure.
 * @remarks
 * @see
 *
 */
int _mmstreamrecorder_audio_command(MMHandleType handle, int command);

/**
 * This function handles EOS(end of stream) when audio recording is finished.
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @return	This function returns TRUE on success, or FALSE on failure.
 * @remarks
 * @see
 *
 */
int _mmstreamrecorder_audio_handle_eos(MMHandleType handle);

int _mmstreamrecorder_create_audiop_with_encodebin(MMHandleType handle);

int _mmstreamrecorder_push_videostream_buffer(MMHandleType handle, unsigned long timestamp, void *buffer, int size);
int _mmstreamrecorder_push_audiostream_buffer(MMHandleType handle, unsigned long timestamp, void *buffer, int size);
int _mmstreamrecorder_push_datastream_buffer(MMHandleType handle, unsigned long timestamp, void *buffer, int size);

#ifdef __cplusplus
}
#endif

#endif /* __MM_STREAMRECORDER_RECORDER_H__ */
