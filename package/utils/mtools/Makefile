#
# Copyright (C) 2012-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
include $(TOPDIR)/rules.mk

PKG_NAME:=mtools
PKG_VERSION:=4.0.18

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=@GNU/$(PKG_NAME)
PKG_MD5SUM:=17b9f525c1ae3583a478338deb7fbc19
PKG_CAT:=zcat
PKG_INSTALL:=1

PKG_FIXUP:=autoreconf

PKG_BUILD_PARALLEL:=1

include $(BUILD_DIR)/package.mk

CONFIGURE_ARGS += \
	ac_cv_header_iconv_h=no \
	--disable-debug \
	$(if $(CONFIG_MTOOLS_MATTRIB), --enable-mattrib, --disable-mattrib) \
	$(if $(CONFIG_MTOOLS_MBADBLOCKS), --enable-mbadblocks, --disable-mbadblocks) \
	$(if $(CONFIG_MTOOLS_MCAT), --enable-mcat, --disable-mcat) \
	$(if $(CONFIG_MTOOLS_MCD), --enable-mcd, --disable-mcd) \
	$(if $(CONFIG_MTOOLS_MCLASSERASE), --enable-mclasserase, --disable-mclasserase) \
	$(if $(CONFIG_MTOOLS_MCOPY), --enable-mcopy, --disable-mcopy) \
	$(if $(CONFIG_MTOOLS_MDEL), --enable-mdel, --disable-mdel) \
	$(if $(CONFIG_MTOOLS_MDELTREE), --enable-mdeltree, --disable-mdeltree) \
	$(if $(CONFIG_MTOOLS_MDIR), --enable-mdir, --disable-mdir) \
	$(if $(CONFIG_MTOOLS_MDOCTORFAT), --enable-mdoctorfat, --disable-mdoctorfat) \
	$(if $(CONFIG_MTOOLS_MDU), --enable-mdu, --disable-mdu) \
	$(if $(CONFIG_MTOOLS_MFORMAT), --enable-mformat, --disable-mformat) \
	$(if $(CONFIG_MTOOLS_MINFO), --enable-minfo, --disable-minfo) \
	$(if $(CONFIG_MTOOLS_MLABEL), --enable-mlabel, --disable-mlabel) \
	$(if $(CONFIG_MTOOLS_MMD), --enable-mmd, --disable-mmd) \
	$(if $(CONFIG_MTOOLS_MMOUNT), --enable-mmount, --disable-mmount) \
	$(if $(CONFIG_MTOOLS_MPARTITION), --enable-mpartition, --disable-mpartition) \
	$(if $(CONFIG_MTOOLS_MRD), --enable-mrd, --disable-mrd) \
	$(if $(CONFIG_MTOOLS_MREAD), --enable-mread, --disable-mread) \
	$(if $(CONFIG_MTOOLS_MMOVE), --enable-mmove, --disable-mmove) \
	$(if $(CONFIG_MTOOLS_MREN), --enable-mren, --disable-mren) \
	$(if $(CONFIG_MTOOLS_MSHOWFAT), --enable-mshowfat, --disable-mshowfat) \
	$(if $(CONFIG_MTOOLS_MSHORTNAME), --enable-mshortname, --disable-mshortname) \
	$(if $(CONFIG_MTOOLS_MTOOLSTEST), --enable-mtoolstest, --disable-mtoolstest) \
	$(if $(CONFIG_MTOOLS_MTYPE), --enable-mtype, --disable-mtype) \
	$(if $(CONFIG_MTOOLS_MWRITE), --enable-mwrite, --disable-mwrite) \
	$(if $(CONFIG_MTOOLS_MZIP), --enable-mzip, --disable-mzip)

define Package/mtools/Default
  SECTION:=utils
  CATEGORY:=Utilities
endef

define Package/mtools
$(call Package/mtools/Default)
  TITLE:= Utilities to access DOS disks in Unix.
  MENU:=1
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

$(eval $(call BuildPackage,mtools))

