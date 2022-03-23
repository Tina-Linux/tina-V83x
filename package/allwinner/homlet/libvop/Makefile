include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=libvop
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

#TARGET_LDFLAGS += -lminigui_ths -lpthread -ljpeg -lpng -lm -lfreetype -lz -lpng12 -lfreetype -lpthread -lbz2 -lwifimg -lsmartlinkd_client

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
	SUBMENU:=Homlet
	CATEGORY:=Allwinner
	DEPENDS:=+libpthread +libstdcpp
	TITLE:=Allwinner Homlet Video Output Port control API library.
endef

define Package/$(PKG_NAME)/description
	Allwinner Homlet Video Output Port control API library.
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./libvop/* $(PKG_BUILD_DIR)/
endef

define Build/InstallDev
	echo Nothing for InstallDev
endef

define Build/Compile
	cd $(PKG_BUILD_DIR)/; \
	./bootstrap ; \
	./configure --prefix=$(PKG_INSTALL_DIR) --host=$(CONFIG_ARCH)-openwrt-linux; \
	make install ; \
	cd -
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/voptest $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
