#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmgncs
PKG_VERSION:=1.2.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=8ab1080ac2b05a2513fa146234ff4d0f

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/libmgncs
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libmgutils +libsqlite3 +libxml2 +libstdcpp +libmgplus
  TITLE:=libmgncs
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/libmgncs/description
  mGNCS provides a new control set for MiniGUI
  application. By using mGNCS, you can use miniStudio
  to develop MiniGUI application in WYSIWYG way.
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef

LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ make V=s

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

define Build/Compile

endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgncs \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgncs*.{a,la,so*} \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mgncs.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/libmgncs/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/etc/* \
		$(1)/usr/local/etc/
endef

$(eval $(call BuildPackage,libmgncs))
