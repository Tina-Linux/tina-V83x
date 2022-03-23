#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mdolphin-samples
PKG_VERSION:=3.0.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libmgutils +libmgplus +libcrypto +libcurl +libnghttp2 +libssl +libmdolphin
  TITLE:=mdolphin-samples
  MAINTAINER:=xielinfei<xielinfei@allwinnertech.com>
  URL:=http://www.minigui.org/
endef

define Package/$(PKG_NAME)/description
  The Mdolphin Test Case
endef

CONFIGURE_ARGS += \
	--with-lang=zhcn
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lfreetype -lcrypto -lts -lcurl -lssl -lcurl -lnghttp2 -lmdolphin
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/mobile
	$(CP) \
		$(PKG_BUILD_DIR)/mobile/mdolphin_app \
		$(1)/usr/bin/mobile/
	$(INSTALL_DIR) $(1)/usr/bin/pc
	$(CP) \
		$(PKG_BUILD_DIR)/pc/mdolphin_app \
		$(1)/usr/bin/pc/
	$(INSTALL_DIR) $(1)/usr/bin/pda
	$(CP) \
		$(PKG_BUILD_DIR)/pda/mdolphin_app \
		$(1)/usr/bin/pda/
	$(INSTALL_DIR) $(1)/usr/bin/pc-3
	$(CP) \
		$(PKG_BUILD_DIR)/pc-3/mdolphin_app \
		$(1)/usr/bin/pc-3/
	$(INSTALL_DIR) $(1)/usr/bin/spider
	$(CP) \
		$(PKG_BUILD_DIR)/spider/mdolphin_app \
		$(1)/usr/bin/spider/
	$(INSTALL_DIR) $(1)/usr/bin/simplespider
	$(CP) \
		$(PKG_BUILD_DIR)/simplespider/mdolphin_app \
		$(1)/usr/bin/simplespider/
endef

$(eval $(call BuildPackage,mdolphin-samples))
