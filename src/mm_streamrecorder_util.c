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
#include <stdio.h>
#include <stdarg.h>
#include <sys/vfs.h>			/* struct statfs */

#include "mm_streamrecorder_util.h"
#include <mm_util_imgp.h>
#include <mm_types.h>
#include <mm_error.h>
#include <glib.h>
#include <stdlib.h>
#include "iniparser.h"
#include <glib/gstdio.h>
#include <gst/video/video-info.h>

/*-----------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/
#define TIME_STRING_MAX_LEN     64

/*---------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:												|
---------------------------------------------------------------------------*/
/* STATIC INTERNAL FUNCTION */

/* static gint		skip_mdat(FILE *f); */

/*===========================================================================================
|																							|
|  FUNCTION DEFINITIONS																		|
========================================================================================== */
/*---------------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:											|
---------------------------------------------------------------------------*/

gint32 _mmstreamrecorder_double_to_fix(gdouble d_number)
{
	return (gint32) (d_number * 65536.0);
}

int _mmstreamrecorder_get_freespace(const gchar *path, guint64 *free_space)
{
	struct statfs fs;

	g_assert(path);

	if (!g_file_test(path, G_FILE_TEST_EXISTS | G_FILE_TEST_IS_DIR)) {
		_mmstreamrec_dbg_log("File(%s) doesn't exist.", path);
		return -2;
	}

	if (-1 == statfs(path, &fs)) {
		_mmstreamrec_dbg_log("Getting free space is failed.(%s)", path);
		return -1;
	}

	*free_space = (guint64) fs.f_bsize * fs.f_bavail;
	return 1;
}

int _mmstreamrecorder_get_file_size(const char *filename, guint64 * size)
{
	struct stat buf;

	if (stat(filename, &buf) != 0)
		return -1;
	*size = (guint64) buf.st_size;
	return 1;
}

void _mmstreamrecorder_err_trace_write(char *str_filename, char *func_name, int line_num, char *fmt, ...)
{
	FILE *f = NULL;
	va_list ap = { 0 };
	char time_string[TIME_STRING_MAX_LEN] = { '\0', };

	time_t current_time;
	struct tm new_time;

	mmf_return_if_fail(str_filename);

	current_time = time(NULL);
	localtime_r(&current_time, &new_time);

	f = fopen(str_filename, "a");
	if (f == NULL) {
		_mmstreamrec_dbg_warn("Failed to open file.[%s]", str_filename);
		return;
	}

	asctime_r(&new_time, time_string);
	fprintf(f, "[%.19s][%05d][%s]", time_string, line_num, func_name);

	va_start(ap, fmt);
	vfprintf(f, fmt, ap);
	va_end(ap);

	fprintf(f, "\n");

	fclose(f);
}

int _mmstreamrecorder_get_pixel_format(GstCaps *caps)
{
	const GstStructure *structure;
	const char *media_type = NULL;
	MMPixelFormatType type = 0;
	unsigned int fourcc = 0;
	GstVideoInfo media_info;

	mmf_return_val_if_fail(caps != NULL, MM_PIXEL_FORMAT_INVALID);

	structure = gst_caps_get_structure(caps, 0);
	media_type = gst_structure_get_name(structure);

	if (media_type == NULL) {
		_mmstreamrec_dbg_log("failed to get media_type");
		return MM_PIXEL_FORMAT_INVALID;
	}

	if (!strcmp(media_type, "image/jpeg")) {
		_mmstreamrec_dbg_log("It is jpeg.");
		type = MM_PIXEL_FORMAT_ENCODED;
	} else if (!strcmp(media_type, "video/x-raw-yuv")) {
		_mmstreamrec_dbg_log("It is yuv.");
		gst_video_info_init(&media_info);
		gst_video_info_from_caps(&media_info, caps);
		fourcc = gst_video_format_to_fourcc(GST_VIDEO_INFO_FORMAT(&media_info));
		type = _mmstreamrecorder_get_pixtype(fourcc);
	} else if (!strcmp(media_type, "video/x-raw-rgb")) {
		_mmstreamrec_dbg_log("It is rgb.");
		type = MM_PIXEL_FORMAT_RGB888;
	} else {
		_mmstreamrec_dbg_err("Not supported format");
		type = MM_PIXEL_FORMAT_INVALID;
	}

	_mmstreamrec_dbg_log("Type [%d]", type);

	gst_caps_unref(caps);
	caps = NULL;

	return type;
}

