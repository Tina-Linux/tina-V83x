#
# Copyright (C) 2008-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org

include $(TOPDIR)/rules.mk

PKG_NAME:=libepoxy
PKG_VERSION:=v1.3.1
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=
PKG_MD5SUM:=

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_BUILD_PARALLEL:=0
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)-1.3.1

include $(BUILD_DIR)/package.mk

TARGET_LDFLAGS+= \
	-Wl,-rpath-link=$(STAGING_DIR)/usr/lib

define Package/libepoxy
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=epoxy
  DEPENDS:=+util-macros
  URL:=http://www.gtk.org/
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r16 r40 r18 r311))
	DEPENDS+= +mali400-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r30))
	DEPENDS+= +mali-t760-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r58))
	DEPENDS+= +sgx544-um
endif
endef

define Package/libepoxy/description
Gdk-Pixbuf is an image loader and scaler. It uses GObject
and the GLib, to integrate well with GNOME applications.
endef

CONFIGURE_ARGS += \
	--enable-egl \
	--disable-glx

TARGET_LDFLAGS+= \
	-L$(STAGING_DIR)/usr/lib/

CONFIGURE_VARS += \
	CXX="$(TARGET_CC)"

define Build/Configure
	( cd $(PKG_BUILD_DIR); ./autogen.sh )
		$(call Build/Configure/Default)
endef

define Build/InstallDev
	$(INSTALL_DIR) \
		$(1)/usr/lib/pkgconfig \
		$(1)/usr/include

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib

	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/* \
		$(1)/usr/include/
endef

define Package/libepoxy/install
	$(INSTALL_DIR) \
		$(1)/usr/lib

	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/*.so* \
		$(1)/usr/lib/
endef

$(eval $(call BuildPackage,libepoxy))
