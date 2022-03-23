
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = demoOmxVdec

demoOmxVdec_SOURCES =   ./async_queue.c       \
                        ./OmxCodec.cpp        \
                        ./demoOmxVdec.cpp     \
                        ./sem.c

demoOmxVdec_CFLAGS = $(CFLAGS_CDXG)
demoOmxVdec_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/stream/include \
		-I$(top_srcdir)/libcore/parser/include \
		-I$(top_srcdir)/libcore/common/iniparser \
		-I$(top_srcdir)/libcore/playback/include/ \
		-I$(top_srcdir)/external/include/adecoder \
		-I$(top_srcdir)/external/include \
		-I$(top_srcdir)/../libcedarc/include \
		-I$(top_srcdir)/../libcedarc/openmax/omxcore/inc/

demoOmxVdec_CFLAGS += $(LOCAL_INCLUDE)
demoOmxVdec_CPPFLAGS += $(LOCAL_INCLUDE)

demoOmxVdec_LDFLAGS = $(LDFLAGS_CDXG)

demoOmxVdec_LDADD = $(top_srcdir)/libcore/parser/base/libcdx_parser.la \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/stream/base/libcdx_stream.la \
			$(top_srcdir)/libcore/common/libcdx_common.la

LOCAL_LIB = -lpthread -ldl -lz -lssl -lvdecoder -lVE  \
		-lcrypto -lMemAdapter -lcdc_base

demoOmxVdec_LDFLAGS += $(LOCAL_LIB)
