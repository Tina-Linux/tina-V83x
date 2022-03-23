include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=tina-app-upgrade
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=upgrade app partition
  DEPENDS:=+uboot-envtools
endef

define Package/$(PKG_NAME)/description
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) ./aw_upgrade_dual_app.sh $(1)/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
