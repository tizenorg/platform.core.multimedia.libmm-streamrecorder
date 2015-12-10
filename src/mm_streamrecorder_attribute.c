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
#include <mm_error.h>

#include "mm_streamrecorder_util.h"
#include "mm_streamrecorder_attribute.h"
#include "mm_streamrecorder_gstdispatch.h"

/*-----------------------------------------------------------------------
|    MACRO DEFINITIONS:							|
-----------------------------------------------------------------------*/
#define MM_STREAMRECORDER_ATTRIBUTE_MAX_VALUE  15
#define MM_STREAMRECORDER_RESOLUTION_MAX_VALUE 4080

/*---------------------------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/* basic attributes' info */
mm_streamrecorder_attr_construct_info stream_attrs_const_info[] = {
	/* 0 */
	{
	 MM_STR_VIDEO_BUFFER_TYPE,	/* ID */
	 "videobuffer-type",		/* Name */
	 MMF_VALUE_TYPE_INT,		/* Type */
	 MM_ATTRS_FLAG_RW,			/* Flag */
	 {(void *)MM_STREAMRECORDER_VIDEO_TYPE_TBM_BO},	/* Default value */
	 MM_ATTRS_VALID_TYPE_INT_RANGE,	/* Validity type */
	 MM_STREAMRECORDER_VIDEO_TYPE_TBM_BO,	/* Validity val1 (min, *array,...) */
	 MM_STREAMRECORDER_VIDEO_TYPE_NORMAL_BUFFER,	/* Validity val2 (max, count, ...) */
	 NULL,						/* Runtime setting function of the attribute */
	 },
	/* 1  */
	{MM_STR_VIDEO_FORMAT,
	 "videosource-format",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)MM_STREAMRECORDER_INPUT_FORMAT_NV12},
	 MM_ATTRS_VALID_TYPE_INT_RANGE,
	 MM_STREAMRECORDER_INPUT_FORMAT_INVALID,
	 MM_STREAMRECORDER_INPUT_FORMAT_NUM,
	 NULL,
	 },
	/* 2  */
	{
	 MM_STR_VIDEO_FRAMERATE,
	 "video-framerate",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_RANGE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 NULL,
	 },
	/* 3  */
	{
	 MM_STR_VIDEO_ENCODER_BITRATE,
	 "video-bitrate",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_RANGE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 NULL,	  /*_mmstreamrecorder_commit_video_bitrate, */
	 },
	/* 4  */
	{
	 MM_STR_VIDEO_RESOLUTION_WIDTH,
	 "video-resolution-width",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_ARRAY,
	 0,
	 0,
	 NULL,
	 },
	/* 5  */
	{
	 MM_STR_VIDEO_RESOLUTION_HEIGHT,
	 "video-resolution-height",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_ARRAY,
	 0,
	 0,
	 NULL,
	 },
	/* 6  */
	{
	 MM_STR_AUDIO_FORMAT,
	 "audio-source-format",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)MM_STREAMRECORDER_AUDIO_FORMAT_PCM_S16_LE},
	 MMF_VALUE_TYPE_INT,
	 MM_STREAMRECORDER_AUDIO_FORMAT_PCM_U8,
	 MM_STREAMRECORDER_AUDIO_FORMAT_PCM_S16_LE,
	 _mmstreamrecorder_commit_audio_bitformat,
	 },
	/* 7  */
	{
	 MM_STR_AUDIO_ENCODER_BITRATE,
	 "audio-bitrate",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)128000},
	 MM_ATTRS_VALID_TYPE_INT_RANGE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 _mmstreamrecorder_commit_audio_bitrate,
	 },
	/* 8  */
	{
	 MM_STR_AUDIO_SAMPLERATE,
	 "audio-samplerate",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_RANGE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 _mmstreamrecorder_commit_audio_samplingrate,
	 },
	/* 9  */
	{
	 MM_STR_VIDEO_ENCODER,
	 "video-encoder",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_ARRAY,
	 0,
	 0,
	 _mmstreamrecorder_commit_video_encoder,
	 },
	/* 10  */
	{
	 MM_STR_AUDIO_ENCODER,
	 "audio-encoder",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_ARRAY,
	 0,
	 0,
	 _mmstreamrecorder_commit_audio_encoder,
	 },
	/* 11  */
	{
	 MM_STR_AUDIO_CHENNEL_COUNT,
	 "audio-channel-count",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)2},
	 MM_ATTRS_VALID_TYPE_INT_RANGE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 _mmstreamrecorder_commit_audio_channel,
	 },
	/* 12  */
	{
	 MM_STR_FILE_FORMAT,
	 "file-format",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_ARRAY,
	 0,
	 0,
	 NULL,
	 },
	/* 13  */
	{
	 MM_STR_TARGET_FILE_NAME,
	 "filename",
	 MMF_VALUE_TYPE_STRING,
	 MM_ATTRS_FLAG_RW,
	 {NULL},
	 MM_ATTRS_VALID_TYPE_NONE,
	 0,
	 0,
	 NULL,
	 },
	/* 14  */
	{
	 MM_STR_VIDEO_ENABLE,
	 "video-enable",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)FALSE},
	 MM_ATTRS_VALID_TYPE_NONE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 _mmstreamrecorder_commit_video_enable,
	 },
	/* 15  */
	{
	 MM_STR_AUDIO_ENABLE,
	 "audio-enable",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)FALSE},
	 MM_ATTRS_VALID_TYPE_NONE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 _mmstreamrecorder_commit_audio_enable,
	 },
	/* 16 */
	{
	 MM_STR_MODE,
	 "recorder-mode",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)MM_STREAMRECORDER_MODE_MEDIABUFFER},
	 MM_ATTRS_VALID_TYPE_NONE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 NULL,
	 },
	/*17*/
	{
	 MM_STR_TARGET_MAX_SIZE,
	 "target-max-size",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_RANGE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 NULL,
	},
	/*18*/
	{
	 MM_STR_TARGET_TIME_LIMIT,
	 "target-time-limit",
	 MMF_VALUE_TYPE_INT,
	 MM_ATTRS_FLAG_RW,
	 {(void *)0},
	 MM_ATTRS_VALID_TYPE_INT_RANGE,
	 0,
	 _MMSTREAMRECORDER_MAX_INT,
	 NULL,
	},

};

