include $(TOPDIR)/rules.mk

PKG_NAME:=dlna-service
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base +liballwinner
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=libdlna and services for Tina
endef

define Package/$(PKG_NAME)/description
  libdlna and services for Tina
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./service $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile

	#compile libs and service
	$(MAKE) -C $(PKG_BUILD_DIR)/service \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/dlna
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/dlna/* $(1)/usr/include/dlna/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
endef

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),r16))
$(eval $(call BuildPackage,$(PKG_NAME)))
endif