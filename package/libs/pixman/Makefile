#
# Copyright (C) 2006-2015 OpenWrt.org
#
# Pixman is free and open source software.
# It is available to be redistributed and/or modified under the terms of the MIT license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=pixman
PKG_VERSION:=0.34.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://xorg.freedesktop.org/releases/individual/lib
PKG_MD5SUM:=002a4fcb644ddfcb4b0e4191576a0d59
PKG_MAINTAINER:=<pixman@lists.freedesktop.org>

PKG_LICENSE:=MIT
PKC_LICENSE_FILES:=COPYING

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0

include $(BUILD_DIR)/package.mk

define Package/libpixman
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Pixman is a low-level software library for pixel manipulation
  URL:=https://cgit.freedesktop.org/pixman/
endef

TARGET_CFLAGS += $(FPIC)

# Don't build gtk based demos
# The ARM SIMD code from pixman requires a recent enough ARM core, but
# there is a runtime CPU check that makes sure it doesn't get used if
# the HW doesn't support it. The only case where the ARM SIMD code
# cannot be *built* at all is when the platform doesn't support ARM
# instructions at all, so we have to disable that explicitly.
# disable iwmmxt support for CPU's that don't have this feature
CONFIGURE_ARGS += \
	--disable-gtk \
	--disable-arm-simd \
	--disable-arm-neon \
	--disable-arm-iwmmxt

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/pixman-1 $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpixman-1.{a,la,so*} $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/pixman-1.pc $(1)/usr/lib/pkgconfig/
endef

define Package/libpixman/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libpixman-1.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,libpixman))
