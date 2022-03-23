LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

include $(TOP)/frameworks/av/media/libcedarx/config.mk
CEDARX = $(TOP)/frameworks/av/media/libcedarx/
CEDARC = $(TOP)/frameworks/av/media/libcedarc/
################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE =
CONFIG_FOR_LINK =

LOCAL_CFLAGS += $(CONFIG_FOR_COMPILE)
LOCAL_MODULE_TAGS := optional

## set the include path for compile flags.
LOCAL_SRC_FILES:= $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_SRC_FILES+= raw2wav/src/wavheader.c

LOCAL_SRC_FILES+= Notify.cpp

LOCAL_C_INCLUDES := $(SourcePath)                             \
                    $(CEDARX)/                                \
                    $(CEDARC)/include/                        \
                    $(CEDARX)/libcore/base/                   \
                    $(CEDARX)/libcore/parser/include          \
                    $(CEDARX)/libcore/stream/include          \
                    $(CEDARX)/libcore/base/include/           \
                    $(CEDARX)/libcore/common/iniparser/       \
                    $(CEDARX)/libcore/include/       \
                    $(CEDARX)/external/include/adecoder/      \
                    $(CEDARX)/external/include/               \
                    $(TOP)/external/tinyalsa/include \
                    $(LOCAL_PATH)/raw2wav/inc \

LOCAL_SHARED_LIBRARIES := \
            libcutils       \
            libutils        \
            libadecoder     \
            libcdx_base     \
            libcdx_stream   \
            libcdx_parser   \
            libcdx_common   \
            libbinder       \
            libmedia        \
            libtinyalsa

#LOCAL_32_BIT_ONLY := true
LOCAL_MODULE:= demoAdecoder

include $(BUILD_EXECUTABLE)
