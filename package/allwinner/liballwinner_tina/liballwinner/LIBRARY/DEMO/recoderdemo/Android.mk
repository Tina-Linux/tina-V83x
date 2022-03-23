LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../config.mk

LOCAL_SRC_FILES = \
		$(notdir $(wildcard $(LOCAL_PATH)/*.c)) \
		$(notdir $(wildcard $(LOCAL_PATH)/*.cpp))

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
		libawrecorder


LOCAL_C_INCLUDES:= \
		$(LOCAL_PATH)/../../../awrecorder  \
        $(LOCAL_PATH)/ \
        $(LOCAL_PATH)/../../CODEC/VIDEO/DECODER/include       \
        $(LOCAL_PATH)/../../CODEC/VIDEO/ENCODER/include       \
        $(LOCAL_PATH)/../../CODEC/AUDIO/DECODER/include       \
        $(LOCAL_PATH)/../../CODEC/AUDIO/ENCODER/include       \
        $(LOCAL_PATH)/../../CODEC/SUBTITLE/DECODER/include    \
        $(LOCAL_PATH)/../../PLAYER/include                    \
        $(LOCAL_PATH)/../../PLUGIN/include/            \
        $(LOCAL_PATH)/../../DEMUX/PARSER/include/      \
        $(LOCAL_PATH)/../../DEMUX/STREAM/include/      \
        $(LOCAL_PATH)/../../DEMUX/BASE/include/        \
        $(LOCAL_PATH)/../../DEMUX/MUXER/include/       \
        $(LOCAL_PATH)/../../                           \
        $(LOCAL_PATH)/../../MEMORY/include

#LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= recoderdemo

include $(BUILD_EXECUTABLE)
