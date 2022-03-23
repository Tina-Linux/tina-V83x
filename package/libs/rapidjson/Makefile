include $(TOPDIR)/rules.mk

PKG_NAME:=rapidjson
PKG_VERSION:=1.1.0

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/Tencent/rapidjson.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=f54b0e47a08782a6131cc3d60f94d038fa6e0a51
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_SOURCE_SUBDIR)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libc
  TITLE:=A fast JSON parser/generator for C++
endef

define Package/$(PKG_NAME)/description
  RapidJSON is a JSON parser and generator for C++ with both SAX/DOM style API.
  It was inspired by RapidXml.
endef

define Build/Compile
	:
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/* $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	:
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
