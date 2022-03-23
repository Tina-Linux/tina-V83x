
include $(top_srcdir)/Makefile.inc

lib_LTLIBRARIES = libvencoder.la

libvencoder_la_SOURCES = vencoder.c

libvencoder_la_CFLAGS = $(CFLAGS_CDCG)

LOCAL_INCLUDE = -I$(top_srcdir)/include \
				-I$(top_srcdir)/vencoder/base/include \
				-I$(top_srcdir)/base/include \
				-I$(top_srcdir)/ve/include

libvencoder_la_CFLAGS += $(LOCAL_INCLUDE)


libvencoder_la_LDFLAGS = $(LDFLAGS_CDCG)

libvencoder_la_LIBADD = $(top_srcdir)/base/libcdc_base.la \
						$(top_srcdir)/memory/libMemAdapter.la \
						$(top_srcdir)/vencoder/base/libvenc_base.la

LOCAL_LIB = -lm -lrt -lVE -lvenc_codec

libvencoder_la_LDFLAGS += $(LOCAL_LIB)
