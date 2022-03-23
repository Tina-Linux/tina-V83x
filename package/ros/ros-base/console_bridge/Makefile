#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=console_bridge
PKG_VERSION:=0.4.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL:=1

CONFIGURE_ARGS += \
        --enable-shared \
        --enable-static \

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+libstdcpp
  TITLE:=ros console bridge
  URL:=https://github.com/ros/console_bridge/tree/0.4.2
endef

define Package/$(PKG_NAME)/description
A ROS-independent package for logging that seamlessly pipes into rosconsole/rosout for ROS-dependent packages.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/$(PKG_NAME) $(1)/usr/lib/$(PKG_NAME)

	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc $(1)/usr/lib/pkgconfig/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
