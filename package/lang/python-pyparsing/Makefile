#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyparsing
PKG_VERSION:=2.3.0

PKG_SOURCE:=pyparsing-pyparsing_$(PKG_VERSION).zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_LICENSE:=MIT

PKG_BUILD_DEPENDS:=python

HOST_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(COMPILE_DIR)/pyparsing-pyparsing_$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/pyparsing-pyparsing_$(PKG_VERSION)

PKG_BUILD_DEPENDS:=python $(PKG_NAME)/host
HOST_BUILD_DEPENDS:=python/host

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  DEPENDS:=+python
  TITLE:=A Python Parsing Module
  URL:=https://github.com/pyparsing/pyparsing/tree/pyparsing_2.3.0
endef

define Package/$(PKG_NAME)/description
The pyparsing module is an alternative approach to creating and executing simple grammars, vs. the traditional lex/yacc approach, or the use of regular expressions. The pyparsing module provides a library of classes that client code uses to construct the grammar directly in Python code.
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
	$(info ************** host do not need to execute install **************)
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
