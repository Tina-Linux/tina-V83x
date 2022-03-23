include $(TOPDIR)/rules.mk

PKG_NAME:=rtk_hcidump
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  TITLE:=Realtek hcidump
  DEPENDS:=+glib2 +libc
endef

define Package/$(PKG_NAME)/description
  The realtek hcidump tool
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -rf ./hcidump/ $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/hcidump \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