/*-----------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:						|
-----------------------------------------------------------------------*/
/* STATIC INTERNAL FUNCTION */
static int __mmstreamrecorder_set_conf_to_valid_info(MMHandleType handle)
{
	int *format = NULL;
	int total_count = 0;

	/* Video width */
	total_count = _mmstreamrecorder_get_available_format(handle, KEYWORD_VIDEO_WIDTH, &format);
	stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_WIDTH].validity_value1 = (int)format;
	stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_WIDTH].validity_value2 = (int)total_count;

	/* Video height */
	total_count = _mmstreamrecorder_get_available_format(handle, KEYWORD_VIDEO_HEIGHT,  &format);
	stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_HEIGHT].validity_value1 = (int)format;
	stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_HEIGHT].validity_value2 = (int)total_count;

	/* Audio encoder */
	total_count = _mmstreamrecorder_get_available_format(handle, KEYWORD_AUDIO_ENCODERS, &format);
	stream_attrs_const_info[MM_STR_AUDIO_ENCODER].validity_value1 = (int)format;
	stream_attrs_const_info[MM_STR_AUDIO_ENCODER].validity_value2 = (int)total_count;

	/*Video encoder*/
	total_count = _mmstreamrecorder_get_available_format(handle, KEYWORD_VIDEO_ENCODERS, &format);
	stream_attrs_const_info[MM_STR_VIDEO_ENCODER].validity_value1 = (int)format;
	stream_attrs_const_info[MM_STR_VIDEO_ENCODER].validity_value2 = (int)total_count;

	/* File Format */
	total_count = _mmstreamrecorder_get_available_format(handle, KEYWORD_FILE_FORMATS, &format);
	stream_attrs_const_info[MM_STR_FILE_FORMAT].validity_value1 = (int)format;
	stream_attrs_const_info[MM_STR_FILE_FORMAT].validity_value2 = (int)total_count;

	return MM_ERROR_NONE;
}

static int __mmstreamrecorder_release_conf_valid_info(MMHandleType handle)
{
	int *allocated_memory = NULL;

	_mmstreamrec_dbg_log("START");

	allocated_memory = (int *)(stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_WIDTH].validity_value1);
	if (allocated_memory) {
		free(allocated_memory);
		stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_WIDTH].validity_value1 = (int)NULL;
		stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_WIDTH].validity_value2 = (int)0;
	}

	allocated_memory = (int *)(stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_HEIGHT].validity_value1);
	if (allocated_memory) {
		free(allocated_memory);
		stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_HEIGHT].validity_value1 = (int)NULL;
		stream_attrs_const_info[MM_STR_VIDEO_RESOLUTION_HEIGHT].validity_value2 = (int)0;
	}

	allocated_memory = (int *)(stream_attrs_const_info[MM_STR_AUDIO_ENCODER].validity_value1);
	if (allocated_memory) {
		free(allocated_memory);
		stream_attrs_const_info[MM_STR_AUDIO_ENCODER].validity_value1 = (int)NULL;
		stream_attrs_const_info[MM_STR_AUDIO_ENCODER].validity_value2 = (int)0;
	}

	allocated_memory = (int *)(stream_attrs_const_info[MM_STR_VIDEO_ENCODER].validity_value1);
	if (allocated_memory) {
		free(allocated_memory);
		stream_attrs_const_info[MM_STR_VIDEO_ENCODER].validity_value1 = (int)NULL;
		stream_attrs_const_info[MM_STR_VIDEO_ENCODER].validity_value2 = (int)0;
	}

	allocated_memory = (int *)(stream_attrs_const_info[MM_STR_FILE_FORMAT].validity_value1);
	if (allocated_memory) {
		free(allocated_memory);
		stream_attrs_const_info[MM_STR_FILE_FORMAT].validity_value1 = (int)NULL;
		stream_attrs_const_info[MM_STR_FILE_FORMAT].validity_value2 = (int)0;
	}
	_mmstreamrec_dbg_log("DONE");

	return MM_ERROR_NONE;
}

