include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=hpplay-receiver
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
	SUBMENU:=Homlet
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=HappyCast for Linux
	DEPENDS:=+libcedarx
endef

define Package/$(PKG_NAME)/description
	HappyCast for Linux
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
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin

	$(LN) ld-2.21.so $(1)/lib/ld-linux.so.3

	$(LN) libVE.so $(1)/usr/lib/libcdc_ve.so
	$(LN) libvideoengine.so $(1)/usr/lib/libcdc_videoengine.so
	$(LN) libMemAdapter.so $(1)/usr/lib/libcdc_memory.so
	$(LN) libvdecoder.so $(1)/usr/lib/libcdc_vdecoder.so

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/mdnsd_lebo $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/dlna_app $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/demo_player $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/*.sh $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
