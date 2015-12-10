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

#ifndef __MM_STREAMRECORDER_BUFFER_MANAGER_H__
#define __MM_STREAMRECORDER_BUFFER_MANAGER_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/
#include <glib.h>
#include <gst/gst.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| GLOBAL DEFINITIONS AND DECLARATIONS FOR STREAMRECORDER					|
========================================================================================*/

/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/

typedef struct _GstStreamRecorderBuffer GstStreamRecorderBuffer;

struct _GstStreamRecorderBuffer {
	GstBuffer *buffer;
	MMHandleType str_handle;
	void *user_buffer;
};

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
void _mmstreamrecorder_buffer_destroy(gpointer stream_buffer);

#ifdef __cplusplus
}
#endif
#endif							/* __MM_STREAMRECORDER_BUFFER_MANAGER_H__ */
