#
# Copyright (C) 2006-2015 OpenWrt.org
#
# All library source is in the src/lib/ directory. All binaries are in
# src/bin/. All loadable modules are in src/modules/. All data files are
# in the data/ directory. Licensing details are listed in COPYING and
# the licenses/ directory. The pc/ directory holds all the Package
# Config files installed to help developers build against EFL.
#
# BSD, custom, GPL2, LGPL2.1
#

include $(TOPDIR)/rules.mk

PKG_NAME:=efl
PKG_VERSION:=1.20.6
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://download.enlightenment.org/rel/libs/efl
PKG_MD5SUM:=0c23c4b3c8f93b173b36ba2352ccffa0

PKG_FIXUP:=autoreconf
PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_BUILD_DEPENDS:=EFL_BUILD_HOST:efl/host
HOST_BUILD_DEPENDS:=luajit/host libfreetype/host dbus/host lua/host libpng/host libjpeg/host

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk

define Package/efl
  SECTION:=libs
  SUBMENU:=EFL
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp +libxkbcommon +libmount +libpng +libjpeg +bullet +zlib +luajit +libcurl +libfreetype +eudev +dbus +libinput +libopenssl +EFL_GLIB:glib2 +EFL_GSTREAMER1:gstreamer1-libs \
	  +EFL_GSTREAMER1:gstreamer1-plugins-base +EFL_PIXMAN:libpixman +EFL_FRIBIDI:libfribidi +EFL_FONT_CONFIG:fontconfig +EFL_TSLIB:tslib +EFL_WAYLAND:wayland +EFL_WAYLAND:wayland-protocols \
	  +EFL_WAYLAND:weston +EFL_DRM:libdrm +EFL_AUDIO:libsndfile
  TITLE:=Enlightenment Foundation Libraries
  MAINTAINER:=enlightenment-devel@lists.sourceforge.net
  URL:=https://www.enlightenment.org/start
  MENU:=1
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r16 r40 r18 r311))
	DEPENDS+= +EFL_OPENGLES:mali400-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r30))
	DEPENDS+= +EFL_OPENGLES:mali-t760-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r58))
	DEPENDS+= +EFL_OPENGLES:sgx544-um
endif
endef

define Package/efl/description
EFL, or the Enlightenment Foundation Libraries, powers millions of systems from mobile phones to set-top boxes,
desktops, laptops, game systems and more. EFL is recognized for its forward-thinking approach which allows
product designers and developers to offer more than the run-of-the-mill user experience of the past.
This is where EFL excels.

Free.fr has shipped millions of set top boxes in France powered by EFL.
The Openmoko Freerunner project also sold thousands of devices using EFL and Enlightenment.

EFL powers Samsung Galaxy Gear smartwatches and is behind Samsung Smart Televisions
such as the Class Q9F QLED 4K TV and refrigerators such as the Samsung Family Hub Refrigerator,
which are based on Tizen. GPS devices such as the Coyote also run EFL.
endef

# Host
HOST_CONFIGURE_ARGS += \
	--disable-systemd \
	--with-x11=none \
	--with-glib=no \
	--with-crypto=none \
	--disable-image-loader-gif \
	--disable-image-loader-tiff \
	--disable-image-loader-bmp \
	--disable-image-loader-eet \
	--disable-image-loader-generic \
	--disable-image-loader-ico \
	--disable-image-loader-jp2k \
	--disable-image-loader-pmaps \
	--disable-image-loader-psd \
	--disable-image-loader-tga \
	--disable-image-loader-tiff \
	--disable-image-loader-wbmp \
	--disable-image-loader-xpm \
	--disable-image-loader-tgv \
	--disable-image-loader-dds \
	--disable-spectre \
	--disable-libraw \
	--disable-librsvg \
	--disable-audio \
	--disable-pulseaudio \
	--disable-physics \
	--disable-gstreamer1 \
	--disable-fontconfig \
	--disable-fribidi \
	--disable-poppler \
	--disable-tslib \
	--disable-doc \
	--disable-neon \
	--disable-libeeze \
	--disable-magic-debug \
	--disable-cocoa \
	--disable-cserve \
	--disable-xcf \
	--disable-libmount \
	--disable-avahi \
	--disable-gesture \
	--disable-xpresent \
	--disable-xinput22 \
	--disable-xinput2 \
	--disable-xim \
	--disable-scim \
	--disable-ibus \
	--disable-elua \
	--enable-i-really-know-what-i-am-doing-and-that-this-will-probably-break-things-and-i-will-fix-them-myself-and-send-patches-abb

