include $(TOPDIR)/rules.mk

PKG_NAME:=bluez-alsa
PKG_VERSION:=20180913
PKG_RELEASE:=1
PKG_MAINTAINER:=
PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE.txt

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)-20180913

PKG_FIXUP:=autoreconf

include $(BUILD_DIR)/package.mk

define Package/bluez-alsa
  SECTION:=multimedia
  CATEGORY:=Multimedia
  DEPENDS:=+alsa-lib +bluez-daemon +glib2 +sbc +fdk-aac
  TITLE:=Bluetooth Audio ALSA backend
  URL:=https://github.com/Arkq/bluez-alsa
endef

define Package/bluez-alsa/description
  ALSA plugin supporting Bluetooth audio
endef

CONFIGURE_ARGS +=

define Package/bluez-alsa/install
	$(INSTALL_DIR) $(1)/usr/share/alsa/alsa.conf.d/
	$(CP) $(PKG_BUILD_DIR)/src/asound/20-bluealsa.conf $(1)/usr/share/alsa/alsa.conf.d/
	$(INSTALL_DIR) $(1)/usr/lib/alsa-lib
	$(CP) $(PKG_BUILD_DIR)/src/asound/.libs/libasound_module_*_bluealsa.so $(1)/usr/lib/alsa-lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/bluealsa $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/utils/bluealsa-aplay $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d
endef

$(eval $(call BuildPackage,bluez-alsa))
