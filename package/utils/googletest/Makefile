include $(TOPDIR)/rules.mk

PKG_NAME:=googletest
PKG_VERSION:=1.8.1
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/google/googletest
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-release-$(PKG_VERSION)
PKG_SOURCE_VERSION:=2fe3bd994b3189899d93f1d5a881e725e046fdc2
PKG_SOURCE:=$(PKG_NAME)-release-$(PKG_VERSION).tar.gz

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_SOURCE_SUBDIR)

PKG_LICENSE:=BSD 3-Clause
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS += -DBUILD_SHARED_LIBS=ON

TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Google's C++ test framework
  DEPENDS:=+libstdcpp
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
