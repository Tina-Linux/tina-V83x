#
# Copyright (C) 2007-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=gobject-introspection
PKG_VERSION:=1.34.2
PKG_RELEASE:=1

PKG_SOURCE:=gobject-introspection-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=@GNOME/gobject-introspection/1.34
PKG_MD5SUM:=9dc9822eb2912e9b73e51ae67fe86145

PKG_FIXUP:=autoreconf
HOST_BUILD_DEPENDS:=glib2/host libffi/host python/host
PKG_HOST_ONLY:=1

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/nls.mk

define Package/gobject-introspection
  SECTION:=devel
  CATEGORY:=Development
  DEPENDS:=$(ICONV_DEPENDS) $(INTL_DEPENDS) +zlib +libpthread +libffi +glib2 +python
  TITLE:=gobject introspection
  URL:=https://live.gnome.org/GObjectIntrospection
  BUILDONLY:=1
endef

define Package/gobject-introspection/description
  gobject introspection
endef

# Makefile does lots of pkg-config expansions, so need to provide the
# exports even when installing.
$(eval $(call Host/Exports,$(HOST_STAMP_INSTALLED)))

$(eval $(call HostBuild))
$(eval $(call BuildPackage,gobject-introspection))
