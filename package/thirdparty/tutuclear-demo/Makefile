include $(TOPDIR)/rules.mk

PKG_NAME:=tutuclear-demo
PKG_VERSION:=1.0
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  TITLE:=GMEMS AEC and wake-up demo application
  DEPENDS:=+tconfigs +tutuclear-lib
endef

define Package/$(PKG_NAME)/description
  The demo applications for GMEMS AEC and wake-up libraries
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CXX="$(TARGET_CXX)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/tutuclear
	$(CP) \
		$(PKG_BUILD_DIR)/resources/tutuKeyword_*_xiaoaitongxue_*.raw \
		$(1)/etc/tutuclear/tutuKeyword.raw
	$(CP) $(PKG_BUILD_DIR)/resources/wakeup_hint.wav $(1)/etc/tutuclear
	$(CP) $(PKG_BUILD_DIR)/resources/background_music.wav $(1)/etc/tutuclear
	$(INSTALL_DIR) $(1)/usr/share/alsa/alsa.conf.d
	if [ -d "$(PKG_BUILD_DIR)/configs/$(TARGET_BOARD)" ]; then \
		$(CP) $(PKG_BUILD_DIR)/configs/$(TARGET_BOARD)/app_conf/* $(1)/etc/tutuclear; \
		$(CP) $(PKG_BUILD_DIR)/configs/$(TARGET_BOARD)/alsa_conf/* $(1)/usr/share/alsa/alsa.conf.d; \
	else \
		$(CP) $(PKG_BUILD_DIR)/configs/default/app_conf/* $(1)/etc/tutuclear; \
		$(CP) $(PKG_BUILD_DIR)/configs/default/alsa_conf/* $(1)/usr/share/alsa/alsa.conf.d; \
	fi
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tutuclear-demo-vad $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
