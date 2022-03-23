include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=libuapi
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

KERNEL_VERSION = CONF_KERNEL_VERSION_3_10

#config the kernel version,which depends on $(KERNEL_PATCHVER)
ifeq ($(KERNEL_PATCHVER),3.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
endif

ifeq ($(KERNEL_PATCHVER),3.10)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
endif

ifeq ($(KERNEL_PATCHVER),4.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_4
endif

ifeq ($(KERNEL_PATCHVER),4.9)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_9
endif
#end config the kernel version

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=unit api for Allwinner
  MENU:=1
  DEPENDS:=+SUNXI_DISPLAY_GPU:libdrm
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r16 r40 r18 r311))
	DEPENDS+= +SUNXI_DISPLAY_GPU:mali400-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r30))
	DEPENDS+= +SUNXI_DISPLAY_GPU:mali-t760-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r58))
	DEPENDS+= +SUNXI_DISPLAY_GPU:sgx544-um
endif
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  unit api for allwinner
endef

define Package/$(PKG_NAME)/config
if PACKAGE_$(PKG_NAME)
choice
prompt "Choose display framework"
	default SUNXI_DISPLAY_TWO
	help
	Select the preferred minigui running mode.
config SUNXI_DISPLAY_ONE
	bool "sunxi_display1"
config SUNXI_DISPLAY_TWO
	bool "sunxi_display2"
endchoice
choice
prompt "Choose alloc method"
	default SUNXI_ALLOC_CMA
	help
	Select how to apply for memory.
config SUNXI_ALLOC_CMA
	bool "sunxi_alloc_cma"
config SUNXI_ALLOC_IOMMU
	bool "sunxi_alloc_iommu"
endchoice
choice
prompt "Choose display rendering mode"
	default SUNXI_DISPLAY_DE
	help
		Choose display rendering mode, DE or GPU.
config SUNXI_DISPLAY_DE
	bool "DE"
config SUNXI_DISPLAY_GPU
	bool "GPU"
	help
	Define the macro SUNXI_DISPLAY_GPU, use GPU rendering, send the RGBA data
	after rendering to fb1 display, execute export FRAMEBUFFER=/dev/fb1
	before executing the program, no longer support DispSetEnable, DispSetSrcRect,
	DispSetZorder and DispSetIoctl functions.

	Define the macro SUNXI_DISPLAY_GPU and SUNXI_DISPLAY_GPU_BUF, use GPU rendering,
	send the RGBA data after rendering to layer. No longer support DispSetSrcRect functions.
	Default use.
endchoice
endif
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

ifeq ($(CONFIG_SUNXI_DISPLAY_TWO),y)
TARGET_CFLAGS+=-D__SUNXI_DISPLAY2__
TARGET_CFLAGS+=-D$(KERNEL_VERSION)
endif
ifeq ($(CONFIG_SUNXI_DISPLAY_ONE),y)
TARGET_CFLAGS+=-D__SUNXI_DISPLAY__
TARGET_CFLAGS+=-D$(KERNEL_VERSION)
endif

ifeq ($(CONFIG_SUNXI_ALLOC_CMA),y)
TARGET_CFLAGS+=-DCONF_KERNEL_CMA
endif
ifeq ($(CONFIG_SUNXI_ALLOC_IOMMU),y)
TARGET_CFLAGS+=-DCONF_KERNEL_IOMMU
endif

ifeq ($(CONFIG_SUNXI_DISPLAY_GPU),y)
	#TARGET_CFLAGS+=-DSUNXI_DISPLAY_GPU
	TARGET_CFLAGS+=-DSUNXI_DISPLAY_GPU -DSUNXI_DISPLAY_GPU_BUF
	TARGET_LDFLAGS+=-lEGL -lMali -lGLESv2
else
	TPLAYER_EXTRA_CFLAGS+=-DSUNXI_DISPLAY_DE
endif

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
