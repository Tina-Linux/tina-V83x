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
PKG_NAME:=dm-verity
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
PKG_FILE_DEPENDS += $(TOPDIR)/package/security/dm-verity
HOST_BUILD_DIR := $(COMPILE_DIR_HOST)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/host-build.mk

PKG_BUILD_DEPENDS:=dm-verity/host
PKG_CONFIG_DEPENDS += CONFIG_BUSYBOX_INIT_BASE_FILES_PATH

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Security
  SUBMENU:=Device Mapper
  TITLE:=dm-verity
  DEPENDS:=+@USE_DM_VERITY +liballwinner-base +cryptsetup
endef


# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
	This package contains keys and apps for dm-verity.
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	rm -rf $(TARGET_OUT_DIR)/verity
	mkdir -p $(TARGET_OUT_DIR)/verity/keys
	cp ./files/* $(TARGET_OUT_DIR)/verity/keys
	cp ./src/* $(PKG_BUILD_DIR)/
endef

define Host/Prepare
	cp ./src/* $(HOST_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		parse_squashfs_verity
endef

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) generate_squashfs_verity
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
	$(INSTALL_DIR) $(1)
	$(INSTALL_DIR) $(1)/usr
	$(INSTALL_DIR) $(1)/usr/bin
ifeq ($(findstring ramfs,$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)),ramfs)
	$(CP) ./files/dm-verity-pub.pem $(1)/verity_key
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/parse_squashfs_verity $(1)/usr/bin/
endif
endef

define Host/Install
	$(INSTALL_BIN) $(HOST_BUILD_DIR)/generate_squashfs_verity $(STAGING_DIR_HOST)/bin
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.
$(eval $(call HostBuild))
$(eval $(call BuildPackage,$(PKG_NAME)))
