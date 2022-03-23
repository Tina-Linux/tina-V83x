#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=mad-demo
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:= MAD demo
  DEPENDS:= +alsa-lib
endef

define Package/$(PKG_NAME)/description
    MAD/VAD demo
endef

define Build/Prepare
	$(CP) -r ./files/src $(PKG_BUILD_DIR)/

endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mad-capture $(1)/usr/bin
	$(INSTALL_BIN) ./files/mad-para $(1)/usr/bin
	$(INSTALL_BIN) ./files/mad-ac108-demo $(1)/usr/bin
	$(INSTALL_BIN) ./files/mad-dmic-demo $(1)/usr/bin
	$(INSTALL_BIN) ./files/vad-demo $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
