LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE =
CONFIG_FOR_LINK =

LOCAL_CFLAGS += $(CONFIG_FOR_COMPILE)
LOCAL_MODULE_TAGS := optional

## set the include path for compile flags.
LOCAL_SRC_FILES:= $(notdir $(wildcard $(LOCAL_PATH)/*.c))

LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include/                                        \
                    $(LOCAL_PATH)/../../                        \
					$(LOCAL_PATH)/../../VE/include                      \
					$(LOCAL_PATH)/../../MEMORY/include                  \
                    $(LOCAL_PATH)/../../CODEC/VIDEO/DECODER/include     \
                    $(LOCAL_PATH)/../../CODEC/AUDIO/DECODER/include     \
                    $(LOCAL_PATH)/../../CODEC/SUBTITLE/DECODER/include  \
                    $(LOCAL_PATH)/../../DEMUX/BASE/include      \
                    $(LOCAL_PATH)/../../DEMUX/STREAM/include    \
                    $(LOCAL_PATH)/../../DEMUX/PARSER/include


#LOCAL_SHARED_LIBRARIES := \
            libcutils       \
            libutils        \
            libplayer       \
            libvdecoder     \
            libadecoder     \
            libnormal_audio \
            libsdecoder     \
            libcdx_base     \
            libcdx_stream   \
            libcdx_parser   \
            libVE           \
            libMemAdapter   \
            libaw_plugin    \
            librx           \
            librx           \
            libdxx          \
            libaw_wvm       \
            libcrypto

LOCAL_SHARED_LIBRARIES := \
            libcutils       \
            libutils        \
            libplayer       \
            libvdecoder     \
            libadecoder     \
            libsdecoder     \
            libcdx_base     \
            libcdx_stream   \
            libcdx_parser   \
            libVE           \
            libMemAdapter   \
            libaw_plugin    \
            libaw_wvm       \
            libcrypto
#LOCAL_32_BIT_ONLY := true
LOCAL_MODULE:= demoVdecoder

include $(BUILD_EXECUTABLE)