HOST_CFLAGS += -D_GNU_SOURCE

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) all
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install
endef

# Target
CONFIGURE_ARGS += \
	--with-edje-cc=$(STAGING_DIR)/host/bin/edje_cc \
	--with-eet-eet=$(STAGING_DIR)/host/bin/eet \
	--with-eldbus_codegen=$(STAGING_DIR)/host/bin/eldbus-codegen \
	--with-elementary-codegen=$(STAGING_DIR)/host/bin/elementary_codegen \
	--with-elm-prefs-cc=$(STAGING_DIR)/host/bin/elm_prefs_cc \
	--with-elua=$(STAGING_DIR)/host/bin/elua \
	--with-eolian-gen=$(STAGING_DIR)/host/bin/eolian_gen \
	--with-eolian-cxx=$(STAGING_DIR)/host/bin/eolian_cxx \
	--disable-systemd \
	--with-x11=none \
	--disable-image-loader-gif \
	--disable-image-loader-tiff \
	--disable-spectre \
	--disable-libraw \
	--disable-librsvg \
	--disable-doc \
	--disable-cocoa \
	--disable-elua \
	--enable-elput \
	--disable-poppler \
	--enable-i-really-know-what-i-am-doing-and-that-this-will-probably-break-things-and-i-will-fix-them-myself-and-send-patches-abb

MAKE_FLAGS += \
	WAYLAND_PROTOCOLS_DATADIR=$(STAGING_DIR)/usr/share/wayland-protocols


define Package/efl/config
  source "$(SOURCE)/Config.in"
endef

TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/bullet -I$(STAGING_DIR)/usr/include

ifeq ($(CONFIG_EFL_FB),y)
CONFIGURE_ARGS += --enable-fb
else
CONFIGURE_ARGS += --disable-fb
endif

ifeq ($(CONFIG_EFL_WAYLAND),y)
CONFIGURE_ARGS += \
	--enable-wayland \
	--enable-wayland-ivi-shell
else
CONFIGURE_ARGS += \
	--disable-wayland \
	--disable-wayland-ivi-shell
endif

ifeq ($(CONFIG_EFL_DRM),y)
CONFIGURE_ARGS += \
	--enable-drm \
	--enable-gl-drm
else
CONFIGURE_ARGS += \
	--disable-drm \
	--disable-gl-drm
endif

ifeq ($(CONFIG_EFL_OPENGLES),y)
CONFIGURE_ARGS += \
	--with-opengl=es \
	--enable-egl
else
CONFIGURE_ARGS += \
	--with-opengl=none \
	--disable-egl
endif

ifeq ($(CONFIG_EFL_GLIB),y)
CONFIGURE_ARGS += --with-glib=yes
else
CONFIGURE_ARGS += --with-glib=no
endif

ifeq ($(CONFIG_EFL_GSTREAMER1),y)
CONFIGURE_ARGS += --enable-gstreamer1
else
CONFIGURE_ARGS += --disable-gstreamer1
endif

ifeq ($(CONFIG_EFL_AUDIO),y)
CONFIGURE_ARGS += \
	--enable-audio \
	--disable-pulseaudio
else
CONFIGURE_ARGS += \
	--disable-audio \
	--disable-pulseaudio
endif

ifeq ($(CONFIG_EFL_PIXMAN),y)
CONFIGURE_ARGS += \
	--enable-pixman \
	--enable-pixman-font \
	--enable-pixman-rect \
	--enable-pixman-line \
	--enable-pixman-poly \
	--enable-pixman-image \
	--enable-pixman-image-scale-sample
else
CONFIGURE_ARGS += \
	--disable-pixman \
	--disable-pixman-font \
	--disable-pixman-rect \
	--disable-pixman-line \
	--disable-pixman-poly \
	--disable-pixman-image \
	--disable-pixman-image-scale-sample
endif

ifeq ($(CONFIG_EFL_FONT_CONFIG),y)
CONFIGURE_ARGS += --enable-fontconfig
else
CONFIGURE_ARGS += --disable-fontconfig
endif

ifeq ($(CONFIG_EFL_FRIBIDI),y)
CONFIGURE_ARGS += --enable-fribidi
else
CONFIGURE_ARGS += --disable-fribidi
endif

ifeq ($(CONFIG_EFL_TSLIB),y)
CONFIGURE_ARGS += --enable-tslib
else
CONFIGURE_ARGS += --disable-tslib
endif

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/efl/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/e* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
	$(CP) ./files/* $(1)/usr/share/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,efl))
