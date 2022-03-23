include $(TOPDIR)/rules.mk

PKG_NAME:=led_test
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:= +libstdcpp
  TITLE:=led array test
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/InstallDev
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR)  $(1)/etc/avs/ $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/$(PKG_NAME) $(1)/usr/bin
endef
$(eval $(call BuildPackage,$(PKG_NAME)))
