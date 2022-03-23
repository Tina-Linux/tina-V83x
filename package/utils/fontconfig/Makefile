#
# Copyright (C) 2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=fontconfig
PKG_VERSION:=2.12.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_SOURCE_URL:=http://fontconfig.org/release/
PKG_MD5SUM:=b5af5a423ee3b5cfc34846838963c058

PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/fontconfig
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Fontconfig, configure and customize font access
  DEPENDS:=+libfreetype +libexpat +libpthread
  URL:=http://fontconfig.org
endef

define Package/fontconfig/description
	The Fontconfig package contains a library and support programs used for
        configuring and customizing font access.
endef

CONFIGURE_ARGS+= --disable-docs

define Package/fontconfig/install
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib $(1)/usr/share/fontconfig/conf.avail
	$(INSTALL_DIR) $(1)/etc/fonts/conf.d
	$(CP) $(PKG_INSTALL_DIR)/etc/fonts/fonts.conf $(1)/etc/fonts
	$(CP) $(PKG_INSTALL_DIR)/etc/fonts/conf.d/* $(1)/etc/fonts/conf.d
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/share/fontconfig/conf.avail/* $(1)/usr/share/fontconfig/conf.avail
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/fontconfig.init $(1)/etc/init.d/fontconfig
endef

define Package/fontconfig/postinst
#!/bin/sh
# check if we are on a real system
if [ -z "$${IPKG_INSTROOT}" ]; then
   echo "Enabling rc.d symlink for fontconfig cache rebuild"
   /etc/init.d/fontconfig enable
   /usr/bin/fc-cache --force --verbose
fi
exit 0
endef

define Package/fontconfig/prerm
#!/bin/sh
# check if we are on a real system
if [ -z "$${IPKG_INSTROOT}" ]; then
   echo "Removing rc.d symlink for fontconfig"
   /etc/init.d/fontconfig disable
fi
exit 0
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/include $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,fontconfig))
