#
# Copyright (C) 2006-2015 OpenWrt.org
#
# Permission is hereby granted, free of charge, to any person obtaining a
# copy of this software and associated documentation files (the "Software"),
# to deal in the Software without restriction, including without limitation
# the rights to use, copy, modify, merge, publish, distribute, sublicense,
# and/or sell copies of the Software, and to permit persons to whom the
# Software is furnished to do so, subject to the following conditions:
#
# The above copyright notice and this permission notice (including the next
# paragraph) shall be included in all copies or substantial portions of the
# Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
# FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
# DEALINGS IN THE SOFTWARE.
#
# ---
#
# The above is the version of the MIT "Expat" License used by X.org:
#
#    http://cgit.freedesktop.org/xorg/xserver/tree/COPYING
#

include $(TOPDIR)/rules.mk

PKG_NAME:=wayland-protocols
PKG_VERSION:=1.7
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://wayland.freedesktop.org/releases
PKG_MD5SUM:=9acfc9556f7cfedc44d97af60da66a5f

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=0
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/wayland-protocols
  SECTION:=gui
  CATEGORY:=Wayland
  DEPENDS:=+wayland
  TITLE:=wayland-protocols
  MAINTAINER:=<wayland-devel@lists.freedesktop.org>
  URL:=https://wayland.freedesktop.org/
endef

define Package/wayland-protocols/description
Wayland protocols that adds functionality not available in
the Wayland core protocol.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/wayland-protocols/install
	$(INSTALL_DIR) $(1)/usr/share/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
endef

$(eval $(call BuildPackage,wayland-protocols))
