#
# Copyright (C) 2007 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 9396 2007-10-21 15:59:31Z blogic $

include $(TOPDIR)/rules.mk

PKG_NAME:=directfb-examples
PKG_VERSION:=1.7.0
PKG_RELEASE:=3

PKG_SOURCE:=DirectFB-examples-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://free-electrons.com/~thomas/pub/
PKG_BUILD_DIR:=$(COMPILE_DIR)/DirectFB-examples-$(PKG_VERSION)

PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/directfb-examples
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=directfb examples
  DEPENDS:=+directfb
  URL:=http://www.mucross.com/downloads/tonga-demo/sources/GPL/directfb-examples/
endef

define Package/directfb-examples/description
	directfb examples
endef

#define Build/Compile
#	$(MAKE) -C $(PKG_BUILD_DIR) CC="$(TARGET_CC)" CFLAGS="$(TARGET_CFLAGS)" LD="$(TARGET_LD)" LDFLAGS="$(TARGET_LDFLAGS)"
#endef

define Package/directfb-examples/install
	$(INSTALL_DIR) $(1)/usr/share/directfb-examples
	$(INSTALL_DIR) $(1)/usr/share/directfb-examples/fonts
	$(INSTALL_DIR) $(1)/usr/share/directfb-examples/spacedream
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/directfb-examples/*.* $(1)/usr/share/directfb-examples
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/directfb-examples/fonts/*.* $(1)/usr/share/directfb-examples/fonts
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/share/directfb-examples/spacedream/*.* $(1)/usr/share/directfb-examples/spacedream

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,directfb-examples))
