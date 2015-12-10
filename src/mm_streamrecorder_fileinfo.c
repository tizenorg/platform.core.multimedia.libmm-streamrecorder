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

#include "mm_streamrecorder_internal.h"
#include "mm_streamrecorder_util.h"
#include "mm_streamrecorder_fileinfo.h"

/*-----------------------------------------------------------------------
|    GLOBAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/

/*-----------------------------------------------------------------------
|    LOCAL VARIABLE DEFINITIONS for internal				|
-----------------------------------------------------------------------*/

/*---------------------------------------------------------------------------
|    LOCAL FUNCTION PROTOTYPES:												|
---------------------------------------------------------------------------*/
/* STATIC INTERNAL FUNCTION */

/*===========================================================================================
|																							|
|  FUNCTION DEFINITIONS																		|
========================================================================================== */
/*---------------------------------------------------------------------------
|    GLOBAL FUNCTION DEFINITIONS:											|
---------------------------------------------------------------------------*/

gboolean _mmstreamrecorder_find_fourcc(FILE *f, guint32 tag_fourcc, gboolean do_rewind)
{
	guchar buf[8];

	if (do_rewind)
		rewind(f);

	while (fread(&buf, sizeof(guchar), 8, f) > 0) {
		unsigned long long buf_size = 0;
		unsigned int buf_fourcc = MMSTREAMRECORDER_FOURCC(buf[4], buf[5], buf[6], buf[7]);

		if (tag_fourcc == buf_fourcc) {
			_mmstreamrec_dbg_log("find tag : %c%c%c%c", MMSTREAMRECORDER_FOURCC_ARGS(tag_fourcc));
			return TRUE;
		} else if ((buf_fourcc == MMSTREAMRECORDER_FOURCC('m', 'o', 'o', 'v')) && (tag_fourcc != buf_fourcc)) {
			if (_mmstreamrecorder_find_fourcc(f, tag_fourcc, FALSE))
				return TRUE;
			else
				continue;
		} else {
			_mmstreamrec_dbg_log("skip [%c%c%c%c] tag", MMSTREAMRECORDER_FOURCC_ARGS(buf_fourcc));

			buf_size = (unsigned long long)_mmstreamrecorder_get_container_size(buf);
			buf_size = buf_size - 8;	/* include tag */

			do {
				if (buf_size > _MMSTREAMRECORDER_MAX_INT) {
					_mmstreamrec_dbg_log("seek %d", _MMSTREAMRECORDER_MAX_INT);
					if (fseek(f, _MMSTREAMRECORDER_MAX_INT, SEEK_CUR) != 0) {
						_mmstreamrec_dbg_err("fseek() fail");
						return FALSE;
					}

					buf_size -= _MMSTREAMRECORDER_MAX_INT;
				} else {
					_mmstreamrec_dbg_log("seek %d", buf_size);
					if (fseek(f, buf_size, SEEK_CUR) != 0) {
						_mmstreamrec_dbg_err("fseek() fail");
						return FALSE;
					}
					break;
				}
			} while (TRUE);
		}
	}

	_mmstreamrec_dbg_log("cannot find tag : %c%c%c%c", MMSTREAMRECORDER_FOURCC_ARGS(tag_fourcc));

	return FALSE;
}

gboolean _mmstreamrecorder_update_composition_matrix(FILE *f, int orientation)
{
	/* for 0 degree */
	guint32 a = 0x00010000;
	guint32 b = 0;
	guint32 c = 0;
	guint32 d = 0x00010000;

	switch (orientation) {
	case MM_STREAMRECORDER_TAG_VIDEO_ORT_90:	/* 90 degree */
		a = 0;
		b = 0x00010000;
		c = 0xffff0000;
		d = 0;
		break;
	case MM_STREAMRECORDER_TAG_VIDEO_ORT_180:	/* 180 degree */
		a = 0xffff0000;
		d = 0xffff0000;
		break;
	case MM_STREAMRECORDER_TAG_VIDEO_ORT_270:	/* 270 degree */
		a = 0;
		b = 0xffff0000;
		c = 0x00010000;
		d = 0;
		break;
	case MM_STREAMRECORDER_TAG_VIDEO_ORT_NONE:	/* 0 degree */
	default:
		break;
	}

	write_to_32(f, a);
	write_to_32(f, b);
	write_to_32(f, 0);
	write_to_32(f, c);
	write_to_32(f, d);
	write_to_32(f, 0);
	write_to_32(f, 0);
	write_to_32(f, 0);
	write_to_32(f, 0x40000000);

	_mmstreamrec_dbg_log("orientation : %d, write data 0x%x 0x%x 0x%x 0x%x", orientation, a, b, c, d);

	return TRUE;
}

guint64 _mmstreamrecorder_get_container_size(const guchar *size)
{
	guint64 result = 0;
	guint64 temp = 0;

	temp = size[0];
	result = temp << 24;
	temp = size[1];
	result = result | (temp << 16);
	temp = size[2];
	result = result | (temp << 8);
	result = result | size[3];

	_mmstreamrec_dbg_log("result : %lld", (unsigned long long)result);

	return result;
}

