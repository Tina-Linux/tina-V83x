#
# Copyright (C) 2006-2015 OpenWrt.org
# Copyright (C) 2000-2016 Stephen Houston
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
#    * Redistributions of source code must retain the above copyright notice,
#        this list of conditions and the following disclaimer.
#    * Redistributions in binary form must reproduce the above copyright notice,
#	this list of conditions and the following disclaimer in the documentation
#	and/or other materials provided with the distribution.
#    * Neither the name of ephoto nor the names of its contributors may be used
#	to endorse or promote products derived from this software without specific
#	prior written permission.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ephoto
PKG_VERSION:=1.5
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://download.enlightenment.org/rel/apps/ephoto
PKG_MD5SUM:=82e2bfa4f4cde07786df11bc36daf762

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=1

include $(BUILD_DIR)/package.mk

define Package/ephoto
  SECTION:=libs
  SUBMENU:=EFL
  CATEGORY:=Libraries
  TITLE:=A comprehensive image viewer written using the core EFL
  DEPENDS:=+efl +eudev
  MAINTAINER:=Houston <smhouston88@gmail.com>
  URL:=http://www.smhouston.us/ephoto/
endef

define Package/ephoto/description
Ephoto is an image viewer and editor written with a focus on simplicity and ease of use
while taking full advantage of EFL's speed and tiny memory footprint.
Browse images via thumbnail, single view with editing features, or display a moving slideshow.
There are neat filters to try, from Gaussian blurs to intricate sketches.
Ephoto is extremely versatile, in that it can act as both a simple viewer and an advanced image editor.
endef

CONFIGURE_ARGS += \
	--with-edje-cc=$(STAGING_DIR)/host/bin/edje_cc

TARGET_LDFLAGS+= -ludev
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -ludev

define Package/ephoto/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
endef

$(eval $(call BuildPackage,ephoto))
