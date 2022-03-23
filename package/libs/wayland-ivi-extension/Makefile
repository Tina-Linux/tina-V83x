#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wayland-ivi-extension
PKG_VERSION:=1.13.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://github.com/GENIVI/wayland-ivi-extension/releases
PKG_MD5SUM:=

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

CMAKE_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS=-DBUILDARCH=$(CONFIG_ARCH) \
	-DENABLEEXAMPLES=0 \
	-DNODE_EXECUTABLE=$(OUT_DIR)/host/bin/node \
	-DSWIG_DIR=$(OUT_DIR)/host/bin

CMAKE_OPTIONS += -DBUILD_ILM_API_TESTS=1

define Package/libwayland-ivi-extension
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Wayland-ivi-extension
  URL:=http://github.com/GENIVI/wayland-ivi-extension
endef


define Package/libwayland-ivi-extension/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so.* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libwayland-ivi-extension))
