#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=civetweb
PKG_VERSION:=1.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/civetweb/civetweb.git
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=MIT

CMAKE_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_SOURCE_DIR:=$(PKG_BUILD_DIR)/civetweb_build/
CMAKE_OPTIONS += -DBUILD_TESTING=OFF

define Package/civetweb
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=civetweb
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/civetweb_build/
	unzip -d $(PKG_BUILD_DIR)/civetweb_build/ $(DL_DIR)/$(PKG_SOURCE)
	mv $(PKG_BUILD_DIR)/civetweb_build/civetweb-1.9/* $(PKG_BUILD_DIR)/civetweb_build/

endef

define Package/civetweb/description
Civetweb is a small footprint HTTP Server with optional Lua and SQLite support written in C.
It can be embedded into C/C++ applications or used as stand alone web server for Windows, Linux, BSD or Mac-OS.
This project is based on the Mongoose web server, but maintains the MIT license and is free from GPL licenses.
endef

define Package/civetweb/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/civetweb $(1)/usr/bin/
endef

$(eval $(call BuildPackage,civetweb))
