bin_PROGRAMS = mm_streamrecorder_testsuite

mm_streamrecorder_testsuite_SOURCES = mm_streamrecorder_testsuite.c

mm_streamrecorder_testsuite_CFLAGS = -I$(srcdir)/../src/include \
			     $(GLIB_CFLAGS)\
			     $(GST_CFLAGS)\
			     $(MM_COMMON_CFLAGS)\
			     $(MM_SOUND_CFLAGS)\
			     $(MDM_CFLAGS)


############################################
#mm_streamrecorder_testsuite_CFLAGS += -DAPPSRC_TEST
############################################

mm_streamrecorder_testsuite_DEPENDENCIES = $(top_builddir)/src/libmmfstreamrecorder.la
mm_streamrecorder_testsuite_LDADD = $(top_builddir)/src/libmmfstreamrecorder.la \
			     $(GLIB_LIBS)\
			     $(GST_LIBS)\
			     $(MM_COMMON_LIBS)\
			     $(MM_SOUND_LIBS)\
			     $(MDM_LIBS)


