#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=orocos_kdl
PKG_VERSION:=1.4.0

PKG_SOURCE:=orocos_kinematics_dynamics-$(PKG_VERSION).zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=LGPL

PKG_BUILD_DIR:=$(COMPILE_DIR)/orocos_kinematics_dynamics-$(PKG_VERSION)

PKG_BUILD_DEPENDS:= eigen python-sip

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=ros-3rdparty
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+eigen +python-sip
  TITLE:=ros orocos_kdl
  URL:=http://wiki.ros.org/orocos_kdl
endef

define Package/$(PKG_NAME)/description
Orocos Kinematics and Dynamics C++ library
endef

define Build/Compile
	cd $(PKG_BUILD_DIR) && \
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(STAGING_DIR)/usr/bin/catkin_make_isolated --install \
		--source=$(PKG_BUILD_DIR) \
		--build=$(PKG_BUILD_DIR)/build_isolated \
		-DCATKIN_DEVEL_PREFIX=$(PKG_BUILD_DIR)/devel \
		-DCMAKE_INSTALL_PREFIX=$(PKG_INSTALL_DIR) \
		-DCMAKE_C_COMPILER="$(TARGET_CC)" \
		-DCMAKE_CXX_COMPILER="$(TARGET_CXX)" \
		-DCMAKE_PREFIX_PATH="$(STAGING_DIR)/usr;$(PKG_INSTALL_DIR)" \
		-DHOST_SIP_PATH="$(OUT_DIR)/host"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
