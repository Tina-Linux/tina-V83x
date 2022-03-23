#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=openmp-test
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/openmp-test
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=openmp testing tool
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)/usr"

endef

define Package/openmp-test/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openmp1 $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/openmp2 $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/fib $(1)/usr/bin/
endef

$(eval $(call BuildPackage,openmp-test))
