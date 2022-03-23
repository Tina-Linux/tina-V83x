#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=xkeyboard-config
PKG_VERSION:=2.19
PKG_RELEASE:=1

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYING

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://www.x.org/releases/individual/data/xkeyboard-config
PKG_MD5SUM:=88cbf55f2f32b80ba9bb872889daf7ff

PKG_BUILD_DEPENDS:=intltool/host

include $(BUILD_DIR)/package.mk

PKG_BUILD_PARALLEL:=0
PKG_INSTALL:=1

define Package/xkeyboard-config
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=keyboard configuration database for X
  URL:=https://www.freedesktop.org/wiki/Software/XKeyboardConfig/
  MAINTAINER:=
endef

define Package/xkeyboard-config/description
   There are many X Window implementations which have very poor support for
non-standard keyboards, national layouts and options.Open Source X Window
System implementations (xfree86, x.org) currently have non-synchronized
XKB configuration databases the bugs fixed in one repository are not reflected
in the other, new configuration elements are added (in best case) separately
and independently to both CVS trees - but usually only one tree gets them. Also,
these implementations contain unbalanced and unstructured layout trees
(very often one country/language have several layouts, each with its own set of variants).
endef

CONFIGURE_ARGS += \
	--disable-runtime-deps

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/xkeyboard-config/install
	$(INSTALL_DIR) $(1)/usr/share/X11
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/X11/* \
		$(1)/usr/share/X11/
endef

$(eval $(call BuildPackage,xkeyboard-config))
