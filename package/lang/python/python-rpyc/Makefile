#
# Copyright (C) 2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=rpyc
PKG_VERSION:=5.0.1
PKG_RELEASE:=3

PKG_SOURCE:=$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/tomerfiliba-org/rpyc/archive/
# PKG_HASH:=0db042011bffcbd65362b67eb3cca87eaefa9f2a55b747fa75e922c706b8ce1a

include ../pypi.mk
include $(BUILD_DIR)/package.mk
include ../python3-package.mk

define Package/python3-rpyc
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=rpyc
  URL:=https://pypi.org/project/rpyc
  DEPENDS:= \
	  +python3 \
    +python3-plumbum
endef

define Package/python3-rpyc/description
 rpyc
endef

$(eval $(call Py3Package,python3-rpyc))
$(eval $(call BuildPackage,python3-rpyc))
$(eval $(call BuildPackage,python3-rpyc-src))
