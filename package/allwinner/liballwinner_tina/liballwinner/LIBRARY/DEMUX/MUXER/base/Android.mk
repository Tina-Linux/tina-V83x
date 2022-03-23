LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LIB_ROOT=$(LOCAL_PATH)/../..
include $(LIB_ROOT)/../config.mk

LOCAL_SRC_FILES = \
                $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_C_INCLUDES:= $(LOCAL_PATH) \
        $(LIB_ROOT)/BASE/include \
		    $(LIB_ROOT)/MUXER/include \
		    $(LIB_ROOT)/../CODEC/VIDEO/ENCODER/include \
        $(LIB_ROOT)/../CODEC/AUDIO/ENCODER/include \
        $(LIB_ROOT)/../ \

LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libcdx_muxer

LOCAL_SHARED_LIBRARIES = libutils libcutils libdl libcdx_base

LOCAL_WHOLE_STATIC_LIBRARIES = libcdx_aac_muxer libcdx_mp4_muxer libcdx_ts_muxer

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_SHARED_LIBRARY)
