#
# Copyright (C) 2015-2016 Allwinner
#
# This is free software, licensed under the GNU General Public License v2.
# See /build/LICENSE for more information.

define KernelPackage/vin-v4l2
  SUBMENU:=$(VIDEO_MENU)
  TITLE:=Video input support (staging)
  DEPENDS:=
  FILES:=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-core.ko
  FILES+=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-dma-contig.ko
  FILES+=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-memops.ko
  FILES+=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-v4l2.ko
  FILES+=$(LINUX_DIR)/drivers/media/platform/sunxi-vin/vin_io.ko
  FILES+=$(LINUX_DIR)/drivers/media/platform/sunxi-vin/modules/sensor/sc2232_mipi.ko
  FILES+=$(LINUX_DIR)/drivers/media/platform/sunxi-vin/vin_v4l2.ko
#  FILES+=$(LINUX_DIR)/drivers/video/fbdev/sunxi/disp2/hdmi2/hdmi20.ko
  AUTOLOAD:=$(call AutoProbe,videobuf2-core videobuf2-dma-contig videobuf2-memops videobuf2-v4l2 vin_io sc2232_mipi vin_v4l2)
endef

define KernelPackage/vin_v4l2/description
 Kernel modules for video input support
endef

$(eval $(call KernelPackage,vin-v4l2))

#define KernelPackage/EISE-ISE
#  SUBMENU:=$(VIDEO_MENU)
#  TITLE:=Video ISE&EISE support (staging)
#  DEPENDS:=
#  FILES:=$(LINUX_DIR)/drivers/media/platform/sunxi-ise/sunxi_ise.ko
#  FILES+=$(LINUX_DIR)/drivers/media/platform/sunxi-eise/sunxi_eise.ko
#  UTOLOAD:=$(call AutoProbe,sunxi_ise sunxi_eise)
#endef
#
#define KernelPackage/EISE-ISE/description
# Kernel modules for video ISE&EISE support
#endef
#
#$(eval $(call KernelPackage,EISE-ISE))
#
#define KernelPackage/sunxi-rf-wlan
#  SUBMENU:=$(WIRELESS_MENU)
#  TITLE:=sunxi rfkill wlan support (staging)
#  DEPENDS:=
#  KCONFIG:= CONFIG_RFKILL \
#	  CONFIG_SUNXI_RFKILL
#  FILES:=$(LINUX_DIR)/drivers/misc/sunxi-rf/sunxi-wlan.ko
#  AUTOLOAD:=$(call AutoProbe, sunxi-rf-wlan)
#endef
#
#define KernelPackage/sunxi-rf-wlan/description
# Kernel modules for sunx-wlan support
#endef
#
#$(eval $(call KernelPackage,sunxi-rf-wlan))
#
#define KernelPackage/sunxi-rf-bluetooth
#  SUBMENU:=$(WIRELESS_MENU)
#  TITLE:=sunxi rfkill bluetooth support (staging)
#  DEPENDS:=
#  KCONFIG:= CONFIG_RFKILL \
#	  CONFIG_SUNXI_RFKILL
#  FILES:=$(LINUX_DIR)/drivers/misc/sunxi-rf/sunxi-bluetooth.ko
#  AUTOLOAD:=$(call AutoProbe, sunxi-rf-bluetooth)
#endef
#
#define KernelPackage/sunxi-rf-bluetooth/description
# Kernel modules for sunx-bluetooth support
#endef
#
#$(eval $(call KernelPackage,sunxi-rf-bluetooth))
#
define KernelPackage/8189fs
  SUBMENU:=$(WIRELESS_MENU)
  TITLE:=8189fs support (staging)
  DEPENDS:= +kmod-sunxi-rf-wlan +kmod-cfg80211
  FILES:=$(LINUX_DIR)/drivers/net/wireless/rtl8189fs/8189fs.ko
  AUTOLOAD:=$(call AutoProbe, 8189fs.ko)
endef

define KernelPackage/net-xr819/description
 Kernel modules for 8189fs support
endef

$(eval $(call KernelPackage,8189fs))

