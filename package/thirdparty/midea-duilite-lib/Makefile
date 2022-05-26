#
# Copyright (C) 2019 Allwinner Technology Limited. All rights reserved
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
#

include $(TOPDIR)/rules.mk
ifeq (, ($(wildcard $(TINA_BUILD_TOP)/.config)))
        include $(TINA_BUILD_TOP)/.config
endif

PKG_NAME:=midea-duilite-lib
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  TITLE:=duilite libs for midea speechmodule
  DEPENDS:= +libstdcpp +libc +alsa-lib
endef

define Package/$(PKG_NAME)/description
  aispeech duilite for asr/tts/wakeup and so on
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/config
choice
prompt "Choosing ASR libraries"
	depends on PACKAGE_midea-duilite-lib
	default R328_DUILITE_V1
	help
	Select duilite ASR lib version.
config R328_DUILITE_V1
	bool "duilite libs v1 version based on R328"
endchoice
endef

define Build/Configure
endef

define Build/Compile
endef

#
# add ld path /mnt/app/libs
# app should add "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mnt/app/libs"
#
define Build/InstallDev

ifeq ($(CONFIG_DUILITE_R328),y)
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include

	#$(INSTALL_DIR) $(1)/usr/lib
	#$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/mnt/app $(1)/mnt/app/libs
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/* $(1)/mnt/app/libs
endif
endef

define Package/$(PKG_NAME)/install

ifeq ($(CONFIG_R328_DUILITE_V1),y)
	#$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/bin
	#$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/*.so $(1)/usr/lib
	#$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/*.bin $(1)/usr/bin

	$(INSTALL_DIR) $(1)/mnt/app $(1)/mnt/app/libs $(1)/mnt/app/res
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/*.so $(1)/mnt/app/libs
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/*.bin $(1)/mnt/app/res
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
