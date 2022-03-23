
include $(top_srcdir)/Makefile.inc

bin_PROGRAMS = recoderdemo

recoderdemo_SOURCES =  recoderdemo.c \
			RecoderWriter.c \
			CdxIon.c

recoderdemo_CFLAGS = $(CFLAGS_CDXG)
recoderdemo_CPPFLAGS = $(CPPFLAGS_CDXG)
LOCAL_INCLUDE = -I$(top_srcdir) \
		-I$(top_srcdir)/libcore/include \
		-I$(top_srcdir)/awrecorder  \
		-I$(top_srcdir)/libcore/base/include \
		-I$(top_srcdir)/libcore/muxer/include \
		-I$(top_srcdir)/external/include/aencoder \
		-I$(top_srcdir)/../libcedarc/include

recoderdemo_CFLAGS += $(LOCAL_INCLUDE) -DCONF_KERNEL_VERSION_3_10
recoderdemo_CPPFLAGS += $(LOCAL_INCLUDE)

recoderdemo_LDFLAGS = $(LDFLAGS_CDXG)

recoderdemo_LDADD =  \
			$(top_srcdir)/libcore/base/libcdx_base.la \
			$(top_srcdir)/libcore/muxer/base/libcdx_muxer.la \
			$(top_srcdir)/awrecorder/libawrecorder.la \
                        $(top_srcdir)/libcore/common/libcdx_common.la

LOCAL_LIB = -lpthread -lz -lvencoder -laencoder -lVE \
		-lcrypto -lMemAdapter -lcdc_base
recoderdemo_LDFLAGS += $(LOCAL_LIB)
