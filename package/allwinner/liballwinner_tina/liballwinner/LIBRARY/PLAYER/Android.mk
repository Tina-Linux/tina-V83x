LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

MOD_PATH=$(LOCAL_PATH)/../..
CEDARC_PATH=$(TOP)/frameworks/av/media/libcedarc
include $(MOD_PATH)/config.mk

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    audioDecComponent.cpp                 \
    audioRenderComponent.cpp              \
    videoDecComponent.cpp                 \
    subtitleDecComponent.cpp              \
    subtitleRenderComponent.cpp           \
    soundControl/soundControl_android.cpp \
    soundControl/soundControl_android_raw.cpp \
    soundControl/IEC61937.c               \
    avtimer.cpp                           \
    messageQueue.cpp                      \
    bitrateEstimater.cpp                  \
    framerateEstimater.cpp                \
    streamManager.cpp                     \
    player.cpp                            \
    deinterlace.cpp                       \
    deinterlaceHw.cpp



ifeq ($(CONFIG_DEINTERLACE),$(OPTION_SW_DEINTERLACE))
LOCAL_SRC_FILES += \
    sw-deinterlace/deinterlaceSw.cpp	\
    sw-deinterlace/postprocess.cpp      \
    sw-deinterlace/AWPostProcess.cpp    \
    sw-deinterlace/yadif.cpp

endif

ifeq ($(USE_NEW_DISPLAY),1)
LOCAL_SRC_FILES += \
    layerControl/layerControl_android_newDisplay.cpp \
    videoRenderComponent_newDisplay.cpp
else
LOCAL_SRC_FILES += \
    videoRenderComponent.cpp \
    layerControl/layerControl_android.cpp
endif

ifeq ($(ENABLE_SUBTITLE_DISPLAY_IN_CEDARX),1)
LOCAL_SRC_FILES += \
    subtitleNativeDisplay/subtitleNativeDisplay.cpp
else ifeq ($(CONFIG_ALI_YUNOS), $(OPTION_ALI_YUNOS_YES))
	# on AliYUNOS android4.4
	ifeq ($(CONFIG_OS_VERSION),$(OPTION_OS_VERSION_ANDROID_4_4))
		LOCAL_SRC_FILES += \
		    subtitleNativeDisplay/subtitleNativeDisplay.cpp
	endif
endif

LOCAL_C_INCLUDES  := \
        $(TOP)/frameworks/av/                    \
        $(TOP)/frameworks/av/include/            \
        $(TOP)/frameworks/native/include/android/            \
        $(TOP)/frameworks/av/media/libcedarc/include \
        $(CEDARC_PATH)/include    \
        $(MOD_PATH)    \
        $(MOD_PATH)/libcore/include    \
        $(MOD_PATH)/external/include/adecoder    \
        $(MOD_PATH)/libcore/memory/include \
        $(LOCAL_PATH)/include             \
        $(LOCAL_PATH)/subtitleNativeDisplay             \
        external/skia/include/libcore \
		external/skia/include/effects \
		external/skia/include/images \
		external/skia/src/ports \
		external/skia/src/libcore \
		external/skia/include/utils \
		external/icu4c/common

ifeq ($(CONFIG_OS_VERSION), $(OPTION_OS_VERSION_ANDROID_5_0))
LOCAL_C_INCLUDES  += external/icu/icu4c/source/common
LOCAL_C_INCLUDES  += external/skia/src/core/
endif
ifeq ($(CONFIG_OS_VERSION), $(OPTION_OS_VERSION_ANDROID_5_0))
ifeq ($(CONFIG_TARGET_PRODUCT), r16)
LOCAL_C_INCLUDES  += $(TOP)/hardware/aw/hwc/r16/
endif
endif

ifeq ($(CONFIG_OS_VERSION), $(OPTION_OS_VERSION_ANDROID_6_0))
LOCAL_C_INCLUDES  += external/skia/src/core
ifeq ($(CONFIG_TARGET_PRODUCT), r16)
LOCAL_C_INCLUDES  += $(TOP)/hardware/aw/hwc/r16/
endif
endif

LOCAL_MODULE_TAGS := optional

TARGET_GLOBAL_CFLAGS += -DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

LOCAL_CFLAGS += $(LAW_CFLAGS)

LOCAL_CFLAGS += -Wno-deprecated-declarations
#LOCAL_CFLAGS += -Werror -Wno-deprecated-declarations

LOCAL_MODULE:= libplayer

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
        libbinder           \
        libmedia            \
        libui               \
        libgui              \
        libvdecoder	    \
        libadecoder         \
        libsdecoder         \
        libMemAdapter       \
        libtinyalsa        \
        libion \
        libskia \
        libicuuc

include $(BUILD_SHARED_LIBRARY)
