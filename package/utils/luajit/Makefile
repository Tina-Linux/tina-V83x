#
# Copyright (C) 2006-2008 OpenWrt.org
#
# LuaJIT is Copyright 2005-2017 Mike Pall, released under the MIT open source license.
#

include $(TOPDIR)/rules.mk

# Because 2.0.5 official version does not support ARM64, so choose 2.1.0 beta version
PKG_NAME:=LuaJIT
PKG_VERSION:=2.1.0-beta3
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=http://luajit.org/download
PKG_MD5SUM:=eae40bc29d06ee5e3078f9444fcea39b

PKG_LICENSE:=MIT
PKG_LICENSE_FILES:=COPYRIGHT

include $(BUILD_DIR)/host-build.mk
include $(BUILD_DIR)/package.mk

PKG_BUILD_PARALLEL:=1
PKG_INSTALL:=1

define Package/luajit
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=a Just-In-Time Compiler for Lua
  URL:=http://luajit.org/index.html
  MAINTAINER:=web-07@luajit.org
endef

define Package/luajit/description
LuaJIT is a Just-In-Time Compiler (JIT) for the Lua programming language.
Lua is a powerful, dynamic and light-weight programming language.
It may be embedded or used as a general-purpose, stand-alone language.
endef

LUAJIT_BUILDMODE = dynamic

# Host
HOST_CFLAGS += -D_GNU_SOURCE

define Host/Compile
	$(MAKE) -C $(HOST_BUILD_DIR) all
endef

define Host/Install
	$(MAKE) -C $(HOST_BUILD_DIR) install PREFIX=$(STAGING_DIR)/host
endef

ifeq ($(CONFIG_COMPLILE_KERNEL64_USER64),y)
LUAJIT_HOST_CC = $(HOSTCC)
else
LUAJIT_HOST_CC = $(HOSTCC) -m32
endif

# Target
define Build/Compile
	(cd $(PKG_BUILD_DIR); $(MAKE) PREFIX=$(CONFIGURE_PREFIX) \
				HOST_CC="$(LUAJIT_HOST_CC)" \
				STATIC_CC="$(TARGET_CC)" \
				DYNAMIC_CC="$(TARGET_CC) -fPIC" \
				TARGET_LD="$(TARGET_CC)" \
				TARGET_AR="$(TARGET_AR) rcus" \
				TARGET_STRIP=true \
				TARGET_CFLAGS="$(TARGET_CFLAGS)" \
				TARGET_LDFLAGS="$(TARGET_LDFLAGS)" \
				BUILDMODE=$(LUAJIT_BUILDMODE))
endef

define Build/Install
	(cd $(PKG_BUILD_DIR); $(MAKE) install PREFIX=$(PKG_INSTALL_DIR)$(CONFIGURE_PREFIX))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)
	$(CP) $(PKG_INSTALL_DIR)/* $(1)/
endef

define Package/luajit/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
endef

$(eval $(call HostBuild))
$(eval $(call BuildPackage,luajit))
