#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=librsync
PKG_VERSION:=2.0.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/librsync/librsync.git
PKG_MD5SUM:=12602fe9e59851de7fed2f757be0f359
CMAKE_INSTALL:=1

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_LICENSE:=LGPL-2.1


include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/librsync
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=rdiff lib
  URL:=https://github.com/librsync/librsync.git
endef

define Package/librsync/description
  This package contains the librsync.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && cmake $(PKG_BUILD_DIR))
endef

CONFIGURE_ARGS += \
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/local/include/librsync.h $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/local/lib/librsync.so* $(1)/usr/lib/
endef

define Package/librsync/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/local/lib/librsync.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,librsync))
