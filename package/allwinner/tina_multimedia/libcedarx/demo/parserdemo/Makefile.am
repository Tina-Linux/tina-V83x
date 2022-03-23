
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = parserdemo

parserdemo_SOURCES =  testParser.c

parserdemo_CFLAGS = $(CFLAGS_CDXG)
parserdemo_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/libcore/include \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/parser/include \
		-I$(top_srcdir)/libcore/stream/include \
		-I$(top_srcdir)/../libcedarc/include \
		-I$(top_srcdir)/external/include/adecoder

parserdemo_CFLAGS += $(LOCAL_INCLUDE)
parserdemo_CPPFLAGS += $(LOCAL_INCLUDE)

parserdemo_LDFLAGS = $(LDFLAGS_CDXG)

parserdemo_LDADD = \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/stream/base/libcdx_stream.la \
		 $(top_srcdir)/libcore/parser/base/libcdx_parser.la

LOCAL_LIB = -lpthread -lz -lssl -lcrypto

parserdemo_LDFLAGS += $(LOCAL_LIB)
