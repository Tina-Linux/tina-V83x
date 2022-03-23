#
# Copyright (C) 2006-2013 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

INPUT_MODULES_MENU:=Input modules

define KernelPackage/hid
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=HID Devices
  DEPENDS:=+kmod-input-core +kmod-input-evdev
  KCONFIG:=CONFIG_HID CONFIG_HIDRAW=y CONFIG_HID_BATTERY_STRENGTH=y
  FILES:=$(LINUX_DIR)/drivers/hid/hid.ko
  AUTOLOAD:=$(call AutoLoad,61,hid)
endef

define KernelPackage/hid/description
 Kernel modules for HID devices
endef

$(eval $(call KernelPackage,hid))

define KernelPackage/hid-generic
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=Generic HID device support
  DEPENDS:=+kmod-hid
  KCONFIG:=CONFIG_HID_GENERIC
  FILES:=$(LINUX_DIR)/drivers/hid/hid-generic.ko
  AUTOLOAD:=$(call AutoProbe,hid-generic)
endef

define KernelPackage/hid/description
 Kernel modules for generic HID device (e.g. keyboards and mice) support
endef

$(eval $(call KernelPackage,hid-generic))

define KernelPackage/input-core
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=Input device core
  KCONFIG:=CONFIG_INPUT
  FILES:=$(LINUX_DIR)/drivers/input/input-core.ko
endef

define KernelPackage/input-core/description
 Kernel modules for support of input device
endef

$(eval $(call KernelPackage,input-core))


define KernelPackage/input-evdev
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=Input event device
  DEPENDS:=+kmod-input-core
  KCONFIG:=CONFIG_INPUT_EVDEV
  FILES:=$(LINUX_DIR)/drivers/input/evdev.ko
  AUTOLOAD:=$(call AutoLoad,60,evdev)
endef

define KernelPackage/input-evdev/description
 Kernel modules for support of input device events
endef

$(eval $(call KernelPackage,input-evdev))


define KernelPackage/input-gpio-keys
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=GPIO key support
  DEPENDS:= @GPIO_SUPPORT +kmod-input-core
  KCONFIG:= \
	CONFIG_KEYBOARD_GPIO \
	CONFIG_INPUT_KEYBOARD=y
  FILES:=$(LINUX_DIR)/drivers/input/keyboard/gpio_keys.ko
  AUTOLOAD:=$(call AutoProbe,gpio_keys)
endef

define KernelPackage/input-gpio-keys/description
 This driver implements support for buttons connected
 to GPIO pins of various CPUs (and some other chips).

 See also gpio-button-hotplug which is an alternative, lower overhead
 implementation that generates uevents instead of kernel input events.
endef

$(eval $(call KernelPackage,input-gpio-keys))


define KernelPackage/input-gpio-keys-polled
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=Polled GPIO key support
  DEPENDS:=@GPIO_SUPPORT +kmod-input-polldev
  KCONFIG:= \
	CONFIG_KEYBOARD_GPIO_POLLED \
	CONFIG_INPUT_KEYBOARD=y
  FILES:=$(LINUX_DIR)/drivers/input/keyboard/gpio_keys_polled.ko
  AUTOLOAD:=$(call AutoProbe,gpio_keys_polled,1)
endef

define KernelPackage/input-gpio-keys-polled/description
 Kernel module for support polled GPIO keys input device

 See also gpio-button-hotplug which is an alternative, lower overhead
 implementation that generates uevents instead of kernel input events.
endef

$(eval $(call KernelPackage,input-gpio-keys-polled))


define KernelPackage/input-gpio-encoder
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=GPIO rotay encoder
  DEPENDS:=@GPIO_SUPPORT +kmod-input-core
  KCONFIG:=CONFIG_INPUT_GPIO_ROTARY_ENCODER
  FILES:=$(LINUX_DIR)/drivers/input/misc/rotary_encoder.ko
  AUTOLOAD:=$(call AutoProbe,rotary_encoder)
endef

define KernelPackage/gpio-encoder/description
 Kernel module to use rotary encoders connected to GPIO pins
endef

$(eval $(call KernelPackage,input-gpio-encoder))


define KernelPackage/input-joydev
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=Joystick device support
  DEPENDS:=+kmod-input-core
  KCONFIG:=CONFIG_INPUT_JOYDEV
  FILES:=$(LINUX_DIR)/drivers/input/joydev.ko
  AUTOLOAD:=$(call AutoProbe,joydev)
endef

define KernelPackage/input-joydev/description
 Kernel module for joystick support
endef

$(eval $(call KernelPackage,input-joydev))


define KernelPackage/input-polldev
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=Polled Input device support
  DEPENDS:=+kmod-input-core
  KCONFIG:=CONFIG_INPUT_POLLDEV
  FILES:=$(LINUX_DIR)/drivers/input/input-polldev.ko
endef

define KernelPackage/input-polldev/description
 Kernel module for support of polled input devices
endef

$(eval $(call KernelPackage,input-polldev))


define KernelPackage/input-matrixkmap
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=Input matrix devices support
  DEPENDS:=+kmod-input-core
  KCONFIG:=CONFIG_INPUT_MATRIXKMAP
  FILES:=$(LINUX_DIR)/drivers/input/matrix-keymap.ko
  AUTOLOAD:=$(call AutoProbe,matrix-keymap)
