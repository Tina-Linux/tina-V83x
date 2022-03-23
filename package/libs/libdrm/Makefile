#
# Copyright (C) 2006-2015 OpenWrt.org
#
# Pixman is free and open source software.
# It is available to be redistributed and/or modified under the terms of the MIT license.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libdrm
PKG_VERSION:=2.4.80
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://dri.freedesktop.org/libdrm
PKG_MD5SUM:=4867866477f228973a64e68051ca9c66
PKG_MAINTAINER:=<dri-devel@lists.freedesktop.org>

PKG_LICENSE:=MIT
PKC_LICENSE_FILES:=COPYING

PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0

include $(BUILD_DIR)/package.mk

define Package/libdrm
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+PACKAGE_valgrind:valgrind +PACKAGE_eudev:eudev +PACKAGE_libatomic:libatomic
  TITLE:=libdrm provides a user space library for accessing the DRM
  URL:=http://www.linuxfromscratch.org/blfs/view/svn/x/libdrm.html
  MENU:=1
endef

define Package/libdrm/description
  libdrm provides a user space library for accessing the DRM, direct rendering manager,
on operating systems that support the ioctl interface. libdrm is a low-level library,
typically used by graphics drivers such as the Mesa DRI drivers, the X drivers,
libva and similar projects.
endef

TARGET_CFLAGS += $(FPIC) -std=gnu99

CONFIGURE_ARGS += \
	--disable-cairo-tests \
	--disable-manpages \
	--disable-radeon \
	--disable-intel \
	--disable-amdgpu \
	--disable-nouveau \
	--disable-vmwgfx \
	--disable-freedreno \
	--disable-omap-experimental-api \
	--disable-etnaviv-experimental-api \
	--disable-exynos-experimental-api \
	--disable-tegra-experimental-api

define Package/libdrm/config
	source "$(SOURCE)/Config.in"
endef

ifeq ($(CONFIG_LIBDRM_TEST_PROGRAMS),y)
CONFIGURE_ARGS += --enable-install-test-programs
else
CONFIGURE_ARGS += --disable-install-test-programs
endif

ifeq ($(CONFIG_LIBDRM_V4C),y)
CONFIGURE_ARGS += --enable-vc4
else
CONFIGURE_ARGS += --disable-vc4
endif

ifeq ($(CONFIG_PACKAGE_valgrind),y)
CONFIGURE_ARGS += --enable-valgrind
else
CONFIGURE_ARGS += --disable-valgrind
endif

ifeq ($(CONFIG_PACKAGE_eudev),y)
CONFIGURE_ARGS += --enable-udev
else
CONFIGURE_ARGS += --disable-udev
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/libdrm/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/

ifeq ($(CONFIG_LIBDRM_TEST_PROGRAMS),y)
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endif
endef

$(eval $(call BuildPackage,libdrm))
