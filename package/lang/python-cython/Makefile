#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-cython
PKG_VERSION:=0.29.2
PKG_RELEASE:=1
PKG_LICENSE:=Apache

PKG_SOURCE:=Cython-$(PKG_VERSION).tar.gz

PKG_BUILD_DEPENDS:=python $(PKG_NAME)/host

PKG_BUILD_DIR:=$(COMPILE_DIR)/Cython-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/Cython-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python Cython
  DEPENDS:= +python
  URL:=https://cython.org/
endef

define Package/$(PKG_NAME)/description
Cython is an optimising static compiler for both the Python programming language and the extended Cython programming language (based on Pyrex).
endef

define Host/Compile
	$(call Build/Compile/HostPyMod,,\
		install --root="/" --prefix="$(OUT_DIR)/host" \
	)
endef

define Build/Compile
	$(call Build/Compile/PyMod,,\
		install --root="$(PKG_INSTALL_DIR)" --prefix="/usr" \
	)
endef

define Host/Install
	$(INSTALL_DIR) $(OUT_DIR)/host/bin/
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/bin/* $(OUT_DIR)/host/bin/
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define PyPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
