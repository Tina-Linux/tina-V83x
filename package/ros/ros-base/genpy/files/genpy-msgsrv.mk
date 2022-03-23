#
# Copyright (C) 2015-2019 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

$(call include_mk, python-package.mk)

# $(1) => build subdir
# $(2) => ros package the generated msg files belongs to
# $(3) => directory in which to place output files
# $(4) => msg files
# $(5) => include path to search for msg files
# $(6) => additional variables
define Build/Compile/MsgToPy
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/lib/python$(PYTHON_VERSION)/site-packages/$(strip $(2))/$(strip $(3)); \
	$(call HostPython, \
		cd $(PKG_BUILD_DIR)/$(strip $(1)); \
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
		$(6) \
		, \
		$(STAGING_DIR)/usr/lib/genpy/genmsg_py.py \
			-p $(2) \
			-o $(PKG_INSTALL_DIR)/lib/python$(PYTHON_VERSION)/site-packages/$(strip $(2))/$(strip $(3)) \
			-I $(4) \
			$(5) \
	)
endef

# $(1) => build subdir
# $(2) => ros package the generated srv files belongs to
# $(3) => directory in which to place output files
# $(4) => srv files
# $(5) => include path to search for srv files
# $(6) => additional variables
define Build/Compile/SrvToPy
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/lib/python$(PYTHON_VERSION)/site-packages/$(strip $(2))/$(strip $(3)); \
	$(call HostPython, \
		cd $(PKG_BUILD_DIR)/$(strip $(1)); \
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
		$(6) \
		, \
		$(STAGING_DIR)/usr/lib/genpy/gensrv_py.py \
			-p $(2) \
			-o $(PKG_INSTALL_DIR)/lib/python$(PYTHON_VERSION)/site-packages/$(strip $(2))/$(strip $(3)) \
			-I $(4) \
			$(5) \
	)
endef
