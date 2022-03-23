#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libmdolphin
PKG_VERSION:=3.0.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libc-ares +libcurl +libxml2 +libxslt +libopenssl +libstdcpp
  TITLE:=mDolphin-core
  MAINTAINER:=xielinfei <xielinfei@allwinnertech.com>
  URL:=http://www.minigui.org/
endef

define Package/${PKG_NAME}/description
  The mDolphin Core Lib
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef
TARGET_CXXFLAGS+=-std=gnu++98 -fpermissive

CONFIGURE_ARGS += \
	--enable-unicodesupport \
	--enable-ttfsupport

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mdolphin \
		$(1)/usr/include/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/* \
		$(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
	$(PKG_INSTALL_DIR)/usr/lib/* \
	$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libmdolphin))
