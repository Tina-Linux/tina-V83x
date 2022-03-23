#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=test_r16_record_play
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/test_r16_record_play
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=record play auto testing tool
  DEPENDS:=@DISPLAY_SUPPORT
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -Wall" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/test_r16_record_play/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/media
	$(INSTALL_DATA) ./media/*.pcm $(1)/media/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/test_record_play $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,test_r16_record_play))
