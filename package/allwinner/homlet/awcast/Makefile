include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=awcast
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

TARGET_LDFLAGS += -lminigui_ths -lpthread -ljpeg -lpng -lm -lfreetype -lz -lpng12 -lfreetype -lpthread -lbz2 -lwifimg -lsmartlinkd_client

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
	SUBMENU:=Homlet
	SECTION:=gui
	CATEGORY:=Allwinner
	TITLE:=Awcast Main Program Demo (with MiniGUI)
	DEPENDS:=+libminigui-gpl +libpthread +libjpeg +libpng +libm +libfreetype +libz +libpng12 +libfreetype +libpthread +libbz2 +libwifimg +libsmartlinkd_client +wifimanager +smartlinkd
endef

define Package/$(PKG_NAME)/description
	Awcast Main Program Demo (with MiniGUI)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	echo Empty InstallDev 
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
	$(INSTALL_DIR) $(1)/usr/res
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) -r ./res/* $(1)/usr/res
	$(CP) -r ./conf/MiniGUI.cfg $(1)/usr/local/etc
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/awcast $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
