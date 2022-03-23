#
# Copyright (C) 2015-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=imagesize
PKG_VERSION:=1.1.0
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_BUILD_DEPENDS:=python $(PKG_NAME)/host
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/$(PKG_NAME)-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-$(PKG_NAME)
	SECTION:=lang
	CATEGORY:=Languages
	SUBMENU:=Python
	TITLE:=python-imagesize
	URL:=https://pypi.org/project/imagesize
endef

define Package/python-$(PKG_NAME)/description
Foreign
endef

define Build/Compile
	$(call Build/Compile/PyMod,,\
		install --prefix="/usr" --root="$(PKG_INSTALL_DIR)")
endef

define Host/Compile
	$(call Build/Compile/HostPyMod,,\
		install --root="/" --prefix="$(OUT_DIR)/host")
endef

define Host/Install
	$(info ************** host do not need to execute install **************)
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,python-$(PKG_NAME)))
$(eval $(call BuildPackage,python-$(PKG_NAME)))