#define KernelPackage/touchscreen-ft6236
#  SUBMENU:=$(INPUT_MODULES_MENU)
#  TITLE:= FT6236 I2C touchscreen
#  DEPENDS:= +kmod-input-core
#  KCONFIG:= CONFIG_INPUT_TOUCHSCREEN \
#	  CONFIG_TOUCHSCREEN_PROPERTIES \
#	CONFIG_TOUCHSCREEN_FT6236
#  FILES:=$(LINUX_DIR)/drivers/input/touchscreen/ft6236.ko
#  FILES+=$(LINUX_DIR)/drivers/input/touchscreen/of_touchscreen.ko
#  AUTOLOAD:=$(call AutoProbe,of_touchscreen ft6236)
#endef
#
#define KernelPackage/touchscreen-ft6236/description
# Enable support for Focaltech 6236 touchscreen port.
#endef
#
#$(eval $(call KernelPackage,touchscreen-ft6236))
#
#define KernelPackage/sunxi-gpadc
#  SUBMENU:=$(INPUT_MODULES_MENU)
#  TITLE:= FT6236 I2C touchscreen
#  DEPENDS:= +kmod-input-core +kmod-input-evdev
#  KCONFIG:= CONFIG_INPUT_TOUCHSCREEN \
#	  CONFIG_TOUCHSCREEN_PROPERTIES \
#	CONFIG_TOUCHSCREEN_FT6236
#  FILES:=$(LINUX_DIR)/drivers/input/sensor/sunxi_gpadc.ko
#  AUTOLOAD:=$(call AutoProbe,sunxi_gpadc.ko)
#endef
#
#define KernelPackage/sunxi-gpadc/description
# Enable support for Focaltech 6236 touchscreen port.
#endef
#
#$(eval $(call KernelPackage,sunxi-gpadc))
#
#define KernelPackage/iio-mpu6xxx-i2c
#  SUBMENU:=$(IIO_MENU)
#  TITLE:=MPU6050/6500/9150  inertial measurement sensor (I2C)
#  DEPENDS:=+kmod-i2c-core +kmod-iio-core
#  KCONFIG:=CONFIG_INV_MPU6050_I2C \
#	  CONFIG_INV_MPU6050_IIO
#  FILES:=$(LINUX_DIR)/drivers/iio/imu/inv_mpu6050/inv-mpu6050.ko  \
#	  $(LINUX_DIR)/drivers/iio/imu/inv_mpu6050/inv-mpu6050-i2c.ko
#  AUTOLOAD:=$(call AutoProbe,inv-mpu6050 inv-mpu6050-i2c)
#endef
#
#define KernelPackage/iio-mpu6xxx-i2c/description
#  This driver supports the Invensense MPU6050/6500/9150 and ICM20608
#  motion tracking devices over I2C.
#  This driver can be built as a module. The module will be called
#  inv-mpu6050-i2c.
#endef
#
#$(eval $(call KernelPackage,iio-mpu6xxx-i2c))
#
#define KernelPackage/iio-mpu6xxx-spi
#  SUBMENU:=$(IIO_MENU)
#  TITLE:=MPU6050/6500/9150  inertial measurement sensor (SPI)
#  DEPENDS:=+kmod-spi-bitbang +kmod-iio-core
#  KCONFIG:=CONFIG_INV_MPU6050_SPI CONFIG_INV_MPU6050_IIO
#  FILES:=$(LINUX_DIR)/drivers/iio/imu/inv_mpu6050/inv-mpu6050.ko
#  FILES+=$(LINUX_DIR)/drivers/iio/imu/inv_mpu6050/inv-mpu6050-spi.ko
#  AUTOLOAD:=$(call AutoProbe,inv-mpu6050 inv-mpu6050-spi)
#endef
#
#define KernelPackage/iio-mpu6xxx-spi/description
#  This driver supports the Invensense MPU6050/6500/9150 and ICM20608
#  motion tracking devices over SPI.
#  This driver can be built as a module. The module will be called
#  inv-mpu6050-spi.
#endef
#
#$(eval $(call KernelPackage,iio-mpu6xxx-spi))
#
