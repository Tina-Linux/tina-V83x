LOCAL_PATH:= $(call my-dir)

################################################################################

include $(CLEAR_VARS)

LOCAL_MODULE_TAGS := optional

DEMUX_PATH = $(LOCAL_PATH)/../..
LIBRARY_PATH = $(DEMUX_PATH)/..

LOCAL_SRC_FILES:= MuxerWriter.c testVideoMuxer.c \


LOCAL_C_INCLUDES := \
  $(LOCAL_PATH) \
  $(DEMUX_PATH)/BASE/include \
  $(DEMUX_PATH)/MUXER/include \
	$(LIBRARY_PATH) \
	$(LIBRARY_PATH)/CODEC/VIDEO/ENCODER/include/ \
  $(LIBRARY_PATH)/CODEC/AUDIO/ENCODER/include/ \




LOCAL_SHARED_LIBRARIES := \
	libcutils \
	libutils \
	libVE \
	libMemAdapter \
	libvencoder \
	libaencoder \
	libcdx_muxer \
	libcdx_base \

LOCAL_MODULE:= muxerdemo

include $(BUILD_EXECUTABLE)
