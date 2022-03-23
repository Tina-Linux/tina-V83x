#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=catkin
PKG_VERSION:=kinetic

PKG_SOURCE:=catkin-$(PKG_VERSION)-devel.zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DEPENDS:=python/host python-empy/host catkin_pkg/host python-nose/host $(PKG_NAME)/host
HOST_BUILD_DEPENDS:=python/host python-empy/host catkin_pkg/host python-nose/host python-pyparsing/host

PKG_BUILD_DIR:=$(COMPILE_DIR)/catkin-$(PKG_VERSION)-devel
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/catkin-$(PKG_VERSION)-devel

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+python +catkin_pkg +python-empy +python-nose +python-pyparsing
  TITLE:=catkin kinetic devel
  URL:=https://github.com/ros/catkin
endef

define Package/$(PKG_NAME)/description
A CMake-based build system that is used to build all packages in ROS.
endef

define Host/Compile
	$(call Build/Compile/HostPyMod,,\
		install --root="/" --prefix="$(OUT_DIR)/host" \
	)
endef

define Host/Install
	$(INSTALL_DIR) $(OUT_DIR)/host/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/devel/bin/* $(OUT_DIR)/host/bin/

	$(INSTALL_DIR) $(OUT_DIR)/host/lib/
	$(CP) $(HOST_BUILD_DIR)/devel/lib/* $(OUT_DIR)/host/lib/

	$(INSTALL_DIR) $(OUT_DIR)/host/lib/share/$(PKG_NAME)
	$(CP) $(HOST_BUILD_DIR)/cmake $(OUT_DIR)/host/lib/share/$(PKG_NAME)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/etc
	$(CP) $(PKG_INSTALL_DIR)/usr/etc/* $(1)/usr/etc
endef

define PyPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/etc
	$(CP) $(PKG_INSTALL_DIR)/usr/etc/* $(1)/usr/etc

#	get workspaces need .catkin
	$(CP) $(PKG_INSTALL_DIR)/usr/.catkin $(1)/usr
	$(CP) $(PKG_INSTALL_DIR)/usr/.rosinstall $(1)/usr
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
