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
#include <mm_types.h>
#include <mm_error.h>
#include <mm_message.h>
#include "mm_streamrecorder_video.h"
#include "mm_streamrecorder_util.h"
#include "mm_streamrecorder_buffer_manager.h"
#include "mm_streamrecorder_internal.h"
#include "mm_streamrecorder_gstdispatch.h"

/*---------------------------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal						|
---------------------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/

void _mmstreamrecorder_buffer_destroy(gpointer p_stream_buffer)
{
	_MMStreamRecorderMsgItem msg;
	GstStreamRecorderBuffer *stream_buffer = (GstStreamRecorderBuffer *) p_stream_buffer;

	msg.id = MM_MESSAGE_STREAMRECORDER_CONSUME_COMPLETE;
	msg.param.union_type = MM_MSG_UNION_CONSUME_RECORDER_BUFFER;
	msg.param.consumed_mediabuffer.consumed_buffer = stream_buffer->user_buffer;
	_mmstreamrecorder_send_message(stream_buffer->str_handle, &msg);
	free(stream_buffer);
	return;
}
