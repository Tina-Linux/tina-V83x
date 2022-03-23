##############################################
# OpenWrt Makefile for tinacvr program
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
TARGET=tinacvr

# Name and release number of this package
PKG_NAME:=$(TARGET)
PKG_VERSION:=1.0
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
define Package/$(TARGET)
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:= +libminigui-gpl +libcedarx +alsa-utils +libuapi
  TITLE:=tina cvr project

endef

ifeq ($(TARGET_BOARD_PLATFORM),r16)
	CBOARD = CONFIG_R16
endif

ifeq ($(TARGET_BOARD_PLATFORM),r6)
	CBOARD = CONFIG_R6
endif

ifeq ($(TARGET_BOARD_PLATFORM),r40)
	CBOARD = CONFIG_R40
endif

ifeq ($(TARGET_BOARD_PLATFORM),r11)
	CBOARD = CONFIG_R11
endif

ifeq ($(TARGET_BOARD_PLATFORM),r7)
	CBOARD = CONFIG_R7
endif

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/src
	mkdir -p $(PKG_BUILD_DIR)/res
	mkdir -p $(PKG_BUILD_DIR)/res/cfg
	mkdir -p $(PKG_BUILD_DIR)/res/data
	mkdir -p $(PKG_BUILD_DIR)/res/font
	mkdir -p $(PKG_BUILD_DIR)/res/lang
	mkdir -p $(PKG_BUILD_DIR)/res/menu
	mkdir -p $(PKG_BUILD_DIR)/res/others
	mkdir -p $(PKG_BUILD_DIR)/res/topbar
	mkdir -p $(PKG_BUILD_DIR)/res/sound
	mkdir -p $(PKG_BUILD_DIR)/res/wm_res
	mkdir -p $(PKG_BUILD_DIR)/res/bin
	mkdir -p $(PKG_BUILD_DIR)/res/minigui
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/src
	#$(CP) -r ./res/* $(PKG_BUILD_DIR)/res/
	$(CP) -r ./res/cfg/* $(PKG_BUILD_DIR)/res/cfg/
	$(CP) -r ./res/data/* $(PKG_BUILD_DIR)/res/data/
	$(CP) -r ./res/font/* $(PKG_BUILD_DIR)/res/font/
	$(CP) -r ./res/lang/* $(PKG_BUILD_DIR)/res/lang/
	$(CP) -r ./res/menu/* $(PKG_BUILD_DIR)/res/menu/
	$(CP) -r ./res/others/* $(PKG_BUILD_DIR)/res/others/
	$(CP) -r ./res/topbar/* $(PKG_BUILD_DIR)/res/topbar/
	$(CP) -r ./res/sound/* $(PKG_BUILD_DIR)/res/sound/
	$(CP) -r ./res/wm_res/* $(PKG_BUILD_DIR)/res/wm_res/
	$(CP) -r ./res/bin/* $(PKG_BUILD_DIR)/res/bin/
	$(CP) -r ./res/minigui/* $(PKG_BUILD_DIR)/res/minigui/
endef

LDFLAGS+=-L$(STAGING_DIR)/usr/lib/ -lfreetype

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D$(CBOARD)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
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
define Package/$(TARGET)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/usr/res/
	$(INSTALL_DIR) $(1)/usr/res/cfg/
	$(INSTALL_DIR) $(1)/usr/res/data/
	$(INSTALL_DIR) $(1)/usr/res/font/
	$(INSTALL_DIR) $(1)/usr/res/lang/
	$(INSTALL_DIR) $(1)/usr/res/menu/
	$(INSTALL_DIR) $(1)/usr/res/others/
	$(INSTALL_DIR) $(1)/usr/res/topbar/
	$(INSTALL_DIR) $(1)/usr/res/sound/
	$(INSTALL_DIR) $(1)/etc/res/
	$(INSTALL_DIR) $(1)/etc/
	$(INSTALL_DIR) $(1)/usr/local/etc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(TARGET) $(1)/usr/bin/
	#$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/* $(1)/usr/res/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/cfg/* $(1)/usr/res/cfg/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/data/* $(1)/usr/res/data/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/font/* $(1)/usr/res/font/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/lang/* $(1)/usr/res/lang/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/menu/* $(1)/usr/res/menu/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/others/* $(1)/usr/res/others/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/topbar/* $(1)/usr/res/topbar/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/sound/* $(1)/usr/res/sound/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/wm_res/* $(1)/etc/res/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/res/bin/* $(1)/usr/bin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/minigui/sunxi-keyboard.kl $(1)/etc/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/res/minigui/MiniGUI.cfg $(1)/usr/local/etc/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(TARGET).init $(1)/etc/init.d/v
endef


# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(TARGET)))
