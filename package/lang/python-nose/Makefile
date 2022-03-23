#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-nose
PKG_VERSION:=1.3.7
PKG_RELEASE:=1
PKG_LICENSE:=LGPL

PKG_SOURCE:=nose-$(PKG_VERSION).tar.gz

PKG_BUILD_DEPENDS:=+python/host +$(PKG_NAME)/host

PKG_BUILD_DIR:=$(COMPILE_DIR)/nose-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/nose-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python nose
  URL:=https://pypi.org/project/nose/
  DEPENDS:=+python
endef

define Package/$(PKG_NAME)/description
  A powerful and robust templating system for Python.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,\
		install --root="$(PKG_INSTALL_DIR)" --prefix="/usr" \
	)
endef

define Host/Compile
	$(call Build/Compile/HostPyMod,,\
		install --root="/" --prefix="$(OUT_DIR)/host" \
	)
endef

define Host/Install
	$(INSTALL_DIR) $(OUT_DIR)/host/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/* $(OUT_DIR)/host/bin/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/man

	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/man/* $(1)/usr/man
endef

define PyPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/man

	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/man/* $(1)/usr/man
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
