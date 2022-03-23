include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME:=smartlinkd
PKG_VERSION:=0.2.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

ifeq ($(TARGET_CPU_VARIANT),arm926ej-s)
  ARCH=arm926ej-s
endif

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  SUBMENU:=$(PKG_NAME)
endef

define Package/$(PKG_NAME)-lib
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=$(PKG_NAME)-lib
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base +alsa-lib +libopenssl +libuci
endef

define Package/$(PKG_NAME)-demo
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=smartlink demo
  DEPENDS:= +$(PKG_NAME)-lib +wifimanager
endef

define Package/$(PKG_NAME)-lib/description
	smartlink for none input wifi direct platform
endef

define Package/$(PKG_NAME)-lib/config
    config XRADIO_AIRKISS
        bool "enable xradio airkiss support for lib"
    config XRADIO_SMARTCONFIG
        bool "enable xradio smartconfig support for lib"
    config SOFT_AP
        bool "enable soft ap support for lib"
    config SOUND_WAVE
        bool "enable sound wave support for lib"
endef

define Package/$(PKG_NAME)-demo/description
	smartlink demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Compile/$(PKG_NAME)-lib
	#compile libs
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		CONFIG_LIBC="$(CONFIG_LIBC)" \
		CPU="$(ARCH)" \
		XRADIO_AIRKISS_CONFIG="$(CONFIG_XRADIO_AIRKISS)" \
		XRADIO_SMARTCONFIG_CONFIG="$(CONFIG_XRADIO_SMARTCONFIG)" \
		ADT_CONFIG="$(CONFIG_SOUND_WAVE)" \
		SOFT_AP_CONFIG="$(CONFIG_SOFT_AP)" \
		all
endef

ifeq ($(CONFIG_PACKAGE_smartlinkd-demo),y)
define Build/Compile/$(PKG_NAME)-demo
	#compile demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		XRADIO_AIRKISS_CONFIG="$(CONFIG_XRADIO_AIRKISS)" \
		XRADIO_SMARTCONFIG_CONFIG="$(CONFIG_XRADIO_SMARTCONFIG)" \
		ADT_CONFIG="$(CONFIG_SOUND_WAVE)" \
		LIBC_CONFIG="$(CONFIG_LIBC)" \
		SOFT_AP_CONFIG="$(CONFIG_SOFT_AP)" \
		all
endef
endif


define Build/Compile
	$(Build/Compile/$(PKG_NAME)-lib)
	$(Build/Compile/$(PKG_NAME)-demo)
endef

define Package/$(PKG_NAME)-lib/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libsm_mg.so $(1)/usr/lib/
endef

define Package/$(PKG_NAME)-demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef


$(eval $(call BuildPackage,$(PKG_NAME)-lib))
$(eval $(call BuildPackage,$(PKG_NAME)-demo))
