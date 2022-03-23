#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-setuptools_scm
PKG_VERSION:=3.3.3
PKG_RELEASE:=1

PKG_SOURCE:=setuptools_scm-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.org/project/setuptools-scm/

HOST_BUILD_DEPENDS:=python python/host

PKG_BUILD_DIR:=$(COMPILE_DIR)/setuptools_scm-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/setuptools_scm-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)
$(call include_mk, python-host.mk)

define Package/$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=the blessed package to manage your versions by scm tags
  DEPENDS:=+python
endef

define Package/$(PKG_NAME)/description
  setuptools_scm handles managing your Python package versions in SCM metadata instead of declaring them as the version argument or in a SCM managed file.
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

define Host/Compile
	$(call Build/Compile/HostPyMod,,\
		install --root="$(OUT_DIR)/host" --prefix="" \
		--single-version-externally-managed \
	)
endef

define Host/Install
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

