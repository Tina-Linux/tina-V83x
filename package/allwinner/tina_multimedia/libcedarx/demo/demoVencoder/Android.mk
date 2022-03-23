LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES:= EncoderTest.c \

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/../../ \
	$(LOCAL_PATH)/../../libcore/base/include/      \
	$(LOCAL_PATH)/../../CODEC/VIDEO/ENCODER/include/ \
	$(LOCAL_PATH)/../../libcore/playback/include  \
	$(LOCAL_PATH)/../../external/include/adecoder \
	$(TOP)/frameworks/av/media/libcedarc/include \


LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libVE \
	libMemAdapter \
	libvencoder \
	libcdx_base

LOCAL_MODULE:= testEnc

#include $(BUILD_SHARED_LIBRARY)
include $(BUILD_EXECUTABLE)
