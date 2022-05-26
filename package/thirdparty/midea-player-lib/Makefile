#
# Copyright (C) 2018 Allwinner Technology Limited. All rights reserved
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
#

include $(TOPDIR)/rules.mk
ifeq (, ($(wildcard $(TINA_BUILD_TOP)/.config)))
        include $(TINA_BUILD_TOP)/.config
endif

PKG_NAME:=midea-player-lib
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  TITLE:=mplayer lib
  DEPENDS:=+alsa-lib +libstdcpp
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/config
choice
prompt "Choosing mplayer lib version"
	depends on PACKAGE_midea-player-lib
	default MPLAYER_V1
	help
	Select mplayer lib version.
config MPLAYER_V1
	bool "mplayer version V1 @20190723"
config MPLAYER_V2
	bool "mplayer version V2 @21991230"
endchoice
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/mnt/app/libs $(1)/usr/include
ifeq ($(CONFIG_MPLAYER_V1),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib/v1/* $(1)/mnt/app/libs/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/v1/*.h $(1)/usr/include/
endif

ifeq ($(CONFIG_MPLAYER_V2),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib/v2/* $(1)/mnt/app/libs/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/v2/*.h $(1)/usr/include/
endif

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/mnt/app/libs
ifeq ($(CONFIG_MPLAYER_V1),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib/v1/*.so $(1)/mnt/app/libs/
endif

ifeq ($(CONFIG_MPLAYER_V2),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib/v2/*.so $(1)/mnt/app/libs/
endif

endef

$(eval $(call BuildPackage,$(PKG_NAME)))
