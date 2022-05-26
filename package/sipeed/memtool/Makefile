include $(TOPDIR)/rules.mk

PKG_NAME:=sipeed_memtool
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Sipeed
  TITLE:=Sipeed tools memtool
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/memtool $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
