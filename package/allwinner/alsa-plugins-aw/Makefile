include $(TOPDIR)/rules.mk

PKG_NAME:=alsa-plugins-aw
PKG_VERSION:=1.0.2
PKG_RELEASE:=1

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_CHECK_FORMAT_SECURITY:=0

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=ALSA plugins from Allwinner
  DEPENDS:=+alsa-lib
endef

define Package/$(PKG_NAME)/config
	menu "Customize plugins"
		depends on PACKAGE_alsa-plugins-aw

	config ALSA_PLUGINS_AW_AWRATE
		bool "awrate"
		default y
	config ALSA_PLUGINS_AW_AWEQUAL
		bool "awequal"
		default y

	endmenu
endef

CONFIGURE_ARGS += \
		--enable-awrate=$(if $(filter $(CONFIG_ALSA_PLUGINS_AW_AWRATE),y),yes,no) \
		--enable-awequal=$(if $(filter $(CONFIG_ALSA_PLUGINS_AW_AWEQUAL),y),yes,no)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)
endef

define Build/InstallDev
	if [ -d "$(PKG_INSTALL_DIR)/usr/lib" ]; then \
		$(INSTALL_DIR) $(1)/usr/lib/; \
		$(CP) $(PKG_INSTALL_DIR)/usr/lib/ $(1)/usr/lib/; \
	fi
endef

define Package/$(PKG_NAME)/install
	if [ -d "$(PKG_INSTALL_DIR)/usr/lib/alsa-lib" ]; then \
		$(INSTALL_DIR) $(1)/usr/lib/alsa-lib; \
		$(CP) \
			$(PKG_INSTALL_DIR)/usr/lib/alsa-lib/libasound_module*.so \
			$(1)/usr/lib/alsa-lib; \
	fi
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
