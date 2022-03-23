
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


LOCAL_LIB = -lpthread -lm -lcdc_base -lVE -lvencoder -lMemAdapter

demoVencoder_LDFLAGS += $(LOCAL_LIB)