#if 0
static bool __mmstreamrecorder_attrs_is_supported(MMHandleType handle, int idx)
{
	mmf_attrs_t *attr = (mmf_attrs_t *) handle;
	int flag;

	if (mm_attrs_get_flags(handle, idx, &flag) == MM_ERROR_NONE) {
		if (flag == MM_ATTRS_FLAG_NONE)
			return FALSE;
	} else {
		return FALSE;
	}

	if (attr->items[idx].value_spec.type == MM_ATTRS_VALID_TYPE_INT_RANGE) {
		int min, max;
		mm_attrs_get_valid_range((MMHandleType) attr, idx, &min, &max);
		if (max < min)
			return FALSE;
	} else if (attr->items[idx].value_spec.type == MM_ATTRS_VALID_TYPE_INT_ARRAY) {
		int count;
		int *array;
		mm_attrs_get_valid_array((MMHandleType) attr, idx, &count, &array);
		if (count == 0)
			return FALSE;
	}

	return TRUE;
}
#endif
static int __mmstreamrecorder_check_valid_pair(MMHandleType handle, char **err_attr_name, const char *attribute_name, va_list var_args)
{
#define INIT_VALUE            -1
#define CHECK_COUNT           2
#define CAPTURE_RESOLUTION    1

	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	MMHandleType attrs = 0;

	int ret = MM_ERROR_NONE;
	int i = 0, j = 0;
	const char *name = NULL;
	const char *check_pair_name[2][3] = {
		{MMSTR_VIDEO_RESOLUTION_WIDTH, MMSTR_VIDEO_RESOLUTION_HEIGHT, "MMSTR_VIDEO_RESOLUTION_WIDTH and HEIGHT"},
		{NULL, NULL, NULL}
	};

	int check_pair_value[2][2] = {
		{INIT_VALUE, INIT_VALUE},
		{INIT_VALUE, INIT_VALUE},
	};

	if (hstreamrecorder == NULL || attribute_name == NULL) {
		_mmstreamrec_dbg_warn("handle[%p] or attribute_name[%p] is NULL.", hstreamrecorder, attribute_name);
		return MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT;
	}

	if (err_attr_name)
		*err_attr_name = NULL;

	/* _mmstreamrec_dbg_log( "ENTER" ); */

	attrs = MMF_STREAMRECORDER_ATTRS(handle);

	name = attribute_name;

	while (name) {
		int idx = -1;
		MMAttrsType attr_type = MM_ATTRS_TYPE_INVALID;

		/*_mmstreamrec_dbg_log( "NAME : %s", name );*/

		/* attribute name check */
		if ((ret = mm_attrs_get_index(attrs, name, &idx)) != MM_ERROR_NONE) {
			if (err_attr_name)
				*err_attr_name = strdup(name);

			if (ret == (int)MM_ERROR_COMMON_OUT_OF_ARRAY)
				return MM_ERROR_COMMON_ATTR_NOT_EXIST;
			else
				return ret;
		}

		/* type check */
		if ((ret = mm_attrs_get_type(attrs, idx, &attr_type)) != MM_ERROR_NONE)
			return ret;

		switch (attr_type) {
		case MM_ATTRS_TYPE_INT:
			{
				va_arg((var_args), int);
				break;
			}
		case MM_ATTRS_TYPE_DOUBLE:
			va_arg((var_args), double);
			break;
		case MM_ATTRS_TYPE_STRING:
			va_arg((var_args), char *);	/* string */
			va_arg((var_args), int);	/* size */
			break;
		case MM_ATTRS_TYPE_DATA:
			va_arg((var_args), void *);	/* data */
			va_arg((var_args), int);	/* size */
			break;
		case MM_ATTRS_TYPE_INVALID:
		default:
			_mmstreamrec_dbg_err("Not supported attribute type(%d, name:%s)", attr_type, name);
			if (err_attr_name)
				*err_attr_name = strdup(name);
			return MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT;
		}

		/* next name */
		name = va_arg(var_args, char *);
	}

	for (i = 0; i < CHECK_COUNT; i++) {
		if (check_pair_value[i][0] != INIT_VALUE || check_pair_value[i][1] != INIT_VALUE) {
			gboolean check_result = FALSE;
			char *err_name = NULL;
			MMStreamRecorderAttrsInfo attr_info_0, attr_info_1;

			if (check_pair_value[i][0] == INIT_VALUE) {
				mm_attrs_get_int_by_name(attrs, check_pair_name[i][0], &check_pair_value[i][0]);
				err_name = strdup(check_pair_name[i][1]);
			} else if (check_pair_value[i][1] == INIT_VALUE) {
				mm_attrs_get_int_by_name(attrs, check_pair_name[i][1], &check_pair_value[i][1]);
				err_name = strdup(check_pair_name[i][0]);
			} else {
				err_name = strdup(check_pair_name[i][2]);
			}

			mm_streamrecorder_get_attribute_info(handle, check_pair_name[i][0], &attr_info_0);
			mm_streamrecorder_get_attribute_info(handle, check_pair_name[i][1], &attr_info_1);

			check_result = FALSE;

			for (j = 0; j < attr_info_0.int_array.count; j++) {
				if (attr_info_0.int_array.array[j] == check_pair_value[i][0]
					&& attr_info_1.int_array.array[j] == check_pair_value[i][1]) {
					_mmstreamrec_dbg_log("Valid Pair[%s,%s] existed %dx%d[index:%d]", check_pair_name[i][0], check_pair_name[i][1], check_pair_value[i][0], check_pair_value[i][1], i);
					check_result = TRUE;
					break;
				}
			}

			if (check_result == FALSE) {
				_mmstreamrec_dbg_err("INVALID pair[%s,%s] %dx%d", check_pair_name[i][0], check_pair_name[i][1], check_pair_value[i][0], check_pair_value[i][1]);
				if (err_attr_name)
					*err_attr_name = err_name;

				return MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT;
			}

			if (err_name) {
				free(err_name);
				err_name = NULL;
			}
		}
	}

	/*_mmstreamrec_dbg_log("DONE");*/

	return MM_ERROR_NONE;
}

