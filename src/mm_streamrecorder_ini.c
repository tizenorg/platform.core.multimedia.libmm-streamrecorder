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

#include <glib.h>
#include <glib/gstdio.h>
#include <stdlib.h>
#include <iniparser.h>
#include <mm_debug.h>
#include <mm_error.h>
#include "mm_streamrecorder_ini.h"

static gboolean loaded = FALSE;

/* global variables here */
#ifdef MM_STREAMRECORDER_DEFAULT_INI
static gboolean __generate_default_ini(void);
#endif

static void __mm_streamrecorder_ini_check_status(void);

/* macro */
#define MM_STREAMRECORDER_INI_GET_STRING(x_dict, x_item, x_ini, x_default) \
do { \
	gchar *str = NULL; \
	gint length = 0; \
	str = iniparser_getstring(x_dict, x_ini, x_default); \
	if (str) { \
		length = strlen(str); \
		if ((length > 1) && (length < STREAMRECORDER_INI_MAX_STRLEN)) \
			strncpy(x_item, str, length+1); \
		else \
			strncpy(x_item, x_default, STREAMRECORDER_INI_MAX_STRLEN-1); \
	} else { \
		strncpy(x_item, x_default, STREAMRECORDER_INI_MAX_STRLEN-1); \
	} \
} while (0)

#ifdef MM_STREAMRECORDER_DEFAULT_INI
static
gboolean __generate_default_ini(void)
{
	FILE *fp = NULL;
	const gchar *default_ini = MM_STREAMRECORDER_DEFAULT_INI;

	/* create new file */
	fp = fopen(MM_STREAMRECORDER_INI_DEFAULT_PATH, "wt");

	if (!fp)
		return FALSE;

	/* writing default ini file */
	if (strlen(default_ini) != fwrite(default_ini, 1, strlen(default_ini), fp)) {
		fclose(fp);
		return FALSE;
	}

	fclose(fp);
	return TRUE;
}
#endif

