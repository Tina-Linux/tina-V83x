#######################################
#  This makefile is written for a GNU make or equivalent.
#  For example:
#   make clean
#   make
# Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
# Broadcom Bluetooth Core. Proprietary and confidential.
#######################################
# Basic path definitions
GLOBALPATH := ../../../../../..
customer := bsa_examples
project := linux
application := app_ble
os_name := linux
BLUETOOTH_3RD_DIR := $(GLOBALPATH)/3rdparty

# include common makefile rules
include $(BLUETOOTH_3RD_DIR)/embedded/$(customer)/$(project)/app_common/build/make.common

# This flag is used to remove the debug information in the app trace
#GLOBAL_DEFINE += -DAPP_TRACE_NODEBUG

# This flag is used to add timestamp in the app traces
#GLOBAL_DEFINE += -DAPP_TRACE_TIMESTAMP

# This flag is used to add color in the app traces
GLOBAL_DEFINE += -DAPP_TRACE_COLOR

ENABLE_BLE_FWDL ?= FALSE
ifeq ($(strip $(ENABLE_BLE_FWDL)),TRUE)
  GLOBAL_DEFINE += -DAPP_BLE_OTA_FW_DL_INCLUDED=TRUE
endif

# extra source list list
srcdirs :=
appsources :=

# include variable definitions for each component
include $(BLUETOOTH_APPLICATION_DIR)/build/def/*.def

# incdirs uses srcdirs
incdirs := $(incdirs) $(srcdirs)

# include the build rules for all the sample applications
include $(BLUETOOTH_3RD_DIR)/embedded/$(customer)/$(project)/app_common/build/make_sample_apps.common
