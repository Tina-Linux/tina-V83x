##############################################
# OpenWrt Makefile for rotpk program
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
PKG_NAME:=optee-rotpk
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

PKG_CONFIG_DEPENDS += CONFIG_OPTEE_ENCRYPT_TA CONFIG_OPTEE_ENCRYPT_TA_SSK_KEY CONFIG_OPTEE_ENCRYPT_TA_ROTPK_DERIVE_KEY

PKG_FILE_DEPENDS += $(STAGING_DIR)/usr/dev_kit/arm-plat-$(TARGET_CHIP)/export-ta_arm32/keys/default_ta.pem
ifeq ($(CONFIG_OPTEE_ENCRYPT_TA),y)
PKG_FILE_DEPENDS += $(STAGING_DIR)/usr/dev_kit/arm-plat-$(TARGET_CHIP)/export-ta_arm32/keys/ta_aes_key.bin
endif

include $(BUILD_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Security
  SUBMENU:=OPTEE
  TITLE:=OPTEE rotpk read write
  DEPENDS:= +liballwinner-base +optee-client +optee-os-dev-kit
endef


# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
	rotpk read/write TA & CA demo source code!
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

# TA MUST be compiled with 32bit cross toolchain.
ifeq ($(CONFIG_ARCH_64BIT),y)
TARGET_CROSS_TA=$(subst aarch64,arm,$(CONFIG_TOOLCHAIN_ROOT)/bin/arm-openwrt-linux-)
else
TARGET_CROSS_TA:=$(TARGET_CROSS)
endif

define Build/Compile/Source
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE_TA="$(TARGET_CROSS_TA)" \
		CROSS_COMPILE_HOST="$(TARGET_CROSS)" \
		PLATFORM="$(TARGET_CHIP)" \
		TA_LDFLAGS="$(TARGET_LDFLAGS)"\
		DEV_KIT_DIR="$(STAGING_DIR)/usr/dev_kit"\
		CA_DEV_KIT_DIR="$(STAGING_DIR)/usr"
endef

define Build/Compile
	$(call Build/Compile/Source)
endef

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one


# Specify where and how to install the program. Since we only have one file,
# the rotpk executable, install it by copying it to the /bin directory on
# the router. The $(1) variable represents the root directory on the router running
# OpenWrt. The $(INSTALL_DIR) variable contains a command to prepare the install
# directory if it does not already exist.  Likewise $(INSTALL_BIN) contains the
# command to copy the binary file from its current location (in our case the build
# directory) to the install directory.
define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_BUILD_DIR)/na/librotpk.h $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/out/na/lib/librotpk.so $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/lib/optee_armtz

	$(CP) $(PKG_BUILD_DIR)/out/na/lib/librotpk.so $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/out/na/*_na $(1)/usr/bin
	$(CP) $(PKG_BUILD_DIR)/out/ta/*.ta $(1)/lib/optee_armtz
endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(PKG_NAME)))
