#
# Copyright (C) 2007-2014 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=python-sip
PKG_VERSION:=4.19.13
PKG_RELEASE:=1
PKG_LICENSE:=GPL v2

PKG_SOURCE:=sip-$(PKG_VERSION).tar.gz

PKG_BUILD_DEPENDS:=python $(PKG_NAME)/host

PKG_BUILD_DIR:=$(COMPILE_DIR)/sip-$(PKG_VERSION)
HOST_BUILD_DIR:=$(COMPILE_DIR_HOST)/sip-$(PKG_VERSION)

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk
$(call include_mk, python-package.mk)

define Package/$(PKG_NAME)
  SUBMENU:=Python
  SECTION:=lang
  CATEGORY:=Languages
  TITLE:=python sip
  DEPENDS:=+python +libstdcpp
  URL:=https://pypi.org/project/SIP/
endef

define Package/$(PKG_NAME)/description
Python extension module generator for C and C++ libraries
endef

define Host/Compile
	$(call HostPython, \
		cd $(HOST_BUILD_DIR); \
		CC="$(HOSTCC)" \
		CCSHARED="$(HOSTCC) $(HOST_FPIC)" \
		CXX="$(HOSTCXX)" \
		LD="$(HOSTCC)" \
		LDSHARED="$(HOSTCC) -shared" \
		CFLAGS="$(HOST_CFLAGS)" \
		CPPFLAGS="$(HOST_CPPFLAGS) -I$(HOST_PYTHON_INC_DIR)" \
		LDFLAGS="$(HOST_LDFLAGS) -lpython$(PYTHON_VERSION) -Wl$(comma)-rpath=$(OUT_DIR)/host/lib" \
		_PYTHON_HOST_PLATFORM=linux2 \
		, \
		./configure.py --sysroot="$(OUT_DIR)/host/" \
			-d $(OUT_DIR)/host/lib/python$(PYTHON_VERSION)/site-packages \
		, \
		HOST \
	)
	$(call Host/Compile/Default)
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
		STRIP="$(TARGET_CROSS)strip" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -I$(PYTHON_INC_DIR)" \
		LDFLAGS="$(TARGET_LDFLAGS) -lpython$(PYTHON_VERSION)" \
		_PYTHON_HOST_PLATFORM=linux2 \
		__PYVENV_LAUNCHER__="/usr/bin/$(PYTHON)" \
		, \
		./configure.py --sysroot=$(PKG_INSTALL_DIR) \
			-d $(PKG_INSTALL_DIR)/lib/$(PYTHON)/site-packages \
			STRIP="$(TARGET_CROSS)strip" \
		, \
	)
	find $(PKG_INSTALL_DIR) -name "*\.pyc" -o -name "*\.pyo" | xargs rm -f

	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CCSHARED="$(TARGET_CC) $(FPIC)" \
		CXX="$(TARGET_CXX)" \
		LINK="$(TARGET_CXX)" \
		LD="$(TARGET_CC)" \
		LDSHARED="$(TARGET_CC) -shared" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -I$(PYTHON_INC_DIR)" \
		LDFLAGS="$(TARGET_LDFLAGS) -lpython$(PYTHON_VERSION)" \
		all install
endef

define Host/Install
	$(info ************** host do not need to execute install **************)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/* $(1)/usr/lib
endef

define PyPackage/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call HostBuild))

$(eval $(call PyPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)))
