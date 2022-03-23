#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=binutils
PKG_VERSION:=$(call qstrip,$(CONFIG_BINUTILS_VERSION))
BIN_VERSION:=$(PKG_VERSION)

PKG_SOURCE_URL:=@GNU/binutils/
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

ifeq ($(PKG_VERSION),2.26)
  PKG_MD5SUM:=64146a0faa3b411ba774f47d41de239f
endif

ifeq ($(PKG_VERSION),2.27)
  PKG_MD5SUM:=41b053ed4fb2c6a8173ef421460fbb28
endif

ifeq ($(PKG_VERSION),2.28)
  PKG_MD5SUM:=d5d270fd0b698ed59ca5ade8e1b5059c
endif

HOST_BUILD_PARALLEL:=1

PATCH_DIR:=./patches/$(PKG_VERSION)

REAL_STAGING_DIR_HOST:=$(STAGING_DIR_HOST)

include $(BUILD_DIR)/toolchain-build.mk

BINUTILS_CONFIGURE:= \
	./configure \
		--prefix=$(TOOLCHAIN_DIR) \
		--build=$(GNU_HOST_NAME) \
		--host=$(GNU_HOST_NAME) \
		--target=$(REAL_GNU_TARGET_NAME) \
		--with-sysroot=$(TOOLCHAIN_DIR) \
		--enable-deterministic-archives \
		--enable-plugins \
		--enable-lto \
		--disable-multilib \
		--disable-werror \
		--disable-nls \
		--disable-sim \
		--disable-gdb \
		$(GRAPHITE_CONFIGURE) \
		$(SOFT_FLOAT_CONFIG_OPTION) \
		$(call qstrip,$(CONFIG_EXTRA_BINUTILS_CONFIG_OPTIONS)) 

ifneq ($(CONFIG_SSP_SUPPORT),)
  BINUTILS_CONFIGURE+= \
		--enable-libssp
else
  BINUTILS_CONFIGURE+= \
		--disable-libssp
endif

ifneq ($(CONFIG_EXTRA_TARGET_ARCH),)
  BINUTILS_CONFIGURE+= \
		--enable-targets=$(call qstrip,$(CONFIG_EXTRA_TARGET_ARCH_NAME))-linux-$(TARGET_SUFFIX)
endif

define Host/Prepare
	$(call Host/Prepare/Default)
	ln -snf $(notdir $(HOST_BUILD_DIR)) $(COMPILE_DIR_TOOLCHAIN)/$(PKG_NAME)
	$(CP) $(SCRIPT_DIR)/config.{guess,sub} $(HOST_BUILD_DIR)/
	$(SED) 's, " Linaro.*,,' $(HOST_BUILD_DIR)/bfd/version.h
endef

define Host/Configure
	(cd $(HOST_BUILD_DIR); \
		$(BINUTILS_CONFIGURE) \
	);
endef

define Host/Compile
	+$(MAKE) $(HOST_JOBS) -C $(HOST_BUILD_DIR) all
endef

define Host/Install
	mkdir -p $(TOOLCHAIN_DIR)/initial
	$(MAKE) -C $(HOST_BUILD_DIR) \
		prefix=$(TOOLCHAIN_DIR)/initial \
		install
	$(MAKE) -C $(HOST_BUILD_DIR) \
		prefix=$(TOOLCHAIN_DIR) \
		install
	$(call FixupLibdir,$(TOOLCHAIN_DIR)/initial)
	$(RM) $(TOOLCHAIN_DIR)/initial/lib/libiberty.a
	$(CP) $(TOOLCHAIN_DIR)/bin/$(REAL_GNU_TARGET_NAME)-readelf $(REAL_STAGING_DIR_HOST)/bin/readelf
	# ARC gcc requires extlib.
	# If extlib is not available in "initial" folder
	# initial gcc will fail to build libc.
	if [ -d $(TOOLCHAIN_DIR)/extlib ]; then \
		$(CP) -r $(TOOLCHAIN_DIR)/extlib $(TOOLCHAIN_DIR)/initial/; \
	fi
endef

define Host/Clean
	rm -rf \
		$(HOST_BUILD_DIR) \
		$(COMPILE_DIR_TOOLCHAIN)/$(PKG_NAME)
endef

$(eval $(call HostBuild))
