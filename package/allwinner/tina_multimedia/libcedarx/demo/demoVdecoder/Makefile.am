
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = vdecoderdemo

vdecoderdemo_SOURCES =  ./demoVdecoder.c

vdecoderdemo_CFLAGS = $(CFLAGS_CDXG)
#jpegdemo_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/stream/include \
		-I$(top_srcdir)/libcore/parser/include \
		-I$(top_srcdir)/libcore/common/iniparser \
		-I$(top_srcdir)/external/include/adecoder \
		-I$(top_srcdir)/external/include \
		-I$(top_srcdir)/../libcedarc/include

vdecoderdemo_CFLAGS += $(LOCAL_INCLUDE)

vdecoderdemo_LDFLAGS = $(LDFLAGS_CDXG)

vdecoderdemo_LDADD = $(top_srcdir)/libcore/parser/base/libcdx_parser.la \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/stream/base/libcdx_stream.la \
			$(top_srcdir)/libcore/common/libcdx_common.la

LOCAL_LIB = -lpthread -lz -lssl -lvdecoder  -lVE \
		-lcrypto -lMemAdapter -lcdc_base -lvideoengine
vdecoderdemo_LDFLAGS += $(LOCAL_LIB)
