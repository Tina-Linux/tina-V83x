#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free and open source software.
# It is available to be redistributed and/or modified under the terms of the Apache-2.0 license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libump
PKG_VERSION:=ec0680628744f30b8fac35e41a7bd8e23e59c39f
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://github.com/linux-sunxi/libump
PKG_MD5SUM:=27e502e8063849f33932b3dc83c3c3f9

PKG_LICENSE:=Apache-2.0
PKC_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0

include $(BUILD_DIR)/package.mk

define Package/libump
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=This library is a requirement for ARMs binary Mali driver
  URL:=http://github.com/linux-sunxi/libump
endef

define Package/libump/description
  The ARMs Universal Memory Provider userspace library.
This library is a requirement for ARMs binary Mali driver.
endef

TARGET_CFLAGS += $(FPIC)

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/libump/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libump))
