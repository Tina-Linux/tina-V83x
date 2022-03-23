#
# Copyright (C) 2015 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=opencv
PKG_VERSION:=4.1.0

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)

PKG_EXTRA_NAME:=opencv_contrib
PKG_EXTRA_SOURCE:=$(PKG_EXTRA_NAME)-$(PKG_VERSION).zip
PKG_EXTRA_SOURCE_SUBDIR:=$(PKG_EXTRA_NAME)-$(PKG_VERSION)

ADE_FRAMEWORK:=ade
ADE_FRAMEWORK_VERSION:=0.1.1d
ADE_FRAMEWORK_SOURCE:=$(ADE_FRAMEWORK)-$(ADE_FRAMEWORK_VERSION).zip
ADE_FRAMEWORK_SOURCE_SUBDIR:=$(ADE_FRAMEWORK)-$(ADE_FRAMEWORK_VERSION)

PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_LICENSE:=BSD

PKG_INSTALL:=1

CONFIGURE_ARGS += \
        --enable-shared \
        --enable-static \

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)/config
	if PACKAGE_$(PKG_NAME)
		config ENABLE_SUNXIVIN_ISP
			bool "Enabel sunxi vin isp support"
			default n
	endif
endef

CMAKE_SOURCE_DIR:=$(PKG_BUILD_DIR)/opencv_build/
CMAKE_OPTIONS += -DCMAKE_INSTALL_PREFIX=$(PKG_INSTALL_DIR)/usr/
CMAKE_OPTIONS += -DWITH_GTK=OFF
CMAKE_OPTIONS += -DWITH_TIFF=OFF
CMAKE_OPTIONS += -DWITH_QT=OFF
CMAKE_OPTIONS += -DWITH_GSTREAMER=OFF
CMAKE_OPTIONS += -DCMAKE_BUILD_TYPE=Release
CMAKE_OPTIONS += -DCMAKE_CONFIGURATION_TYPES=Release
CMAKE_OPTIONS += -DBUILD_PERF_TESTS=OFF
CMAKE_OPTIONS += -DBUILD_TESTS=OFF
CMAKE_OPTIONS += -DCV_TRACE=OFF
CMAKE_OPTIONS += -DQt5Core_DIR=$(STAGING_DIR)/usr/lib/cmake/Qt5Core
CMAKE_OPTIONS += -DQt5Gui_DIR=$(STAGING_DIR)/usr/lib/cmake/Qt5Gui
CMAKE_OPTIONS += -DQt5Test_DIR=$(STAGING_DIR)/usr/lib/cmake/Qt5Test
CMAKE_OPTIONS += -DQt5Widgets_DIR=$(STAGING_DIR)/usr/lib/cmake/Qt5Widgets
CMAKE_OPTIONS += -DOPENCV_EXTRA_MODULES_PATH=$(PKG_BUILD_DIR)/$(PKG_EXTRA_SOURCE_SUBDIR)/modules

ifeq ($(CONFIG_ENABLE_SUNXIVIN_ISP),y)
CMAKE_OPTIONS += -DENABLE_SUNXIVINISP=ON
else
CMAKE_OPTIONS += -DENABLE_SUNXIVINISP=OFF
endif

#opencv modules
CMAKE_OPTIONS += -DBUILD_opencv_apps=OFF
CMAKE_OPTIONS += -DBUILD_opencv_calib3d=ON
CMAKE_OPTIONS += -DBUILD_opencv_features2d=ON
CMAKE_OPTIONS += -DBUILD_opencv_flann=ON
CMAKE_OPTIONS += -DBUILD_opencv_highgui=ON
CMAKE_OPTIONS += -DBUILD_opencv_imgcodecs=ON
CMAKE_OPTIONS += -DBUILD_opencv_imgproc=ON
CMAKE_OPTIONS += -DBUILD_opencv_ml=ON
CMAKE_OPTIONS += -DBUILD_opencv_objdetect=ON
CMAKE_OPTIONS += -DBUILD_opencv_photo=ON
CMAKE_OPTIONS += -DBUILD_opencv_shape=ON
CMAKE_OPTIONS += -DBUILD_opencv_stitching=ON
CMAKE_OPTIONS += -DBUILD_opencv_superres=ON
CMAKE_OPTIONS += -DBUILD_opencv_video=ON
CMAKE_OPTIONS += -DBUILD_opencv_videoio=ON
CMAKE_OPTIONS += -DBUILD_opencv_videostab=ON
CMAKE_OPTIONS += -DBUILD_opencv_viz=ON
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/harfbuzz/

