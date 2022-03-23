include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=libcamapi
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=camera api for Allwinner
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  camera api for allwinner
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_BUILD_DIR)/src/include/CameraInterface.h $(1)/usr/include/
	$(CP) $(PKG_BUILD_DIR)/src/libcamapi.so $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/etc

	$(CP) $(PKG_BUILD_DIR)/src/configs/* $(1)/etc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
