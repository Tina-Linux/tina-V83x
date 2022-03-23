include $(TOPDIR)/rules.mk

PKG_NAME:=midea-uart
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Midea voice assistant demo
  DEPENDS:=+tinyalsa-lib +uvoice
endef

define Package/$(PKG_NAME)/description
	A test program for uart protcol
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
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
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/share
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/midea_uart $(1)/usr/bin/
	$(INSTALL_DATA) ./resources/*.wav $(1)/usr/share/
	$(INSTALL_BIN) ./midea.init $(1)/etc/init.d/midea
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
