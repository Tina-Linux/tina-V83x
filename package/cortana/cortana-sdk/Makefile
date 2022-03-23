include $(TOPDIR)/rules.mk

PKG_NAME:=cortana-sdk
PKG_VERSION:=2.0.4-12012017
PKG_RELEASE:=1

PKG_LICENSE:=MIT

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz
PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  CATEGORY:=Cortana
  TITLE:=Microsoft Cortana Devices SDK
  DEPENDS:=@EXT_TOOLCHAIN_LIBC_USE_GLIBC \
	  +glib2 +libstdcpp +alsa-lib \
	  +libcurl +libopenssl +libwebsockets-openssl +ca-certificates \
	  +breakpad-libs +libuuid +libb64 +libexpat +libsqlite3 \
	  +gstreamer1-libs +libgstreamer1 +gst1-libav \
	  +gstreamer1-plugins-base +@gst1-plugins-base-all \
	  +gstreamer1-plugins-good +@gst1-plugins-good-all \
	  +gstreamer1-plugins-bad +@gst1-plugins-bad-all \
	  +gstreamer1-plugins-aw +@gst1-plugins-aw-all
endef

define Package/$(PKG_NAME)/description
  This SDK contains both IoT device SDKs and Cortana service SDKs.
  Device SDKs enable you connect client devices to the Cortana Cloud services
  and uses the Azure IoT device SDK as a platform abstraction.  Service SDKs
  enable you to manage and customize your Cortana experiences.
endef

define Package/$(PKG_NAME)/config
	source "$(SOURCE)/Config.in"
endef

ARCHITECTURE:="arm-sunxi"

ifeq ($(CONFIG_CORTANA_SDK_CMAKE_BUILD_TYPE_DEBUG),y)
    CMAKE_BUILD_TYPE:=Debug
else ifeq ($(CONFIG_CORTANA_SDK_CMAKE_BUILD_TYPE_RELEASE),y)
    CMAKE_BUILD_TYPE:=Release
else ifeq ($(CONFIG_CORTANA_SDK_CMAKE_BUILD_TYPE_RELWITHDEBINFO),y)
    CMAKE_BUILD_TYPE:=RelWithDebInfo
else ifeq ($(CONFIG_CORTANA_SDK_CMAKE_BUILD_TYPE_MINSIZEREL),y)
    CMAKE_BUILD_TYPE:=MinSizeRel
else
    CMAKE_BUILD_TYPE:=None
endif

CMAKE_OPTIONS+=-DARCHITECTURE=$(ARCHITECTURE) \
	-DIOT_CORTANA_NO_CDP=$(if $(filter-out $(CONFIG_CORTANA_SDK_ENABLE_CDP),y),1) \
	-DIOT_CORTANA_NO_COMPANION=$(if $(filter-out $(CONFIG_CORTANA_SDK_ENABLE_COMPANION),y),1) \
    -DENABLE_SKYPE=$(if $(filter $(CONFIG_CORTANA_SDK_ENABLE_SKYPE),y),ON,OFF) \
    -DENABLE_HARNESS=$(if $(filter $(CONFIG_CORTANA_SDK_ENABLE_HARNESS),y),ON,OFF) \
	-DDISABLE_KEYWORD_SPOTTER=$(if $(filter-out $(CONFIG_CORTANA_SDK_ENABLE_KEYWORD_SPOTTER),y),1) \
	-DSHARED_UTILS_NO_LOGGING=$(if $(filter-out $(CONFIG_CORTANA_SDK_ENABLE_LOGGING),y),1) \
	-DIOT_CORTANA_USE_USBEXT=$(if $(filter $(CONFIG_CORTANA_SDK_USE_USBEXT),y),1) \
	-DIOT_CORTANA_EXTERNAL_MUSIC_MANAGER=$(if $(filter $(CONFIG_CORTANA_SDK_EXTERNAL_MUSIC_MANAGER),y),1) \
	-DIOT_CORTANA_SOUND_VOICE_PATH="$(CONFIG_CORTANA_SDK_SOUND_VOICE_PATH)/" \
	-DIOT_CORTANA_SOUND_PATH="$(CONFIG_CORTANA_SDK_SOUND_PATH)/" \
	-DIOT_CORTANA_DATA_PATH="$(CONFIG_CORTANA_SDK_DATA_PATH)/" \
	-DWRITE_DUMP_PATH="$(CONFIG_CORTANA_SDK_DUMP_PATH)/" \
	-DENABLE_CORTANA_BASIC_FIRST_SAMPLE=$(if $(filter $(CONFIG_CORTANA_SDK_BUILD_BASIC_FIRST_SAMPLE),y),ON,OFF) \
	-DENABLE_IOT_CORTANA_SDK_SAMPLE=$(if $(filter $(CONFIG_CORTANA_SDK_BUILD_IOT_CORTANA_SDK_SAMPLE),y),ON,OFF) \
	-DENABLE_TESTS=$(if $(filter $(CONFIG_CORTANA_SDK_BUILD_TESTS),y),ON,OFF) \
	-DCMAKE_VERBOSE_MAKEFILE=$(if $(filter $(CONFIG_CORTANA_SDK_CMAKE_VERBOSE_MAKEFILE),y),ON,OFF) \
	-DCMAKE_BUILD_TYPE=$(CMAKE_BUILD_TYPE)

