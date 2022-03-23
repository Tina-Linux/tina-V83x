#
# Copyright (C) 2016 Trace Wong <tracewong@outlook.com>
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# TODO:
# qt5 configure need to customer by package config
#
include $(TOPDIR)/rules.mk

PKG_NAME:=qt5
PKG_VERSION:=5.10.1
PKG_RELEASE:=1

IS_PREBUILT:=yes

ifeq ($(IS_PREBUILT),yes)
	PKG_MD5SUM:=7b478dd1e5fbb05b3a6b63bf1169b1da
	PKG_SOURCE:=qt-everywhere-opensource-src-$(PKG_VERSION)-prebuilt.tar.bz2
	PKG_BUILD_DIR=$(COMPILE_DIR)/qt-everywhere-opensource-src-$(PKG_VERSION)-prebuilt
else ifeq ($(IS_PREBUILT),64yes)
	PKG_MD5SUM:=e2b53856efe9e219a6dd22af57962b7f
	PKG_SOURCE:=qt-everywhere-opensource-src-$(PKG_VERSION)-prebuilt_64bit.tar.gz
	PKG_BUILD_DIR=$(COMPILE_DIR)/qt-everywhere-opensource-src-$(PKG_VERSION)-prebuilt_64bit
else
	PKG_MD5SUM:=5175fba2f221fd4c91e94771a57a5557
	PKG_SOURCE:=qt-everywhere-opensource-src-$(PKG_VERSION).tar.xz
	PKG_BUILD_DIR=$(COMPILE_DIR)/qt-everywhere-opensource-src-$(PKG_VERSION)
endif

PKG_SOURCE_URL:=http://mirrors.ustc.edu/qtproject/archive/5.6/$(PKG_VERSION)/single
PKG_BUILD_PARALLEL:=1
PKG_BUILD_DEPENDS:=+libstdcpp

include $(BUILD_DIR)/package.mk
-include $(if $(DUMP),,./files/qmake.mk)

define Package/qt5/Default
  SECTION:=Qt
  CATEGORY:=Qt
  TITLE:=qt5
  DEPENDS:=qt5-core
  URL:=http://qt-project.org
  MAINTAINER:=TraceWong <tracewong@outlook.org>
endef

ifeq ($(PACKAGE_qt5-wayland),y)
CONFIGURE_ARGS += -qpa wayland
endif

define Package/qt5-core
  $(call Package/qt5/Default)
  TITLE+=core
  DEPENDS:=+librt +zlib +libstdcpp +libinput +libpthread +glib2 @!LINUX_2_6
endef

define Package/qt5-gui
  $(call Package/qt5/Default)
  TITLE+=gui(eglfs)
  DEPENDS+=+qt5-core +qt5-dbus +libinput +tslib +mtdev +glib2 +fontconfig +libump \
	   +PACKAGE_qt5-gui:libpng +PACKAGE_qt5-gui:libfreetype \
	   +PACKAGE_qt5-gui:libjpeg +mali400-um @DISPLAY_SUPPORT
endef

define Package/qt5-multimedia
  $(call Package/qt5/Default)
  TITLE+=multimedia
  DEPENDS+=+qt5-core +qt5-gui +qt5-network +qt5-quick +qt5-quick +qt5-opengl
endef

define Package/qt5-network
  $(call Package/qt5/Default)
  TITLE+=network
  DEPENDS+=+qt5-core
endef

define Package/qt5-qml
  $(call Package/qt5/Default)
  TITLE+=qml
  DEPENDS+=+qt5-core +qt5-network +qt5-gui +qt5-remoteobjects
endef

define Package/qt5-remoteobjects
  $(call Package/qt5/Default)
  TITLE+=remoteobjects
  DEPENDS+=+qt5-core +qt5-network
endef

define Package/qt5-quick
  $(call Package/qt5/Default)
  TITLE+=quick
  DEPENDS+=+qt5-core +qt5-network +qt5-gui +qt5-qml
endef

define Package/qt5-quick-2
  $(call Package/qt5/Default)
  TITLE+=quick.2
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick
endef

define Package/qt5-quick-controls
  $(call Package/qt5/Default)
  TITLE+=quick-controls
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick +qt5-widgets
endef

define Package/qt5-quick-controls-2
  $(call Package/qt5/Default)
  TITLE+=quick-controls-2
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick-2 +qt5-widgets
endef

define Package/qt5-quick-dialogs
  $(call Package/qt5/Default)
  TITLE+=quick-dialogs
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick
endef

define Package/qt5-quick-extras
  $(call Package/qt5/Default)
  TITLE+=quick-extras
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick
endef

