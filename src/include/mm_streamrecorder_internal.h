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

#ifndef __MM_STREAMRECORDER_INTERNAL_H__
#define __MM_STREAMRECORDER_INTERNAL_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#include <mm_types.h>
#include <mm_attrs.h>
#include <mm_attrs_private.h>
#include <mm_message.h>

#include "mm_streamrecorder.h"
#include <gst/gst.h>

/* streamrecorder sub module */
#include "mm_streamrecorder_util.h"
#include "mm_streamrecorder_video.h"
#include "mm_streamrecorder_audio.h"
#include "mm_streamrecorder_fileinfo.h"
#include "mm_streamrecorder_gstcommon.h"
#include "mm_streamrecorder_ini.h"
#include "mm_streamrecorder_buffer_manager.h"

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| MACRO DEFINITIONS									|
========================================================================================*/

#ifndef ARRAY_SIZE
/**
 *	Macro for getting array size
 */
#define ARRAY_SIZE(a) (sizeof((a)) / sizeof((a)[0]))
#endif

#define	_MMSTREAMRECORDER_STATE_SET_COUNT		3	/* checking interval */
#define	_MMSTREAMRECORDER_STATE_CHECK_INTERVAL	1000	//5000      /* checking interval */

/**
 *	Functions related with LOCK and WAIT
 */
#define _MMSTREAMRECORDER_CAST_MTSAFE(handle)				(((mmf_streamrecorder_t*)handle)->mtsafe)
#define _MMSTREAMRECORDER_LOCK_FUNC(mutex)				pthread_mutex_lock(&mutex)
#define _MMSTREAMRECORDER_TRYLOCK_FUNC(mutex)			(!pthread_mutex_trylock(&mutex))
#define _MMSTREAMRECORDER_UNLOCK_FUNC(mutex)				pthread_mutex_unlock(&mutex)

#define _MMSTREAMRECORDER_GET_LOCK(handle)					(_MMSTREAMRECORDER_CAST_MTSAFE(handle).lock)
#define _MMSTREAMRECORDER_LOCK(handle)						_MMSTREAMRECORDER_LOCK_FUNC(_MMSTREAMRECORDER_GET_LOCK(handle))
#define _MMSTREAMRECORDER_TRYLOCK(handle)					_MMSTREAMRECORDER_TRYLOCK_FUNC(_MMSTREAMRECORDER_GET_LOCK(handle))
#define _MMSTREAMRECORDER_UNLOCK(handle)					_MMSTREAMRECORDER_UNLOCK_FUNC(_MMSTREAMRECORDER_GET_LOCK(handle))

#define _MMSTREAMRECORDER_GET_COND(handle)					(_MMSTREAMRECORDER_CAST_MTSAFE(handle).cond)
#define _MMSTREAMRECORDER_WAIT(handle)						pthread_cond_wait(&_MMSTREAMRECORDER_GET_COND(handle), _MMSTREAMRECORDER_GET_LOCK(handle))
#define _MMSTREAMRECORDER_TIMED_WAIT(handle, timeout)		pthread_cond_timedwait(&_MMSTREAMRECORDER_GET_COND(handle), _MMSTREAMRECORDER_GET_LOCK(handle),&timeout)

/* for command */
#define _MMSTREAMRECORDER_GET_CMD_LOCK(handle)					(_MMSTREAMRECORDER_CAST_MTSAFE(handle).cmd_lock)
#define _MMSTREAMRECORDER_LOCK_CMD(handle)						_MMSTREAMRECORDER_LOCK_FUNC(_MMSTREAMRECORDER_GET_CMD_LOCK(handle))
#define _MMSTREAMRECORDER_TRYLOCK_CMD(handle)					_MMSTREAMRECORDER_TRYLOCK_FUNC(_MMSTREAMRECORDER_GET_CMD_LOCK(handle))
#define _MMSTREAMRECORDER_UNLOCK_CMD(handle)						_MMSTREAMRECORDER_UNLOCK_FUNC(_MMSTREAMRECORDER_GET_CMD_LOCK(handle))

