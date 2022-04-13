#
# Copyright (C) 2017 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=python-spidev
PKG_VERSION:=3.5
PKG_RELEASE:=2

PYPI_NAME:=spidev

include ../pypi.mk
include $(BUILD_DIR)/package.mk
include ../python3-package.mk

define Package/python3-spidev
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python3-spidev
  URL:=https://github.com/doceme/py-spidev
  DEPENDS:= \
      +python3-light \
      +python3-ctypes
endef

define Package/python3-spidev/description
	Bindings to the Linux input handling subsystem
endef

PYTHON3_PKG_SETUP_GLOBAL_ARGS:= \
	build_ext

$(eval $(call Py3Package,python3-spidev))
$(eval $(call BuildPackage,python3-spidev))
$(eval $(call BuildPackage,python3-spidev-src))
