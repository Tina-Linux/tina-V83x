#
# Copyright (C) 2006-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=glibc
PKG_VERSION:=$(call qstrip,$(CONFIG_GLIBC_VERSION))

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=git://sourceware.org/git/glibc.git
PKG_SOURCE_VERSION:=$(PKG_VERSION)

PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_REVISION)
PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.bz2

GLIBC_PATH:=

ifneq ($(CONFIG_GLIBC_USE_VERSION_2_21),)
  REVISION:=2015-02-06
  PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
  PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
  MD5SUM:=9cb398828e8f84f57d1f7d5588cf40cd
endif

ifneq ($(CONFIG_GLIBC_USE_VERSION_2_22),)
  REVISION:=b995d95
  PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)-$(REVISION)
  PKG_SOURCE:=$(PKG_SOURCE_SUBDIR).tar.bz2
  MD5SUM:=b575850e77b37d70f96472285290b391
endif

ifneq ($(CONFIG_GLIBC_USE_VERSION_2_23),)
  REVISION:=2016-02-18
  PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
  PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
  MD5SUM:=456995968f3acadbed39f5eba31678df
endif

ifneq ($(CONFIG_GLIBC_USE_VERSION_2_29),)
  PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
  PKG_SOURCE_VERSION:=56c86f5dd516284558e106d04b92875d5b623b7a
  PKG_MIRROR_HASH:=fa85d88e67467a8b026af441bf4376bbb3952042a1ba89c75dc8d2034f7e5505
  PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.xz
endif

ifneq ($(CONFIG_GLIBC_USE_VERSION_2_11),)
  REVISION:=2010.09
  PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(REVISION).tar.bz2
  MD5SUM:=00c6e9e52725496f0ee6a6fb8359ba83
endif

PATCH_DIR:=$(PATH_PREFIX)/patches/$(PKG_VERSION)

HOST_BUILD_DIR:=$(COMPILE_DIR_TOOLCHAIN)/$(PKG_SOURCE_SUBDIR)
CUR_BUILD_DIR:=$(HOST_BUILD_DIR)-$(VARIANT)

include $(BUILD_DIR)/toolchain-build.mk

HOST_STAMP_PREPARED:=$(HOST_BUILD_DIR)/.prepared
HOST_STAMP_CONFIGURED:=$(CUR_BUILD_DIR)/.configured
HOST_STAMP_BUILT:=$(CUR_BUILD_DIR)/.built
HOST_STAMP_INSTALLED:=$(TOOLCHAIN_DIR)/stamp/.glibc_$(VARIANT)_installed

ifeq ($(ARCH),mips64)
  ifdef CONFIG_MIPS64_ABI_N64
    TARGET_CFLAGS += -mabi=64
  endif
  ifdef CONFIG_MIPS64_ABI_N32
    TARGET_CFLAGS += -mabi=n32
  endif
  ifdef CONFIG_MIPS64_ABI_O32
    TARGET_CFLAGS += -mabi=32
  endif
endif

GLIBC_CONFIGURE:= \
	BUILD_CC="$(HOSTCC)" \
	$(TARGET_CONFIGURE_OPTS) \
	CFLAGS="$(TARGET_CFLAGS)" \
	CPPFLAGS="$(filter -m%,$(TARGET_CFLAGS))" \
	libc_cv_slibdir="/lib" \
	BASH_SHELL="/bin/sh" \
	use_ldconfig=no \
	$(HOST_BUILD_DIR)/$(GLIBC_PATH)configure \
		--prefix= \
		--build=$(GNU_HOST_NAME) \
		--host=$(REAL_GNU_TARGET_NAME) \
		--with-headers=$(TOOLCHAIN_DIR)/include \
		--disable-profile \
		--disable-werror \
		--without-gd \
		--without-cvs \
		--enable-add-ons \
		--$(if $(CONFIG_SOFT_FLOAT),without,with)-fp

export libc_cv_ssp=no
export ac_cv_header_cpuid_h=yes
export HOST_CFLAGS := $(HOST_CFLAGS) -idirafter $(CURDIR)/$(PATH_PREFIX)/include

define Host/SetToolchainInfo
	$(SED) 's,^\(LIBC_TYPE\)=.*,\1=$(PKG_NAME),' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_URL\)=.*,\1=http://www.gnu.org/software/libc/,' $(TOOLCHAIN_DIR)/info.mk
	$(SED) 's,^\(LIBC_VERSION\)=.*,\1=$(PKG_VERSION),' $(TOOLCHAIN_DIR)/info.mk
ifneq ($(CONFIG_GLIBC_USE_VERSION_2_11),)
	$(SED) 's,^\(LIBC_SO_VERSION\)=.*,\1=$(PKG_VERSION).1,' $(TOOLCHAIN_DIR)/info.mk
else
	$(SED) 's,^\(LIBC_SO_VERSION\)=.*,\1=$(PKG_VERSION),' $(TOOLCHAIN_DIR)/info.mk
endif
endef

define Host/Configure
	[ -f $(HOST_BUILD_DIR)/.autoconf ] || { \
		cd $(HOST_BUILD_DIR)/$(GLIBC_PATH); \
		autoconf --force && \
		touch $(HOST_BUILD_DIR)/.autoconf; \
	}
	mkdir -p $(CUR_BUILD_DIR)
	( cd $(CUR_BUILD_DIR); rm -f config.cache; \
		$(GLIBC_CONFIGURE) \
	);
endef

define Host/Prepare
	$(call Host/Prepare/Default)
	ln -snf $(PKG_SOURCE_SUBDIR) $(COMPILE_DIR_TOOLCHAIN)/$(PKG_NAME)
endef

define Host/Clean
	rm -rf $(CUR_BUILD_DIR)* \
		$(COMPILE_DIR_TOOLCHAIN)/$(LIBC)-dev \
		$(COMPILE_DIR_TOOLCHAIN)/$(PKG_NAME)
endef