define Package/qt5-quick-layouts
  $(call Package/qt5/Default)
  TITLE+=quick-layouts
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick
endef

define Package/qt5-quick-localstorage
  $(call Package/qt5/Default)
  TITLE+=quick-localstorage
  DEPENDS+=+qt5-network +qt5-qml +qt5-sql
endef

define Package/qt5-quick-particles
  $(call Package/qt5/Default)
  TITLE+=quick-particles.2
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick +qt5-quickparticles
endef

define Package/qt5-quick-privatewidgets
  $(call Package/qt5/Default)
  TITLE+=quick-privatewidgets
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick +qt5-widgets
endef

define Package/qt5-quick-scene3d
  $(call Package/qt5/Default)
  TITLE+=quick-scene3d
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick +qt5-3d +qt5-concurrent
endef

define Package/qt5-quick-window
  $(call Package/qt5/Default)
  TITLE+=quick-window.2
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick
endef

define Package/qt5-quick-xmllistmodel
  $(call Package/qt5/Default)
  TITLE+=quick-xmllistmodel
  DEPENDS+=+qt5-network +qt5-xml +qt5-xmlpatterns +qt5-qml
endef

define Package/qt5-sql
  $(call Package/qt5/Default)
  TITLE+=sql(sqlite)
  DEPENDS+=+libsqlite3 +sqlite3
endef

define Package/qt5-test
  $(call Package/qt5/Default)
  TITLE+=test
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick +qt5-widgets
endef

define Package/qt5-enginio
  $(call Package/qt5/Default)
  TITLE+=enginio
  DEPENDS+=+qt5-core +qt5-network +qt5-qml
endef

define Package/qt5-widgets
  $(call Package/qt5/Default)
  TITLE+=widgets
  DEPENDS+=+qt5-core +qt5-gui
endef

define Package/qt5-bluetooth
  $(call Package/qt5/Default)
  TITLE+=bluetooth
  DEPENDS+=+qt5-core +qt5-concurrent +qt5-dbus +qt5-gui +qt5-network +qt5-qml +qt5-quick
endef

define Package/qt5-concurrent
  $(call Package/qt5/Default)
  TITLE+=concurrent
  DEPENDS+=+qt5-core
endef

define Package/qt5-dbus
  $(call Package/qt5/Default)
  TITLE+=dbus
  DEPENDS+=+qt5-core +PACKAGE_qt5-dbus:libdbus
endef

define Package/qt5-location
  $(call Package/qt5/Default)
  TITLE+=location
  DEPENDS+=+qt5-core +qt5-network +qt5-gui +qt5-qml +qt5-quick +qt5-positioning
endef

define Package/qt5-nfc
  $(call Package/qt5/Default)
  TITLE+=nfc
  DEPENDS+=+qt5-core +qt5-dbus +qt5-network +qt5-qml
endef

define Package/qt5-opengl
  $(call Package/qt5/Default)
  TITLE+=opengl
  DEPENDS+=+qt5-core +qt5-gui +qt5-widgets
endef

define Package/qt5-positioning
  $(call Package/qt5/Default)
  TITLE+=positioning
  DEPENDS+=+qt5-core +qt5-gui +qt5-network +qt5-qml +qt5-quick
endef

define Package/qt5-printsupport
  $(call Package/qt5/Default)
  TITLE+=printsupport
  DEPENDS+=+qt5-core +qt5-gui +qt5-widgets
endef

define Package/qt5-quickwidgets
  $(call Package/qt5/Default)
  TITLE+=quickwidgets
  DEPENDS+=+qt5-core +qt5-network +qt5-gui +qt5-qml +qt5-quick +qt5-widgets
endef

define Package/qt5-script
  $(call Package/qt5/Default)
  TITLE+=script
  DEPENDS+=+qt5-core
endef

define Package/qt5-scripttools
  $(call Package/qt5/Default)
  TITLE+=scripttools
  DEPENDS+=+qt5-core +qt5-network +qt5-qml +qt5-gui +qt5-quick +qt5-widgets +qt5-script
endef

define Package/qt5-sensors
  $(call Package/qt5/Default)
  TITLE+=sensors
  DEPENDS+=+qt5-core +qt5-gui +qt5-network +qt5-qml
endef

define Package/qt5-serialport
  $(call Package/qt5/Default)
  TITLE+=serialport
  DEPENDS+=+qt5-core +eudev
endef

define Package/qt5-svg
  $(call Package/qt5/Default)
  TITLE+=svg
  DEPENDS+=+qt5-core +qt5-widgets
