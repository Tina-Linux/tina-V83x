#
#Copyright (C) 2006-2016 OpenWrt.org
#
#This is free software, licensed under the GNU General Public License v2.
#See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uriparser
PKG_VERSION:=0.8.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_LICENSE:=BSD-3-Clause

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_REMOVE_FILES:=autogen.sh aclocal.m4

include $(BUILD_DIR)/package.mk

define Package/uriparser
 SECTION:=libs
 CATEGORY:=Libraries
 TITLE:=uriparser
endef

define Package/uriparser/description
uriparser is a strictly RFC 3986 compliant URI parsing and handling library written in C.
uriparser is cross-platform, fast, supports Unicode and is licensed under the New BSD license.
endef


CONFIGURE_ARGS+= \
		--disable-test \
		--disable-doc


define Package/uriparser/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liburiparser.so.* $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/uriparse $(1)/usr/bin/
endef

$(eval $(call BuildPackage,uriparser))
