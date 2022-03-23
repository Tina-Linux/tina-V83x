include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=misc-upgrade
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)


include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=read and write the misc partition
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
	$(INSTALL_BIN) ./aw_upgrade_image.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_reboot.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_upgrade_process.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_upgrade_log.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_upgrade_vendor_default.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_upgrade_utils.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_upgrade_normal.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_upgrade_lite.sh $(1)/sbin/
	$(INSTALL_BIN) ./aw_upgrade_plus.sh $(1)/sbin/

	$(INSTALL_BIN) ./aw_upgrade_autorun.init $(1)/etc/init.d/aw_upgrade_autorun
ifdef CONFIG_SUNXI_SMALL_STORAGE_OTA
	$(INSTALL_BIN) ./aw_fstab.init $(1)/etc/init.d/aw_fstab
endif
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/* $(1)/sbin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
