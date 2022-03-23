#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=lcms2
PKG_VERSION:=2.9
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://sourceforge.net/projects/lcms/files/latest/download?source=typ_redirect
PKG_MD5SUM:=8de1b7724f578d2995c8fdfa35c3ad0e
PKG_MAINTAINER:=

PKG_LICENSE:=MIT
PKC_LICENSE_FILES:=COPYING

PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/lcms2
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+PACKAGE_zlib:zlib +PACKAGE_libjpeg:libjpeg
  TITLE:=Small-footprint color management engine, version 2
  URL:=http://www.littlecms.com
endef

define Package/lcms2/description
lcms is a Library OF CODE FOR PROGRAMMERS.
lcms is FREE and does include SOURCE CODE.
lcms deals with color management stuff.
lcms is a CMM engine; it implements fast transforms between ICC profiles.
lcms is a standalone engine; it doesn't need ICM or ColorSync to work.
lcms intends to be PORTABLE across several platforms.
lcms is distributed under the MIT license agreement.
endef


TARGET_CFLAGS += $(FPIC)

ifeq ($(CONFIG_PACKAGE_libjpeg),y)
CONFIGURE_ARGS += --with-jpeg
else
CONFIGURE_ARGS += --without-jpeg
endif

ifeq ($(CONFIG_PACKAGE_zlib),y)
CONFIGURE_ARGS += --with-zlib
else
CONFIGURE_ARGS += --without-zlib
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/lcms2/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/liblcms2.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,lcms2))
