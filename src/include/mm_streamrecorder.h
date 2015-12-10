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

#ifndef __MM_STREAMRECORDER_H__
#define __MM_STREAMRECORDER_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/
#include <glib.h>

#include <mm_types.h>
#include <mm_error.h>
#include <mm_message.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| GLOBAL DEFINITIONS AND DECLARATIONS FOR STREAMRECORDER					|
========================================================================================*/

/*=======================================================================================
| MACRO DEFINITIONS									|
========================================================================================*/

/* Attributes Macros */

#define MMSTR_VIDEO_DISABLE                     "video-disable"

#define MMSTR_VIDEO_BUFFER_TYPE                          "videobuffer-type"

#define MMSTR_VIDEO_SOURCE_FORMAT                   "videosource-format"

#define MMSTR_VIDEO_FRAMERATE                              "video-framerate"

#define MMSTR_VIDEO_BITRATE                                    "video-bitrate"

#define MMSTR_VIDEO_RESOLUTION_WIDTH             "video-resolution-width"

#define MMSTR_VIDEO_RESOLUTION_HEIGHT            "video-resolution-height"

/**
 * Disable Audio stream when record.
 */
#define MMSTR_AUDIO_DISABLE                     "audio-disable"

#define MMSTR_AUDIO_SOURCE_FORMAT                    "audio-source-format"

#define MMSTR_AUDIO_BITRATE			"audio-bitrate"

#define MMSTR_AUDIO_SAMPLERATE		"audio-samplerate"

#define MMSTR_VIDEO_ENCODER			"video-encoder"

#define MMSTR_AUDIO_ENCODER			"audio-encoder"

#define MMSTR_AUDIO_CHANNEL			"audio-channel-count"

#define MMSTR_FILE_FORMAT				"file-format"

#define MMSTR_FILENAME         "filename"

#define MMSTR_RECORDER_MODE			"recorder-mode"

#define MMSTR_TARGET_MAX_SIZE       "target-max-size"

#define MMSTR_TARGET_TIME_LIMIT      "target-time-limit"

/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/
/**
 * An enumeration for streamrecorder states.
 */
typedef enum {
	MM_STREAMRECORDER_STATE_NONE,	/**< Streamrecorder is not created yet */
	MM_STREAMRECORDER_STATE_CREATED,	/**< Streamrecorder is created, but not initialized yet */
	MM_STREAMRECORDER_STATE_PREPARED,	/**< Streamrecorder is prepared to capture */
	MM_STREAMRECORDER_STATE_RECORDING,	/**< Streamrecorder is now recording */
	MM_STREAMRECORDER_STATE_PAUSED,	/**< Streamrecorder is paused while recording */
	MM_STREAMRECORDER_STATE_NUM,		/**< Number of streamrecorder states */
} MMStreamRecorderStateType;

/**
 * An enumeration of Audio Format.
 */
typedef enum {
	MM_STREAMRECORDER_AUDIO_FORMAT_PCM_U8 = 0,	/**< unsigned 8bit audio */
	MM_STREAMRECORDER_AUDIO_FORMAT_PCM_S16_LE = 2,
												/**< signed 16bit audio. Little endian. */
} MMStreamRecorderAudioFormat;

/**********************************
*          Stream data            *
**********************************/

typedef enum {
	MM_STREAM_TYPE_NONE,
	MM_STREAM_TYPE_VIDEO,
	MM_STREAM_TYPE_AUDIO,
	MM_STREAM_TYPE_DATA,
} MMStreamRecorderStreamType;

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/

/* General Structure */
/**
 * Structure for capture data.
 */
typedef struct {
	void *data;		/**< pointer of captured image */
	unsigned int length;	/**< length of captured image (in byte)*/
	MMPixelFormatType format;
							/**< image format */
	int width;		/**< width of captured image */
	int height;		/**< height of captured image */
	int encoder_type;	/**< encoder type */
} MMStreamRecorderCaptureDataType;


/**
 * An enumeration for streamrecorder mode.
 */

typedef enum {
	MM_STREAMRECORDER_MODE_MEDIABUFFER = 0,/**< Recording with mediabuffer */
	MM_STREAMRECORDER_MODE_SCREENRECORD,   /**< Recording with screenrecord */
} MMStreamRecorderModeType;

