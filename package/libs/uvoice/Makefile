#
# Copyright (C) 2018 Allwinner Technology Limited. All rights reserved
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
#

include $(TOPDIR)/rules.mk

PKG_NAME:=uvoice
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Uvoice for voice assitant
  DEPENDS:=+libstdcpp
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include/
ifeq ($(CONFIG_PLATFORM_v316),y)
	$(CP) $(PKG_BUILD_DIR)/lib32/v316/* $(1)/usr/lib
else
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/arm926ejs/* $(1)/usr/lib/
endif
endef

define Package/uvoice/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
ifeq ($(CONFIG_PLATFORM_v316),y)
	$(CP) $(PKG_BUILD_DIR)/lib32/v316/kws.bin $(1)/usr/bin/
else
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/arm926ejs/*.so $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lib32/arm926ejs/kws.bin $(1)/usr/bin/
endif
endef

$(eval $(call BuildPackage,uvoice))