gboolean _mmstreamrecorder_update_size(FILE *f, gint64 prev_pos, gint64 curr_pos)
{
	_mmstreamrec_dbg_log("size : %" G_GINT64_FORMAT "", curr_pos - prev_pos);
	if (fseek(f, prev_pos, SEEK_SET) != 0) {
		_mmstreamrec_dbg_err("fseek() fail");
		return FALSE;
	}

	if (!write_to_32(f, curr_pos - prev_pos))
		return FALSE;

	if (fseek(f, curr_pos, SEEK_SET) != 0) {
		_mmstreamrec_dbg_err("fseek() fail");
		return FALSE;
	}

	return TRUE;
}

gboolean _mmstreamrecorder_write_udta_m4a(FILE *f)
{
	gint64 current_pos, pos;

	_mmstreamrec_dbg_log("");

	if ((pos = ftell(f)) < 0) {
		_mmstreamrec_dbg_err("ftell() returns negative value");
		return FALSE;
	}

	if (!write_to_32(f, 0))		/* udta atomic size */
		return FALSE;

	if (!write_tag(f, "udta"))	/* user data fourcc */
		return FALSE;

	if ((current_pos = ftell(f)) < 0) {
		_mmstreamrec_dbg_err("ftell() returns negative value");
		return FALSE;
	}

	if (!_mmstreamrecorder_update_size(f, pos, current_pos))
		return FALSE;

	return TRUE;
}

gboolean _mmstreamrecorder_write_udta(FILE *f, _MMStreamRecorderLocationInfo info)
{
	gint64 current_pos, pos;

	_mmstreamrec_dbg_log("");

	if ((pos = ftell(f)) < 0) {
		_mmstreamrec_dbg_err("ftell() returns negative value");
		return FALSE;
	}

	if (!write_to_32(f, 0))		/*size */
		return FALSE;

	if (!write_tag(f, "udta"))	/* type */
		return FALSE;

	if ((current_pos = ftell(f)) < 0) {
		_mmstreamrec_dbg_err("ftell() returns negative value");
		return FALSE;
	}

	if (!_mmstreamrecorder_update_size(f, pos, current_pos))
		return FALSE;

	return TRUE;
}

/* START TAG HERE */
gboolean _mmstreamrecorder_audio_add_metadata_info_m4a(MMHandleType handle)
{
	FILE *f = NULL;
	guchar buf[4];
	guint64 udta_size = 0;
	gint64 current_pos = 0;
	gint64 moov_pos = 0;
	gint64 udta_pos = 0;
	char err_msg[128] = { '\0', };

	_MMStreamRecorderFileInfo *finfo = NULL;
	mmf_streamrecorder_t *hstreamrecorder = MMF_STREAMRECORDER(handle);
	_MMStreamRecorderSubContext *sc = NULL;

	mmf_return_val_if_fail(hstreamrecorder, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	sc = MMF_STREAMRECORDER_SUBCONTEXT(handle);

	mmf_return_val_if_fail(sc, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);
	mmf_return_val_if_fail(sc->info_file, MM_ERROR_STREAMRECORDER_NOT_INITIALIZED);

	finfo = sc->info_file;

	f = fopen(finfo->filename, "rb+");
	if (f == NULL) {
		strerror_r(errno, err_msg, 128);
		_mmstreamrec_dbg_err("file open failed [%s]", err_msg);
		return FALSE;
	}

	/* find udta container.
	   if, there are udta container, write loci box after that
	   else, make udta container and write loci box. */
	if (_mmstreamrecorder_find_fourcc(f, MMSTREAMRECORDER_FOURCC('u', 'd', 't', 'a'), TRUE)) {
		size_t nread = 0;

		_mmstreamrec_dbg_log("find udta container");

		/* read size */
		if (fseek(f, -8L, SEEK_CUR) != 0)
			goto fail;

		udta_pos = ftell(f);
		if (udta_pos < 0)
			goto ftell_fail;

		nread = fread(&buf, sizeof(char), sizeof(buf), f);

		_mmstreamrec_dbg_log("recorded file fread %d", nread);

		udta_size = _mmstreamrecorder_get_container_size(buf);

		/* goto end of udta and write 'smta' box */
		if (fseek(f, (udta_size - 4L), SEEK_CUR) != 0)
			goto fail;

		current_pos = ftell(f);
		if (current_pos < 0)
			goto ftell_fail;

		if (!_mmstreamrecorder_update_size(f, udta_pos, current_pos))
			goto fail;
	} else {
		_mmstreamrec_dbg_log("No udta container");
		if (fseek(f, 0, SEEK_END) != 0)
			goto fail;

		if (!_mmstreamrecorder_write_udta_m4a(f))
			goto fail;
	}

	/* find moov container.
	   update moov container size. */
	if ((current_pos = ftell(f)) < 0)
		goto ftell_fail;

	if (_mmstreamrecorder_find_fourcc(f, MMSTREAMRECORDER_FOURCC('m', 'o', 'o', 'v'), TRUE)) {

		_mmstreamrec_dbg_log("found moov container");
		if (fseek(f, -8L, SEEK_CUR) != 0)
			goto fail;

		moov_pos = ftell(f);
		if (moov_pos < 0)
			goto ftell_fail;

		if (!_mmstreamrecorder_update_size(f, moov_pos, current_pos))
			goto fail;

	} else {
		_mmstreamrec_dbg_err("No 'moov' container");
		goto fail;
	}

	fclose(f);
	return TRUE;

 fail:
	fclose(f);
	return FALSE;

 ftell_fail:
	_mmstreamrec_dbg_err("ftell() returns negative value.");
	fclose(f);
	return FALSE;
}

/* END TAG HERE */
