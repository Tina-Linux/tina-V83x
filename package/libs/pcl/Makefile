#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pcl
PKG_VERSION:=1.9.1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_NAME)-$(PKG_VERSION)
PKG_LICENSE:=BSD

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS += -DWITH_VTK=OFF
CMAKE_OPTIONS += -DWITH_OPENGL=OFF
TARGET_CFLAGS += -DMALLOC_ALIGNED=1

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp +zlib +libpng +eigen +flann +qhull +boost +boost-date_time +boost-filesystem +boost-iostreams +boost-system +boost-thread
  TITLE:=Point Cloud Library (PCL)
  URL:=https://github.com/PointCloudLibrary/pcl/tree/pcl-1.9.1
endef

define Package/$(PKG_NAME)/description
The Point Cloud Library (PCL) is a standalone, large scale, open project for 2D/3D image and point cloud processing.
endef

define Build/Compile
	$(MAKE) -j64 -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		FC=$(TARGET_FORTRAN) \
		HOSTCC=$(HOSTCC) \
		PREFIX=$(PKG_INSTALL_DIR) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
