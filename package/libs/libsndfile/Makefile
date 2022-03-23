#
# Copyright (C) 2009-2010 OpenWrt.org
#
# Primarily used for software libraries, the GNU LGPL requires that derived works be licensed under the same license,
# but works that only link to it do not fall under this restriction. There are two commonly used versions of the GNU LGPL.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libsndfile
PKG_VERSION:=1.0.28
PKG_RELEASE:=1
PKG_LICENSE:=LGPL

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://sources.voidlinux.eu/libsndfile-1.0.28
PKG_MD5SUM:=646b5f98ce89ac60cdb060fcd398247c

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/libsndfile
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=A C library for reading and writing files containing sampled audio data
  URL:=http://kbs.cs.tu-berlin.de/~jutta/toast.html
  MAINTAINER:=Erik de Castro Lopo <erikd@mega-nerd.com>
endef

define Package/libsndfile/description
libsndfile is a widely used C library written by Erik de Castro Lopo for
reading and writing audio files. It supports a wide variety of audio file formats
and will convert automatically from one to another.
It allows the programmer to ignore many details, such as endianness.
endef

TARGET_CFLAGS += $(FPIC)

CONFIGURE_ARGS += \
	--disable-static \
	--disable-bow-docs \
	--enable-alsa \
	--disable-test-coverage \
	--disable-full-suite \
	--disable-external-libs

CONFIGURE_VARS += \
	ac_cv_sys_file_offset_bits=64 \
	ac_cv_sys_large_files=yes \
	ac_cv_sys_largefile_CFLAGS=-D_LARGFILE_SOURCE \
	ac_cv_sys_largefile_LDFLAGS= \
	ac_cv_sys_largefile_LIBS= \
	ac_cv_sys_largefile_source=yes

define Package/libsndfile/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

$(eval $(call BuildPackage,libsndfile))