/* attribute commiter */
void __mmstreamrecorder_print_attrs(const char *attr_name, const mmf_value_t * value, const char *cmt_way)
{
	switch (value->type) {
	case MMF_VALUE_TYPE_INT:
		_mmstreamrec_dbg_log("%s :(%s:%d)", cmt_way, attr_name, value->value.i_val);
		break;
	case MMF_VALUE_TYPE_DOUBLE:
		_mmstreamrec_dbg_log("%s :(%s:%f)", cmt_way, attr_name, value->value.d_val);
		break;
	case MMF_VALUE_TYPE_STRING:
		_mmstreamrec_dbg_log("%s :(%s:%s)", cmt_way, attr_name, value->value.s_val);
		break;
	case MMF_VALUE_TYPE_DATA:
		_mmstreamrec_dbg_log("%s :(%s:%p)", cmt_way, attr_name, value->value.p_val);
		break;
	default:
		break;
	}

	return;
}

int _mmstreamrecorder_get_audio_codec_format(MMHandleType handle, const char *name)
{
	int codec_index = MM_AUDIO_CODEC_INVALID;

	if (!name) {
		_mmstreamrec_dbg_err("name is NULL");
		return MM_AUDIO_CODEC_INVALID;
	}

	if (!strcmp(name, "AMR"))
		codec_index = MM_AUDIO_CODEC_AMR;
	else if (!strcmp(name, "AAC"))
		codec_index = MM_AUDIO_CODEC_AAC;
	else if (!strcmp(name, "PCM"))
		codec_index = MM_AUDIO_CODEC_PCM;
	else if (!strcmp(name, "VORBIS"))
		codec_index = MM_AUDIO_CODEC_VORBIS;

	return codec_index;
}

int _mmstreamrecorder_get_video_codec_format(MMHandleType handle, const char *name)
{
	int codec_index = MM_VIDEO_CODEC_INVALID;

	if (!name) {
		_mmstreamrec_dbg_err("name is NULL");
		return MM_VIDEO_CODEC_INVALID;
	}

	if (!strcmp(name, "H263"))
		codec_index = MM_VIDEO_CODEC_H263;
	else if (!strcmp(name, "H264"))
		codec_index = MM_VIDEO_CODEC_H264;
	else if (!strcmp(name, "MPEG4"))
		codec_index = MM_VIDEO_CODEC_MPEG4;

	return codec_index;
}


int _mmstreamrecorder_get_mux_format(MMHandleType handle, const char *name)
{
	int mux_index = MM_FILE_FORMAT_INVALID;

	if (!name) {
		_mmstreamrec_dbg_err("name is NULL");
		return MM_FILE_FORMAT_INVALID;
	}

	if (!strcmp(name, "3GP"))
		mux_index = MM_FILE_FORMAT_3GP;
	else if (!strcmp(name, "AMR"))
		mux_index = MM_FILE_FORMAT_AMR;
	else if (!strcmp(name, "MP4"))
		mux_index = MM_FILE_FORMAT_MP4;
	else if (!strcmp(name, "WAV"))
		mux_index = MM_FILE_FORMAT_WAV;
	return mux_index;
}


