#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=glib-networking
PKG_VERSION:=2.50.0
PKG_RELEASE:=1

PKG_SOURCE:=glib-networking-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(COMPILE_DIR)/glib-networking-$(PKG_VERSION)

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/nls.mk

define Package/glib-networking
  SUBMENU:=gnome
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2 +libgnutls +ca-certificates
  TITLE:=The GLib Networking
endef

define Package/glib-networking/description
  The GLib Networking package contains Network related gio modules for GLib.
  This package is known to build and work properly using an LFS-8.0 platform.
  `export GIO_USE_TLS=gnutls` to config & enable the module to gio
endef


CONFIGURE_ARGS += \
    --disable-more-warnings \
	--with-ca-certificates=/etc/ssl/certs/ca-certificates.crt \
    --disable-static

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/gio/modules
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/gio/modules/libgiognutls.{so,la} $(1)/usr/lib/gio/modules/
endef

define Package/glib-networking/install
	$(INSTALL_DIR) $(1)/usr/lib/gio/modules
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/gio/modules/libgiognutls.so $(1)/usr/lib/gio/modules/
endef

$(eval $(call BuildPackage,glib-networking))