endef

define Package/qt5-webchannel
  $(call Package/qt5/Default)
  TITLE+=webchannel
  DEPENDS+=+qt5-core +qt5-network +qt5-qml +qt5-gui +qt5-quick
endef

define Package/qt5-webengine
  $(call Package/qt5/Default)
  TITLE+=webengine
  DEPENDS+=+qt5-core +qt5-network
endef

define Package/qt5-websocket
  $(call Package/qt5/Default)
  TITLE+=websocket
  DEPENDS+=+qt5-core +qt5-network +qt5-qml
endef

define Package/qt5-xml
  $(call Package/qt5/Default)
  TITLE+=xml
  DEPENDS+=+qt5-core
endef

define Package/qt5-xmlpatterns
  $(call Package/qt5/Default)
  TITLE+=xmlpatterns
  DEPENDS+=+qt5-core +qt5-network
endef

define Package/qt5-serialbus
  $(call Package/qt5/Default)
  TITLE+=serialbus
  DEPENDS+=+qt5-core +qt5-network +qt5-serialport +eudev
endef

define Package/qt5-help
  $(call Package/qt5/Default)
  TITLE+=help
  DEPENDS+=+qt5-core +qt5-gui +qt5-widgets +qt5-sql +qt5-network +qt5-clucene
endef

define Package/qt5-labstemplates
  $(call Package/qt5/Default)
  TITLE+=labstemplates
  DEPENDS+=+qt5-core +qt5-gui +qt5-quick +qt5-network +qt5-qml
endef

define Package/qt5-clucene
  $(call Package/qt5/Default)
  TITLE+=clucene
  DEPENDS+=+qt5-core
endef

define Package/qt5-quickparticles
  $(call Package/qt5/Default)
  TITLE+=quickparticles
  DEPENDS+=+qt5-core +qt5-gui +qt5-quick +qt5-network \
	   +qt5-qml
endef

define Package/qt5-3d
  $(call Package/qt5/Default)
  TITLE+=3d
  DEPENDS+=+qt5-core +qt5-gui +qt5-quick +qt5-network \
	   +qt5-qml +qt5-concurrent +qt5-gamepad
endef

define Package/qt5-charts
  $(call Package/qt5/Default)
  TITLE+=charts
  DEPENDS+=+qt5-core +qt5-opengl +qt5-gui +qt5-qml +qt5-quick +qt5-widgets
endef

define Package/qt5-speech
  $(call Package/qt5/Default)
  TITLE+=charts
  DEPENDS+=+qt5-core
endef

define Package/qt5-scxml
  $(call Package/qt5/Default)
  TITLE+=scxml
  DEPENDS+=+qt5-core +qt5-qml +qt5-network
endef

define Package/qt5-purchasing
  $(call Package/qt5/Default)
  TITLE+=purchasing
  DEPENDS+=+qt5-core +qt5-opengl +qt5-gui +qt5-qml +qt5-network qt5-quick
endef

define Package/qt5-datavis3d
  $(call Package/qt5/Default)
  TITLE+=datavis3d
  DEPENDS+=+qt5-core +qt5-opengl +qt5-qml
endef
define Package/qt5-examples
  $(call Package/qt5/Default)
  TITLE+=examples
  DEPENDS+=+qt5-core +qt5-gui +qt5-3d \
	   +qt5-svg +qt5-sql +qt5-network +qt5-quick +qt5-quickwidgets \
	   +qt5-multimedia +qt5-network \
	   +qt5-qml +qt5-quick +qt5-quick-2 +qt5-sql +qt5-test \
	   +qt5-widgets +qt5-bluetooth +qt5-concurrent +qt5-dbus \
	   +qt5-location +qt5-nfc +qt5-opengl +qt5-positioning \
	   +qt5-printsupport +qt5-quickwidgets +qt5-script \
	   +qt5-scripttools +qt5-sensors +qt5-serialport \
	   +qt5-svg +qt5-webchannel +qt5-websocket +qt5-enginio\
	   +qt5-xml +qt5-xmlpatterns +qt5-serialbus +qt5-help \
	   +qt5-clucene +qt5-labstemplates +qt5-quickparticles \
           +qt5-charts +qt5-datavis3d +qt5-network +qt5-purchasing +qt5-quick-2 \
           +qt5-scxml +qt5-speech +qt5-bluetooth  +qt5-remoteobjects \
           +qt5-script
endef

define Package/qt5-wayland
  $(call Package/qt5/Default)
  TITLE+=wayland
  DEPENDS+=+qt5-core +qt5-gui +qt5-dbus +qt5-network +qt5-qml +qt5-quick +qt5-wayland:weston +qt5-wayland:wayland
