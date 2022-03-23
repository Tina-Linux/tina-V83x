LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

CEDARM_PATH=$(LOCAL_PATH)/..
CEDARC_PATH=$(TOP)/frameworks/av/media/libcedarc

include $(CEDARM_PATH)/config.mk

LOCAL_ARM_MODE := arm

LOCAL_SRC_FILES := \
    xmetadataretriever.c            \

LOCAL_C_INCLUDES  := \
        $(TOP)/frameworks/av/                               \
        $(TOP)/frameworks/av/include/                       \
        $(TOP)/frameworks/native/include/                   \
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
        $(CEDARM_PATH)/                                     \
        $(CEDARM_PATH)/xplayer/include/                     \
        $(CEDARM_PATH)/xmetadataretriever/include/

LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Werror

LOCAL_MODULE:= libxmetadata_retriever

LOCAL_SHARED_LIBRARIES +=   \
        libcutils           \
	liblog              \
        libcdx_parser       \
        libcdx_stream       \
        libMemAdapter       \
        libvdecoder         \
        libvencoder         \
        libcdx_base         \
        libcdx_common

include $(BUILD_SHARED_LIBRARY)
