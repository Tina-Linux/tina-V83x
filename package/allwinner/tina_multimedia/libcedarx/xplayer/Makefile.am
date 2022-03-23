
include $(top_srcdir)/Makefile.inc

lib_LTLIBRARIES = libxplayer.la

libxplayer_la_SOURCES =  xplayer.c \
			./cache.c \
			./demuxComponent.c

libxplayer_la_CFLAGS = $(CFLAGS_CDXG)

if TEST_ONLY_ENABLE_AUDIO
LOCAL_INCLUDE = -I$(top_srcdir) \
                -I$(top_srcdir)/xplayer/include \
		-I$(top_srcdir)/libcore/common/iniparser \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/stream/include \
		-I$(top_srcdir)/libcore/playback/include \
		-I$(top_srcdir)/libcore/parser/include \
		-I$(top_srcdir)/external/include/adecoder
else
LOCAL_INCLUDE = -I$(top_srcdir) \
                -I$(top_srcdir)/xplayer/include \
		-I$(top_srcdir)/libcore/common/iniparser \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/stream/include \
		-I$(top_srcdir)/libcore/playback/include \
		-I$(top_srcdir)/libcore/parser/include \
		-I$(top_srcdir)/external/include/adecoder \
		-I$(top_srcdir)/../libcedarc/include
endif

libxplayer_la_CFLAGS += $(LOCAL_INCLUDE)

libxplayer_la_LDFLAGS = $(LDFLAGS_CDXG)

libxplayer_la_LIBADD = $(top_srcdir)/libcore/parser/base/libcdx_parser.la \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/stream/base/libcdx_stream.la \
			$(top_srcdir)/libcore/playback/libcdx_playback.la \
			$(top_srcdir)/libcore/common/libcdx_common.la

if TEST_ONLY_ENABLE_AUDIO
LOCAL_LIB = -lpthread -lz -ladecoder
else
LOCAL_LIB = -lpthread -lz -lvdecoder -ladecoder -lsubdecoder
endif
libxplayer_la_LDFLAGS += $(LOCAL_LIB)
