#
# Copyright (C) 2006-2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

OTHER_MENU:=Other modules

WATCHDOG_DIR:=watchdog


define KernelPackage/bluetooth
  SUBMENU:=$(OTHER_MENU)
  TITLE:=Bluetooth support
  DEPENDS:=@USB_SUPPORT +kmod-usb-core +kmod-crypto-hash +kmod-crypto-ecb +kmod-lib-crc16 +kmod-hid +!LINUX_3_18:kmod-crypto-cmac +LINUX_4_4:kmod-regmap
  KCONFIG:= \
	CONFIG_BLUEZ \
	CONFIG_BLUEZ_L2CAP \
	CONFIG_BLUEZ_SCO \
	CONFIG_BLUEZ_RFCOMM \
	CONFIG_BLUEZ_BNEP \
	CONFIG_BLUEZ_HCIUART \
	CONFIG_BLUEZ_HCIUSB \
	CONFIG_BLUEZ_HIDP \
	CONFIG_BT \
	CONFIG_BT_BREDR=y \
	CONFIG_BT_DEBUGFS=n \
	CONFIG_BT_L2CAP=y \
	CONFIG_BT_LE=y \
	CONFIG_BT_SCO=y \
	CONFIG_BT_RFCOMM \
	CONFIG_BT_BNEP \
	CONFIG_BT_HCIBTUSB \
	CONFIG_BT_HCIBTUSB_BCM=n \
	CONFIG_BT_HCIUSB \
	CONFIG_BT_HCIUART \
	CONFIG_BT_HCIUART_BCM=n \
	CONFIG_BT_HCIUART_INTEL=n \
	CONFIG_BT_HCIUART_H4 \
	CONFIG_BT_HIDP \
	CONFIG_HID_SUPPORT=y
  $(call AddDepends/rfkill)
  FILES:= \
	$(LINUX_DIR)/net/bluetooth/bluetooth.ko \
	$(LINUX_DIR)/net/bluetooth/rfcomm/rfcomm.ko \
	$(LINUX_DIR)/net/bluetooth/bnep/bnep.ko \
	$(LINUX_DIR)/net/bluetooth/hidp/hidp.ko \
	$(LINUX_DIR)/drivers/bluetooth/hci_uart.ko \
	$(LINUX_DIR)/drivers/bluetooth/btusb.ko
ifeq ($(strip $(call CompareKernelPatchVer,$(KERNEL_PATCHVER),ge,4.1.0)),1)
  FILES+= \
	$(LINUX_DIR)/drivers/bluetooth/btintel.ko
endif
  AUTOLOAD:=$(call AutoProbe,bluetooth rfcomm bnep hidp hci_uart btusb)
endef

define KernelPackage/bluetooth/description
 Kernel support for Bluetooth devices
endef

$(eval $(call KernelPackage,bluetooth))
