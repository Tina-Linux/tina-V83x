
include $(top_srcdir)/Makefile.inc

noinst_LTLIBRARIES = libcdx_customer_stream.la

## set the source files.
libcdx_customer_stream_la_SOURCES =  CustomerStream.c

libcdx_customer_stream_la_CFLAGS = $(CFLAGS_CDXG)
LOCAL_INCLUDE = -I../include \
		-I../../include \
		-I../../base/include \
		-I$(top_srcdir)/

libcdx_customer_stream_la_CFLAGS += $(LOCAL_INCLUDE)


libcdx_customer_stream_la_LDFLAGS = $(LDFLAGS_CDXG)

libcdx_customer_stream_la_LIBADD = $(top_srcdir)/libcore/base/libcdx_base.la

#LOCAL_LIB = -lpthread
libcdx_customer_stream_la_LDFLAGS += $(LOCAL_LIB)
