
include $(top_srcdir)/Makefile.inc

noinst_LTLIBRARIES = libcdx_sstr_parser.la

## set the source files.
libcdx_sstr_parser_la_SOURCES =  AwSstrParser.c \
				AwSstrIsmc.c \
				AwSstrUtils.c

if BOARD_USE_PLAYREADY
libcdx_sstr_parser_la_SOURCES += license.c
endif

libcdx_sstr_parser_la_CFLAGS = $(CFLAGS_CDXG)
LOCAL_INCLUDE = -I../include \
		-I../../include \
		-I../../base/include \
		-I../../stream/include \
		-I$(top_srcdir)/ \
		-I$(top_srcdir)/external/include/adecoder/ \
		-I$(top_srcdir)/external/include/zlib/ \
		-I$(top_srcdir)/external/include/ \
		-I$(top_srcdir)/../libcedarc/include

libcdx_sstr_parser_la_CFLAGS += $(LOCAL_INCLUDE)


libcdx_sstr_parser_la_LDFLAGS = $(LDFLAGS_CDXG)

libcdx_sstr_parser_la_LIBADD =

LOCAL_LIB = -lxml2
libcdx_sstr_parser_la_LDFLAGS += $(LOCAL_LIB)