define Package/$(PKG_NAME)
  MENU:=1
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp +libpng +libjpeg +zlib +libfreetype +harfbuzz +libwebp +ENABLE_SUNXIVIN_ISP:libAWIspApi
  TITLE:=opencv libs
  URL:=https://opencv.org/releases.html
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/opencv_build/
	mkdir -p $(PKG_BUILD_DIR)/opencv_build/patches
	unzip -d $(PKG_BUILD_DIR)/opencv_build/ $(DL_DIR)/$(PKG_SOURCE)
	unzip -d $(PKG_BUILD_DIR)/ $(DL_DIR)/$(PKG_EXTRA_SOURCE)
	mv $(PKG_BUILD_DIR)/opencv_build/$(PKG_SOURCE_SUBDIR)/* $(PKG_BUILD_DIR)/opencv_build/

	mkdir -p $(PKG_BUILD_DIR)/3rdparty/ade
	unzip -d $(PKG_BUILD_DIR)/3rdparty/ade $(DL_DIR)/$(ADE_FRAMEWORK_SOURCE)

	cp ./patches/* $(PKG_BUILD_DIR)/opencv_build/patches
	patch -p1 -d $(PKG_BUILD_DIR)/opencv_build/ < $(PKG_BUILD_DIR)/opencv_build/patches/0001-disable-download-ade.patch
	patch -p1 -d $(PKG_BUILD_DIR) < $(PKG_BUILD_DIR)/opencv_build/patches/0002-disable-download-xfeatures2d-i.patch
	patch -p1 -d $(PKG_BUILD_DIR)/$(PKG_EXTRA_SOURCE_SUBDIR) < $(PKG_BUILD_DIR)/opencv_build/patches/0002-disable-download-xfeatures2d-cmake.patch
	patch -p1 -d $(PKG_BUILD_DIR)/$(PKG_EXTRA_SOURCE_SUBDIR) < $(PKG_BUILD_DIR)/opencv_build/patches/0003-disable-download-face_landmark_model_dat.patch
	patch -p1 -d $(PKG_BUILD_DIR)/opencv_build/ < $(PKG_BUILD_DIR)/opencv_build/patches/0004-support-sunxi-vin-camera.patch

	## add xfeatures2d related header file
	mkdir -p $(PKG_BUILD_DIR)/opencv2/
	cp $(PKG_BUILD_DIR)/$(PKG_EXTRA_SOURCE_SUBDIR)/modules/xfeatures2d/include/opencv2/* $(PKG_BUILD_DIR)/opencv2/ -rf
endef

define Build/Compile
	$(MAKE) -j4 -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		FC=$(TARGET_FORTRAN) \
		HOSTCC=$(HOSTCC) \
		PREFIX=$(PKG_INSTALL_DIR) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		all
endef

define Build/Install
	CFLAGS="$(TARGET_CFLAGS)" \
	CXXFLAGS="$(TARGET_CXXFLAGS)" \
	LDFLAGS="$(TARGET_LDFLAGS)" \
	LIBS="$(TARGET_LIBS)" \
	$(MAKE) -j64 -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		PREFIX=$(PKG_INSTALL_DIR) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		ARCH="$(ARCH)" \
		install
endef

define Package/$(PKG_NAME)/description
OpenCV (Open Source Computer Vision Library) is an open source computer vision and machine learning software library. OpenCV was built to provide a common infrastructure for computer vision applications and to accelerate the use of machine perception in the commercial products. Being a BSD-licensed product, OpenCV makes it easy for businesses to utilize and modify the code.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/share
	$(CP) $(PKG_INSTALL_DIR)/usr/share/* $(1)/usr/share

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
