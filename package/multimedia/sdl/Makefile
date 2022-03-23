include $(TOPDIR)/rules.mk

PKG_NAME:=sdl
PKG_VERSION:=2.0.10
PKG_RELEASE:=1

PKG_SOURCE:=SDL2-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://www.libsdl.org/release
PKG_MD5SUM:=5a2114f2a6f348bdab5bf52b994811db

PKG_LICENSE:=zlib
PKG_LICENSE_FILES:=COPYING.txt

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/SDL2-$(PKG_VERSION)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=multimedia
  CATEGORY:=Multimedia
  TITLE:=Simple DirectMedia Layer
  URL:=https://libsdl.org/
endef

define Package/$(PKG_NAME)/decription
  Simple DirectMedia Layer is a cross-platform development library
  designed to provide low level access to audio, keyboard, mouse,
  joystick, and graphics hardware via OpenGL and Direct3D. It is used
  by video playback software, emulators, and popular games including
  Valve's award winning catalog and many Humble Bundle games.
endef

CONFIGURE_ARGS += \
		--enable-static=no \
		--disable-esd \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
