#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=newfs_msdos
PKG_RELEASE:=1
PKG_VERSION:=1.0

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=Filesystem
  TITLE:=The newfs_msdos is a fat filesystem format tool.
endef

define Package/$(PKG_NAME)/description
  Newfs_msdos is a tool for formating msdos filesystem. (such as fat32, vfat),
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r  ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
  		ARCH="$(TARGET_ARCH)" \
  		AR="$(TARGET_AR)" \
  		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
  		CFLAGS="$(TARGET_CFLAGS)" \
  		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Install
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/newfs_msdos $(PKG_INSTALL_DIR)/usr/sbin/

endef

define Build/Installdev
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/newfs_msdos $(1)/usr/sbin/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/newfs_msdos $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))

