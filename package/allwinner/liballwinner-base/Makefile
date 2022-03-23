include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=liballwinner-base
PKG_VERSION:=1
PKG_RELEASE:=1

HAVE_SRC := no

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

ifeq ($(CONFIG_ARCH),"aarch64")
LIB_PATH=arm64/$(CONFIG_LIBC)
else ifeq ($(TARGET_CPU_VARIANT),arm926ej-s)
LIB_PATH=arm9/$(CONFIG_LIBC)
else ifeq ($(CONFIG_ARCH), "arm")
LIB_PATH=arm/$(CONFIG_LIBC)
endif

define Package/liballwinner-base
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=base
endef

define Package/liballwinner-base/Default
endef

define Package/liballwinner-base/description
endef


define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)/
	$(CP) ./misc_tools $(PKG_BUILD_DIR)/

ifeq ($(HAVE_SRC),yes)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endif

endef

define Build/Configure
endef

define Build/Compile

ifeq ($(HAVE_SRC),yes)
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endif

	$(MAKE) -C $(PKG_BUILD_DIR)/misc_tools \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/lib
	$(INSTALL_DATA) tina_log.h $(1)/usr/include/
	$(INSTALL_DATA) locker.h $(1)/usr/include/
	$(INSTALL_DATA) taskbase.h $(1)/usr/include/
	$(INSTALL_DATA) thread.h $(1)/usr/include/

ifeq ($(HAVE_SRC),yes)
	$(CP) $(PKG_INSTALL_DIR)/lib/libawbase.so $(1)/lib/
else
	$(CP) $(LIB_PATH)/libawbase.so $(1)/lib/
endif
endef

define Package/liballwinner-base/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/sbin/* $(1)/sbin/

ifeq ($(HAVE_SRC),yes)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libawbase.so $(LIB_PATH)
endif
	$(INSTALL_BIN) $(LIB_PATH)/libawbase.so $(1)/lib/

endef

$(eval $(call BuildPackage,liballwinner-base))
