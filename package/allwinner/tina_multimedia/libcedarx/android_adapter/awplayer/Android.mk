LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

CEDARM_PATH=$(LOCAL_PATH)/../..
CEDARC_PATH=$(TOP)/frameworks/av/media/libcedarc

include $(CEDARM_PATH)/config.mk

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    awplayer.cpp        \
    awStreamingSource.cpp \
    awStreamListener.cpp  \
    subtitleUtils.cpp     \
    AwHDCPModule.cpp \
    awLogRecorder.cpp   \

LOCAL_C_INCLUDES  := \
        $(TOP)/frameworks/av/                               \
        $(TOP)/frameworks/av/include/                       \
        $(TOP)/frameworks/native/include/android            \
        $(CEDARC_PATH)/include                              \
        $(CEDARC_PATH)/vdecoder/include                     \
        $(CEDARC_PATH)/adecoder/include                     \
        $(CEDARC_PATH)/sdecoder/include                     \
        $(CEDARM_PATH)/external/include/adecoder            \
        $(CEDARM_PATH)/libcore/playback/include             \
        $(CEDARM_PATH)/libcore/common/iniparser/            \
        $(CEDARM_PATH)/libcore/parser/include/              \
        $(CEDARM_PATH)/libcore/stream/include/              \
        $(CEDARM_PATH)/libcore/base/include/                \
        $(CEDARM_PATH)/xplayer/include                      \
        $(CEDARM_PATH)/android_adapter/output               \
        $(CEDARM_PATH)/android_adapter/base                 \
        $(CEDARM_PATH)/                                     \


# for subtitle character set transform.
ifeq ($(CONF_ANDROID_VERSION), 5.0)
LOCAL_C_INCLUDES += $(TOP)/external/icu/icu4c/source/common
else ifeq ($(CONF_ANDROID_VERSION), 5.1)
LOCAL_C_INCLUDES += $(TOP)/external/icu/icu4c/source/common
else ifeq ($(CONF_ANDROID_VERSION), 6.0)
LOCAL_C_INCLUDES += $(TOP)/external/icu/icu4c/source/common
else
LOCAL_C_INCLUDES += $(TOP)/external/icu4c/common
endif

ifeq ($(BOARD_USE_PLAYREADY), 1)
LOCAL_SRC_FILES += \
        awPlayReadyLicense.cpp
ifeq ($(PLAYREADY_DEBUG), 1)
PLAYREADY_DIR:= $(TOP)/hardware/aw/playready
include $(PLAYREADY_DIR)/config.mk
LOCAL_CFLAGS += $(PLAYREADY_CFLAGS)
LOCAL_C_INCLUDES +=                                 \
        $(PLAYREADY_DIR)/source/inc                 \
        $(PLAYREADY_DIR)/source/oem/common/inc      \
        $(PLAYREADY_DIR)/source/oem/ansi/inc        \
        $(PLAYREADY_DIR)/source/results             \
        $(PLAYREADY_DIR)/source/tools/shared/common \
        $(PLAYREADY_DIR)/source/tools/shared/netio
else
include $(TOP)/hardware/aw/playready/config.mk
LOCAL_CFLAGS += $(PLAYREADY_CFLAGS)
LOCAL_C_INCLUDES +=                                              \
        $(TOP)/hardware/aw/playready/include/inc                 \
        $(TOP)/hardware/aw/playready/include/oem/common/inc      \
        $(TOP)/hardware/aw/playready/include/oem/ansi/inc        \
        $(TOP)/hardware/aw/playready/include/results             \
        $(TOP)/hardware/aw/playready/include/tools/shared/common \
        $(TOP)/hardware/aw/playready/include/tools/shared/netio
endif
LOCAL_SHARED_LIBRARIES += libplayreadypk
LOCAL_CFLAGS += -DBOARD_USE_PLAYREADY=${BOARD_USE_PLAYREADY}
LOCAL_CFLAGS += -DBOARD_PLAYREADY_USE_SECUREOS=${BOARD_PLAYREADY_USE_SECUREOS}
else
LOCAL_CFLAGS += -DBOARD_USE_PLAYREADY=0
LOCAL_CFLAGS += -DBOARD_PLAYREADY_USE_SECUREOS=0
endif

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS +=

LOCAL_MODULE:= libawplayer

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
        libbinder           \
	liblog              \
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
        libxplayer          \
        libaw_output        \
        libawadapter_base   \
        libcdx_common

ifeq ($(CONF_ANDROID_VERSION), 8.1)
LOCAL_SHARED_LIBRARIES += libmediametrics libstagefright_foundation
endif

include $(BUILD_SHARED_LIBRARY)