endef

define Package/qt5-gamepad
  $(call Package/qt5/Default)
  TITLE+=gamepad
  DEPENDS+=+qt5-core +qt5-gui +qt5-network +qt5-quick
endef

define Package/qt5-openssl
  $(call Package/qt5/Default)
  TITLE+=openssl
  DEPENDS+=+PACKAGE_qt5-openssl:libopenssl
endef

define Package/qt5-drivers-mouse
  $(call Package/qt5/Default)
  TITLE+=drivers-mouse
  DEPENDS+=qt5-gui
endef

define Package/qt5-drivers-keyboard
  $(call Package/qt5/Default)
  TITLE+=drivers-keyboard
  DEPENDS+=qt5-gui
endef

define Package/qt5-drivers-touchscreen
  $(call Package/qt5/Default)
  TITLE+=drivers-touchscreen(tslib)
  DEPENDS+=+qt5-gui +PACKAGE_qt5-drivers-touchscreen:tslib
endef

define Package/qt5-drivers-linuxfb
  $(call Package/qt5/Default)
  TITLE+=drivers-linuxfb
  DEPENDS+=qt5-gui
endef

define Package/qt5-drivers-audio
  $(call Package/qt5/Default)
  TITLE+=drivers-audio(alsa)
  DEPENDS+=+qt5-network +qt5-multimedia +alsa-lib
endef

define Package/qt5-drivers-customwidget
  $(call Package/qt5/Default)
  TITLE+=drivers-customwidget
  DEPENDS+=+qt5-widgets
endef

define Package/qt5-drivers-worldtimeclock
  $(call Package/qt5/Default)
  TITLE+=drivers-worldtimeclock
  DEPENDS+=+qt5-widgets
endef

define Package/qt5-drivers-imageformats
  $(call Package/qt5/Default)
  TITLE+=drivers-imageformats
  DEPENDS+=+qt5-svg +qt5-widgets
endef

define Package/qt5-graphicaleffects
  $(call Package/qt5/Default)
  TITLE+=graphicaleffects
  DEPENDS+=+qt5-gui +qt5-network +qt5-qml +qt5-quick
endef

define Package/qt5-gsttools
  $(call Package/qt5/Default)
  TITLE+=gsttools
  DEPENDS+=+qt5-gui +qt5-multimedia +qt5-multimediawidgets \
	   +qt5-network +qt5-widgets +alsa-lib +gstreamer1-plugins-base \
	   +gstreamer1-libs +gst1-plugins-base
endef

TARGET_LDFLAGS+=-Wl,-rpath-link=$(STAGING_DIR)/usr/lib,-rpath-link=$(PKG_BUILD_DIR)/lib
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/freetype2 -I$(PKG_BUILD_DIR)/include

define Build/Prepare

ifneq ($(IS_PREBUILT),no)
	$(call PKG_UNPACK)
	$(CP) ./files/qt.conf \
		$(PKG_BUILD_DIR)/ipkg-install/usr/bin/qt.conf
else
	$(call Build/Prepare/Default,)
endif

endef


define Build/Configure

