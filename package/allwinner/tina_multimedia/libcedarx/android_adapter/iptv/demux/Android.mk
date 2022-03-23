LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    tsdemux.c       \

LOCAL_C_INCLUDES := $(SourcePath)                               \
					$(LOCAL_PATH)/../../../           \
                    $(LOCAL_PATH)/../../../libcore/base/include      \

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Werror -DVERBOSE

LOCAL_MODULE:= libtvdemux

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
        libcdx_base

include $(BUILD_SHARED_LIBRARY)
