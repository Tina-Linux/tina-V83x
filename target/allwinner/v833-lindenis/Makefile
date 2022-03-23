#
# Copyright (C) 2013-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

ARCH:=arm
BOARD:=v833-lindenis
BOARDNAME:=v833-lindenis
FEATURES:=fpu usb ext4 display rtc
CPU_TYPE:=cortex-a7
CPU_SUBTYPE:=neon
MAINTAINER:=Allwinner

KERNEL_PATCHVER:=4.9
UBOOT_PATCHVER:=2018
KERNELNAME:=zImage dtbs

include $(BUILD_DIR)/target.mk

DEFAULT_PACKAGES +=
TARGET_CFLAGS += -D_GNU_SOURCE -D_FILE_OFFSET_BITS=64 -DUSE_LOG_LIB_GLOG
#-DUSE_STD_LOG
TARGET_CXXFLAGS += -std=c++11 -fno-rtti

$(eval $(call BuildTarget))
