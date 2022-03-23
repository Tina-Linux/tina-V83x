#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=optee-test
PKG_VERSION:=2.5.0
PKG_RELEASE:=1

PKG_SOURCE:=2.5.0.tar.gz
PKG_SOURCE_URL:=https://github.com/OP-TEE/optee_test/archive
PKG_MD5SUM:=e26c80269cb29add5ff4dd309a27ed4e

PKG_BUILD_DIR:=$(COMPILE_DIR)/optee_test-$(PKG_VERSION)

PKG_LICENSE:=GPL-2.0 & BSD 2-Clause
PKG_LICENSE_FILES:=LICENSE

PKG_CONFIG_DEPENDS += CONFIG_OPTEE_ENCRYPT_TA CONFIG_OPTEE_ENCRYPT_TA_SSK_KEY CONFIG_OPTEE_ENCRYPT_TA_ROTPK_DERIVE_KEY

PKG_FILE_DEPENDS += $(STAGING_DIR)/usr/dev_kit/arm-plat-$(TARGET_CHIP)/export-ta_arm32/keys/default_ta.pem
ifeq ($(CONFIG_OPTEE_ENCRYPT_TA),y)
PKG_FILE_DEPENDS += $(STAGING_DIR)/usr/dev_kit/arm-plat-$(TARGET_CHIP)/export-ta_arm32/keys/ta_aes_key.bin
endif
include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Security
  SUBMENU:=OPTEE
  DEPENDS:=+liballwinner-base +optee-client +optee-os-dev-kit
  TITLE:=optee-test
endef

define Package/$(PKG_NAME)/description
This component provides the TEE sanity testsuite in Linux using the ARM(R) TrustZone(R) technology.
endef


# TA MUST be compiled with 32bit cross toolchain.
ifeq ($(CONFIG_ARCH_64BIT),y)
TARGET_CROSS_TA=$(subst aarch64,arm,$(CONFIG_TOOLCHAIN_ROOT)/bin/arm-openwrt-linux-)
else
TARGET_CROSS_TA:=$(TARGET_CROSS)
endif

define Build/Compile/Source
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE_TA="$(TARGET_CROSS_TA)" \
		CROSS_COMPILE_HOST="$(TARGET_CROSS)" \
		TA_LDFLAGS="$(TARGET_LDFLAGS)" \
		TA_DEV_KIT_DIR="$(STAGING_DIR)/usr/dev_kit/arm-plat-$(TARGET_CHIP)/export-ta_arm32" \
		OPTEE_CLIENT_EXPORT="$(STAGING_DIR)/usr"
endef

define Build/Compile
        $(call Build/Compile/Source)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/lib/optee_armtz

	$(CP) $(PKG_BUILD_DIR)/out/xtest/xtest $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/out/ta/*/*.ta $(1)/lib/optee_armtz
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
