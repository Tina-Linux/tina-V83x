#
# Copyright (C) 2014-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=procd
PKG_VERSION:=2016-02-08

PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL=$(OPENWRT_GIT)/project/procd.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=57fe34225206c572b5e049826505f738191e4db5
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION)-$(PKG_SOURCE_VERSION).tar.gz
CMAKE_INSTALL:=1

PKG_LICENSE:=GPL-2.0
PKG_LICENSE_FILES:=

PKG_MAINTAINER:=John Crispin <blogic@openwrt.org>

PKG_CONFIG_DEPENDS:= \
	CONFIG_TARGET_INIT_PATH CONFIG_KERNEL_SECCOMP \
	CONFIG_NAND_SUPPORT CONFIG_PROCD_SHOW_BOOT CONFIG_PROCD_ZRAM_TMPFS \
	CONFIG_KERNEL_NAMESPACES CONFIG_PACKAGE_procd-ujail CONFIG_PACKAGE_procd-seccomp

PKG_PREPARED_DEPENDS:= \
	CONFIG_USE_SELINUX CONFIG_USE_DM_VERITY

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell $(SH_FUNC) echo $(CONFIG_TARGET_INIT_PATH) | md5s)
endif

CMAKE_OPTIONS += -DEARLY_PATH="$(TARGET_INIT_PATH)"
TARGET_CFLAGS += $(if $(CONFIG_USE_GLIBC),-pthread)
TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lrt)

TARGET_LDFLAGS += $(if $(CONFIG_USE_SELINUX),-lsepol -lselinux)

define Package/procd
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+ubusd +ubus +libjson-script +ubox +USE_GLIBC:librt +libubox +libubus +NAND_SUPPORT:procd-nand +USE_SELINUX:selinux +USE_SELINUX:sepolicy-demoi +block-mount
  DEPENDS+= @SYSTEM_INIT_PROCD
  TITLE:=OpenWrt system process manager
endef

define Package/procd-ujail
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=@KERNEL_NAMESPACES +@KERNEL_UTS_NS +@KERNEL_IPC_NS +@KERNEL_PID_NS +libubox +libblobmsg-json
  TITLE:=OpenWrt process jail helper
endef

define Package/procd-seccomp
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=@arm||@armeb||@mips||@mipsel||@i386||@x86_64 @!TARGET_uml @KERNEL_SECCOMP +libubox +libblobmsg-json
  TITLE:=OpenWrt process seccomp helper + utrace
endef

define Package/procd-nand
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=@NAND_SUPPORT +ubi-utils
  TITLE:=OpenWrt sysupgrade nand helper
endef

define Package/procd-nand-firstboot
  SECTION:=utils
  CATEGORY:=Utilities
  DEPENDS:=procd-nand
  TITLE:=OpenWrt firstboot nand helper
endef

define Package/procd/config
menu "Configuration"
	depends on PACKAGE_procd

config PROCD_SHOW_BOOT
	bool
	default n
	prompt "Print the shutdown to the console as well as logging it to syslog"

config PROCD_ZRAM_TMPFS
	bool
	default n
	prompt "Mount /tmp using zram."
endmenu
endef


ifeq ($(CONFIG_NAND_SUPPORT),y)
  CMAKE_OPTIONS += -DBUILD_UPGRADED=1
endif

ifeq ($(CONFIG_PROCD_SHOW_BOOT),y)
  CMAKE_OPTIONS += -DSHOW_BOOT_ON_CONSOLE=1
endif

ifeq ($(CONFIG_PROCD_ZRAM_TMPFS),y)
  CMAKE_OPTIONS += -DZRAM_TMPFS=1
endif

ifdef CONFIG_PACKAGE_procd-ujail
  CMAKE_OPTIONS += -DJAIL_SUPPORT=1
endif

ifdef CONFIG_PACKAGE_procd-seccomp
  CMAKE_OPTIONS += -DSECCOMP_SUPPORT=1 -DUTRACE_SUPPORT=1
endif

ifdef CONFIG_BOOT_PLAY_IN_PREINIT
ifdef CONFIG_BOOT_PLAY_IN_PREINIT_AFTER_KMODLOADER
  CMAKE_OPTIONS += -DBOOT_PLAY=2
else
  CMAKE_OPTIONS += -DBOOT_PLAY=1
endif
endif

ifeq ($(CONFIG_USE_SELINUX)$(CONFIG_PACKAGE_selinux),yy)
  CMAKE_OPTIONS += -DSELINUX_ENABLED=1
endif

ifneq ($(CONFIG_USE_SELINUX)$(CONFIG_USE_DM_VERITY),)
  CMAKE_OPTIONS += -DDEVTMPFS_ENABLED=1
endif

define Package/procd/install
	$(INSTALL_DIR) $(1)/sbin $(1)/etc $(1)/lib/functions

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/{init,procd,askfirst,udevtrigger} $(1)/sbin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libsetlbf.so $(1)/lib
	$(INSTALL_BIN) ./files/reload_config $(1)/sbin/
	$(INSTALL_DATA) ./files/hotplug*.json $(1)/etc/
	$(INSTALL_DATA) ./files/procd.sh $(1)/lib/functions/
endef

define Package/procd-ujail/install
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/ujail $(1)/sbin/
endef

define Package/procd-seccomp/install
	$(INSTALL_DIR) $(1)/sbin $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libpreload-seccomp.so $(1)/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/utrace $(1)/sbin/
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libpreload-trace.so $(1)/lib
endef

define Package/procd-nand/install
	$(INSTALL_DIR) $(1)/sbin $(1)/lib/upgrade

	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/upgraded $(1)/sbin/
	$(INSTALL_DATA) ./files/nand.sh $(1)/lib/upgrade/
endef

define Package/procd-nand-firstboot/install
	$(INSTALL_DIR) $(1)/lib/preinit

	$(INSTALL_DATA) ./files/nand-preinit.sh $(1)/lib/preinit/60-nand-firstboot.sh
endef

$(eval $(call BuildPackage,procd))
$(eval $(call BuildPackage,procd-ujail))
$(eval $(call BuildPackage,procd-seccomp))
$(eval $(call BuildPackage,procd-nand))
$(eval $(call BuildPackage,procd-nand-firstboot))
