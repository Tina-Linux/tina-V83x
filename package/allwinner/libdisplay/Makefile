# *
# * screent test base on framebuffer device
# * Copyright (C) 2015-2018 AllwinnerTech
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
# *

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=libdisplay
PKG_VERSION:=1
PKG_RELEASE:=1

# This specifies the directory where we're going to build the program.
# The root build directory, $(BUILD_DIR), is by default the build_mipsel
# directory in your OpenWrt SDK directory

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

TARGET_DEMO_LDFLAGS+= -ldisplay

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=display testcase for allwinner
endef

define Package/$(PKG_NAME)
	$(call Package/$(PKG_NAME)/Default)
	MENU:=1
	DEFAULT:=1
endef

define Package/$(PKG_NAME)-demo
	$(call Package/$(PKG_NAME)/Default)
	TITLE:=display testcase demo for allwinner
	DEPENDS:=libdisplay +libdisplay
endef

# Specify what needs to be done to prepare for building the package.
# In our case, we need to copy the source files to the build directory.
# This is NOT the default.  The default uses the PKG_SOURCE_URL and the
# PKG_SOURCE which is not defined here to download the source from the web.
# In order to just build a simple program that we have just written, it is
# much easier to do it this way.
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)
	$(CP) -r ./demo $(PKG_BUILD_DIR)
endef

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
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef


define Build/Compile
	$(Build/Compile/$(PKG_NAME))
	$(Build/Compile/$(PKG_NAME)-demo)
endef

#define Package/$(PKG_NAME)/install
#	$(INSTALL_DIR) $(1)/usr/lib/
#	$(INSTALL_DIR) $(1)/usr/include/libdisplay/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/framebuffer/*.a   $(1)/usr/lib/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/testcase/*.a   $(1)/usr/lib/
#	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/virtual/*.a   $(1)/usr/lib/
#endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/display_test   $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))

