#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=pcre
PKG_VERSION:=8.38
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/
PKG_MD5SUM:=00aabbfe56d5a48b270f999b508c5ad2
PKG_MAINTAINER:=Thomas Heil <heil@terminal-consulting.de>

HOST_BUILD_PARALLEL:=1

include $(BUILD_DIR)/host-build.mk

TARGET_CFLAGS += $(FPIC)

HOST_CONFIGURE_ARGS += \
	--enable-utf8 \
	--enable-unicode-properties \
	--enable-pcre16 \

HOST_MAKE_FLAGS += \
	CFLAGS="$(HOST_CFLAGS)"

define Host/Install
	$(call Host/Install/Default)
endef

$(eval $(call HostBuild))
