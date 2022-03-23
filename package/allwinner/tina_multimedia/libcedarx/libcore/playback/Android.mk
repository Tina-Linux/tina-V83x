LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

MOD_PATH=$(LOCAL_PATH)/../..
CEDARC_PATH=$(TOP)/frameworks/av/media/libcedarc
include $(MOD_PATH)/config.mk

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    audioDecComponent.c                 \
    audioRenderComponent.c              \
    videoDecComponent.c                 \
    subtitleDecComponent.c              \
    subtitleRenderComponent.c           \
    avtimer.c                          \
    bitrateEstimater.c                  \
    framerateEstimater.c                \
    streamManager.c                     \
    player.c                            \
    baseComponent.c                     \

LOCAL_SRC_FILES += \
    videoRenderComponent_newDisplay.c

# Please keep the list in some order
LOCAL_C_INCLUDES  := \
        $(TOP)/frameworks/av/                    \
        $(TOP)/frameworks/av/include/            \
        $(TOP)/frameworks/av/media/libcedarc/include \
        $(TOP)/frameworks/native/include/android/            \
        $(CEDARC_PATH)/include    \
        $(MOD_PATH)    \
        $(MOD_PATH)/libcore/common/iniparser/ \
        $(MOD_PATH)/libcore/base/include/ \
        $(MOD_PATH)/libcore/memory/include \
        $(MOD_PATH)/external/include/adecoder    \
        $(MOD_PATH)/external/include/sdecoder    \
        $(LOCAL_PATH)/include             \


LOCAL_MODULE_TAGS := optional

#TARGET_GLOBAL_CFLAGS += -DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

LOCAL_CFLAGS += -Wno-deprecated-declarations
#LOCAL_CFLAGS += -Werror -Wno-deprecated-declarations

LOCAL_MODULE:= libcdx_playback

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
        libbinder           \
	liblog              \
        libmedia            \
        libvdecoder         \
        libsubdecoder         \
        libMemAdapter       \
        libcdx_base         \
        libion              \
        libcdx_common

include $(BUILD_SHARED_LIBRARY)