/* for state change */
#define _MMSTREAMRECORDER_GET_STATE_LOCK(handle)					(_MMSTREAMRECORDER_CAST_MTSAFE(handle).state_lock)
#define _MMSTREAMRECORDER_LOCK_STATE(handle)						_MMSTREAMRECORDER_LOCK_FUNC(_MMSTREAMRECORDER_GET_STATE_LOCK(handle))
#define _MMSTREAMRECORDER_TRYLOCK_STATE(handle)					_MMSTREAMRECORDER_TRYLOCK_FUNC(_MMSTREAMRECORDER_GET_STATE_LOCK(handle))
#define _MMSTREAMRECORDER_UNLOCK_STATE(handle)						_MMSTREAMRECORDER_UNLOCK_FUNC(_MMSTREAMRECORDER_GET_STATE_LOCK(handle))

/* for gstreamer state change */
#define _MMSTREAMRECORDER_GET_GST_STATE_LOCK(handle)					(_MMSTREAMRECORDER_CAST_MTSAFE(handle).gst_state_lock)
#define _MMSTREAMRECORDER_LOCK_GST_STATE(handle)						_MMSTREAMRECORDER_LOCK_FUNC(_MMSTREAMRECORDER_GET_GST_STATE_LOCK(handle))
#define _MMSTREAMRECORDER_TRYLOCK_GST_STATE(handle)					_MMSTREAMRECORDER_TRYLOCK_FUNC(_MMSTREAMRECORDER_GET_GST_STATE_LOCK(handle))
#define _MMSTREAMRECORDER_UNLOCK_GST_STATE(handle)						_MMSTREAMRECORDER_UNLOCK_FUNC(_MMSTREAMRECORDER_GET_GST_STATE_LOCK(handle))

/* for setting/calling callback */
#define _MMSTREAMRECORDER_GET_MESSAGE_CALLBACK_LOCK(handle)      (_MMSTREAMRECORDER_CAST_MTSAFE(handle).message_cb_lock)
#define _MMSTREAMRECORDER_LOCK_MESSAGE_CALLBACK(handle)          _MMSTREAMRECORDER_LOCK_FUNC(_MMSTREAMRECORDER_GET_MESSAGE_CALLBACK_LOCK(handle))
#define _MMSTREAMRECORDER_TRYLOCK_MESSAGE_CALLBACK(handle)       _MMSTREAMRECORDER_TRYLOCK_FUNC(_MMSTREAMRECORDER_GET_MESSAGE_CALLBACK_LOCK(handle))
#define _MMSTREAMRECORDER_UNLOCK_MESSAGE_CALLBACK(handle)        _MMSTREAMRECORDER_UNLOCK_FUNC(_MMSTREAMRECORDER_GET_MESSAGE_CALLBACK_LOCK(handle))

/**
 * Caster of main handle (streamrecorder)
 */
#define MMF_STREAMRECORDER(h) (mmf_streamrecorder_t *)(h)

/**
 * Caster of subcontext
 */
#define MMF_STREAMRECORDER_SUBCONTEXT(h) (((mmf_streamrecorder_t *)(h))->sub_context)

/* LOCAL CONSTANT DEFINITIONS */

/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/

/**
 * streamrecorder Pipeline's Element name.
 * @note index of element.
 */
