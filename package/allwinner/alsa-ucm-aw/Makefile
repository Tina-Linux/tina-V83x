include $(TOPDIR)/rules.mk

PKG_NAME:=alsa-ucm-aw
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/alsa-ucm-aw/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  SUBMENU:=ALSA UCM for Allwinner
endef

define Package/alsa-ucm-aw/description/Default
  The ALSA UCM (Use Case Manager) works by configuring the sound card ALSA
  kcontrols to change the hardware digital and analog audio routing to match
  the requested device use case.
endef

define Package/alsa-ucm-aw-configs
  $(call Package/alsa-ucm-aw/Default)
  TITLE:=Allwinner ALSA UCM configurations
endef

define Package/alsa-ucm-aw-configs/description
  $(call Package/alsa-ucm-aw/description/Default)
endef

define Package/alsa-ucm-aw-lib
  $(call Package/alsa-ucm-aw/Default)
  TITLE:=A library to use Allwinner ALSA UCM configurations
  DEPENDS:=+alsa-lib +alsa-ucm-aw-configs
endef

define Package/alsa-ucm-aw-lib/description
  $(call Package/alsa-ucm-aw/description/Default)
  This library provides upper APIs for Allwinner ALSA UCM users.
endef

define Package/alsa-ucm-aw-utils
  $(call Package/alsa-ucm-aw/Default)
  TITLE:=Utilities to use Allwinner ALSA UCM configurations
  DEPENDS:=+alsa-ucm-aw-lib
endef

define Package/alsa-ucm-aw-utils/description
  $(call Package/alsa-ucm-aw/description/Default)
  This package provides utilities for Allwinner ALSA UCM users.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/lib \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
	$(MAKE) -C $(PKG_BUILD_DIR)/utils \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS) -I$(PKG_BUILD_DIR)/lib" \
		LDFLAGS="$(TARGET_LDFLAGS) -L$(PKG_BUILD_DIR)/lib" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/alsa-ucm-aw-configs/install
	$(INSTALL_DIR) $(1)/usr/share/alsa/ucm
	./scripts/cp_ucm_configs_by_target.sh $(TARGET_BOARD) ./ucm_configs \
		$(1)/usr/share/alsa/ucm
endef

define Package/alsa-ucm-aw-lib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/alsa-ucm-aw-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/alsa-ucm-aw $(1)/usr/bin/
endef

$(eval $(call BuildPackage,alsa-ucm-aw-configs))
$(eval $(call BuildPackage,alsa-ucm-aw-lib))
$(eval $(call BuildPackage,alsa-ucm-aw-utils))
