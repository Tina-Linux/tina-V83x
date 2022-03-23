include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=evevehicledetect
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
LIBS_PATH :=

ifeq ($(CONFIG_LIBC),"musl")
	LIBS_PATH = lib/openwrt-arm-musl
endif
ifeq ($(CONFIG_LIBC),"glibc")
	LIBS_PATH = lib/openwrt-arm-glibc
endif

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=evevehicledetect
  DEPENDS:=+libstdcpp +libcamapi +libAWIspApi +opencv +libuapi
endef

define Package/$(PKG_NAME)/description
	Eve test
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
		LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/$(LIBS_PATH)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/eve/

	$(INSTALL_DATA) $(PKG_BUILD_DIR)/classifier/*		$(1)/etc/eve/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(LIBS_PATH)/*.so 	$(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(PKG_NAME)			$(1)/usr/bin/
endef

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),t7))
$(eval $(call BuildPackage,$(PKG_NAME)))
endif
