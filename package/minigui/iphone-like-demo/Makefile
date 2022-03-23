#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=iphone-like-demo
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=5519dc825f927e2bf0c97db376bd5647

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/iphone-like-demo
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libstdcpp
  TITLE:=iphone like demo
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/iphone-like-demo/description
  iphone-like-demo is a component of MiniGUI, which provides 
  support for advanced graphics function. 
endef

ifeq ($(CONFIG_TSLIB_SUPPORT),y)
  TARGET_LDFLAGS+= -lts
  LDFLAGS+=-lts
endif

CONFIGURE_ARGS +=

define Package/iphone-like-demo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/local
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/ \
		$(1)/usr/local/
endef

$(eval $(call BuildPackage,iphone-like-demo))
