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
include $(BUILD_DIR)/package.mk

# Name and release number of this package
PKG_NAME:=speech
PKG_VERSION:=0.0.11
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
HAVE_SOURCE := yes

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:=+libpthread \
		   +libstdcpp \
		   +alsa-lib \
           +libresample

endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=speech
endef

define Package/$(PKG_NAME)/description
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	-unlink $(PKG_NAME)
	-ln -s $(PKG_BUILD_DIR)
endef

define Build/InstallDev
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/speech/bin
	$(CP) ./service/init $(1)/usr/bin/speech
	$(CP) ./service/luaclib $(1)/usr/bin/speech
	$(CP) ./service/res $(1)/usr/bin/speech
	$(CP) ./service/test $(1)/usr/bin/speech

	$(INSTALL_BIN) ./service/bootloader-awr16.sh $(1)/usr/bin/speech
	$(INSTALL_BIN) ./service/bin/* $(1)/usr/bin/speech/bin/

endef

$(eval $(call BuildPackage,$(PKG_NAME)))
