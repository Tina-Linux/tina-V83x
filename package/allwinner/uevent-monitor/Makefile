include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=uevent-monitor
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:=+libev
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=tina uevent monitor
endef

define Package/$(PKG_NAME)/description
  tina uevent monitor demo, contains function:
listen uevent
handler usb connect/disconnect,sdcard plugin/plugout event
mount/umount UDISK
enable/disable usb gadget mass_storage
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

$(warning "KERNEL: $(KERNEL_PATCHVER)")
ifneq ($(filter 4.4 4.9, $(KERNEL_PATCHVER)), )
  LOCAL_CFLAGS=-DGADGET_CONFIGFS
endif

define Build/Compile
	#compile demo
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(LOCAL_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all \
		V=1
endef

define Build/InstallDev
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