ifeq ($(IS_PREBUILT),no)
	mkdir -p $(PKG_BUILD_DIR)/qtbase/mkspecs/linux-openwrt-g++
	$(CP) ./files/qmake.conf $(PKG_BUILD_DIR)/qtbase/mkspecs/linux-openwrt-g++/qmake.conf
	$(CP) ./files/qplatformdefs.h $(PKG_BUILD_DIR)/qtbase/mkspecs/linux-openwrt-g++/qplatformdefs.h
	(cd $(PKG_BUILD_DIR); \
		TARGET_CC="$(TARGET_CROSS)gcc" \
		TARGET_CXX="$(TARGET_CROSS)g++" \
		TARGET_AR="$(TARGET_CROSS)ar cqs" \
		TARGET_OBJCOPY="$(TARGET_CROSS)objcopy" \
		TARGET_RANLIB="$(TARGET_CROSS)ranlib" \
		TARGET_CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
		TARGET_CXXFLAGS="$(TARGET_CFLAGS) -fpermissive $(EXTRA_CFLAGS)" \
		TARGET_LDFLAGS="$(TARGET_LDFLAGS) $(EXTRA_LDFLAGS)" \
		TARGET_INCDIRS="$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" \
		TARGET_LIBDIRS="$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
		CFLAGS= \
		CXXFLAGS= \
		LDFLAGS= \
		STAGING_DIR="$(STAGING_DIR)" \
		STAGING_DIR_HOST="$(STAGING_DIR_HOST)" \
		PKG_CONFIG_SYSROOT_DIR="$(STAGING_DIR)" \
		PKG_CONFIG_PATH="$(STAGING_DIR)/usr/lib/pkgconfig" \
		./configure \
		-prefix $(CONFIGURE_PREFIX) \
		-bindir $(CONFIGURE_PREFIX)/bin \
		-headerdir $(CONFIGURE_PREFIX)/include \
		-libdir $(CONFIGURE_PREFIX)/lib \
		-datadir $(CONFIGURE_PREFIX)/share/qt5 \
		-qmldir $(CONFIGURE_PREFIX)/lib/qt5/qml \
		-plugindir $(CONFIGURE_PREFIX)/lib/qt5/plugins \
		-release \
		-xplatform linux-openwrt-g++ \
		-opensource \
		-confirm-license \
		-c++std c++11 \
		-shared \
		-accessibility \
		-no-qml-debug \
		-no-sse2 \
		-no-sse3 \
		-no-ssse3 \
		-no-sse4.1 \
		-no-sse4.2 \
		-no-avx \
		-no-avx2 \
		-no-mips_dsp \
		-no-mips_dspr2 \
		-pkg-config \
		-system-zlib \
		-mtdev \
		-no-journald \
		-syslog \
		-system-libpng \
		-system-libjpeg \
		-system-freetype \
		-no-sql-db2 \
		-no-sql-ibase \
		-no-sql-mysql \
		-no-sql-oci \
		-no-sql-odbc \
		-no-sql-psql \
		-sql-sqlite \
		-sqlite \
		-no-sql-tds \
		-qt-harfbuzz \
		-openssl \
		-no-libproxy \
		-qt-pcre \
		-no-xcb \
		-no-xinput2 \
		-no-xcb-xlib \
		-no-glib \
		-no-pulseaudio \
		-alsa \
		-no-gtk \
		-opengl es2 \
		-gui \
		-widgets \
		-no-cups \
		-iconv \
		-evdev \
		-tslib \
		-no-icu \
		-fontconfig \
		-strip \
		-no-pch \
		-no-ltcg \
		-dbus-linked \
		-no-use-gold-linker \
		-eglfs \
		-no-kms \
		-no-gbm \
		-no-directfb \
		-linuxfb \
		-no-mirclient \
		-no-libinput \
		-no-gstreamer \
		-nomake tests \
		-skip qtdoc \
		-skip qtwayland \
		-v \
		)
endif

endef

define Build/Compile

ifeq ($(IS_PREBUILT),no)
	TARGET_CC="$(TARGET_CROSS)gcc" \
		TARGET_CXX="$(TARGET_CROSS)g++" \
		TARGET_AR="$(TARGET_CROSS)ar cqs" \
		TARGET_OBJCOPY="$(TARGET_CROSS)objcopy" \
		TARGET_RANLIB="$(TARGET_CROSS)ranlib" \
		TARGET_CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
		TARGET_CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
		TARGET_LDFLAGS="$(TARGET_LDFLAGS) $(EXTRA_LDFLAGS)" \
		TARGET_INCDIRS="$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" \
		TARGET_LIBDIRS="$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
		STAGING_DIR="$(STAGING_DIR)" \
		STAGING_DIR_HOST="$(STAGING_DIR_HOST)" \
		PKG_CONFIG_SYSROOT_DIR="$(STAGING_DIR)" \
		$(MAKE) -C $(PKG_BUILD_DIR)
	INSTALL_ROOT=$(PKG_INSTALL_DIR) \
		$(MAKE) -C $(PKG_BUILD_DIR) install
	$(CP) ./files/qt.conf \
		$(PKG_BUILD_DIR)/ipkg-install/usr/bin/qt.conf
endif

endef

