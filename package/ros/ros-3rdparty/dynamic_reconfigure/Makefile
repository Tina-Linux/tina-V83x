#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=dynamic_reconfigure
PKG_VERSION:=master

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_BUILD_DEPENDS:= catkin

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-3rdparty
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:= +boost-chrono +boost-thread
  TITLE:=ros dynamic_reconfigure
  URL:=http://wiki.ros.org/dynamic_reconfigure
endef

define Package/$(PKG_NAME)/description
This unary stack contains the dynamic_reconfigure package which provides a means to change node parameters at any time without having to restart the node.
endef

dynamic_reconfigure_MSGSRVFILES:= \
	$(PKG_BUILD_DIR)/msg/BoolParameter.msg \
	$(PKG_BUILD_DIR)/msg/ConfigDescription.msg \
	$(PKG_BUILD_DIR)/msg/Config.msg \
	$(PKG_BUILD_DIR)/msg/DoubleParameter.msg \
	$(PKG_BUILD_DIR)/msg/Group.msg \
	$(PKG_BUILD_DIR)/msg/GroupState.msg \
	$(PKG_BUILD_DIR)/msg/IntParameter.msg \
	$(PKG_BUILD_DIR)/msg/ParamDescription.msg \
	$(PKG_BUILD_DIR)/msg/SensorLevels.msg \
	$(PKG_BUILD_DIR)/msg/StrParameter.msg \
	$(PKG_BUILD_DIR)/srv/Reconfigure.srv

define Build/Compile
	$(info ************** Converts ROS .msg files in a package into C++ source code **************)
	$(foreach pmsg,$(dynamic_reconfigure_MSGSRVFILES),$(call Build/Compile/MsgSrvToH,,dynamic_reconfigure,include/dynamic_reconfigure, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I dynamic_reconfigure:$(PKG_BUILD_DIR)/msg \
			-I dynamic_reconfigure:$(PKG_BUILD_DIR)/srv, $(pmsg),,))

	cd $(PKG_BUILD_DIR) && \
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(STAGING_DIR)/usr/bin/catkin_make_isolated --install \
		--source=$(PKG_BUILD_DIR) \
		--build=$(PKG_BUILD_DIR)/build_isolated \
		-DCATKIN_DEVEL_PREFIX=$(PKG_BUILD_DIR)/devel \
		-DCATKIN_ENABLE_TESTING=OFF \
		-DCMAKE_INSTALL_PREFIX=$(PKG_INSTALL_DIR) \
		-DCMAKE_C_COMPILER="$(TARGET_CC)" \
		-DCMAKE_CXX_COMPILER="$(TARGET_CXX)" \
		-DCMAKE_PREFIX_PATH="$(STAGING_DIR)/usr;$(PKG_INSTALL_DIR)"
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
