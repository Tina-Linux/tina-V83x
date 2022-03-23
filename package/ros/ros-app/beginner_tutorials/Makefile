#
# Copyright (C) 2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=beginner_tutorials
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-app
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:= +ros_comm
  TITLE:=ros beginner tutorials
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

beginner_tutorials_MSGFILES:= \
	$(PKG_BUILD_DIR)/msg/Num.msg

beginner_tutorials_SRVFILES:= \
	$(PKG_BUILD_DIR)/srv/AddTwoInts.srv

define Build/Compile
	$(info ************** Converts beginner_tutorials .msg files in a package into source code **************)
	$(foreach pmsg,$(beginner_tutorials_MSGFILES),$(call Build/Compile/MsgToPy,,beginner_tutorials,msg, \
			beginner_tutorials:$(PKG_BUILD_DIR)/msg -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg,$(pmsg)))

	$(foreach pmsg,$(beginner_tutorials_MSGFILES),$(call Build/Compile/MsgToPy,,beginner_tutorials,msg, \
			beginner_tutorials:$(PKG_BUILD_DIR)/msg -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg, --initpy True $(pmsg)))

	$(foreach pmsg,$(beginner_tutorials_MSGFILES),$(call Build/Compile/MsgToPy,,beginner_tutorials,, \
			beginner_tutorials:$(PKG_BUILD_DIR)/msg -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg, --initpy True $(pmsg)))

	$(foreach pmsg,$(beginner_tutorials_MSGFILES),$(call Build/Compile/MsgSrvToSTAGING_H,,beginner_tutorials,/usr/include/beginner_tutorials, \
			beginner_tutorials:$(PKG_BUILD_DIR)/msg -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg, $(pmsg),,))

	$(info ************** Converts beginner_tutorials .srv files into Python source code **************)
	$(foreach psrv,$(beginner_tutorials_SRVFILES),$(call Build/Compile/SrvToPy,,beginner_tutorials,srv, \
			beginner_tutorials:$(PKG_BUILD_DIR)/srv -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg,$(psrv)))

	$(foreach psrv,$(beginner_tutorials_SRVFILES),$(call Build/Compile/SrvToPy,,beginner_tutorials,srv, \
			beginner_tutorials:$(PKG_BUILD_DIR)/srv -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg, --initpy True $(psrv)))

	$(foreach psrv,$(beginner_tutorials_SRVFILES),$(call Build/Compile/SrvToPy,,beginner_tutorials,, \
			beginner_tutorials:$(PKG_BUILD_DIR)/srv -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg, --initpy True $(psrv)))

	$(foreach psrv,$(beginner_tutorials_SRVFILES),$(call Build/Compile/MsgSrvToSTAGING_H,,beginner_tutorials,/usr/include/beginner_tutorials, \
			beginner_tutorials:$(PKG_BUILD_DIR)/srv -I std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg, $(psrv),,))

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
		-DCMAKE_PREFIX_PATH="$(STAGING_DIR)/usr;$(PKG_INSTALL_DIR)"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

# install executable file
	$(INSTALL_DIR) $(1)/usr/lib/$(PKG_NAME)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/devel/$(PKG_NAME)/lib/$(PKG_NAME)/* $(1)/usr/lib/$(PKG_NAME)

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
