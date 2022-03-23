include $(TOPDIR)/rules.mk

PKG_NAME:=logwrapper
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
	SECTION:=utils
	CATEGORY:=Allwinner
	DEPENDS:=+libpthread +libstdcpp
endef

define Package/$(PKG_NAME)
	$(call Package/$(PKG_NAME)/Default)
	TITLE:=logwrapper
endef

define Package/$(PKG_NAME)/description
	"redirecting stdout and stderr to the tina logging system"
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)" \
		all
endef

define Build/InstallDev
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
