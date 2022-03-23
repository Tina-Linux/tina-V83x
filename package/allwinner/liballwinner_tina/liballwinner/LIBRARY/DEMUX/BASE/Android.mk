LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../config.mk
include $(LOCAL_PATH)/config.mk

LOCAL_SRC_FILES = \
		$(notdir $(wildcard $(LOCAL_PATH)/*.c)) \

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/include    \
                   $(LOCAL_PATH)/../../     \

LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libcdx_base

LOCAL_SHARED_LIBRARIES:= libcutils

ifneq ($(CONFIG_OS_VERSION),$(OPTION_OS_VERSION_ANDROID_5_0))
ifneq ($(CONFIG_OS_VERSION),$(OPTION_OS_VERSION_ANDROID_6_0))
LOCAL_SHARED_LIBRARIES += libcorkscrew
endif
endif

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_SHARED_LIBRARY)
