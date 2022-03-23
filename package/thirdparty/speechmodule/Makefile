include $(TOPDIR)/rules.mk

PKG_NAME:=speechmodule
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
TARGET_LDFLAGS  += -L$(STAGING_DIR)/rootfs/mnt/app/libs

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  TITLE:=midea speechmodule based on duilite
  DEPENDS:=+libstdcpp +alsa-lib +midea-duilite-lib +midea-player-lib  +midea-mspeech-lib +libopenssl
endef

define Package/$(PKG_NAME)/description
	midea speechmodule
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./files/* $(PKG_BUILD_DIR)/
	$(CP) -r ./demo/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/mnt/app/AC $(1)/mnt/app/cfg $(1)/mnt/app/libs/ \
		$(1)/mnt/app/lua/ $(1)/mnt/app/res/ $(1)/mnt/app/tts/ $(1)/mnt/app/demo/
	$(INSTALL_DIR) $(1)/etc/wifi

	$(INSTALL_DATA) $(PKG_BUILD_DIR)/oem/AC/* $(1)/mnt/app/AC
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/oem/cfg/* $(1)/mnt/app/cfg
	#$(INSTALL_DATA) $(PKG_BUILD_DIR)/oem/libs/* $(1)/mnt/app/libs
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/oem/lua/* $(1)/mnt/app/lua
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/oem/res/* $(1)/mnt/app/res
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/oem/tts/* $(1)/mnt/app/tts

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oem/deamon $(1)/mnt/app/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oem/msmart_bt_config $(1)/mnt/app/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oem/msmart_local_net $(1)/mnt/app/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oem/msmart_net $(1)/mnt/app/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oem/msmart_uart $(1)/mnt/app/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/oem/mspeech $(1)/mnt/app/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/speechdemo $(1)/mnt/app/demo
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/speechdemo_readme $(1)/mnt/app/demo

	# note
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/xr_wifi.conf $(1)/etc/wifi/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/F0C9D1B3170E.bin $(1)/mnt/app/demo
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
