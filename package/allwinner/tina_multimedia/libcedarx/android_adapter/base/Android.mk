LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

CEDARM_PATH=$(LOCAL_PATH)/../..

include $(CEDARM_PATH)/config.mk

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    awMediaDataSource.cpp

LOCAL_C_INCLUDES  := \
        $(CEDARM_PATH)/libcore/base/include/                \
        $(CEDARM_PATH)/libcore/stream/include/              \

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS +=

LOCAL_MODULE:= libawadapter_base

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
	liblog              \
        libcdx_base         \


include $(BUILD_SHARED_LIBRARY)
