#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=libsecret
PKG_VERSION:=0.18.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://ftp.gnome.org/pub/GNOME/sources/libsecret/0.18/
PKG_MD5SUM:=

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/libsecret
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=libsecret
  DEPENDS:=+gobject-introspection/host +glib2 +intltool/host +libgcrypt
  URL:=http://ftp.gnome.org/
endef

define Package/libsecret/description
secret libraries
endef

TARGET_CFLAGS += $(FPIC)
CONFIGURE_VARS += CC="$(TARGET_CROSS)gcc"
CONFIGURE_ARGS += \
		--disable-gtk-doc \
		--disable-gtk-doc-html \
		--disable-docs \
		--disable-doc \
		--disable-documentation \
		--with-xmlto=no \
		--with-fop=no \
		--disable-dependency-tracking \
		--disable-manpages \
		--with-pic \
		--disable-static \
		--disable-strict \
		--disable-coverage \
		--enable-introspection=no \
		--enable-vala=no \
		--disable-gcrypt

#define Build/Configure
#	( cd $(PKG_BUILD_DIR); ./autogen.sh ;\
#		CFLAGS="$(TARGET_CFLAGS) $(FPIC)" \
#		./configure $(CONFIGURE_ARGS)
#	);
#endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/libsecret/install
	$(INSTALL_DIR) \
		$(1)/usr/lib

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libsecret))
