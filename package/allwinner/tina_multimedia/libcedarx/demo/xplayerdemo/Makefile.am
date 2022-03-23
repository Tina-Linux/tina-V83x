
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = xplayerdemo

xplayerdemo_SOURCES =  ./xplayerdemo.c \
			./layerControl_null.c \
			./subtitleControl_null.c \
			./deinterlace_null.c \
			./tinasoundcontrol.c \
			./CdxIon.c

if XPLAYERDEMO_DE2_ENABLE
xplayerdemo_SOURCES += ./layerControl_de20.c
else
xplayerdemo_SOURCES += ./layerControl_de.c
endif

#			./tinasoundcontrol.c \
#			./soundControl_null.c
#			./soundControl_alsa.c

xplayerdemo_CFLAGS = $(CFLAGS_CDXG)
#xplayerdemo_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/xplayer/include  \
		-I$(top_srcdir)/libcore/common/iniparser \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/stream/include \
		-I$(top_srcdir)/libcore/playback/include \
		-I$(top_srcdir)/libcore/parser/include \
		-I$(top_srcdir)/external/include/adecoder \
		-I$(top_srcdir)/external/include/   \
		-I$(top_srcdir)/../libcedarc/include

xplayerdemo_CFLAGS += $(LOCAL_INCLUDE)
#xplayerdemo_CPPFLAGS += $(LOCAL_INCLUDE)

xplayerdemo_LDFLAGS = $(LDFLAGS_CDXG)

xplayerdemo_LDADD = $(top_srcdir)/libcore/parser/base/libcdx_parser.la \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/stream/base/libcdx_stream.la \
			$(top_srcdir)/libcore/playback/libcdx_playback.la \
			$(top_srcdir)/xplayer/libxplayer.la \
			$(top_srcdir)/libcore/common/libcdx_common.la

LOCAL_LIB = -ldl -lrt -lpthread -lz -lm -lvdecoder -ladecoder -lm -lsubdecoder -lVE \
		-lcrypto -lMemAdapter -lcdc_base -lssl -lvideoengine -lasound
#-lasound
#-lsalsa
xplayerdemo_LDFLAGS += $(LOCAL_LIB)
