
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = vencoderdemo

vencoderdemo_SOURCES =  ./EncoderTest.c

vencoderdemo_CFLAGS = $(CFLAGS_CDXG)

LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/libcore/include \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/../libcedarc/include

vencoderdemo_CFLAGS += $(LOCAL_INCLUDE)

vencoderdemo_LDFLAGS = $(LDFLAGS_CDXG)

vencoderdemo_LDADD = $(top_srcdir)/libcore/base/libcdx_base.la

LOCAL_LIB = -lpthread -lvencoder  -lVE \
		 -lMemAdapter -lcdc_base
vencoderdemo_LDFLAGS += $(LOCAL_LIB)
