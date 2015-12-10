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

#ifndef __MM_STREAMRECORDER_ATTRIBUTE_H__
#define __MM_STREAMRECORDER_ATTRIBUTE_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/
#include <mm_types.h>
#include <mm_attrs.h>
#include <mm_attrs_private.h>
#include <stdarg.h>
#include "mm_streamrecorder_internal.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| GLOBAL DEFINITIONS AND DECLARATIONS FOR STREAMRECORDER					|
========================================================================================*/
/* Disabled
#define GET_AND_STORE_ATTRS_AFTER_SCENE_MODE
*/

/*=======================================================================================
| MACRO DEFINITIONS									|
========================================================================================*/
/**
 * Caster of attributes handle
 */
#define MMF_STREAMRECORDER_ATTRS(h) (((mmf_streamrecorder_t *)(h))->attributes)

/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/
/**
 * Enumerations for streamrecorder attribute ID.
 */
typedef enum {
	MM_STR_VIDEO_BUFFER_TYPE,	/* 0 */
	MM_STR_VIDEO_SOURCE_FORMAT,
	MM_STR_VIDEO_FRAMERATE,
	MM_STR_VIDEO_BITRATE,
	MM_STR_VIDEO_RESOLUTION_WIDTH,
	MM_STR_VIDEO_RESOLUTION_HEIGHT,
	MM_STR_AUDIO_SOURCE_FORMAT,
	MM_STR_AUDIO_BITRATE,
	MM_STR_AUDIO_SAMPLERATE,
	MM_STR_VIDEO_ENCODER,
	MM_STR_AUDIO_ENCODER,	/* 10 */
	MM_STR_AUDIO_CHENNEL_COUNT,
	MM_STR_FILE_FORMAT,
	MM_STR_VIDEO_FILE_NAME,
	MM_STR_VIDEO_DISABLE,
	MM_STR_AUDIO_DISABLE,
	MM_STR_RECORDER_MODE,
	MM_STR_TARGET_MAX_SIZE,
	MM_STR_TARGET_TIME_LIMIT,
	MM_STR_NUM
} MMStreamRecorderAttrsID;


/*=======================================================================================
| TYPE DEFINITIONS									|
========================================================================================*/
typedef bool(*mmf_streamrecorder_commit_func_t) (MMHandleType handle, int attr_idx, const mmf_value_t * value);

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/
typedef struct {
	MMStreamRecorderAttrsID attrid;
	const char *name;
	int value_type;
	int flags;
	union {
		void *value_void;
		char *value_string;
		int value_int;
		double value_double;
	} default_value;		/* default value */
	MMStreamRecorderAttrsValidType validity_type;
	int validity_value1;	/* can be int min, int *array, double *array, or cast to double min. */
	int validity_value2;	/* can be int max, int count, int count, or cast to double max. */
	mmf_streamrecorder_commit_func_t attr_commit;
} mm_streamrecorder_attr_construct_info;

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
 * This function allocates structure of attributes and sets initial values.
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @param[in]	info		Preset information of streamrecorder.
 * @return	This function returns allocated structure of attributes.
 * @remarks
 * @see		_mmstreamrecorder_dealloc_attribute()
 *
 */
MMHandleType _mmstreamrecorder_alloc_attribute(MMHandleType handle);

/**
 * This function release structure of attributes.
 *
 * @param[in]	attrs		Handle of streamrecorder attribute.
 * @return	void
 * @remarks
 * @see		_mmstreamrecorder_alloc_attribute()
 *
 */
void _mmstreamrecorder_dealloc_attribute(MMHandleType attrs);

/**
 * This is a meta  function to get attributes of streamrecorder with given attribute names.
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @param[out]	err_attr_name	Specifies the name of attributes that made an error. If the function doesn't make an error, this will be null.
 * @param[in]	attribute_name	attribute name that user want to get.
 * @param[in]	var_args	Specifies variable arguments.
 * @return	This function returns MM_ERROR_NONE on Success, minus on Failure.
 * @remarks	You can retrieve multiple attributes at the same time.  @n
 * @see		_mmstreamrecorder_set_attributes
 */
int _mmstreamrecorder_get_attributes(MMHandleType handle, char **err_attr_name, const char *attribute_name, va_list var_args);

