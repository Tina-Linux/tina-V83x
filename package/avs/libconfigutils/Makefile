include $(TOPDIR)/rules.mk

PKG_NAME:=libconfigutils
PKG_VERSION:=1
PKG_RELEASE:=1
PKG_JOBS := -j32
include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=avs
  CATEGORY:=avs
  DEPENDS:= +libjson-c +libstdcpp +libev +avs-sdk +libugpio +resample +libtutuclear
  TITLE:=ConfigUtils for All
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./files/src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/ $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libConfigUtils.so $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/avs/ $(1)/usr/lib $(1)/usr/bin/
	$(INSTALL_DATA) ./files/config.json $(1)/etc/avs/config.json
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libConfigUtils.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

#$(eval $(call BuildPackage,$(PKG_NAME)))
