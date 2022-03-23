#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=nativepower
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
include $(BUILD_DIR)/package.mk

ifeq ($(KERNEL_PATCHVER), 3.4)
USE_TINA_SUSPEND=y
endif

define Package/libscenemanager
  SECTION:=utils
  CATEGORY:=Libraries
  TITLE:= scene mannager library
  DEPENDS:=+libuci
endef

define Package/libscenemanager/description
  Scene manager library
  Detail scene config in uci config(/etc/config/***)
endef

define Package/nativepower_utils
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:= nativepower utilites
  DEPENDS:= +libscenemanager +PACKAGE_nativepower:nativepower
endef

define Package/nativepower_utils/description
  Nativepower Utilites, set system scene
endef

define Package/nativepower
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=nativepower for allwinner
  DEPENDS:=+libpthread +NATIVEPOWER_USE_DBUS:dbus +libscenemanager
  MENU:=1
endef

define Package/nativepower/description
  Nativepower for Allwinner
endef

define Package/nativepower/config
if PACKAGE_nativepower
config NATIVEPOWER_USE_DBUS
    bool
    prompt "nativepower communicate other process by dbus"
    default n
endif
endef

define Build/Prepare
	$(CP) -r ./libscenemanager $(PKG_BUILD_DIR)/
	$(CP) -r ./daemon $(PKG_BUILD_DIR)/
	$(CP) -r ./libnativepower $(PKG_BUILD_DIR)/
	$(CP) -r ./libsuspend $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/libscenemanager \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

	$(MAKE) -C $(PKG_BUILD_DIR)/libsuspend \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		USE_TINA_SUSPEND=$(USE_TINA_SUSPEND) \
		all

ifeq ($(CONFIG_NATIVEPOWER_USE_DBUS), y)
	$(MAKE) -C $(PKG_BUILD_DIR)/libnativepower \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endif

ifeq ($(CONFIG_PACKAGE_nativepower), y)
	$(MAKE) -C $(PKG_BUILD_DIR)/daemon \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		USE_TINA_SUSPEND=$(USE_TINA_SUSPEND) \
		USE_DBUS=$(CONFIG_NATIVEPOWER_USE_DBUS) \
		all
endif
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		USE_DBUS="$(CONFIG_NATIVEPOWER_USE_DBUS)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libscenemanager.so $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include

ifeq ($(CONFIG_NATIVEPOWER_USE_DBUS), y)
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libnativepower.so $(1)/lib/
endif
endef

define Package/libscenemanager/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libscenemanager.so $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/nativepower $(1)/etc/config/nativepower
endef

define Package/nativepower_utils/install
	$(INSTALL_DIR) $(1)/usr/bin
ifneq ($(CONFIG_PACKAGE_nativepower), y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/nativepower_client $(1)/usr/bin/
endif
	$(LN) /usr/bin/nativepower_client $(1)/usr/bin/nativepower_utils
endef

define Package/nativepower/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/daemon/nativepower_daemon $(1)/usr/sbin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/demo/nativepower_client $(1)/usr/bin/

ifeq ($(CONFIG_NATIVEPOWER_USE_DBUS), y)
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libnativepower.so $(1)/lib/
endif

	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/nativepower.init $(1)/etc/init.d/nativepower
ifeq ($(CONFIG_NATIVEPOWER_USE_DBUS), y)
	$(INSTALL_DIR) $(1)/usr/share/dbus-1/system.d
	$(INSTALL_BIN) \
		./files/nativepower.conf \
		$(1)/usr/share/dbus-1/system.d/nativepower.conf
endif
endef

$(eval $(call BuildPackage,libscenemanager))
$(eval $(call BuildPackage,nativepower))
$(eval $(call BuildPackage,nativepower_utils))
