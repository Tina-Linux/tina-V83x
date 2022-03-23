include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=healthd
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/healthd/Default
  SECTION:=utils
  CATEGORY:=Allwinner
endef

define Package/healthd
  $(call Package/healthd/Default)
  TITLE:=Battery Daemon for tina linux
  DEPENDS:=+libstdcpp +libuci +liballwinner-base \
	+HEALTHD_BROADCAST_INFO:dbus \
	+HEALTHD_CHARGE_ANIMATION:libminui \
	+HEALTHD_CHARGE_ANIMATION:libpng
  MENU:=1
  DEFAULT:=1
endef

define Package/healthd/config
  source "$(SOURCE)/Config.in"
endef

define Package/healthd/description
	Tina Battery Daemon, refer to the Android Healthd
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./libbatinfo $(PKG_BUILD_DIR)/
	$(CP) ./src $(PKG_BUILD_DIR)/
	$(CP) ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/libbatinfo \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"  \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		BROADCAST_INFO=$(CONFIG_HEALTHD_BROADCAST_INFO) \
		CHARGE_ANIMATION=$(CONFIG_HEALTHD_CHARGE_ANIMATION) \
		all

	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"  \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		BROADCAST_INFO=$(CONFIG_HEALTHD_BROADCAST_INFO) \
		SHUTDOWN_CHARGER=$(CONFIG_HEALTHD_SHUTDOWN_CHARGER) \
		CHARGE_ANIMATION=$(CONFIG_HEALTHD_CHARGE_ANIMATION) \
		all
ifeq ($(CONFIG_HEALTHD_BROADCAST_DEMO), y)
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"  \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/batinfo
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/batinfo/* $(1)/usr/include/batinfo/
endef


define Package/healthd/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_BIN) ./files/healthd.init $(1)/etc/init.d/healthd
	$(INSTALL_CONF) ./files/healthd $(1)/etc/config/healthd
ifeq ($(CONFIG_HEALTHD_BROADCAST_INFO),y)
	$(INSTALL_DIR) $(1)/usr/share/dbus-1/system.d
	$(INSTALL_CONF) \
			./files/healthd.conf \
			$(1)/usr/share/dbus-1/system.d/healthd.conf
endif
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
ifeq ($(CONFIG_HEALTHD_BROADCAST_DEMO), y)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/healthd-demo $(1)/usr/bin
endif
endef

$(eval $(call BuildPackage,healthd))
