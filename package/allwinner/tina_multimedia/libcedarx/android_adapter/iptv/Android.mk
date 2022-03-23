
LOCAL_PATH:= $(call my-dir)

include $(LOCAL_PATH)/../../config.mk

ifeq ($(CONF_PRODUCT_STB), yes)

include $(CLEAR_VARS)
include $(call all-makefiles-under,$(LOCAL_PATH))

endif
