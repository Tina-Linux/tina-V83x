
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = demoOmxVenc

demoOmxVenc_SOURCES =   ./async_queue.c       \
                        ./OmxCodec.cpp        \
                        ./demoOmxVenc.cpp     \
                        ./sem.c

demoOmxVenc_CFLAGS = $(CFLAGS_CDXG)
demoOmxVenc_CPPFLAGS = $(CPPFLAGS_CDXG)
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

demoOmxVenc_CFLAGS += $(LOCAL_INCLUDE)
demoOmxVenc_CPPFLAGS += $(LOCAL_INCLUDE)

demoOmxVenc_LDFLAGS = $(LDFLAGS_CDXG)

demoOmxVenc_LDADD = $(top_srcdir)/libcore/parser/base/libcdx_parser.la \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/stream/base/libcdx_stream.la \
			$(top_srcdir)/libcore/common/libcdx_common.la

LOCAL_LIB = -lpthread -ldl -lz -lssl -lcdc_vdecoder -lcdc_ve  \
		-lcrypto -lcdc_memory -lcdc_base

demoOmxVenc_LDFLAGS += $(LOCAL_LIB)
