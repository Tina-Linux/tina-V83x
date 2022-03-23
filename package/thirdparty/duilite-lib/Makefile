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

PKG_NAME:=duilite-lib
PKG_VERSION:=0.11.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  #SECTION:=libs
  #CATEGORY:=Libraries
  CATEGORY:=Thirdparty
  TITLE:=duilite for voice assitant
  DEPENDS:= +libstdcpp +libc +alsa-lib
endef

define Package/$(PKG_NAME)/description
  aispeech duilite for asr/tts/wakeup and so on
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/config
choice
prompt "Choosing aispeech ASR libraries"
	depends on PACKAGE_duilite-lib
	default DUILITE_R328
	help
	Select duilite ASR libraries.
config DUILITE_R328
	bool "duilite libs based on R328"
endchoice
endef

define Build/Configure
endef

define Build/Compile
endef

#
# add ld path /mnt/app/lib
# app should add "export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/mnt/app/lib"
#
define Build/InstallDev

ifeq ($(CONFIG_DUILITE_R328),y)
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/*.h $(1)/usr/include

	#$(INSTALL_DIR) $(1)/usr/lib
	#$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/mnt/app $(1)/mnt/app/lib
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/* $(1)/mnt/app/lib
endif
endef

define Package/$(PKG_NAME)/install

ifeq ($(CONFIG_DUILITE_R328),y)
	#$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/bin
	#$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/*.so $(1)/usr/lib
	#$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/*.bin $(1)/usr/bin

	$(INSTALL_DIR) $(1)/mnt/app $(1)/mnt/app/lib $(1)/mnt/app/bin
	$(CP) -uvd $(PKG_BUILD_DIR)/lib32/a7/*.so $(1)/mnt/app/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/*.bin $(1)/mnt/app/bin
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