unsigned int _mmstreamrecorder_get_fourcc(int pixtype, int codectype, int use_zero_copy_format)
{
	unsigned int fourcc = 0;

	_mmstreamrec_dbg_log("pixtype(%d)", pixtype);

	switch (pixtype) {
	case MM_PIXEL_FORMAT_NV12:
		if (use_zero_copy_format)
			fourcc = GST_MAKE_FOURCC('S', 'N', '1', '2');
		else
			fourcc = GST_MAKE_FOURCC('N', 'V', '1', '2');
		break;
	case MM_PIXEL_FORMAT_YUYV:
		if (use_zero_copy_format)
			fourcc = GST_MAKE_FOURCC('S', 'U', 'Y', 'V');
		else
			fourcc = GST_MAKE_FOURCC('Y', 'U', 'Y', '2');
		break;
	case MM_PIXEL_FORMAT_UYVY:
		if (use_zero_copy_format)
			fourcc = GST_MAKE_FOURCC('S', 'Y', 'V', 'Y');
		else
			fourcc = GST_MAKE_FOURCC('U', 'Y', 'V', 'Y');

		break;
	case MM_PIXEL_FORMAT_I420:
		if (use_zero_copy_format)
			fourcc = GST_MAKE_FOURCC('S', '4', '2', '0');
		else
			fourcc = GST_MAKE_FOURCC('I', '4', '2', '0');
		break;
	case MM_PIXEL_FORMAT_YV12:
		fourcc = GST_MAKE_FOURCC('Y', 'V', '1', '2');
		break;
	case MM_PIXEL_FORMAT_422P:
		fourcc = GST_MAKE_FOURCC('4', '2', '2', 'P');
		break;
	case MM_PIXEL_FORMAT_RGB565:
		fourcc = GST_MAKE_FOURCC('R', 'G', 'B', 'P');
		break;
	case MM_PIXEL_FORMAT_RGB888:
		fourcc = GST_MAKE_FOURCC('R', 'G', 'B', ' ');
		break;
	case MM_PIXEL_FORMAT_ENCODED:
		if (codectype == MM_IMAGE_CODEC_JPEG) {
			fourcc = GST_MAKE_FOURCC('J', 'P', 'E', 'G');
		} else if (codectype == MM_IMAGE_CODEC_JPEG_SRW) {
			fourcc = GST_MAKE_FOURCC('J', 'P', 'E', 'G');	/*TODO: JPEG+SamsungRAW format */
		} else if (codectype == MM_IMAGE_CODEC_SRW) {
			fourcc = GST_MAKE_FOURCC('J', 'P', 'E', 'G');	/*TODO: SamsungRAW format */
		} else if (codectype == MM_IMAGE_CODEC_PNG) {
			fourcc = GST_MAKE_FOURCC('P', 'N', 'G', ' ');
		} else {
			/* Please let us know what other fourcces are. ex) BMP, GIF? */
			fourcc = GST_MAKE_FOURCC('J', 'P', 'E', 'G');
		}
		break;
	case MM_PIXEL_FORMAT_ITLV_JPEG_UYVY:
		fourcc = GST_MAKE_FOURCC('I', 'T', 'L', 'V');
		break;
	default:
		_mmstreamrec_dbg_log("Not proper pixel type[%d]. Set default - I420", pixtype);
		if (use_zero_copy_format)
			fourcc = GST_MAKE_FOURCC('S', '4', '2', '0');
		else
			fourcc = GST_MAKE_FOURCC('I', '4', '2', '0');
		break;
	}

	return fourcc;
}