int _mmstreamrecorder_get_format(MMHandleType handle , int category, const char *name)
{
	int fmt = -1;

	mmf_return_val_if_fail(name, -1);

	switch (category) {
	case KEYWORD_AUDIO_ENCODERS:
		fmt = _mmstreamrecorder_get_audio_codec_format(handle, name);
		break;
	case KEYWORD_VIDEO_ENCODERS:
		fmt = _mmstreamrecorder_get_video_codec_format(handle, name);
		break;
	case KEYWORD_FILE_FORMATS:
		fmt = _mmstreamrecorder_get_mux_format(handle, name);
		break;
	default:
		break;
	}
	return fmt;
}

int _mmstreamrecorder_get_type_count(MMHandleType handle, int type)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	int count = 0;
	int i = 0;
	if (type == KEYWORD_VIDEO_WIDTH) {
		while (hstreamrecorder->ini.supported_video_width[i++])
			count++;
	} else if (type == KEYWORD_VIDEO_HEIGHT) {
		while (hstreamrecorder->ini.supported_video_height[i++])
			count++;
	} else if (type == KEYWORD_AUDIO_ENCODERS) {
		while (hstreamrecorder->ini.supported_audio_encoders[i++][0])
			count++;
	} else if (type == KEYWORD_VIDEO_ENCODERS) {
		while (hstreamrecorder->ini.supported_video_encoders[i++][0])
			count++;
	} else if (type == KEYWORD_FILE_FORMATS) {
		while (hstreamrecorder->ini.supported_file_formats[i++][0])
			count++;
	}
	return count;
}

void __mmstreamrecorder_get_supported_name(MMHandleType handle, int type , char **str, int i)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	if (type == KEYWORD_AUDIO_ENCODERS) {
		*str = hstreamrecorder->ini.supported_audio_encoders[i];
		return;
	} else if (type == KEYWORD_VIDEO_ENCODERS) {
		*str = hstreamrecorder->ini.supported_video_encoders[i];
		return;
	} else if (type == KEYWORD_FILE_FORMATS) {
		*str = hstreamrecorder->ini.supported_file_formats[i];
		return;
	} else {
		*str = NULL;
		return;
	}
}

int __mmstreamrecorder_get_supported_value(MMHandleType handle, int type, int i)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);

	if (type == KEYWORD_VIDEO_WIDTH)
		return hstreamrecorder->ini.supported_video_width[i];
	else if (type == KEYWORD_VIDEO_HEIGHT)
		return hstreamrecorder->ini.supported_video_height[i];
	else
		return -1;
}

int
_mmstreamrecorder_get_available_format(MMHandleType handle, int type, int ** format)
{
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	int *arr = NULL;
	int total_count = 0;
	int count = 0;
	int i = 0;
	int fmt = 0;
	const char *name = NULL;

	mmf_return_val_if_fail(hstreamrecorder, 0);

	count = _mmstreamrecorder_get_type_count(handle, type);

	if (count <= 0)
		return -1;

	arr = (int*) g_malloc0(count * sizeof(int));
	if (arr == NULL) {
		_mmstreamrec_dbg_err("malloc failed : %d", count * sizeof(int));
		return -1;
	}

	if (type == KEYWORD_VIDEO_WIDTH || type == KEYWORD_VIDEO_HEIGHT) {
		for (i = 0 ; i < count ; i++) {
			fmt = __mmstreamrecorder_get_supported_value(handle, type, i);
			if (fmt >= 0)
				arr[total_count++] = fmt;
		}
	} else {
		for (i = 0 ; i < count ; i++) {
			__mmstreamrecorder_get_supported_name(handle, type, &name, i);
			fmt = _mmstreamrecorder_get_format(handle, type, name);
			if (fmt >= 0)
				arr[total_count++] = fmt;
		}
	}
	*format = arr;
	return total_count;
}

