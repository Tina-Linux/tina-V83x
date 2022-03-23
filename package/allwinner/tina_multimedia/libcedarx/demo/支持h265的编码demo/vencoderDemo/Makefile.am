
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = demoVencoder

demoVencoder_SOURCES =  \
			EncoderTest.c

demoVencoder_CFLAGS = $(CFLAGS_CDCG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/base/include \
		-I$(top_srcdir)/include

demoVencoder_CFLAGS += $(LOCAL_INCLUDE)

demoVencoder_LDFLAGS = $(LDFLAGS_CDCG)


LOCAL_LIB = -lpthread -lm -lrt

demoVencoder_LDFLAGS += $(top_srcdir)/base/libcdc_base.la \
                            $(top_srcdir)/ve/libVE.la \
                            $(top_srcdir)/memory/libMemAdapter.la \
                            $(top_srcdir)/vencoder/libvencoder.la \
                            $(top_srcdir)/vencoder/base/libvenc_base.la \
                            $(top_srcdir)/vencoder/libcodec/libvenc_codec.la


demoVencoder_LDFLAGS += $(LOCAL_LIB)
