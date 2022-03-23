#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=minigui-res-be
PKG_VERSION:=3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=c3ec3552d9ec35b987839932a5db1fcf

PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=glib2/host libpthread zlib libintl libffi
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/minigui-res-be
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:=
  TITLE:=minigui-res-be
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/minigui-res-be/description
  The minigui res library
endef
#define Build/Prepare
#	$(call Build/Prepare/Default)
#	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
#endef

CONFIGURE_PREFIX:=/usr

CONFIGURE_ARGS = \
	--target=$(GNU_TARGET_NAME) \
	--host=$(GNU_TARGET_NAME) \
	--build=$(GNU_HOST_NAME) \
	--program-prefix="" \
	--program-suffix="" \
	--prefix=$(CONFIGURE_PREFIX) \
	--exec-prefix=$(CONFIGURE_PREFIX) \
	--bindir=$(CONFIGURE_PREFIX)/bin \
	--sbindir=$(CONFIGURE_PREFIX)/sbin \
	--libexecdir=$(CONFIGURE_PREFIX)/lib \
	--sysconfdir=/etc \
	--datadir=$(CONFIGURE_PREFIX)/share/local \
	--localstatedir=/var \
	--mandir=$(CONFIGURE_PREFIX)/man \
	--infodir=$(CONFIGURE_PREFIX)/info \

define Build/Configure
	$(call Build/Configure/Default)
endef

define Package/minigui-res-be/install
	$(INSTALL_DIR) $(1)/usr/share/local/minigui/res/bmp/
	$(INSTALL_DIR) $(1)/usr/share/local/minigui/res/font/
	$(INSTALL_DIR) $(1)/usr/share/local/minigui/res/icon/
	$(INSTALL_DIR) $(1)/usr/share/local/minigui/res/cursor/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/local/minigui/res/bmp/* \
		$(1)/usr/share/local/minigui/res/bmp/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/local/minigui/res/font/* \
		$(1)/usr/share/local/minigui/res/font/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/local/minigui/res/icon/* \
		$(1)/usr/share/local/minigui/res/icon/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/local/minigui/res/cursor/* \
		$(1)/usr/share/local/minigui/res/cursor/
endef

$(eval $(call BuildPackage,minigui-res-be))