/**
 * An enumeration for stream data type.
 */
typedef enum {
	MM_STREAMRECORDER_STREAM_DATA_YUV420 = 0,	   /**< YUV420 Packed type - 1 plane */
	MM_STREAMRECORDER_STREAM_DATA_YUV422,		   /**< YUV422 Packed type - 1 plane */
	MM_STREAMRECORDER_STREAM_DATA_YUV420SP,		   /**< YUV420 SemiPlannar type - 2 planes */
	MM_STREAMRECORDER_STREAM_DATA_YUV420P,		   /**< YUV420 Plannar type - 3 planes */
	MM_STREAMRECORDER_STREAM_DATA_YUV422P,		   /**< YUV422 Plannar type - 3 planes */
} MMStreamRecorderStreamData;

typedef enum {
	MM_STREAMRECORDER_VIDEO_TYPE_GEM_HANDLE,
	MM_STREAMRECORDER_VIDEO_TYPE_NATIVE_BUFFER,
	MM_STREAMRECORDER_VIDEO_TYPE_TBM_BO,
	MM_STREAMRECORDER_VIDEO_TYPE_NORMAL_BUFFER
} MMStreamRecorder_video_buf_type_e;

typedef enum {
	MM_STREAMRECORDER_INPUT_FORMAT_INVALID = -1,/**< Invalid pixel format */
	MM_STREAMRECORDER_INPUT_FORMAT_NV12,	   /**< NV12 pixel format */
	MM_STREAMRECORDER_INPUT_FORMAT_NV21,	   /**< NV21 pixel format */
	MM_STREAMRECORDER_INPUT_FORMAT_I420,	   /**< I420 pixel format */
	MM_STREAMRECORDER_INPUT_FORMAT_UYVY,	   /**< UYVY pixel format */
	MM_STREAMRECORDER_INPUT_FORMAT_YUYV,	   /**< YUYV pixel format */
	MM_STREAMRECORDER_INPUT_FORMAT_BGRA,	   /**< RGBA pixel format */
	MM_STREAMRECORDER_INPUT_FORMAT_NUM			/**< Number of the pixel format */
} MMStreamRecorder_video_source_format_e;

/**********************************
*          Attribute info         *
**********************************/

/**
 * Structure for video stream data.
 */
typedef struct {
	union {
		struct {
			unsigned char *yuv;
			unsigned int length_yuv;
		} yuv420, yuv422;
		struct {
			unsigned char *y;
			unsigned int length_y;
			unsigned char *uv;
			unsigned int length_uv;
		} yuv420sp;
		struct {
			unsigned char *y;
			unsigned int length_y;
			unsigned char *u;
			unsigned int length_u;
			unsigned char *v;
			unsigned int length_v;
		} yuv420p, yuv422p;
	} data;						/**< pointer of captured stream */
	MMStreamRecorderStreamData data_type;  /**< data type */
	unsigned int length_total;	/**< total length of stream buffer (in byte)*/
	unsigned int num_planes;	/**< number of planes */
	MMPixelFormatType format;	/**< image format */
	int width;					/**< width of video buffer */
	int height;					/**< height of video buffer */
	unsigned int timestamp;		/**< timestamp of stream buffer (msec)*/
} MMVideoStreamDataType;

/**
 * Structure for audio stream data.
 */
typedef struct {
	void *data;			/**< pointer of captured stream */
	unsigned int length;		/**< length of stream buffer (in byte)*/
	MMStreamRecorderAudioFormat format;	/**< audio format */
	int channel;			/**< number of channel of the stream */
	unsigned int timestamp;		/**< timestamp of stream buffer (msec)*/
	float volume_dB;		/**< dB value of audio stream */
} MMAudioStreamDataType;

/**
 * Report structure of recording file
 */
typedef struct {
	char *recording_filename;	/**< File name of stored recording file. Please free after using. */
} MMStreamRecordingReport; /**< report structure definition of recording file */

