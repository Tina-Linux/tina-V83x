include $(TOPDIR)/rules.mk

PKG_NAME:=wifimanager
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Tina wifimanager
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base
endef

define Package/$(PKG_NAME)
  $(call Package/wifimanager/Default)
  MENU:=1
  DEFAULT:=1
endef

define Package/$(PKG_NAME)-demo
  $(call Package/wifimanager/Default)
  TITLE:=Tina wifimanager app demo
  DEPENDS := wifimanager +wifimanager
endef

define Package/$(PKG_NAME)-daemon-demo
  $(call Package/wifimanager/Default)
  TITLE:=Tina wifimanager daemon demo
  DEPENDS := wifimanager +wifimanager
endef

PKG_CONFIG_DEPENDS += CONFIG_WIFIMANAGER_ENABLE_DAEMON

define Package/$(PKG_NAME)/description
	Tina wifi manager
endef

define Package/$(PKG_NAME)/config
config WIFIMANAGER_ENABLE_DAEMON
	bool "Enable wifimanager daemon support"
	depends on PACKAGE_wifimanager
	default n
endef

define Package/$(PKG_NAME)-demo/description
	Tina wifimanager app demo
endef

define Package/$(PKG_NAME)-daemon-demo/description
	Tina wifimanager daemon demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
	$(CP) -r ./daemon-demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)

define Build/Compile/$(PKG_NAME)
	#libwifimg.so
	$(MAKE) -C $(PKG_BUILD_DIR)/src/core/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

ifeq ($(CONFIG_WIFIMANAGER_ENABLE_DAEMON), y)
	$(MAKE) -C $(PKG_BUILD_DIR)/src/daemon/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endif
endef

ifeq ($(CONFIG_PACKAGE_wifimanager-demo),y)
define Build/Compile/$(PKG_NAME)-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_wifimanager-daemon-demo),y)
define Build/Compile/$(PKG_NAME)-daemon-demo

	$(MAKE) -C $(PKG_BUILD_DIR)/daemon-demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef
endif

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
	$(Build/Compile/$(PKG_NAME)-demo)
	$(Build/Compile/$(PKG_NAME)-daemon-demo)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifimg.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
ifeq ($(CONFIG_WIFIMANAGER_ENABLE_DAEMON), y)
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifid.so $(1)/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_daemon $(1)/bin
	$(INSTALL_BIN) ./files/wifi_daemon.init $(1)/etc/init.d/wifi_daemon
endif
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_DIR) $(1)/etc/init.d

	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifimg.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DATA) ./files/wpa_supplicant.conf $(1)/etc/wifi
	$(INSTALL_DATA) ./files/wpa_supplicant_src.conf $(1)/etc/wifi
	$(INSTALL_DATA) ./files/wpa_supplicant_overlay.conf $(1)/etc/wifi
	$(INSTALL_BIN) ./files/wpa_supplicant.init $(1)/etc/init.d/wpa_supplicant
	$(INSTALL_BIN) ./files/udhcpc_wlan0.init $(1)/etc/wifi/udhcpc_wlan0
ifeq ($(CONFIG_WIFIMANAGER_ENABLE_DAEMON), y)
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libwifid.so $(1)/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_daemon $(1)/bin
	$(INSTALL_BIN) ./files/wifi_daemon.init $(1)/etc/init.d/wifi_daemon
endif
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_connect_ap_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_on_off_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_connect_chinese_ap_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_scan_results_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_list_networks_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_remove_network_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_connect_ap_with_netid_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_get_netid_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_longtime_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_remove_all_networks_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_wps_pbc_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_longtime_scan_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_disconnect_ap_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_reconnect_ap_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifi_get_connection_info_test $(1)/bin
endef

define Package/$(PKG_NAME)-daemon-demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/wifid $(1)/bin
endef
$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
$(eval $(call BuildPackage,$(PKG_NAME)-daemon-demo))
