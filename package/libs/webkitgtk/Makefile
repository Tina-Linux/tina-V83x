#
# Copyright (C) 2017-2020 OpenWrt.org
#
# WebkitGTK is free software and is available to be redistributed and/or
# modified under the terms of either the GNU Lesser General Public License (LGPL)
# version 2.1 or the Mozilla Public License (MPL) #version 1.1 at your option.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=webkitgtk
PKG_VERSION:=2.18.6
PKG_RELEASE:=1
PKG_LICENSE:=LGPLv2.1+, BSD-2c

PKG_BUILD_PARALLEL:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=http://www.webkitgtk/releases/
PKG_MD5SUM:=

CMAKE_INSTALL:=1

PKG_CONFIG_DEPENDS := \
	CONFIG_WEBKITGTK_HTTPS \
	CONFIG_WEBKITGTK_MULTIMEDIA

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/libwebkitgtk/Default
  TITLE:=WebkitGtk is a port for webkit base on Gtk
  URL:=https://www.cairographics.org/
  MAINTAINER:=Karl Palsson <karlp@remake.is>
endef

define Package/webkitgtk/config
source "$(SOURCE)/Config.in"
endef

define Package/webkitgtk
$(call Package/libwebkitgtk/Default)
  SECTION:=libs
  SUBMENU:=WebkitGtk
  CATEGORY:=Libraries
  DEPENDS:=+ruby/host +flex/host +bison/host +gperf/host \
	  +enchant +harfbuzz +icu +libjpeg +libgtk3 \
	  +libsecret +libsoup +libxml2 +libxslt +libsqlite3 \
	  +libegl +PACKAGE_libgles:libgles \
	  +libwebp +libgles +lcms2 +libtasn1 \
	  +CONFIG_WEBKITGTK_MULTIMEDIA:gstreamer1 \
	  +CONFIG_WEBKITGTK_MULTIMEDIA:gst1-libav \
	  +CONFIG_WEBKITGTK_MULTIMEDIA:gst1-plugins-base \
	  +CONFIG_WEBKITGTK_MULTIMEDIA:gst1-plugins-good \
	  +CONFIG_WEBKITGTK_MULTIMEDIA:gst1-plugins-ugly \
	  +CONFIG_WEBKITGTK_MULTIMEDIA:gst1-plugins-bad
  MENU:=1
endef

define Package/webkitgtk/description
WebkitGtk is a port
endef

WEBKITGTK_CONF_OPTS = \
	-DENABLE_JIT=OFF \
	-DENABLE_API_TESTS=OFF \
	-DENABLE_GEOLOCATION=OFF \
	-DENABLE_GTKDOC=OFF \
	-DENABLE_INTROSPECTION=OFF \
	-DENABLE_MINIBROWSER=ON \
	-DENABLE_SPELLCHECK=ON \
	-DPORT=GTK \
	-DUSE_LIBNOTIFY=OFF \
	-DUSE_LIBHYPHEN=OFF \
	-DENABLE_GLES2=ON \
	-DENABLE_OPENGL=ON \
	-DENABLE_WAYLAND_TARGET=ON \
	-DENABLE_VIDEO=OFF \
	-DENABLE_WEB_AUDIO=OFF \
	-DENABLE_PLUGIN_PROCESS_GTK2=OFF \
	-DCMAKE_SKIP_RPATH=ON
#ifeq ($(CONFIG_WEBKITGTK_MULTIMEDIA),y)
#WEBKITGTK_CONF_OPTS += \
#	-DENABLE_VIDEO=ON \
#	-DENABLE_WEB_AUDIO=ON
#else
#WEBKITGTK_CONF_OPTS += \
#	-DENABLE_VIDEO=OFF \
#	-DENABLE_WEB_AUDIO=OFF
#endif

#GLESv2 support is optional though
#ifeq ($(PACKAGE_libgles),y)
#WEBKITGTK_CONF_OPTS += \
	-DENABLE_GLES2=ON \
	-DENABLE_OPENGL=ON
#else
# Disable general OpenGL (shading) if there's no GLESv2
#WEBKITGTK_CONF_OPTS += \
#	-DENABLE_GLES2=OFF \
#	-DENABLE_OPENGL=OFF
#endif
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/harfbuzz/
CMAKE_OPTIONS = \
	$(WEBKITGTK_CONF_OPTS)

define Package/webkitgtk/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/libexec
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/libexec/* $(1)/usr/libexec/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

$(eval $(call BuildPackage,webkitgtk))
