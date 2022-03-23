#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=stress-ng
PKG_VERSION:=0.09.28
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://kernel.ubuntu.com/~cking/stress-ng/
PKG_MD5SUM:=b60b5b9b4d88730786581b3517929fca

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=COPYING

#PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lrt)

define Package/stress-ng
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=stress-ng is a more powerful stress utility
  URL:=http://kernel.ubuntu.com/~cking/stress-ng/
endef

define Package/stress-ng/description
stress-ng is a powerful tool that imposes certain types of compute \ stress on
UNIX-like operating systems.
endef

define Build/Compile
	#stress-ng
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		stress-ng
endef


CONFIGURE_ARGS += \
	--prefix="/usr"

MAKE_FLAGS += \
	CFLAGS="$(TARGET_CFLAGS)"

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/stress-ng  $(1)/usr/bin
endef

$(eval $(call BuildPackage,stress-ng))
