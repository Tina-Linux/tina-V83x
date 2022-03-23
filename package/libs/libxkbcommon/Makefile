#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free and open source software.
# It is available to be redistributed and/or modified under the terms of the MIT license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libxkbcommon
PKG_VERSION:=0.6.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://xkbcommon.org/download/libxkbcommon-0.6.1.tar.xz
PKG_MD5SUM:=67a8f322b5fa32352272e811bb90dd73
PKG_MAINTAINER:=Ran Benita<ran234@gmail.com>

PKG_LICENSE:=MIT/X11
PKC_LICENSE_FILES:=LICENSE

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0

include $(BUILD_DIR)/package.mk

define Package/libxkbcommon
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+xkeyboard-config
  TITLE:=xkbcommon is a library to handle keyboard descriptions
  URL:=https://xkbcommon.org/
endef

define Package/libxkbcommon/description
  xkbcommon is a library to handle keyboard descriptions, including loading them from disk,
parsing them and handling their state. It's mainly meant for client toolkits, window systems,
and other system applications; currently that includes Wayland, kmscon, GTK+, Qt, Clutter,
KWin and more. It is also used by some XCB applications for proper keyboard support.

endef

TARGET_CFLAGS += $(FPIC)

# uses C99 features
TARGET_CFLAGS += --std=gnu99

ifeq ($(CONFIG_WESTON_X11),y)
CONFIGURE_ARGS += --enable-x11
DEPENDS += libxcb
else
CONFIGURE_ARGS += --disable-x11
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/xkbcommon
	$(CP) $(PKG_INSTALL_DIR)/usr/include/xkbcommon/xkbcommon*.h $(1)/usr/include/xkbcommon
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxkbcommon.{a,la,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/xkbcommon.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libxkbcommon/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libxkbcommon.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libxkbcommon))
