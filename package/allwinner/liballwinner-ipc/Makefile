include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=liballwinner-ipc
PKG_VERSION:=0.0.1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)


include $(BUILD_DIR)/package.mk

define Package/liballwinner-ipc
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=liballwinner ipc for tina parrot
  DEPENDS:=+libpthread +libstdcpp
endef


define Package/liballwinner-ipc/description
	liballwinner ipc for tina parrot
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" -Wall \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libipc.so $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/liballwinner-ipc/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libipc.so $(1)/usr/lib
endef



$(eval $(call BuildPackage,liballwinner-ipc))
