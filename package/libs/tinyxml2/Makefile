include $(TOPDIR)/rules.mk

PKG_NAME:=tinyxml2
PKG_VERSION:=7.0.1
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_LICENSE:=BSD

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS=-DTIDEBUG=1 -DTIXML_USE_STL=0

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:= +libstdcpp
  TITLE:= tinyxml2 library
  URL:=https://github.com/leethomason/tinyxml2/tree/7.0.1
endef

define Package/$(PKG_NAME)/description
  TinyXML is a simple, small, C++ XML parser that can be easily integrating into other programs.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(STAGING_DIR)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(STAGING_DIR)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
