#
# Copyright (C) 2006-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cryptsetup
PKG_VERSION:=1.7.5
PKG_RELEASE:=1
PKG_LICENSE:=GPL-2.0+ LGPL-2.1+
PKG_LICENSE_FILES:=COPYING COPYING.LGPL

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@KERNEL/linux/utils/cryptsetup/v1.7
PKG_HASH:=2b30cd1d0dd606a53ac77b406e1d37798d4b0762fa89de6ea546201906a251bd
PKG_MAINTAINER:=Daniel Golle <daniel@makrotopia.org>

HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)

HOST_BUILD_PARALLEL:=1

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk

HOST_BUILD_DEPENDS:=libpopt/host lvm2/host
PKG_BUILD_DEPENDS:=cryptsetup/host

TARGET_LDFLAGS+=-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/cryptsetup
  SECTION:=utils
  CATEGORY:=Security
  SUBMENU:=Device Mapper
  TITLE:=Cryptsetup
  DEPENDS:=+libblkid +libuuid +libpopt +lvm2 +libdevmapper +@KERNEL_DIRECT_IO +CRYPTSETUP_OPENSSL:libopenssl +CRYPTSETUP_GCRYPT:libgcrypt
  URL:=http://code.google.com/p/cryptsetup/
endef

define Package/cryptsetup/config
source "$(SOURCE)/Config.in"
endef

define Package/cryptsetup/description
	Cryptsetup-luks
	Cryptsetup is utility used to conveniently setup disk encryption based
on DMCrypt kernel module.These include plain dm-crypt volumes, LUKS volumes,
loop-AES and TrueCrypt (including VeraCrypt extension) format. Project also
includes veritysetup utility used to conveniently setup DMVerity block
integrity checking kernel module.
endef

ifdef CONFIG_CRYPTSETUP_OPENSSL
CONFIGURE_ARGS+= \
        --with-crypto_backend=openssl
endif

HOST_CONFIGURE_ARGS+=--with-crypto_backend=kernel

define Package/cryptsetup/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/.libs/cryptsetup $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/src/.libs/veritysetup $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/lib/.libs/libcryptsetup.so* $(1)/usr/lib
endef

define Host/Install
	$(CP) $(HOST_BUILD_DIR)/lib/.libs/libcryptsetup.so* $(STAGING_DIR)/host/lib
	$(CP) $(HOST_BUILD_DIR)/src/.libs/cryptsetup $(STAGING_DIR)/host/bin
	$(CP) $(HOST_BUILD_DIR)/src/.libs/cryptsetup $(STAGING_DIR)/host/bin

	$(CP) $(HOST_BUILD_DIR)/lib/.libs/libcryptsetup.so* $(STAGING_DIR_HOST)/lib
	$(CP) $(HOST_BUILD_DIR)/src/.libs/cryptsetup $(STAGING_DIR_HOST)/bin
	$(CP) $(HOST_BUILD_DIR)/src/.libs/veritysetup $(STAGING_DIR_HOST)/bin
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,cryptsetup))
