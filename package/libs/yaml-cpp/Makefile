#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=yaml-cpp
PKG_VERSION:=0.6.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION)
PKG_LICENSE:=MIT

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
#  DEPENDS:=+libstdcpp
  TITLE:=yaml-cpp libs
  URL:=https://github.com/jbeder/yaml-cpp/tree/yaml-cpp-0.6.2
endef

define Package/$(PKG_NAME)/description
A YAML parser and emitter in C++
endef

CMAKE_SOURCE_DIR:=$(PKG_BUILD_DIR)/devel

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/devel
	unzip $(DL_DIR)/$(PKG_SOURCE) -d $(PKG_BUILD_DIR)/devel
	$(CP) $(PKG_BUILD_DIR)/devel/$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION)/* $(PKG_BUILD_DIR)/devel
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
