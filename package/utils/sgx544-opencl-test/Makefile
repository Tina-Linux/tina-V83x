##############################################
# OpenWrt Makefile for helloworld program
#
#
# Most of the variables used here are defined in
# the include directives below. We just need to
# specify a basic description of the package,
# where to build our program, where to find
# the source files, and where to install the
# compiled program on the router.
#
# Be very careful of spacing in this file.
# Indents should be tabs, not spaces, and
# there should be no trailing whitespace in
# lines that are not commented.
#
##############################################
include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:= sgx544-opencl-test
PKG_VERSION:=0.0.1
PKG_RELEASE:=1


# This specifies the directory where we're going to build the program. 
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)


include $(BUILD_DIR)/package.mk
 
# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/sgx544-opencl-test
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=SGX544 OpenCL test
  DEPENDS:=+kmod-sgx544-km +sgx544-um
endef


# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/sgx544-opencl-test/description
	SGX544 OpenCL test
endef
 
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/bin
	$(CP) bin/* $(PKG_BUILD_DIR)/bin
endef

define Build/Configure
endef

define Build/Compile
endef

define Package/sgx544-opencl-test/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,sgx544-opencl-test))
