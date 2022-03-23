include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/version.mk

PKG_NAME:=atmel
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile
endef

define Package/atmel_mxt224s-config
  SECTION:=firmware
  CATEGORY:=Firmware
  DEPENDS:= +kmod-touchscreen-atmel-mxt
  TITLE:= Atmel mxt224s config
endef

define Build/prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/compile

endef

define Package/atmel_mxt224s-config/install
	$(INSTALL_DIR) $(1)/lib/firmware
	$(INSTALL_DATA) ./mxt224s.cfg $(1)/lib/firmware/
endef

$(eval $(call BuildPackage,atmel_mxt224s-config))
