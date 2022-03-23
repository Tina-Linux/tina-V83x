#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=playready
PKG_VERSION:=3.0.2527

###Currently only r18 r30 r16 is supported
ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),r18 r30 r16))

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=BSD

PKG_INSTALL:=1


include $(BUILD_DIR)/package.mk

SOURCE_DIR:=$(PKG_BUILD_DIR)


define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Microsoft PlayReady for Allwinner
  DEPENDS:=+libc
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	unzip -d $(PKG_BUILD_DIR) $(DL_DIR)/$(PKG_SOURCE)
	mv $(PKG_BUILD_DIR)/playready/* $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/description
Microsoft PlayReady for Allwinner
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(CP) $(PKG_BUILD_DIR)/lib/$(CONFIG_LIBC)/$(TARGET_ARCH)/* 		$(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/include/* 	$(1)/usr/include/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/drm
	$(INSTALL_DIR) $(1)/etc/playready

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/$(CONFIG_LIBC)/$(TARGET_ARCH)/* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/keys/*		$(1)/etc/playready/
endef
$(eval $(call BuildPackage,$(PKG_NAME)))

endif
