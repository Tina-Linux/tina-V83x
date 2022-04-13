#
# Copyright (C) 2019-2O20 CZ.NIC, z. s. p. o. (https://www.nic.cz/)
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-plumbum
PKG_VERSION:=1.6.9
PKG_RELEASE:=2

PYPI_NAME:=plumbum
PKG_HASH:=16b9e19d96c80f2e9d051ef5f04927b834a6ac0ce5d2768eb8662b5cd53e43df

HOST_PYTHON3_PACKAGE_BUILD_DEPENDS:=setuptools-scm

include ../pypi.mk
include $(BUILD_DIR)/package.mk
include ../python3-package.mk

define Package/python3-plumbum
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=plumbum implementation in Python
  URL:=https://python-plumbum.github.io/
  DEPENDS:= \
	  +python3-light
endef

define Package/python3-plumbum/description
  A fast and complete Python implementation of plumbum.
endef

$(eval $(call Py3Package,python3-plumbum))
$(eval $(call BuildPackage,python3-plumbum))
$(eval $(call BuildPackage,python3-plumbum-src))