int _mm_streamrecorder_ini_load(mm_streamrecorder_ini_t *ini)
{
	dictionary *dict = NULL;

	debug_fenter();

	__mm_streamrecorder_ini_check_status();

	/* first, try to load existing ini file */
	dict = iniparser_load(MM_STREAMRECORDER_INI_DEFAULT_PATH);

	/* if no file exists. create one with set of default values */
	if (!dict) {
		_mmstreamrec_dbg_err("No ini file found. \n");
		return MM_ERROR_FILE_NOT_FOUND;
	}

	/* get ini values */
	memset(ini, 0, sizeof(mm_streamrecorder_ini_t));

	if (dict) {					/* if dict is available */
		/* general */
		ini->encsink_src_islive = iniparser_getboolean(dict, "general:encscink source is live", DEFAULT_ENCSINK_SRC_IS_LIVE);
		ini->retrial_count = iniparser_getint(dict, "general:retrialcount", DEFAULT_RETRIAL_COUNT);
		ini->minimum_frame = iniparser_getint(dict, "general:minimum frame", DEFAULT_MINIMUM_FRAME);
		ini->convert_output_buffer_num = iniparser_getint(dict, "general:convert output buffer num", DEFAULT_CONVERT_OUTPUT_BUFFER_NUM);
		ini->reset_pause_time = iniparser_getint(dict, "general:reset pause time", DEFAULT_RESET_PAUSE_TIME);
		ini->screen_record = iniparser_getint(dict, "general:screen record", DEFAULT_SCREEN_RECORD);

		/*encodebin */
		ini->encsink_bin_profile = iniparser_getint(dict, "encodebin:encsink bin profile", DEFAULT_ENCSINK_BIN_PROFILE);
		ini->encsink_bin_auto_audio_resample = iniparser_getboolean(dict, "encodebin:encsink bin auto audio resample", DEFAULT_ENCSINK_BIN_AUTO_AUDIO_RESAMPLE);
		ini->encsink_bin_auto_colorspace = iniparser_getboolean(dict, "encodebin: encsink bin auto colorspace", DEFAULT_ENCSINK_BIN_AUTO_COLORSPACE);
		ini->encsink_bin_use_video_toggle = iniparser_getboolean(dict, "encodebin:encsink bin use video toggle", DEFAULT_ENCSINK_BIN_USE_VIDEO_TOGGLE);
		ini->encsink_bin_auto_audio_convert = iniparser_getboolean(dict, "encodebin:encsink bin auto audio convert", DEFAULT_ENCSINK_BIN_AUTO_CONVERT);

		/* pipeline */
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->name_of_encsink_src, (const char *)"pipeline:encsink bin source", (char *)DEFAULT_VIDEO_SOURCE);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->name_of_audio_src, (const char *)"pipeline:name of audio src", (char *)DEFAULT_AUDIO_SRC);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->h264_video_encoder, (const char *)"pipeline:h264 encoder", (char *)DEFAULT_NAME_OF_H264_VIDEO_ENCODER);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->h263_video_encoder, (const char *)"pipeline:h263 encoder", (char *)DEFAULT_NAME_OF_H263_VIDEO_ENCODER);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->mpeg4_video_encoder, (const char *)"pipeline:mpeg4 encoder", (char *)DEFAULT_NAME_OF_MPEG4_VIDEO_ENCODER);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->name_of_encsink_bin_audio_encoder, (const char *)"pipeline:name of audio encoder", (char *)DEFAULT_NAME_OF_AUDIO_ENCODER);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->encsink_bin_use_parser, (const char *)"pipeline:use parser", (char *)DEFAULT_USE_PARSER);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->name_of_encsink_bin_video_converter, (const char *)"pipeline:name of video converter", (char *)DEFAULT_NAME_OF_VIDEO_CONVERTER);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->name_of_encsink_bin_3GPMUXER, (const char *)"pipeline:name of 3GP muxer", (char *)DEFAULT_NAME_OF_3GP_MUXER);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->name_of_encsink_bin_MP4MUXER, (const char *)"pipeline:name of MP4 muxer", (char *)DEFAULT_NAME_OF_MP4_MUXER);
		MM_STREAMRECORDER_INI_GET_STRING(dict, ini->name_of_encsink_sink, (const char *)"pipeline:name of sink", (char *)DEFAULT_NAME_OF_BIN_SINK);

		/* audio parameter */
		ini->audio_frame_minimum_space = iniparser_getint(dict, "audio param:audio frame minimum space", DEFAULT_AUDIO_FRAME_MINIMUM_SPACE);
		ini->audio_frame_wait_time = iniparser_getint(dict, "audio param:audio frame wait time", DEFAULT_AUDIO_FRAME_WAIT_TIME);

		/* video parameter */
		ini->video_frame_wait_time = iniparser_getint(dict, "video param:video frame wait time", DEFAULT_VIDEO_FRAME_WAIT_TIME);

	} else {					/* if dict is not available just fill the structure with default value */
		_mmstreamrec_dbg_err("failed to load ini. using hardcoded default\n");
		/* general */
		ini->encsink_src_islive = DEFAULT_ENCSINK_SRC_IS_LIVE;
		ini->retrial_count = DEFAULT_RETRIAL_COUNT;
		ini->minimum_frame = DEFAULT_MINIMUM_FRAME;
		ini->convert_output_buffer_num = DEFAULT_CONVERT_OUTPUT_BUFFER_NUM;
		ini->reset_pause_time = DEFAULT_RESET_PAUSE_TIME;
		ini->screen_record = DEFAULT_SCREEN_RECORD;

		/*encodebin */
		ini->encsink_bin_profile = DEFAULT_ENCSINK_BIN_PROFILE;
		ini->encsink_bin_auto_audio_resample = DEFAULT_ENCSINK_BIN_AUTO_AUDIO_RESAMPLE;
		ini->encsink_bin_auto_colorspace = DEFAULT_ENCSINK_BIN_AUTO_COLORSPACE;
		ini->encsink_bin_use_video_toggle = DEFAULT_ENCSINK_BIN_USE_VIDEO_TOGGLE;
		ini->encsink_bin_auto_audio_convert = DEFAULT_ENCSINK_BIN_AUTO_CONVERT;

		/* pipeline */
		strncpy(ini->name_of_encsink_src, DEFAULT_VIDEO_SOURCE, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->name_of_audio_src, DEFAULT_AUDIO_SRC, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->h264_video_encoder, DEFAULT_NAME_OF_H264_VIDEO_ENCODER, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->h263_video_encoder, DEFAULT_NAME_OF_H263_VIDEO_ENCODER, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->mpeg4_video_encoder, DEFAULT_NAME_OF_MPEG4_VIDEO_ENCODER, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->name_of_encsink_bin_audio_encoder, DEFAULT_NAME_OF_AUDIO_ENCODER, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->encsink_bin_use_parser, DEFAULT_USE_PARSER, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->name_of_encsink_bin_video_converter, DEFAULT_NAME_OF_VIDEO_CONVERTER, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->name_of_encsink_bin_3GPMUXER, DEFAULT_NAME_OF_3GP_MUXER, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->name_of_encsink_bin_MP4MUXER, DEFAULT_NAME_OF_MP4_MUXER, STREAMRECORDER_INI_MAX_STRLEN - 1);
		strncpy(ini->name_of_encsink_sink, DEFAULT_NAME_OF_BIN_SINK, STREAMRECORDER_INI_MAX_STRLEN - 1);

		/* audio parameter */
		ini->audio_frame_minimum_space = DEFAULT_AUDIO_FRAME_MINIMUM_SPACE;
		ini->audio_frame_wait_time = DEFAULT_AUDIO_FRAME_WAIT_TIME;

		/* video parameter */
		ini->video_frame_wait_time = DEFAULT_VIDEO_FRAME_WAIT_TIME;

	}

	/* free dict as we got our own structure */
	iniparser_freedict(dict);

	/* dump structure */
	_mmstreamrec_dbg_log("Stream Recorder initial settings.......................................\n");

	/* general */
	_mmstreamrec_dbg_log("encsink_src_islive : %d\n", ini->encsink_src_islive);
	_mmstreamrec_dbg_log("retrial_count : %d\n", ini->retrial_count);
	_mmstreamrec_dbg_log("minimum_frame : %d\n", ini->minimum_frame);
	_mmstreamrec_dbg_log("convert_output_buffer_num : %d\n", ini->convert_output_buffer_num);
	_mmstreamrec_dbg_log("reset_pause_time : %d\n", ini->reset_pause_time);
	_mmstreamrec_dbg_log("screen_record : %d\n", ini->screen_record);

	/*encodebin */
	_mmstreamrec_dbg_log("encode bin profile : %d\n", ini->encsink_bin_profile);
	_mmstreamrec_dbg_log("encode bin auto audio resample property  : %d\n", ini->encsink_bin_auto_audio_resample);
	_mmstreamrec_dbg_log("encode bin auto colorspace property : %d\n", ini->encsink_bin_auto_colorspace);
	_mmstreamrec_dbg_log("encode bin use video toggle property : %d\n", ini->encsink_bin_use_video_toggle);
	_mmstreamrec_dbg_log("encode bin auto audio convert property : %d\n", ini->encsink_bin_auto_audio_convert);

	/* pipeline */
	_mmstreamrec_dbg_log("name_of_encodebin_source : %s\n", ini->name_of_encsink_src);
	_mmstreamrec_dbg_log("name_of_audio_source : %s\n", ini->name_of_audio_src);
	_mmstreamrec_dbg_log("name_of_h264_video_encoder : %s\n", ini->h264_video_encoder);
	_mmstreamrec_dbg_log("name_of_h263_video_encoder : %s\n", ini->h263_video_encoder);
	_mmstreamrec_dbg_log("name_of_mpeg4_video_encoder : %s\n", ini->mpeg4_video_encoder);
	_mmstreamrec_dbg_log("name_of_audio_encoder : %s\n", ini->name_of_encsink_bin_audio_encoder);
	_mmstreamrec_dbg_log("name_of_video_converter : %s\n", ini->name_of_encsink_bin_video_converter);
	_mmstreamrec_dbg_log("name_of_3GP_muxer : %s\n", ini->name_of_encsink_bin_3GPMUXER);
	_mmstreamrec_dbg_log("name_of_MP4_muxer : %s\n", ini->name_of_encsink_bin_MP4MUXER);
	_mmstreamrec_dbg_log("name_of_sink : %s\n", ini->name_of_encsink_sink);

	/* audio parameter */
	_mmstreamrec_dbg_log("audio_frame_minimum_space : %d\n", ini->audio_frame_minimum_space);
	_mmstreamrec_dbg_log("audio_frame_wait_time : %d\n", ini->audio_frame_wait_time);

	/* video parameter */
	_mmstreamrec_dbg_log("video_frame_wait_time : %d\n", ini->video_frame_wait_time);

	_mmstreamrec_dbg_log("---------------------------------------------------\n");

	loaded = TRUE;

	debug_fleave();

	return MM_ERROR_NONE;
}

static
void __mm_streamrecorder_ini_check_status(void)
{
	struct stat ini_buff;

	debug_fenter();

	if (g_stat(MM_STREAMRECORDER_INI_DEFAULT_PATH, &ini_buff) < 0) {
		_mmstreamrec_dbg_err("failed to get mmfw_wfd_sink ini status\n");
	} else {
		if (ini_buff.st_size < 5) {
			_mmstreamrec_dbg_err("mmfw_wfd_sink.ini file size=%d, Corrupted! So, Removed\n", (int)ini_buff.st_size);
			g_remove(MM_STREAMRECORDER_INI_DEFAULT_PATH);
		}
	}

	debug_fleave();
}

int _mm_streamrecorder_ini_unload(mm_streamrecorder_ini_t *ini)
{
	debug_fenter();

	loaded = FALSE;

	debug_fleave();

	return MM_ERROR_NONE;
}
