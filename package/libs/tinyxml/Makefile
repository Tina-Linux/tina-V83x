include $(TOPDIR)/rules.mk

PKG_NAME:=tinyxml
PKG_VERSION:=2.6.2
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_LICENSE:=BSD

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS=-DTIDEBUG=1 -DTIXML_USE_STL=0

define Package/libtinyxml
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:= +libstdcpp
  TITLE:= tinyxml library
endef

define Package/libtinyxml/description
  TinyXML is a simple, small, C++ XML parser that can be easily integrating into other programs.
endef

define Package/xmltest
  SECTION:=Utilities
  CATEGORY:=Utilities
  DEPENDS:= +libtinyxml
  TITLE:= test for tinyxml
endef

define Package/xmltest/description
  test for libtinyxml
endef

define Build/Prepare
	unzip -d $(PKG_BUILD_DIR) $(DL_DIR)/$(PKG_SOURCE)
	mv $(PKG_BUILD_DIR)/tinyxml/* $(PKG_BUILD_DIR)
	cp CMakeLists.txt $(PKG_BUILD_DIR)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/tinyxml $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/tinyxml/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libtinyxml.so $(1)/usr/lib/
endef

define Package/libtinyxml/install
	$(INSTALL_DIR) $(1)/usr/lib $(1)/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libtinyxml.so $(1)/usr/lib/
endef

define Package/xmltest/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/bin/xmltest $(1)/usr/bin
endef
$(eval $(call BuildPackage,libtinyxml))
$(eval $(call BuildPackage,xmltest))
