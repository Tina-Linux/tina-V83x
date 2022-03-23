include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=recovery
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/recovery/Default
  SECTION:=utils
  CATEGORY:=Allwinner
endef

define Package/recovery
  $(call Package/recovery/Default)
  TITLE:=recovery ui tina linux
  DEPENDS:=+libstdcpp +libminui +libpng
  DEFAULT:=1
endef

define Package/recovery/description
	Tina recovery ui, refer to the Android 8.1
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

TARGET_CFLAGS += -fpermissive

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"  \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/recovery/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/usr/share/system/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/bin/
	$(CP) $(PKG_BUILD_DIR)/src/images/* $(1)/usr/share/system/
endef

$(eval $(call BuildPackage,recovery))
