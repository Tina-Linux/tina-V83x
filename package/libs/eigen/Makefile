#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=eigen
PKG_VERSION:=3.3.7

PKG_SOURCE:=$(PKG_NAME)-$(PKG_NAME)-323c052e1731.tar.bz2
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_NAME)-323c052e1731
PKG_LICENSE:=LGPL3+

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp
  TITLE:=eigen libs
  URL:=http://eigen.tuxfamily.org/index.php?title=Main_Page
endef

define Package/$(PKG_NAME)/description
Eigen is a C++ template library for linear algebra: matrices, vectors, numerical solvers, and related algorithms.
endef

CMAKE_SOURCE_DIR:=$(PKG_BUILD_DIR)/devel

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/devel
	tar -xjf $(DL_DIR)/$(PKG_SOURCE) -C $(PKG_BUILD_DIR)/devel
	$(CP) $(PKG_BUILD_DIR)/devel/$(PKG_NAME)-$(PKG_NAME)-323c052e1731/* $(PKG_BUILD_DIR)/devel
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
