#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=kernel
PKG_FLAGS:=hold

PKG_BUILD_DIR:=$(KERNEL_BUILD_DIR)/packages
PKG_FILE_DEPENDS+=$(TOPDIR)/target/allwinner/$(TARGET_BOARD)/modules.mk $(TOPDIR)/target/allwinner/generic/modules.mk $(TOPDIR)/target/allwinner/$(TARGET_PLATFORM)-common/modules.mk
SCAN_DEPS= modules/*.mk $(TOPDIR)/target/allwinner/$(TARGET_BOARD)/modules.mk $(TOPDIR)/build/netfilter.mk $(TOPDIR)/target/allwinner/generic/modules.mk $(TOPDIR)/target/allwinner/$(TARGET_PLATFORM)-common/modules.mk

PKG_LICENSE:=GPLv2
PKG_LICENSE_FILES:=

export SHELL:=/bin/sh
.ONESHELL:

include $(BUILD_DIR)/package.mk

STAMP_BUILT:=$(STAMP_BUILT)_$(firstword $(shell $(SCRIPT_DIR)/kconfig.pl $(LINUX_DIR)/.config | md5sum))

ifeq ($(DUMP),)
  -include $(LINUX_DIR)/.config
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Configure
endef

define Build/Compile
endef

define KernelPackage/depends
endef

CONFIG_PACKAGE_kernel=y
define Package/kernel
  SECTION:=sys
  CATEGORY:=Kernel
  DEFAULT:=y
  TITLE:=Virtual kernel package
  VERSION:=$(LINUX_VERSION)-$(LINUX_RELEASE)-$(LINUX_VERMAGIC)
  URL:=http://www.kernel.org/
endef

define Package/kernel/install
  # nothing to do
endef

define Package/kernel/extra_provides
	sed -e 's,.*/,,' $(LINUX_DIR)/modules.builtin;
endef

$(eval $(if $(DUMP),,$(call BuildPackage,kernel)))

include $(sort $(wildcard ./modules/*.mk))
-include $(TOPDIR)/target/allwinner/$(TARGET_BOARD)/modules.mk
-include $(TOPDIR)/target/allwinner/$(TARGET_PLATFORM)-common/modules.mk
-include $(TOPDIR)/target/allwinner/generic/modules.mk
