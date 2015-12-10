ACLOCAL_AMFLAGS='-I m4'

lib_LTLIBRARIES = libmmfstreamrecorder.la

includelibmmfstreamrecorderdir = $(includedir)/mmf

includelibmmfstreamrecorder_HEADERS = include/mm_streamrecorder.h

noinst_HEADERS = include/mm_streamrecorder.h \
		 include/mm_streamrecorder_internal.h	\
		 include/mm_streamrecorder_fileinfo.h	\
		 include/mm_streamrecorder_util.h \
		 include/mm_streamrecorder_gstdispatch.h \
		 include/mm_streamrecorder_attribute.h \
		 include/mm_streamrecorder_video.h \
		 include/mm_streamrecorder_gstcommon.h \
		 include/mm_streamrecorder_recorder.h \
		 include/mm_streamrecorder_audio.h \
		 include/mm_streamrecorder_buffer_manager.h\
		 include/mm_streamrecorder_ini.h

libmmfstreamrecorder_la_SOURCES = mm_streamrecorder.c \
			     mm_streamrecorder_internal.c	\
			     mm_streamrecorder_fileinfo.c	\
			     mm_streamrecorder_util.c \
			     mm_streamrecorder_gstdispatch.c \
			     mm_streamrecorder_attribute.c \
			     mm_streamrecorder_video.c \
			     mm_streamrecorder_gstcommon.c \
			     mm_streamrecorder_recorder.c \
			     mm_streamrecorder_audio.c \
			     mm_streamrecorder_buffer_manager.c\
			     mm_streamrecorder_ini.c

libmmfstreamrecorder_la_CFLAGS = -I$(srcdir)/include \
			     $(GST_CFLAGS) \
			     $(GST_PLUGIN_BASE_CFLAGS) \
			     $(GST_VIDEO_CFLAGS) \
			     $(GST_INTERFACES_CFLAGS) \
			     $(MM_LOG_CFLAGS) \
			     $(MM_COMMON_CFLAGS) \
			     $(GST_APP_CFLAGS)

libmmfstreamrecorder_la_LIBADD = \
			    $(GST_LIBS) \
			    $(GST_PLUGIN_BASE_LIBS) \
			    $(GST_VIDEO_LIBS) \
			    $(GST_INTERFACES_LIBS) \
			    $(MM_COMMON_LIBS) \
			    $(MM_LOG_LIBS) \
			    $(VCONF_LIBS) \
			    $(GST_APP_LIBS) \
			    $(INIPARSER_LIBS)

libmmfstreamrecorder_la_CFLAGS += $(MMLOGSVR_CFLAGS) -DMMF_LOG_OWNER=0x00400000 -DMMF_DEBUG_PREFIX=\"MMF-STREAMRECORDER\"
libmmfstreamrecorder_la_LIBADD += $(MMLOGSVR_LIBS)

