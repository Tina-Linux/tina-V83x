include $(TOPDIR)/rules.mk

PKG_NAME:=avs-device-sdk
PKG_VERSION:=1.11
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://github.com/alexa/avs-device-sdk/archive/v$(PKG_VERSION)
PKG_HASH:=f1be94811fd5b41455d3be4a66ec906dbacb6d17774dac3ef70e9728276c1d5a

PKG_BUILD_PARALLEL:=1
PKG_JOBS:=-j16

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)/build_dir
CMAKE_SOURCE_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)/$(PKG_NAME)-$(PKG_VERSION)

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  SUBMENU:=AVS
  DEPENDS:= +libstdcpp +libcurl +@LIBCURL_NGHTTP2 +libsqlite3 \
			+gstreamer1 +gst1-libav +gstreamer1-plugins-base \
			+gstreamer1-plugins-good +gstreamer1-plugins-bad +libgst1app \
			+glib-networking \
			+ca-certificates \
			+libpryon-lite
  TITLE:=Alexa Voice Service Device SDK
endef

define Build/Patch
	$(if $(QUILT),rm -rf $(CMAKE_SOURCE_DIR)/patches; mkdir -p $(CMAKE_SOURCE_DIR)/patches)
	$(call PatchDir,$(CMAKE_SOURCE_DIR),$(PATCH_DIR),)
	$(if $(QUILT),touch $(CMAKE_SOURCE_DIR)/.quilt_used)
endef

define Build/Prepare
	$(Build/Prepare/Default)
	if [ -d "files/avs-sdk-adapter-for-wakeword-lite" ]; then \
		python ./files/avs-sdk-adapter-for-wakeword-lite/apply_adapter.py $(CMAKE_SOURCE_DIR); \
	fi
endef

define Build/Clean
	rm -rf $(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
endef

CMAKE_OPTIONS += \
		-DCMAKE_BUILD_TYPE=DEBUG \
		-DBUILD_SHARED_LIBS=ON \
		-DCOVERAGE=ON \
		-DBUILD_TESTING=ON \
		-DBUILD_GTEST=ON \

#Player
CMAKE_OPTIONS += \
		-DGSTREAMER_MEDIA_PLAYER=ON

#Amazonlite detector
CMAKE_OPTIONS += \
		-DAMAZONLITE_KEY_WORD_DETECTOR=ON \
		-DAMAZONLITE_KEY_WORD_DETECTOR_LIB_PATH=pryon_lite \
		-DAMAZONLITE_KEY_WORD_DETECTOR_INCLUDE_DIR=$(STAGING_DIR)/usr/include \
		-DAMAZONLITE_KEY_WORD_DETECTOR_DYNAMIC_MODEL_LOADING=ON

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
