#
# Copyright (C) 2006-2011 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

NETWORK_DEVICES_MENU:=Network Devices

define KernelPackage/ifb
  SUBMENU:=$(NETWORK_DEVICES_MENU)
  TITLE:=Intermediate Functional Block support
  KCONFIG:= \
	CONFIG_IFB \
	CONFIG_NET_CLS=y
  FILES:=$(LINUX_DIR)/drivers/net/ifb.ko
  AUTOLOAD:=$(call AutoLoad,34,ifb)
endef

define KernelPackage/ifb/description
  The Intermediate Functional Block
endef

$(eval $(call KernelPackage,ifb))
