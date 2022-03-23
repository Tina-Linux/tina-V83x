#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-empy
PKG_VERSION:=3.3.3
PKG_RELEASE:=1
PKG_LICENSE:=LGPL

PKG_SOURCE:=empy-latest.tar.gz
PKG_SOURCE_URL:=https://pypi.org/project/EmPy/

PKG_BUILD_DEPENDS:=python/host PACKAGE_python-empy:python-empy/host

PKG_BUILD_DIR:=$(COMPILE_DIR)/empy-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/empy-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python empy
  URL:=https://pypi.org/project/EmPy/
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
	$(info ************** host do not need to execute install **************)
endef

define Host/Install
	$(INSTALL_DIR) $(STAGING_DIR_HOST)/lib

	$(CP) $(HOST_BUILD_DIR)/em.py $(STAGING_DIR_HOST)/lib/python$(PYTHON_VERSION)/site-packages
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define PyPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