typedef enum {
	_MMSTREAMRECORDER_ENCODE_NONE = (-1),

	/* Main Pipeline Element */
	_MMSTREAMRECORDER_ENCODE_MAIN_PIPE = 0x00,

	/* Pipeline element of Audio input */
	_MMSTREAMRECORDER_AUDIOSRC_BIN,
	_MMSTREAMRECORDER_AUDIOSRC_SRC,	/* appsrc */
	_MMSTREAMRECORDER_AUDIOSRC_FILT,

	/* Pipeline element of Encodebin */
	_MMSTREAMRECORDER_ENCSINK_BIN,
	_MMSTREAMRECORDER_ENCSINK_SRC,	/* video appsrc */
	_MMSTREAMRECORDER_ENCSINK_FILT,
	_MMSTREAMRECORDER_ENCSINK_ENCBIN,
	_MMSTREAMRECORDER_ENCSINK_AQUE,
	_MMSTREAMRECORDER_ENCSINK_CONV,
	_MMSTREAMRECORDER_ENCSINK_AENC,
	_MMSTREAMRECORDER_ENCSINK_AENC_QUE,
	_MMSTREAMRECORDER_ENCSINK_VQUE,
	_MMSTREAMRECORDER_ENCSINK_VCONV,
	_MMSTREAMRECORDER_ENCSINK_VENC,
	_MMSTREAMRECORDER_ENCSINK_VENC_QUE,
	_MMSTREAMRECORDER_ENCSINK_PARSER,
	_MMSTREAMRECORDER_ENCSINK_ITOG,
	_MMSTREAMRECORDER_ENCSINK_ICROP,
	_MMSTREAMRECORDER_ENCSINK_ISCALE,
	_MMSTREAMRECORDER_ENCSINK_IFILT,
	_MMSTREAMRECORDER_ENCSINK_IQUE,
	_MMSTREAMRECORDER_ENCSINK_IENC,
	_MMSTREAMRECORDER_ENCSINK_MUX,
	_MMSTREAMRECORDER_ENCSINK_SINK,

	_MMSTREAMRECORDER_ENCODE_PIPELINE_ELEMENT_NUM,
} _MMSTREAMRECORDER_ENCODE_PIPELINE_ELELMENT;

/**
 * Command type for streamrecorder.
 */

typedef enum {
	_MM_STREAMRECORDER_CMD_CREATE,
	_MM_STREAMRECORDER_CMD_DESTROY,
	_MM_STREAMRECORDER_CMD_REALIZE,
	_MM_STREAMRECORDER_CMD_UNREALIZE,
	_MM_STREAMRECORDER_CMD_START,
	_MM_STREAMRECORDER_CMD_STOP,
	_MM_STREAMRECORDER_CMD_RECORD,
	_MM_STREAMRECORDER_CMD_PAUSE,
	_MM_STREAMRECORDER_CMD_COMMIT,
	_MM_STREAMRECORDER_CMD_CANCEL,
	_MM_STREAMRECORDER_CMD_QUIT,
} _MMstreamrecorderCommandType;

/**
 * System state change cause
 */
typedef enum {
	_MMSTREAMRECORDER_STATE_CHANGE_NORMAL = 0,
} _MMstreamrecorderStateChange;


/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/

/**
 * MMstreamrecorder information for Multi-Thread Safe
 */
typedef struct {
	pthread_mutex_t lock;		/**< Mutex (for general use) */
	pthread_cond_t cond;		/**< Condition (for general use) */
	pthread_mutex_t cmd_lock;	/**< Mutex (for command) */
	pthread_mutex_t state_lock;	/**< Mutex (for state change) */
	pthread_mutex_t gst_state_lock;	/**< Mutex (for state change) */
	pthread_mutex_t message_cb_lock;/**< Mutex (for message callback) */
} _MMStreamRecorderMTSafe;

/**
 * MMstreamrecorder information for command loop
 */

/**
 * MMstreamrecorder Sub Context
 */
