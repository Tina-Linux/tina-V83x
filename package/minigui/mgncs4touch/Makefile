#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mgncs4touch
PKG_VERSION:=1.2.0
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
#PKG_MD5SUM:=9227acf41d2d8810e5c4ece042f1f175

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/${PKG_NAME}
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libmgplus +libmgeff +libmgncs +libchipmunk
  TITLE:=mgncs4touch
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/$(PKG_NAME)/description
endef

CONFIGURE_ARGS += \

ifeq ($(CONFIG_TSLIB_SUPPORT),y)
TARGET_LDFLAGS+= -lts
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lts

endif

ifeq ($(CONFIG_G2D_SUPPORT),y)
TARGET_LDFLAGS+= -luapi
LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -luapi
endif
define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgncs4touch \
		$(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgncs4touch*.{a,la,so*} \
		$(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mgncs4touch.pc \
		$(1)/usr/lib/pkgconfig/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig
	$(INSTALL_DIR) $(1)/usr/include/mgncs4touch
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/libmgncs4touch*.{a,la,so*} \
		$(1)/usr/lib/
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/include/mgncs4touch/* \
		$(1)/usr/include/mgncs4touch
	$(CP) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/mgncs4touch.pc \
		$(1)/usr/lib/pkgconfig
	$(CP) \
		$(PKG_BUILD_DIR)/etc/mgncs4touch.cfg \
		$(1)/usr/local/etc
endef

$(eval $(call BuildPackage,${PKG_NAME}))
