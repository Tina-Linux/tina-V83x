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
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=sepolicy-demo
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

HAVE_SRC := no
HAVE_SOURCE :=  $(wildcard src)
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
define Package/$(PKG_NAME)
  SECTION:=selinux
  CATEGORY:=Security
  SUBMENU:=SELINUX
  TITLE:=sepolicy-demo for Tina Linux
  DEPENDS:=@USE_SELINUX +selinux
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
	it comes form Android 4.4!!
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)

ifeq ($(HAVE_SOURCE), src)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endif

endef

define Build/Configure
endef

define Build/Compile/Source
	$(MAKE) -C $(PKG_BUILD_DIR) \
		M4="$(STAGING_DIR_HOST)/bin/m4" \
		CHECKPOLICY="$(STAGING_DIR_HOST)/usr/bin/checkpolicy" \
		all
endef

define Build/Compile

ifeq ($(HAVE_SOURCE), src)
	$(call Build/Compile/Source)
endif

endef

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one


# Specify where and how to install the program. Since we only have one file,
# the helloworld executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc
	$(INSTALL_DIR) $(1)/etc/selinux
	$(INSTALL_DIR) $(1)/etc/selinux/targeted
	$(INSTALL_DIR) $(1)/etc/selinux/targeted/policy
	$(INSTALL_DIR) $(1)/etc/selinux/targeted/contexts
	$(INSTALL_DIR) $(1)/etc/selinux/targeted/contexts/files
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/policy.[0-9]* $(1)/etc/selinux/targeted/policy/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/file_contexts $(1)/etc/selinux/targeted/contexts/files/
#	$(INSTALL_DATA) $(PKG_BUILD_DIR)/seapp_contexts $(1)/etc/selinux/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/config $(1)/etc/selinux/
endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(PKG_NAME)))
