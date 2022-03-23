include $(TOPDIR)/rules.mk

PKG_NAME:=avs-sampleapp
PKG_VERSION:=1.11
PKG_RELEASE:=1

PKG_JOBS := -j32

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)/build_dir
CMAKE_SOURCE_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)/source_dir

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  SUBMENU:=AVS
  DEPENDS:= +avs-device-sdk +tconfigs +tutuclear-lib
  TITLE:=avs sdk SampleApp only
endef

define Build/Prepare
	mkdir -p $(CMAKE_SOURCE_DIR)
	$(CP) ./files/* $(CMAKE_SOURCE_DIR)/
endef

define Build/Clean
	rm -rf $(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
endef

CMAKE_OPTIONS += \
		-DCMAKE_BUILD_TYPE=DEBUG \
		-DGSTREAMER_MEDIA_PLAYER=ON \
		-DTCONFIGS=ON \

# Amazonlite detector
CMAKE_OPTIONS += \
		-DAMAZONLITE_KEY_WORD_DETECTOR=ON \
		-DAMAZONLITE_KEY_WORD_DETECTOR_LIB_PATH=pryon_lite \
		-DAMAZONLITE_KEY_WORD_DETECTOR_INCLUDE_DIR=$(STAGING_DIR)/usr/include \
		-DAMAZONLITE_KEY_WORD_DETECTOR_DYNAMIC_MODEL_LOADING=ON

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib
	$(INSTALL_DIR) $(1)/etc/avs
	$(CP) $(CMAKE_SOURCE_DIR)/configs/$(TARGET_BOARD)/* $(1)/etc/avs
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
