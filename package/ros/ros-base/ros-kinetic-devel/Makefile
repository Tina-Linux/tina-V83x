#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ros-kinetic-devel
PKG_VERSION:=kinetic

PKG_SOURCE:=$(PKG_NAME).zip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

PKG_BUILD_DEPENDS:=catkin rospack

CONFIGURE_ARGS += \
        --enable-shared \
        --enable-static \

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=ros-base
  SECTION:=ROS
  CATEGORY:=ROS
  DEPENDS:=+libstdcpp +python +boost-filesystem +boost-program_options +boost-date_time +boost-atomic +boost-chrono +boost-thread +libtinyxml +rospack +bash
  TITLE:=ros kinetic devel
  URL:=https://github.com/ros/ros/tree/kinetic-devel
endef

define Package/$(PKG_NAME)/description
ROS is a meta-operating system for your robot.  It provides
language-independent and network-transparent communication for a
distributed robot control system.
endef

define Build/Compile
	cd $(PKG_BUILD_DIR) && \
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	$(STAGING_DIR)/usr/bin/catkin_make_isolated --install \
		--source=$(PKG_BUILD_DIR) \
		--build=$(PKG_BUILD_DIR)/build_isolated \
		-DCATKIN_DEVEL_PREFIX=$(PKG_INSTALL_DIR) \
		-DCMAKE_INSTALL_PREFIX=$(PKG_INSTALL_DIR) \
		-DCMAKE_C_COMPILER="$(TARGET_CC)" \
		-DCMAKE_CXX_COMPILER="$(TARGET_CXX)" \
		-DCMAKE_PREFIX_PATH=$(STAGING_DIR)/usr/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/usr/etc
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/ros/* $(1)/usr

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/etc
	$(CP) $(PKG_INSTALL_DIR)/etc/* $(1)/usr/etc
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
