LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
include $(LOCAL_PATH)/../LIBRARY/config.mk


LOCAL_SRC_FILES := \
		$(notdir $(wildcard $(LOCAL_PATH)/*.c)) \
		$(notdir $(wildcard $(LOCAL_PATH)/*.cpp))

LOCAL_C_INCLUDES  := \
        $(TOP)/frameworks/av/                               \
        $(TOP)/frameworks/av/include/                       \
		$(TOP)/frameworks/av/media/libstagefright/include   \
        $(TOP)/frameworks/native/include/                   \
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


LOCAL_MODULE_TAGS := optional

LOCAL_CFLAGS += -Werror

LOCAL_MODULE:= libawrecorder

LOCAL_SHARED_LIBRARIES +=   \
        libutils            \
        libcutils           \
        libbinder           \
        libmedia            \
		libstagefright	    \
        libui               \
        libgui              \
        libplayer           \
        libaw_plugin        \
        libcdx_parser       \
        libcdx_stream       \
        libicuuc		    \
		libMemAdapter       \
		libvencoder         \
		libaencoder         \
		libcdx_muxer


include $(BUILD_SHARED_LIBRARY)
