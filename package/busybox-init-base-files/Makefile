#
# Copyright (C) 2007-2016 OpenWrt.org
# Copyright (C) 2010 Vertical Communications
# Copyright (C) 2017-2018 Allwinner Technology Co., Ltd.
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk
include $(BUILD_DIR)/version.mk

PKG_NAME:=busybox-init-base-files
PKG_RELEASE:=167

PKG_FILE_DEPENDS:=$(PLATFORM_DIR)/ $(GENERIC_PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/
PKG_BUILD_DEPENDS:=usign/host
PKG_LICENSE:=GPL-2.0

PKG_CONFIG_DEPENDS := CONFIG_SIGNED_PACKAGES CONFIG_TARGET_INIT_PATH
PKG_DIR:=$(shell pwd)

include $(BUILD_DIR)/package.mk

ifneq ($(DUMP),1)
  STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell $(SH_FUNC) echo $(CONFIG_TARGET_INIT_PATH) | md5s)
  TARGET:=-$(BOARD)
  ifneq ($(wildcard $(PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)-$(PROFILE) $(PLATFORM_SUBDIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)-$(PROFILE)),)
    TARGET:=$(TARGET)-$(PROFILE)
  endif
endif

define Package/busybox-init-base-files
  SECTION:=base
  CATEGORY:=Base system
  DEPENDS:=+libc +SIGNED_PACKAGES:usign
  DEPENDS+= @SYSTEM_INIT_BUSYBOX
  TITLE:=Busybox init base system
  URL:=http://openwrt.org/
  MENU:=1
  VERSION:=$(PKG_RELEASE)-$(REVISION)
endef

define Package/busybox-init-base-files/conffiles
/etc/inittab
/etc/passwd
/etc/shadow
/etc/profile
/etc/shells
$(call $(TARGET)/conffiles)
endef

define Package/busybox-init-base-files/description
 This package contains a busybox init base filesystem and system scripts for OpenWrt.
endef

define Package/busybox-init-base-files/config
	source "$(SOURCE)/Config.in"
endef

define check_nonexisted
	if [ -f $(1) ]; then \
		echo -e "\033[33m************************************************\033[0m"; \
		echo -e "\033[32mCan't customize the \"$(1).\"\033[0m"; \
		echo -e "\033[32mFor details, please refer to \"$(PKG_DIR)/files/etc/init.d/README.\" \033[0m"; \
		echo -e "\033[33m************************************************\033[0m"; \
		exit 1; \
	fi
endef

define Build/Prepare
	$(call check_nonexisted,"$(GENERIC_PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/pseudo_init")
	$(call check_nonexisted,"$(PLATFORM_COMMON_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/pseudo_init")
	$(call check_nonexisted,"$(PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/pseudo_init")

	$(call check_nonexisted,"$(GENERIC_PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/etc/init.d/rcS")
	$(call check_nonexisted,"$(PLATFORM_COMMON_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/etc/init.d/rcS")
	$(call check_nonexisted,"$(PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/etc/init.d/rcS")

	mkdir -p $(PKG_BUILD_DIR)
endef

define Build/Compile/Default
	if [ -f $(PLATFORM_DIR)/tina_busybox-init-base-files.mk ]; then \
		$(MAKE) -C $(PLATFORM_DIR) -f tina_busybox-init-base-files.mk \
			ARCH="$(TARGET_ARCH)" \
			AR="$(TARGET_AR)" \
			CC="$(TARGET_CC)" \
			CXX="$(TARGET_CXX)" \
			CFLAGS="$(TARGET_CFLAGS)" \
			CXXFLAGS="$(TARGET_CXXFLAGS)" \
			CPPFLAGS="$(TARGET_CPPFLAGS)" \
			LDFLAGS="$(TARGET_LDFLAGS)" \
			all; \
	fi
endef
Build/Compile = $(Build/Compile/Default)

define Build/Clean
	if [ -f $(PLATFORM_DIR)/tina_busybox-init-base-files.mk ]; then \
		$(MAKE) -C $(PLATFORM_DIR) -f tina_busybox-init-base-files.mk \
			ARCH="$(TARGET_ARCH)" \
			AR="$(TARGET_AR)" \
			CC="$(TARGET_CC)" \
			CXX="$(TARGET_CXX)" \
			CFLAGS="$(TARGET_CFLAGS)" \
			CXXFLAGS="$(TARGET_CXXFLAGS)" \
			CPPFLAGS="$(TARGET_CPPFLAGS)" \
			LDFLAGS="$(TARGET_LDFLAGS)" \
			clean; \
	fi
endef


ifdef CONFIG_SIGNED_PACKAGES
  define Build/Configure
	[ -s $(BUILD_KEY) -a -s $(BUILD_KEY).pub ] || \
		$(STAGING_DIR_HOST)/bin/usign -G -s $(BUILD_KEY) -p $(BUILD_KEY).pub -c "Local build key"

  endef

  define Package/busybox-init-base-files/install-key
	mkdir -p $(1)/etc/opkg/keys
	$(CP) $(BUILD_KEY).pub $(1)/etc/opkg/keys/`$(STAGING_DIR_HOST)/bin/usign -F -p $(BUILD_KEY).pub`

  endef
