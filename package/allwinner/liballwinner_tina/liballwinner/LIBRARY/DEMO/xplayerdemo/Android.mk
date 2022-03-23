LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../LIBRARY/config.mk

LOCAL_SRC_FILES = \
		$(notdir $(wildcard $(LOCAL_PATH)/*.c)) \
		$(notdir $(wildcard $(LOCAL_PATH)/*.cpp))

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
		libawrecorder


LOCAL_C_INCLUDES:= \
	$(LOCAL_PATH)/. \
	$(LOCAL_PATH)/../awrecorder  \
                $(LOCAL_PATH)/ \
        $(LOCAL_PATH)/../LIBRARY/CODEC/VIDEO/DECODER/include       \
                $(LOCAL_PATH)/../LIBRARY/CODEC/VIDEO/ENCODER/include       \
        $(LOCAL_PATH)/../LIBRARY/CODEC/AUDIO/DECODER/include       \
                $(LOCAL_PATH)/../LIBRARY/CODEC/AUDIO/ENCODER/include       \
        $(LOCAL_PATH)/../LIBRARY/CODEC/SUBTITLE/DECODER/include    \
        $(LOCAL_PATH)/../LIBRARY/PLAYER/include                    \
        $(LOCAL_PATH)/../LIBRARY/PLUGIN/include/            \
        $(LOCAL_PATH)/../LIBRARY/DEMUX/PARSER/include/      \
        $(LOCAL_PATH)/../LIBRARY/DEMUX/STREAM/include/      \
        $(LOCAL_PATH)/../LIBRARY/DEMUX/BASE/include/        \
                $(LOCAL_PATH)/../LIBRARY/DEMUX/MUXER/include/       \
        $(LOCAL_PATH)/../LIBRARY/                           \
        $(LOCAL_PATH)/../LIBRARY/MEMORY/include

#LOCAL_CFLAGS += $(CDX_CFLAGS)

LOCAL_MODULE_TAGS := optional

LOCAL_MODULE:= playerdemo

include $(BUILD_EXECUTABLE)
