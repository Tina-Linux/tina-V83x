LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

CEDARM_PATH=$(LOCAL_PATH)/../..
CEDARC_PATH=$(TOP)/frameworks/av/media/libcedarc

include $(CEDARM_PATH)/config.mk

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    soundControl.cpp \
    soundControl_null.cpp \
    layerControl.cpp \
    subtitleControl.cpp \
    deinterlace.cpp   \
    VideoFrameScheduler.cpp \
    VideoFrameSchedulerWrap.cpp \

############# the interface of skia changed in AndroidN,
############# we do not support native render for subtitle
ifeq ($(CONF_ANDROID_VERSION), 4.4)
	LOCAL_SRC_FILES += subtitleNativeDisplay/subtitleNativeDisplay.cpp
else ifeq ($(CONF_ANDROID_VERSION), 5.0)
	LOCAL_SRC_FILES += subtitleNativeDisplay/subtitleNativeDisplay.cpp
else ifeq ($(CONF_ANDROID_VERSION), 5.1)
	LOCAL_SRC_FILES += subtitleNativeDisplay/subtitleNativeDisplay.cpp
endif

LOCAL_C_INCLUDES  := \
        $(TOP)/frameworks/av/                               \
        $(TOP)/frameworks/av/include/                       \
        $(TOP)/frameworks/native/include/android            \
        $(CEDARC_PATH)/include                              \
        $(CEDARC_PATH)/vdecoder/include                     \
        $(CEDARC_PATH)/adecoder/include                     \
        $(CEDARC_PATH)/sdecoder/include                     \
        $(CEDARC_PATH)/base/include                         \
        $(CEDARM_PATH)/external/include/adecoder            \
        $(CEDARM_PATH)/libcore/playback/include             \
        $(CEDARM_PATH)/libcore/common/iniparser/            \
        $(CEDARM_PATH)/libcore/parser/include/              \
        $(CEDARM_PATH)/libcore/stream/include/              \
        $(CEDARM_PATH)/libcore/base/include/                \
        $(CEDARM_PATH)/android_adapter/awplayer             \
		$(CEDARM_PATH)/android_adapter/output               \
		$(CEDARM_PATH)/xplayer/include                      \
        $(CEDARM_PATH)/                                     \
        external/skia/include/core \
		external/skia/include/effects \
		external/skia/include/images \
		external/skia/src/ports \
		external/skia/src/core \
		external/skia/include/utils \
		external/icu4c/common


# for subtitle character set transform.
ifeq ($(CONF_ANDROID_VERSION), 4.4)
LOCAL_C_INCLUDES += $(TOP)/external/icu4c/common
else
LOCAL_C_INCLUDES += $(TOP)/external/icu/icu4c/source/common
endif


LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Wno-unused-parameter
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

LOCAL_MODULE:= libaw_output

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
	liblog              \
        libbinder           \
        libmedia            \
        libui               \
        libgui              \
        libion              \
        libcdx_playback     \
        libcdx_parser       \
        libcdx_stream       \
        libcdx_base         \
        libicuuc            \
        libMemAdapter       \
        libcdc_base         \
        libxplayer          \
        libcdx_common

ifneq ($(CONF_ANDROID_VERSION), 7.0)
LOCAL_SHARED_LIBRARIES +=   libskia
endif

ifeq ($(CONF_ANDROID_VERSION), 7.0)
LOCAL_C_INCLUDES += $(TOP)/hardware/aw/displayd/libdispclient
LOCAL_SHARED_LIBRARIES += libdispclient
endif

ifeq ($(CONF_ANDROID_VERSION), 8.0)
LOCAL_SHARED_LIBRARIES += libaudioclient
endif

ifeq ($(CONF_ANDROID_VERSION), 8.1)
LOCAL_SHARED_LIBRARIES += libaudioclient
endif


include $(BUILD_SHARED_LIBRARY)
