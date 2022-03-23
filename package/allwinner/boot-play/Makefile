include $(TOPDIR)/rules.mk

PKG_NAME:=boot-play
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/kernel.mk
include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:= +libpng +libubox +tinymp3
  TITLE:=boot play
endef

define Package/$(PKG_NAME)/config
menu "Configuration"
        depends on PACKAGE_boot-play
config BOOT_PLAY_IN_PREINIT
        bool
        default n
        prompt "Perform boot-play in preinit or not"
config BOOT_PLAY_IN_PREINIT_AFTER_KMODLOADER
	bool
	default n
	depends on BOOT_PLAY_IN_PREINIT
	prompt "Perform boot-play in preinit but after kmodloader"
config BOOT_ANIMATION_PRIORITY
        int "boot animation process priority"
        default -20
        range -20 0
config BOOT_MUSIC_PRIORITY
        int "boot music process priority"
        default -10
        range -20 0
config BOOT_ANIMATION_EACHLOAD_IMAGE
	bool
	default n
	prompt "bootaniamtion image each-load"
	help
	Say Y here, means load image,show image,free image everytime
choice
	prompt "boot-play finish condiction"
	default BOOT_PLAY_COMPLETE_ANIMATION
	help
	  Select boot-play complete condition.

	config BOOT_PLAY_COMPLETE_ANIMATION
		bool "animation complete or exit"
	config BOOT_PLAY_COMPLETE_MUSIC
		bool "music complete"
	config BOOT_PLAY_COMPLETE_TIMEOUT
		bool "boot-play timeout"
endchoice
config BOOT_PLAY_COMPLETE_TIMEOUT_VALUE
	depends on BOOT_PLAY_COMPLETE_TIMEOUT
	int "set boot-play timeout value"
	default 5
	range 1 10
endmenu
endef

PKG_CONFIG_DEPENDS += CONFIG_BOOT_PLAY_IN_PREINIT \
			CONFIG_BOOT_PLAY_IN_PREINIT_AFTER_KMODLOADER \
			CONFIG_BOOT_ANIMATION_PRIORITY \
			CONFIG_BOOT_MUSIC_PRIORITY \
			CONFIG_BOOT_ANIMATION_EACHLOAD_IMAGE \
			CONFIG_BOOT_PLAY_COMPLETE_ANIMATION \
			CONFIG_BOOT_PLAY_COMPLETE_MUSIC \
			CONFIG_BOOT_PLAY_COMPLETE_TIMEOUT \
			CONFIG_BOOT_PLAY_COMPLETE_TIMEOUT_VALUE

ifeq ($(CONFIG_BOOT_ANIMATION_EACHLOAD_IMAGE),y)
CMAKE_OPTIONS += -DBOOT_ANIMATION_EACHLOAD_IMAGE=1
endif

CMAKE_OPTIONS += -DBOOT_ANIMATION_PRIORITY=$(CONFIG_BOOT_ANIMATION_PRIORITY)
CMAKE_OPTIONS += -DBOOT_MUSIC_PRIORITY=$(CONFIG_BOOT_MUSIC_PRIORITY)

ifeq ($(CONFIG_BOOT_PLAY_COMPLETE_TIMEOUT),y)
CMAKE_OPTIONS += -DCOMPLETE_TIMEOUT=$(CONFIG_BOOT_PLAY_COMPLETE_TIMEOUT_VALUE)
endif
ifeq ($(CONFIG_BOOT_PLAY_COMPLETE_ANIMATION),y)
CMAKE_OPTIONS += -DCOMPLETE_ANIMATION=1
endif
ifeq ($(CONFIG_BOOT_PLAY_COMPLETE_MUSIC),y)
CMAKE_OPTIONS += -DCOMPLETE_MUSIC=1
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_DIR) $(1)/usr/res/boot-play
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/sbin/boot-play $(1)/sbin/
	$(INSTALL_BIN) play.init $(1)/etc/init.d/play
ifeq ($(CONFIG_BOOT_PLAY_IN_PREINIT), y)
	sed -i "/START/d" $(1)/etc/init.d/play
endif
	if [ -d $(GENERIC_PLATFORM_DIR)/boot-play/. ]; then \
		$(CP) $(GENERIC_PLATFORM_DIR)/boot-play/* $(1)/usr/res/boot-play/; \
	fi
	if [ -d $(PLATFORM_COMMON_DIR)/boot-play/. ]; then \
		$(CP) $(PLATFORM_COMMON_DIR)/boot-play/* $(1)/usr/res/boot-play/; \
	fi
	if [ -d $(PLATFORM_DIR)/boot-play/. ]; then \
		$(CP) $(PLATFORM_DIR)/boot-play/* $(1)/usr/res/boot-play/; \
	fi
	if [ "`ls -A $(1)/usr/res/boot-play/`" = "" ]; then \
		$(CP) files/bootanimation/* $(1)/usr/res/boot-play/; \
		$(CP) files/music/* $(1)/usr/res/boot-play/; \
	fi
	$(INSTALL_BIN) music-play/default.sh $(1)/sbin/music-play
ifeq ($(TARGET_BOARD_PLATFORM),r16)
	$(INSTALL_BIN) music-play/r16_boot-play.sh $(1)/sbin/music-play
endif

ifeq ($(TARGET_BOARD_PLATFORM),r58)
	$(INSTALL_BIN) music-play/r58_boot-play.sh $(1)/sbin/music-play
endif

ifeq ($(TARGET_BOARD_PLATFORM),r40)
	$(INSTALL_BIN) music-play/r40_boot-play.sh $(1)/sbin/music-play
endif

ifeq ($(TARGET_BOARD_PLATFORM),r18)
	$(INSTALL_BIN) music-play/r18_boot-play.sh $(1)/sbin/music-play
endif

ifeq ($(TARGET_BOARD_PLATFORM),r6)
	$(INSTALL_BIN) music-play/c600_boot-play.sh $(1)/sbin/music-play
endif

ifeq ($(TARGET_BOARD_PLATFORM),g102)
	$(INSTALL_BIN) music-play/G102_boot-play.sh $(1)/sbin/music-play
endif

ifeq ($(TARGET_BOARD_PLATFORM),r11)
	$(INSTALL_BIN) music-play/MR100_boot-play.sh $(1)/sbin/music-play
endif

ifeq ($(TARGET_BOARD_PLATFORM),r7)
	$(INSTALL_BIN) music-play/MR100_boot-play.sh $(1)/sbin/music-play
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