/*=======================================================================
|  FUNCTION DEFINITIONS							|
=======================================================================*/
/*-----------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:					|
-----------------------------------------------------------------------*/
MMHandleType _mmstreamrecorder_alloc_attribute(MMHandleType handle)
{
	_mmstreamrec_dbg_log("");

	MMHandleType attrs = 0;
	mmf_attrs_construct_info_t *attrs_const_info = NULL;
	unsigned int attr_count = 0;
	unsigned int idx;

	/* Create attribute constructor */
	_mmstreamrec_dbg_log("start");

	/* alloc 'mmf_attrs_construct_info_t' */
	attr_count = ARRAY_SIZE(stream_attrs_const_info);
	attrs_const_info = malloc(attr_count * sizeof(mmf_attrs_construct_info_t));

	if (!attrs_const_info) {
		_mmstreamrec_dbg_err("Fail to alloc constructor.");
		return 0;
	}

	for (idx = 0; idx < attr_count; idx++) {
		/* attribute order check. This should be same. */
		if (idx != stream_attrs_const_info[idx].attrid) {
			_mmstreamrec_dbg_err("Please check attributes order. Is the idx same with enum val?");
			free(attrs_const_info);
			attrs_const_info = NULL;
			return 0;
		}

		attrs_const_info[idx].name = stream_attrs_const_info[idx].name;
		attrs_const_info[idx].value_type = stream_attrs_const_info[idx].value_type;
		attrs_const_info[idx].flags = stream_attrs_const_info[idx].flags;
		attrs_const_info[idx].default_value = stream_attrs_const_info[idx].default_value.value_void;
	}

	_mmstreamrec_dbg_log("Create Streamrecorder Attributes[%p, %d]", attrs_const_info, attr_count);

	attrs = mmf_attrs_new_from_data("Streamrecorder_Attributes", attrs_const_info, attr_count, _mmstreamrecorder_commit_streamrecorder_attrs, (void *)handle);

	free(attrs_const_info);
	attrs_const_info = NULL;

	if (attrs == 0) {
		_mmstreamrec_dbg_err("Fail to alloc attribute handle");
		return 0;
	}

	__mmstreamrecorder_set_conf_to_valid_info(handle);

	for (idx = 0; idx < attr_count; idx++) {
		mmf_attrs_set_valid_type(attrs, idx, stream_attrs_const_info[idx].validity_type);

		switch (stream_attrs_const_info[idx].validity_type) {
		case MM_ATTRS_VALID_TYPE_INT_ARRAY:
			if (stream_attrs_const_info[idx].validity_value1 && stream_attrs_const_info[idx].validity_value2 > 0)
				mmf_attrs_set_valid_array(attrs, idx, (const int *)(stream_attrs_const_info[idx].validity_value1), stream_attrs_const_info[idx].validity_value2, (int)(stream_attrs_const_info[idx].default_value.value_int));
			break;
		case MM_ATTRS_VALID_TYPE_INT_RANGE:
			mmf_attrs_set_valid_range(attrs, idx, stream_attrs_const_info[idx].validity_value1, stream_attrs_const_info[idx].validity_value2, (int)(stream_attrs_const_info[idx].default_value.value_int));
			break;
		case MM_ATTRS_VALID_TYPE_DOUBLE_ARRAY:
			if (stream_attrs_const_info[idx].validity_value1 && stream_attrs_const_info[idx].validity_value2 > 0)
				mmf_attrs_set_valid_double_array(attrs, idx, (const double *)(stream_attrs_const_info[idx].validity_value1), stream_attrs_const_info[idx].validity_value2, (double)(stream_attrs_const_info[idx].default_value.value_double));
			break;
		case MM_ATTRS_VALID_TYPE_DOUBLE_RANGE:
			mmf_attrs_set_valid_double_range(attrs, idx, (double)(stream_attrs_const_info[idx].validity_value1), (double)(stream_attrs_const_info[idx].validity_value2), (double)(stream_attrs_const_info[idx].default_value.value_double));
			break;
		case MM_ATTRS_VALID_TYPE_NONE:
			break;
		case MM_ATTRS_VALID_TYPE_INVALID:
		default:
			_mmstreamrec_dbg_err("Valid type error.");
			break;
		}
	}

	__mmstreamrecorder_release_conf_valid_info(handle);

	return attrs;
}

void _mmstreamrecorder_dealloc_attribute(MMHandleType attrs)
{
	_mmstreamrec_dbg_log("");

	if (attrs) {
		mmf_attrs_free(attrs);

		_mmstreamrec_dbg_log("released attribute");
	}
}

