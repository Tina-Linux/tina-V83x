#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=geneus
PKG_VERSION:=2.2.6

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=BSD

PKG_BUILD_DEPENDS:=catkin

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:= +catkin
  TITLE:= ros geneus
  URL:=http://wiki.ros.org/geneus
endef

define Package/$(PKG_NAME)/description
EusLisp ROS message and service generators.
endef

define Build/Compile
	cd $(PKG_BUILD_DIR) && \
	$(STAGING_DIR)/usr/bin/catkin_make_isolated --install \
		--source=$(PKG_BUILD_DIR) \
		--build=$(PKG_BUILD_DIR)/build_isolated \
		-DCATKIN_DEVEL_PREFIX=$(PKG_BUILD_DIR)/devel \
		-DCMAKE_INSTALL_PREFIX=$(PKG_INSTALL_DIR) \
		-DCMAKE_C_COMPILER="$(TARGET_CC)" \
		-DCMAKE_CXX_COMPILER="$(TARGET_CXX)" \
		-DCMAKE_PREFIX_PATH="$(STAGING_DIR)/usr"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/usr/etc
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/usr/etc
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
