include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=p2p_utils
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

#TARGET_LDFLAGS += -lminigui_ths -lpthread -ljpeg -lpng -lm -lfreetype -lz -lpng12 -lfreetype -lpthread -lbz2 -lwifimg -lsmartlinkd_client

include $(BUILD_DIR)/package.mk

define Package/p2p_group_owner
	SUBMENU:=Homlet
	CATEGORY:=Allwinner
	DEPENDS:=+libpthread +libstdcpp
	TITLE:=Wifi P2P Group Owner 
endef

define Package/p2p_group_owner/description
	Wifi P2P Group Owner Test Utils
endef

define Package/p2p_group_owner/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/p2p_go $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/files/p2p_demo_usage_new_go/go_wfd_start.sh $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/src/files/p2p_demo_usage_new_go/p2p_supplicant_go.conf $(1)/etc/p2p_supplicant_go.conf
endef

define Package/p2p_group_client
	SUBMENU:=Homlet
	CATEGORY:=Allwinner
	DEPENDS:=+libpthread +libstdcpp
	TITLE:=Wifi P2P Group Client 
endef

define Package/p2p_group_client/description
	Wifi P2P Group Owner Test Utils
endef

define Package/p2p_group_client/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/p2p_gc $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/files/p2p_demo_usage_new_gc/gc_wfd_start.sh $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/src/files/p2p_demo_usage_new_gc/p2p_supplicant_gc.conf $(1)/etc/p2p_supplicant_gc.conf
endef



define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/src
	$(CP) -r ./group_owner $(PKG_BUILD_DIR)/src/group_owner
	$(CP) -r ./group_client $(PKG_BUILD_DIR)/src/group_client
	$(CP) -r ./libs $(PKG_BUILD_DIR)/src/libs
	$(CP) -r ./files $(PKG_BUILD_DIR)/src/files
endef

define Build/InstallDev
	echo Nothing for InstallDev
endef

define Build/Compile
	cd $(PKG_BUILD_DIR)/; \
	$(MAKE) -C $(PKG_BUILD_DIR)/src/group_owner \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all; \
	cd -

	cd $(PKG_BUILD_DIR)/; \
	$(MAKE) -C $(PKG_BUILD_DIR)/src/group_client \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all; \
	cd -

endef



$(eval $(call BuildPackage,p2p_group_owner))
$(eval $(call BuildPackage,p2p_group_client))
