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


PKG_NAME:=uvoice-lib
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  TITLE:=Uvoice Library for offline ASR
  DEPENDS:=+libstdcpp
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/config
choice
prompt "Choosing uvoice ASR libraries"
	depends on PACKAGE_uvoice-lib
	default UVOICE_AC_XIAOZHI_R328
	help
	Select UVOICE ASR libraries.
config UVOICE_AC_XIAOZHI_R328
	bool "R328:uvoice libs for ac xiaozhi"
config UVOICE_HR
	bool "R6:uvoice libs for hr small and big(default) mode"
config UVOICE_YM
	bool "R6:uvoice libs for ym small and big(default) mode"
endchoice
endef

define Build/Configure
endef

define Build/Compile
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
ifeq ($(CONFIG_UVOICE_HR),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/arm926ejs/hr_libs/* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/hr_libs/*.h $(1)/usr/include/
endif

ifeq ($(CONFIG_UVOICE_AC_XIAOZHI_R328),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/ac_libs/* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/ac_libs/*.h $(1)/usr/include/
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/ac_libs/*.so $(STAGING_DIR)/usr/lib/
endif

ifeq ($(CONFIG_UVOICE_YM),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/arm926ejs/ym_libs/* $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/ym_libs/*.h $(1)/usr/include/
endif
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/bin

ifeq ($(CONFIG_UVOICE_HR),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/arm926ejs/hr_libs/*.so $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lib32/arm926ejs/hr_libs/kws.bin $(1)/usr/bin/
endif

ifeq ($(CONFIG_UVOICE_AC_XIAOZHI_R328),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/ac_libs/*.so $(1)/usr/lib/
endif

ifeq ($(CONFIG_UVOICE_YM),y)
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/arm926ejs/ym_libs/*.so $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/lib32/arm926ejs/ym_libs/kws.bin $(1)/usr/bin/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
