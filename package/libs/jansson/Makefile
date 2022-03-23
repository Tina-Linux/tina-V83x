include $(TOPDIR)/rules.mk

PKG_NAME:=jansson
PKG_VERSION:=2.11

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/akheron/jansson
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=34ffa583936aeb6938636c9c0a26a322b69b0d26
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

CMAKE_INSTALL:=1
PKG_RELEASE=$(PKG_SOURCE_VERSION)


CMAKE_OPTIONS += -DJANSSON_BUILD_SHARED_LIBS=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_SOURCE_SUBDIR)

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/libjansson
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libc
  TITLE:=A small library for encoding, decoding and manipulating JSON data
endef

define Package/libjansson/description
 Jansson is a C library for encoding, decoding and manipulating JSON data. It features:
 Simple and intuitive API and data model
 Comprehensive documentation
 No dependencies on other libraries
 Full Unicode support (UTF-8)
 Extensive test suite
endef


define Package/libjansson/install
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib* $(1)/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib* $(1)/lib/
endef


$(eval $(call BuildPackage,libjansson))

