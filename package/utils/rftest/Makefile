include $(TOPDIR)/rules.mk

PKG_NAME:=rftest
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Utilities
  SUBMENU:=rf test tool
endef

define Package/$(PKG_NAME)/description/Default
	RF test tools
endef

define Package/broadcom-rftest
  $(Package/$(PKG_NAME)/Default)
  TITLE:=broadcom rf test tools
  DEPENDS:=+libcutils +liblog +libc +bluez-utils +brcm_patchram_plus
endef

define Package/xr819-rftest
  $(Package/$(PKG_NAME)/Default)
  TITLE:=xr819 rf test tools
endef

define Package/xr829-rftest
  $(Package/$(PKG_NAME)/Default)
  TITLE:=xr829 rf test tools
endef

define Package/realtek-rftest
  $(Package/$(PKG_NAME)/Default)
  TITLE:=realtek rf test tools
  DEPENDS:=+libcutils +liblog +libc +libstdcpp
endef

define Package/broadcom-rftest/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Package/xr819-rftest/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Package/xr829-rftest/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Package/realtek-rftest/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./ $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
        $(MAKE) -C $(PKG_BUILD_DIR)/realtek/ \
        ARCH="$(TARGET_ARCH)" \
        AR="$(TARGET_AR)" \
        CC="$(TARGET_CC)" \
        CXX="$(TARGET_CXX)" \
        CFLAGS="$(TARGET_CFLAGS)" \
        LDFLAGS="$(TARGET_LDFLAGS)" \
        CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
        all
endef

define Package/broadcom-rftest/install
	$(INSTALL_DIR) $(1)/etc/rftest
ifeq ($(findstring arm,$(TARGET_CC)), arm)
	$(INSTALL_BIN) ./broadcom/wl $(1)/etc/rftest/wl
else
ifeq ($(findstring aarch64, $(TARGET_CC)), aarch64)
	$(INSTALL_BIN) ./broadcom/wl_64 $(1)/etc/rftest/wl
endif
endif
	$(INSTALL_BIN) ./broadcom/*.bin $(1)/etc/rftest/
	$(INSTALL_BIN) ./broadcom/wifi_test.sh $(1)/etc/rftest/
	$(INSTALL_BIN) ./broadcom/bt_test.sh $(1)/etc/rftest/
endef

define Package/xr819-rftest/install
	$(INSTALL_DIR) /$(1)/usr/bin
ifeq ($(TARGET_CPU_VARIANT), arm926ej-s)
	$(INSTALL_BIN) ./xr819/$(TARGET_CPU_VARIANT)/$(CONFIG_LIBC)/etf /$(1)/usr/bin
else
	$(INSTALL_BIN) ./xr819/$(CONFIG_ARCH)/$(CONFIG_LIBC)/etf /$(1)/usr/bin
endif
endef

define Package/xr829-rftest/install
	$(INSTALL_DIR) /$(1)/usr/bin
	$(INSTALL_BIN) ./xr829/btetf /$(1)/usr/bin
	$(INSTALL_BIN) ./xr829/etf /$(1)/usr/bin
endef

define Package/realtek-rftest/install
	$(INSTALL_DIR) /$(1)/usr/sbin
	$(INSTALL_DIR) /$(1)/lib/firmware
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/rtwpriv $(1)/usr/sbin/rtwpriv
	$(INSTALL_BIN) ./realtek/BT/rtlbtmp $(1)/usr/sbin/rtlbtmp
	$(INSTALL_DATA) ./realtek/BT/mp_rtl8723d_config_h5 $(1)/lib/firmware/mp_rtl8723d_config_h5
	$(INSTALL_DATA) ./realtek/BT/mp_rtl8723d_config_h4 $(1)/lib/firmware/mp_rtl8723d_config_h4
	$(INSTALL_DATA) ./realtek/BT/mp_rtl8723d_fw $(1)/lib/firmware/mp_rtl8723d_fw
endef

$(eval $(call BuildPackage,broadcom-rftest))
$(eval $(call BuildPackage,xr819-rftest))
$(eval $(call BuildPackage,xr829-rftest))
$(eval $(call BuildPackage,realtek-rftest))