/**
* An enumeration for attribute values types.
*/
typedef enum {
   MM_STR_REC_ATTRS_TYPE_INVALID = -1, /**< Type is invalid */
   MM_STR_REC_ATTRS_TYPE_INT,	   /**< Integer type attribute */
   MM_STR_REC_ATTRS_TYPE_DOUBLE,   /**< Double type attribute */
   MM_STR_REC_ATTRS_TYPE_STRING,   /**< UTF-8 String type attribute */
   MM_STR_REC_ATTRS_TYPE_DATA,	   /**< Pointer type attribute */
} MMStreamRecorderAttrsType;

/**
* An enumeration for attribute validation type.
*/
typedef enum {
   MM_STR_REC_ATTRS_VALID_TYPE_INVALID = -1,
										   /**< Invalid validation type */
   MM_STR_REC_ATTRS_VALID_TYPE_NONE,   /**< Do not check validity */
   MM_STR_REC_ATTRS_VALID_TYPE_INT_ARRAY,
									   /**< validity checking type of integer array */
   MM_STR_REC_ATTRS_VALID_TYPE_INT_RANGE,
									   /**< validity checking type of integer range */
   MM_STR_REC_ATTRS_VALID_TYPE_DOUBLE_ARRAY,
										   /**< validity checking type of double array */
   MM_STR_REC_ATTRS_VALID_TYPE_DOUBLE_RANGE,
										   /**< validity checking type of double range */
} MMStreamRecorderAttrsValidType;

/**
* An enumeration for attribute access flag.
*/
typedef enum {
   MM_STR_REC_ATTRS_FLAG_DISABLED = 0, /**< None flag is set. This means the attribute is not allowed to use.  */
   MM_STR_REC_ATTRS_FLAG_READABLE = 1 << 0,/**< Readable */
   MM_STR_REC_ATTRS_FLAG_WRITABLE = 1 << 1,/**< Writable */
   MM_STR_REC_ATTRS_FLAG_MODIFIED = 1 << 2,/**< Modified */
   MM_STR_REC_ATTRS_FLAG_RW = MM_STR_REC_ATTRS_FLAG_READABLE | MM_STR_REC_ATTRS_FLAG_WRITABLE,
																						   /**< Readable and Writable */
} MMStreamRecorderAttrsFlag;

/**
* A structure for attribute information
*/
typedef struct {
   MMStreamRecorderAttrsType type;
   MMStreamRecorderAttrsFlag flag;
   MMStreamRecorderAttrsValidType validity_type;

/**
* A union that describes validity of the attribute.
*/
   union {
   /**
	* Validity structure for integer array.
	*/
	   struct {
		   int *array; /**< a pointer of array */
		   int count;  /**< size of array */
		   int def;    /**< default value. Real value not index of array */
	   } int_array;

   /**
	* Validity structure for integer range.
	*/
	   struct {
		   int min;    /**< minimum range */
		   int max;    /**< maximum range */
		   int def;    /**< default value */
	   } int_range;

   /**
	* Validity structure for double array.
	*/
	   struct {
		   double *array;  /**< a pointer of array */
		   int count;  /**< size of array */
		   double def; /**< default value. Real value not index of array */
	   } double_array;

   /**
	* Validity structure for double range.
	*/
	   struct {
		   double min; /**< minimum range */
		   double max; /**< maximum range */
		   double def; /**< default value */
	   } double_range;
   };
} MMStreamRecorderAttrsInfo;


/*=======================================================================================
| TYPE DEFINITIONS									|
========================================================================================*/
/**
 *	Function definition for video stream callback.
 *  Be careful! In this function, you can't call functions that change the state of streamrecorder such as mm_streamrecorder_stop(), 
 *  mm_streamrecorder_unrealize(), mm_streamrecorder_record(), mm_streamrecorder_commit(), and mm_streamrecorder_cancel(), etc.
 *  Please don't hang this function long. It may cause low performance of preview or occur timeout error from video source. 
 *  Also, you're not allowed to call mm_streamrecorder_stop() even in other context, while you're hanging this function. 
 *  I recommend to you releasing this function ASAP.
 *
 *	@param[in]	stream			Reference pointer to video stream data
 *	@param[in]	user_param		User parameter which is received from user when callback function was set
 *	@return		This function returns true on success, or false on failure.
 *	@remarks		This function is issued in the context of gstreamer (video sink thread).
 */