int _mmstreamrecorder_get_pixtype(unsigned int fourcc)
{
	int pixtype = MM_PIXEL_FORMAT_INVALID;
/*
	char *pfourcc = (char*)&fourcc;
	_mmstreamrec_dbg_log("fourcc(%c%c%c%c)", pfourcc[0], pfourcc[1], pfourcc[2], pfourcc[3]);
*/
	switch (fourcc) {
	case GST_MAKE_FOURCC('S', 'N', '1', '2'):
	case GST_MAKE_FOURCC('N', 'V', '1', '2'):
		pixtype = MM_PIXEL_FORMAT_NV12;
		break;
	case GST_MAKE_FOURCC('S', 'U', 'Y', 'V'):
	case GST_MAKE_FOURCC('Y', 'U', 'Y', 'V'):
	case GST_MAKE_FOURCC('Y', 'U', 'Y', '2'):
		pixtype = MM_PIXEL_FORMAT_YUYV;
		break;
	case GST_MAKE_FOURCC('S', 'Y', 'V', 'Y'):
	case GST_MAKE_FOURCC('U', 'Y', 'V', 'Y'):
		pixtype = MM_PIXEL_FORMAT_UYVY;
		break;
	case GST_MAKE_FOURCC('S', '4', '2', '0'):
	case GST_MAKE_FOURCC('I', '4', '2', '0'):
		pixtype = MM_PIXEL_FORMAT_I420;
		break;
	case GST_MAKE_FOURCC('Y', 'V', '1', '2'):
		pixtype = MM_PIXEL_FORMAT_YV12;
		break;
	case GST_MAKE_FOURCC('4', '2', '2', 'P'):
		pixtype = MM_PIXEL_FORMAT_422P;
		break;
	case GST_MAKE_FOURCC('R', 'G', 'B', 'P'):
		pixtype = MM_PIXEL_FORMAT_RGB565;
		break;
	case GST_MAKE_FOURCC('R', 'G', 'B', '3'):
		pixtype = MM_PIXEL_FORMAT_RGB888;
		break;
	case GST_MAKE_FOURCC('A', 'R', 'G', 'B'):
	case GST_MAKE_FOURCC('x', 'R', 'G', 'B'):
		pixtype = MM_PIXEL_FORMAT_ARGB;
		break;
	case GST_MAKE_FOURCC('B', 'G', 'R', 'A'):
	case GST_MAKE_FOURCC('B', 'G', 'R', 'x'):
		pixtype = MM_PIXEL_FORMAT_RGBA;
		break;
	case GST_MAKE_FOURCC('J', 'P', 'E', 'G'):
	case GST_MAKE_FOURCC('P', 'N', 'G', ' '):
		pixtype = MM_PIXEL_FORMAT_ENCODED;
		break;
	 /*FIXME*/ case GST_MAKE_FOURCC('I', 'T', 'L', 'V'):
		pixtype = MM_PIXEL_FORMAT_ITLV_JPEG_UYVY;
		break;
	default:
		_mmstreamrec_dbg_log("Not supported fourcc type(%x)", fourcc);
		pixtype = MM_PIXEL_FORMAT_INVALID;
		break;
	}

	return pixtype;
}

gboolean _mmstreamrecorder_resize_frame(unsigned char *src_data, unsigned int src_width, unsigned int src_height, unsigned int src_length, int src_format, unsigned char **dst_data, unsigned int *dst_width, unsigned int *dst_height, unsigned int *dst_length)
{
	int ret = TRUE;
	int mm_ret = MM_ERROR_NONE;
	int input_format = MM_UTIL_IMG_FMT_YUV420;
	unsigned char *dst_tmp_data = NULL;

	if (!src_data || !dst_data || !dst_width || !dst_height || !dst_length) {
		_mmstreamrec_dbg_err("something is NULL %p,%p,%p,%p,%p", src_data, dst_data, dst_width, dst_height, dst_length);
		return FALSE;
	}

	/* set input format for mm-util */
	switch (src_format) {
	case MM_PIXEL_FORMAT_I420:
		input_format = MM_UTIL_IMG_FMT_I420;
		break;
	case MM_PIXEL_FORMAT_YV12:
		input_format = MM_UTIL_IMG_FMT_YUV420;
		break;
	case MM_PIXEL_FORMAT_NV12:
		input_format = MM_UTIL_IMG_FMT_NV12;
		break;
	case MM_PIXEL_FORMAT_YUYV:
		input_format = MM_UTIL_IMG_FMT_YUYV;
		break;
	case MM_PIXEL_FORMAT_UYVY:
		input_format = MM_UTIL_IMG_FMT_UYVY;
		break;
	case MM_PIXEL_FORMAT_RGB888:
		input_format = MM_UTIL_IMG_FMT_RGB888;
		break;
	default:
		_mmstreamrec_dbg_err("NOT supported format", src_format);
		return FALSE;
	}

	_mmstreamrec_dbg_log("src size %dx%d -> dst size %dx%d", src_width, src_height, *dst_width, *dst_height);

	/* get length of resized image */
	mm_ret = mm_util_get_image_size(input_format, *dst_width, *dst_height, dst_length);
	if (mm_ret != MM_ERROR_NONE) {
		GST_ERROR("mm_util_get_image_size failed 0x%x", ret);
		return FALSE;
	}

	_mmstreamrec_dbg_log("dst_length : %d", *dst_length);

	dst_tmp_data = (unsigned char *)malloc(*dst_length);
	if (dst_tmp_data == NULL) {
		_mmstreamrec_dbg_err("failed to alloc dst_thumb_size(size %d)", *dst_length);
		return FALSE;
	}

	mm_ret = mm_util_resize_image(src_data, src_width, src_height, input_format, dst_tmp_data, dst_width, dst_height);
	if (mm_ret != MM_ERROR_NONE) {
		GST_ERROR("mm_util_resize_image failed 0x%x", ret);
		free(dst_tmp_data);
		return FALSE;
	}

	*dst_data = dst_tmp_data;

	_mmstreamrec_dbg_log("resize done %p, %dx%d", *dst_data, *dst_width, *dst_height);

	return TRUE;
}

