#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ros_comm
PKG_VERSION:=kinetic

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-devel.zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)-devel

PKG_BUILD_DEPENDS:=lz4 boost-regex boost-signals genpy roscpp_core ros-kinetic-devel std_msgs message_runtime ros_comm-msgs

CONFIGURE_ARGS += \
        --enable-shared \
        --enable-static \

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:= +boost +boost-regex +boost-signals +python-defusedxml +python-netifaces +lz4 +glog +roscpp_core +ros-kinetic-devel +std_msgs +message_runtime +ros_comm-msgs +rospkg
  TITLE:=ROS communications-related packages
  URL:=https://github.com/ros/ros/tree/kinetic-devel
endef

define Package/$(PKG_NAME)/description
ROS communications-related packages, including core client libraries (roscpp, rospy, roslisp) and graph introspection tools (rostopic, rosnode, rosservice, rosparam).
endef

roscpp_MSGFILES:= \
	$(PKG_BUILD_DIR)/clients/roscpp/msg/Logger.msg

roscpp_SRVFILES:= \
	$(PKG_BUILD_DIR)/clients/roscpp/srv/Empty.srv \
	$(PKG_BUILD_DIR)/clients/roscpp/srv/SetLoggerLevel.srv \
	$(PKG_BUILD_DIR)/clients/roscpp/srv/GetLoggers.srv

topic_tools_SRVFILES:= \
	$(PKG_BUILD_DIR)/tools/topic_tools/srv/DemuxAdd.srv \
	$(PKG_BUILD_DIR)/tools/topic_tools/srv/DemuxDelete.srv \
	$(PKG_BUILD_DIR)/tools/topic_tools/srv/DemuxList.srv \
	$(PKG_BUILD_DIR)/tools/topic_tools/srv/DemuxSelect.srv \
	$(PKG_BUILD_DIR)/tools/topic_tools/srv/MuxAdd.srv \
	$(PKG_BUILD_DIR)/tools/topic_tools/srv/MuxDelete.srv \
	$(PKG_BUILD_DIR)/tools/topic_tools/srv/MuxList.srv \
	$(PKG_BUILD_DIR)/tools/topic_tools/srv/MuxSelect.srv

define Build/Compile
	@echo -e "\n************** Converts roscpp .msg files in a package into source code **************\n"
	$(foreach pmsg,$(roscpp_MSGFILES),$(call Build/Compile/MsgToPy,,roscpp,msg, \
			roscpp:$(PKG_BUILD_DIR)/clients/roscpp/msg,$(pmsg)))

	$(foreach pmsg,$(roscpp_MSGFILES),$(call Build/Compile/MsgToPy,,roscpp,msg, \
			roscpp:$(PKG_BUILD_DIR)/clients/roscpp/msg, --initpy True $(pmsg)))

	$(foreach pmsg,$(roscpp_MSGFILES),$(call Build/Compile/MsgToPy,,roscpp,, \
			roscpp:$(PKG_BUILD_DIR)/clients/roscpp/msg, --initpy True $(pmsg)))

	$(foreach pmsg,$(roscpp_MSGFILES),$(call Build/Compile/MsgSrvToSTAGING_H,,roscpp,/usr/include/roscpp, \
			roscpp:$(PKG_BUILD_DIR)/clients/roscpp/msg, $(pmsg),,))

	@echo -e "\n************** Converts roscpp .srv files into Python source code **************\n"
	$(foreach psrv,$(roscpp_SRVFILES),$(call Build/Compile/SrvToPy,,roscpp,srv, \
			roscpp:$(PKG_BUILD_DIR)/clients/roscpp/srv -I roscpp:$(PKG_BUILD_DIR)/clients/roscpp/msg,$(psrv)))

	$(foreach psrv,$(roscpp_SRVFILES),$(call Build/Compile/SrvToPy,,roscpp,srv, \
			roscpp:$(PKG_BUILD_DIR)/clients/roscpp/srv -I roscpp:$(PKG_BUILD_DIR)/clients/roscpp/msg, --initpy True $(psrv)))

	$(foreach psrv,$(roscpp_SRVFILES),$(call Build/Compile/SrvToPy,,roscpp,, \
			roscpp:$(PKG_BUILD_DIR)/clients/roscpp/srv -I roscpp:$(PKG_BUILD_DIR)/clients/roscpp/msg, --initpy True $(psrv)))

	$(foreach psrv,$(roscpp_SRVFILES),$(call Build/Compile/MsgSrvToSTAGING_H,,roscpp,/usr/include/roscpp, \
			roscpp:$(PKG_BUILD_DIR)/clients/roscpp/srv -I roscpp:$(PKG_BUILD_DIR)/clients/roscpp/msg, $(psrv),,))

	@echo -e "\n************** Converts topic_tools .srv files into Python source code **************\n"
	$(foreach psrv,$(topic_tools_SRVFILES),$(call Build/Compile/SrvToPy,,topic_tools,srv, \
			topic_tools:$(PKG_BUILD_DIR)/tools/topic_tools/srv -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg,$(psrv)))

	$(foreach psrv,$(topic_tools_SRVFILES),$(call Build/Compile/SrvToPy,,topic_tools,srv, \
			topic_tools:$(PKG_BUILD_DIR)/tools/topic_tools/srv -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg, --initpy True $(psrv)))

	$(foreach psrv,$(topic_tools_SRVFILES),$(call Build/Compile/SrvToPy,,topic_tools,, \
			topic_tools:$(PKG_BUILD_DIR)/tools/topic_tools/srv -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg, --initpy True $(psrv)))

	$(foreach psrv,$(topic_tools_SRVFILES),$(call Build/Compile/MsgSrvToSTAGING_H,,topic_tools,/usr/include/topic_tools, \
			topic_tools:$(PKG_BUILD_DIR)/tools/topic_tools/srv  -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg, $(psrv),,))

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

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/usr/etc
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/usr/etc

#	add setup env
	$(INSTALL_DIR) $(1)/etc/profile.d
	$(INSTALL_BIN) ./files/rossetupenv.sh $(1)/etc/profile.d
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
