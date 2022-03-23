include $(TOPDIR)/rules.mk

PKG_NAME:=cortana-demo
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Cortana
  TITLE:=Microsoft Cortana Demo Application
  DEPENDS:=@EXT_TOOLCHAIN_LIBC_USE_GLIBC +cortana-sdk
endef

define Package/$(PKG_NAME)/description
  The demo application of Microsoft Cortana
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

ifeq ($(CONFIG_CORTANA_SDK_ENABLE_HARNESS),y)
    LIBS += -lharness-client -lharness-utility -lbreakpad-utility
endif

ifeq ($(CONFIG_CORTANA_SDK_ENABLE_CDP),y)
    LIBS += -lCDP_host
endif

ifeq ($(CONFIG_CORTANA_SDK_ENABLE_SKYPE),y)
    LIBS += -lskype_call
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LIBS="$(LIBS)" \
		INSTALL_DIR="$(PKG_INSTALL_DIR)/usr/bin" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