define Build/Prepare
	$(PKG_UNPACK)
	$(CP) ./build_without_git/* $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/c/public/lib/Linux/arm-sunxi
	$(CP) ./libs/* $(PKG_BUILD_DIR)/c/public/lib/Linux/arm-sunxi
	find $(PKG_BUILD_DIR)/OSS -maxdepth 1 -name "*.sh" -a ! -perm /u=x \
		-exec chmod a+x {} \;
	find $(PKG_BUILD_DIR)/c -name "*.sh" -a ! -perm /u=x -exec chmod a+x {} \;
	find $(PKG_BUILD_DIR)/c/public/res/Linux/scripts -type f -exec chmod a+x {} \;
	find $(PKG_BUILD_DIR)/provision -name "*.sh" -a ! -perm /u=x -exec chmod a+x {} \;
	find $(PKG_BUILD_DIR)/tests -name "*.sh" -a ! -perm /u=x -exec chmod a+x {} \;
	$(Build/Patch)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/c/cortana-core/lib*-stub.a $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	find $(PKG_INSTALL_DIR)/usr/lib -name "*.so*" -exec $(CP) {} $(1)/usr/lib \;
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/c/public/res/Linux/scripts/* \
		$(1)/usr/bin/
ifeq ($(CONFIG_CORTANA_SDK_BUILD_BASIC_FIRST_SAMPLE),y)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cortana-basic-first-sample \
		$(1)/usr/bin/
endif
ifeq ($(CONFIG_CORTANA_SDK_BUILD_IOT_CORTANA_SDK_SAMPLE),y)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/iot-cortana-sdk-sample \
		$(1)/usr/bin/
endif
	$(INSTALL_DIR) $(1)/etc
	$(CP) $(PKG_BUILD_DIR)/c/public/res/Linux/etc/asound.conf \
		$(1)/etc/
	$(INSTALL_DIR) $(1)/$(CONFIG_CORTANA_SDK_SOUND_VOICE_PATH)
	$(CP) $(PKG_BUILD_DIR)/c/public/res/C_*.wav \
		$(1)/$(CONFIG_CORTANA_SDK_SOUND_VOICE_PATH)/
	$(INSTALL_DIR) $(1)/$(CONFIG_CORTANA_SDK_SOUND_PATH)
	find $(PKG_BUILD_DIR)/c/public/res -name "*.wav" -a ! -name "C_*" \
		-exec $(CP) {} $(1)/$(CONFIG_CORTANA_SDK_SOUND_PATH)/ \;
	$(INSTALL_DIR) $(1)/$(CONFIG_CORTANA_SDK_DATA_PATH)
	$(CP) $(PKG_BUILD_DIR)/c/public/res/heycortana_en-US.table \
		$(1)/$(CONFIG_CORTANA_SDK_DATA_PATH)/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
