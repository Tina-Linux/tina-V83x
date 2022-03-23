#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ros_comm-msgs
PKG_VERSION:=indigo

SOURCE_NAME:=ros_comm_msgs
PKG_SOURCE:=$(SOURCE_NAME)-$(PKG_VERSION)-devel.zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=BSD

PKG_BUILD_DEPENDS:=catkin genpy gencpp message_generation std_msgs message_runtime roscpp_core
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(SOURCE_NAME)-$(PKG_VERSION)-devel

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+message_generation +gencpp +std_msgs +message_runtime +roscpp_core
  TITLE:= ros_comm_msgs
  URL:=https://github.com/ros/ros_comm_msgs
endef

define Package/$(PKG_NAME)/description
Messages relating to the ROS Computation Graph. These are generally considered to be low-level messages that end users do not interact with.
endef

rosgraph_msgs_MSGFILES:= \
	$(PKG_BUILD_DIR)/rosgraph_msgs/msg/Clock.msg \
	$(PKG_BUILD_DIR)/rosgraph_msgs/msg/TopicStatistics.msg \
	$(PKG_BUILD_DIR)/rosgraph_msgs/msg/Log.msg

std_srvs_SRVFILES:= \
	$(PKG_BUILD_DIR)/std_srvs/srv/Empty.srv \
	$(PKG_BUILD_DIR)/std_srvs/srv/SetBool.srv \
	$(PKG_BUILD_DIR)/std_srvs/srv/Trigger.srv

define Build/Compile
	@echo -e "\n************** Converts ROS .msg files in a package into Python source code **************\n"
	$(foreach pmsg,$(rosgraph_msgs_MSGFILES),$(call Build/Compile/MsgToPy,,rosgraph_msgs,msg, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I rosgraph_msgs:$(PKG_BUILD_DIR)/rosgraph_msgs/msg,$(pmsg)))

	@echo -e "\n************** Converts ROS .msg files in a package into Python source code initpy **************\n"
	$(foreach pmsg,$(rosgraph_msgs_MSGFILES),$(call Build/Compile/MsgToPy,,rosgraph_msgs,msg, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I rosgraph_msgs:$(PKG_BUILD_DIR)/rosgraph_msgs/msg, --initpy True $(pmsg)))
	$(foreach pmsg,$(rosgraph_msgs_MSGFILES),$(call Build/Compile/MsgToPy,,rosgraph_msgs,, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I rosgraph_msgs:$(PKG_BUILD_DIR)/rosgraph_msgs/msg, --initpy True $(pmsg)))

	@echo -e "\n************** Converts ROS .msg files in a package into C++ source code **************\n"
	$(foreach pmsg,$(rosgraph_msgs_MSGFILES),$(call Build/Compile/MsgSrvToH,,rosgraph_msgs,include/rosgraph_msgs, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I rosgraph_msgs:$(PKG_BUILD_DIR)/rosgraph_msgs/msg, $(pmsg),,))

	@echo -e "\n************** Converts ROS .srv files into Python source code **************\n"
	$(foreach psrv,$(std_srvs_SRVFILES),$(call Build/Compile/SrvToPy,,std_srvs,srv, \
			std_srvs:$(PKG_BUILD_DIR)/std_srvs/srv,$(psrv)))

	@echo -e "\n************** Converts ROS .srv files in a package into Python source code initpy **************\n"
	$(foreach psrv,$(std_srvs_SRVFILES),$(call Build/Compile/SrvToPy,,std_srvs,srv, \
			std_srvs:$(PKG_BUILD_DIR)/std_srvs/srv, --initpy True $(psrv)))
	$(foreach psrv,$(std_srvs_SRVFILES),$(call Build/Compile/SrvToPy,,std_srvs,, \
			std_srvs:$(PKG_BUILD_DIR)/std_srvs/srv, --initpy True $(psrv)))

	@echo -e "\n************** Converts ROS .msg files in a package into C++ source code **************\n"
	$(foreach psrv,$(std_srvs_SRVFILES),$(call Build/Compile/MsgSrvToH,,std_srvs,include/std_srvs, \
			std_srvs:$(PKG_BUILD_DIR)/std_srvs/srv, $(psrv),,))

	cd $(PKG_BUILD_DIR) && \
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(STAGING_DIR)/usr/bin/catkin_make_isolated --install \
		--source=$(PKG_BUILD_DIR) \
		--build=$(PKG_BUILD_DIR)/build_isolated \
		-DCATKIN_DEVEL_PREFIX="$(PKG_BUILD_DIR)/devel" \
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
