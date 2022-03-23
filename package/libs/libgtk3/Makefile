#
# Copyright (C) 2018-2020 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# xielinfei@allwinnertech.com

include $(TOPDIR)/rules.mk

PKG_NAME:=gtk
PKG_VERSION:=3.22.10
PKG_RELEASE:=1
PKG_LICENSE:=LGPLv2.1+

PKG_BUILD_PARALLEL:=16

PKG_SOURCE:=gtk+-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:= http://ftp.gnome.org/pub/gnome/sources/gtk+/$(PKG_SOURCE)
PKG_BUILD_DIR:=$(COMPILE_DIR)/gtk+-3.22.10

PKG_FIXUP:=libtool auoreconf
PKG_REMOVE_FILES:=aclocal.m4 build/ltmain.sh
PKG_INSTALL:=1

PKG_BUILD_DEPENDS += \
	+pkgconf/host \
	+gettext-full/host \
	+atk \
	+glib2 \
	+libcairo \
	+pango \
	+gdk-pixbuf \
	+libepoxy \
	+libxkbcommon \
	+libpixman \
	+libinput \
	+wayland \
	+wayland-protocols \
	+udev \
	+libdrm

ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r16 r40 r18 r311))
	PKG_BUILD_DEPENDS+= +mali400-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r30))
	PKG_BUILD_DEPENDS+= +mali-t760-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r58))
	PKG_BUILD_DEPENDS+= +sgx544-um
endif

PKG_CONFIG_DEPENDS := \
	CONFIG_LIBGTK3_X11 \
	CONFIG_LIBGTK3_WAYLAN \
	CONFIG_LIBGTK3_BROADWA \
	CONFIG_XLIB_LIBXINERAM \
	CONFIG_XLIB_LIBXRAND \
	CONFIG_XLIB_LIBXCURSOR \
	CONFIG_XLIB_LIBXFIXE \
	CONFIG_XLIB_LIBXCOMPOSIT \
	CONFIG_LIBGTK3_DEMO \
	CONFIG_LIBGTK3_TEST

include $(BUILD_DIR)/package.mk

LIBGTK3_CONF_OPTS = \
	--enable-debug=yes \
	--disable-glibtest \
	--enable-explicit-deps=no \
	--disable-doc \
	--disable-docs \
	--disable-gtk-doc \
	--disable-gtk-doc-html \
	--disable-documentation \
	--disable-dependency-tracking \
	--enable-static \
	--enable-shared

CONFIGURE_VARS == \
	ac_cv_path_GTK_UPDATE_ICON_CACHE=$(STAGING_DIR_HOSTPKG)/usr/bin/gtk-update-icon-cache \
	ac_cv_path_GDK_PIXBUF_CSOURCE=$(STAGING_DIR_HOSTPKG)/usr/bin/gdk-pixbuf-csource \
	PKG_CONFIG_FOR_BUILD=$(STAGING_DIR_HOSTPKG)/usr/bin/pkgconf

ifeq ($(CONFIG_LIBGTK3_X11),y)
LIBGTK3_DEPENDENCIES += fontconfig xlib_libX11 xlib_libXext \
							xlib_libXrender xlib_libXi
LIBGTK3_CONF_OPTS += \
	--enable-x11-backend \
	--x-includes=$(STAGING_DIR)/usr/include/X11 \
	--x-libraries=$(STAGING_DIR)/usr/lib
else
LIBGTK3_CONF_OPTS += --disable-x11-backend
endif

ifeq ($(CONFIG_LIBGTK3_WAYLAND),y)
LIBGTK3_DEPENDENCIES += wayland wayland-protocols libxkbcommon
LIBGTK3_CONF_OPTS += --enable-wayland-backend
else
LIBGTK3_CONF_OPTS += --disable-wayland-backend
endif

ifeq ($(CONFIG_LIBGTK3_BROADWAY),y)
LIBGTK3_CONF_OPTS += --enable-broadway-backend
else
LIBGTK3_CONF_OPTS += --disable-broadway-backend
endif

ifeq ($(CONFIG_XLIB_LIBXINERAMA),y)
LIBGTK3_CONF_OPTS += --enable-xinerama
LIBGTK3_DEPENDENCIES += xlib_libXinerama
else
LIBGTK3_CONF_OPTS += --disable-xinerama
endif

ifeq ($(CONFIG_XLIB_LIBXRANDR),y)
LIBGTK3_CONF_OPTS += --enable-xrandr
LIBGTK3_DEPENDENCIES += xlib_libXrandr
else
LIBGTK3_CONF_OPTS += --disable-xrandr
endif

ifeq ($(CONFIG_XLIB_LIBXCURSOR),y)
LIBGTK3_DEPENDENCIES += xlib_libXcursor
endif

ifeq ($(CONFIG_XLIB_LIBXFIXES),y)
LIBGTK3_CONF_OPTS += --enable-xfixes
LIBGTK3_DEPENDENCIES += xlib_libXfixes
else
LIBGTK3_CONF_OPTS += --disable-xfixes
endif

