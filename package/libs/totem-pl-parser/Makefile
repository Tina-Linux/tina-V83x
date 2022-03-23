#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=totem-pl-parser
PKG_VERSION:=3.10.8
PKG_RELEASE:=1

PKG_SOURCE:=totem-pl-parser-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(COMPILE_DIR)/totem-pl-parser-$(PKG_VERSION)

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/totem-pl-parser
  SUBMENU:=gnome
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2 +libsoup +libxml2 +gmime +libgcrypt
  TITLE:=The GLib Networking
endef

define Package/totem-pl-parser/description
  The GLib Networking package contains Network related gio modules for GLib.
  This package is known to build and work properly using an LFS-8.0 platform.
  `export GIO_USE_TLS=gnutls` to config & enable the module to gio
endef


CONFIGURE_ARGS += \
    --disable-more-warnings \
	--enable-uninstalled-tests

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/
	$(CP) $(PKG_INSTALL_DIR)/usr/* $(1)/usr/
endef

define Package/totem-pl-parser/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtotem-plparser.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtotem-plparser-mini.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,totem-pl-parser))
