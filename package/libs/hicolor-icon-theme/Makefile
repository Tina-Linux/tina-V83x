#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# $Id: Makefile 12225 2008-08-06 22:10:29Z nbd $

include $(TOPDIR)/rules.mk

PKG_NAME:=hicolor-icon-theme
PKG_VERSION:=0.17
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://icon-theme.freedesktop.org/releases/
#PKG_MD5SUM:=dee233bf288ee795ac96a98cc2e369b6

include $(BUILD_DIR)/package.mk

PKG_INSTALL=1

define Package/hicolor-icon-theme
	SECTION:=libs
	CATEGORY:=Libraries
  SUBMENU:=data
  TITLE:=Default icon theme
  URL:=http://icon-theme.freedesktop.org/wiki/HicolorTheme
endef

define Package/hicolor-icon-theme/install
	$(INSTALL_DIR) $(1)/usr/share/icons
	$(CP) $(PKG_INSTALL_DIR)/usr/share/icons/* $(1)/usr/share/icons/
endef

$(eval $(call BuildPackage,hicolor-icon-theme))
