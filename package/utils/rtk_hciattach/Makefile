include $(TOPDIR)/rules.mk

PKG_NAME:=rtk_hciattach
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)


include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=rtk_hciattach
endef

define Package/$(PKG_NAME)/description/Default
    HCI UART initialization tool for realtek BT module
endef

define Package/rtk_hciattach
  $(Package/$(PKG_NAME)/Default)
  TITLE:=Realtek BT HCI UART initialization tools
  DEPENDS:=+libcutils +liblog +libc +bluez-utils +brcm_patchram_plus
endef


define Package/rtk_hciattach/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
        $(MAKE) -C $(PKG_BUILD_DIR)/src/ \
        ARCH="$(TARGET_ARCH)" \
        AR="$(TARGET_AR)" \
        CC="$(TARGET_CC)" \
        CXX="$(TARGET_CXX)" \
        CFLAGS="$(TARGET_CFLAGS)" \
        LDFLAGS="$(TARGET_LDFLAGS)" \
        CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
        all
endef

define Package/rtk_hciattach/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/rtk_hciattach $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,rtk_hciattach))
