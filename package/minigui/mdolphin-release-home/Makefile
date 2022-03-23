#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=mdolphin-release-home
PKG_VERSION:=2.0.4
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
#PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
PKG_SOURCE_URL:=http://www.minigui.org/downloads/
PKG_MD5SUM:=2b294448049370d9ef8136c699545688

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

PKG_FIXUP:=patch-libtool

include $(BUILD_DIR)/package.mk

define Package/${PKG_NAME}
  SECTION:=gui
  CATEGORY:=Minigui
  DEPENDS:= +libminigui-gpl +libmdolphin +libmgutils +libmgplus +libcurl
  TITLE:=mdolphin-release-home
  MAINTAINER:=tracewong <tracewong@outlook.com>
  URL:=http://www.minigui.org/
endef

define Package/${PKG_NAME}/description
  The minigui mdolphin test case
endef

define Build/Prepare
	$(call Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef

CONFIGURE_ARGS += \
        --with-lang=zhcn \
        --disable-silent-rules \

LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lfreetype -lts
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(CP) \
		$(PKG_BUILD_DIR)/src/mgi/include \
		$(1)/usr/include/
endef
define Package/${PKG_NAME}/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(CP) \
		$(PKG_BUILD_DIR)/src/mdhome \
		$(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/res/
	$(CP) \
		$(PKG_BUILD_DIR)/src/res/* \
		$(1)/usr/res/
	$(INSTALL_DIR) $(1)/usr/res-800/
	$(CP) \
		$(PKG_BUILD_DIR)/src/res-800/* \
		$(1)/usr/res-800/
ifeq ($(TARGET_BOARD_PLATFORM),r11)
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) \
		./MiniGUI.cfg \
		$(1)/usr/local/etc/
endif
ifeq ($(TARGET_BOARD_PLATFORM),r7)
	$(INSTALL_DIR) $(1)/usr/local/etc
	$(CP) \
		./MiniGUI.cfg \
		$(1)/usr/local/etc/
endif
endef
$(eval $(call BuildPackage,${PKG_NAME}))