typedef struct {
	bool isMaxsizePausing;				/**< Because of size limit, pipeline is paused. */
	bool isMaxtimePausing;				/**< Because of time limit, pipeline is paused. */
	int encode_element_num;				/**< count of encode element */
	GstClockTime pipeline_time;			/**< current time of Gstreamer Pipeline */
	GstClockTime pause_time;			/**< amount of time while pipeline is in PAUSE state.*/
	gboolean error_occurs;				/**< flag for error */
	int error_code;						/**< error code for internal gstreamer error */
	gboolean ferror_send;				/**< file write/seek error **/
	guint ferror_count;					/**< file write/seek error count **/
	gboolean bget_eos;					/**< Whether getting EOS */
	gboolean video_enable;				/**< whether video is disabled or not when record */
	gboolean audio_enable;				/**< whether audio is disabled or not when record */

	/* INI information */
	unsigned int fourcc;				/**< Get fourcc value */
	_MMStreamRecorderVideoInfo *info_video;	 /**< extra information for video recording */
	_MMStreamRecorderAudioInfo *info_audio;	 /**< extra information for audio recording */
	_MMStreamRecorderFileInfo *info_file;  /**< extra information for audio recording */
	_MMStreamRecorderGstElement *encode_element;
											 /**< array of encode element */

//  type_element *VideosinkElement;         /**< configure data of videosink element */
} _MMStreamRecorderSubContext;

/**
  * _MMstreamrecorderContext
  */
typedef struct mmf_streamrecorder {
	/* information */
	//  int type;               /**< mmstreamrecorder_mode_type */
	int state;              /**< state of streamrecorder */
	//  int target_state;       /**< Target state that want to set. This is a flag that
	//                             * stands for async state changing. If this value differ from state,
	//                             * it means state is changing now asychronously. */

	/* handles */
	MMHandleType attributes;			   /**< Attribute handle */
	_MMStreamRecorderSubContext *sub_context;	/**< sub context */
	GList *buffer_probes;				   /**< a list of buffer probe handle */
	GList *event_probes;				   /**< a list of event probe handle */
	GList *data_probes;					   /**< a list of data probe handle */
	GList *signals;						   /**< a list of signal handle */
	GList *msg_data;					   /**< a list of msg data */
	guint pipeline_cb_event_id;			   /**< Event source ID of pipeline message callback */
	guint encode_pipeline_cb_event_id;	   /**< Event source ID of encode pipeline message callback */

	/* callback handlers */
	MMMessageCallback msg_cb;								/**< message callback */
	void *msg_cb_param;										/**< message callback parameter */
	int (*command) (MMHandleType, int);						/**< streamrecorder's command */

	/* etc */
	_MMStreamRecorderMTSafe mtsafe;								 /**< Thread safe */
	mm_streamrecorder_ini_t ini;

	int reserved[4];			/**< reserved */
} mmf_streamrecorder_t;

/*=======================================================================================
| EXTERN GLOBAL VARIABLE								|
========================================================================================*/

/*=======================================================================================
| GLOBAL FUNCTION PROTOTYPES								|
========================================================================================*/
/**
 *	This function creates streamrecorder for capturing still image and recording.
 *
 *	@param[out]	handle		Specifies the streamrecorder  handle
 *	@param[in]	info		Preset information of streamrecorder
 *	@return		This function returns zero on success, or negative value with error code.
 *	@remarks	When this function calls successfully, streamrecorder  handle will be filled with a @n
 *			valid value and the state of  the streamrecorder  will become MM_streamrecorder_STATE_NULL.@n
 *			Note that  it's not ready to working streamrecorder. @n
 *			You should call mmstreamrecorder_realize before starting streamrecorder.
 *	@see		_mmstreamrecorder_create
 */
int _mmstreamrecorder_create(MMHandleType *handle);

/**
*  This function gets the current state of streamreccorder.
*  mm_streamrecorderr is working on the base of its state. An user should check the state of mm_streamrecorder before calling its functions.
*  If the handle is avaiable, user can retrieve the value.
*
*  @param[in]  streamrecorder   A handle of streamrecorder.
*  @param[out] state       On return, it contains current state of streamrecorder.
*  @return     This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.
*          Please refer 'mm_error.h' to know the exact meaning of the error.
*  @see        MMStreamRecorderStateType
*  @pre        None
*  @post       None
*  @remarks    None
*/

MMStreamRecorderStateType _mmstreamrecorder_get_state(MMHandleType handle);

