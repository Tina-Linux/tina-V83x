#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cell-phone-ux-demo
PKG_VERSION:=
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=e14dc09754a20c11e88ddbfcf3fdb04f

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1
CMAKE_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libmgncs +libmgeff +libxml2 +libsqlite3 +libchipmunk +mgncs4touch
  TITLE:=cellphoneuxdemo
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/$(PKG_NAME)/description
  The minigui test case
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef

ifeq ($(CONFIG_G2D_SUPPORT),y)
TARGET_LDFLAGS+= -luapi
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -luapi
endif
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/cellphoneuxdemo
	$(INSTALL_DIR) $(1)/usr/bin/cellphoneuxdemo/etc
	$(INSTALL_DIR) $(1)/usr/bin/cellphoneuxdemo/res
	$(INSTALL_DIR) $(1)/usr/bin/cellphoneuxdemo/res/font
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) \
		./run_cellphone.sh \
		$(1)/usr/bin/
	$(CP) \
		./MiniGUI.cfg \
		$(1)/usr/local/etc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/etc/* \
		$(1)/usr/bin/cellphoneuxdemo/etc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/res/* \
		$(1)/usr/bin/cellphoneuxdemo/res
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/{activity-loader,mguxdemo} \
		$(1)/usr/bin/cellphoneuxdemo
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/minigui/res/font/* \
		$(1)/usr/bin/cellphoneuxdemo/res/font
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/{mgncs4touch.cfg,mgncs.cfg} \
		$(1)/usr/local/etc
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
