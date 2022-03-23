#
# Copyright (C) 2007-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=memtester
PKG_VERSION:=4.3.0
PKG_RELEASE:=1
PKG_IREMAP:=1
PKG_FORTIFY_SOURCE:=1
PKG_RELRO:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://pyropus.ca/software/memtester/old-versions/
PKG_MD5SUM:=598f41b7308e1f736164bca3ab84ddbe

include $(BUILD_DIR)/package.mk

define Package/memtester
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Userspace utility for testing the memory subsystem for faults
  URL:=http://pyropus.ca/software/memtester/
endef

define Package/memtester/description
 A userspace utility for testing the memory subsystem for faults.
endef

define Build/Configure
	$(SED) 's,cc -O2,$(TARGET_CC) $(TARGET_CFLAGS),' $(PKG_BUILD_DIR)/conf-cc
	$(SED) 's,cc,$(TARGET_CC),' $(PKG_BUILD_DIR)/conf-ld
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) all
endef

define Package/memtester/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/memtester $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,memtester))
