
include $(top_srcdir)/Makefile.inc

lib_LTLIBRARIES = libvenc_base.la

libvenc_base_la_SOURCES = FrameBufferManager.c \
                         BitstreamManager.c \
                         EncAdapter.c


libvenc_base_la_CFLAGS = $(CFLAGS_CDCG)
LOCAL_INCLUDE = -I./include \
				-I$(top_srcdir)/include/ \
				-I$(top_srcdir)/base/include \
				-I$(top_srcdir)/ve/include

libvenc_base_la_CFLAGS += $(LOCAL_INCLUDE)


libvenc_base_la_LDFLAGS = $(LDFLAGS_CDCG)

libvenc_base_la_LIBADD = $(top_srcdir)/base/libcdc_base.la \
						$(top_srcdir)/memory/libMemAdapter.la

LOCAL_LIB = -lm -lrt -lVE
libvenc_base_la_LDFLAGS += $(LOCAL_LIB)
