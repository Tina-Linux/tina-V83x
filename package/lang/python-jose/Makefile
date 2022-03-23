#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-jose
PKG_VERSION:=1.3.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/51/e6/2b912c22275955ee5051aff888b2c363d8339b5887878e52ce7721d66659/python-jose-1.3.2.tar.gz
PKG_MD5SUM:=d1b7ce67e1ccca6d7f8c4105fbaa1a75

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Michael Davis

include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/python-jose
	SECTION:=lang-python
	CATEGORY:=Languages
	SUBMENU:=Python
	TITLE:=python-jose
	URL:=http://www.pyjose.org/
	DEPENDS:=+python +libgmp +python-crypto
endef

define Package/python-jose/description
The JavaScript Object Signing and Encryption (JOSE) technologies - JSON Web Signature (JWS), JSON Web Encryption (JWE), JSON Web Key (JWK), and JSON Web Algorithms (JWA) - collectively can be used to encrypt and/or sign content using a variety of algorithms. While the full set of permutations is extremely large, and might be daunting to some, it is expected that most applications will only use a small set of algorithms to meet their needs.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,\
		install --prefix=/usr --root=$(PKG_INSTALL_DIR),\
		CONFIG_BIG_ENDIAN="$(CONFIG_BIG_ENDIAN)" \
	)
endef

define Package/python-jose/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)/
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)/
endef

$(eval $(call PyPackage,python-jose))
$(eval $(call BuildPackage,python-jose))
