#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=ecdsa
PKG_VERSION:=0.13
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/f9/e5/99ebb176e47f150ac115ffeda5fedb6a3dbb3c00c74a59fd84ddf12f5857/ecdsa-0.13.tar.gz#md5=1f60eda9cb5c46722856db41a3ae6670
PKG_MD5SUM:=1f60eda9cb5c46722856db41a3ae6670

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Brian Warner

include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/ecdsa
	SECTION:=lang-python
	CATEGORY:=Languages
	SUBMENU:=Python
	TITLE:=ecdsa
	URL:=http://www.pyecdsa.org/
	DEPENDS:=+python
endef

define Package/ecdsa/description
This is an easy-to-use implementation of ECDSA cryptography (Elliptic Curve
Digital Signature Algorithm), implemented purely in Python, released under
the MIT license. With this library, you can quickly create keypairs (signing
key and verifying key), sign messages, and verify the signatures. The keys
and signatures are very short, making them easy to handle and incorporate
into other protocols.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,\
		install --prefix=/usr --root=$(PKG_INSTALL_DIR),\
		CONFIG_BIG_ENDIAN="$(CONFIG_BIG_ENDIAN)" \
	)
endef

define Package/ecdsa/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)/
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)/
endef

$(eval $(call PyPackage,ecdsa))
$(eval $(call BuildPackage,ecdsa))
