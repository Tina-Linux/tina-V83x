include $(TOPDIR)/rules.mk

PKG_NAME:=peq
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/peq/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Tina peq manager
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base +libopenssl +iw +alsa-utils
endef

define Package/peq
  $(call Package/peq/Default)
  MENU:=1
  DEFAULT:=1
endef

define Package/peq-demo
  $(call Package/peq/Default)
  TITLE:=Tina peq app demo
  DEPENDS := peq +peq
endef

define Package/peq/description
	Tina peq manager
endef

define Package/peq-demo/description
	Tina peq app demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	#libpeq.so
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

	#peq_test
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

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libpeq.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/peq/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libpeq.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/peq-demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/peq_test $(1)/bin
endef

$(eval $(call BuildPackage,peq))
$(eval $(call BuildPackage,peq-demo))
