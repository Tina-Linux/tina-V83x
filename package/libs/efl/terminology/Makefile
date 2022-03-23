#
# Copyright (C) 2006-2015 OpenWrt.org
# Copyright notice for Terminology (BSD 2-Clause License):
#
# Copyright (C) 2012-2016 Carsten Haitzler and various contributors (see AUTHORS)
#
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#   1. Redistributions of source code must retain the above copyright
#      notice, this list of conditions and the following disclaimer.
#   2. Redistributions in binary form must reproduce the above copyright
#      notice, this list of conditions and the following disclaimer in the
#      documentation and/or other materials provided with the distribution.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=terminology
PKG_VERSION:=1.1.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://download.enlightenment.org/rel/apps/terminology
PKG_MD5SUM:=c5d9603f60d2ba3b6d99cc8ee0526806

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(BUILD_DIR)/package.mk

define Package/terminology
  SECTION:=libs
  SUBMENU:=EFL
  CATEGORY:=Libraries
  TITLE:=A terminal emulator for Linux/BSD/UNIX systems that use EFL
  DEPENDS:=+efl +eudev
  MAINTAINER:=Boris Faure <billiob@gmail.com>
  URL:=https://www.enlightenment.org/about-terminology
endef

define Package/terminology/description
Terminology is a terminal emulator bristling with advanced features including
tabs, splits, 256-color support, wallpapers (even animated ones),
in-line display of media (click on a file path to a video and watch it play),
link detection, compressed scrollback stored in RAM, translucency and
a gorgeous look and feel inspired by classic CRT monitors.
endef

CONFIGURE_ARGS += \
	--with-edje-cc=$(STAGING_DIR)/host/bin/edje_cc

TARGET_LDFLAGS+= -ludev
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -ludev

define Package/terminology/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
endef

$(eval $(call BuildPackage,terminology))
