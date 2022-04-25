#
# Copyright (C) 2009-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=squashfs4
PKG_VERSION:=4.3

PKG_SOURCE:=squashfs$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@SF/squashfs
PKG_MD5SUM:=d92ab59aabf5173f2a59089531e30dbf
PKG_CAT:=zcat

HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/squashfs$(PKG_VERSION)

HOST_PREPARED_DEPENDS += CONFIG_USE_SELINUX

include $(BUILD_DIR)/host-build.mk

ifneq ($(CONFIG_USE_SELINUX),)
SELINUX_ENABLE=1
else
SELINUX_ENABLE=0
endif

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR)/squashfs-tools \
		CC="$(HOSTCC)" \
		XZ_SUPPORT=1 \
		LZMA_XZ_SUPPORT=1 \
		XATTR_SUPPORT=1 \
		SELINUX_SUPPORT=$(SELINUX_ENABLE) \
		LZMA_LIB="$(STAGING_DIR_HOST)/lib/liblzma.a" \
		EXTRA_CFLAGS="-I$(STAGING_DIR_HOST)/include -I$(STAGING_DIR_HOST)/usr/include" \
		LDFLAGS="$(HOST_LDFLAGS)" \
		mksquashfs unsquashfs
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/mksquashfs $(STAGING_DIR_HOST)/bin/mksquashfs4
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/unsquashfs $(STAGING_DIR_HOST)/bin/unsquashfs4
ifneq ($(CONFIG_USE_SELINUX),)
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/mksquashfs $(STAGING_DIR_HOST)/bin/mksquashfs4_se
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/squashfs-tools/unsquashfs $(STAGING_DIR_HOST)/bin/unsquashfs4_se
endif
endef

define Host/Clean
	rm -f $(STAGING_DIR_HOST)/bin/mksquashfs4
	rm -f $(STAGING_DIR_HOST)/bin/unsquashfs4
endef

$(eval $(call HostBuild))
