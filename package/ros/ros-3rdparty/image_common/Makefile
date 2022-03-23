#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=image_common
PKG_VERSION:=hydro

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-devel.zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)-devel

PKG_BUILD_DEPENDS:= yaml-cpp

include $(BUILD_DIR)/package.mk
$(call include_mk, genpy-msgsrv.mk)
$(call include_mk, gencpp-msgsrv.mk)

define Package/$(PKG_NAME)
  SUBMENU:=ros-3rdparty
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+yaml-cpp +boost-python +pluginlib
  TITLE:=ros image_common
  URL:=http://wiki.ros.org/image_common?distro=kinetic
endef

define Package/$(PKG_NAME)/description
Common code for working with images in ROS.
endef

CMAKE_OPTIONS += -DCMAKE_FIND_ROOT_PATH="$(STAGING_DIR)/usr;$(TOOLCHAIN_DIR)"

polled_camera_srvs_MSGFILES:= \
	$(PKG_BUILD_DIR)/polled_camera/srv/GetPolledImage.srv

define Build/Compile
	$(info ************** Converts ROS .msg files in a package into C++ source code **************)
	$(foreach psrv,$(polled_camera_srvs_MSGFILES),$(call Build/Compile/MsgSrvToH,,polled_camera,include/polled_camera, \
			std_msgs:$(STAGING_DIR)/usr/share/std_msgs/msg -I sensor_msgs:$(STAGING_DIR)/usr/share/sensor_msgs/msg, $(psrv),,))

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
		--cmake-args $(CMAKE_OPTIONS)
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
