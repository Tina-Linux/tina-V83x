#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=powerkey
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
include $(BUILD_DIR)/package.mk


define Package/powerkey
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=powerkey for allwinner
  DEPENDS:=+libpthread
  MENU:=1
endef

define Package/powerkey/description
  powerkey for Allwinner
endef


define Build/Prepare
	$(CP) -r ./daemon $(PKG_BUILD_DIR)/
	$(CP) -r ./files $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/daemon \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef
$(warning "the value of LOCAL_PATH is 111 $(1)")
$(warning "the value of  is$(PKG_BUILD_DIR)")
define Package/powerkey/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/daemon/powerkey_daemon $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/powerkey.init $(1)/etc/init.d/

endef

$(eval $(call BuildPackage,powerkey))
