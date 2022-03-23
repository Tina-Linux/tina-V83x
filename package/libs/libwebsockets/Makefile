#
# Copyright (C) 2014-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=libwebsockets
PKG_VERSION:=2.4.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_PROTO:=git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/warmcat/libwebsockets.git
PKG_SOURCE_VERSION:=fcf5b2c25a580189bbad955815adac8dd398b612

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(BUILD_VARIANT)/$(PKG_SOURCE_SUBDIR)

PKG_LICENSE:=LGPL-2.1+exception
PKG_LICENSE_FILES:=LICENSE

CMAKE_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS += -DLWS_IPV6=$(if $(CONFIG_IPV6),ON,OFF)
CMAKE_OPTIONS += -DLWS_WITHOUT_TESTAPPS=ON

# other options worth noting
# CMAKE_OPTIONS += -DLWS_WITHOUT_EXTENSIONS=ON
# CMAKE_OPTIONS += -DLWS_WITHOUT_DAEMONIZE=ON
# CMAKE_OPTIONS += -DLWS_WITHOUT_SERVER=ON
# CMAKE_OPTIONS += -DLWS_WITHOUT_DEBUG=ON


define Package/$(PKG_NAME)/Default
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=libwebsockets
	DEPENDS:=+zlib
	URL:=https://libwebsockets.org
	MAINTAINER:=Karl Palsson <karlp@remake.is>
endef

define Package/libwebsockets-openssl
	$(call Package/$(PKG_NAME)/Default)
	TITLE += (OpenSSL)
	DEPENDS += +libopenssl +libcap
	VARIANT:=openssl
endef

define Package/libwebsockets-cyassl
	$(call Package/$(PKG_NAME)/Default)
	TITLE += (CyaSSL)
	DEPENDS += +libcyassl
	VARIANT:=cyassl
endef

ifeq ($(BUILD_VARIANT),openssl)
    CMAKE_OPTIONS += -DLWS_OPENSSL_CLIENT_CERTS=/etc/ssl/certs
    CMAKE_OPTIONS += -DLWS_OPENSSL_SUPPORT=ON
    CMAKE_OPTIONS += -DLWS_WITH_SSL=ON
endif

ifeq ($(BUILD_VARIANT),cyassl)
    CMAKE_OPTIONS += -DLWS_OPENSSL_CLIENT_CERTS=/etc/ssl/certs
    CMAKE_OPTIONS += -DLWS_OPENSSL_SUPPORT=ON
    CMAKE_OPTIONS += -DLWS_WITH_SSL=ON
# for cyassl, edit package/libs/cyassl/Makefile to include --enable-opensslextra
# NOTE: it will compile without it, untested whether it is needed?!
    CMAKE_OPTIONS += -DLWS_USE_CYASSL=ON
    CMAKE_OPTIONS += -DLWS_CYASSL_LIBRARIES=$(STAGING_DIR)/usr/lib/libcyassl.so
    CMAKE_OPTIONS += -DLWS_CYASSL_INCLUDE_DIRS=$(STAGING_DIR)/usr/include
endif

define Package/libwebsockets/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libwebsockets.so* $(1)/usr/lib/
endef

Package/$(PKG_NAME)-cyassl/install = $(Package/$(PKG_NAME)/install)
Package/$(PKG_NAME)-openssl/install = $(Package/$(PKG_NAME)/install)

$(eval $(call BuildPackage,libwebsockets-openssl))
$(eval $(call BuildPackage,libwebsockets-cyassl))
