include $(TOPDIR)/rules.mk

PKG_NAME:=libsocket_db
PKG_VERSION:=0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=library for socket_db
  DEPENDS:=+adb +libxml2
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/description
Library for socket_db which base on adb.
Devices can use it to communicate with PC which is running DragonMAT.
If you are no sure, please say N.
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(CP) $(PKG_BUILD_DIR)/src/*.so $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/init_entry.h $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/src/xml_packet.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/*.so $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
