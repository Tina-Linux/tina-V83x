#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=avs-sdk
PKG_VERSION:=1.7.1


PKG_INSTALL:=1
PKG_JOBS := -j32

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_SOURCE_DIR := avs-device-sdk

CMAKE_OPTIONS +=-DBUILD_SHARED_LIBS=ON \
				-DBUILD_TESTING=ON \
				-DCMAKE_BUILD_TYPE=DEBUG \
				-DCOVERAGE=ON \
				-DBUILD_GTEST=ON \

#-DACSDK_EMIT_SENSITIVE_LOGS=ON

#ESP
#CMAKE_OPTIONS += \
				-DESP_PROVIDER=ON \
				-DESP_INCLUDE_DIR=$(STAGING_DIR)/usr/include/esp/ \
				-DESP_LIB_PATH=esp


#Player
CMAKE_OPTIONS += \
			-DGSTREAMER_MEDIA_PLAYER=ON \

#-DTOTEM_PLPARSER=ON

#SampleApp
CMAKE_OPTIONS += \
			-DPORTAUDIO=ON \
			-DPORTAUDIO_LIB_PATH=portaudio \
			-DPORTAUDIO_INCLUDE_DIR=$(STAGING_DIR)/usr/include

#KITT.AI detector
#CMAKE_OPTIONS += \
			-DKITTAI_KEY_WORD_DETECTOR=ON \
			-DKITTAI_KEY_WORD_DETECTOR_LIB_PATH=SnowboyDetect \
			-DKITTAI_KEY_WORD_DETECTOR_INCLUDE_DIR=$(STAGING_DIR)/usr/include

#Sensory detector
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
  DEPENDS:= +libstdcpp +libcurl +@LIBCURL_NGHTTP2 +libsqlite3 \
			+gstreamer1 +gst1-libav +gstreamer1-plugins-base \
			+gstreamer1-plugins-good +gstreamer1-plugins-ugly \
			+gstreamer1-plugins-bad +libgst1app \
			+libsensory \
			+glib-networking \
			+ca-certificates \
			+portaudio \
			+libpryon-lite \
			+libesp
  TITLE:=avs library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./files/avs-device-sdk $(PKG_BUILD_DIR)/avs-device-sdk
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr
	$(CP) $(PKG_INSTALL_DIR)/usr/include $(1)/usr/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib $(1)/usr/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) -p $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
endef

#$(eval $(call BuildPackage,$(PKG_NAME)))
