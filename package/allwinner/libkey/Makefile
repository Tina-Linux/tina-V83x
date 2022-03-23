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
PKG_NAME:=libsec_key
PKG_VERSION:=1
PKG_RELEASE:=2

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

TARGET_DEMO_LDFLAGS+= -lsec_key

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=secure key for allwinner
endef

define Package/$(PKG_NAME)
	$(call Package/$(PKG_NAME)/Default)
	MENU:=1
	DEFAULT:=1
endef

define Package/$(PKG_NAME)-demo
	$(call Package/$(PKG_NAME)/Default)
	TITLE:=secure key demo for allwinner
	DEPENDS:=libsec_key +libsec_key
endef


# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Package/$(PKG_NAME)/config
config SUNXI_SUPPORT_PRIVATE_KEY
	bool "support private key"
	depends on PACKAGE_libsec_key
	default n
if SUNXI_SUPPORT_PRIVATE_KEY
choice
prompt "private bare key mode"
	default SUNXI_SUPPORT_PRIVATE_BARE_FIRST_8K
	help
	select the store mode of private bare

config SUNXI_SUPPORT_PRIVATE_BARE_FIRST_8K
	bool "Enable private bare key first 8K support"

config SUNXI_SUPPORT_PRIVATE_BARE_TAIL_8K
	bool "Enable private bare key tail 8K support"

endchoice
endif

config SUNXI_SUPPORT_SECURE_STORAGE_KEY
	bool "Enable secure storage key support"
	default n
	depends on PACKAGE_libsec_key

endef


define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)
	$(CP) -r ./demo $(PKG_BUILD_DIR)
endef

#config the key type
ifeq ($(CONFIG_SUNXI_SUPPORT_PRIVATE_BARE_FIRST_8K),y)
TARGET_CFLAGS+=-DCONF_SUPPORT_PRIVATE_BARE_KEY
endif

ifeq ($(CONFIG_SUNXI_SUPPORT_PRIVATE_BARE_TAIL_8K),y)
TARGET_CFLAGS+=-DCONF_SUPPORT_PRIVATE_BARE_KEY -DCONF_SUNXI_PRIVATE_BARE_TAIL_8K
endif

ifeq ($(CONFIG_SUNXI_SUPPORT_SECURE_STORAGE_KEY),y)
TARGET_CFLAGS+=-DCONF_SUPPORT_SECURE_STORAGE_KEY
endif

# We do not need to define Build/Configure or Build/Compile directives
# The defaults are appropriate for compiling a simple program such as this one
define Build/Compile/$(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/$(PKG_NAME)-demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_DEMO_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
	$(Build/Compile/$(PKG_NAME)-demo)
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/include/libkey/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/libsec_key.so 	$(1)/usr/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/include/api.h	$(1)/usr/include/libkey/
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/sec_key_test 	$(1)/usr/bin/
endef



$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
