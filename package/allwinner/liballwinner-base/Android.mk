LOCAL_PATH := $(call my-dir)
HERE_PATH := $(call my-dir)

include $(CLEAR_VARS)
LOCAL_MODULE    := libawbase

LOCAL_MODULE_PATH := $(LOCAL_PATH)/release
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := src/check_chip.c

include $(BUILD_SHARED_LIBRARY)
