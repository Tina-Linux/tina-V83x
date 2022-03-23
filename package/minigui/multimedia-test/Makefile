#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=multimedia-test
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +alsa-utils +tslib +libcedarx +libuapi
  TITLE:=multimedia-test
  MAINTAINER:=arl<anruliu@allwinnertech.com>
endef

define Package/$(PKG_NAME)/description
  The minigui application for multimedia-test.Compatible with R11, R16, R18, R40, F35 platform.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	mkdir -p $(PKG_BUILD_DIR)/src
	mkdir -p $(PKG_BUILD_DIR)/res
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./res $(PKG_BUILD_DIR)/

ifeq ($(TARGET_BOARD_PLATFORM),r16)
	$(CP) ./res/SetAudioPass/r16_multimedia-test.sh $(PKG_BUILD_DIR)/res/SetAudioPass
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
endif

ifeq ($(TARGET_BOARD_PLATFORM),r40)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) ./res/SetAudioPass/r40_multimedia-test.sh $(PKG_BUILD_DIR)/res/SetAudioPass
endif

ifeq ($(TARGET_BOARD_PLATFORM),r18)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) ./res/SetAudioPass/r18_multimedia-test.sh $(PKG_BUILD_DIR)/res/SetAudioPass
endif

ifeq ($(TARGET_BOARD_PLATFORM),r11)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) ./res/SetAudioPass/MR100_multimedia-test.sh $(PKG_BUILD_DIR)/res/SetAudioPass
endif
ifeq ($(TARGET_BOARD_PLATFORM),r7)
	$(info *****************platform = $(TARGET_BOARD_PLATFORM)**************)
	$(CP) ./res/SetAudioPass/MR100_multimedia-test.sh $(PKG_BUILD_DIR)/res/SetAudioPass
endif
endef

TARGET_LDFLAGS+=-lfreetype

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/res
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/etc/res/
	$(CP) \
		$(PKG_BUILD_DIR)/src/multimedia-test \
		$(1)/usr/bin/
	$(CP) \
		$(PKG_BUILD_DIR)/res/* \
		$(1)/usr/res/
	$(CP) \
		$(PKG_BUILD_DIR)/res/wm_res/* \
		$(1)/etc/res/
	$(CP) \
		$(PKG_BUILD_DIR)/res/minigui/MiniGUI.cfg \
		$(1)/usr/local/etc/
	$(CP) \
		$(PKG_BUILD_DIR)/src/pointercal \
		$(1)/etc/
	$(CP) \
		$(PKG_BUILD_DIR)/res/minigui/sunxi-keyboard.kl \
		$(1)/etc/
#ifeq ($(TARGET_BOARD_PLATFORM),y)
#	rm -r $(1)/usr/res/menu/64x64
#	cp $(1)/usr/res/config/system_R11.cfg $(1)/usr/res/config/system.cfg
#	cp $(1)/usr/res/minigui/MiniGUI_R11.cfg $(1)/usr/local/etc/MiniGUI.cfg
#endif
ifeq ($(TARGET_BOARD_PLATFORM),r16)
#	rm -r $(1)/usr/res/menu/64x64
	cp $(1)/usr/res/config/system_R16.cfg $(1)/usr/res/config/system.cfg
	cp $(1)/usr/res/minigui/MiniGUI_R16.cfg $(1)/usr/local/etc/MiniGUI.cfg
endif
ifeq ($(TARGET_BOARD_PLATFORM),r18)
#	rm -r $(1)/usr/res/menu/64x64
	cp $(1)/usr/res/config/system_R18.cfg $(1)/usr/res/config/system.cfg
	cp $(1)/usr/res/minigui/MiniGUI_R18.cfg $(1)/usr/local/etc/MiniGUI.cfg
endif
ifeq ($(TARGET_BOARD_PLATFORM),r40)
#	rm -r $(1)/usr/res/menu/128x128
	cp $(1)/usr/res/config/system_R40.cfg $(1)/usr/res/config/system.cfg
	cp $(1)/usr/res/minigui/MiniGUI_R40.cfg $(1)/usr/local/etc/MiniGUI.cfg
endif
ifeq ($(TARGET_BOARD_PLATFORM),r11)
#	rm -r $(1)/usr/res/menu/128x128
	cp $(1)/usr/res/config/system_F35.cfg $(1)/usr/res/config/system.cfg
	cp $(1)/usr/res/minigui/MiniGUI_F35.cfg $(1)/usr/local/etc/MiniGUI.cfg
endif

ifeq ($(TARGET_BOARD_PLATFORM),r7)
#	rm -r $(1)/usr/res/menu/128x128
	cp $(1)/usr/res/config/system_F35.cfg $(1)/usr/res/config/system.cfg
	cp $(1)/usr/res/minigui/MiniGUI_F35.cfg $(1)/usr/local/etc/MiniGUI.cfg
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