typedef gboolean(*mm_streamrecorder_video_stream_callback) (MMVideoStreamDataType * stream, void *user_param);

/**
 *	Function definition for audio stream callback.
 *  Be careful! In this function, you can't call functions that change the state of streamrecorder such as mm_streamrecorder_stop(),
 *  mm_streamrecorder_unrealize(), mm_streamrecorder_record(), mm_streamrecorder_commit(), and mm_streamrecorder_cancel(), etc.
 *  Please don't hang this function long. It may cause low performance of streamrecorder or occur timeout error from audio source.
 *  I recommend to you releasing this function ASAP.
 *
 *	@param[in]	stream			Reference pointer to audio stream data
 *	@param[in]	user_param		User parameter which is received from user when callback function was set
 *	@return		This function returns true on success, or false on failure.
 *	@remarks
 */
typedef gboolean(*mm_streamrecorder_audio_stream_callback) (MMAudioStreamDataType * stream, void *user_param);

/**
 *	Function definition for video capture callback.
 *  Like '#mm_streamrecorder_video_stream_callback', you can't call mm_streamrecorder_stop() while you are hanging this function.
 *
 *	@param[in]	frame			Reference pointer to captured data
 *	@param[in]	thumbnail		Reference pointer to thumbnail data
 *	@param[in]	user_param		User parameter which is received from user when callback function was set
 *	@return		This function returns true on success, or false on failure.
 *	@remarks		This function is issued in the context of gstreamer (video src thread).
 */
typedef gboolean(*mm_streamrecorder_video_capture_callback) (MMStreamRecorderCaptureDataType * frame, MMStreamRecorderCaptureDataType * thumbnail, void *user_param);

/*=======================================================================================
| GLOBAL FUNCTION PROTOTYPES								|
========================================================================================*/
/**
 *    mm_streamrecorder_create:\n
 *  Create streamrecorder object. This is the function that an user who wants to use mm_streamrecorder calls first. 
 *  This function creates handle structure and initialize mutex, attributes, gstreamer.
 *  When this function success, it will return  a handle of newly created object. 
 *  A user have to put the handle when he calls every function of mm_streamrecorder. \n
 *
 *	@param[out]	streamrecorder	A handle of streamrecorder.
 *	@param[in]	info		Information for devices
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@see		mm_streamrecorder_destroy
 *	@pre		None
 *	@post		Next state of mm-streamrecorder will be MM_STREAMRECORDER_STATE_CREATED
 *	@remarks	You can create multiple handles on a context at the same time. However,
 *			streamrecordercorder cannot guarantee proper operation because of limitation of resources.
 *	@par example
 *	@code

#include <mm_streamrecorder.h>

gboolean initialize_streamrecorder()
{
	int err;

	err = mm_streamrecorder_create(&hstream);

	if (err != MM_ERROR_NONE) {
		return FALSE;
	}

	return TRUE;
}

 *	@endcode
 */

//INITIAL GSTREAMER
int mm_streamrecorder_create(MMHandleType * streamrecorder);

/**
 *    mm_streamrecorder_destroy:\n
 *  Destroy streamrecorder object. Release handle and all of the resources that were created in mm_streamrecorder_create().\n
 *  This is the finalizing function of mm_streamrecorder. If this function is not called or fails to call, the handle isn't released fully.
 *  This function releases attributes, mutexes, sessions, and handle itself. This function also removes all of remaining messages.
 *  So if your application should wait a certain message of mm_streamrecorder, please wait to call this function till getting the message.
 *	
 *
 *	@param[in]	streamrecorder	A handle of streamrecorder.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@see		mm_streamrecorder_create
 *	@pre		Previous state of mm-streamrecorder should be MM_STREAMRECORDER_STATE_CREATED
 *	@post		Because the handle is not valid, you can't check the state.
 *	@remarks	None
 *	@par example
 *	@code

#include <mm_streamrecorder.h>

gboolean destroy_streamrecorder()
{
	int err;

	//Destroy streamrecorder handle
	err = mm_streamrecorder_destroy(hstreamrecorder);
	if (err < 0) {
		return FALSE;
	}

	return TRUE;
}

 *	@endcode
 */

