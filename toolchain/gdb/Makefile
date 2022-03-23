#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=gdb

PKG_VERSION:=8.0
#PKG_VERSION:=8.2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNU/gdb
PKG_MD5SUM:=c3d35cd949084be53b92cc1e03485f88
#PKG_MD5SUM:=b5a49dbff00d9a87fbe114d14b3101c0    # md5sum of gdb 8.2
GDB_DIR:=$(PKG_NAME)-$(PKG_VERSION)

PATCH_DIR:=./patches/$(PKG_VERSION)

HOST_BUILD_DIR:=$(COMPILE_DIR_TOOLCHAIN)/$(GDB_DIR)

HOST_BUILD_PARALLEL:=1

include $(BUILD_DIR)/toolchain-build.mk

HOST_CONFIGURE_VARS += \
	gdb_cv_func_sigsetjmp=yes

HOST_CONFIGURE_ARGS = \
	--prefix=$(TOOLCHAIN_DIR) \
	--build=$(GNU_HOST_NAME) \
	--host=$(GNU_HOST_NAME) \
	--target=$(REAL_GNU_TARGET_NAME) \
	--disable-werror \
	--without-uiout \
	--disable-tui --disable-gdbtk --without-x \
	--without-included-gettext \
	--enable-threads \
	--with-expat \
	--without-python \
	--disable-binutils \
	--disable-ld \
	--disable-gas \
	--disable-sim

define Host/Install
	mkdir -p $(TOOLCHAIN_DIR)/bin
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/gdb/gdb $(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb
	ln -fs $(TARGET_CROSS)gdb $(TOOLCHAIN_DIR)/bin/$(GNU_TARGET_NAME)-gdb
	strip $(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(TOOLCHAIN_DIR)/bin/$(TARGET_CROSS)gdb \
		$(TOOLCHAIN_DIR)/bin/$(GNU_TARGET_NAME)-gdb
endef

$(eval $(call HostBuild))
