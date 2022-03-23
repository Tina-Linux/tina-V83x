#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=geometry2
PKG_VERSION:=indigo

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-devel.zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)-devel

PKG_BUILD_DEPENDS:= catkin bullet actionlib

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-3rdparty
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+bullet +actionlib
  TITLE:=ros geometry2
  URL:=http://wiki.ros.org/geometry2?distro=kinetic
endef

define Package/$(PKG_NAME)/description
A metapackage to bring in the default packages second generation Transform Library in ros, tf2.
endef

tf2_msgs_MSGFILES:= \
	$(PKG_BUILD_DIR)/files/tf2_msgs/msg/LookupTransformActionFeedback.msg \
	$(PKG_BUILD_DIR)/files/tf2_msgs/msg/LookupTransformActionGoal.msg \
	$(PKG_BUILD_DIR)/files/tf2_msgs/msg/LookupTransformAction.msg \
	$(PKG_BUILD_DIR)/files/tf2_msgs/msg/LookupTransformActionResult.msg \
	$(PKG_BUILD_DIR)/files/tf2_msgs/msg/LookupTransformFeedback.msg \
	$(PKG_BUILD_DIR)/files/tf2_msgs/msg/LookupTransformGoal.msg \
	$(PKG_BUILD_DIR)/files/tf2_msgs/msg/LookupTransformResult.msg \
	$(PKG_BUILD_DIR)/files/tf2_msgs/msg/TF2Error.msg \
	$(PKG_BUILD_DIR)/files/tf2_msgs/msg/TFMessage.msg

tf2_srvs_SRVFILES:= \
	$(PKG_BUILD_DIR)/tf2_msgs/srv/FrameGraph.srv

define Build/Compile
	mkdir -p $(PKG_BUILD_DIR)/files;
	$(CP) ./files/* $(PKG_BUILD_DIR)/files;
	$(info ************** Converts ROS .msg files in a package into C++ source code **************)
	$(foreach pmsg,$(tf2_msgs_MSGFILES),$(call Build/Compile/MsgSrvToH,,tf2_msgs,include/tf2_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I tf2_msgs:$(PKG_BUILD_DIR)/files/tf2_msgs/msg \
			-I geometry_msgs:$(STAGING_DIR)/usr/share/geometry_msgs/msg \
			-I actionlib_msgs:$(STAGING_DIR)/usr/share/actionlib_msgs/msg, $(pmsg),,))

	$(info ************** Converts ROS .srv files in a package into C++ source code **************)
	$(foreach psrv,$(tf2_srvs_SRVFILES),$(call Build/Compile/MsgSrvToH,,tf2_msgs,include/tf2_msgs, \
			std_srvs:$(PKG_BUILD_DIR)/std_srvs/srv -I tf2_msgs:$(PKG_BUILD_DIR)/tf2_msgs/srv \
			-I geometry_msgs:$(STAGING_DIR)/usr/share/geometry_msgs/msg, $(psrv),,))
	$(foreach psrv,$(tf2_srvs_SRVFILES),$(call Build/Compile/MsgSrvToH,,tf2_srvs,include/tf2_srvs, \
			std_srvs:$(PKG_BUILD_DIR)/std_srvs/srv -I tf2_msgs:$(PKG_BUILD_DIR)/tf2_msgs/srv \
			-I geometry_msgs:$(STAGING_DIR)/usr/share/geometry_msgs/msg, $(psrv),,))

	$(info ************** Converts ROS .msg files in a package into C++ source code **************)
	$(foreach pmsg,$(tf2_msgs_MSGFILES),$(call Build/Compile/MsgSrvToSTAGING_H,,tf2_msgs,usr/include/tf2_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I tf2_msgs:$(PKG_BUILD_DIR)/files/tf2_msgs/msg \
			-I geometry_msgs:$(STAGING_DIR)/usr/share/geometry_msgs/msg \
			-I actionlib_msgs:$(STAGING_DIR)/usr/share/actionlib_msgs/msg, $(pmsg),,))

	$(info ************** Converts ROS .srv files in a package into C++ source code **************)
	$(foreach psrv,$(tf2_srvs_SRVFILES),$(call Build/Compile/MsgSrvToSTAGING_H,,tf2_srvs,usr/include/tf2_srvs, \
			std_srvs:$(PKG_BUILD_DIR)/std_srvs/srv -I tf2_msgs:$(PKG_BUILD_DIR)/tf2_msgs/srv \
			-I geometry_msgs:$(STAGING_DIR)/usr/share/geometry_msgs/msg, $(psrv),,))

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
		-DCMAKE_PREFIX_PATH="$(STAGING_DIR)/usr;$(PKG_INSTALL_DIR)" \
		-Dbullet_INCLUDE_DIRS="$(STAGING_DIR)"
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
