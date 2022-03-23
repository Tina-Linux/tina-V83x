include $(TOPDIR)/rules.mk

PKG_NAME:=alsa-conf-aw
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/alsa-conf-aw
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=ALSA Configuration Files for Allwinner
  DEPENDS:= +alsa-lib
endef

define Package/alsa-conf-aw/description
  ALSA configuration files for Allwinner R series Soc. This files contain some
useful pcm devices, such as playback with speaker, playback with headphone,
Loopback and so on.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./files/ $(PKG_BUILD_DIR)/
endef

define Build/Compile
endef

define Package/alsa-conf-aw/install
	$(INSTALL_DIR) $(1)/etc/
	if [ -e ./files/$(TARGET_PLATFORM)/$(TARGET_BOARD)/asound.conf ]; then \
		$(INSTALL_DATA) ./files/$(TARGET_PLATFORM)/$(TARGET_BOARD)/asound.conf $(1)/etc/; \
	else \
		$(INSTALL_DATA) ./files/asound.conf $(1)/etc/; \
	fi
endef

$(eval $(call BuildPackage,alsa-conf-aw))
