#
#Copyright (c) 2018-2020 Allwinner Technology Co. Ltd.
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=swupdate
PKG_DISTNAME:=swupdate
PKG_VERSION:=2019.04
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE:=$(PKG_DISTNAME)-$(PKG_VERSION).tar.xz
PKG_SOURCE_SUBDIR:=$(PKG_DISTNAME)-$(PKG_VERSION)
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_URL:=https://github.com/sbabic/swupdate.git
PKG_SOURCE_VERSION:=e83730c4ec30134cdf56a2fd1d62a3767ade8133

SWUPDATE_SYM=$(if $(CONFIG_SWUPDATE_CUSTOM),CONFIG,DEFAULT)
SWUPDATE_IF_ENABLED=$(if $(CONFIG_SWUPDATE_$(SWUPDATE_SYM)_$(1)),$(2))

PKG_BUILD_DEPENDS:=\
  +zlib \
  +SWUPDATE_$(SWUPDATE_SYM)_LIBCONFIG:libconfig \
  +SWUPDATE_$(SWUPDATE_SYM)_CURL:curl \
  +SWUPDATE_$(SWUPDATE_SYM)_UBOOT:uboot-envtools \
  +SWUPDATE_$(SWUPDATE_SYM)_MTD:mtd-utils \
  +SWUPDATE_$(SWUPDATE_SYM)_AWBOOT_HANDLER:ota-burnboot \
  +SWUPDATE_$(SWUPDATE_SYM)_HASH_VERIFY:libopenssl \
  +SWUPDATE_$(SWUPDATE_SYM)_RDIFFHANDLER:librsync

TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include
TARGET_CFLAGS += -I$(STAGING_DIR)/include

include $(BUILD_DIR)/package.mk

ifeq ($(DUMP),)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell $(SH_FUNC) grep '^CONFIG_SWUPDATE_' $(TOPDIR)/.config | md5s)
endif

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=software update for embedded system
  DEPENDS:=\
    +zlib \
    +SWUPDATE_CONFIG_LIBCONFIG:libconfig \
    +SWUPDATE_CONFIG_CURL:curl \
    +SWUPDATE_CONFIG_UBOOT:uboot-envtools \
    +SWUPDATE_CONFIG_MTD:mtd-utils \
    +SWUPDATE_CONFIG_AWBOOT_HANDLER:ota-burnboot \
    +SWUPDATE_CONFIG_SURICATTA_HAWKBIT:libjson-c \
    +SWUPDATE_CONFIG_HASH_VERIFY:libopenssl \
    +SWUPDATE_CONFIG_RDIFFHANDLER:librsync \

  MENU:=1
endef

define Package/$(PKG_NAME)/description
  SWUpdate provides a reliable way to update the software on an embedded system.
  Sources are hosted at https://github.com/sbabic/swupdate
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

define Build/Configure
	rm -f $(PKG_BUILD_DIR)/.configured*
	grep 'CONFIG_SWUPDATE_$(SWUPDATE_SYM)' $(TOPDIR)/.config | sed -e "s,\\(# \)\\?CONFIG_SWUPDATE_$(SWUPDATE_SYM)_\\(.*\\),\\1CONFIG_\\2,g" > $(PKG_BUILD_DIR)/.config
	yes 'n' | $(MAKE) -C $(PKG_BUILD_DIR) $(MAKE_FLAGS) oldconfig
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		CC="$(TARGET_CC)" \
		CROSS_COMPILE="$(TARGET_CROSS)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LD="$(TARGET_CC)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		install
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/swupdate $(1)/sbin
ifeq ($(CONFIG_SWUPDATE_CONFIG_REMOVE_PROGRESS),)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/progress $(1)/sbin
endif
ifeq ($(CONFIG_SWUPDATE_CONFIG_REMOVE_CLIENT),)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/client $(1)/sbin
endif

ifneq ($(CONFIG_SWUPDATE_CONFIG_SURICATTA_HAWKBIT),)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/hawkbitcfg $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/sendtohawkbit $(1)/sbin
endif

ifeq ($(CONFIG_SWUPDATE_CONFIG_REMOVE_AUTORUN),)
	$(INSTALL_BIN) ./swupdate_autorun.init $(1)/etc/init.d/S99swupdate_autorun
	$(INSTALL_BIN) ./swupdate_cmd.sh $(1)/sbin
endif

endef

$(eval $(call BuildPackage,$(PKG_NAME)))
