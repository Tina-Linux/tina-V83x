LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

OMX_TOP=$(LOCAL_PATH)/..
CDC_TOP=$(OMX_TOP)/..

include $(CDC_TOP)/config.mk

#LOCAL_SRC_FILES = $(notdir $(wildcard $(LOCAL_PATH)/*.c))
LOCAL_SRC_FILES:= src/omx_vdec.c src/omx_vdec_port.c \
                                 src/omx_vdec_aw_decoder_android.cpp \
                                 looper/omx_thread.c \
                                 looper/omx_mutex.c \
                                 looper/omx_sem.c

LOCAL_C_INCLUDES:= $(OMX_TOP)/omxcore/include  \
                   $(OMX_TOP)/include/     \
                   $(CDC_TOP)/include/ \
                   $(CDC_TOP)/base/include/  \
                   $(CDC_TOP)/ve/include \
                   $(CDC_TOP)/memory/include \
                   $(LOCAL_PATH)/inc           \
                   $(LOCAL_PATH)/looper/   \
                   $(TOP)/frameworks/native/include/     \
                   $(TOP)/frameworks/native/include/media/hardware \
                   $(TOP)/frameworks/av/media/libcedarc/include \
                   $(TOP)/hardware/libhardware/include/hardware/ \
                   $(TOP)/frameworks/av/media/libstagefright/foundation/include \
                   $(TOP)/frameworks/native/libs/nativebase/include \
                   $(TOP)/frameworks/native/libs/nativewindow/include \
                   $(TOP)/frameworks/native/libs/arect/include

LOCAL_CFLAGS += $(CDX_CFLAGS)
LOCAL_CFLAGS += -DTARGET_BOARD_PLATFORM=$(TARGET_BOARD_PLATFORM)

LOCAL_CFLAGS += -DCONF_OMX_USE_ZERO_COPY

LOCAL_MODULE_TAGS := optional
LOCAL_PRELINK_MODULE := false

ifeq ($(OREO_AND_NEWER), yes)
LOCAL_SHARED_LIBRARIES += libvdecoder.vendor \
			libVE.vendor  \
			libvideoengine.vendor \
			libMemAdapter.vendor  \
			libcdc_base.vendor \
                        liblog libcutils libutils libbinder  libui libdl libion

# build so in vendor
LOCAL_PROPRIETARY_MODULE := true
else
LOCAL_SHARED_LIBRARIES += libvdecoder \
                        libVE  \
                        libvideoengine \
                        libMemAdapter  \
                        libcdc_base \
                        liblog libcutils libutils libbinder  libui libdl libion
endif

LOCAL_MODULE:= libOmxVdec

include $(BUILD_SHARED_LIBRARY)
