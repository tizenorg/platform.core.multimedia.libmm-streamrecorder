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

#ifndef __MM_STREAMRECORDER_UTIL_H__
#define __MM_STREAMRECORDER_UTIL_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/
#include <gst/gst.h>
#include <gst/gstutils.h>
#include <gst/gstpad.h>
#include "mm_debug.h"
#include <glib.h>
#include<stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| GLOBAL DEFINITIONS AND DECLARATIONS FOR STREAMRECORDER					|
========================================================================================*/

/*=======================================================================================
| MACRO DEFINITIONS									|
========================================================================================*/
#ifndef CLEAR
#define CLEAR(x)            memset (&(x), 0, sizeof (x))
#endif

#define SAFE_FREE(x) \
	if (x) {\
		g_free(x); \
		x = NULL; \
	}

#define _mmstreamrec_dbg_verb(fmt, args...)  debug_verbose(" "fmt"\n", ##args);
#define _mmstreamrec_dbg_log(fmt, args...)   debug_log(" "fmt"\n", ##args);
#define _mmstreamrec_dbg_warn(fmt, args...)  debug_warning(" "fmt"\n", ##args);
#define _mmstreamrec_dbg_err(fmt, args...)   debug_error(" "fmt"\n", ##args);
#define _mmstreamrec_dbg_crit(fmt, args...)  debug_critical(" "fmt"\n", ##args);

/**
 *	Macro for checking validity and debugging
 */
#define mmf_return_if_fail( expr )	\
	if( expr ){}					\
	else						\
	{							\
		_mmstreamrec_dbg_err( "failed [%s]", #expr);	\
		return;						\
	};

/**
 *	Macro for checking validity and debugging
 */
#define mmf_return_val_if_fail( expr, val )	\
	if( expr ){}					\
	else						\
	{							\
		_mmstreamrec_dbg_err("failed [%s]", #expr);	\
		return( val );						\
	};

/**
 *	Minimum integer value
 */
#define _MMSTREAMRECORDER_MIN_INT	(INT_MIN)

/**
 *	Maximum integer value
 */
#define _MMSTREAMRECORDER_MAX_INT	(INT_MAX)

/**
 *	Minimum double value
 */
#define _MMSTREAMRECORDER_MIN_DOUBLE	(DBL_MIN)

/**
 *	Maximum integer value
 */
#define _MMSTREAMRECORDER_MAX_DOUBLE	(DBL_MAX)

// TODO : MOVE OTHERS
enum {
	PUSH_ENCODING_BUFFER_INIT = 0,
	PUSH_ENCODING_BUFFER_RUN,
	PUSH_ENCODING_BUFFER_STOP,
};

#define FPUTC_CHECK(x_char, x_file)\
{\
	if (fputc(x_char, x_file) == EOF) \
	{\
		_mmstreamrec_dbg_err("[Critical] fputc() returns fail.\n");	\
		return FALSE;\
	}\
}

#define FPUTS_CHECK(x_str, x_file)\
{\
	if (fputs(x_str, x_file) == EOF) \
	{\
		_mmstreamrec_dbg_err("[Critical] fputs() returns fail.\n");\
		SAFE_FREE(str);	\
		return FALSE;\
	}\
}

#if 0
#define MM_STREAMRECORDER_INI_DEFAULT_PATH	"/usr/etc/mmfw_transcode.ini"
#define STREAMRECORDER_INI_MAX_STRLEN	100
#endif
/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/
#define MAX_ERROR_MESSAGE_LEN         128
#define MM_STREAMRECORDER_MIN_LOG_COUNT 10

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/
#if 0
	typedef struct __mm_streamrecorder_ini {
		/* video converter element */
		gchar video_converter_element[STREAMRECORDER_INI_MAX_STRLEN];
	} mm_streamrecorder_ini_t;
#endif

/*=======================================================================================
| CONSTANT DEFINITIONS									|
========================================================================================*/

/*=======================================================================================
| GLOBAL FUNCTION PROTOTYPES								|
========================================================================================*/

/* Pixel format */

int _mmstreamrecorder_get_pixel_format(GstCaps *caps);
int _mmstreamrecorder_get_pixtype(unsigned int fourcc);
unsigned int _mmstreamrecorder_get_fourcc(int pixtype, int codectype, int use_zero_copy_format);

/* Recording */
/* find top level tag only, do not use this function for finding sub level tags. tag_fourcc is Four-character-code (FOURCC) */
gint32 _mmstreamrecorder_double_to_fix(gdouble d_number);

/* File system */
int _mmstreamrecorder_get_freespace(const gchar *path, guint64 *free_space);
int _mmstreamrecorder_get_file_size(const char *filename, guint64 *size);

/* Debug */
void _mmstreamrecorder_err_trace_write(char *str_filename, char *func_name, int line_num, char *fmt, ...);

guint16 get_language_code(const char *str);
gchar *str_to_utf8(const gchar *str);
inline gboolean write_tag(FILE *f, const gchar *tag);
inline gboolean write_to_32(FILE *f, guint val);
inline gboolean write_to_16(FILE *f, guint val);
inline gboolean write_to_24(FILE *f, guint val);
#ifdef __cplusplus
}
#endif
#endif							/* __MM_STREAMRECORDER_UTIL_H__ */
