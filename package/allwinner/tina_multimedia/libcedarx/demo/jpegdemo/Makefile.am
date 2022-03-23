
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = jpegdemo

jpegdemo_SOURCES =  ./demojpeg.c

jpegdemo_CFLAGS = $(CFLAGS_CDXG)
jpegdemo_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/xmetadataretriever  \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/stream/include \
		-I$(top_srcdir)/libcore/parser/include \
		-I$(top_srcdir)/external/include/adecoder \
		-I$(top_srcdir)/../libcedarc/include

jpegdemo_CFLAGS += $(LOCAL_INCLUDE)
jpegdemo_CPPFLAGS += $(LOCAL_INCLUDE)

jpegdemo_LDFLAGS = $(LDFLAGS_CDXG)

jpegdemo_LDADD = $(top_srcdir)/libcore/parser/base/libcdx_parser.la \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/stream/base/libcdx_stream.la \
			$(top_srcdir)/xmetadataretriever/libxmetadataretriever.la \
			$(top_srcdir)/libcore/common/libcdx_common.la

LOCAL_LIB = -lpthread -lz -lvdecoder -lvencoder -lVE \
		-lcrypto -lMemAdapter -lcdc_base -lvideoengine
jpegdemo_LDFLAGS += $(LOCAL_LIB)
