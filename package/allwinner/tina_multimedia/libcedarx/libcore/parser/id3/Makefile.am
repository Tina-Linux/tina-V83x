
include $(top_srcdir)/Makefile.inc

noinst_LTLIBRARIES = libcdx_id3_parser.la

## set the source files.
libcdx_id3_parser_la_SOURCES =  CdxId3Parser.c

libcdx_id3_parser_la_CFLAGS = $(CFLAGS_CDXG)
LOCAL_INCLUDE = -I../include \
		-I../../include \
		-I../../base/include \
		-I../../stream/include \
		-I$(top_srcdir)/ \
		-I$(top_srcdir)/external/include/adecoder/ \
		-I$(top_srcdir)/../libcedarc/include

libcdx_id3_parser_la_CFLAGS += $(LOCAL_INCLUDE)


libcdx_id3_parser_la_LDFLAGS = $(LDFLAGS_CDXG)

libcdx_id3_parser_la_LIBADD =

#LOCAL_LIB = -lpthread
libcdx_id3_parser_la_LDFLAGS += $(LOCAL_LIB)
