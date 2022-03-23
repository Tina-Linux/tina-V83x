include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/package.mk

PKG_NAME:=resample
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)


define Package/resample/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=Tina resample
  DEPENDS:=+libpthread +liballwinner-base
endef

define Package/resample
  $(call Package/resample/Default)
  MENU:=1
  DEFAULT:=1
endef

define Package/resample/description
	Tina resample
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./AC320 $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef

define Build/Compile
	#tinyalsa-master
	$(MAKE) -C $(PKG_BUILD_DIR)/AC320/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		BUILDPATH="$(PKG_BUILD_DIR)/.build" \
		LIBPATH="$(PKG_BUILD_DIR)/.lib" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib $(1)/usr/include
ifeq ($(CONFIG_ARCH), "aarch64")
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/.lib/libresamplerate_64.so $(1)/usr/lib/libresamplerate.so
else
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/AC320/arm/$(CONFIG_LIBC)/libresamplerate.so $(1)/usr/lib
endif
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/AC320/*.h $(1)/usr/include
endef

define Package/resample/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/bin
ifeq ($(CONFIG_ARCH), "aarch64")
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/.lib/libresamplerate_64.so $(1)/lib/libresamplerate.so
else
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/AC320/arm/$(CONFIG_LIBC)/libresamplerate.so $(1)/lib
endif
endef

$(eval $(call BuildPackage,resample))
