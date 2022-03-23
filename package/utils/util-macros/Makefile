
#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=util-macros
PKG_VERSION:=1.19.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://xorg.freedesktop.org/releases/individual/util
PKG_MD5SUM:=1cf984125e75f8204938d998a8b6c1e1

include $(BUILD_DIR)/package.mk

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

define Package/util-macros
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=X.Org Autotools macros
  URL:=https://xorg.freedesktop.org/
  MAINTAINER:=<xorg_security@x.org>
endef

define Package/util-macros/description
  The util-macros package contains the m4 macros used by all of the Xorg packages.
  This package is known to build and work properly using an LFS-8.1 platform.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/share/aclocal
	$(CP) $(PKG_INSTALL_DIR)/usr/share/aclocal/xorg-macros.m4 $(1)/usr/share/aclocal
	$(INSTALL_DIR) $(1)/usr/share/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/share/pkgconfig/xorg-macros.pc $(1)/usr/share/pkgconfig
	$(INSTALL_DIR) $(1)/usr/share/util-macros
	$(CP) $(PKG_INSTALL_DIR)/usr/share/util-macros/INSTALL $(1)/usr/share/util-macros
endef

define Package/util-macros/install
endef

$(eval $(call BuildPackage,util-macros))
