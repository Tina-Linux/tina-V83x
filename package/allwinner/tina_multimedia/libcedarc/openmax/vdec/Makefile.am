
include $(top_srcdir)/Makefile.inc

lib_LTLIBRARIES = libOmxVdec.la

libOmxVdec_la_SOURCES = src/omx_vdec.c src/omx_vdec_port.c \
                        src/omx_vdec_aw_decoder_linux.c \
                        looper/omx_thread.c \
                        looper/omx_mutex.c \
                        looper/omx_sem.c

libOmxVdec_la_CFLAGS = $(CFLAGS_CDCG)
libOmxVdec_la_CPPFLAGS = $(CPPFLAGS_CDCG)
LOCAL_INCLUDE = -I../include \
                -I./inc \
                -I./looper \
                -I$(top_srcdir)/include \
                -I$(top_srcdir)/openmax/omxcore/include  \
                -I$(top_srcdir)/openmax/include  \
                -I$(top_srcdir)/openmax/vdec/inc  \
                -I$(top_srcdir)/openmax/vdec/looper  \
                -I$(top_srcdir)/base/include \
                -I$(top_srcdir)/ve/include

libOmxVdec_la_CFLAGS += $(LOCAL_INCLUDE)
libOmxVdec_la_CPPFLAGS += $(LOCAL_INCLUDE)

#libOmxVdec_la_CFLAGS += -DCONF_OMX_USE_ZERO_COPY

libOmxVdec_la_LDFLAGS = $(LDFLAGS_CDCG)

libOmxVdec_la_LIBADD = $(top_builddir)/base/libcdc_base.la \
                       $(top_builddir)/vdecoder/libvdecoder.la \
                       $(top_builddir)/memory/libMemAdapter.la
LOCAL_LIB = -ldl -lrt -lpthread -lVE -lvideoengine
libOmxVdec_la_LDFLAGS += $(LOCAL_LIB)
