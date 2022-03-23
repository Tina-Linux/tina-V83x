include $(TOPDIR)/rules.mk

PKG_NAME:=libsensory
PKG_VERSION:=5.0.0
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS:= -DARCH=$(CONFIG_ARCH)

define Package/$(PKG_NAME)
  SECTION:=avs
  CATEGORY:=avs
  DEPENDS:= +alsa-lib +libstdcpp
  TITLE:=Sensory keyword detect lib
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./files/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/ $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/snsr.h $(1)/usr/include/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libsensory.so $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/avs/ $(1)/usr/lib $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/models/*.snsr $(1)/etc/avs/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libsensory.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sensory-test $(1)/usr/bin/
endef

#$(eval $(call BuildPackage,$(PKG_NAME)))
