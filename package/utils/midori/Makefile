#
# Copyright (C) 2006-2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=midori
PKG_VERSION:=0.5.11
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)_$(PKG_VERSION)_all_.tar.bz2
PKG_SOURCE_URL:=http://www.midori-browser.org/downloads/
PKG_HASH:=
PKG_MAINTAINER:=xlf<xielinfei@allwinnertech.com>

PKG_LICENSE:=LGPL-2.1+
PKG_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf

include $(BUILD_DIR)/nls.mk
include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

MIDORI_CONF_OPTS = \
	-DHALF_BRO_INCOM_WEBKIT2=ON \
	-DUSE_GRANITE=ON \
	-DUSE_GTK3=ON \
	-DUSE_ZEITGEIST=OFF

CMAKE_OPTIONS += \
	$(MIDORI_CONF_OPTS)

TARGET_LDFLAGS+= \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
	-L$(TOOLCHAIN_DIR)/usr/lib -L$(TOOLCHAIN_DIR)/lib

define Package/midori
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=$(ICONV_DEPENDS) $(INTL_DEPENDS) +libncurses \
	  +intltool/host +pkgconf/host \
	  +vala/host +python/host +granite \
	  +libgtk3 +libsoup +libxml2 +libsqlite3 \
	  +webkitgtk
  TITLE:=Midori browser
  URL:=https://midori-browser.org/
  SUBMENU:=Browser
endef

define Package/midori/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/etc/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
endef

$(eval $(call BuildPackage,midori))
