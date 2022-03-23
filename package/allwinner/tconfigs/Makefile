include $(TOPDIR)/rules.mk

PKG_NAME:=tconfigs
PKG_VERSION:=1.0.0
PKG_RELEASE:=1
PKG_JOBS:=-j32
include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Tina Config Utils
  DEPENDS:= +libstdcpp +rapidjson +alsa-lib +libev \
		+PACKAGE_tutuclear-lib:tutuclear-lib
  DEPENDS+= +googletest
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./files/* $(PKG_BUILD_DIR)/
endef

CMAKE_OPTIONS += \
		-DBUILD_UNIT_TEST=ON \
		-DTCONFIGS_TUTUCLEAR=$(if $(filter $(CONFIG_PACKAGE_tutuclear-lib),y),ON,OFF)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	if [ -d "$(PKG_INSTALL_DIR)/usr/etc" ]; then \
		$(INSTALL_DIR) $(1)/etc; \
		$(CP) $(PKG_INSTALL_DIR)/usr/etc/* $(1)/etc; \
	fi
	if [ -d "$(PKG_INSTALL_DIR)/usr/bin" ]; then \
		$(INSTALL_DIR) $(1)/usr/bin; \
		$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin; \
	fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
