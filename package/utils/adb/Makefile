include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=adb
PKG_VERSION:=0.0.1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)


include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=adb for Tina Linux
endef


define Package/$(PKG_NAME)/description
	it comes form Android 4.2!!
endef

define Package/libawadbauth
  SECTION:=libs
  CATEGORY:=Allwinner
  DEPENDS:= +libev
  TITLE:=adb auth lib for Tina Linux
endef

define Package/adbd_auth_service
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:= +libawadbauth
  TITLE:=adb auth service for Tina Linux
endef

PKG_CONFIG_DEPENDS += CONFIG_PACKAGE_libawadbauth \
			CONFIG_PACKAGE_adbd_auth_service

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
	$(CP) -r ./auth/ $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile/$(PKG_NAME)
	#compile libmincrypt.a (Android)
	$(MAKE) -C $(PKG_BUILD_DIR)/libs/libmincrypt \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)"

	#complie libcutils.a (Android)
	$(MAKE) -C $(PKG_BUILD_DIR)/libs/libcutils \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"

	#copy static lib
	$(CP) -r $(PKG_BUILD_DIR)/*/*/*.a $(PKG_BUILD_DIR)/

	#complie adbd (Android)
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"

endef

define Build/Compile/libawadbauth
	$(MAKE) -C $(PKG_BUILD_DIR)/auth lib\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Build/Compile/adbd_auth_service
	$(MAKE) -C $(PKG_BUILD_DIR)/auth service\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef

define Build/Compile
	$(call Build/Compile/$(PKG_NAME))
ifeq ($(CONFIG_PACKAGE_libawadbauth), y)
	$(call Build/Compile/libawadbauth)
endif
ifeq ($(CONFIG_PACKAGE_adbd_auth_service), y)
	$(call Build/Compile/adbd_auth_service)
endif
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_BIN) ./adb_shell $(1)/bin/
ifneq ($(CONFIG_PACKAGE_adbd_auth_service), y)
	$(INSTALL_DIR) $(1)/etc/init.d
	if [[ $(KERNEL_PATCHVER) == 4.* ]]; then \
	  $(INSTALL_BIN) ./adbd-configfs.init $(1)/etc/init.d/adbd; \
	else \
	  $(INSTALL_BIN) ./adbd.init $(1)/etc/init.d/adbd; \
	fi
endif
	$(INSTALL_DATA) ./adb_profile $(1)/etc/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/adbd $(1)/bin/

endef

define Package/libawadbauth/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/auth/*.so $(1)/usr/lib
endef

define Package/adbd_auth_service/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/auth/adbd_auth_service $(1)/bin/

	$(INSTALL_DIR) $(1)/etc/init.d
	if [[ $(KERNEL_PATCHVER) == 4.* ]]; then \
	  $(INSTALL_BIN) ./adbd-configfs.init $(1)/etc/init.d/adbd; \
	else \
	  $(INSTALL_BIN) ./adbd.init $(1)/etc/init.d/adbd; \
	fi
	sed -i "s/adbd/adbd_auth_service/g" $(1)/etc/init.d/adbd
endef


ifeq ($(CONFIG_PACKAGE_libawadbauth), y)
  Build/InstallDev = $(Build/InstallDev/libawadbauth)
endif
define Build/InstallDev/libawadbauth
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/auth/*.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/auth/include/*.h $(1)/usr/include
endef


$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,libawadbauth))
$(eval $(call BuildPackage,adbd_auth_service))
