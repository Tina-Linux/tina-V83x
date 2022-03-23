#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-setuptools
PKG_VERSION:=41.4.0
PKG_RELEASE:=1

PKG_SOURCE:=setuptools-$(PKG_VERSION).zip
PKG_SOURCE_URL:=https://pypi.org/project/setuptools/

HOST_BUILD_DEPENDS:=python python/host

PKG_BUILD_DIR:=$(COMPILE_DIR)/setuptools-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/setuptools-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)
$(call include_mk, python-host.mk)

define Package/python-setuptools
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=Tool for installing Python packages.
  URL:=https://bitbucket.org/pypa/setuptools
  DEPENDS:=+python +ca-certificates
  MAINTAINER:=Alexandru Ardelean <ardeleanalex@gmail.com>
endef

define Package/python-setuptools/description
  Easily download, build, install, upgrade, and uninstall Python packages
endef

define Build/Compile
	$(call Build/Compile/PyMod,,\
		install --root="$(PKG_INSTALL_DIR)" --prefix="/usr" \
		--single-version-externally-managed \
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(PYTHON_LIB_DIR)
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(PYTHON_LIB_DIR)
endef

define PyPackage/python-setuptools/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

define Host/Compile
	$(call Build/Compile/HostPyMod,,\
		install --root="$(OUT_DIR)/host" --prefix="" \
		--single-version-externally-managed \
	)
endef

define Host/Install
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,python-setuptools))
$(eval $(call BuildPackage,python-setuptools))

