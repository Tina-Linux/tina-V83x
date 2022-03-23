
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = muxdemo

muxdemo_SOURCES =  MuxerWriter.c \
			testVideoMuxer.c

muxdemo_CFLAGS = $(CFLAGS_CDXG)
muxdemo_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/libcore/include \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/muxer/include \
		-I$(top_srcdir)/external/include/aencoder \
		-I$(top_srcdir)/../libcedarc/include

muxdemo_CFLAGS += $(LOCAL_INCLUDE)
muxdemo_CPPFLAGS += $(LOCAL_INCLUDE)

muxdemo_LDFLAGS = $(LDFLAGS_CDXG)

muxdemo_LDADD = \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/muxer/base/libcdx_muxer.la

LOCAL_LIB = -lpthread -lz -lVE -lvencoder -laencoder -lMemAdapter -lcdc_base

muxdemo_LDFLAGS += $(LOCAL_LIB)
