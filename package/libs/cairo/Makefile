#
# Copyright (C) 2009-2010 OpenWrt.org
#
# Cairo is free software and is available to be redistributed and/or
# modified under the terms of either the GNU Lesser General Public License (LGPL)
# version 2.1 or the Mozilla Public License (MPL) #version 1.1 at your option.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=cairo
PKG_VERSION:=1.14.6
PKG_RELEASE:=1
PKG_LICENSE:=LGPLv2.1+

PKG_BUILD_PARALLEL:=0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://www.cairographics.org/releases/
PKG_MD5SUM:=23a0b2f0235431d35238df1d3a517fdb

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_BUILD_DEPENDS += pkgconf/host
PKG_CONFIG_DEPENDS := \
	CONFIG_CAIRO_PS \
	CONFIG_CAIRO_PDF \
	CONFIG_CAIRO_PNG \
	CONFIG_CAIRO_SCRIPT \
	CONFIG_CAIRO_SVG \
	CONFIG_CAIRO_TEE \
	CONFIG_CAIRO_XML

include $(BUILD_DIR)/package.mk

ifeq ($(CONFIG_PACKAGE_libpthread),)
CAIRO_CONF_ENV += CPPFLAGS="$(TARGET_CPPFLAGS) -DCAIRO_NO_MUTEX=1"
endif

# cairo can use C++11 atomics when available, so we need to link with
# libatomic for the architectures who need libatomic.
ifeq ($(CONFIG_PACKAGE_libatomic),y)
CAIRO_CONF_ENV += LIBS="-latomic"
endif

define Package/cairo/Default
  TITLE:=Cairo is a 2D graphics library
  URL:=https://www.cairographics.org/
  MAINTAINER:=Karl Palsson <karlp@remake.is>
endef

define Package/libcairo/config
source "$(SOURCE)/Config.in"
endef

define Package/libcairo
$(call Package/cairo/Default)
  SECTION:=libs
  SUBMENU:=cairo
  CATEGORY:=Libraries
  DEPENDS:=+libpixman +libpng +zlib +fontconfig \
	  +PACKAGE_libfreetype:libfreetype \
	  +PACKAGE_glib2:glib2 \
	  +PACKAGE_liblzo:liblzo
  MENU:=1
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r16 r40 r18 r311))
	DEPENDS+= +WESTON_OPENGL_ES:mali400-um +WESTON_OPENGL_ES:libump
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r30))
	DEPENDS+= +WESTON_OPENGL_ES:mali-t760-um +WESTON_OPENGL_ES:libump
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r58))
	DEPENDS+= +WESTON_OPENGL_ES:sgx544-um +WESTON_OPENGL_ES:libump
endif
endef

define Package/libcairo/description
  Cairo is a 2D graphics library with support for multiple output devices.
Currently supported output targets include the X Window System (via both Xlib and XCB).
Quartz, Win32, image buffers, PostScript, PDF, and SVG file output.
Experimental backends include OpenGL, BeOS, OS/2, and DirectFB.

Cairo is designed to produce consistent output on all output media while
taking advantage of display hardware acceleration when available
(eg. through the X Render Extension).
endef

# DirectFB svg support rely on Cairo and Cairo DirectFB support depends on
# DirectFB. Break circular dependency by disabling DirectFB support in Cairo
# (which is experimental)
CAIRO_CONF_OPTS += --disable-directfb

#ifeq ($(PACKAGE_libfreetype),y)
CAIRO_CONF_OPTS += --enable-ft
#CAIRO_DEPENDENCIES += libfreetype
#else
#CAIRO_CONF_OPTS += --disable-ft
#endif

#ifeq ($(PACKAGE_glib2),y)
CAIRO_CONF_OPTS += --enable-gobject
CAIRO_DEPENDENCIES += glib2
#else
#CAIRO_CONF_OPTS += --disable-gobject
#endif

# Can use GL or GLESv2 but not both
#ifeq ($(PACKAGE_HAS_LIBGL),y)
#CAIRO_CONF_OPTS += --enable-gl --disable-glesv2
#CAIRO_DEPENDENCIES += libgl
#else
ifeq ($(CONFIG_WESTON_OPENGL_ES),y)
CAIRO_CONF_OPTS += --disable-gl --enable-glesv2 --enable-egl
else
CAIRO_CONF_OPTS += --disable-gl --disable-glesv2 --disable-egl
endif
#endif

#ifeq ($(PACKAGE_HAS_LIBOPENVG),y)
#CAIRO_CONF_OPTS += --enable-vg
#CAIRO_DEPENDENCIES += libopenvg
#else
#CAIRO_CONF_OPTS += --disable-vg
#endif

#ifeq ($(BR2_PACKAGE_LZO),y)
#CAIRO_DEPENDENCIES += lzo
#endif

CAIRO_CONF_OPTS += --disable-xlib --disable-xcb --without-x
CAIRO_CONF_OPTS += --disable-xlib-xrender

CAIRO_DEPENDENCIES += libdirectfb
ifeq ($(CONFIG_CAIRO_PS),y)
CAIRO_CONF_OPTS += --enable-ps
CAIRO_DEPENDENCIES += zlib
else
CAIRO_CONF_OPTS += --disable-ps
endif

ifeq ($(CONFIG_CAIRO_PDF),y)
CAIRO_CONF_OPTS += --enable-pdf
CAIRO_DEPENDENCIES += zlib
else
CAIRO_CONF_OPTS += --disable-pdf
endif

ifeq ($(CONFIG_CAIRO_PNG),y)
CAIRO_CONF_OPTS += --enable-png
CAIRO_DEPENDENCIES += libpng
else
CAIRO_CONF_OPTS += --disable-png
endif

ifeq ($(CONFIG_CAIRO_SCRIPT),y)
CAIRO_CONF_OPTS += --enable-script
else
CAIRO_CONF_OPTS += --disable-script
endif

ifeq ($(CONFIG_CAIRO_SVG),y)
CAIRO_CONF_OPTS += --enable-svg
else
CAIRO_CONF_OPTS += --disable-svg
endif

ifeq ($(CONFIG_CAIRO_TEE),y)
CAIRO_CONF_OPTS += --enable-tee
else
CAIRO_CONF_OPTS += --disable-tee
endif

ifeq ($(CONFIG_CAIRO_XML),y)
CAIRO_CONF_OPTS += --enable-xml
else
CAIRO_CONF_OPTS += --disable-xml
endif

LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lfreetype
TARGET_CFLAGS += $(FPIC)
CONFIGURE_ARGS += \
	--enable-trace=no \
	--enable-interpreter=no \
	$(CAIRO_CONF_OPTS)

define Package/libcairo/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

$(eval $(call BuildPackage,libcairo))
