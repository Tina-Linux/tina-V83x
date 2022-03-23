#
# Copyright (C) 2006-2008 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

WIRELESS_MENU:=Wireless Drivers

define KernelPackage/net-rtl8188eu
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=RTL8188EU support (staging)
  DEPENDS:=@USB_SUPPORT +@DRIVER_WEXT_SUPPORT +r8188eu-firmware +kmod-usb-core
#  KCONFIG:=\
#	CONFIG_STAGING=y \
#	CONFIG_R8188EU \
#	CONFIG_88EU_AP_MODE=y \
#	CONFIG_88EU_P2P=n
  FILES:=$(LINUX_DIR)/drivers/net/wireless/rtl8188eu/8188eu.ko
  AUTOLOAD:=$(call AutoProbe,8188eu)
endef

define KernelPackage/net-rtl8188eu/description
 Kernel modules for RealTek RTL8188EU support
endef

$(eval $(call KernelPackage,net-rtl8188eu))

define KernelPackage/net-rtl8723bs
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=RTL8723BS support (staging)
  DEPENDS:=@USB_SUPPORT +@DRIVER_WEXT_SUPPORT +r8723bs-firmware
#  KCONFIG:=\
#	CONFIG_STAGING=y \
#	CONFIG_R8723BS \
#	CONFIG_23BS_AP_MODE=y \
#	CONFIG_23BS_P2P=n
  FILES:=$(LINUX_DIR)/drivers/net/wireless/rtl8723bs/8723bs.ko
  AUTOLOAD:=$(call AutoProbe,8723bs)
endef

define KernelPackage/net-rtl8723bs/description
 Kernel modules for RealTek RTL8723BS support
endef

$(eval $(call KernelPackage,net-rtl8723bs))

define KernelPackage/net-mrvl8977
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=Marvell 8977 support (staging)
  DEPENDS:=@USB_SUPPORT +@DRIVER_WEXT_SUPPORT
  FILES :=$(LINUX_DIR)/drivers/net/wireless/88w8977/mlan.ko
  FILES +=$(LINUX_DIR)/drivers/net/wireless/88w8977/sd8xxx.ko
  AUTOLOAD:=$(call AutoProbe,mrvl8977)
endef

define KernelPackage/net-mrvl8977/description
 Kernel modules for Marvell 8977 support
endef

$(eval $(call KernelPackage,net-mrvl8977))

define KernelPackage/net-qca9377
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=Qualcomm qca9377 support (staging)
  DEPENDS:=@USB_SUPPORT +@DRIVER_WEXT_SUPPORT +qca9377-firmware-cfgfile
  FILES :=$(LINUX_DIR)/drivers/net/wireless/qcacld-new/wlan.ko
  AUTOLOAD:=$(call AutoProbe,qca9377)
endef

define KernelPackage/net-qca9377/description
 Kernel modules for Qualcomm qca9377 support
endef

$(eval $(call KernelPackage,net-qca9377))

define KernelPackage/cfg80211
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=cfg80211 support (staging)
  DEPENDS:=
  FILES:=$(LINUX_DIR)/net/wireless/cfg80211.ko
  AUTOLOAD:=$(call AutoProbe,cfg80211)
endef

define KernelPackage/cfg80211/description
 Kernel modules for CFG80211 support
endef

$(eval $(call KernelPackage,cfg80211))

define KernelPackage/esp8089
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=esp8089 support (staging)
  DEPENDS:=
  FILES :=$(LINUX_DIR)/drivers/net/wireless/esp8089/esp8089.ko
  AUTOLOAD:=$(call AutoProbe,esp8089)
endef

define KernelPackage/esp8089/description
 Kernel modules for esp8089 support
endef

$(eval $(call KernelPackage,esp8089))
