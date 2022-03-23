LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

MOD_PATH=$(LOCAL_PATH)/../..
CEDARC_PATH=$(TOP)/frameworks/av/media/libcedarc

include $(MOD_PATH)/config.mk

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    awmetadataretriever.cpp \

LOCAL_C_INCLUDES  := \
        $(TOP)/frameworks/av/                               \
        $(TOP)/frameworks/av/include/                       \
        $(CEDARC_PATH)/include/                             \
        $(MOD_PATH)/libcore/parser/include/                 \
        $(MOD_PATH)/libcore/stream/include/                 \
        $(MOD_PATH)/libcore/base/include/                   \
        $(MOD_PATH)/libcore/common/iniparser/               \
        $(MOD_PATH)/libcore/memory/include/                 \
        $(MOD_PATH)/                                        \
        $(MOD_PATH)/xmetadataretriever/include/             \
        $(MOD_PATH)/external/include/adecoder/              \
        $(TOP)/frameworks/av/media/libcedarc/include/       \
        $(MOD_PATH)/android_adapter/base                 \


ifeq ($(CONF_ANDROID_VERSION), 5.0)
LOCAL_C_INCLUDES += $(TOP)/external/icu/icu4c/source/common \
                    $(TOP)/external/icu/icu4c/source/i18n
else ifeq ($(CONF_ANDROID_VERSION), 5.1)
LOCAL_C_INCLUDES += $(TOP)/external/icu/icu4c/source/common \
                    $(TOP)/external/icu/icu4c/source/i18n
else ifeq ($(CONF_ANDROID_VERSION), 6.0)
LOCAL_C_INCLUDES += $(TOP)/external/icu/icu4c/source/common \
                    $(TOP)/external/icu/icu4c/source/i18n
else
LOCAL_C_INCLUDES += $(TOP)/external/icu4c/common \
                    $(TOP)/external/icu4c/i18n
endif

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Werror

LOCAL_MODULE:= libawmetadataretriever

LOCAL_SHARED_LIBRARIES +=      \
        libutils               \
        libcutils              \
        libbinder              \
	liblog                 \
        libmedia               \
        libui                  \
        libgui                 \
        libMemAdapter          \
        libicuuc               \
        libicui18n             \
        libcdx_base            \
        libxmetadata_retriever \
        libawadapter_base      \
        libcdx_common

include $(BUILD_SHARED_LIBRARY)
