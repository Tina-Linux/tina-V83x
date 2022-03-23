#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=vmtouch
PKG_VERSION:=1.3.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/hoytech/vmtouch/archive/v1.3.1.tar.gz
PKG_MD5SUM:=46c153c48ab035d37d16e8bc1587e8d8

PKG_LICENSE:=BSD

include $(BUILD_DIR)/package.mk

define Package/vmtouch
  SECTION:=devel
  CATEGORY:=Development
  TITLE:=the Virtual Memory Toucher
endef

define Package/vmtouch/description
  Portable file system cache diagnostics and control.

  vmtouch is a tool for learning about and controlling the file system
cache of unix and unix-like systems. It is BSD licensed so you can
basically do whatever you want with it.
endef

define Package/vmtouch/install
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/vmtouch $(1)/usr/bin
endef

$(eval $(call BuildPackage,vmtouch))
