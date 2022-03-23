LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../config.mk

LOCAL_SRC_FILES = \
		$(notdir $(wildcard $(LOCAL_PATH)/*.c)) \

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/include    \
                   $(LOCAL_PATH)/../../     \
                   $(LOCAL_PATH)/../include/     \
		   $(LOCAL_PATH)/../memory/ionMemory/     \
                   $(TOP)/frameworks/av/                               \
		   $(TOP)/system/core/libion/include/ \
		   $(TOP)/system/core/include/ \
           $(TOP)/system/core/libion/kernel-headers/ \

LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

LOCAL_MODULE:= libcdc_base

LOCAL_SHARED_LIBRARIES:= libcutils liblog

ifeq ($(LOLLIPOP_AND_NEWER), no)
LOCAL_SHARED_LIBRARIES += libcorkscrew
endif

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

ifeq ($(CONFIG_COMPILE_STATIC_LIB), y)
    include $(BUILD_STATIC_LIBRARY)
else
    include $(BUILD_SHARED_LIBRARY)
endif
