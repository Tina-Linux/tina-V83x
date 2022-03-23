#
# Copyright (C) 2009-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=paho-mqtt
PKG_VERSION:=1.2.2
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://pypi.python.org/packages/31/0f/53f71dc9033b492ba6546c452c35722621f9120cee6b905808d893fa0d97/paho-mqtt-1.2.2.tar.gz
PKG_MD5SUM:=e48a835a02d0f6c2cdec10d3a6c99e5e

PKG_LICENSE:=Eclipse
PKG_LICENSE_FILES:=LICENSE
PKG_MAINTAINER:=Roger Light

include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/paho-mqtt
	SECTION:=lang-python
	CATEGORY:=Languages
	SUBMENU:=Python
	TITLE:=paho-mqtt
	URL:=http://www.pyjose.org/
	DEPENDS:=+python +libgmp +python-crypto
endef

define Package/paho-mqtt/description
The JavaScript Object Signing and Encryption (JOSE) technologies - JSON Web Signature (JWS), JSON Web Encryption (JWE), JSON Web Key (JWK), and JSON Web Algorithms (JWA) - collectively can be used to encrypt and/or sign content using a variety of algorithms. While the full set of permutations is extremely large, and might be daunting to some, it is expected that most applications will only use a small set of algorithms to meet their needs.
endef

define Build/Compile
	$(call Build/Compile/PyMod,,\
		install --prefix=/usr --root=$(PKG_INSTALL_DIR),\
		CONFIG_BIG_ENDIAN="$(CONFIG_BIG_ENDIAN)" \
	)
endef

define Package/paho-mqtt/install
	$(INSTALL_DIR) $(1)$(PYTHON_PKG_DIR)/
	$(CP) \
		$(PKG_INSTALL_DIR)$(PYTHON_PKG_DIR)/* \
		$(1)$(PYTHON_PKG_DIR)/
endef

$(eval $(call PyPackage,paho-mqtt))
$(eval $(call BuildPackage,paho-mqtt))
