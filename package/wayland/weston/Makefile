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

PKG_NAME:=weston
PKG_VERSION:=2.0.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://wayland.freedesktop.org/releases
PKG_MD5SUM:=15f38945942bf2a91fe2687145fb4c7d

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=0
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/weston
  SECTION:=gui
  CATEGORY:=Wayland
  DEPENDS:=+libwebp +libxkbcommon +libpixman +libpng +libjpeg +mtdev \
	  +eudev +libcairo +libinput +wayland +wayland-protocols \
	  +WESTON_DRM:libdrm +WESTON_OPENGL_ES:libump +WESTON_DBUS:dbus \
	  +WESTON_LAUNCH:libpam +WESTON_LIBXML2:libxml2 \
	  +WESTON_SYSTEMD:systemd +WESTON_LCMS2:lcms2 \
	  +WESTON_LIBVA:libva +pango
  TITLE:=Weston is the reference implementation of a Wayland compositor
  MAINTAINER:=<wayland-devel@lists.freedesktop.org>
  URL:=https://wayland.freedesktop.org/
  MENU:=1
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r16 r40 r18 r311))
	DEPENDS+= +WESTON_OPENGL_ES:mali400-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r30))
	DEPENDS+= +WESTON_OPENGL_ES:mali-t760-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r58))
	DEPENDS+= +WESTON_OPENGL_ES:sgx544-um
endif
endef

define Package/weston/description
  Weston is the reference implementation of a Wayland
compositor, and a useful compositor in its own right.
Weston has various backends that lets it run on Linux kernel
modesetting and evdev input as well as under X11.
endef

CONFIGURE_ARGS += \
	--with-dtddir=$(STAGING_DIR)/usr/share/wayland \
	--disable-headless-compositor \
	--disable-colord \
	--disable-devdocs \
	--disable-libunwind \
	--disable-setuid-install \
	--disable-x11-compositor \
	--disable-xwayland \
	--disable-rdp-compositor \
	--with-cairo-glesv2

MAKE_FLAGS += \
	WAYLAND_PROTOCOLS_DATADIR=$(STAGING_DIR)/usr/share/wayland-protocols

TARGET_LDFLAGS+= -ludev -lGLESv2
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -ludev -lGLESv2

define Package/weston/config
  source "$(SOURCE)/Config.in"
endef

ifeq ($(CONFIG_WESTON_DBUS),y)
CONFIGURE_ARGS += --enable-dbus
else
CONFIGURE_ARGS += --disable-dbus
endif

# weston-launch must be u+s root in order to work properly
ifeq ($(CONFIG_WESTON_LAUNCH),y)
define WESTON_PERMISSIONS
	/usr/bin/weston-launch f 4755 0 0
endef
define WESTON_USERS
	weston-launch -1 Weston launcher group
endef
CONFIGURE_ARGS += --enable-weston-launch
else
CONFIGURE_ARGS += --disable-weston-launch
endif

# Needs wayland-egl, which normally only mesa provides
ifeq ($(CONFIG_WESTON_OPENGL_ES),y)
CONFIGURE_ARGS += --enable-egl
CONFIGURE_ARGS += --enable-simple-egl-clients
else
CONFIGURE_ARGS += \
			--disable-egl \
			--disable-simple-egl-clients
endif

ifeq ($(CONFIG_WESTON_FBDEV),y)
CONFIGURE_ARGS += \
	--enable-fbdev-compositor \
	WESTON_NATIVE_BACKEND=fbdev-backend.so
else
CONFIGURE_ARGS += --disable-fbdev-compositor
endif

ifeq ($(CONFIG_WESTON_DRM),y)
CONFIGURE_ARGS += \
	--enable-drm-compositor \
	WESTON_NATIVE_BACKEND=drm-backend.so
else
CONFIGURE_ARGS += --disable-drm-compositor
endif

ifeq ($(CONFIG_WESTON_LIBVA),y)
CONFIGURE_ARGS += --enable-vaapi-recorder
else
CONFIGURE_ARGS += --disable-vaapi-recorder
endif

ifeq ($(CONFIG_WESTON_LCMS2),y)
CONFIGURE_ARGS += --enable-lcms
else
CONFIGURE_ARGS += --disable-lcms
endif

ifeq ($(CONFIG_WESTON_SYSTEMD),y)
CONFIGURE_ARGS += --enable-systemd-login --enable-systemd-notify
else
CONFIGURE_ARGS += --disable-systemd-login --disable-systemd-notify
endif

ifeq ($(CONFIG_WESTON_LIBXML2),y)
CONFIGURE_ARGS += --enable-junit-xml
else
CONFIGURE_ARGS += --disable-junit-xml
endif

ifeq ($(CONFIG_WESTON_DEMO_CLIENTS),y)
CONFIGURE_ARGS += --enable-demo-clients-install
else
CONFIGURE_ARGS += --disable-demo-clients-install
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/weston/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/libweston-2
	$(INSTALL_DIR) $(1)/usr/lib/weston
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/share
	$(INSTALL_DIR) $(1)/etc/xdg
	$(INSTALL_DIR) $(1)/etc/init.d
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libweston-2/*.so* $(1)/usr/lib/libweston-2/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/weston/*.so* $(1)/usr/lib/weston/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/weston-* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
	$(CP) ./weston.ini $(1)/etc/xdg
#	$(CP) ./weston $(1)/etc/init.d
endef

$(eval $(call BuildPackage,weston))