endef

define KernelPackage/input-matrix/description
 Kernel module support for input matrix devices
endef

$(eval $(call KernelPackage,input-matrixkmap))


define KernelPackage/keyboard-imx
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=IMX keypad support
  DEPENDS:=@(TARGET_mxs||TARGET_imx6) +kmod-input-matrixkmap
  KCONFIG:= \
	CONFIG_KEYBOARD_IMX \
	CONFIG_INPUT_KEYBOARD=y
  FILES:=$(LINUX_DIR)/drivers/input/keyboard/imx_keypad.ko
  AUTOLOAD:=$(call AutoProbe,imx_keypad)
endef

define KernelPackage/keyboard-imx/description
 Enable support for IMX keypad port.
endef

$(eval $(call KernelPackage,keyboard-imx))


define KernelPackage/input-uinput
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=user input module
  DEPENDS:=+kmod-input-core
  KCONFIG:= \
	CONFIG_INPUT_MISC=y \
	CONFIG_INPUT_UINPUT
  FILES:=$(LINUX_DIR)/drivers/input/misc/uinput.ko
  AUTOLOAD:=$(call AutoProbe,uinput)
endef

define KernelPackage/input-uinput/description
  user input modules needed for bluez
endef

$(eval $(call KernelPackage,input-uinput))

define KernelPackage/touchscreen-ft5x
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=ft5x  support
  DEPENDS:= +kmod-input-core
  KCONFIG:= \
	CONFIG_INPUT_TOUCHSCREEN \
	CONFIG_TOUCHSCREEN_FT5X_TS
  FILES:=$(LINUX_DIR)/drivers/input/touchscreen/ft5x/ft5x_ts.ko
  AUTOLOAD:=$(call AutoProbe,ft5x_ts)
endef

define KernelPackage/touchscreen-ft5x/description
 Enable support for ft5x touchscreen port.
endef

$(eval $(call KernelPackage,touchscreen-ft5x))

define KernelPackage/touchscreen-gslx680new
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=gslx680new  support
  DEPENDS:= +kmod-input-core
  KCONFIG:= \
	CONFIG_INPUT_TOUCHSCREEN \
	CONFIG_TOUCHSCREEN_GSLX680NEW
  FILES:=$(LINUX_DIR)/drivers/input/touchscreen/gslx680new/gslX680new.ko
  AUTOLOAD:=$(call AutoProbe,gslX680new)
endef

define KernelPackage/touchscreen-gslx680new/description
 Enable support for gslx680new touchscreen port.
endef

$(eval $(call KernelPackage,touchscreen-gslx680new))

define KernelPackage/touchscreen-gt82x
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=gt82x  support
  DEPENDS:= +kmod-input-core
  KCONFIG:= \
	CONFIG_INPUT_TOUCHSCREEN \
	CONFIG_TOUCHSCREEN_GT82X
  FILES:=$(LINUX_DIR)/drivers/input/touchscreen/gt82x.ko
  AUTOLOAD:=$(call AutoProbe,gt82x)
endef

define KernelPackage/touchscreen-gt82x/description
 Enable support for gt82x touchscreen port.
endef

$(eval $(call KernelPackage,touchscreen-gt82x))

define KernelPackage/touchscreen-ft6336
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=ft6336 support
  DEPENDS:= +kmod-input-core
  KCONFIG:= \
	CONFIG_INPUT_TOUCHSCREEN=y \
	CONFIG_INPUT_SENSORINIT=y \
	CONFIG_TOUCHSCREEN_FT6336
  FILES:=$(LINUX_DIR)/drivers/input/touchscreen/ft6336.ko
  AUTOLOAD:=$(call AutoProbe,ft6336)
endef

define KernelPackage/touchscreen-ft6336/description
 Enable support for ft6336 touchscreen port.
endef

$(eval $(call KernelPackage,touchscreen-ft6336))

define KernelPackage/touchscreen-atmel-mxt
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:=Atmel MXT  support
  DEPENDS:= +kmod-input-core
  KCONFIG:= \
	CONFIG_INPUT_TOUCHSCREEN \
	CONFIG_TOUCHSCREEN_ATMEL_MXT
  FILES:=$(LINUX_DIR)/drivers/input/touchscreen/atmel_mxt_ts.ko
  AUTOLOAD:=$(call AutoProbe,atmel_mxt_ts.ko)
endef

define KernelPackage/touchscreen-atmel-mxt/description
 Enable support for Atmel MXT touchscreen port.
endef

$(eval $(call KernelPackage,touchscreen-atmel-mxt))

define KernelPackage/touchscreen-focaltech
  SUBMENU:=$(INPUT_MODULES_MENU)
  TITLE:= Focaltech touchscreen support
  DEPENDS:= +kmod-input-core
  KCONFIG:= \
	CONFIG_INPUT_TOUCHSCREEN \
	CONFIG_TOUCHSCREEN_FTS
  FILES:=$(LINUX_DIR)/drivers/input/touchscreen/focaltech_touch/focaltech_ts.ko
  AUTOLOAD:=$(call AutoProbe,focaltech_ts.ko)
endef

define KernelPackage/touchscreen-focaltech/description
 Enable support for Focaltech touchscreen port.
endef

$(eval $(call KernelPackage,touchscreen-focaltech))
