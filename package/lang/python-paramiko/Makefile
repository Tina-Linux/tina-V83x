#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-paramiko
PKG_VERSION:=1.18.1
PKG_RELEASE:=1
PKG_LICENSE:=LGPL

PKG_SOURCE:=paramiko-$(PKG_VERSION).zip

PKG_BUILD_DEPENDS:=+python/host +$(PKG_NAME)/host

PKG_BUILD_DIR:=$(COMPILE_DIR)/paramiko-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/paramiko-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python paramiko
  DEPENDS:=+python
  URL:=http://www.paramiko.org/
endef

define Package/$(PKG_NAME)/description
The leading native Python SSHv2 protocol library.
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
