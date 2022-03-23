#
# Copyright (C) 2009-2010 OpenWrt.org
#
# Primarily used for software libraries, the GNU LGPL requires that derived works be licensed under the same license,
# but works that only link to it do not fall under this restriction. There are two commonly used versions of the GNU LGPL.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fribidi
PKG_VERSION:=1.0.1
PKG_RELEASE:=1
PKG_LICENSE:=GPL

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=https://github.com/fribidi/fribidi/releases/download/v1.0.1
PKG_MD5SUM:=43dc37bd95551d69a7b04c05f1d95721

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/libfribidi
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Free Implementation of the Unicode Bidirectional Algorithm
  DEPENDS:=+glib2
  URL:=https://github.com/fribidi/fribidi
  MAINTAINER:=Behdad Esfahbod <behdad@gnu.org>
endef

define Package/libfribidi/description
One of the missing links stopping the penetration of free software in Middle
East is the lack of support for the Arabic and Hebrew alphabets. In order to
have proper Arabic and Hebrew support, the bidi algorithm needs to be implemented. It is
our hope that this library will stimulate more free software in the Middle
Eastern countries.
endef

TARGET_CFLAGS += $(FPIC)

define Package/libfribidi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

$(eval $(call BuildPackage,libfribidi))
