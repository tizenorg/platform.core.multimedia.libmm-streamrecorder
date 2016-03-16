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

#ifndef __MM_STREAMRECORDER_INI_H__
#define __MM_STREAMRECORDER_INI_H__

#include <glib.h>
#include "mm_debug.h"
#include "mm_streamrecorder.h"
#include "mm_streamrecorder_util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MM_STREAMRECORDER_INI_DEFAULT_PATH	"/usr/etc/mmfw_streamrecorder.ini"

#define STREAMRECORDER_INI_MAX_STRLEN	256
#define STREAMRECORDER_INI_MAX_ELEMENT	10
#define STREAMRECORDER_ATTRIBUTE_NUM_MAX 8

enum keyword_type {
	KEYWORD_VIDEO_WIDTH,
	KEYWORD_VIDEO_HEIGHT,
	KEYWORD_AUDIO_ENCODERS,
	KEYWORD_VIDEO_ENCODERS,
	KEYWORD_FILE_FORMATS,
};

typedef struct __mm_streamrecorder_ini {
	/* general */
	gboolean encsink_src_islive;
	guint retrial_count;
	guint minimum_frame;
	guint convert_output_buffer_num;
	guint reset_pause_time;
	guint screen_record;

	/*encodebin */
	guint encsink_bin_profile;
	gboolean encsink_bin_auto_audio_resample;
	gboolean encsink_bin_auto_colorspace;
	gboolean encsink_bin_auto_audio_convert;
	gboolean encsink_bin_use_video_toggle;

	/* pipeline */
	gchar name_of_encsink_src[STREAMRECORDER_INI_MAX_STRLEN];
	gchar name_of_audio_src[STREAMRECORDER_INI_MAX_STRLEN];
	gchar h264_video_encoder[STREAMRECORDER_INI_MAX_STRLEN];
	gchar h263_video_encoder[STREAMRECORDER_INI_MAX_STRLEN];
	gchar mpeg4_video_encoder[STREAMRECORDER_INI_MAX_STRLEN];
	gchar name_of_encsink_bin_audio_encoder[STREAMRECORDER_INI_MAX_STRLEN];
	gchar name_of_encsink_bin_video_converter[STREAMRECORDER_INI_MAX_STRLEN];
	gchar encsink_bin_use_parser[STREAMRECORDER_INI_MAX_STRLEN];
	gchar name_of_encsink_bin_3GPMUXER[STREAMRECORDER_INI_MAX_STRLEN];
	gchar name_of_encsink_bin_MP4MUXER[STREAMRECORDER_INI_MAX_STRLEN];
	gchar name_of_encsink_sink[STREAMRECORDER_INI_MAX_STRLEN];

	/* audio parameter */
	guint audio_frame_minimum_space;
	guint audio_frame_wait_time;

	/* video parameter */
	guint video_frame_wait_time;

	/*list attributed*/
	gint supported_video_width[STREAMRECORDER_ATTRIBUTE_NUM_MAX];
	gint supported_video_height[STREAMRECORDER_ATTRIBUTE_NUM_MAX];
	gchar supported_audio_encoders[STREAMRECORDER_ATTRIBUTE_NUM_MAX][STREAMRECORDER_INI_MAX_STRLEN];
	gchar supported_video_encoders[STREAMRECORDER_ATTRIBUTE_NUM_MAX][STREAMRECORDER_INI_MAX_STRLEN];
	gchar supported_file_formats[STREAMRECORDER_ATTRIBUTE_NUM_MAX][STREAMRECORDER_INI_MAX_STRLEN];

} mm_streamrecorder_ini_t;

/*Default sink ini values*/
/* General*/
#define DEFAULT_ENCSINK_SRC_IS_LIVE  FALSE
#define DEFAULT_RETRIAL_COUNT 10
#define DEFAULT_MINIMUM_FRAME  5
#define DEFAULT_CONVERT_OUTPUT_BUFFER_NUM 6
#define DEFAULT_RESET_PAUSE_TIME 0
#define DEFAULT_SCREEN_RECORD 1

/*encodebin*/
#define DEFAULT_ENCSINK_BIN_PROFILE 0
#define DEFAULT_ENCSINK_BIN_AUTO_AUDIO_RESAMPLE FALSE
#define DEFAULT_ENCSINK_BIN_AUTO_COLORSPACE TRUE
#define DEFAULT_ENCSINK_BIN_AUTO_CONVERT TRUE
#define DEFAULT_ENCSINK_BIN_USE_VIDEO_TOGGLE FALSE

/* Pipeline */
#define DEFAULT_VIDEO_SOURCE "appsrc"
#define DEFAULT_AUDIO_SRC "pulsesrc"
#define DEFAULT_NAME_OF_H264_VIDEO_ENCODER "omx_h264enc"
#define DEFAULT_NAME_OF_H263_VIDEO_ENCODER "avenc_h263p"
#define DEFAULT_NAME_OF_MPEG4_VIDEO_ENCODER "avenc_mpeg4"
#define DEFAULT_NAME_OF_AUDIO_ENCODER "avenc_aac"
#define DEFAULT_USE_PARSER ""
#define DEFAULT_NAME_OF_3GP_MUXER "avmux_3gp"
#define DEFAULT_NAME_OF_MP4_MUXER "avmux_mp4"
#define DEFAULT_NAME_OF_VIDEO_CONVERTER "videoconvet"
#define DEFAULT_NAME_OF_BIN_SINK "filesink"

/*audio param*/
#define DEFAULT_AUDIO_FRAME_MINIMUM_SPACE 102400
#define DEFAULT_AUDIO_FRAME_WAIT_TIME 20000

/*video param*/
#define DEFAULT_VIDEO_FRAME_WAIT_TIME  200000

/*supported attribute*/
#define DEFAULT_SUPPORTED_WIDTH ""
#define DEFAULT_SUPPORTED_HEIGHT ""
#define DEFAULT_SUPPORTED_AUDIO_ENCODERS ""
#define DEFAULT_SUPPORTED_VIDEO_ENCODERS ""
#define DEFAULT_SUPPORTED_FILE_FORMATS ""

int
 _mm_streamrecorder_ini_load(mm_streamrecorder_ini_t * ini);

int
 _mm_streamrecorder_ini_unload(mm_streamrecorder_ini_t * ini);

#ifdef __cplusplus
}
#endif
#endif
