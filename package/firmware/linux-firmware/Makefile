#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=linux-firmware
PKG_RELEASE:=1

PKG_MAINTAINER:=Trace Wong

SCAN_DEPS = *.mk

ifeq ($(CONFIG_FIRMWARE_PATH),)
FIRMWARE_PATH:=/lib/firmware/
else
FIRMWARE_PATH:=$(CONFIG_FIRMWARE_PATH)
endif

include $(BUILD_DIR)/package.mk

define firmware/config
$(info Config:
  config FIRMWARE_PATH
  string
  prompt "Firmware's directory"
  default "/lib/firmware/"
  help
     Configure the location of firmware for system.
@@
)
endef

define Package/firmware-default
  SECTION:=firmware
  CATEGORY:=Firmware
  TITLE:=$(1)
endef


define Build/Compile

endef


include $(wildcard */*.mk)

ifneq ($(DUMP),)
$(call firmware/config,)
endif

#$(eval $(call BuildPackage,linux-firmware))
