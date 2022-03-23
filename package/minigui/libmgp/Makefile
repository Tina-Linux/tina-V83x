#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmgp
PKG_VERSION:=1.2.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=68e6aee42895d8bddd933f35b9ad7385

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/libmgp
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl
  TITLE:=libmgp
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/libmgp/description
  mGp is one of components of MiniGUI,
  which provides support for printing on MiniGUI.
endef

CONFIGURE_ARGS += \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgp \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgp*.{a,la,so*} \
		$(1)/usr/lib/
endef

define Package/libmgp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/* \
		$(1)/usr/bin/
endef

$(eval $(call BuildPackage,libmgp))
