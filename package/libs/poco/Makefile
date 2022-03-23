#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=poco
PKG_VERSION:=1.9.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION)-release.zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION)-release
PKG_LICENSE:=BSL-1.0

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp +libopenssl
  TITLE:=poco C++ libs
  URL:=https://github.com/pocoproject/poco/tree/poco-1.9.0-release
endef

define Package/$(PKG_NAME)/description
The POCO C++ Libraries are powerful cross-platform C++ libraries for building network- and internet-based applications that run on desktop, server, mobile, IoT, and embedded systems.
endef

CMAKE_SOURCE_DIR:=$(PKG_BUILD_DIR)/devel

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/devel
	unzip $(DL_DIR)/$(PKG_SOURCE) -d $(PKG_BUILD_DIR)/devel
	$(CP) $(PKG_BUILD_DIR)/devel/$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION)-release/* $(PKG_BUILD_DIR)/devel
	patch -p1 -d $(PKG_BUILD_DIR)/devel/ < ./patches/*.patch
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