int _mmstreamrecorder_get_attributes(MMHandleType handle, char **err_attr_name, const char *attribute_name, va_list var_args)
{
	MMHandleType attrs = 0;
	int ret = MM_ERROR_NONE;

	mmf_return_val_if_fail(handle, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	attrs = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attrs, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	ret = mm_attrs_get_valist(attrs, err_attr_name, attribute_name, var_args);

	return ret;
}

int _mmstreamrecorder_set_attributes(MMHandleType handle, char **err_attr_name, const char *attribute_name, va_list var_args)
{
	MMHandleType attrs = 0;
	int ret = MM_ERROR_NONE;

	mmf_return_val_if_fail(handle, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	attrs = MMF_STREAMRECORDER_ATTRS(handle);
	if (!attrs) {
		_mmstreamrec_dbg_err("handle 0x%x, attrs is NULL, attr name [%s]", handle, attribute_name);
		return MM_ERROR_STREAMRECORDER_NOT_INITIALIZED;
	}

	ret = __mmstreamrecorder_check_valid_pair(handle, err_attr_name, attribute_name, var_args);

	_mmstreamrec_dbg_err("__mmstreamrecorder_check_valid_pair handle 0x%x, attr name [%s] , ret = %d", handle, attribute_name, ret);

	if (ret == MM_ERROR_NONE)
		ret = mm_attrs_set_valist(attrs, err_attr_name, attribute_name, var_args);

	_mmstreamrec_dbg_err("mm_attrs_set_valist handle 0x%x, attr name [%s] , ret = %d", handle, attribute_name, ret);
	return ret;
}

int _mmstreamrecorder_get_attribute_info(MMHandleType handle, const char *attr_name, MMStreamRecorderAttrsInfo * info)
{
	MMHandleType attrs = 0;
	MMAttrsInfo attrinfo;
	int ret = MM_ERROR_NONE;

	mmf_return_val_if_fail(handle, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);
	mmf_return_val_if_fail(attr_name, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);
	mmf_return_val_if_fail(info, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	attrs = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attrs, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	ret = mm_attrs_get_info_by_name(attrs, attr_name, (MMAttrsInfo *) & attrinfo);

	if (ret == MM_ERROR_NONE) {
		memset(info, 0x00, sizeof(MMStreamRecorderAttrsInfo));
		info->type = attrinfo.type;
		info->flag = attrinfo.flag;
		info->validity_type = attrinfo.validity_type;

		switch (attrinfo.validity_type) {
		case MM_ATTRS_VALID_TYPE_INT_ARRAY:
			info->int_array.array = attrinfo.int_array.array;
			info->int_array.count = attrinfo.int_array.count;
			info->int_array.def = attrinfo.int_array.dval;
			break;
		case MM_ATTRS_VALID_TYPE_INT_RANGE:
			info->int_range.min = attrinfo.int_range.min;
			info->int_range.max = attrinfo.int_range.max;
			info->int_range.def = attrinfo.int_range.dval;
			break;
		case MM_ATTRS_VALID_TYPE_DOUBLE_ARRAY:
			info->double_array.array = attrinfo.double_array.array;
			info->double_array.count = attrinfo.double_array.count;
			info->double_array.def = attrinfo.double_array.dval;
			break;
		case MM_ATTRS_VALID_TYPE_DOUBLE_RANGE:
			info->double_range.min = attrinfo.double_range.min;
			info->double_range.max = attrinfo.double_range.max;
			info->double_range.def = attrinfo.double_range.dval;
			break;
		case MM_ATTRS_VALID_TYPE_NONE:
			break;
		case MM_ATTRS_VALID_TYPE_INVALID:
		default:
			break;
		}
	}

	return ret;
}

bool _mmstreamrecorder_commit_streamrecorder_attrs(int attr_idx, const char *attr_name, const mmf_value_t * value, void *commit_param)
{
	bool bret = FALSE;

	mmf_return_val_if_fail(commit_param, FALSE);
	mmf_return_val_if_fail(attr_idx >= 0, FALSE);
	mmf_return_val_if_fail(attr_name, FALSE);
	mmf_return_val_if_fail(value, FALSE);

	if (stream_attrs_const_info[attr_idx].attr_commit) {
		/* _mmstreamrec_dbg_log("Dynamic commit:(%s)", attr_name); */
		__mmstreamrecorder_print_attrs(attr_name, value, "Dynamic");
		bret = stream_attrs_const_info[attr_idx].attr_commit((MMHandleType) commit_param, attr_idx, value);
	} else {
		/* _mmstreamrec_dbg_log("Static commit:(%s)", attr_name); */
		__mmstreamrecorder_print_attrs(attr_name, value, "Static");
		bret = TRUE;
	}

	return bret;
}

bool _mmstreamrecorder_commit_video_enable(MMHandleType handle, int attr_idx, const mmf_value_t *value)
{
	MMHandleType attr = 0;
	/* mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle); */
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderVideoInfo *info = NULL;

	mmf_return_val_if_fail(handle, FALSE);
	attr = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attr, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);

	_mmstreamrec_dbg_log("(%d)", attr_idx);

	mmf_return_val_if_fail(sc, FALSE);

	info = (_MMStreamRecorderVideoInfo *) sc->info_video;

	if (value->value.i_val) {

	} else {
		if (info == NULL) {
			if (_mmstreamrecorder_alloc_subcontext_videoinfo(handle))
				return FALSE;
		}
	}

	return TRUE;
}

bool _mmstreamrecorder_commit_video_encoder(MMHandleType handle, int attr_idx, const mmf_value_t *value)
{
	MMHandleType attr = 0;
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderVideoInfo *info = NULL;

	mmf_return_val_if_fail(handle, FALSE);
	attr = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attr, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);

	info = (_MMStreamRecorderVideoInfo *) sc->info_video;
	mmf_return_val_if_fail(info, FALSE);

	_mmstreamrec_dbg_log("(%d)", attr_idx);

	info->iVideoEncoder = value->value.i_val;

	return TRUE;
}

bool _mmstreamrecorder_commit_audio_enable(MMHandleType handle, int attr_idx, const mmf_value_t *value)
{
	MMHandleType attr = 0;
	/* mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle); */
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;

	mmf_return_val_if_fail(handle, FALSE);
	attr = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attr, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);

	_mmstreamrec_dbg_log("(%d)", attr_idx);

	info = (_MMStreamRecorderAudioInfo *) sc->info_audio;

	if (value->value.i_val) {

	} else {
		if (info == NULL) {
			if (_mmstreamrecorder_alloc_subcontext_audioinfo(handle))
				return FALSE;
		}
	}

	return TRUE;
}