endif

define Package/busybox-init-base-files/install

	$(CP) ./$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/* $(1)/

ifeq ($(CONFIG_BUSYBOX_INIT_BASE_FILES_RC_LOG),)
	rm -rf $(1)/etc/init.d/rc.log
endif

ifeq ($(CONFIG_BUSYBOX_INIT_BASE_FILES_RC_LOAD_SCRIPT),)
	rm -rf $(1)/etc/init.d/load_script.conf
endif

ifeq ($(CONFIG_BUSYBOX_INIT_BASE_FILES_RC_MODULES),)
	rm -rf $(1)/etc/init.d/rc.modules
endif

	rm -f $(1)/etc/init.d/README.txt

	$(Package/busybox-init-base-files/install-key)
	-if [ -d $(GENERIC_PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/. ]; then \
		$(CP) $(GENERIC_PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/* $(1)/; \
	fi
	if [ -d $(PLATFORM_COMMON_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/. ]; then \
		$(CP) $(PLATFORM_COMMON_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/* $(1)/; \
	fi
	if [ -d $(PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/. ]; then \
		$(CP) $(PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/* $(1)/; \
	fi
	if [ -d $(PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)-$(PROFILE)/. ]; then \
		$(CP) $(PLATFORM_DIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)-$(PROFILE)/* $(1)/; \
	fi
	if [ -d $(PLATFORM_DIR)/$(PROFILE)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/. ]; then \
		$(CP) $(PLATFORM_DIR)/$(PROFILE)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/* $(1)/; \
	fi
	$(if $(filter-out $(PLATFORM_DIR),$(PLATFORM_SUBDIR)), \
		if [ -d $(PLATFORM_SUBDIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/. ]; then \
			$(CP) $(PLATFORM_SUBDIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/* $(1)/; \
		fi; \
		if [ -d $(PLATFORM_SUBDIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)-$(PROFILE)/. ]; then \
			$(CP) $(PLATFORM_SUBDIR)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)-$(PROFILE)/* $(1)/; \
		fi; \
		if [ -d $(PLATFORM_SUBDIR)/$(PROFILE)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/. ]; then \
			$(CP) $(PLATFORM_SUBDIR)/$(PROFILE)/$(CONFIG_BUSYBOX_INIT_BASE_FILES_PATH)/* $(1)/; \
		fi \
	)

	$(VERSION_SED) \
		$(1)/etc/banner \
		$(1)/etc/openwrt_version

	$(VERSION_SED_SCRIPT) \
		$(1)/etc/openwrt_release \
		$(1)/etc/device_info

	$(SED) "s#%PATH%#$(TARGET_INIT_PATH)#g" \
		$(1)/etc/profile

	mkdir -p $(1)/CONTROL
	mkdir -p $(1)/dev
	mkdir -p $(1)/etc/crontabs
	mkdir -p $(1)/etc/rc.d
	mkdir -p $(1)/overlay
	mkdir -p $(1)/lib/firmware
	$(if $(LIB_SUFFIX),-$(LN) lib $(1)/lib$(LIB_SUFFIX))
	mkdir -p $(1)/mnt
	mkdir -p $(1)/mnt/UDISK
	mkdir -p $(1)/mnt/exUDISK
	mkdir -p $(1)/mnt/SDCARD
	mkdir -p $(1)/mnt/app
	mkdir -p $(1)/proc
	mkdir -p $(1)/tmp
	mkdir -p $(1)/usr/lib
	$(if $(LIB_SUFFIX),-$(LN) lib $(1)/usr/lib$(LIB_SUFFIX))
	mkdir -p $(1)/usr/bin
	mkdir -p $(1)/sys
	mkdir -p $(1)/www
	mkdir -p $(1)/root
	$(LN) /proc/mounts $(1)/etc/mtab
	rm -f $(1)/var
	$(LN) /tmp $(1)/var
	mkdir -p $(1)/tmp/lock
	mkdir -p $(1)/etc

	chmod 1777 $(1)/tmp

	$(call ImageConfigOptions,$(1))
	$(call Package/busybox-init-filess/install-target,$(1))
	for conffile in $(1)/etc/config/*; do \
		if [ -f "$$$$conffile" ]; then \
			grep "$$$${conffile##$(1)}" $(1)/CONTROL/conffiles || \
				echo "$$$${conffile##$(1)}" >> $(1)/CONTROL/conffiles; \
		fi \
	done
endef

ifneq ($(DUMP),1)
  -include $(PLATFORM_DIR)/busybox-init-base-files.mk
endif
$(eval $(call BuildPackage,busybox-init-base-files))
