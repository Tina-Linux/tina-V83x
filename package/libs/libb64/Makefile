include $(TOPDIR)/rules.mk

PKG_NAME:=libb64
PKG_VERSION:=1.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).src.zip
PKG_SOURCE_URL:=@SF/libb64
PKG_MD5SUM:=a609809408327117e2c643bed91b76c5

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Base64 Encoding/Decoding Routines
endef

define Package/$(PKG_NAME)/description
  a library of ANSI C routines for fast encoding/decoding data into and from a
  base64-encoded format.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/*.a $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/src/*.so* $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/src/*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
