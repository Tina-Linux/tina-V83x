include $(TOPDIR)/rules.mk

PKG_NAME:=sampleapp
PKG_VERSION:=1.7.1
PKG_RELEASE:=1

PKG_JOBS := -j32

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS += -DGSTREAMER_MEDIA_PLAYER=ON -DPORTAUDIO=ON  -DCMAKE_BUILD_TYPE=DEBUG \
				 -DPORTAUDIO_LIB_PATH=$(STAGING_DIR)/usr/lib/libportaudio.so \
				 -DPORTAUDIO_INCLUDE_DIR=$(STAGING_DIR)/usr/include

#ESP
#CMAKE_OPTIONS += \
				-DESP_PROVIDER=ON \
				-DESP_INCLUDE_DIR=$(STAGING_DIR)/usr/include/esp \
				-DESP_LIB_PATH=esp

#sensory
#CMAKE_OPTIONS += \
			-DSENSORY_KEY_WORD_DETECTOR=ON \
			-DSENSORY_KEY_WORD_DETECTOR_LIB_PATH=sensory \
			-DSENSORY_KEY_WORD_DETECTOR_INCLUDE_DIR=$(STAGING_DIR)/usr/include

#Amazonlite detector
CMAKE_OPTIONS += \
			-DAMAZONLITE_KEY_WORD_DETECTOR=ON \
			-DAMAZONLITE_KEY_WORD_DETECTOR_LIB_PATH=pryon_lite \
			-DAMAZONLITE_KEY_WORD_DETECTOR_INCLUDE_DIR=$(STAGING_DIR)/usr/include \
			-DAMAZONLITE_KEY_WORD_DETECTOR_DYNAMIC_MODEL_LOADING=ON

define Package/$(PKG_NAME)
  SECTION:=avs
  CATEGORY:=avs
  DEPENDS:= +avs-sdk +portaudio +libconfigutils +libtutuclear +libev
  TITLE:=avs sdk SampleApp only
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./files/source/* $(PKG_BUILD_DIR)/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/ $(1)/etc/avs/json $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
	$(INSTALL_BIN) ./files/AlexaClientSDKConfig.json $(1)/etc/avs/
endef
#$(eval $(call BuildPackage,$(PKG_NAME)))
