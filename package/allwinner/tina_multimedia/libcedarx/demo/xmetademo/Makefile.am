
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = retriverdemo

retriverdemo_SOURCES =  ./xmetademo.c

retriverdemo_CFLAGS = $(CFLAGS_CDXG)
retriverdemo_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/xmetadataretriever/include  \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/stream/include \
		-I$(top_srcdir)/libcore/parser/include \
		-I$(top_srcdir)/external/include/adecoder \
		-I$(top_srcdir)/../libcedarc/include

retriverdemo_CFLAGS += $(LOCAL_INCLUDE)
retriverdemo_CPPFLAGS += $(LOCAL_INCLUDE)

retriverdemo_LDFLAGS = $(LDFLAGS_CDXG)

retriverdemo_LDADD = $(top_srcdir)/libcore/parser/base/libcdx_parser.la \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/stream/base/libcdx_stream.la \
			$(top_srcdir)/xmetadataretriever/libxmetadataretriever.la \
			$(top_srcdir)/libcore/common/libcdx_common.la

LOCAL_LIB = -lpthread -lz -lvdecoder -lvencoder -ladecoder -lsubdecoder -lVE \
		-lcrypto -lMemAdapter -lcdc_base -lvideoengine
retriverdemo_LDFLAGS += $(LOCAL_LIB)
