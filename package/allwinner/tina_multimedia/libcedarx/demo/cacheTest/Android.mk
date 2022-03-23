LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../android_adapter/awplayer/ \
                    $(LOCAL_PATH)/../../libcore/base/include/ \
                    $(LOCAL_PATH)/../../libcore/common/iniparser/ \
                    $(LOCAL_PATH)/../../xplayer/

LOCAL_SRC_FILES := \
    cache.cpp \
    ../../xplayer/cache.c

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= awplayer-cache

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
        libbinder           \
        libcdx_common       \
        libcdx_base         \

include $(BUILD_EXECUTABLE)
