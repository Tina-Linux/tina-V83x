LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
LOCAL_LDLIBS   := -lm -llog

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= EncoderTest.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../../ \
	$(LOCAL_PATH)/../../include/ \
	$(LOCAL_PATH)/../../base/include/


LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libVE \
	libMemAdapter \
	libvencoder

LOCAL_MODULE:= testEnc

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)
