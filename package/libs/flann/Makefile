#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=flann
PKG_VERSION:=1.9.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_INSTALL:=1

CONFIGURE_ARGS += \
        --enable-shared \
        --enable-static \

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS += -DBUILD_MATLAB_BINDINGS=OFF
CMAKE_OPTIONS += -DBUILD_EXAMPLES=OFF
CMAKE_OPTIONS += -DBUILD_TESTS=OFF
CMAKE_OPTIONS += -DBUILD_DOC=OFF

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp
  TITLE:=Fast Library for Approximate Nearest Neighbors
  URL:=https://github.com/mariusmuja/flann/tree/1.9.1
endef

define Package/$(PKG_NAME)/description
FLANN is a library for performing fast approximate nearest neighbor searches in high dimensional spaces.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/share
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
