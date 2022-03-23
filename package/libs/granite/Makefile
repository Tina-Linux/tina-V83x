#
# Copyright (C) 2006 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# blogic@openwrt.org

include $(TOPDIR)/rules.mk
PKG_NAME:=granite
PKG_VERSION:=0.3.1
PKG_RELEASE:=1
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_URL:=https://launchpad.net/granite/0.3/0.3.1/+download/
STAGING_DIR_HOSTPKG:=$(STAGING_DIR)/host

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/granite
	SECTION:=libs
	CATEGORY:=Libraries
	TITLE:=Accesibility library for gtk+ apps
	DEPENDS:=+glib2 +libgee +libgtk3
endef


define Build/InstallDev
	$(INSTALL_DIR) $(STAGING_DIR)/usr/{lib,include}
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib* $(STAGING_DIR)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(STAGING_DIR)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig/
	$(INSTALL_DATA) \
		$(PKG_INSTALL_DIR)/usr/lib/pkgconfig/*.pc \
		$(1)/usr/lib/pkgconfig/
	$(INSTALL_DIR) $(STAGING_DIR_HOSTPKG)/share/vala-0.34/vapi/
	$(INSTALL_DATA) \
                $(PKG_INSTALL_DIR)/usr/share/vala/vapi/* \
                $(STAGING_DIR_HOSTPKG)/share/vala-0.34/vapi
endef

define Package/granite/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*so* $(1)/usr/lib/
endef
$(eval $(call BuildPackage,granite))
