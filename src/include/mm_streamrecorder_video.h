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

#ifndef __MM_STREAMRECORDER_VIDEO_H__
#define __MM_STREAMRECORDER_VIDEO_H__

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
 * StreamRecorder VideoInfo information for video mode
 */
typedef struct {
	int iVideoEncoder;		/**< Video Encoder */
	gboolean b_commiting;	/**< Is it commiting now? */
	guint64 video_frame_count;
							/**< current video frame */
	guint64 filesize;	/**< current file size */
	guint64 max_size;	/**< max recording size */
	guint64 max_time;	/**< max recording time */
} _MMStreamRecorderVideoInfo;

/*=======================================================================================
| GLOBAL FUNCTION PROTOTYPES								|
========================================================================================*/

int _mmstreamrecorder_check_videocodec_fileformat_compatibility(unsigned int video_codec, unsigned int file_format);

#ifdef __cplusplus
}
#endif
#endif							/* __MM_STREAMRECORDER_VIDEO_H__ */