// DESTROY GSTREAMER
int mm_streamrecorder_destroy(MMHandleType streamrecorder);

/**
 *    mm_streamrecorder_realize:\n
 *  Allocate resources for streamrecorder and initialize it.
 *  This also creates streamer pipeline. So you have to set attributes that are pivotal to create
 *  the pipeline before calling this function. This function also takes a roll to manage confliction
 *  between different applications which use streamrecorder. For example, if you try to use streamrecorder when
 *  other application that is more important such as call application, this function will return
 *  'MM_ERROR_POLICY_BLOCKED'. On the contrary, if your application that uses streamrecorder starts to launch
 *  while another application that uses speaker and has lower priority, your application will kick
 *  another application.
 *
 *	@param[in]	streamrecorder	A handle of streamrecorder.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@see		mm_streamrecorder_unrealize
 *	@pre		Previous state of mm-streamrecorder should be MM_STREAMRECORDER_STATE_CREATED
 *	@post		Next state of mm-streamrecorder will be MM_STREAMRECORDER_STATE_READY
 *	@remarks	None
 */

// CONSTRUCT PIPLINE
int mm_streamrecorder_realize(MMHandleType streamrecorder);

/**
 *    mm_streamrecorder_unrealize:\n
 *  Uninitialize streamrecoder resources and free allocated memory.
 *  Most important resource that is released here is gstreamer pipeline of mm_streamrecorder.
 *  Because most of resources are operating on the gstreamer pipeline, 
 *  this function should be called to release its resources.
 *  Moreover, mm_streamrecorder is controlled by audio session manager. If an user doesn't call this function when he want to release mm_streamrecorder,
 *  other multimedia frameworks may face session problem. For more detail information, please refer mm_session module.
 *
 *	@param[in]	streamrecorder	A handle of streamrecorder.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@see		mm_streamrecorder_realize
 *	@pre		Previous state of mm-streamrecorder should be MM_STREAMRECORDER_STATE_READY
 *	@post		Next state of mm-streamrecorder will be MM_STREAMRECORDER_STATE_CREATED
 *	@remarks	None
 */

// DESTROY PIPELINE
int mm_streamrecorder_unrealize(MMHandleType streamrecorder);

/**
 *	mm_streamrecorder_start:\n
 *   Start previewing. (Image/Video mode)
 *  'mm_streamrecorder_video_stream_callback' is activated after calling this function.
 *
 *	@param[in]	streamrecorder	A handle of streamrecorder.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@see		mm_streamrecorder_stop
 *	@pre		Previous state of mm-streamrecorder should be MM_STREAMRECORDER_STATE_READY
 *	@post		Next state of mm-streamrecorder will be MM_STREAMRECORDER_STATE_PREPARED
 *	@remarks	None
 */

// START ENCODE
int mm_streamrecorder_record(MMHandleType streamrecorder);

/**
 *    mm_streamrecorder_pause:\n
 *  Pause A/V recording or Audio recording. (Audio/Video mode only)
 *
 *  @param[in]  streamrecorder   A handle of streamrecorder.
 *  @return     This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *          Please refer 'mm_error.h' to know the exact meaning of the error.
 *  @see        mm_streamrecorder_record
 *  @pre        Previous state of mm-streamrecorder should be MM_STREAMRECORDER_STATE_RECORDING
 *  @post       Next state of mm-streamrecorder will be MM_STREAMRECORDER_STATE_PAUSED
 *  @remarks    Even though this function is for pausing recording, small amount of buffers could be recorded after pause().
 *          Because the buffers which are existed in the queue were created before pause(), the buffers should be recorded.
 */
int mm_streamrecorder_pause(MMHandleType streamrecorder);

