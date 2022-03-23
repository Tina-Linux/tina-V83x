#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=catkin_pkg
PKG_VERSION:=0.4.3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

HOST_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=python python-pyparsing $(PKG_NAME)/host
HOST_BUILD_DEPENDS:=python/host python-pyparsing/host

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+python +python-pyparsing +python-dateutil +python-docutils
  TITLE:=catkin pkg
  URL:=https://github.com/ros-infrastructure/catkin_pkg/tree/0.4.3
endef

define Package/$(PKG_NAME)/description
  Standalone Python library for the catkin build system.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,\
		install --root="$(PKG_INSTALL_DIR)" --prefix="/usr" \
	)
endef

define Host/Compile
	$(call Build/Compile/HostPyMod,,\
		install --root="/" --prefix="$(OUT_DIR)/host" \
	)
endef

define Host/Install
	$(INSTALL_DIR) $(OUT_DIR)/host/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/* $(OUT_DIR)/host/bin/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

define PyPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/python$(PYTHON_VERSION)
	$(CP) ./files/python_info/* $(1)/usr/lib/python$(PYTHON_VERSION)

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
