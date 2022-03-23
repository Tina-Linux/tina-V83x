LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LIB_ROOT:=$(LOCAL_PATH)/../..
include $(LIB_ROOT)/../config.mk
include $(LIB_ROOT)/STREAM/config.mk

LOCAL_SRC_FILES = \
		$(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_C_INCLUDES:= $(LIB_ROOT)/BASE/include \
	$(LIB_ROOT)/STREAM/include  \
	$(LIB_ROOT)/../             \

LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libcdx_stream

LOCAL_SHARED_LIBRARIES = libdl libutils libcutils libcrypto libcdx_base libssl liblive555 libz

LOCAL_WHOLE_STATIC_LIBRARIES = libcdx_rtmp_stream  libcdx_mms_stream

LOCAL_STATIC_LIBRARIES = \
	libcdx_rtsp_stream \
	libcdx_file_stream \
	libcdx_tcp_stream \
	libcdx_http_stream \
	libcdx_udp_stream \
	libcdx_customer_stream \
	libcdx_ssl_stream \
	libcdx_aes_stream \
	libcdx_bdmv_stream \
	libcdx_widevine_stream \
	libcdx_videoResize_stream \
	libcdx_write_stream \

#	libcdx_mms_stream \
	libcdx_rtmp_stream \

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_SHARED_LIBRARY)

######################################################################
######################################################################
