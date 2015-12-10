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

#ifndef __MM_STREAMRECORDER_AUDIO_H__
#define __MM_STREAMRECORDER_AUDIO_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/
#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/
/**
 * MMStreamRecorder information for audio mode
 */
typedef struct {
	int iAudioEncoder;		/**<Encoder */
	int iSamplingRate;	/**< Sampling rate */
	int iBitDepth;		/**< Bit depth */
	int iBitrate;		/**< audio bitrate */
	int iChannels;		/**< audio channels */
	gboolean b_commiting;	/**< Is it commiting now? */
	gboolean bMuxing;	/**< whether muxing */
	guint64 filesize;	/**< current recorded file size */
	guint64 max_size;
	guint64 max_time;	/**< max recording time */
	int audio_encode_depth;
	guint64 audio_frame_count; /**< current audio frame */
} _MMStreamRecorderAudioInfo;

/**
* Enumerations for AMR bitrate
*/
typedef enum _MMStreamRecorderAMRBitRate {
	MM_STREAMRECORDER_MR475,/**< MR475 : 4.75 kbit/s */
	MM_STREAMRECORDER_MR515,/**< MR515 : 5.15 kbit/s */
	MM_STREAMRECORDER_MR59,
						/**< MR59 : 5.90 kbit/s */
	MM_STREAMRECORDER_MR67,
						/**< MR67 : 6.70 kbit/s */
	MM_STREAMRECORDER_MR74,
						/**< MR74 : 7.40 kbit/s */
	MM_STREAMRECORDER_MR795,/**< MR795 : 7.95 kbit/s */
	MM_STREAMRECORDER_MR102,/**< MR102 : 10.20 kbit/s */
	MM_STREAMRECORDER_MR122,/**< MR122 : 12.20 kbit/s */
	MM_STREAMRECORDER_MRDTX
						/**< MRDTX */
} MMStreamRecorderAMRBitRate;

/*=======================================================================================
| GLOBAL FUNCTION PROTOTYPES								|
========================================================================================*/

int _mmstreamrecorder_check_audiocodec_fileformat_compatibility(unsigned int audio_codec, unsigned int file_format);

int _mmstreamrecorder_get_amrnb_bitrate_mode(int bitrate);

#ifdef __cplusplus
}
#endif
#endif /* __MM_STREAMRECORDER_AUDIO_H__ */