/**
 *    mm_streamrecorder_stop:\n
 *  Stop previewing. (Image/Video mode)
 *  This function will change the status of pipeline. If an application doesn't return callbacks
 *  of streamrecorder, this function can be locked. For example, if your application still
 *  holds '#mm_streamrecorder_video_capture_callback' or '#mm_streamrecorder_video_stream_callback',
 *  this function could be hung. So users have to return every callback before calling this function.
 *
 *	@param[in]	streamrecorder	A handle of streamrecorder.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@see		mm_streamrecorder_start
 *	@pre		Previous state of mm-streamrecorder should be MM_STREAMRECORDER_STATE_PREPARED
 *	@post		Next state of mm-streamrecorder will be MM_STREAMRECORDER_STATE_READY
 *	@remarks	None
 *	@par example
 *	@code

#include <mm_streamrecorder.h>

gboolean stop_streamrecorder()
{
	int err;

	//Stop preview
	err =  mm_streamrecorder_stop(hstreamrecorder);
	if (err < 0) {
		return FALSE;
	}

	return TRUE;
}

 *	@endcode
 */
int mm_streamrecorder_commit(MMHandleType streamrecorder);

int mm_streamrecorder_cancel(MMHandleType streamrecorder);

int mm_streamrecorder_push_stream_buffer(MMHandleType streamrecorder, MMStreamRecorderStreamType streamtype, unsigned long timestamp, void *buffer, int size);

/**
 *    mm_streamrecorder_commit:\n
 *  Stop recording and save results.  (Audio/Video mode only)\n
 *  After starting recording, encoded data frame will be stored in the location specified in MMSTR_FILENAME.
 *  Some encoder or muxer require a certain type of finalizing such as adding some information to header.
 *  This function takes that roll. So if you don't call this function after recording, the result file may not be playable.\n
 *  Because this is the function for saving the recording result, the operation is available
 *
 *	@param[in]	streamrecorder	A handle of streamrecorder.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@see		mm_streamrecorder_cancel
 *	@pre		Previous state of mm-streamrecorder should be MM_STREAMRECORDER_STATE_RECORDING
 *	@post		Next state of mm-streamrecorder will be MM_STREAMRECORDER_STATE_PREPARED
 *	@remarks	This function can take a few second when recording time is long.
 *			and if there are only quite few input buffer from video src or audio src,
 *			committing could be failed.
 *	@par example
 *	@code

#include <mm_streamrecorder.h>

gboolean record_and_save_video_file()
{
	int err;

	// Start recording
	err =  mm_streamrecorder_record(hstreamrecorder);
	if (err < 0) {
		return FALSE;
	}

	// Wait while recording for test...
	// In normal case, mm_streamrecorder_record() and mm_streamrecorder_commit() aren't called in the same function.

	// Save file
	err =  mm_streamrecorder_commit(hstreamrecorder);
	if (err < 0) {
		return FALSE;
	}

	return TRUE;
}

 *	@endcode
 */
int mm_streamrecorder_commit(MMHandleType streamrecorder);

/**
 *	mm_streamrecorder_cancel:\n
 *    Stop recording and discard the result. (Audio/Video mode only)
 *	When a user want to finish recording without saving the result file, this function can be used.
 *	Like mm_streamrecorder_commit(), this function also stops recording, release related resources(like codec) ,and goes back to preview status.
 *	However, instead of saving file, this function unlinks(delete) the result.\n
 *	Because this is the function for canceling recording, the operation is available
 *
 *	@param[in]	streamrecorder	A handle of streamrecorder.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@see		mm_streamrecorder_commit
 *	@pre		Previous state of mm-streamrecorder should be MM_STREAMRECORDER_STATE_RECORDING
 *	@post		Next state of mm-streamrecorder will be MM_STREAMRECORDER_STATE_PREPARED
 *	@remarks	None
 *	@par example
 *	@code

#include <mm_streamrecorder.h>

gboolean record_and_cancel_video_file()
{
	int err;

	// Start recording
	err =  mm_streamrecorder_record(hstreamrecorder);
	if (err < 0) {
		return FALSE;
	}

	// Wait while recording...

	// Cancel recording
	err =  mm_streamrecorder_cancel(hstreamrecorder);
	if (err < 0) {
		return FALSE;
	}

	return TRUE;
}

 *	@endcode
 */
int mm_streamrecorder_cancel(MMHandleType streamrecorder);

