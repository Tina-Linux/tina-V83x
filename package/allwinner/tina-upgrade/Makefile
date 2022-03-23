include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=tina-upgrade
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)


include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=upgrade tina
endef

define Package/$(PKG_NAME)/config
  source "$(SOURCE)/Config.in"
endef

define Package/$(PKG_NAME)/description
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) tools $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/tools \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./tina-upgrade $(1)/sbin/
	$(INSTALL_BIN) ./aw_ota_prepare_img.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_ota_verify_img.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_ota_apply_img.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_ota_utils.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_ota_autorun.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_auto_connect_wifi.sh $(1)/sbin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/* $(1)/sbin

	$(INSTALL_BIN) ./aw_ota_autorun.init $(1)/etc/init.d/aw_ota_autorun
ifeq ($(CONFIG_TINA_UPGRADE_INSTALL_PUB_KEY),y)
	$(INSTALL_BIN) ./OTA_Key_pub.pem $(1)/
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
