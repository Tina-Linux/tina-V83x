#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gmime
PKG_VERSION:=3.0.1
PKG_RELEASE:=1

PKG_SOURCE:=gmime-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(COMPILE_DIR)/gmime-$(PKG_VERSION)

PKG_BUILD_PARALLEL:=1
PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/nls.mk

define Package/gmime
  SUBMENU:=gnome
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glib2 +$(ICONV_DEPENDS)
  TITLE:=An extremely robust high-performance parser
endef

define Package/gmime/description
  GMime is a C/C++ library which may be used for the creation and parsing of messages using the Multipurpose
  Internet Mail Extension (MIME) as defined by [numerous IETF specifications](https://github.com/jstedfast/gmime/blob/master/RFCs.md).
endef


CONFIGURE_ARGS += \
    --disable-static \

CONFIGURE_VARS += \
	ac_cv_have_iconv_detect_h=no

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/
	$(CP) $(PKG_INSTALL_DIR)/usr/* $(1)/usr/
endef

define Package/gmime/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libgmime-3.0.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,gmime))
