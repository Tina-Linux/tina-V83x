#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rospkg
PKG_VERSION:=1.1.7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

HOST_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=python $(PKG_NAME)/host

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+python
  TITLE:=rospkg Python library for ROS
  URL:=https://github.com/ros-infrastructure/rospkg/tree/1.1.7
endef

define Package/$(PKG_NAME)/description
Standalone Python library for the ROS package system.
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
	@echo -e "\n************** host do not need to execute install **************\n"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define PyPackage/$(PKG_NAME)/install
	# fix "#! ... tina/out/host/bin/python2.7" \
		to "#!/usr/bin/python2.7"
	sed -i '/\/host\/bin\/python2.7/c\#!/usr/bin/python2.7' \
		$(PKG_INSTALL_DIR)/usr/bin/rosversion

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
