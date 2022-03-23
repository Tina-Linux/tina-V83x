include $(TOPDIR)/rules.mk

PKG_NAME:=btmanager
PKG_VERSION:=2.1.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

#BT_EXT_CFLAG +=-DDEBUG_LOG_LEVEL=2

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  SUBMENU:=btmanager
endef

define Package/$(PKG_NAME)-core
  $(call Package/btmanager/Default)
  TITLE:=bluetooth manager core
  DEPENDS:=+alsa-lib +bluez-daemon +glib2 +sbc
endef

define Package/$(PKG_NAME)-demo
  $(call Package/btmanager/Default)
  TITLE:=Tina btmanager app demo
  DEPENDS := +btmanager-core
endef

define Package/$(PKG_NAME)-core/description
	bluetooth manager
endef

define Package/$(PKG_NAME)-demo/description
	Tina btmanager app demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread -lz -lrt -lm)

define Build/Configure
endef
ifeq ($(CONFIG_PACKAGE_btmanager-core),y)
define Build/Compile/$(PKG_NAME)-core
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		BT_EXT_CFLAG="$(BT_EXT_CFLAG)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		STAGING_DIR="$(STAGING_DIR)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		CONFIG_LIBC="$(CONFIG_LIBC)" \
		all
	$(MAKE) -C $(PKG_BUILD_DIR)/src/gatt_server \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		BT_EXT_CFLAG="$(BT_EXT_CFLAG)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		STAGING_DIR="$(STAGING_DIR)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef
endif

ifeq ($(CONFIG_PACKAGE_btmanager-demo),y)
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

define Build/Compile
	$(Build/Compile/$(PKG_NAME)-core)
	$(Build/Compile/$(PKG_NAME)-demo)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbttool.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbtmg.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libgatt-server.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbluetooth-internal.a $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libshared-mainloop.a $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef


define Package/$(PKG_NAME)-core/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/etc/bluetooth
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbttool.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libbtmg.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libgatt-server.so $(1)/lib
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aw_bt_test $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bt_test $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/aw_gatt_server_test $(1)/usr/bin/
endef
$(eval $(call BuildPackage,$(PKG_NAME)-core))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
