#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mdolphin-release-tv
PKG_VERSION:=2.0.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
#PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=e4b9966979888fb5e1196d2d533258d9

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/${PKG_NAME}
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libmdolphin +libmgutils +libmgplus +libxml2 +libmgi
  TITLE:=mdolphin-release-tv
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/${PKG_NAME}/description
  The minigui mdolphin test case
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef
TARGET_CXXFLAGS+="-Wno-narrowing" -fpermissive

CONFIGURE_ARGS += \
	--with-lang=zhcn

TARGET_LDFLAGS+= -lts
LDFLAGS+=-lts
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lfreetype -lmgplus -lmdolphin -lmgi -lstdc++ -lxml2 -lts

define Package/${PKG_NAME}/install
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(INSTALL_DIR) $(1)/usr/share/mdolphin-release-tv
	$(INSTALL_DIR) $(1)/usr/share/mdolphin-release-tv/mdtv
	$(INSTALL_DIR) $(1)/usr/share/mdolphin-release-tv/mdtv/res
	$(INSTALL_DIR) $(1)/usr/share/mdolphin-release-tv/svgui_test
	$(CP) \
		./MiniGUI.cfg \
		$(1)/usr/local/etc
	$(CP) \
		$(PKG_BUILD_DIR)/src/mdtv \
		$(1)/usr/share/mdolphin-release-tv/mdtv
	$(CP) \
		$(PKG_BUILD_DIR)/svgui/svgui_test \
		$(1)/usr/share/mdolphin-release-tv/svgui_test
	$(CP) \
		$(PKG_BUILD_DIR)/src/res/* \
		$(1)/usr/share/mdolphin-release-tv/mdtv/res/
	$(CP) \
		$(PKG_BUILD_DIR)/src/mdtv.cfg \
		$(1)/usr/share/mdolphin-release-tv/mdtv
	$(CP) \
		$(PKG_BUILD_DIR)/svgui/*.png \
		$(1)/usr/share/mdolphin-release-tv/svgui_test
endef
$(eval $(call BuildPackage,${PKG_NAME}))
