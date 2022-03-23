#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ota-burnboot
PKG_VERSION:=0.1
PKG_RELEASE:=0.1

include $(BUILD_DIR)/package.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=burn boot0/uboot for ota.
  DEPENDS:= libc
endef

define Package/$(PKG_NAME)/description
  This package is used for ota to burn boot0/uboot.

  Contains:
   - ota-burnboot0
   - ota-burnuboot
   - libota-burnboot.so
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/OTA_BurnBoot.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/*.so $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/OTA_BurnBoot.h $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/*.so $(1)/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/{ota-burnboot0,ota-burnuboot} $(1)/sbin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
