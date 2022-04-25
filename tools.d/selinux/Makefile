#
# Copyright (C) 2006-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=selinux
PKG_VERSION:=20170804

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/SELinuxProject/selinux/releases/tag/20170804/
PKG_MD5SUM:=8de0165799cae72f015f65e4511eb3db
PKG_CAT:=zcat

HOST_BUILD_DIR := $(COMPILE_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk

define Host/Compile
	# compile libsepol
	$(MAKE) -C $(HOST_BUILD_DIR)/libsepol \
		CC="$(HOSTCC)" \
		CXX="$(CXX)" \
		CFLAGS="$(HOST_CFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		DESTDIR="$(STAGING_DIR_HOST)" \
		SHLIBDIR="$(STAGING_DIR_HOST)/usr/lib" \
		all install

	# compile libselinux
	$(MAKE) -C $(HOST_BUILD_DIR)/libselinux \
		CC="$(HOSTCC)" \
		CXX="$(CXX)" \
		CFLAGS="$(HOST_CFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS) -lpcre" \
		DESTDIR="$(STAGING_DIR_HOST)" \
		SHLIBDIR="$(STAGING_DIR_HOST)/usr/lib" \
		all install

	# compile checkpolicy
	$(MAKE) -C $(HOST_BUILD_DIR)/checkpolicy \
		CC="$(HOSTCC)" \
		CXX="$(CXX)" \
		CFLAGS="$(HOST_CFLAGS)" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		DESTDIR="$(STAGING_DIR_HOST)" \
		SHLIBDIR="$(STAGING_DIR_HOST)/usr/lib" \
		all install
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/libsepol/src/libsepol.so* $(STAGING_DIR_HOST)/usr/lib
	$(CP) $(HOST_BUILD_DIR)/libselinux/src/libselinux.so* $(STAGING_DIR_HOST)/usr/lib
	$(CP) $(HOST_BUILD_DIR)/checkpolicy/checkpolicy $(STAGING_DIR_HOST)/usr/bin
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/usr/lib/libsepol.*
	rm -f $(STAGING_DIR_HOST)/usr/lib/libselinux.*
	rm -f $(STAGING_DIR_HOST)/usr/bin/checkpolicy
endef

$(eval $(call HostBuild))
