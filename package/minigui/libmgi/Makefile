#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmgi
PKG_VERSION:=2.0.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=22ce4240b0d4c1c6268a27ab68fb02d7

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/libmgi
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libstdcpp
  TITLE:=libmgi
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/libmgi/description
  mGi is a component of MiniGUI, which provides 
  support for input methods on MiniGUI.
endef

CONFIGURE_ARGS += \
	--with-skbsize=800x260

TARGET_LDFLAGS+= -lts -luapi

LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lfreetype -lts

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgi \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgi*.{a,la,so*} \
		$(1)/usr/lib/
endef

define Package/libmgi/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmgi))
