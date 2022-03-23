include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME := mtdbadblk
PKG_VERSION := 0.0.1
PKG_RELEASE := 1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION := utils
  CATEGORY := Allwinner
  TITLE := get bad block infomation of mtd block device
  DEPENDS :=
endef

define Package/$(PKG_NAME)/description
The tools to search bad block infomation of mtd block device.
endef

define Build/Prepare
	@$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	@$(CP) ./src $(PKG_BUILD_DIR)
endef

define Build/Compile
	@$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		BUILD_DIR="$(PKG_BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/mtdbadblk $(1)/sbin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
