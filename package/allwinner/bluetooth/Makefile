include $(TOPDIR)/rules.mk

PKG_NAME:=bluetooth
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

export ARMGCC=$(TARGET_CC)

include $(BUILD_DIR)/package.mk
define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  SUBMENU:=bluetooth
endef

define Package/$(PKG_NAME)/description/Default
  Bluetooth app
endef

define Package/ampak
  $(Package/$(PKG_NAME)/Default)
  TITLE:=ampak bluetooth(bsa stack)
  DEPENDS:=+libpthread +librt +alsa-lib +resample +libuci +libstdcpp
endef

define Package/xradio
  $(Package/$(PKG_NAME)/Default)
  TITLE:=xradio bluetooth
  DEPENDS:=+common +kmod-net-xrbtlpm +kmod-net-xr829 +xr829-firmware
endef

define Package/realtek
  $(Package/$(PKG_NAME)/Default)
  TITLE:=realtek bluetooth
  DEPENDS:=+common +kmod-net-rtl8723ds +r8723ds-firmware
endef

define Package/common
  $(Package/$(PKG_NAME)/Default)
  TITLE:=common files
  DEPENDS:=+bluez-daemon +bluez-alsa +alsa-lib
endef

define Package/ampak/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Package/xradio/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Package/realtek/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Package/common/description
  $(call Package/$(PKG_NAME)/description/Default)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./common $(PKG_BUILD_DIR)/
	$(CP) ./ampak $(PKG_BUILD_DIR)/
	$(CP) ./xradio  $(PKG_BUILD_DIR)/
	$(CP) ./realtek $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef


ifeq ($(CONFIG_PACKAGE_ampak),y)
define Build/Compile/ampak
  #compile libbtapp
	$(MAKE) -C $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/build \
		-f Makefile \
		ENABLE_ALSA=TRUE \
		SYSTEM_INIT_PROCD="$(CONFIG_SYSTEM_INIT_PROCD)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		PKG_PREFIX="$(PKG_BUILD_DIR)" \
		STRIP="$(STRIP)" \
		CONFIG_LIBC="$(CONFIG_LIBC)" \
		all

	#compile bt_demo
	$(MAKE) -C $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test \
		-f Makefile \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
	$(STRIP) $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/bt_test
	$(STRIP) $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/bt_connect_disconnect_test
	$(STRIP) $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/bt_discovery_test

	#copy bsa_server and apps
	$(CP) -r $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/build/bsa_server $(PKG_BUILD_DIR)/
	$(CP) -r $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/bt_test $(PKG_BUILD_DIR)/
	$(CP) -r $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/bt_connect_disconnect_test $(PKG_BUILD_DIR)/
	$(CP) -r $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/bt_change_phone_vol $(PKG_BUILD_DIR)/
	$(CP) -r $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/bt_discovery_test $(PKG_BUILD_DIR)/
	$(CP) -r $(PKG_BUILD_DIR)/ampak/3rdparty/embedded/bsa_examples/linux/libbtapp/bt_test/ble_test $(PKG_BUILD_DIR)/

endef
endif

define Build/Compile
	$(Build/Compile/ampak)
	$(Build/Compile/xradio)
	$(Build/Compile/realtek)
endef

define Package/ampak/install
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/etc/bluetooth
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbsa.so $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbtapp.so $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bsa_server $(1)/usr/bin/
	$(INSTALL_BIN) ./ampak/btenable.sh $(1)/etc/bluetooth/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbsa.so $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbtapp.so $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bt_test $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bt_connect_disconnect_test $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bt_change_phone_vol $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bt_discovery_test $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/ble_test $(1)/usr/bin/
endef

define Package/xradio/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./xradio/hciattach_xr829.init $(1)/etc/init.d/hciattach
endef

define Package/common/install
	$(INSTALL_DIR) $(1)/etc/bluetooth
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) ./common/bt-init $(1)/etc/bluetooth/bt-init
	$(INSTALL_BIN) ./common/bt-a2dpsink-test $(1)/usr/bin
endef

define Package/realtek/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./realtek/hciattach_rtk.init $(1)/etc/init.d/hciattach
endef

$(eval $(call BuildPackage,common))
$(eval $(call BuildPackage,ampak))
$(eval $(call BuildPackage,xradio))
$(eval $(call BuildPackage,realtek))