/**
 *	This function destroys instance of streamrecorder.
 *
 *	@param[in]	hstreamrecorder	Specifies the streamrecorder  handle
 *	@return		This function returns zero on success, or negative value with error code.
 *	@see		_mmstreamrecorder_create
 */
int _mmstreamrecorder_destroy(MMHandleType handle);

/**
 *	This function allocates memory for streamrecorder.
 *
 *	@param[in]	hstreamrecorder	Specifies the streamrecorder  handle
 *	@return		This function returns zero on success, or negative value with error code.
 *	@remarks	This function can  be called successfully when current state is MM_streamrecorder_STATE_NULL @n
 *			and  the state of the streamrecorder  will become MM_streamrecorder_STATE_READY. @n
 *			Otherwise, this function will return MM_ERROR_streamrecorder_INVALID_CONDITION.
 *	@see		_mmstreamrecorder_unrealize
 *	@pre		MM_streamrecorder_STATE_NULL
 *	@post		MM_streamrecorder_STATE_READY
 */
int _mmstreamrecorder_realize(MMHandleType hstreamrecorder);

/**
 *	This function free allocated memory for streamrecorder.
 *
 *	@param[in]	hstreamrecorder	Specifies the streamrecorder  handle
 *	@return		This function returns zero on success, or negative value with error code.
 *	@remarks	This function release all resources which are allocated for the streamrecorder engine.@n
 *			This function can  be called successfully when current state is MM_streamrecorder_STATE_READY and  @n
 *			the state of the streamrecorder  will become MM_streamrecorder_STATE_NULL. @n
 *			Otherwise, this function will return MM_ERROR_streamrecorder_INVALID_CONDITION.
 *	@see		_mmstreamrecorder_realize
 *	@pre		MM_streamrecorder_STATE_READY
 *	@post		MM_streamrecorder_STATE_NULL
 */
int _mmstreamrecorder_unrealize(MMHandleType hstreamrecorder);

/**
 *	This function is to start previewing.
 *
 *	@param[in]	hstreamrecorder	Specifies the streamrecorder  handle
 *	@return		This function returns zero on success, or negative value with error code.
 *	@remarks	This function can  be called successfully when current state is MM_streamrecorder_STATE_READY and  @n
 *			the state of the streamrecorder  will become MM_streamrecorder_STATE_PREPARE. @n
 *			Otherwise, this function will return MM_ERROR_streamrecorder_INVALID_CONDITION.
 *	@see		_mmstreamrecorder_stop
 */
int _mmstreamrecorder_record(MMHandleType hstreamrecorder);

int _mmstreamrecorder_push_stream_buffer(MMHandleType handle, MMStreamRecorderStreamType streamtype, unsigned long timestamp, void *buffer, int size);

/**
 *	This function is to pause video and audio recording
 *
 *	@param[in]	hstreamrecorder	Specifies the streamrecorder  handle
 *	@return		This function returns zero on success, or negative value with error code.
 *	@remarks	This function can  be called successfully when current state is MM_streamrecorder_STATE_RECORDING and  @n
 *			the  state of the streamrecorder  will become MM_streamrecorder_STATE_PAUSED.@n
 *			Otherwise, this function will return MM_ERROR_streamrecorder_INVALID_CONDITION.@n
 *	@see		_mmstreamrecorder_record
 */
int _mmstreamrecorder_pause(MMHandleType hstreamrecorder);

/**
 *	This function is to stop video and audio  recording and  save results.
 *
 *	@param[in]	hstreamrecorder	Specifies the streamrecorder  handle
 *	@return		This function returns zero on success, or negative value with error code.
 *	@remarks	This function can  be called successfully when current state is @n
 *			MM_streamrecorder_STATE_PAUSED or MM_streamrecorder_STATE_RECORDING and  @n
 *			the state of the streamrecorder  will become MM_streamrecorder_STATE_PREPARE. @n
 *			Otherwise, this function will return MM_ERROR_streamrecorder_INVALID_CONDITION
 *	@see		_mmstreamrecorder_cancel
 */
