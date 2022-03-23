include $(TOPDIR)/rules.mk

PKG_NAME:=avs-fetch-device-sn
PKG_VERSION:=1.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  SUBMENU:=AVS
  TITLE:=A tool to fetch device serial number for AVS
  DEPENDS:= +libstdcpp +libjson-c
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./files/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
