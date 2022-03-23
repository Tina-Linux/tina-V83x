#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org

include $(TOPDIR)/rules.mk
PKG_NAME:=libwebp
PKG_VERSION:=0.4.3
PKG_RELEASE:=1
PKG_LICENSE:BSD

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://downloads.webmproject.org/release/webp

include $(BUILD_DIR)/package.mk

WEBP_CONF_OPTS += \
	--with-jpegincludedir=$(STAGING_DIR)/usr/include \
	--with-jpeglibdir=$(STAGING_DIR)/usr/lib \
	--with-tiffincludedir=$(STAGING_DIR)/usr/include \
	--with-tifflibdir=$(STAGING_DIR)/usr/lib

ifeq ($(CONFIG_WEBP_DEMUX),y)
WEBP_CONF_OPTS += --enable-libwebpdemux
else
WEBP_CONF_OPTS += --disable-libwebpdemux
endif

ifeq ($(PACKAGE_LIBPNG),y)
WEBP_CONF_ENV += ac_cv_path_LIBPNG_CONFIG=$(STAGING_DIR)/usr/bin/libpng-config
else
WEBP_CONF_ENV += ac_cv_path_LIBPNG_CONFIG=/bin/false
endif

define Package/libwebp/config
source "$(SOURCE)/Config.in"
endef

define Package/libwebp
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=webp libraries
	DEPENDS:=+PACKAGE_libjpeg:libjpeg +PACKAGE_libtiff:libtiff \
			+PACKAGE_libpng:libpng
endef

CONFIGURE_ARGS += \
	$(WEBP_CONF_OPTS)

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR)/ install DESTDIR=$(PKG_INSTALL_DIR)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef
define Package/libwebp/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*so* $(1)/usr/lib/
endef
$(eval $(call BuildPackage,libwebp))
