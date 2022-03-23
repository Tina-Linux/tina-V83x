##############################################
# OpenWrt Makefile for smartlinkd program
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
PKG_NAME:=libminui
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:=+libstdcpp +zlib +libpng +PACKAGE_kmod-sunxi-drm:libdrm
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=tiny miniui library
  MENU:=1
endef

define Package/$(PKG_NAME)/config
if PACKAGE_$(PKG_NAME)
choice
prompt "Choose the display rotation angle"
	default MINUI_ROTATION_NONE
	help
		Choose the display rotation angle.
config MINUI_ROTATION_NONE
	bool "ROTATION_NONE"
config MINUI_ROTATION_DOWN
	bool "ROTATION_DOWN"
config MINUI_ROTATION_RIGHT
	bool "ROTATION_RIGHT"
config MINUI_ROTATION_LEFT
	bool "ROTATION_LEFT"
endchoice
endif
endef

define Package/$(PKG_NAME)/description
	Tiny miniui library, refer to the android 9.0
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) -r ./src $(PKG_BUILD_DIR)/
endef

ifeq ($(CONFIG_PACKAGE_kmod-sunxi-disp),y)
TARGET_CFLAGS+=-DRECOVERY_BGRA
endif

ifeq ($(CONFIG_PACKAGE_kmod-sunxi-drm),y)
TARGET_CFLAGS+=-I$(STAGING_DIR)/usr/include/libdrm -DRECOVERY_RGBX
TARGET_LDFLAGS+= -ldrm
endif

ROTATION = -DDEFAULT_ROTATION=ROTATION_NONE

ifeq ($(CONFIG_MINUI_ROTATION_NONE),y)
ROTATION = -DDEFAULT_ROTATION=ROTATION_NONE
endif

ifeq ($(CONFIG_MINUI_ROTATION_DOWN),y)
ROTATION = -DDEFAULT_ROTATION=ROTATION_DOWN
endif

ifeq ($(CONFIG_MINUI_ROTATION_RIGHT),y)
ROTATION = -DDEFAULT_ROTATION=ROTATION_RIGHT
endif

ifeq ($(CONFIG_MINUI_ROTATION_LEFT),y)
ROTATION = -DDEFAULT_ROTATION=ROTATION_LEFT
endif

TARGET_CFLAGS+=$(ROTATION)

define Build/Compile

	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include/minui
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/lib/libminui.so $(1)/lib/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/minui/*.h $(1)/usr/include/minui/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/lib/libminui.so $(1)/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
