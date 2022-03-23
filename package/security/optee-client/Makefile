#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=optee-client
PKG_VERSION:=2.5.0
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/OP-TEE/optee_client.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=32d9b77918ee634d1a16299164b543d628291123
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz

PKG_LICENSE:=BSD 2-Clause
PKG_LICENSE_FILES:=LICENSE

include $(BUILD_DIR)/package.mk

define Package/optee-client
  SECTION:=utils
  CATEGORY:=Security
  SUBMENU:=OPTEE
  DEPENDS:=+liballwinner-base
  TITLE:=optee-client
endef

define Package/optee-client/description
This component provides the TEE Client API as defined by the GlobalPlatform TEE standard.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_BUILD_DIR)/out/export/include/* $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/out/export/bin/*	$(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/out/export/lib/* $(1)/usr/lib
endef

define Package/optee-client/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/init.d

	$(CP) ./files/tee-supplicant.init $(1)/etc/init.d/tee-supplicant
	$(CP) $(PKG_BUILD_DIR)/out/export/bin/* $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/out/export/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,optee-client))
