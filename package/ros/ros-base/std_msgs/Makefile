#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=std_msgs
PKG_VERSION:=groovy

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-devel.zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=BSD

PKG_BUILD_DEPENDS:=python-pyyaml catkin message_generation genpy gencpp geneus gennodejs genlisp

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)-devel

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+python-pyyaml +message_generation +gencpp +geneus +gennodejs +genlisp +genpy
  TITLE:=std msgs
  URL:=http://wiki.ros.org/std_msgs?distro=kinetic
endef

define Package/$(PKG_NAME)/description
std_msgs contains wrappers for ROS primitive types, which are documented in the msg specification.
endef

#MSGFILES:=$(shell find $(PKG_BUILD_DIR)/msg -name ".msg")

MSGFILES:= \
	$(PKG_BUILD_DIR)/msg/Bool.msg \
	$(PKG_BUILD_DIR)/msg/Byte.msg \
	$(PKG_BUILD_DIR)/msg/Char.msg \
	$(PKG_BUILD_DIR)/msg/ColorRGBA.msg \
	$(PKG_BUILD_DIR)/msg/Duration.msg \
	$(PKG_BUILD_DIR)/msg/Empty.msg \
	$(PKG_BUILD_DIR)/msg/Float32.msg \
	$(PKG_BUILD_DIR)/msg/Float64.msg \
	$(PKG_BUILD_DIR)/msg/Header.msg \
	$(PKG_BUILD_DIR)/msg/Int8.msg \
	$(PKG_BUILD_DIR)/msg/Int16.msg \
	$(PKG_BUILD_DIR)/msg/Int32.msg \
	$(PKG_BUILD_DIR)/msg/Int64.msg \
	$(PKG_BUILD_DIR)/msg/String.msg \
	$(PKG_BUILD_DIR)/msg/Time.msg \
	$(PKG_BUILD_DIR)/msg/UInt8.msg \
	$(PKG_BUILD_DIR)/msg/UInt16.msg \
	$(PKG_BUILD_DIR)/msg/UInt32.msg \
	$(PKG_BUILD_DIR)/msg/UInt64.msg \
	$(PKG_BUILD_DIR)/msg/MultiArrayDimension.msg \
	$(PKG_BUILD_DIR)/msg/MultiArrayLayout.msg \
	$(PKG_BUILD_DIR)/msg/ByteMultiArray.msg \
	$(PKG_BUILD_DIR)/msg/Float32MultiArray.msg \
	$(PKG_BUILD_DIR)/msg/Float64MultiArray.msg \
	$(PKG_BUILD_DIR)/msg/Int8MultiArray.msg \
	$(PKG_BUILD_DIR)/msg/Int16MultiArray.msg \
	$(PKG_BUILD_DIR)/msg/Int32MultiArray.msg \
	$(PKG_BUILD_DIR)/msg/Int64MultiArray.msg \
	$(PKG_BUILD_DIR)/msg/UInt8MultiArray.msg \
	$(PKG_BUILD_DIR)/msg/UInt16MultiArray.msg \
	$(PKG_BUILD_DIR)/msg/UInt32MultiArray.msg \
	$(PKG_BUILD_DIR)/msg/UInt64MultiArray.msg

define Build/Compile
	@echo -e "\n************** Converts ROS .msg files in a package into Python source code **************\n"
	$(foreach msg,$(MSGFILES),$(call Build/Compile/MsgToPy,,std_msgs,msg,std_msgs:$(PKG_BUILD_DIR)/msg,$(msg)))

	@echo -e "\n************** Converts ROS .msg files in a package into Python source code initpy **************\n"
	$(foreach msg,$(MSGFILES),$(call Build/Compile/MsgToPy,,std_msgs,msg,std_msgs:$(PKG_BUILD_DIR)/msg, --initpy True $(msg)))
	$(foreach msg,$(MSGFILES),$(call Build/Compile/MsgToPy,,std_msgs,,std_msgs:$(PKG_BUILD_DIR)/msg, --initpy True $(msg)))

	@echo -e "\n************** Converts ROS .msg files in a package into C++ source code **************\n"
	$(foreach msg,$(MSGFILES),$(call Build/Compile/MsgSrvToH,,std_msgs,include/std_msgs, \
						std_msgs:$(PKG_BUILD_DIR)/msg, $(msg),,))

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
