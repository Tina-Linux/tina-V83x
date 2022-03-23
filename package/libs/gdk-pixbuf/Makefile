#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=gdk-pixbuf
PKG_VERSION:=2.36.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://ftp.gnome.org/pub/gnome/sources/gdk-pixbuf/$(PKG_SOURCE)
PKG_MD5SUM:=

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0

include $(BUILD_DIR)/package.mk

TARGET_LDFLAGS+= \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/gdk-pixbuf
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=gdk-pixbuf
  DEPENDS:=+glib2 +libjpeg +libpng
  URL:=http://www.gtk.org/
endef

define Package/gdk-pixbuf/description
Gdk-Pixbuf is an image loader and scaler. It uses GObject
and the GLib, to integrate well with GNOME applications.
endef

CONFIGURE_ARGS += \
	--disable-gtk-doc \
	--disable-gtk-doc-html \
	--disable-doc \
	--disable-docs \
	--disable-documentation \
	--with-xmlto=no \
	--with-fop=no \
	--disable-dependency-tracking \
	--enable-ipv6 \
	--enable-static \
	--enable-shared \
	--disable-glibtest \
	--without-libtiff \
	--disable-gio-sniffing
	#--without-libpng \
	--without-libjpeg \
	--disable-gio-sniffing

TARGET_LDFLAGS+= \
	-L$(STAGING_DIR)/usr/lib/

CONFIGURE_VARS += \
	CXX="$(TARGET_CC)"

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/lib/pkgconfig \
		$(1)/usr/include

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/* \
		$(1)/usr/lib/

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/* \
		$(1)/usr/include/
endef

define Package/gdk-pixbuf/install
	$(INSTALL_DIR) \
		$(1)/usr/bin
	$(INSTALL_DIR) \
		$(1)/usr/lib

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/\
		$(1)/usr/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,gdk-pixbuf))
