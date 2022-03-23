include $(TOPDIR)/rules.mk

PKG_NAME:=pulseaudio
PKG_VERSION:=11.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://freedesktop.org/software/pulseaudio/releases/
PKG_MD5SUM:=390de38231d5cdd6b43ada8939eb74f1

PKG_LICENSE:=LGPL-2.1+
PKG_LICENSE_FILES:=LICENSE

PKG_BUILD_DEPENDS:=intltool/host
PKG_FIXUP:=autoreconf

PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/pulseaudio/Default
  SECTION:=multimedia
  CATEGORY:=Multimedia
  SUBMENU:=PulseAudio
  TITLE:=PulseAudio
  URL:=http://www.pulseaudio.org
  DEPENDS:=+libsndfile +libatomic_ops +libspeexdsp +libltdl +libjson-c \
	  +alsa-lib +libcap +libopenssl +PACKAGE_eudev:eudev +PACKAGE_dbus:dbus
  USERID:=pulse=51:pulse=51
endef

define Package/pulseaudio/Default/description
  PulseAudio is a sound system for POSIX OSes, meaning that it is a proxy
  for your sound applications. It allows you to do advanced operations on
  your sound data as it passes between your application and your hardware.
endef

define Package/pulseaudio-daemon
  $(call Package/pulseaudio/Default)
  MENU:=1
endef

define Package/pulseaudio-daemon/description
  $(call Package/pulseaudio/Default/description)
endef

define Package/pulseaudio-daemon/config
	source "$(SOURCE)/Config.in"
endef

define Package/pulseaudio-utils
  SECTION:=multimedia
  CATEGORY:=Multimedia
  SUBMENU:=PulseAudio
  DEPENDS:=+pulseaudio-daemon
  TITLE:=Utilities for Pulseaudio
  URL:=http://www.pulseaudio.org
endef

define Package/pulseaudio-profiles
  SECTION:=multimedia
  CATEGORY:=Multimedia
  SUBMENU:=PulseAudio
  DEPENDS:=+pulseaudio-daemon
  TITLE:=Profiles for Pulseaudio
  URL:=http://www.pulseaudio.org
endef

CONFIGURE_ARGS += \
	--with-system-user=root \
	--with-system-group=root \
	--with-access-group=audio \
	--with-database=simple \
	--enable-alsa \
	--enable-udev=$(if $(filter $(CONFIG_PULSEAUDIO_DAEMON_ENABLE_UDEV),y),yes,no) \
	--enable-dbus=$(if $(filter $(CONFIG_PULSEAUDIO_DAEMON_ENABLE_DBUS),y),yes,no) \
	--enable-oss-output \
	--disable-oss-wrapper \
	--disable-hal-compat \
	--disable-gconf \
	--disable-nls \
	--disable-manpages \
	--disable-samplerate \
	--disable-per-user-esound-socket \
	--disable-solaris \
	--disable-gtk3 \
	--disable-glib2 \
	--disable-jack \
	--disable-asyncns \
	--disable-lirc \
	--disable-bluez4 \
	--disable-bluez5 \
	--disable-avahi \
	--disable-memfd \
	--without-fftw \
	--without-soxr

TARGET_CFLAGS += -std=gnu99
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lltdl -ldl)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/pulseaudio-daemon/install
	$(INSTALL_DIR) $(1)/etc/pulse
	$(CP) $(PKG_INSTALL_DIR)/etc/pulse/* $(1)/etc/pulse
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./init_scripts/pulseaudio.init $(1)/etc/init.d/pulseaudio
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pulseaudio $(1)/usr/bin/pulseaudio
	$(INSTALL_DIR) $(1)/usr/lib/pulseaudio
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pulseaudio/*.so* $(1)/usr/lib/pulseaudio/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pulse-$(PKG_VERSION)
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pulse-$(PKG_VERSION)/* \
		$(1)/usr/lib/pulse-$(PKG_VERSION)/
ifeq ($(CONFIG_PULSEAUDIO_DAEMON_ENABLE_UDEV),y)
	$(INSTALL_DIR) $(1)/lib/udev/rules.d
	$(CP) $(PKG_INSTALL_DIR)/lib/udev/rules.d/* $(1)/lib/udev/rules.d
endif
ifeq ($(CONFIG_PULSEAUDIO_DAEMON_ENABLE_DBUS),y)
	$(INSTALL_DIR) $(1)/etc/dbus-1/system.d
	$(CP) $(PKG_INSTALL_DIR)/etc/dbus-1/system.d/* $(1)/etc/dbus-1/system.d
endif
endef

define Package/pulseaudio-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/pa* $(1)/usr/bin/
endef

define Package/pulseaudio-profiles/install
	$(INSTALL_DIR) $(1)/usr/share/pulseaudio
	$(CP) $(PKG_INSTALL_DIR)/usr/share/pulseaudio/* $(1)/usr/share/pulseaudio/
endef

$(eval $(call BuildPackage,pulseaudio-daemon))
$(eval $(call BuildPackage,pulseaudio-utils))
$(eval $(call BuildPackage,pulseaudio-profiles))
