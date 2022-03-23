include $(TOPDIR)/rules.mk

PKG_NAME:=SQLiteCpp
PKG_VERSION:=2.0.0
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:= https://codeload.github.com/SRombauts/SQLiteCpp/tar.gz/$(PKG_VERSION)?
#PKG_HASH:=

PKG_LICENSE:=BSD-3-Clause
PKG_LICENSE_FILE:=COPYING

PKG_FIXUP:=autoreconf

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_INSTALL :=1

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=C++ SQLite3 wrapper library
  DEPENDS:= +libstdcpp 
  URL:=https://srombauts.github.com/SQLiteCpp
  MAINTAINER:=
endef

define Package/$(PKG_NAME)/description
  SQLiteC++ offers an encapsulation around the native C APIs of SQLite,
  with a few intuitive and well documented C++ classes.
endef

define Package/$(PKG_NAME)/config
menu "Configuration"
	depends on PACKAGE_SQLiteCpp

config SQLITECPP_USE_INTERNAL_SQLITE
	bool
	default y
	prompt "Add the internal SQLite3 source to the project."

config SQLITECPP_BUILD_EXAMPLES
	bool
	default n
	prompt "Build examples"

config SQLITE_ENABLE_ASSERT_HANDLER
	bool
	default n
	prompt "Enable the user defintion of a assertion_failed() handler"

config SQLITE_ENABLE_COLUMN_METADATA
	bool
	default y
	prompt "Enable Column::getColumnOriginName(). Require support from sqlite3 library"

endmenu
endef

TARGET_CXXFLAGS+=-std=c++11

ifeq ($(CONFIG_SQLITE_ENABLE_COLUMN_METADATA),y)
CMAKE_OPTIONS += -DSQLITE_ENABLE_COLUMN_METADATA=ON
endif

ifeq ($(CONFIG_SQLITE_ENABLE_ASSERT_HANDLER),y)
CMAKE_OPTIONS += -DSQLITE_ENABLE_ASSERT_HANDLER=ON
else
CMAKE_OPTIONS += -DSQLITE_ENABLE_ASSERT_HANDLER=OFF
endif

ifeq ($(CONFIG_SQLITECPP_INTERNAL_SQLITE),y)
CMAKE_OPTIONS += -DSQLITECPP_INTERNAL_SQLITE=ON
endif

ifeq ($(CONFIG_SQLITECPP_BUILD_EXAMPLES),y)
CMAKE_OPTIONS += -DSQLITECPP_BUILD_EXAMPLES=ON
endif

#define Build/Prepare
#	mkdir -p $(PKG_BUILD_DIR)
#endef

define Build/Configure
	$(call Build/Configure/Default,)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(CP) $(PKG_BUILD_DIR)/include/SQLiteCpp $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/libSQLiteCpp.{a,so*} $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/sqlite3/libsqlite3.a $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
