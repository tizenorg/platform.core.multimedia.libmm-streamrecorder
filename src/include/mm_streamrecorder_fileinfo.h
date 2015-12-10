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

#ifndef __MM_STREAMRECORDER_FILEINFO_H__
#define __MM_STREAMRECORDER_FILEINFO_H__

/*=======================================================================================
| INCLUDE FILES										|
========================================================================================*/

#ifdef __cplusplus
extern "C" {
#endif

/*=======================================================================================
| GLOBAL DEFINITIONS AND DECLARATIONS FOR STREAMRECORDER					|
========================================================================================*/

/*=======================================================================================
| MACRO DEFINITIONS									|
========================================================================================*/

#define MMSTREAMRECORDER_FOURCC(a,b,c,d)  (guint32)((a)|(b)<<8|(c)<<16|(d)<<24)
#define MMSTREAMRECORDER_FOURCC_ARGS(fourcc) \
        ((gchar)((fourcc)&0xff)), \
        ((gchar)(((fourcc)>>8)&0xff)), \
        ((gchar)(((fourcc)>>16)&0xff)), \
        ((gchar)(((fourcc)>>24)&0xff))

/*=======================================================================================
| ENUM DEFINITIONS									|
========================================================================================*/
/**
 *Type define of util.
 */
#define _OFFSET_COMPOSITION_MATRIX              40L

/*=======================================================================================
| STRUCTURE DEFINITIONS									|
========================================================================================*/

typedef struct {
	char *filename;		/**< recorded filename */
	guint64 filesize;	/**< current file size */
	guint64 max_size;	/**< max recording size */
	guint64 max_time;	/**< max recording time */
	int fileformat;		/**< recording file format */
} _MMStreamRecorderFileInfo;

/**
 * Structure of location info
 */
typedef struct {
	gint32 longitude;
	gint32 latitude;
	gint32 altitude;
} _MMStreamRecorderLocationInfo;

enum MMStreamReorderTagVideoOrientation {
	MM_STREAMRECORDER_TAG_VIDEO_ORT_NONE = 0,
											/**< No Orientation.*/
	MM_STREAMRECORDER_TAG_VIDEO_ORT_90,	/**< 90 degree */
	MM_STREAMRECORDER_TAG_VIDEO_ORT_180,/**< 180 degree */
	MM_STREAMRECORDER_TAG_VIDEO_ORT_270,/**< 270 degree */
};

/*=======================================================================================
| CONSTANT DEFINITIONS									|
========================================================================================*/

/**
 *
 */
#define MMFILE_FOURCC(a,b,c,d)  (guint32)((a)|(b)<<8|(c)<<16|(d)<<24)
#define MMFILE_FOURCC_ARGS(fourcc) \
			((gchar)((fourcc)&0xff)), \
			((gchar)(((fourcc)>>8)&0xff)), \
			((gchar)(((fourcc)>>16)&0xff)), \
			((gchar)(((fourcc)>>24)&0xff))

/*=======================================================================================
| GLOBAL FUNCTION PROTOTYPES								|
========================================================================================*/

/* Recording */
/* find top level tag only, do not use this function for finding sub level tags.
   tag_fourcc is Four-character-code (FOURCC) */

// READER
gboolean _mmstreamrecorder_find_fourcc(FILE * f, guint32 tag_fourcc, gboolean do_rewind);
gboolean _mmstreamrecorder_update_size(FILE * f, gint64 prev_pos, gint64 curr_pos);
guint64 _mmstreamrecorder_get_container_size(const guchar * size);

// WRITER
// COMMON
gboolean _mmstreamrecorder_write_udta(FILE * f, _MMStreamRecorderLocationInfo info);
gboolean _mmstreamrecorder_update_size(FILE * f, gint64 prev_pos, gint64 curr_pos);

// VIDEO FOURCC
gboolean _mmstreamrecorder_write_loci(FILE * f, _MMStreamRecorderLocationInfo info);
gboolean _mmstreamrecorder_write_udta(FILE * f, _MMStreamRecorderLocationInfo info);
gboolean _mmstreamrecorder_add_locationinfo(MMHandleType handle, int fileformat);

// AUDIO FOURCC
gboolean _mmstreamrecorder_write_udta_m4a(FILE * f);
gboolean _mmstreamrecorder_audio_add_metadata_info_m4a(MMHandleType handle);

#ifdef __cplusplus
}
#endif
#endif							/* __MM_STREAMRECORDER_FILEINFO_H__ */
