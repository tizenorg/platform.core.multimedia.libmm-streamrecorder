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
#include <math.h>
#include <mm_types.h>
#include <mm_error.h>
#include "mm_streamrecorder_audio.h"
#include "mm_streamrecorder_util.h"

/*---------------------------------------------------------------------------------------
|    LOCAL ARRAY DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/* Table for compatibility between audio codec and file format */
gboolean common_audiocodec_fileformat_compatibility_table[MM_AUDIO_CODEC_NUM][MM_FILE_FORMAT_NUM] = {
	/* 3GP ASF AVI MATROSKA MP4 OGG NUT QT REAL AMR AAC MP3 AIFF AU WAV MID MMF DIVX FLV VOB IMELODY WMA WMV JPG */
	 /*AMR*/ {1, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G723.1*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*MP3*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*OGG*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*AAC*/ {1, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*WMA*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*MMF*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*ADPCM*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*WAVE*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*WAVE_NEW*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*MIDI*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*IMELODY*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*MXMF*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*MPA*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*MP2*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G711*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G722*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G722.1*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G722.2*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G723*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G726*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G728*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G729*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G729A*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*G729.1*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*REAL*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*AAC_LC*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*AAC_MAIN*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*AAC_SRS*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*AAC_LTP*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*AAC_HE_V1*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*AAC_HE_V2*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*AC3*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*ALAC*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*ATRAC*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*SPEEX*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*VORBIS*/ {0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*AIFF*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*AU*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*NONE*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*PCM*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*ALAW*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
	 /*MULAW*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
/*MS_ADPCM*/ {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,}
	,
};

int _mmstreamrecorder_check_audiocodec_fileformat_compatibility(unsigned int audio_codec, unsigned int file_format)
{

	/* Check compatibility between audio codec and file format */
	if (audio_codec < MM_AUDIO_CODEC_NUM && file_format < MM_FILE_FORMAT_NUM) {
		if (common_audiocodec_fileformat_compatibility_table[audio_codec][file_format] == 0) {
			_mmstreamrec_dbg_err("Audio codec[%d] and file format[%d] compatibility FAILED.", audio_codec, file_format);
			return MM_ERROR_STREAMRECORDER_ENCODER_WRONG_TYPE;
		}
		_mmstreamrec_dbg_log("Audio codec[%d] and file format[%d] compatibility SUCCESS.", audio_codec, file_format);
	} else {
		_mmstreamrec_dbg_err("Audio codec[%d] or file format[%d] is INVALID.", audio_codec, file_format);
		return MM_ERROR_STREAMRECORDER_ENCODER_WRONG_TYPE;
	}

	return MM_ERROR_NONE;
}

long double _mmstreamrecorder_get_decibel(unsigned char *raw, int size, int format)
{
#define MAX_AMPLITUDE_MEAN_16BIT 23170.115738161934
#define MAX_AMPLITUDE_MEAN_08BIT    89.803909382810

	int i = 0;
	int depthByte = 0;
	int count = 0;

	short *pcm16 = 0;
	char *pcm8 = 0;

	float db = 0.0;
	float rms = 0.0;
	unsigned long long square_sum = 0;

	if (format == 2)			/* MM_STREAMRECORDER_AUDIO_FORMAT_PCM_S16_LE) */
		depthByte = 2;
	else						/* MM_STREAMRECORDER_AUDIO_FORMAT_PCM_U8 */
		depthByte = 1;

	for (; i < size; i += (depthByte << 1)) {
		if (depthByte == 1) {
			pcm8 = (char *)(raw + i);
			square_sum += (*pcm8) * (*pcm8);
		} else {				/* 2byte */
			pcm16 = (short *)(raw + i);
			square_sum += (*pcm16) * (*pcm16);
		}

		count++;
	}
	if (count == 0)
		return 0;

	rms = sqrtf((float)((float)square_sum / count));

	if (depthByte == 1)
		db = 20 * log10(rms / MAX_AMPLITUDE_MEAN_08BIT);
	else
		db = 20 * log10(rms / MAX_AMPLITUDE_MEAN_16BIT);

	_mmstreamrec_dbg_log("size[%d],depthByte[%d],count[%d],rms[%f],db[%f]", size, depthByte, count, rms, db);

	return db;
}

int _mmstreamrecorder_get_amrnb_bitrate_mode(int bitrate)
{
	int result = MM_STREAMRECORDER_MR475;

	if (bitrate < 5150)
		result = MM_STREAMRECORDER_MR475;	/*AMR475 */
	else if (bitrate < 5900)
		result = MM_STREAMRECORDER_MR515;	/*AMR515 */
	else if (bitrate < 6700)
		result = MM_STREAMRECORDER_MR59;	/*AMR59 */
	else if (bitrate < 7400)
		result = MM_STREAMRECORDER_MR67;	/*AMR67 */
	else if (bitrate < 7950)
		result = MM_STREAMRECORDER_MR74;	/*AMR74 */
	else if (bitrate < 10200)
		result = MM_STREAMRECORDER_MR795;	/*AMR795 */
	else if (bitrate < 12200)
		result = MM_STREAMRECORDER_MR102;	/*AMR102 */
	else
		result = MM_STREAMRECORDER_MR122;	/*AMR122 */

	return result;
}
