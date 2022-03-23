
include $(top_srcdir)/Makefile.inc

lib_LTLIBRARIES = libxmetadataretriever.la

libxmetadataretriever_la_SOURCES =  xmetadataretriever.c

libxmetadataretriever_la_CFLAGS = $(CFLAGS_CDXG)
libxmetadataretriever_la_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/stream/include \
		-I$(top_srcdir)/libcore/parser/include \
		-I$(top_srcdir)/libcore/common/iniparser \
		-I$(top_srcdir)/external/include/adecoder \
		-I$(top_srcdir)/../libcedarc/include \
		-I./include

##-I$(top_srcdir)/libcore/playback/include \

libxmetadataretriever_la_CFLAGS += $(LOCAL_INCLUDE)
libxmetadataretriever_la_CPPFLAGS += $(LOCAL_INCLUDE)

libxmetadataretriever_la_LDFLAGS = $(LDFLAGS_CDXG)

libxmetadataretriever_la_LIBADD = $(top_srcdir)/libcore/parser/base/libcdx_parser.la \
			$(top_srcdir)/libcore/stream/base/libcdx_stream.la \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/common/libcdx_common.la

#			$(top_srcdir)/libcore/playback/libcdx_playback.la

LOCAL_LIB = -lpthread -lz -lvdecoder -lMemAdapter -lvencoder
libxmetadataretriever_la_LDFLAGS += $(LOCAL_LIB)
