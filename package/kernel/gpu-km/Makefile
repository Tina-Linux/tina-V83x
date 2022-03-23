#
# Copyright (C) 2018 Allwinner Technology Limited. All rights reserved
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#
# Author: Albert Yu <yuxyun@allwinnertech.com>
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk
-include $(TINA_BUILD_TOP)/.config

ifeq ($(CONFIG_COMPLILE_KERNEL64_USER32),y)
LINUX_KARCH=arm64
TARGET_CROSS=aarch64-openwrt-linux-
endif

MAKE_OPTS:=\
	CROSS_COMPILE=$(TARGET_CROSS) \
	KDIR=$(LINUX_DIR) \
	KERNELDIR=$(LINUX_DIR) \
	LICHEE_PLATFORM=linux \
	LICHEE_KDIR=$(LINUX_DIR)

ifeq ($(TARGET_ARCH), arm)
MAKE_OPTS+=\
	ARCH=arm
else ifeq ($(TARGET_ARCH), aarch64)
MAKE_OPTS+=\
        ARCH=arm64
else
	$(error Invalid TARGET_ARCH)
endif

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r16 r40 r18 r311 t7))
PKG_NAME:=mali-utgard-km
#Make sure TARGET_PLATFORM is null to avoid build error for Mali Utgard Device Driver
TARGET_PLATFORM=
GPU_NEW_DIR := $(LINUX_DIR)/modules/gpu/mali-utgard
GPU_OLD_DIR := $(GPU_NEW_DIR)/kernel_mode
ifneq (wildcard $(GPU_OLD_DIR)/driver/src/devicedrv/mali/.version),)
GPU_BUILD_DIR := $(GPU_OLD_DIR)/driver/src/devicedrv/mali
else
GPU_BUILD_DIR := $(GPU_NEW_DIR)/driver/src/devicedrv/mali
endif
GPU_KO_DIR := $(GPU_BUILD_DIR)/mali.ko
ifeq ($(TARGET_LINUX_VERSION), $(filter $(TARGET_LINUX_VERSION),3.4))
	MAKE_OPTS+=USING_DT=0
else
	MAKE_OPTS+=USING_DT=1
endif
#For Mali Midgard platforms
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r30))
PKG_NAME:=mali-midgard-km
GPU_BUILD_DIR:=$(LINUX_DIR)/modules/gpu/mali-midgard/kernel_mode/driver
GPU_KO_DIR:=$(GPU_BUILD_DIR)/drivers/gpu/arm/midgard/mali_kbase.ko
#For SGX-544 platforms
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r58))
PKG_NAME:=sgx544-km
GPU_BUILD_DIR:=$(LINUX_DIR)/modules/gpu/sgx544/linux/kernel_mode/eurasia_km/eurasiacon/build/linux2/sunxi_linux
GPU_KO_DIR:=$(LINUX_DIR)/modules/gpu/sgx544/linux/kernel_mode/eurasia_km/eurasiacon/binary2_sunxi_linux_release/target/pvrsrvkm.ko
GPU_KO_DIR+=$(LINUX_DIR)/modules/gpu/sgx544/linux/kernel_mode/eurasia_km/eurasiacon/binary2_sunxi_linux_release/target/dc_sunxi.ko
#For GE8300 platforms
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), mr813))
PKG_NAME:=ge8300-km
#GPU_BUILD_DIR:=$(LINUX_DIR)/modules/gpu/img-rgx/linux/rogue_km
GPU_BUILD_DIR:=$(LINUX_DIR)/modules/gpu/
ifeq ($(CPU_ARCH), arm)
GPU_KO_DIR:=$(GPU_BUILD_DIR)/img-rgx/linux/rogue_km/binary_sunxi_linux_release/target_armv7-a/pvrsrvkm.ko
GPU_KO_DIR+=$(GPU_BUILD_DIR)/img-rgx/linux/rogue_km/binary_sunxi_linux_release/target_armv7-a/dc_sunxi.ko
else
GPU_KO_DIR:=$(GPU_BUILD_DIR)/img-rgx/linux/rogue_km/binary_sunxi_linux_release/target_aarch64/pvrsrvkm.ko
GPU_KO_DIR+=$(GPU_BUILD_DIR)/img-rgx/linux/rogue_km/binary_sunxi_linux_release/target_aarch64/dc_sunxi.ko
endif
endif

PKG_RELEASE:=2

include $(BUILD_DIR)/package.mk

define KernelPackage/mali-utgard-km
	SUBMENU:=Video Support
	TITLE:=Mali Utgard Device Driver
	DEPENDS:=+WESTON_DRM:kmod-sunxi-drm +!WESTON_DRM:kmod-sunxi-disp
	FILES:=$(GPU_KO_DIR)
	AUTOLOAD:=$(call AutoLoad,50,mali)
	KCONFIG:=
endef

define KernelPackage/mali-midgard-km
	SUBMENU:=Video Support
	TITLE:=Mali Midgard device driver
	DEPENDS:=+WESTON_DRM:kmod-sunxi-drm +!WESTON_DRM:kmod-sunxi-disp
	FILES:=$(GPU_KO_DIR)
	AUTOLOAD:=$(call AutoLoad,50,mali_kbase)
	KCONFIG:=
endef

define KernelPackage/sgx544-km
	SUBMENU:=Video Support
	TITLE:=SGX-544 Device Driver
	DEPENDS:= +kmod-sunxi-disp
	FILES:=$(GPU_KO_DIR)
	AUTOLOAD:=$(call AutoLoad,20,pvrsrvkm,1)
	AUTOLOAD+=$(call AutoLoad,25,dc_sunxi,1)
	KCONFIG:=
endef

define KernelPackage/ge8300-km
	SUBMENU:=Video Support
	TITLE:=GE8300 Device Driver
	DEPENDS:= +kmod-sunxi-disp
	FILES:=$(GPU_KO_DIR)
	AUTOLOAD:=$(call AutoLoad,20,pvrsrvkm,1)
	AUTOLOAD+=$(call AutoLoad,25,dc_sunxi,1)
	KCONFIG:=
endef

define Build/Configure
endef

define Build/Compile
	$(MAKE) -j -C $(GPU_BUILD_DIR) $(MAKE_OPTS)
endef

ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r16 r40 r18 r311 t7))
$(eval $(call KernelPackage,mali-utgard-km))
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r30))
$(eval $(call KernelPackage,mali-midgard-km))
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), r58))
$(eval $(call KernelPackage,sgx544-km))
else ifeq ($(TARGET_PLATFORM), $(filter $(TARGET_PLATFORM), mr813))
$(eval $(call KernelPackage,ge8300-km))
endif