int _mmstreamrecorder_commit(MMHandleType hstreamrecorder);

/**
 *	This function is to stop video and audio recording and do not save results.
 *
 *	@param[in]	hstreamrecorder	Specifies the streamrecorder  handle
 *	@return		This function returns zero on success, or negative value with error code.
 *	@remarks	This function can  be called successfully when current state is @n
 *			MM_streamrecorder_STATE_PAUSED or MM_streamrecorder_STATE_RECORDING and  @n
 *			the state of the streamrecorder  will become MM_streamrecorder_STATE_PREPARE. @n
 *			Otherwise, this function will return MM_ERROR_streamrecorder_INVALID_CONDITION.
 *	@see		_mmstreamrecorder_commit
 */
int _mmstreamrecorder_cancel(MMHandleType hstreamrecorder);

/**
 *	This function is to set callback for receiving messages from streamrecorder.
 *
 *	@param[in]	hstreamrecorder	Specifies the streamrecorder  handle
 *	@param[in]	callback	Specifies the function pointer of callback function
 *	@param[in]	user_data	Specifies the user poiner for passing to callback function
 *
 *	@return		This function returns zero on success, or negative value with error code.
 *	@remarks	typedef bool (*mm_message_callback) (int msg, mm_messageType *param, void *user_param);@n
 *		@n
 *		typedef union 				@n
 *		{							@n
 *			int code;				@n
 *			struct 					@n
 *			{						@n
 *				int total;			@n
 *				int elapsed;		@n
 *			} time;					@n
 *			struct 					@n
 *			{						@n
 *				int previous;		@n
 *				int current;			@n
 *			} state;					@n
 *		} mm_message_type;	@n
 *									@n
 *		If a  message value for mm_message_callback is MM_MESSAGE_STATE_CHANGED, @n
 *		state value in mm_message_type  will be a mmstreamrecorder_state_type enum value;@n
 *		@n
 *		If  a message value for mm_message_callback is MM_MESSAGE_ERROR,  @n
 *		the code value in mm_message_type will be a mmplayer_error_type enum value;
 *
 *	@see		mm_message_type,  mmstreamrecorder_state_type,  mmstreamrecorder_error_type
 */
int _mmstreamrecorder_set_message_callback(MMHandleType hstreamrecorder, MMMessageCallback callback, void *user_data);

/**
 * This function allocates structure of subsidiary attributes.
 *
 * @param[in]	type		Allocation type of streamrecorder context.
 * @return	This function returns structure pointer on success, NULL value on failure.
 * @remarks
 * @see		_mmstreamrecorder_dealloc_subcontext()
 *
 */
int _mmstreamrecorder_alloc_subcontext(MMHandleType handle);

int _mmstreamrecorder_alloc_subcontext_videoinfo(MMHandleType handle);

int _mmstreamrecorder_alloc_subcontext_audioinfo(MMHandleType handle);

int _mmstreamrecorder_alloc_subcontext_fileinfo(MMHandleType handle);

/**
 * This function releases structure of subsidiary attributes.
 *
 * @param[in]	sc		Handle of streamrecorder subcontext.
 * @return	void
 * @remarks
 * @see		_mmstreamrecorder_alloc_subcontext()
 *
 */
void _mmstreamrecorder_dealloc_subcontext(MMHandleType handle);

/**
 * This function sets command function according to the type.
 *
 * @param[in]	handle		Handle of streamrecorder context.
 * @param[in]	type		Allocation type of streamrecorder context.
 * @return	This function returns MM_ERROR_NONE on success, or other values with error code.
 * @remarks
 * @see		__mmstreamrecorder_video_command(), __mmstreamrecorder_audio_command(), __mmstreamrecorder_image_command()
 *
 */
void _mmstreamrecorder_set_functions(MMHandleType handle);

void _mmstreamrecorder_unset_functions(MMHandleType handle);

#ifdef __cplusplus
}
#endif
#endif							/* __MM_streamrecorder_INTERNAL_H__ */
