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
  FILES+=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-memops.ko
  FILES+=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-v4l2.ko
  FILES+=$(TOPDIR)/target/allwinner/v831-YuzukiIRC/drivers/videobuf2-dma-contig.ko
  FILES+=$(TOPDIR)/target/allwinner/v831-YuzukiIRC/drivers/vin_io.ko
  FILES+=$(TOPDIR)/target/allwinner/v831-YuzukiIRC/drivers/vin_v4l2.ko
  FILES+=$(TOPDIR)/target/allwinner/v831-YuzukiIRC/drivers/sp2305_mipi.ko
  FILES+=$(TOPDIR)/target/allwinner/v831-YuzukiIRC/drivers/ov9732_mipi.ko
  FILES+=$(TOPDIR)/target/allwinner/v831-YuzukiIRC/drivers/sensor_power.ko
  AUTOLOAD:=$(call AutoProbe,videobuf2-core videobuf2-dma-contig videobuf2-memops videobuf2-v4l2 vin_io vin_v4l2 sp2305_mipi ov9732_mipi sensor_power)
endef

define KernelPackage/vin_v4l2/description
 Kernel modules for video input support
endef

$(eval $(call KernelPackage,vin-v4l2))

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