bool _mmstreamrecorder_commit_audio_encoder(MMHandleType handle, int attr_idx, const mmf_value_t *value)
{
	MMHandleType attr = 0;
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;

	mmf_return_val_if_fail(handle, FALSE);
	attr = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attr, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);

	info = (_MMStreamRecorderAudioInfo *) sc->info_audio;
	mmf_return_val_if_fail(info, FALSE);

	_mmstreamrec_dbg_log("(%d)", attr_idx);

	info->iAudioEncoder = value->value.i_val;

	return TRUE;
}

bool _mmstreamrecorder_commit_audio_samplingrate(MMHandleType handle, int attr_idx, const mmf_value_t *value)
{
	MMHandleType attr = 0;
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;

	mmf_return_val_if_fail(handle, FALSE);
	attr = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attr, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);

	info = (_MMStreamRecorderAudioInfo *) sc->info_audio;
	mmf_return_val_if_fail(info, FALSE);

	_mmstreamrec_dbg_log("(%d)", attr_idx);

	info->iSamplingRate = value->value.i_val;

	return TRUE;
}

bool _mmstreamrecorder_commit_audio_bitformat(MMHandleType handle, int attr_idx, const mmf_value_t *value)
{
	MMHandleType attr = 0;
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;

	mmf_return_val_if_fail(handle, FALSE);
	attr = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attr, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);

	info = (_MMStreamRecorderAudioInfo *) sc->info_audio;
	mmf_return_val_if_fail(info, FALSE);

	_mmstreamrec_dbg_log("(%d)", attr_idx);

	if (value->value.i_val == MM_STREAMRECORDER_AUDIO_FORMAT_PCM_U8)
		info->audio_encode_depth = 16;
	else
		info->audio_encode_depth = 8;

	return TRUE;
}

bool _mmstreamrecorder_commit_video_bitrate(MMHandleType handle, int attr_idx, const mmf_value_t *value)
{
	MMHandleType attr = 0;
	_MMStreamRecorderSubContext *sc = NULL;
	int v_bitrate;

	mmf_return_val_if_fail(handle, FALSE);
	attr = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attr, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);

	v_bitrate = value->value.i_val;

	if (sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst) {
		MMSTREAMRECORDER_G_OBJECT_SET(sc->encode_element[_MMSTREAMRECORDER_ENCSINK_VENC].gst, "bitrate", v_bitrate);
		_mmstreamrec_dbg_log("video bitrate set to encoder success = %d", v_bitrate);
	} else {
		_mmstreamrec_dbg_log("_MMSTREAMRECORDER_ENCSINK_VENC is null %d", attr_idx);
	}
	return TRUE;
}

bool _mmstreamrecorder_commit_audio_bitrate(MMHandleType handle, int attr_idx, const mmf_value_t *value)
{
	MMHandleType attr = 0;
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;

	mmf_return_val_if_fail(handle, FALSE);
	attr = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attr, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);

	info = (_MMStreamRecorderAudioInfo *) sc->info_audio;
	mmf_return_val_if_fail(info, FALSE);

	_mmstreamrec_dbg_log("(%d)", attr_idx);

	info->iBitrate = value->value.i_val;

	return TRUE;
}

bool _mmstreamrecorder_commit_audio_channel(MMHandleType handle, int attr_idx, const mmf_value_t *value)
{
	MMHandleType attr = 0;
	_MMStreamRecorderSubContext *sc = NULL;
	_MMStreamRecorderAudioInfo *info = NULL;

	mmf_return_val_if_fail(handle, FALSE);
	attr = MMF_STREAMRECORDER_ATTRS(handle);
	mmf_return_val_if_fail(attr, FALSE);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);
	mmf_return_val_if_fail(sc, FALSE);

	info = (_MMStreamRecorderAudioInfo *) sc->info_audio;
	mmf_return_val_if_fail(info, FALSE);

	_mmstreamrec_dbg_log("(%d)", attr_idx);

	info->iChannels = value->value.i_val;

	return TRUE;
}

int mm_streamrecorder_get_attribute_info(MMHandleType streamrecorder, const char *attribute_name, MMStreamRecorderAttrsInfo * info)
{
	return _mmstreamrecorder_get_attribute_info(streamrecorder, attribute_name, info);
}