/**
 *    mm_streamrecorder_set_message_callback:\n
 *  Set callback for receiving messages from streamrecorder. Through this callback function, streamrecorder
 *  sends various message including status changes, asynchronous error, capturing, and limitations.
 *  One thing you have to know is that message callback is working on the main loop of application.
 *  So until releasing the main loop, message callback will not be called.
 *
 *	@param[in]	streamrecorder	A handle of streamrecorder.
 *	@param[in]	callback	Function pointer of callback function. Please refer 'MMMessageCallback'.
 *	@param[in]	user_data	User parameter for passing to callback function.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@see		MMMessageCallback
 *	@pre		None
 *	@post		None
 *	@remarks	registered 'callback' is called on main loop of the application. So until the main loop is released, 'callback' will not be called.
 *	@par example
 *	@code

#include <mm_streamrecorder.h>

gboolean setting_msg_callback()
{
	//set callback
	mm_streamrecorder_set_message_callback(hstreamrecorder,(MMMessageCallback)msg_callback, (void*)hstreamrecorder);

	return TRUE;
}

 *	@endcode
 */
int mm_streamrecorder_set_message_callback(MMHandleType streamrecorder, MMMessageCallback callback, void *user_data);

/**
 *    mm_streamrecorder_get_attributes:\n
 *  Get attributes of streamrecorder with given attribute names. This function can get multiple attributes
 *  simultaneously. If one of attribute fails, this function will stop at the point. 
 *  'err_attr_name' let you know the name of the attribute.
 *
 *	@param[in]	streamrecorder	Specifies the streamrecorder  handle.
 *	@param[out]	err_attr_name	Specifies the name of attributes that made an error. If the function doesn't make an error, this will be null. @n
 *					Free this variable after using.
 *	@param[in]	attribute_name	attribute name that user want to get.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@remarks	You can retrieve multiple attributes at the same time.  @n
 *			This function must finish with 'NULL' argument.  @n
 *			ex) mm_streamrecorder_get_attributes(....... , NULL);
 *	@see		mm_streamrecorder_set_attributes
 */
int mm_streamrecorder_get_attributes(MMHandleType streamrecorder, char **err_attr_name, const char *attribute_name, ...) G_GNUC_NULL_TERMINATED;

/**
 *    mm_streamrecorder_set_attributes:\n
 *  Set attributes of streamrecorder with given attribute names. This function can set multiple attributes
 *  simultaneously. If one of attribute fails, this function will stop at the point. 
 *  'err_attr_name' let you know the name of the attribute.
 *
 *	@param[in]	streamrecorder	Specifies the streamrecorder  handle.
 *	@param[out]	err_attr_name	Specifies the name of attributes that made an error. If the function doesn't make an error, this will be null. @n
 *					Free this variable after using.
 *	@param[in]	attribute_name	attribute name that user want to set.
 *	@return		This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *			Please refer 'mm_error.h' to know the exact meaning of the error.
 *	@pre		None
 *	@post		None
 *	@remarks	You can put multiple attributes to streamrecorder at the same time.  @n
 *			This function must finish with 'NULL' argument.  @n
 *			ex) mm_streamrecorder_set_attributes(....... , NULL);
 *	@see		mm_streamrecorder_get_attributes
 */
int mm_streamrecorder_set_attributes(MMHandleType streamrecorder, char **err_attr_name, const char *attribute_name, ...) G_GNUC_NULL_TERMINATED;

/**
 *    mm_streamrecorder_get_state:\n
 *  Get the current state of streamreccorder.
 *  mm_streamrecorderr is working on the base of its state. An user should check the state of mm_streamrecorder before calling its functions.
 *  If the handle is avaiable, user can retrieve the value.
 *
 *  @param[in]  streamrecorder   A handle of streamrecorder.
 *  @param[out] state       On return, it contains current state of streamrecorder.
 *  @return     This function returns zero(MM_ERROR_NONE) on success, or negative value with error code.\n
 *          Please refer 'mm_error.h' to know the exact meaning of the error.
 *  @see        MMStreamRecorderStateType
 *  @pre        None
 *  @post       None
 *  @remarks    None
 */
int mm_streamrecorder_get_state(MMHandleType , MMStreamRecorderStateType *status);

/**
	@}
 */

#ifdef __cplusplus
}
#endif

#endif /* __mm_streamrecorder_H__ */
