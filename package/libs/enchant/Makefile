#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=enchant
PKG_VERSION:=1.6.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.abisource.com/downloads/enchant/$(PKG_VERSION)
PKG_MD5SUM:=

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)-1.6.0

include $(BUILD_DIR)/package.mk

define Package/enchant
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=enchant
  DEPENDS:=+glib2
  URL:=http://www.abisource.com/
endef

define Package/enchant/description
enchant libraries
endef

CONFIGURE_VARS += CC="$(TARGET_CROSS)gcc"
TARGET_CFLAGS += $(FPIC)

#define Build/Configure
#	(cd $(PKG_BUILD_DIR); \
#		CFLAGS="$(TARGET_CFLAGS) $(FPIC)" \
#		./configure \
#		--enable-static \
#		--enable-shared \
#	);
#endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/enchant/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,enchant))
