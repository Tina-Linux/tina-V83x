include $(TOPDIR)/rules.mk

PKG_NAME:=brcm_patchram_plus
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/brcm_patchram_plus
SECTION:=utils
CATEGORY:=Utilities
TITLE:=brcm_patchram --bluetooth tool
endef

define Package/brcm_patchram_plus/description
It's my first package demo.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/brcm_patchram_plus/install
	$(INSTALL_DIR) $(1)/usr/bin
	#$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/brcm_patchram_plus $(1)/usr/bin/
	#$(INSTALL_BIN) ./bt.init $(1)/etc/init.d/bt
endef

$(eval $(call BuildPackage,brcm_patchram_plus))
