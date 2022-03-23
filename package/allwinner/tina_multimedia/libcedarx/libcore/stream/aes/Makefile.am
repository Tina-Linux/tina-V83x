
include $(top_srcdir)/Makefile.inc

noinst_LTLIBRARIES = libcdx_aes_stream.la

## set the source files.
libcdx_aes_stream_la_SOURCES =  CdxAesStream.c

libcdx_aes_stream_la_CFLAGS = $(CFLAGS_CDXG)
LOCAL_INCLUDE = -I../include \
		-I../../include \
		-I../../base/include \
		-I$(top_srcdir)/

libcdx_aes_stream_la_CFLAGS += $(LOCAL_INCLUDE)

libcdx_aes_stream_la_LDFLAGS = $(LDFLAGS_CDXG)

libcdx_aes_stream_la_LIBADD = $(top_srcdir)/libcore/base/libcdx_base.la

libcdx_aes_stream_la_LDFLAGS += $(LOCAL_LIB)