/**
 * This is a meta  function to set attributes of steamrecorder with given attribute names.
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @param[out]	err_attr_name	Specifies the name of attributes that made an error. If the function doesn't make an error, this will be null.
 * @param[in]	attribute_name	attribute name that user want to set.
 * @param[in]	var_args	Specifies variable arguments.
 * @return	This function returns MM_ERROR_NONE on Success, minus on Failure.
 * @remarks	You can put multiple attributes to streamrecorder at the same time.  @n
 * @see		_mmstreamrecorder_get_attributes
 */
int _mmstreamrecorder_set_attributes(MMHandleType handle, char **err_attr_name, const char *attribute_name, va_list var_args);

/**
 * This is a meta  function to get detail information of the attribute.
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @param[in]	attr_name	attribute name that user want to get information.
 * @param[out]	info		a structure that holds information related with the attribute.
 * @return	This function returns MM_ERROR_NONE on Success, minus on Failure.
 * @remarks	If the function succeeds, 'info' holds detail information about the attribute, such as type, flag, validity_type, validity_values  @n
 * @see		_mmstreamrecorder_get_attributes, _mmstreamrecorder_set_attributes
 */
int _mmstreamrecorder_get_attribute_info(MMHandleType handle, const char *attr_name, MMStreamRecorderAttrsInfo * info);

/*=======================================================================================
| STREAMRECORDER INTERNAL LOCAL								|
========================================================================================*/
/**
 * A commit function to set streamrecorder attributes
 * If the attribute needs actual setting, this function handles that activity.
 * When application sets an attribute, setting function in MSL common calls this function.
 * If this function fails, original value will not change.
 *
 * @param[in]	attr_idx	Attribute index of subcategory.
 * @param[in]	attr_name	Attribute name.
 * @param[in]	value		Handle of streamrecorder.
 * @param[in]	commit_param	Allocation type of streamrecorder context.
 * @return	This function returns TRUE on success, or FALSE on failure
 * @remarks
 * @see
 *
 */
bool _mmstreamrecorder_commit_streamrecorder_attrs(int attr_idx, const char *attr_name, const mmf_value_t * value, void *commit_param);

bool _mmstreamrecorder_commit_video_enable(MMHandleType handle, int attr_idx, const mmf_value_t * value);

bool _mmstreamrecorder_commit_video_encoder(MMHandleType handle, int attr_idx, const mmf_value_t * value);

bool _mmstreamrecorder_commit_audio_enable(MMHandleType handle, int attr_idx, const mmf_value_t * value);

bool _mmstreamrecorder_commit_audio_encoder(MMHandleType handle, int attr_idx, const mmf_value_t * value);

bool _mmstreamrecorder_commit_audio_samplingrate(MMHandleType handle, int attr_idx, const mmf_value_t * value);

bool _mmstreamrecorder_commit_audio_bitformat(MMHandleType handle, int attr_idx, const mmf_value_t * value);

bool _mmstreamrecorder_commit_audio_channel(MMHandleType handle, int attr_idx, const mmf_value_t * value);

bool _mmstreamrecorder_commit_audio_bitrate(MMHandleType handle, int attr_idx, const mmf_value_t * value);

/**
 * check whether supported or not
 *
 * @param[in]	handle		Handle of streamrecorder.
 * @param[in]	attr_index	index of attribute to check.
 * @return	bool		TRUE if supported or FALSE
 */
bool _mmstreamrecorder_check_supported_attribute(MMHandleType handle, int attr_index);

/**
 *    mm_streamrecorder_get_attribute_info:\n
 *  Get detail information of the attribute. To manager attributes, an user may want to know the exact character of the attribute,
 *  such as type, flag, and validity. This is the function to provide such information.
 *  Depending on the 'validity_type', validity union would be different. To know about the type of union, please refer 'MMStreamRecorderAttrsInfo'.
 *
 *	@param[in]	streamrecorder	Specifies the streamrecorder  handle.
 *	@param[in]	attribute_name	attribute name that user want to get information.
 *	@param[out]	info		a structure that holds information related with the attribute.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@remarks	If the function succeeds, 'info' holds detail information about the attribute, such as type,
 *			flag, validity_type, validity_values, and default values.
 *	@see		mm_streamrecorder_get_attributes, mm_streamrecorder_set_attributes
 */

int mm_streamrecorder_get_attribute_info(MMHandleType streamrecorder, const char *attribute_name, MMStreamRecorderAttrsInfo * info);

bool _mmstreamrecorder_commit_video_bitrate(MMHandleType handle, int attr_idx, const mmf_value_t * value);

#ifdef __cplusplus
}
#endif
#endif							/* __MM_STREAMRECORDER_ATTRIBUTE_H__ */
