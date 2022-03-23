include $(TOPDIR)/rules.mk

PKG_NAME:=tsc_demo
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/tsc_demo

include $(BUILD_DIR)/package.mk

define Package/tsc_demo
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=tsc test demo
endef

define Package/tsc_demo/description
	tsc_test_demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./tsc_test/ $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/tsc_test \
	ARCH="$(TARGET_ARCH)" \
	AR="$(TARGET_AR)" \
	CC="$(TARGET_CC)" \
	CXX="$(TARGET_CXX)" \
	CFLAGS="$(TARGET_CFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Package/tsc_demo/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tsc_test/lib/libtsc.so			$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tsc_test/lib/libMemAdapter.so	$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tsc_test/lib/libtsdemux.so		$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tsc_test/lib/tscTest			$(1)/usr/bin/
endef

$(eval $(call BuildPackage,tsc_demo))
