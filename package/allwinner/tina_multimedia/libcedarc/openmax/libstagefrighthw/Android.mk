
LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

MOD_TOP=$(LOCAL_PATH)/../..
include $(MOD_TOP)/config.mk

ifeq ($(OREO_AND_NEWER), yes)
# build so in verdor
LOCAL_PROPRIETARY_MODULE := true
endif

LOCAL_SRC_FILES := \
    AwOMXPlugin.cpp                      \

LOCAL_CFLAGS += $(PV_CFLAGS_MINUS_VISIBILITY)
LOCAL_CFLAGS += $(AW_OMX_EXT_CFLAGS)
LOCAL_MODULE_TAGS := optional


LOCAL_C_INCLUDES :=    \
        $(MOD_TOP)/include \
	$(MOD_TOP)/base/include \
        $(TOP)/frameworks/native/include/media/hardware \
        $(TOP)/frameworks/native/include/media/openmax \
	$(TOP)/frameworks/native/headers/media_plugin

LOCAL_SHARED_LIBRARIES :=       \
        libbinder               \
        libutils                \
	liblog                  \
        libcutils               \
        libdl                   \
        libui                   \

LOCAL_MODULE := libstagefrighthw

include $(BUILD_SHARED_LIBRARY)
