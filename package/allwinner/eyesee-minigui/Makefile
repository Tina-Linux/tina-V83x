##############################################
# OpenWrt Makefile for eyesee-minigui
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
PKG_NAME:=eyesee-minigui
PKG_VERSION:=3.2
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
SRC_COMPILE_DIR := $(PKG_BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
SRC_CODE_DIR := $(TOPDIR)/softwinner/minigui
SRC_CODE_INSTALL_DIR := $(SRC_COMPILE_DIR)/_install

# This specifies the directory where file depends. when files in it update, increasing compile will be enable.
PKG_FILE_DEPENDS:= $(SRC_CODE_DIR)

include $(BUILD_DIR)/package.mk

# Specify package information for this program.
# The variables defined here should be self explanatory.
# If you are running Kamikaze, delete the DESCRIPTION
# variable below and uncomment the Kamikaze define
# directive for the description below
define Package/$(PKG_NAME)
  SECTION:=eyesee-minigui
  CATEGORY:=Allwinner
  TITLE:=eyesee-minigui for Tina Linux
  DEPENDS:= +tslib +libjpeg +libpng
endef

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

# Uncomment portion below for Kamikaze and delete DESCRIPTION variable above
define Package/$(PKG_NAME)/description
  eyesee-minigui is a GUILib for eyesee project.
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.

define Build/Prepare
#	mkdir -p $(PKG_BUILD_DIR)
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		rm -rf $(SRC_COMPILE_DIR); \
	fi
	ln -s $(SRC_CODE_DIR) $(SRC_COMPILE_DIR)

	if [ -x $(SRC_CODE_DIR)/autogen.sh ]; then \
		cd $(SRC_CODE_DIR); \
		./autogen.sh; \
	fi

	(cd $(SRC_CODE_DIR) && aclocal  || echo "aclocal error")
	(cd $(SRC_CODE_DIR) && autoconf || echo "autoconf error")
	(cd $(SRC_CODE_DIR) && automake --add-missing || echo "automake error")
endef

LOCAL_CONFIGURE_ARGS := \
	--prefix=$(SRC_CODE_INSTALL_DIR)/ \
	--target=$(GNU_TARGET_NAME) \
	--host=$(GNU_TARGET_NAME) \
	--build=$(GNU_HOST_NAME) \
    --with-targetname=fbcon \
    --enable-sunximin \
    --enable-jpgsupport \
    --enable-pngsupport \
    --enable-adv2dapi \
    --enable-videoshadow \
    --enable-msgstr \
    --enable-ctrlanimation \
    --enable-ctrliconview

#	$(CONFIGURE_ARGS)

ifeq ($(CONFIG_SUNXITSLIB_SUPPORT),y)
LOCAL_CONFIGURE_ARGS += --enable-sunxikeytslibial
else ifeq ($(CONFIG_AWMINIGUI_TSLIB_SUPPORT),y)
LOCAL_CONFIGURE_ARGS += --enable-tslibial
TARGET_LDFLAGS+= -lts
LDFLAGS+=-lts
endif

TARGET_CFLAGS +=
TARGET_CXXFLAGS +=
TARGET_CPPFLAGS += -DFORBID_TWOCLICKS_FLING

#ifeq (${TARGET_PRODUCT},v316_sdv)
ifneq ($(findstring $(strip $(TARGET_PRODUCT)),v316_sdv v313_sdv v316_sdvlpddr v533_cdr_axp152),)
TARGET_CPPFLAGS += -DPRODUCT_V316_SDV
else ifneq ($(findstring $(TARGET_PRODUCT),v316_cdr v536_cdr),)
TARGET_CPPFLAGS += -DPRODUCT_V316_CDR
endif

export CFLAGS=$(TARGET_CFLAGS)
export CXXFLAGS=$(TARGET_CXXFLAGS)
export CPPFLAGS=$(TARGET_CPPFLAGS)
export LDFLAGS=$(TARGET_LDFLAGS)

define Build/Configure
	(cd $(SRC_COMPILE_DIR) && ./configure $(LOCAL_CONFIGURE_ARGS))
endef

define Build/Compile
	$(MAKE) -C $(SRC_COMPILE_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
	all
endef

define Build/Clean
	if [ -d $(SRC_COMPILE_DIR) ]; then \
		$(MAKE) -C $(SRC_COMPILE_DIR) \
			ARCH="$(TARGET_ARCH)" \
			AR="$(TARGET_AR)" \
			CC="$(TARGET_CC)" \
			CXX="$(TARGET_CXX)" \
		clean; \
	else \
		echo "$(SRC_COMPILE_DIR) is not exist!"; \
	fi
	rm $(SRC_CODE_INSTALL_DIR) -rf
endef

define Build/InstallDev
	$(MAKE) -C $(SRC_COMPILE_DIR) install-strip
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include/eyesee-minigui
	find $(SRC_COMPILE_DIR)/ -name "libminigui_ths*.so*" | xargs -I{} $(CP) {} $(1)/usr/lib
	find $(SRC_COMPILE_DIR)/ -name "libminigui_ths*.a*" | xargs -I{} $(CP) {} $(1)/usr/lib
	$(CP) -rfd $(SRC_CODE_INSTALL_DIR)/include/* $(1)/usr/include/eyesee-minigui
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
	$(INSTALL_DIR) $(1)/usr/lib
	find $(SRC_COMPILE_DIR)/ -name "libminigui_ths*.so*" | xargs -I{} $(CP) {} $(1)/usr/lib
endef

# This line executes the necessary commands to compile our program.
# The above define directives specify all the information needed, but this
# line calls BuildPackage which in turn actually uses this information to
# build a package.

$(eval $(call BuildPackage,$(PKG_NAME)))
