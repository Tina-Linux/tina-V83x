LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../config.mk

LOCAL_SRC_FILES = \
		$(notdir $(wildcard $(LOCAL_PATH)/*.c)) \
		tablelist/$(TARGET_BOARD_PLATFORM)/aw_registry_table.c

LOCAL_C_INCLUDES:= $(LOCAL_PATH)/include    \
                   $(LOCAL_PATH)/../include/     \
                   $(LOCAL_PATH)/../../include/ \
				   $(LOCAL_PATH)/../../base/include/ \

LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	liblog \
	libbinder \
	libdl

LOCAL_MODULE:= libOmxCore

ifeq ($(OREO_AND_NEWER), yes)
LOCAL_PROPRIETARY_MODULE := true
endif

include $(BUILD_SHARED_LIBRARY)
