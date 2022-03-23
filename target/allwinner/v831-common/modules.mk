#
# Copyright (C) 2015-2016 Allwinner
#
# This is free software, licensed under the GNU General Public License v2.
# See /build/LICENSE for more information.
#
#define KernelPackage/sunxi-vfe
#  SUBMENU:=$(VIDEO_MENU)
#  TITLE:=sunxi-vfe support
#  FILES:=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-core.ko
#  FILES+=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-memops.ko
#  FILES+=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-dma-contig.ko
#  FILES+=$(LINUX_DIR)/drivers/media/v4l2-core/videobuf2-v4l2.ko
#  FILES+=$(LINUX_DIR)/drivers/media/platform/sunxi-vfe/vfe_io.ko
#  FILES+=$(LINUX_DIR)/drivers/media/platform/sunxi-vfe/device/gc1034_mipi.ko
#  FILES+=$(LINUX_DIR)/drivers/media/platform/sunxi-vfe/vfe_v4l2.ko
#  KCONFIG:=\
#    CONFIG_VIDEO_SUNXI_VFE \
#    CONFIG_CSI_VFE \
#    CONFIG_CCI=m
#  AUTOLOAD:=$(call AutoLoad,90,videobuf2-core videobuf2-memops videobuf2-dma-contig videobuf2-v4l2 vfe_io gc1034_mipi vfe_v4l2)
#endef
#
#define KernelPackage/sunxi-vfe/description
#  Kernel modules for sunxi-vfe support
#endef
#
#$(eval $(call KernelPackage,sunxi-vfe))
#
#define KernelPackage/sunxi-sound
#  SUBMENU:=$(SOUND_MENU)
#  DEPENDS:=+kmod-sound-core
#  TITLE:=sun8iw8 sound support
#  FILES:=$(LINUX_DIR)/sound/soc/sunxi/audiocodec/sun8iw8_sndcodec.ko
#  FILES+=$(LINUX_DIR)/sound/soc/snd-soc-core.ko
#  FILES+=$(LINUX_DIR)/sound/soc/sunxi/audiocodec/sunxi_sndcodec.ko
#  FILES+=$(LINUX_DIR)/drivers/base/regmap/regmap-i2c.ko
#  FILES+=$(LINUX_DIR)/drivers/base/regmap/regmap-spi.ko
#  FILES+=$(LINUX_DIR)/sound/soc/sunxi/audiocodec/switch_hdset.ko
#  FILES+=$(LINUX_DIR)/sound/soc/sunxi/audiocodec/sunxi_codecdma.ko
#  FILES+=$(LINUX_DIR)/sound/soc/sunxi/audiocodec/sunxi_codec.ko
#  FILES+=$(LINUX_DIR)/sound/core/seq/snd-seq.ko
#  FILES+=$(LINUX_DIR)/sound/soc/snd-soc-core.ko
#  AUTOLOAD:=$(call AutoLoad,30,sun8iw12_sndcodec switch_hdset snd-soc-core sunxi_sndcodec sunxi_codecdma sunxi_codec snd-seq regmap-i2c   regmap-spi)
#endef
#
#define KernelPackage/sunxi-sound/description
#  Kernel modules for sun8iw8-sound support
#endef
#
#$(eval $(call KernelPackage,sunxi-sound))
#
