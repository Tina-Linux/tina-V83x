#
# Copyright (C) 2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=bsdiff
PKG_VERSION:=4.3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://www.daemonology.net/bsdiff/
PKG_MD5SUM:=e6d812394f0e0ecc8d5df255aa1db22a
PKG_MAINTAINER:=Hauke Mehrtens <hauke@hauke-m.de>
HOST_BUILD_DEPENDS:=bzip2/host

PKG_LICENSE:=BSD-2-Clause

include $(BUILD_DIR)/host-build.mk

# compile static for host without library liblzo2.so.2
HOST_LDFLAGS += -static

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR)/host/bin/
	$(MAKE) -C $(HOST_BUILD_DIR) PREFIX=$(STAGING_DIR)/host/ install
endef

define Host/Compile
	$(HOSTCC) $(HOST_CFLAGS) $(HOST_LDFLAGS) \
		-o $(HOST_BUILD_DIR)/bsdiff \
		$(HOST_BUILD_DIR)/bsdiff.c -lbz2
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bsdiff $(STAGING_DIR_HOST)/bin/
endef

$(eval $(call HostBuild))

