#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=smart-music-player
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +alsa-utils +tslib +libcedarx +libuapi
  TITLE:=smart-music-player
  MAINTAINER:=arl<anruliu@allwinnertech.com>
  MENU:=1
endef

define Package/$(PKG_NAME)/description
  The minigui application for music-player. Compatible with R328 platform.
  Adapt to screen resolutions of 240*320 and 320*480.
endef

define Package/$(PKG_NAME)/config
if PACKAGE_$(PKG_NAME)
config SMART_MUSIC_PLAYER_AUTO_RUN
    bool "smart-music-player auto run"
    default n
        help
        Whether to run from the power on
choice
prompt "Choose display resolution"
	default SMART_MUSIC_PLAYER_320x480
	help
	Select application display resolution
config SMART_MUSIC_PLAYER_240x320
	bool "smart_music_player_240x320"
config SMART_MUSIC_PLAYER_320x480
	bool "smart_music_player_320x480"
endchoice
endif
endef

ifeq ($(CONFIG_SMART_MUSIC_PLAYER_240x320),y)
TARGET_CFLAGS+=-D__SMART_MUSIC_PLAYER_240x320__
endif

ifeq ($(CONFIG_SMART_MUSIC_PLAYER_320x480),y)
TARGET_CFLAGS+=-D__SMART_MUSIC_PLAYER_320x480__
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	mkdir -p $(PKG_BUILD_DIR)/src
	mkdir -p $(PKG_BUILD_DIR)/res
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./res $(PKG_BUILD_DIR)/
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
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(CP) \
		$(PKG_BUILD_DIR)/src/smart-music-player \
		$(1)/usr/bin/
	$(CP) \
		$(PKG_BUILD_DIR)/res/* \
		$(1)/usr/res/
	$(CP) \
		$(PKG_BUILD_DIR)/res/config/MiniGUI.cfg \
		$(1)/usr/local/etc/
#	$(CP) \
		$(PKG_BUILD_DIR)/src/pointercal \
		$(1)/etc/
	$(CP) \
		$(PKG_BUILD_DIR)/res/config/sunxi-keyboard.kl \
		$(1)/etc/
ifeq ($(CONFIG_SMART_MUSIC_PLAYER_240x320),y)
	rm -r $(1)/usr/res/picture/320x480
	rm $(1)/usr/res/config/MiniGUI.cfg
	rm $(1)/usr/res/config/MiniGUI_320x480.cfg
	rm $(1)/usr/res/config/system_320x480.cfg
endif
ifeq ($(CONFIG_SMART_MUSIC_PLAYER_320x480),y)
	rm -r $(1)/usr/res/picture/240x320
	rm $(1)/usr/res/config/MiniGUI.cfg
	cp $(1)/usr/res/config/system_320x480.cfg $(1)/usr/res/config/system.cfg
	cp $(1)/usr/res/config/MiniGUI_320x480.cfg $(1)/usr/local/etc/MiniGUI.cfg
	rm $(1)/usr/res/config/MiniGUI_320x480.cfg
	rm $(1)/usr/res/config/system_320x480.cfg
endif
ifeq ($(CONFIG_SMART_MUSIC_PLAYER_AUTO_RUN),y)
	$(CP) \
		./src/smart-music-player \
		$(1)/etc/init.d/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
