/*
 * mm_streamrecorder_testsuite
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
|  INCLUDE FILES                                                                        |
=======================================================================================*/
#include <stdio.h>
#include <stdlib.h>
#include <glib.h>
#include <gst/gst.h>
#include <sys/time.h>
#include "../src/include/mm_streamrecorder.h"
#include "../src/include/mm_streamrecorder_internal.h"
#include "../src/include/mm_streamrecorder_util.h"
#include "../src/include/mm_streamrecorder_attribute.h"
#include <gst/video/colorbalance.h>

/*-----------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS:                                       |
-----------------------------------------------------------------------*/
#define EXPORT_API __attribute__((__visibility__("default")))

#define PACKAGE "mm_streamrecorder_testsuite"

GMainLoop *g_loop;
GIOChannel *stdin_channel;
int resolution_set;
int g_current_state;
int src_w, src_h;
GstCaps *filtercaps;
bool isMultishot;
int multishot_num;
static int audio_stream_cb_cnt;
static int video_stream_cb_cnt;
static GTimer *timer = NULL;
int g_state;

enum {
	STREAMRECORDER_NULL,
	STREAMRECORDER_NONE,
	STREAMRECORDER_CREATED,
	STREAMRECORDER_STARTED,
	STREAMRECORDER_COMMITED
};

void streamrecorder_set_state(int new_state)
{
	g_state = new_state;
	return;
}

int streamrecorder_get_state()
{
	return g_state;
}



/*-----------------------------------------------------------------------
|    GLOBAL CONSTANT DEFINITIONS:                                       |
-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
|    IMPORTED VARIABLE DECLARATIONS:                                    |
-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
|    IMPORTED FUNCTION DECLARATIONS:                                    |
-----------------------------------------------------------------------*/


/*-----------------------------------------------------------------------
|    LOCAL #defines:                                                    |
-----------------------------------------------------------------------*/
#define test_ffmux_mp4

#define DISPLAY_X_0							0						/* for direct FB */
#define DISPLAY_Y_0							0						/* for direct FB */

#define SRC_VIDEO_FRAME_RATE_15         15    /* video input frame rate */
#define SRC_VIDEO_FRAME_RATE_30         30    /* video input frame rate */

#define STILL_CAPTURE_FILE_PATH_NAME    "/opt/StillshotCapture"
#define MULTI_CAPTURE_FILE_PATH_NAME    "/opt/MultishotCapture"
#define TARGET_FILENAME_PATH            "/opt/"
#define CAPTURE_FILENAME_LEN            256

#define AUDIO_SOURCE_SAMPLERATE_AAC     44100
#define AUDIO_SOURCE_SAMPLERATE_AMR     8000
#define AUDIO_SOURCE_FORMAT             MM_STREAMRECORDER_AUDIO_FORMAT_PCM_S16_LE
#define AUDIO_SOURCE_CHANNEL_AAC        2
#define AUDIO_SOURCE_CHANNEL_AMR        1
#define VIDEO_ENCODE_BITRATE            40000000 /* bps */
/*
 * D E B U G   M E S S A G E
 */
#define MMF_DEBUG                       "** (mmstreamrecorder testsuite) DEBUG: "
#define MMF_ERR                         "** (mmstreamrecorder testsuite) ERROR: "
#define MMF_INFO                        "** (mmstreamrecorder testsuite) INFO: "
#define MMF_WARN                        "** (mmstreamrecorder testsuite) WARNING: "
#define MMF_TIME                        "** (mmstreamrecorder testsuite) TIME: "

#define CHECK_MM_ERROR(expr) \
do {\
	int ret = 0; \
	ret = expr; \
	if (ret != MM_ERROR_NONE) {\
		printf("[%s:%d] error code : %x \n", __func__, __LINE__, ret); \
		return; \
	} \
} while (0)