/* make UYVY smaller as multiple size. ex: 640x480 -> 320x240 or 160x120 ... */
gboolean _mmstreamrecorder_downscale_UYVYorYUYV(unsigned char *src_data, unsigned int src_width, unsigned int src_height, unsigned char **dst_data, unsigned int dst_width, unsigned int dst_height)
{
	int i = 0;
	int src_length = 0;
	int dst_length = 0;
	int src_index = 0;
	int ratio_width = 0;
	int ratio_height = 0;
	int skip_height_num = 0;
	unsigned int byte_width = 0;
	unsigned char *result = NULL;

	if (src_data == NULL || dst_data == NULL) {
		_mmstreamrec_dbg_err("src[%p] or dst[%p] is NULL", src_data, dst_data);
		return FALSE;
	}

	result = (unsigned char *)malloc((dst_width * dst_height) << 1);
	if (!result) {
		_mmstreamrec_dbg_err("failed to alloc dst data");
		return FALSE;
	}

	ratio_width = src_width / dst_width;
	ratio_height = src_height / dst_height;
	skip_height_num = ratio_height - 1;
	src_length = (src_width * src_height) << 1;
	dst_length = (dst_width * dst_height) << 1;
	byte_width = dst_width << 1;

	for (i = 0; i < dst_length; i += 4) {
		src_index = (i * ratio_width) + ((((i / byte_width) * src_width) << 1) * skip_height_num);
		result[i] = src_data[src_index];
		result[i + 1] = src_data[src_index + 1];
		result[i + 2] = src_data[src_index + 2];
		result[i + 3] = src_data[src_index + 3];
	}

	*dst_data = result;

	return TRUE;
}

guint16 get_language_code(const char *str)
{
	return (guint16) (((str[0] - 0x60) & 0x1F) << 10) + (((str[1] - 0x60) & 0x1F) << 5) + ((str[2] - 0x60) & 0x1F);
}

gchar *str_to_utf8(const gchar * str)
{
	return g_convert(str, -1, "UTF-8", "ASCII", NULL, NULL, NULL);
}

inline gboolean write_tag(FILE *f, const gchar *tag)
{
	while (*tag)
		FPUTC_CHECK(*tag++, f);

	return TRUE;
}

inline gboolean write_to_32(FILE *f, guint val)
{
	FPUTC_CHECK(val >> 24, f);
	FPUTC_CHECK(val >> 16, f);
	FPUTC_CHECK(val >> 8, f);
	FPUTC_CHECK(val, f);
	return TRUE;
}

inline gboolean write_to_16(FILE *f, guint val)
{
	FPUTC_CHECK(val >> 8, f);
	FPUTC_CHECK(val, f);
	return TRUE;
}

inline gboolean write_to_24(FILE *f, guint val)
{
	write_to_16(f, val >> 8);
	FPUTC_CHECK(val, f);
	return TRUE;
}
