#
# 1. Set the path and clear environment
#   TARGET_PATH := $(call my-dir)
#   include $(ENV_CLEAR)
#
# 2. Set the source files and headers files
#   TARGET_SRC := xxx_1.c xxx_2.c
#   TARGET_INc := xxx_1.h xxx_2.h
#
# 3. Set the output target
#   TARGET_MODULE := xxx
#
# 4. Include the main makefile
#   include $(BUILD_BIN)
#
# Before include the build makefile, you can set the compilaion
# flags, e.g. TARGET_ASFLAGS TARGET_CFLAGS TARGET_CPPFLAGS
#

#TARGET_PATH :=$(call my-dir)

#########################################
#include $(ENV_CLEAR)
#SRC_TAGS := ./
#TARGET_SRC := $(call all-c-files-under, $(SRC_TAGS))
#TARGET_INC := \
#   $(TARGET_PATH)/SENSOR_H \
#   $(TARGET_PATH)/../iniparser/src/ \
#   $(TARGET_PATH)/../include/ \
#   $(TARGET_PATH)/../isp_dev/
#TARGET_CFLAGS := -fPIC -Wall
#TARGET_MODULE := libisp_ini
#include $(BUILD_STATIC_LIB)

#########################################
#include $(ENV_CLEAR)
#SRC_TAGS := ./
#TARGET_SRC := $(call all-c-files-under, $(SRC_TAGS))
#TARGET_INC := \
#   $(TARGET_PATH)/SENSOR_H \
#   $(TARGET_PATH)/../iniparser/src/ \
#   $(TARGET_PATH)/../include/ \
#   $(TARGET_PATH)/../isp_dev/
#TARGET_CFLAGS := -fPIC -Wall
#TARGET_MODULE := libisp_ini
#include $(BUILD_SHARED_LIB)

#########################################
#LOCAL_PATH:= $(call my-dir)
#include $(CLEAR_VARS)

#LOCAL_MODULE := libisp_ini

#LOCAL_MODULE_TAGS := eng option

#LOCAL_SRC_FILES := \
#    isp_ini_parse.c

#LOCAL_C_INCLUDES += \
#    $(LOCAL_PATH)/SENSOR_H

#include $(BUILD_STATIC_LIBRARY)

#########################################
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)

LOCAL_MODULE := libisp_ini

LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
    isp_ini_parse.c

LOCAL_C_INCLUDES += \
    $(LOCAL_PATH)/SENSOR_H

LOCAL_SHARED_LIBRARIES := \
    libcutils liblog

include $(BUILD_SHARED_LIBRARY)