ifeq ($(CONFIG_XLIB_LIBXCOMPOSITE),y)
LIBGTK3_CONF_OPTS += --enable-xcomposite
LIBGTK3_DEPENDENCIES += xlib_libXcomposite
else
LIBGTK3_CONF_OPTS += --disable-xcomposite
endif

ifeq ($(CONFIG_XLIB_LIBXDAMAGE),y)
LIBGTK3_CONF_OPTS += --enable-xdamage
LIBGTK3_DEPENDENCIES += xlib_libXdamage
else
LIBGTK3_CONF_OPTS += --disable-xdamage
endif

ifeq ($(CONFIG_XLIB_LIBXKBFILE),y)
LIBGTK3_CONF_OPTS += --enable-xkb
LIBGTK3_DEPENDENCIES += xlib_libxkbfile
else
LIBGTK3_CONF_OPTS += --disable-xkb
endif

ifeq ($(CONFIG_CUPS),y)
LIBGTK3_CONF_OPTS += --enable-cups
LIBGTK3_CONF_ENV += ac_cv_path_CUPS_CONFIG=$(STAGING_DIR)/usr/bin/cups-config
LIBGTK3_DEPENDENCIES += cups
else
LIBGTK3_CONF_OPTS += --disable-cups
endif

ifeq ($(CONFIG_LIBGTK3_DEMO),y)
LIBGTK3_DEPENDENCIES += hicolor-icon-theme shared-mime-info
else
define LIBGTK3_REMOVE_DEMOS
	$(RM) $(TARGET_DIR)/usr/bin/gtk3-demo \
		$(TARGET_DIR)/usr/bin/gtk3-demo-application
endef
LIBGTK3_POST_INSTALL_TARGET_HOOKS += LIBGTK3_REMOVE_DEMOS
endif

ifeq ($(CONFIG_LIBGTK3_TESTS),y)
LIBGTK3_CONF_OPTS += --enable-installed-tests
else
LIBGTK3_CONF_OPTS += --disable-installed-tests

define Package/libgtk3/config
source "$(SOURCE)/Config.in"
endef
endif

define Package/gtk3/Default
  TITLE:=GTK+ 3.22.10 package
  URL:=http://ftp.gnome.org/pub/gnome/sources/gtk+/
endef

define Package/libgtk3
	$(call Package/gtk3/Default)
  	SECTION:=libs
  	SUBMENU:=gtk3
  	CATEGORY:=Libraries
  	DEPENDS:=+libcairo +pango +libjpeg +atk +wayland +glib2 +libpcre +libepoxy +gdk-pixbuf +libxkbcommon
  	MENU:=1
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r16 r40 r18 r311))
	DEPENDS+= +mali400-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r30))
	DEPENDS+= +mali-t760-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r58))
	DEPENDS+= +sgx544-um
endif
endef

TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/lib/libintl/include/

TARGET_LDFLAGS+=-Wl,-rpath-link=$(STAGING_DIR)/usr/lib \
	-L$(STAGING_DIR)/usr/lib/libintl/lib/ \
	-L$(STAGING_DIR)/usr/lib/libiconv/lib/ \
	-L$(STAGING_DIR)/usr/lib/ -lMali -lepoxy

CONFIGURE_ARGS += \
	$(LIBGTK3_CONF_OPTS)

define Build/Configure
	(cd $(PKG_BUILD_DIR)/$(CONFIGURE_PATH); \
		if [ -x $(CONFIGURE_CMD) ]; then \
		$(CP) $(SCRIPT_DIR)/config.{guess,sub} $(PKG_BUILD_DIR)/ && \
		$(CONFIGURE_VARS); \
		$(CONFIGURE_CMD) \
		$(CONFIGURE_ARGS_XTRA) \
		$(CONFIGURE_ARGS) ;\
		fi \
		)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)
	$(MAKE) -C $(PKG_BUILD_DIR)/ install DESTDIR=$(PKG_INSTALL_DIR)
endef

define Build/InstallDev
	$(INSTALL_DIR)  $(STAGING_DIR)/usr/{lib/pkgconfig,include}
	$(CP) -r $(PKG_INSTALL_DIR)/usr/include/* $(STAGING_DIR)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(STAGING_DIR)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(STAGING_DIR)/usr/lib/pkgconfig/
	$(SED) "s,"prefix\=.*",prefix=$(STAGING_DIR)/usr," $(STAGING_DIR)/usr/lib/pkgconfig/{gtk,gdk}*-3.0.pc
	$(SED) "s,"exec_prefix\=.*",exec_prefix=$(STAGING_DIR)/usr," $(STAGING_DIR)/usr/lib/pkgconfig/{gtk,gdk}*-3.0.pc
	#$(SED) "s,"libdir\=\'/usr/lib\'",libdir='$(STAGING_DIR)/usr/lib\'," $(STAGING_DIR)/usr/lib/lib{gtk,gdk}*-3.0.la
endef

define Package/libgtk3/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share/
	$(CP) ./files/* $(1)/usr/share/
endef

$(eval $(call BuildPackage,libgtk3))
