#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=pango
PKG_VERSION:=1.40.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNOME/$(PKG_NAME)/1.40/
PKG_MD5SUM:=

PKG_FIXUP:=autoreconf --force --install
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/nls.mk

TARGET_LDFLAGS+= \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/pango
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Text layout and rendering library
  DEPENDS:=+libcairo +glib2 +fontconfig +libfreetype \
	  +harfbuzz
  URL:=http://www.pango.org/
endef

define Package/pango/description
Pango is a library for layout and rendering of text, with an emphasis
on internationalization. Pango can be used anywhere that text layout
is needed; however, most of the work on Pango so far has been done using
the GTK+ widget toolkit as a test platform. Pango forms the core of text
and font handling for GTK+-2.x.
endef

CONFIGURE_ARGS += \
	--enable-explicit-deps=no \
	$(if $(CONFIG_PACKAGE_libX11), --with-x, --without-x)

TARGET_CFLAGS+= \
	-I$(STAGING_DIR)/usr/include/harfbuzz/
TARGET_LDFLAGS+= \
	-L$(STAGING_DIR)/usr/lib/

CONFIGURE_VARS += \
	CXX="$(TARGET_CC)"

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/lib/pkgconfig \
		$(1)/usr/lib/pango/1.6.0/modules \
		$(1)/usr/include

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/* \
		$(1)/usr/include/
endef

define Package/pango/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/pango/1.6.0/modules \
		$(1)/usr/bin \
		$(1)/etc/init.d

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,pango))
