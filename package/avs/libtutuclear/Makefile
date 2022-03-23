include $(TOPDIR)/rules.mk

PKG_NAME:=libtutuclear
PKG_VERSION:=16b
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=avs
  CATEGORY:=avs
  DEPENDS:= +libstdcpp +libsensory
  TITLE:=tutu clear, audio front engine
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./files/src/* $(PKG_BUILD_DIR)
	$(CP) ./files/lib/$(CONFIG_ARCH)/libtutuClear_$(PKG_VERSION)*.a $(PKG_BUILD_DIR)/libtutuClear.a
	$(INSTALL_DATA) ./files/header/*.h $(PKG_BUILD_DIR)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/ $(1)/usr/include/tutu
	$(INSTALL_DATA) ./files/header/*.h $(1)/usr/include/tutu/
	$(INSTALL_DATA) ./files/lib/$(CONFIG_ARCH)/libtutuClear_$(PKG_VERSION)*.a $(1)/usr/lib/libtutuClear.a
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libtutuClearWrapper.so $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR)  $(1)/etc/avs/ $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_DATA) ./files/prm/*.prm $(1)/etc/avs/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/tutuclear-test $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libtutuClearWrapper.so $(1)/usr/lib/
endef
#$(eval $(call BuildPackage,$(PKG_NAME)))
