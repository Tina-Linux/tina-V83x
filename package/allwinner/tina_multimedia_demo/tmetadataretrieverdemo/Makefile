include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=tmetadataretrieverdemo
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/config
	comment "$(PKG_NAME) should not be used together with ONLY_ENABLE_AUDIO in libcedarx"
	depends on ONLY_ENABLE_AUDIO
endef

define Package/$(PKG_NAME)
  SUBMENU:=tina_multimedia_demo
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=use tmetadataretriever interface
  DEPENDS:=+libcedarx
endef

define Package/$(PKG_NAME)/description
  CedarX2.8 tmetadataretrieverdemo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)"
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tmetadataretrieverdemo					 $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
