
include $(top_srcdir)/Makefile.inc

noinst_LTLIBRARIES = libcdx_ssl_stream.la

## set the source files.
libcdx_ssl_stream_la_SOURCES =  CdxSSLStream.c

libcdx_ssl_stream_la_CFLAGS = $(CFLAGS_CDXG)
LOCAL_INCLUDE = -I../include \
		-I../../include \
		-I../../base/include \
		-I$(top_srcdir)/

libcdx_ssl_stream_la_CFLAGS += $(LOCAL_INCLUDE)


libcdx_ssl_stream_la_LDFLAGS = $(LDFLAGS_CDXG)

libcdx_ssl_stream_la_LIBADD = $(top_srcdir)/libcore/base/libcdx_base.la

#LOCAL_LIB = -lpthread
LOCAL_LIB = -lssl -lcrypto
libcdx_ssl_stream_la_LDFLAGS += $(LOCAL_LIB)
