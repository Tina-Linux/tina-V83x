LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

CEDARX_ROOT=$(LOCAL_PATH)/../../../
include $(CEDARX_ROOT)/config.mk

ifneq ($(CONF_ANDROID_VERSION), 7.0)
ifneq ($(CONF_ANDROID_VERSION), 7.1)

LOCAL_SRC_FILES:= \
	Extractor.cpp \
	ExtractorWrapper.cpp \
	WVMDataSource.cpp

LOCAL_C_INCLUDES:= \
    $(CEDARX_ROOT)/ \
    $(CEDARX_ROOT)/libcore \
    $(CEDARX_ROOT)/libcore/base/include \
    $(CEDARX_ROOT)/libcore/parser/include \
    $(CEDARX_ROOT)/libcore/stream/include \
    $(CEDARX_ROOT)/libcore/memory/include \
    $(CEDARX_ROOT)/external/include/adecoder \
    $(TOP)/frameworks/av/media/libcedarc/include \
    $(TOP)/frameworks/av/media/libcedarc/vdecoder/include \
    $(TOP)/frameworks/av/media/libcedarc/sdecoder/include \
    $(TOP)/frameworks/av/include \
    $(TOP)/frameworks/av/media/libstagefright

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

#LOCAL_CFLAGS :=-Werror


ifeq ($(BOARD_WIDEVINE_OEMCRYPTO_LEVEL), 1)
LOCAL_CFLAGS +=-DSECUREOS_ENABLED
endif

LOCAL_SHARED_LIBRARIES := \
        libcutils \
        libstagefright \
        libstagefright_foundation \
        libdrmframework \
        libdl \
        libutils \
        libMemAdapter

LOCAL_MODULE:=libaw_wvm

ifeq ($(TARGET_ARCH),arm)
    LOCAL_CFLAGS += -Wno-psabi
endif

include $(BUILD_SHARED_LIBRARY)

endif
endif