include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=tinarecorderdemo
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/tinarecorderdemo
  SUBMENU:=liballwinner_tina_demo
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=use awrecorder interface in liballwinner_tina
  DEPENDS:=+liballwinner \
	  +libuapi
endef

define Package/tinarecorderdemo/description
	CedarX2.6 recorder
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
		LDFLAGS="$(TARGET_LDFLAGS)" \
		TARGET_BOARD="$(TARGET_BOARD_PLATFORM)"
endef

define Package/tinarecorderdemo/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/tinarecorderdemo					 $(1)/usr/bin/
endef

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),r16 r18))
$(eval $(call BuildPackage,tinarecorderdemo))
endif
