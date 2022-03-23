LOCAL_PATH := $(call my-dir)

#######################
include $(CLEAR_VARS)
CEDARM_PATH=$(LOCAL_PATH)/..
include $(CEDARM_PATH)/config.mk

ifeq ($(CONFIG_OS), $(OPTION_OS_ANDROID))
    product = $(shell echo $(TARGET_PRODUCT) | cut -d '_' -f 1)
    ifeq (sugar, $(product)) #A20
        LOCAL_SRC_FILES := sugar_cedarx.conf
    else ifeq (jaws, $(product)) #A80
        LOCAL_SRC_FILES := jaws_cedarx.conf
    else ifeq (eagle, $(product)) #H8
        LOCAL_SRC_FILES := eagle_cedarx.conf
    else ifeq (dolphin, $(product)) #H3
        ifeq ($(CONFIG_CMCC), $(OPTION_CMCC_YES))
            LOCAL_SRC_FILES := dolphin_cmcc_cedarx.conf
        else
            LOCAL_SRC_FILES := dolphin_cedarx.conf
        endif
    else ifeq (rabbit, $(product)) #H64
        ifeq ($(CONFIG_CMCC), $(OPTION_CMCC_YES))
            LOCAL_SRC_FILES := rabbit_cmcc_cedarx.conf
        else
            LOCAL_SRC_FILES := rabbit_cedarx.conf
        endif
    else ifeq (r16, $(product)) #A33
        LOCAL_SRC_FILES := r16_cedarx.conf
    else ifeq (r58, $(product)) #A83
        LOCAL_SRC_FILES := r58_cedarx.conf
    else ifeq (r18, $(product)) #A64
        LOCAL_SRC_FILES := r18_cedarx.conf
    else ifeq (kylin, $(product)) #A80
        LOCAL_SRC_FILES := kylin_cedarx.conf
    else ifeq (magton, $(product)) #V40
        LOCAL_SRC_FILES := magton_cedarx.conf
    else ifeq (aston, $(product)) #V66
        LOCAL_SRC_FILES := aston_cedarx.conf
    else
        $(warning no config file found for product $(product))
    endif
endif
LOCAL_MODULE := cedarx.conf
LOCAL_MODULE_TAGS := optional
LOCAL_MODULE_PATH := $(TARGET_OUT)/etc
LOCAL_MODULE_CLASS := FAKE
include $(BUILD_PREBUILT)