define Build/InstallDev
	$(INSTALL_DIR) $(2)/bin
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/usr/mkspecs
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DIR) $(1)/usr/lib/cmake
	$(INSTALL_DIR) $(1)/usr/lib/qt5/plugins
	$(INSTALL_DIR) $(1)/usr/lib/qt5/plugins/egldeviceintegrations
	$(INSTALL_DIR) $(1)/mk/
	$(INSTALL_DATA) ./files/qmake.mk $(1)/mk/

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/* \
		$(2)/bin/
	
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/bin/* \
		$(2)/bin/
	
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/mkspecs/* \
		$(1)/usr/mkspecs/
	
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* \
		$(1)/usr/lib/pkgconfig/
	
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/* \
		$(1)/usr/include/
	
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
	
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.prl* \
		$(1)/usr/lib/
	
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.la* \
		$(1)/usr/lib/
	
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/cmake/* \
		$(1)/usr/lib/cmake/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/* \
		$(1)/usr/lib/qt5/plugins/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/egldeviceintegrations/* \
		$(1)/usr/lib/qt5/plugins/egldeviceintegrations/
endef

define Package/qt5-core/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Core.so* \
		$(1)/usr/lib/
endef

define Package/qt5-gui/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/share/fonts
	$(INSTALL_DIR) $(1)/usr/lib/qt5/plugins/platforms
	$(INSTALL_DIR) $(1)/usr/lib/qt5/plugins/egldeviceintegrations
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Gui.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5EglFSDeviceIntegration.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/platforms/libqeglfs.so \
		$(1)/usr/lib/qt5/plugins/platforms/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/egldeviceintegrations/* \
		$(1)/usr/lib/qt5/plugins/egldeviceintegrations/
	$(CP) \
		./files/qt-env.sh \
		$(1)/etc/
	#$(CP) \
	#	$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/platforms/libqlinuxfb.so \
	#	$(1)/usr/lib/qt5/plugins/

	# we use the dejavue font package instead
ifneq ($(IS_PREBUILT),no)
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/share/qt5/examples/quickcontrols2/swipetoremove/fonts/*.ttf \
		$(1)/usr/share/fonts/
else
	$(CP) \
		$(PKG_BUILD_DIR)/qtbase/tests/auto/shared/resources/*.ttf \
		$(1)/usr/share/fonts/

endif
endef

define Package/qt5-multimedia/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(INSTALL_DIR) $(1)/usr/lib/qt5/plugins/video
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Multimedia.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5MultimediaQuick.so* \
		$(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5MultimediaWidgets.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/video/* \
		$(1)/usr/lib/qt5/plugins/video
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtMultimedia \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-network/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Network.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5NetworkAuth.so* \
		$(1)/usr/lib/
endef

define Package/qt5-qml/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Qml.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQml \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-remoteobjects/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5RemoteObjects.so* \
		$(1)/usr/lib/
endef

define Package/qt5-quick/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Quick.so* \
		$(1)/usr/lib/
endef

define Package/qt5-quick-2/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5QuickControls2.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5QuickTemplates2.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick.2 \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-quick-controls/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/Controls \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-controls-2/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/Controls.2 \
		$(1)/usr/lib/qt5/qml/QtQuick/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/Templates.2 \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-dialogs/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/Dialogs \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-extras/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/Extras \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-layouts/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/Layouts \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-localstorage/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/LocalStorage \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-particles/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/Particles.2 \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-privatewidgets/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/PrivateWidgets \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-scene3d/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/Scene3D \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-window/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/Window.2 \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-quick-xmllistmodel/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml/QtQuick
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtQuick/XmlListModel \
		$(1)/usr/lib/qt5/qml/QtQuick/
endef

define Package/qt5-sql/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/plugins/sqldrivers/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Sql.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/sqldrivers/libqsqlite.so \
		$(1)/usr/lib/qt5/plugins/sqldrivers/
endef

define Package/qt5-test/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Test.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtTest \
		$(1)/usr/lib/qt5/qml/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5QuickTest.so* \
		$(1)/usr/lib/
endef

#define Package/qt5-enginio/install
#	$(INSTALL_DIR) $(1)/usr/lib/
#	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
#	$(CP) \
#		$(PKG_INSTALL_DIR)/usr/lib/libEnginio.so* \
#		$(1)/usr/lib/
#	$(CP) \
#		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/Enginio \
#		$(1)/usr/lib/qt5/qml/
#endef

define Package/qt5-widgets/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Widgets.so* \
		$(1)/usr/lib/
endef

define Package/qt5-bluetooth/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Bluetooth.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtBluetooth \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-concurrent/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Concurrent.so* \
		$(1)/usr/lib/
endef

define Package/qt5-dbus/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5DBus.so* \
		$(1)/usr/lib/
endef

define Package/qt5-location/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Location.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtLocation \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-nfc/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Nfc.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtNfc \
		$(1)/usr/lib/qt5/qml
endef

define Package/qt5-opengl/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5OpenGL.so* \
		$(1)/usr/lib/
endef

define Package/qt5-positioning/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Positioning.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtPositioning \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-printsupport/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5PrintSupport.so* \
		$(1)/usr/lib/
endef

define Package/qt5-quickwidgets/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5QuickWidgets.so* \
		$(1)/usr/lib/
endef

define Package/qt5-script/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Script.so* \
		$(1)/usr/lib/
endef

define Package/qt5-scripttools/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5ScriptTools.so* \
		$(1)/usr/lib/
endef

define Package/qt5-sensors/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Sensors.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtSensors \
		$(1)/usr/lib/qt5/qml
endef

define Package/qt5-serialport/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5SerialPort.so* \
		$(1)/usr/lib/
endef

define Package/qt5-svg/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Svg.so* \
		$(1)/usr/lib/
endef

define Package/qt5-webchannel/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5WebChannel.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtWebChannel \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-webengine/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5WebEngine.so* \
		$(1)/usr/lib/
endef

define Package/qt5-websocket/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5WebSockets.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtWebSockets \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-xml/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Xml.so* \
		$(1)/usr/lib/
endef

define Package/qt5-xmlpatterns/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5XmlPatterns.so* \
		$(1)/usr/lib/
endef

define Package/qt5-serialbus/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5SerialBus.so* \
		$(1)/usr/lib/
endef

define Package/qt5-help/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Help.so* \
		$(1)/usr/lib/
endef

define Package/qt5-labstemplates/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5QuickParticles.so* \
		$(1)/usr/lib/
endef

define Package/qt5-quickparticles/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5QuickParticles.so* \
		$(1)/usr/lib/
endef

#define Package/qt5-clucene/install
#	$(INSTALL_DIR) $(1)/usr/lib/
#	$(CP) \
#		$(PKG_INSTALL_DIR)/usr/lib/libQt5CLucene.so* \
#		$(1)/usr/lib/
#endef

define Package/qt5-3d/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DCore.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DInput.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DLogic.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DQuick.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DQuickInput.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DRender.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DQuickRender.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DAnimation.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DExtras.so* \
                $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DQuickAnimation.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt53DQuickExtras.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/Qt3D \
		$(1)/usr/lib/qt5/qml/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtCanvas3D \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-wayland/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(INSTALL_DIR) $(1)/usr/lib/qt5/plugins/platforms
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5WaylandClient.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5WaylandCompositor.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtWayland \
		$(1)/usr/lib/qt5/qml/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/platforms/libqwayland-generic.so \
		$(1)/usr/lib/qt5/plugins/platforms
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/platforms/libqwayland-egl.so \
		$(1)/usr/lib/qt5/plugins/platforms
endef

define Package/qt5-gamepad/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Gamepad.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtGamepad \
		$(1)/usr/lib/qt5/qml/

endef

define Package/qt5-charts/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Charts.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtCharts \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-speech/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5TextToSpeech.so* \
		$(1)/usr/lib/
endef

define Package/qt5-datavis3d/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5DataVisualization.so* \
		$(1)/usr/lib/
endef

define Package/qt5-scxml/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Scxml.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtScxml \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-purchasing/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libQt5Purchasing.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtPurchasing \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-examples/install
	$(INSTALL_DIR) \
		$(1)/usr/share/qt5

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/share/qt5/examples \
		$(1)/usr/share/qt5/

	$(FIND) \
		$(1) \
		-name "*.cpp" -o \
		-name "*.h" -o \
		-name "*.pro" \
		| $(XARGS) rm
endef

define Package/qt5-drivers-mouse/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/qt5/plugins/generic

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/generic/libqevdevmouseplugin.so \
		$(1)/usr/lib/qt5/plugins/generic
endef

define Package/qt5-drivers-keyboard/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/qt5/plugins/generic

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/generic/libqevdevkeyboardplugin.so \
		$(1)/usr/lib/qt5/plugins/generic
endef

define Package/qt5-drivers-touchscreen/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/qt5/plugins/generic

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/generic/libqtslibplugin.so \
		$(1)/usr/lib/qt5/plugins/generic
endef

define Package/qt5-drivers-linuxfb/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/qt5/plugins/platforms

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/platforms/libqlinuxfb.so \
		$(1)/usr/lib/qt5/plugins/platforms
endef

define Package/qt5-drivers-audio/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/qt5/plugins/audio

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/audio/libqtaudio_alsa.so \
		$(1)/usr/lib/qt5/plugins/audio
endef

define Package/qt5-drivers-customwidget/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/qt5/plugins/designer

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/designer/libcustomwidgetplugin.so \
		$(1)/usr/lib/qt5/plugins/designer
endef

define Package/qt5-drivers-worldtimeclock/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/qt5/plugins/designer

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/designer/libworldtimeclockplugin.so \
		$(1)/usr/lib/qt5/plugins/designer
endef

define Package/qt5-drivers-imageformats/install
	$(INSTALL_DIR) \
		$(1)/usr/lib/qt5/plugins/imageformats

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/imageformats/*.so \
		$(1)/usr/lib/qt5/plugins/imageformats/
endef

define Package/qt5-graphicaleffects/install
	$(INSTALL_DIR) $(1)/usr/lib/qt5/qml

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/qml/QtGraphicalEffects \
		$(1)/usr/lib/qt5/qml/
endef

define Package/qt5-gsttools/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/qt5/plugins

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libqgsttools_p.so* \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/qt5/plugins/mediaservice \
		$(1)/usr/lib/qt5/plugins
endef

$(eval $(call BuildPackage,qt5-core))
$(eval $(call BuildPackage,qt5-gui))
$(eval $(call BuildPackage,qt5-multimedia))
$(eval $(call BuildPackage,qt5-network))
$(eval $(call BuildPackage,qt5-qml))
$(eval $(call BuildPackage,qt5-remoteobjects))
$(eval $(call BuildPackage,qt5-quick))
$(eval $(call BuildPackage,qt5-quick-2))
$(eval $(call BuildPackage,qt5-quick-controls))
$(eval $(call BuildPackage,qt5-quick-controls-2))
$(eval $(call BuildPackage,qt5-quick-dialogs))
$(eval $(call BuildPackage,qt5-quick-extras))
$(eval $(call BuildPackage,qt5-quick-layouts))
$(eval $(call BuildPackage,qt5-quick-localstorage))
$(eval $(call BuildPackage,qt5-quick-particles))
$(eval $(call BuildPackage,qt5-quick-privatewidgets))
$(eval $(call BuildPackage,qt5-quick-scene3d))
$(eval $(call BuildPackage,qt5-quick-window))
$(eval $(call BuildPackage,qt5-quick-xmllistmodel))
$(eval $(call BuildPackage,qt5-sql))
$(eval $(call BuildPackage,qt5-test))
$(eval $(call BuildPackage,qt5-enginio))
$(eval $(call BuildPackage,qt5-widgets))
$(eval $(call BuildPackage,qt5-bluetooth))
$(eval $(call BuildPackage,qt5-concurrent))
$(eval $(call BuildPackage,qt5-dbus))
$(eval $(call BuildPackage,qt5-location))
$(eval $(call BuildPackage,qt5-nfc))
$(eval $(call BuildPackage,qt5-opengl))
$(eval $(call BuildPackage,qt5-positioning))
$(eval $(call BuildPackage,qt5-printsupport))
$(eval $(call BuildPackage,qt5-quickwidgets))
$(eval $(call BuildPackage,qt5-script))
$(eval $(call BuildPackage,qt5-scripttools))
$(eval $(call BuildPackage,qt5-sensors))
$(eval $(call BuildPackage,qt5-serialport))
$(eval $(call BuildPackage,qt5-svg))
$(eval $(call BuildPackage,qt5-webchannel))
$(eval $(call BuildPackage,qt5-webengine))
$(eval $(call BuildPackage,qt5-websocket))
$(eval $(call BuildPackage,qt5-xml))
$(eval $(call BuildPackage,qt5-xmlpatterns))
$(eval $(call BuildPackage,qt5-serialbus))
$(eval $(call BuildPackage,qt5-help))
$(eval $(call BuildPackage,qt5-clucene))
$(eval $(call BuildPackage,qt5-labstemplates))
$(eval $(call BuildPackage,qt5-quickparticles))
$(eval $(call BuildPackage,qt5-3d))
$(eval $(call BuildPackage,qt5-charts))
$(eval $(call BuildPackage,qt5-speech))
$(eval $(call BuildPackage,qt5-scxml))
$(eval $(call BuildPackage,qt5-purchasing))
$(eval $(call BuildPackage,qt5-datavis3d))
$(eval $(call BuildPackage,qt5-examples))
$(eval $(call BuildPackage,qt5-wayland))
$(eval $(call BuildPackage,qt5-gamepad))
$(eval $(call BuildPackage,qt5-drivers-mouse))
$(eval $(call BuildPackage,qt5-drivers-keyboard))
$(eval $(call BuildPackage,qt5-drivers-touchscreen))
$(eval $(call BuildPackage,qt5-drivers-linuxfb))
$(eval $(call BuildPackage,qt5-drivers-audio))
$(eval $(call BuildPackage,qt5-drivers-customwidget))
$(eval $(call BuildPackage,qt5-drivers-worldtimeclock))
$(eval $(call BuildPackage,qt5-drivers-imageformats))
$(eval $(call BuildPackage,qt5-graphicaleffects))
$(eval $(call BuildPackage,qt5-gsttools))
