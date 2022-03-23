#
# Copyright (C) 2009-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mtd-utils
PKG_REV:=ab8c6fb93ce9db0f09401c4b819b0b277dc00340
PKG_VERSION:=1.5.2
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=git://git.infradead.org/mtd-utils.git
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=$(PKG_REV)

PKG_INSTALL:=1

PKG_BUILD_DEPENDS:=util-linux

include $(BUILD_DIR)/package.mk

FILES:= \
	doc_loadbios \
	docfdisk \
	flash_erase \
	flash_eraseall:+mtd-utils-flash-erase \
	flash_lock \
	flash_otp_dump \
	flash_otp_lock \
	flash_otp_write \
	flash_unlock \
	flashcp \
	ftl_check \
	ftl_format \
	jffs2dump \
	jffs2reader:+zlib \
	mkfs.jffs2:+zlib \
	mkfs.ubifs:+libuuid:+liblzo:+zlib \
	mtd_debug \
	mtdinfo \
	nanddump \
	nandtest \
	nandwrite \
	nftldump \
	nftl_format \
	recv_image \
	rfddump \
	rfdformat \
	serve_image \
	sumtool \
	ubiattach \
	ubicrc32 \
	ubidetach \
	ubiformat \
	ubimkvol \
	ubinfo \
	ubinize \
	ubirename \
	ubirmvol \
	ubirsvol \
	ubiupdatevol \
	ubiblock

define PartGen
define Package/mtd-utils-$(subst _,-,$(firstword $(subst :, ,$(1))))
  TITLE:=$(firstword $(subst :, ,$(1))) package from mtd-utils
  URL:=http://www.linux-mtd.infradead.org/
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=mtd-utils $(wordlist 2,$(words $(subst :, ,$(1))),$(subst :, ,$(1)))
endef
endef

define Package/mtd-utils
  TITLE:=Utilities for flash info/debug
  SECTION:=utils
  CATEGORY:=Utilities
  URL:=http://www.linux-mtd.infradead.org/
  MENU:=1
endef

define Package/mtd-utils/description
  Utilities for manipulating memory technology devices.
endef

define Package/mtd-utils/install
	true
endef

define Build/InstallDev
	$(INSTALL_DIR)  $(STAGING_DIR)/lib
	$(INSTALL_DIR)  $(STAGING_DIR)/usr/include/mtd
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/lib/libmtd.a $(STAGING_DIR)/lib
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/ubi-utils/libubi.a $(STAGING_DIR)/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/ubi-utils/include/libubi.h $(STAGING_DIR)/usr/include/mtd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/mtd/*.h $(STAGING_DIR)/usr/include/mtd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/libmtd.h $(STAGING_DIR)/usr/include/mtd
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/include/mtd_swab.h $(STAGING_DIR)/usr/include/mtd
endef

$(foreach file,$(FILES),$(eval $(call PartGen,$(file))))

MAKE_FLAGS += \
	DESTDIR="$(PKG_INSTALL_DIR)" \
	BUILDDIR="$(PKG_BUILD_DIR)" \
	LDLIBS+="$(LIBGCC_S)" \
	WITHOUT_XATTR=1 \
	WITHOUT_LZO=1

define PartInstall
define Package/mtd-utils-$(subst _,-,$(firstword $(subst :, ,$(1))))/install
	$(INSTALL_DIR) $$(1)/usr/sbin
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/sbin/$(firstword $(subst :, ,$(1))) \
		$$(1)/usr/sbin/
endef
endef

$(foreach file,$(FILES),$(eval $(call PartInstall,$(file))))

$(eval $(call BuildPackage,mtd-utils))
$(foreach file,$(FILES),$(eval $(call BuildPackage,mtd-utils-$(subst _,-,$(firstword $(subst :, ,$(file)))))))
