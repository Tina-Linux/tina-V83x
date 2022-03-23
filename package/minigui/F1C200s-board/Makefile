#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=F1C200s-board
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +tslib +libmgi +libmgplus +libuapi
  TITLE:=F1C200s-board
  MAINTAINER:=hyx<huangyixiu@allwinnertech.com>
endef

define Package/$(PKG_NAME)/description
  The minigui application for F1C200s-board
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) -r ./src $(PKG_BUILD_DIR)/
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
	$(CP) \
		$(PKG_BUILD_DIR)/src/F1C200s-board \
		$(1)/usr/bin/
	$(CP) \
		$(PKG_BUILD_DIR)/src/res/* \
		$(1)/usr/res/
	$(CP) \
		$(PKG_BUILD_DIR)/src/res/MiniGUI.cfg \
		$(1)/usr/local/etc/
	$(CP) \
		$(PKG_BUILD_DIR)/src/pointercal \
		$(1)/etc/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