#define time_msg_t(fmt, arg...)	\
do { \
	fprintf(stderr, "\x1b[44m\x1b[37m"MMF_TIME"[%s:%05d]  " fmt , __func__, __LINE__, ##arg); \
	fprintf(stderr, "\x1b[0m\n"); \
} while (0)

#define debug_msg_t(fmt, arg...)\
do { \
	fprintf(stderr, MMF_DEBUG"[%s:%05d]  " fmt "\n", __func__, __LINE__, ##arg); \
} while (0)

#define err_msg_t(fmt, arg...)	\
do { \
	fprintf(stderr, MMF_ERR"[%s:%05d]  " fmt "\n", __func__, __LINE__, ##arg); \
} while (0)

#define info_msg_t(fmt, arg...)	\
do { \
	fprintf(stderr, MMF_INFO"[%s:%05d]  " fmt "\n", __func__, __LINE__, ##arg); \
} while (0)

#define warn_msg_t(fmt, arg...)	\
do { \
	fprintf(stderr, MMF_WARN"[%s:%05d]  " fmt "\n", __func__, __LINE__, ##arg); \
} while (0)

#ifndef SAFE_FREE
#define SAFE_FREE(x)       if (x) {g_free(x); x = NULL; }
#endif


GTimeVal previous;
GTimeVal current;
GTimeVal result;
/* temporary buffer */
char buffer[460800] = {0x0,};


/**
 * Enumerations for command
 */
#define SENSOR_FLIP_NUM			3
#define SENSOR_PROGRAM_MODE_NUM		15
#define SENSOR_FOCUS_NUM		6
#define SENSOR_INPUT_ROTATION		4
#define SENSOR_AF_SCAN_NUM		4
#define SENSOR_ISO_NUM			8
#define SENSOR_EXPOSURE_NUM		9
#define SENSOR_IMAGE_FORMAT		9


/*-----------------------------------------------------------------------
|    LOCAL CONSTANT DEFINITIONS:                                        |
-----------------------------------------------------------------------*/
enum {
	MODE_VIDEO_CAPTURE,	/* recording and image capture mode */
	MODE_AUDIO,		/* audio recording*/
	MODE_NUM,
};

enum {
	MENU_STATE_MAIN,
	MENU_STATE_SETTING,
	MENU_STATE_NUM,
};

/*-----------------------------------------------------------------------
|    LOCAL DATA TYPE DEFINITIONS:					|
-----------------------------------------------------------------------*/
typedef struct _streamrecorder_handle {
	MMHandleType streamrecorder;
	int mode;                       /* image(capture)/video(recording) mode */
	bool isMultishot;               /* flag for multishot mode */
	int stillshot_count;            /* total stillshot count */
	int multishot_count;            /* total multishot count */
	const char *stillshot_filename;       /* stored filename of  stillshot  */
	const char *multishot_filename;       /* stored filename of  multishot  */
	int menu_state;
	int fps;
	bool isMute;
	unsigned long long elapsed_time;
} streamrecorder_handle_t;

typedef struct _str_xypair {
	char* attr_subcat_x;
	char* attr_subcat_y;
	int x;
	int y;
} str_xypair_t;

/*---------------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS:											|
---------------------------------------------------------------------------*/
static streamrecorder_handle_t *hstreamrecorder ;

const char *image_fmt[SENSOR_IMAGE_FORMAT] = {
	"NV12",
	"NV12T",
	"NV16",
	"NV21",
	"YUYV",
	"UYVY",
	"422P",
	"I420",
	"YV12",
};

const char *face_zoom_mode[] = {
	"Face Zoom OFF",
	"Face Zoom ON",
};

const char *display_mode[] = {
	"Default",
	"Primary Video ON and Secondary Video Full Screen",
	"Primary Video OFF and Secondary Video Full Screen",
};

const char *output_mode[] = {
	"Letter Box mode",
	"Original Size mode",
	"Full Screen mode",
	"Cropped Full Screen mode",
	"ROI mode",
};

const char *rotate_mode[] = {
	"0",
	"90",
	"180",
	"270",
};

const char* strobe_mode[] = {
	"Strobe OFF",
	"Strobe ON",
	"Strobe Auto",
	"Strobe RedEyeReduction",
	"Strobe SlowSync",
	"Strobe FrontCurtain",
	"Strobe RearCurtain",
	"Strobe Permanent",
};

const char *detection_mode[2] = {
	"Face Detection OFF",
	"Face Detection ON",
};

const char *wdr_mode[] = {
	"WDR OFF",
	"WDR ON",
	"WDR AUTO",
};

const char *hdr_mode[] = {
	"HDR OFF",
	"HDR ON",
	"HDR ON and Original",
};

const char *ahs_mode[] = {
	"Anti-handshake OFF",
	"Anti-handshake ON",
	"Anti-handshake AUTO",
	"Anti-handshake MOVIE",
};

const char *vs_mode[] = {
	"Video-stabilization OFF",
	"Video-stabilization ON",
};

const char *visible_mode[] = {
	"Display OFF",
	"Display ON",
};


/*---------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:												|
---------------------------------------------------------------------------*/
static void print_menu();
void  get_me_out();
static gboolean cmd_input(GIOChannel *channel);
static gboolean init(int type);
static gboolean mode_change();
int streamrecordertest_set_attr_int(const char* attr_subcategory, int value);


static inline void flush_stdin()
{
	int ch;
	while ((ch = getchar()) != EOF && ch != '\n');
}

#if 0
static gboolean test_idle_capture_start()
{
	int err;

	streamrecordertest_set_attr_int(MMSTR_VIDEO_SOURCE_FORMAT, MM_STREAMRECORDER_INPUT_FORMAT_NV12);
	streamrecordertest_set_attr_int(MMSTR_VIDEO_ENCODER, MM_VIDEO_CODEC_H264);
	streamrecordertest_set_attr_int(MMSTR_AUDIO_SOURCE_FORMAT, MM_STREAMRECORDER_AUDIO_FORMAT_PCM_U8);
	streamrecordertest_set_attr_int(MMSTR_AUDIO_ENCODER, MM_AUDIO_CODEC_AAC);

	g_timer_reset(timer);
	err = mm_streamrecorder_record(hstreamrecorder->streamrecorder);

	return FALSE;
}
#endif

int streamrecordertest_set_attr_int(const char * attr_subcategory, int value)
{
	char * err_attr_name = NULL;
	int err;

	if (hstreamrecorder) {
		if (hstreamrecorder->streamrecorder) {
			debug_msg_t("streamrecordertest_set_attr_int(%s, %d)", attr_subcategory, value);

			err = mm_streamrecorder_set_attributes(hstreamrecorder->streamrecorder, &err_attr_name, attr_subcategory, value, NULL);
			if (err != MM_ERROR_NONE) {
				err_msg_t("streamrecordertest_set_attr_int : Error(%s:%x)!!!!!!!", err_attr_name, err);
				SAFE_FREE(err_attr_name);
				return FALSE;
			}

			return TRUE;
		}

		debug_msg_t("streamrecordertest_set_attr_int(!hstreamrecorder->streamrecorder)");
	}

	debug_msg_t("streamrecordertest_set_attr_int(!hstreamrecorder)");

	return FALSE;
}

int streamrecordertest_set_attr_string(const char * attr_subcategory, char  *value)
{
	char * err_attr_name = NULL;
	int err;
	if (value == NULL) {
		err_msg_t("streamrecordertest_set_attr_string : value is null !");
		return FALSE;
	}
	if (hstreamrecorder) {
		if (hstreamrecorder->streamrecorder) {
			debug_msg_t("streamrecordertest_set_attr_string(%s, %s)", attr_subcategory, value);

			err = mm_streamrecorder_set_attributes(hstreamrecorder->streamrecorder, &err_attr_name, attr_subcategory, value, NULL);
			if (err != MM_ERROR_NONE) {
				err_msg_t("streamrecordertest_set_attr_string : Error(%s:%x)!!!!!!!", err_attr_name, err);
				SAFE_FREE(err_attr_name);
				return FALSE;
			}

			return TRUE;
		}

		debug_msg_t("streamrecordertest_set_attr_string(!hstreamrecorder->streamrecorder)");
	}

	debug_msg_t("streamrecordertest_set_attr_string(!hstreamrecorder)");

	return FALSE;
}

int streamrecordertest_set_attr_xypair(str_xypair_t pair)
{
	char * err_attr_name = NULL;
	int err;

	if (hstreamrecorder) {
		if (hstreamrecorder->streamrecorder) {
			debug_msg_t("streamrecordertest_set_attr_xypair((%s, %s), (%d, %d))",  pair.attr_subcat_x, pair.attr_subcat_y, pair.x, pair.y);

			err = mm_streamrecorder_set_attributes(hstreamrecorder->streamrecorder, &err_attr_name,
									pair.attr_subcat_x, pair.x,
									pair.attr_subcat_y, pair.y,
									NULL);
			if (err < 0) {
				err_msg_t("streamrecordertest_set_attr_xypair : Error(%s:%x)!!", err_attr_name, err);
				SAFE_FREE(err_attr_name);
				return FALSE;
			}

			return TRUE;
		}

		debug_msg_t("streamrecordertest_set_attr_xypair(!hstreamrecorder->streamrecorder)");
	}

	debug_msg_t("streamrecordertest_set_attr_xypair(!hstreamrecorder)");
	return FALSE;
}

int streamrecordertest_get_attr_valid_intarray(const char * attr_name, int ** array, int *count)
{
	MMStreamRecorderAttrsInfo info;
	int err = MM_ERROR_NONE;

	if (hstreamrecorder) {
		if (hstreamrecorder->streamrecorder) {
			debug_msg_t("streamrecordertest_get_attr_valid_intarray(%s)", attr_name);

			err = mm_streamrecorder_get_attribute_info(hstreamrecorder->streamrecorder, attr_name, &info);
			if (err != MM_ERROR_NONE) {
				err_msg_t("streamrecordertest_get_attr_valid_intarray : Error(%x)!!", err);
				return FALSE;
			} else {
				if (info.type == MM_STR_REC_ATTRS_TYPE_INT) {
					if (info.validity_type == MM_STR_REC_ATTRS_VALID_TYPE_INT_ARRAY) {
						*array = info.int_array.array;
						*count = info.int_array.count;
						debug_msg_t("INT ARRAY - default value : %d", info.int_array.def);
						return TRUE;
					}
				}

				err_msg_t("streamrecordertest_get_attr_valid_intarray : Type mismatched!!");
				return FALSE;
			}
		}

		debug_msg_t("streamrecordertest_get_attr_valid_intarray(!hstreamrecorder->streamrecorder)");
	}


	debug_msg_t("streamrecordertest_get_attr_valid_intarray(!hstreamrecorder)");
	return FALSE;
}

int streamrecordertest_get_attr_valid_intrange(const char * attr_name, int *min, int *max)
{
	MMStreamRecorderAttrsInfo info;
	int err = MM_ERROR_NONE;

	if (hstreamrecorder) {
		if (hstreamrecorder->streamrecorder) {
			debug_msg_t("streamrecordertest_get_attr_valid_intrange(%s)", attr_name);

			err = mm_streamrecorder_get_attribute_info(hstreamrecorder->streamrecorder, attr_name, &info);
			if (err != MM_ERROR_NONE) {
				err_msg_t("streamrecordertest_get_attr_valid_intarray : Error(%x)!!",  err);
				return FALSE;
			} else {
				if (info.type == MM_STR_REC_ATTRS_TYPE_INT) {
					if (info.validity_type == MM_STR_REC_ATTRS_VALID_TYPE_INT_RANGE) {
						*min = info.int_range.min;
						*max = info.int_range.max;
						debug_msg_t("INT RANGE - default : %d", info.int_range.def);
						return TRUE;
					}
				}

				err_msg_t("streamrecordertest_get_attr_valid_intarray : Type mismatched!!");
				return FALSE;
			}

		}

		debug_msg_t("streamrecordertest_get_attr_valid_intarray(!hstreamrecorder->streamrecorder)");
	}

	debug_msg_t("streamrecordertest_get_attr_valid_intarray(!hstreamrecorder)");
	return FALSE;
}


void  get_me_out()
{
}

static void print_menu()
{
	switch (hstreamrecorder->menu_state) {
	case MENU_STATE_MAIN:
			g_print("\n\t=======================================\n");
			g_print("\t   Stream Recorder Menu \n");
			g_print("\t=======================================\n");
			if (hstreamrecorder->mode == MODE_VIDEO_CAPTURE) {
				if (streamrecorder_get_state() <= STREAMRECORDER_CREATED) {
					g_print("\t   '1' Start Recording\n");
					g_print("\t   '2' Setting\n");
					g_print("\t   '3' Print frame rate\n");
					g_print("\t   'b' back\n");
				} else if (streamrecorder_get_state() == STREAMRECORDER_STARTED) {
					g_print("\t   'r' Resume\n");
					g_print("\t   'c' Cancel Recording\n");
					g_print("\t   's' Save\n");
				}
			} else if (hstreamrecorder->mode == MODE_AUDIO) {
				if (streamrecorder_get_state() <= STREAMRECORDER_CREATED) {
					g_print("\t   '1' Start Recording\n");
					g_print("\t   'b' back\n");
				} else if (streamrecorder_get_state() == STREAMRECORDER_STARTED) {
					g_print("\t   'r' Resume Recording\n");
					g_print("\t   'c' Cancel Recording\n");
					g_print("\t   's' Save Recording\n");
				}
			}
		break;
	case MENU_STATE_SETTING:
		g_print("\n\t=======================================\n");
		g_print("\t   Stream Recorder > Setting\n");
		g_print("\t=======================================\n");
		g_print("\t  >>>>>>>>>>>>>>>>>>>>>>>>>>>> [Stream Recorder]  \n");
		g_print("\t     'T' videobuffer type \n");
		g_print("\t     'F' videosource  format \n");
		g_print("\t     'A' video framerate \n");
		g_print("\t     'B' video bitrate \n");
		g_print("\t     'D' audio source format\n");
		g_print("\t     'I'  audio bitrate \n");
		g_print("\t     'S' audio samplerate \n");
		g_print("\t     'O' video encoder \n");
		g_print("\t     'C' audio encoder \n");
		g_print("\t     'N' audio channel count \n");
		g_print("\t     'm' file format\n");
		g_print("\t     'b'  back \n");
		g_print("\t=======================================\n");
		break;
	default:
		warn_msg_t("unknow menu state !!\n");
		break;
	}

	return;
}

static void main_menu(gchar buf)
{
	int err = 0;

	if (hstreamrecorder->mode == MODE_VIDEO_CAPTURE) {
		if (streamrecorder_get_state() == STREAMRECORDER_CREATED) {
			switch (buf) {
			case '1': /* Start Recording */
				g_print("*Recording start!\n");
				video_stream_cb_cnt = 0;
				audio_stream_cb_cnt = 0;

				g_timer_reset(timer);
				err = mm_streamrecorder_record(hstreamrecorder->streamrecorder);

				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 1000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 2000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 3000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 4000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 5000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 6000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 7000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 8000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 9000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 10000, buffer, (640*480*3/2));
				mm_streamrecorder_push_stream_buffer(hstreamrecorder->streamrecorder, MM_STREAM_TYPE_VIDEO, 11000, buffer, (640*480*3/2));

				if (err != MM_ERROR_NONE)
					warn_msg_t("Rec start mm_streamrecorder_record 0x%x", err);

				streamrecorder_set_state(STREAMRECORDER_STARTED);
				break;

			case '2': /* Setting */
				hstreamrecorder->menu_state = MENU_STATE_SETTING;
				break;

			case '3': /* Print frame rate */
				break;

			case 'b': /* back */
				hstreamrecorder->menu_state = MENU_STATE_MAIN;
				mode_change();
				break;

			default:
				g_print("\t Invalid input \n");
				break;
			}
		} else if (streamrecorder_get_state() == STREAMRECORDER_STARTED) {
			switch (buf) {
			case 'r': /* Resume Recording */
				g_print("*Resume!\n");
				break;

			case 'c': /* Cancel */
				g_print("*Cancel Recording !\n");
				err = mm_streamrecorder_cancel(hstreamrecorder->streamrecorder);

				if (err < 0)
					warn_msg_t("Cancel recording mm_streamrecorder_cancel  = %x", err);

				break;

			case 's': /* Save */
				g_print("*Save Recording!\n");
				g_timer_reset(timer);

				err = mm_streamrecorder_commit(hstreamrecorder->streamrecorder);
				streamrecorder_set_state(STREAMRECORDER_CREATED);

				if (err < 0)
					warn_msg_t("Save recording mm_streamrecorder_commit  = %x", err);

				break;

			case 'n': /* Capture video snapshot */
				break;

			default:
				g_print("\t Invalid input \n");
				break;
			} /* switch */
		} else {
			err_msg_t("Wrong streamrecorder state, check status!!");
		}
	} else if (hstreamrecorder->mode == MODE_AUDIO) {
			switch (buf) {
			case '1': /* Start Recording */
				g_print("*Recording start!\n");
				g_timer_reset(timer);
				err = mm_streamrecorder_record(hstreamrecorder->streamrecorder);

				if (err < 0)
					warn_msg_t("Rec start mm_streamrecorder_record  = %x", err);

				break;

			case 'b': /* back */
					hstreamrecorder->menu_state = MENU_STATE_MAIN;
					mode_change();
					break;

			default:
				g_print("\t Invalid input \n");
				break;
			}
	} else {
		g_print("\t Invalid mode, back to upper menu \n");
		hstreamrecorder->menu_state = MENU_STATE_MAIN;
		mode_change();
	}
}


static void setting_menu(gchar buf)
{
	gboolean bret = FALSE;
	int index_menu = 0;
	int min = 0;
	int max = 0;
	int width_count = 0;
	int height_count = 0;
	int i = 0;
	int count = 0;
	int value = 0;
	int* array = NULL;
	int *width_array = NULL;
	int *height_array = NULL;
	char *err_attr_name = NULL;
	str_xypair_t input_pair;
	char filename[100];
	int err = MM_ERROR_NONE;
	int x = 0, y = 0, width = 0, height = 0;

	if (hstreamrecorder->mode == MODE_VIDEO_CAPTURE) {
		switch (buf) {
		case '0':  /* Setting */
			g_print("*Select the preview resolution!\n");
			streamrecordertest_get_attr_valid_intarray(MMSTR_VIDEO_RESOLUTION_WIDTH, &width_array, &width_count);
			streamrecordertest_get_attr_valid_intarray(MMSTR_VIDEO_RESOLUTION_HEIGHT, &height_array, &height_count);

			if (width_count != height_count) {
				err_msg_t("System has wrong information!!\n");
			} else if (width_count == 0) {
				g_print("Not supported!!\n");
			} else {
				flush_stdin();

				for (i = 0; i < width_count; i++)
					g_print("\t %d. %d*%d\n", i+1, width_array[i], height_array[i]);

				err = scanf("%d", &index_menu);
				if (err == EOF) {
					printf("\nscanf error : errno %d\n", errno);
				} else {
					if (index_menu > 0 && index_menu <= width_count) {
						input_pair.x = width_array[index_menu-1];
						input_pair.y = height_array[index_menu-1];
						bret = streamrecordertest_set_attr_xypair(input_pair);
					}
				}
			}
			break;

		case '1': /* Setting > Capture Resolution setting */
			break;

		case 'r': /* Setting > Rotate input when recording */
			g_print("Not supported !! \n");
			break;

		case 'H': /* Setting > Hybrid mode */
			g_print("* Hybrid mode\n");

			g_print("\t 0. DISABLE\n");
			g_print("\t 1. ENABLE\n");

			flush_stdin();
			err = scanf("%d", &index_menu);

			if (index_menu < 0 || index_menu > 1)
				g_print("Wrong INPUT[%d]!! \n", index_menu);

			break;

		case 'R': /*  Setting > Stream Recorder-rotation setting */
			g_print("*Stream Recorder-Rotation setting!\n");

			g_print("\t0.  0 degree\n");
			g_print("\t1.  90 degree\n");
			g_print("\t2.  180 degree\n");
			g_print("\t3.  270 degree\n");
			flush_stdin();
			err = scanf("%d", &index_menu);
			if (index_menu < 0 || index_menu > 3)
				g_print("Wrong Input[%d] !!\n\n", index_menu);

			g_print("*Stream Recorder-Flip setting!\n");
			g_print("\t0.  NONE\n");
			g_print("\t1.  HORIZONTAL\n");
			g_print("\t2.  VERTICAL\n");
			g_print("\t3.  BOTH\n");
			flush_stdin();
			err = scanf("%d", &index_menu);
			if (index_menu < 0 || index_menu > 3)
				g_print("Wrong Input[%d] !!\n\n", index_menu);

			break;

		case 'T': /* Setting > videobuffer-type setting */
			g_print("*videobuffer type  !\n");
			streamrecordertest_get_attr_valid_intrange("videobuffer-type", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  videobuffer type (%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("videobuffer-type", index_menu);
			}
			break;

		case 'F': /* Setting > videosource-format setting */
			g_print("*videosource  format  !\n");
			streamrecordertest_get_attr_valid_intrange("videosource-format", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select videosource-format (%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("videosource-format", index_menu);
			}
			break;

		case 'A': /* Setting > video framerate setting */
			g_print("*video framerate  !\n");
			streamrecordertest_get_attr_valid_intrange("video-framerate", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  video-framerate (%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("video-framerate", index_menu);
			}
			break;

		case 'B': /* Setting > video bitrate setting */
			g_print("*video bitrate  !\n");
			streamrecordertest_get_attr_valid_intrange("video-bitrate", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  video-bitrate (%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("video-bitrate", index_menu);
			}
			break;

		case 'D': /* Setting > audio-source-format setting */
			g_print("*audio-source-format !\n");
			streamrecordertest_get_attr_valid_intrange("audio-source-format", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  audio-source-format (%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("audio-source-format", index_menu);
			}
			break;

		case 'I': /* Setting > audio-bitrate setting */
			g_print("*audio-bitrate !\n");
			streamrecordertest_get_attr_valid_intrange("audio-bitrate", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  audio-bitrate (%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("audio-bitrate", index_menu);
			}
			break;

		case 'S': /* Setting > audio-samplerate setting */
			g_print("*audio-samplerate !\n");
			streamrecordertest_get_attr_valid_intrange("audio-samplerate", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  audio-samplerate (%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("audio-samplerate", index_menu);
			}
			break;

		case 'O': /* Setting > video-encoder setting */
			g_print("*video-encoder!\n");
			streamrecordertest_get_attr_valid_intrange("video-encoder", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  video-encoder (%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("video-encoder", index_menu);
			}
			break;

		case 'C': /* Setting >audio-encoder setting */
			g_print("*audio-encoder!\n");
			streamrecordertest_get_attr_valid_intrange("audio-encoder", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  audio-encoder (%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("audio-encoder", index_menu);
			}
			break;

		case 'N': /* Setting >audio-channel-count setting */
			g_print("*audio-channel-count!\n");
			streamrecordertest_get_attr_valid_intrange("audio-channel-count", &min, &max);

			if (min >= max) {
				g_print("Not supported !! \n");
			} else {
				flush_stdin();
				g_print("\n Select  audio-channel-count(%d ~ %d)\n", min, max);
				err = scanf("%d", &index_menu);
				bret = streamrecordertest_set_attr_int("audio-channel-count", index_menu);
			}
			break;

		case 'b': /* back */
			hstreamrecorder->menu_state = MENU_STATE_MAIN;
			break;

		default:
			g_print("\t Invalid input \n");
			break;
		}
	} else {
		g_print("\t Invalid mode, back to upper menu \n");
		hstreamrecorder->menu_state = MENU_STATE_MAIN;
	}
}


/**
 * This function is to execute command.
 *
 * @param	channel	[in]	1st parameter
 *
 * @return	This function returns TRUE/FALSE
 * @remark
 * @see
 */
static gboolean cmd_input(GIOChannel *channel)
{
	gchar *buf = NULL;
	gsize read_size;
	GError *g_error = NULL;

	debug_msg_t("ENTER");

	g_io_channel_read_line(channel, &buf, &read_size, NULL, &g_error);
	if (g_error) {
		debug_msg_t("g_io_channel_read_chars error");
		g_error_free(g_error);
		g_error = NULL;
	}

	if (buf) {
		g_strstrip(buf);

		debug_msg_t("Menu Status : %d", hstreamrecorder->menu_state);
		switch (hstreamrecorder->menu_state) {
		case MENU_STATE_MAIN:
			main_menu(buf[0]);
			break;
		case MENU_STATE_SETTING:
			setting_menu(buf[0]);
			break;
		default:
			break;
		}

		g_free(buf);
		buf = NULL;

		print_menu();
	} else {
		debug_msg_t("No read input");
	}

	return TRUE;
}

/**
 * This function is to initiate streamrecorder attributes .
 *
 * @param	type	[in]	image(capture)/video(recording) mode
 *
 * @return	This function returns TRUE/FALSE
 * @remark
 * @see		other functions
 */
static gboolean init(int type)
{
	MMHandleType str_handle = 0;

	char *err_attr_name = NULL;
	int video_codec = MM_VIDEO_CODEC_INVALID;
	int audio_codec = MM_AUDIO_CODEC_INVALID;
	int file_format = MM_FILE_FORMAT_INVALID;
	int audio_disable = FALSE;
	int video_disable = FALSE;
	int audio_enc = MM_AUDIO_CODEC_INVALID;
	int channel = 0;
	int v_bitrate = 0;
	int a_bitrate = 0;
	int video_width = 0;
	int video_height = 0;
	int video_fps = 0;
	int audio_samplerate = 0;
	int audio_src_format = 0;
	int video_src_format = 0;
	int rec_mode = 0;
	const char *filename = "/opt/usr/media/test.mp4";

	if (!hstreamrecorder)
		return FALSE;

	if (!hstreamrecorder->streamrecorder)
		return FALSE;

	str_handle = (MMHandleType)(hstreamrecorder->streamrecorder);

	/*================================================================================
		Video capture mode
	*=================================================================================*/
	if (type == MODE_VIDEO_CAPTURE) {
		video_codec = MM_VIDEO_CODEC_MPEG4;
		/*audio_codec = MM_AUDIO_CODEC_AAC;*/
		file_format = MM_FILE_FORMAT_MP4;
		/*audio_enc = MM_AUDIO_CODEC_PCM;*/
		/*channel = 1;
		v_bitrate = 8000000;
		/*a_bitrate = 64000;*/
		video_width = 640;
		video_height = 480;
		video_fps = 30;
		/*audio_samplerate = 48000;*/
		/*audio_src_format = 2;*/
		rec_mode = 0;
		video_src_format = MM_STREAMRECORDER_INPUT_FORMAT_NV12;
		audio_disable = TRUE;
		mm_streamrecorder_set_attributes((MMHandleType)str_handle, &err_attr_name,
								MMSTR_VIDEO_DISABLE,  video_disable,
								MMSTR_AUDIO_DISABLE,  audio_disable,
								NULL);

		mm_streamrecorder_set_attributes((MMHandleType)str_handle, &err_attr_name,
								MMSTR_VIDEO_ENCODER, video_codec,
								/*MMSTR_AUDIO_ENCODER,  audio_codec,*/
								MMSTR_FILE_FORMAT, file_format,
								MMSTR_VIDEO_BITRATE, v_bitrate,
								MMSTR_VIDEO_RESOLUTION_WIDTH , video_width,
								MMSTR_VIDEO_RESOLUTION_HEIGHT, video_height,
								MMSTR_VIDEO_FRAMERATE, video_fps,
								/*MMSTR_AUDIO_CHANNEL, channel,*/
								/*MMSTR_AUDIO_SAMPLERATE, audio_samplerate,*/
								/*MMSTR_AUDIO_BITRATE, a_bitrate,*/
								/*MMSTR_AUDIO_SOURCE_FORMAT, audio_src_format,*/
								MMSTR_VIDEO_SOURCE_FORMAT, video_src_format,
								MMSTR_RECORDER_MODE, rec_mode,
								NULL);
		mm_streamrecorder_set_attributes((MMHandleType)str_handle, &err_attr_name,
								MMSTR_FILENAME, filename, strlen(filename), NULL);


	}
	/*================================================================================
		Audio mode
	*=================================================================================*/
	debug_msg_t("Init DONE.");

	return TRUE;
}

static gboolean init_handle()
{
	hstreamrecorder->mode = MODE_VIDEO_CAPTURE;  /* image(capture)/video(recording) mode */
	hstreamrecorder->isMultishot =  FALSE;
	hstreamrecorder->stillshot_count = 0;        /* total stillshot count */
	hstreamrecorder->multishot_count = 0;        /* total multishot count */
	hstreamrecorder->stillshot_filename = STILL_CAPTURE_FILE_PATH_NAME;  /* stored filename of  stillshot  */
	hstreamrecorder->multishot_filename = MULTI_CAPTURE_FILE_PATH_NAME;  /* stored filename of  multishot  */
	hstreamrecorder->menu_state = MENU_STATE_MAIN;
	hstreamrecorder->isMute = FALSE;
	hstreamrecorder->elapsed_time = 0;
	hstreamrecorder->fps = SRC_VIDEO_FRAME_RATE_15; /*SRC_VIDEO_FRAME_RATE_30;*/

	return TRUE;
}
/**
 * This function is to change streamrecorder mode.
 *
 * @param	type	[in]	image(capture)/video(recording) mode
 *
 * @return	This function returns TRUE/FALSE
 * @remark
 * @see		other functions
 */
static gboolean mode_change()
{
	int err = MM_ERROR_NONE;
	int state = STREAMRECORDER_NONE;
	int device_count = 0;
	int facing_direction = 0;
	char media_type = '\0';
	char *evassink_name = NULL;
	bool check = FALSE;

	state = streamrecorder_get_state();
	debug_msg_t("MMStreamrecorder State : %d", state);
	if (state != STREAMRECORDER_NULL) {
		if ((state == STREAMRECORDER_STARTED)) {
			debug_msg_t("mm_streamrecorder_cancel");
			err = mm_streamrecorder_cancel(hstreamrecorder->streamrecorder);

			if (err < 0) {
				warn_msg_t("exit mm_streamrecorder_cancel  = %x", err);
				return FALSE;
			}
		}

		state = streamrecorder_get_state();
		if (state == STREAMRECORDER_CREATED) {
			debug_msg_t("mm_streamreorder_unrealize");
			mm_streamrecorder_unrealize(hstreamrecorder->streamrecorder);
			streamrecorder_set_state(STREAMRECORDER_NONE);
		}

		state = streamrecorder_get_state();
		if (state == STREAMRECORDER_NONE) {
			debug_msg_t("mm_streamrecorder_destroy");
			mm_streamrecorder_destroy(hstreamrecorder->streamrecorder);

			streamrecorder_set_state(STREAMRECORDER_NULL);
		}
	}

	init_handle();
	while (!check) {
		g_print("\n\t=======================================\n");
		g_print("\t   MM_STREAMRECORDER_TESTSUIT\n");
		g_print("\t=======================================\n");
		g_print("\t   '1' STREAM RECORDER CREATE\n");
		g_print("\t   'q' Exit\n");
		g_print("\t=======================================\n");

		g_print("\t  Enter the media type:\n\t");

		err = scanf("%c", &media_type);

		switch (media_type) {
		case '1':
			hstreamrecorder->mode = MODE_VIDEO_CAPTURE;
			check = TRUE;
			break;
		case '2':
			hstreamrecorder->mode = MODE_VIDEO_CAPTURE;
			check = TRUE;
			break;
		case '3':
			hstreamrecorder->mode = MODE_AUDIO;
			check = TRUE;
			break;
		case '4':
			hstreamrecorder->mode = MODE_VIDEO_CAPTURE;
			check = TRUE;
			break;
		case 'q':
			g_print("\t Quit streamrecorder Testsuite!!\n");
			hstreamrecorder->mode = -1;
			if (g_main_loop_is_running(g_loop))
				g_main_loop_quit(g_loop);

			return FALSE;
		default:
			g_print("\t Invalid media type(%d)\n", media_type);
			continue;
		}
	}

	debug_msg_t("mm_streamrecorder_create");
	g_get_current_time(&previous);
	g_timer_reset(timer);

	FILE *fp = NULL;
	size_t nread;
	fp = fopen("/opt/usr/media/test.nv12", "a+");
	if (!fp)
		return -1;
	nread = fread(&buffer, sizeof(char), sizeof(buffer), fp);
	time_msg_t("mm_streamrecorder_create()  : nread %d, sizeof(buffer) %d", nread, sizeof(buffer));
	fclose(fp);

	err = mm_streamrecorder_create(&hstreamrecorder->streamrecorder);
	time_msg_t("mm_streamrecorder_create()  : %12.6lfs", g_timer_elapsed(timer, NULL));

	if (err != MM_ERROR_NONE) {
		err_msg_t("mmstreamrecorder_create = %x", err);
		return -1;
	} else {

	}

	debug_msg_t("evassink name[%s], device count[%d], facing direction[%d]", evassink_name, device_count, facing_direction);

	if (!init(hstreamrecorder->mode)) {
		err_msg_t("testsuite init() failed.");
		return -1;
	}

	debug_msg_t("mm_streamrecorder_realize");

	g_timer_reset(timer);

	err =  mm_streamrecorder_realize(hstreamrecorder->streamrecorder);
	streamrecorder_set_state(STREAMRECORDER_CREATED);
	time_msg_t("mm_streamrecorder_realize()  : %12.6lfs", g_timer_elapsed(timer, NULL));
	if (err != MM_ERROR_NONE) {
		err_msg_t("mm_streamrecorder_realize  = %x", err);
		return -1;
	}

	g_timer_reset(timer);

	g_get_current_time(&current);
	timersub(&current, &previous, &result);
	time_msg_t("streamrecorder Starting Time  : %ld.%lds", result.tv_sec, result.tv_usec);

	return TRUE;
}


/**
 * This function is the example main function for mmstreamrecorder API.
 *
 * @param
 *
 * @return	This function returns 0.
 * @remark
 * @see		other functions
 */
int main(int argc, char **argv)
{
	int bret;

	timer = g_timer_new();

	gst_init(&argc, &argv);

	time_msg_t("gst_init() : %12.6lfs", g_timer_elapsed(timer, NULL));

	hstreamrecorder = (streamrecorder_handle_t *) g_malloc0(sizeof(streamrecorder_handle_t));

	g_timer_reset(timer);

	bret = mode_change();
	if (!bret)
		return bret;

	print_menu();

	g_loop = g_main_loop_new(NULL, FALSE);

	stdin_channel = g_io_channel_unix_new(fileno(stdin));/* read from stdin */
	g_io_add_watch(stdin_channel, G_IO_IN, (GIOFunc)cmd_input, NULL);

	debug_msg_t("RUN main loop");

	g_main_loop_run(g_loop);

	debug_msg_t("STOP main loop");

	if (timer) {
		g_timer_stop(timer);
		g_timer_destroy(timer);
		timer = NULL;
	}

	g_free(hstreamrecorder);
	g_main_loop_unref(g_loop);
	g_io_channel_unref(stdin_channel);

	return bret;
}

/*EOF*/
