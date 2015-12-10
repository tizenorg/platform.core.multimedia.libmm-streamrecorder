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

/* ==============================================================================
|  INCLUDE FILES								|
===============================================================================*/
#include <stdio.h>
#include <string.h>

#include <mm_error.h>

#include <mm_attrs_private.h>
#include "mm_streamrecorder.h"
#include "mm_streamrecorder_internal.h"
#include "mm_streamrecorder_attribute.h"

/*===============================================================================
|  FUNCTION DEFINITIONS								|
===============================================================================*/
/*-------------------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:						|
-------------------------------------------------------------------------------*/
int mm_streamrecorder_create(MMHandleType *streamrecorder)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_log("");

	error = _mmstreamrecorder_create(streamrecorder);

	_mmstreamrec_dbg_log("END");

	return error;
}


int mm_streamrecorder_get_state(MMHandleType streamrecorder, MMStreamRecorderStateType *status)
{
	int ret = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	mmf_return_val_if_fail((void *)status, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_err("");

	*status = _mmstreamrecorder_get_state(streamrecorder);

	return ret;

}

int mm_streamrecorder_destroy(MMHandleType streamrecorder)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_log("");

	error = _mmstreamrecorder_destroy(streamrecorder);

	_mmstreamrec_dbg_log("END!!!");

	return error;
}

int mm_streamrecorder_realize(MMHandleType streamrecorder)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_log("");

	error = _mmstreamrecorder_realize(streamrecorder);

	_mmstreamrec_dbg_log("END");

	return error;
}

int mm_streamrecorder_pause(MMHandleType streamrecorder)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_err("");

	error = _mmstreamrecorder_pause(streamrecorder);

	_mmstreamrec_dbg_err("END");

	return error;
}


int mm_streamrecorder_unrealize(MMHandleType streamrecorder)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_log("");

	error = _mmstreamrecorder_unrealize(streamrecorder);

	_mmstreamrec_dbg_log("END");

	return error;
}

int mm_streamrecorder_record(MMHandleType streamrecorder)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_log("");

	error = _mmstreamrecorder_record(streamrecorder);

	_mmstreamrec_dbg_log("END");

	return error;
}

int mm_streamrecorder_push_stream_buffer(MMHandleType streamrecorder, MMStreamRecorderStreamType streamtype, unsigned long timestamp, void *buffer, int size)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_log("");

	error = _mmstreamrecorder_push_stream_buffer(streamrecorder, streamtype, timestamp, buffer, size);

	_mmstreamrec_dbg_log("END");

	return error;
}

int mm_streamrecorder_commit(MMHandleType streamrecorder)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_log("");

	error = _mmstreamrecorder_commit(streamrecorder);

	_mmstreamrec_dbg_log("END");

	return error;
}

int mm_streamrecorder_cancel(MMHandleType streamrecorder)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	_mmstreamrec_dbg_log("");

	error = _mmstreamrecorder_cancel(streamrecorder);

	_mmstreamrec_dbg_log("END");

	return error;
}

int mm_streamrecorder_set_message_callback(MMHandleType streamrecorder, MMMessageCallback callback, void *user_data)
{
	int error = MM_ERROR_NONE;

	mmf_return_val_if_fail((void *)streamrecorder, MM_ERROR_STREAMRECORDER_INVALID_ARGUMENT);

	error = _mmstreamrecorder_set_message_callback(streamrecorder, callback, user_data);

	return error;
}

int mm_streamrecorder_get_attributes(MMHandleType streamrecorder, char **err_attr_name, const char *attribute_name, ...)
{
	va_list var_args;
	int ret = MM_ERROR_NONE;

	return_val_if_fail(attribute_name, MM_ERROR_COMMON_INVALID_ARGUMENT);

	va_start(var_args, attribute_name);
	ret = _mmstreamrecorder_get_attributes(streamrecorder, err_attr_name, attribute_name, var_args);
	va_end(var_args);

	return ret;
}

int mm_streamrecorder_set_attributes(MMHandleType streamrecorder, char **err_attr_name, const char *attribute_name, ...)
{
	va_list var_args;
	int ret = MM_ERROR_NONE;

	return_val_if_fail(attribute_name, MM_ERROR_COMMON_INVALID_ARGUMENT);

	va_start(var_args, attribute_name);
	ret = _mmstreamrecorder_set_attributes(streamrecorder, err_attr_name, attribute_name, var_args);
	va_end(var_args);

	return ret;
}
