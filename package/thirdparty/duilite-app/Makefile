include $(TOPDIR)/rules.mk

PKG_NAME:=duilite-app
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

TARGET_LDFLAGS  += -L$(STAGING_DIR)/mnt/app/lib

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  #SECTION:=utils
  #CATEGORY:=Utilities
  CATEGORY:=Thirdparty
  TITLE:=duilite asr demo based on aispeech
  DEPENDS:=+tinyalsa-lib +libstdcpp +alsa-lib +duilite-lib
endef

define Package/$(PKG_NAME)/description
	duilite asr app
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef

define Package/$(PKG_NAME)/install
	#$(INSTALL_DIR) $(1)/usr/bin/ $(1)/usr/lib/

	$(INSTALL_DIR) $(1)/mnt/app/ $(1)/mnt/app/local_asr_bin/ \
		$(1)/mnt/app/auth/ $(1)/mnt/app/temp/ $(1)/mnt/app/configs/ \
		$(1)/mnt/app/audios/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/duilite-app $(1)/mnt/app/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/thirdparty/local_asr_bin/*.bin $(1)/mnt/app/local_asr_bin/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/thirdparty/auth/ec08463b20034e4dabf5c6e86b600b24 $(1)/mnt/app/auth/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/configs/* $(1)/mnt/app/configs/

	#test audios
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/resources/audios/*.wav $(1)/mnt/app/audios/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
