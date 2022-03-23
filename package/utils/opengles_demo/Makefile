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
PKG_NAME:= opengles_demo
PKG_VERSION:=0.0.1
PKG_RELEASE:=1


# This specifies the directory where we're going to build the program. 
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)


include $(BUILD_DIR)/package.mk

ifeq ($(TARGET_BOARD_PLATFORM),r58 mr813)
  TARGET_CFLAGS+=-DIMG_GPU
endif

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/opengles_demo
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=OpenGLES demo
ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r16 r40 r18 r311 t7))
  DEPENDS:= +mali400-um
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r30))
  DEPENDS:= +mali-t760-um
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r58))
  DEPENDS:= +sgx544-um
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), mr813))
  DEPENDS:= +ge8300-um
endif
endef


# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/opengles_demo/description
	If you can't figure out what this program does, you're probably
	brain-dead and need immediate medical attention.
endef
 
# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

TARGET_LDFLAGS+=-lm -ldl
define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" -Wall \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LIBS="-lEGL -lGLESv2" \
		all
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
define Package/opengles_demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/opengles_demo $(1)/usr/bin/
endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,opengles_demo))
