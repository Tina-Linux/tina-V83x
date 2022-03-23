#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-netifaces
PKG_VERSION:=0.10.9
PKG_RELEASE:=1
PKG_LICENSE:=MIT

PKG_SOURCE:=netifaces-$(PKG_VERSION).tar.gz

PKG_BUILD_DEPENDS:=+python/host +$(PKG_NAME)/host

PKG_BUILD_DIR:=$(COMPILE_DIR)/netifaces-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/netifaces-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python netifaces
  DEPENDS:=+python
  URL:=https://pypi.org/project/netifaces/
endef

define Package/$(PKG_NAME)/description
Portable network interface information.
endef

define Build/Compile
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/$(PYTHON_PKG_DIR)
	$(call HostPython, \
		cd $(PKG_BUILD_DIR); \
		CC="$(TARGET_CC)" \
		CCSHARED="$(TARGET_CC) $(FPIC)" \
		CXX="$(TARGET_CXX)" \
		LD="$(TARGET_CC)" \
		LDSHARED="$(TARGET_CC) -shared" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -I$(PYTHON_INC_DIR)" \
		LDFLAGS="$(TARGET_LDFLAGS) -lpython$(PYTHON_VERSION)" \
		_PYTHON_HOST_PLATFORM=linux2 \
		__PYVENV_LAUNCHER__="/usr/bin/$(PYTHON)" \
		, \
		./setup.py install --root="$(PKG_INSTALL_DIR)" --prefix="/usr"  \
		, HOST \
	)
	find $(PKG_INSTALL_DIR) -name "*\.pyc" -o -name "*\.pyo" | xargs rm -f
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
