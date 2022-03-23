#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=sysbench
PKG_VERSION:=0.5

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_FIXUP:=autoreconf
PKG_LICENSE:=GPL-2.0

PKG_INSTALL:=1


include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=+libstdcpp +libpng +libjpeg
  TITLE:=sysbench
  URL:=http://github.com/akopytov/sysbench
endef

define Package/$(PKG_NAME)/description
	sysbench Package
endef


CONFIGURE_ARGS += \
        --without-mysql \
	--prefix=/usr \

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
