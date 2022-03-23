include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=player-utils
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
	SUBMENU:=Homlet
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=CedarX Player Utils for Linux
	DEPENDS:=+libcedarx +libuci
endef

define Package/$(PKG_NAME)/description
	CedarX Player for Linux
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
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/config

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/etc/config/* $(1)/etc/config
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
