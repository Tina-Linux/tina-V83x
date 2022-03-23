L_CFLAGS = -DANDROID_ENV
#L_CFLAGS += -DSIMPLE_CONFIG_PBC_SUPPORT
#L_CFLAGS += -DPLATFORM_MSTAR             #mStar platform doesn't support system() function.
#L_CFLAGS += -DCONFIG_IOCTL_CFG80211	#Using iw command, not iwconfig command.
#L_CFLAGS += -DCONFIG_NOFCS


LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_PREBUILT_LIBS := libairkiss_aes.a
LOCAL_STATIC_LIBRARIES := libairkiss_aes
include $(BUILD_MULTI_PREBUILT)



include $(CLEAR_VARS)
LOCAL_MODULE    := rtw_ak
LOCAL_STATIC_LIBRARIES := libairkiss_aes
LOCAL_CFLAGS := $(L_CFLAGS)
LOCAL_SRC_FILES := main.c
include $(BUILD_EXECUTABLE)
