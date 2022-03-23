#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-pyyaml
PKG_VERSION:=4.1

PKG_SOURCE:=pyyaml-$(PKG_VERSION).zip

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DEPENDS:=python

HOST_BUILD_PARALLEL:=1
PKG_BUILD_DIR:=$(COMPILE_DIR)/pyyaml-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/pyyaml-$(PKG_VERSION)

PKG_BUILD_DEPENDS:=$(PKG_NAME)/host

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  DEPENDS:=+python
  TITLE:=Canonical source repository for PyYAML
  URL:=https://github.com/yaml/pyyaml/tree/4.1
endef

define Package/$(PKG_NAME)/description
PyYAML - The next generation YAML parser and emitter for Python.
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
	$(INSTALL_DIR) $(STAGING_DIR)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(STAGING_DIR)/usr/lib
endef

define PyPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
