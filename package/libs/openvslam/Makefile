include $(TOPDIR)/rules.mk

PKG_NAME:=openvslam
PKG_VERSION:=master

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_LICENSE:=BSD

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS += -DBUILD_WITH_MARCH_NATIVE=OFF \
				-DUSE_PANGOLIN_VIEWER=OFF \
				-DUSE_SOCKET_PUBLISHER=OFF \
				-DUSE_STACK_TRACE_LOGGER=ON \
				-DBOW_FRAMEWORK=DBoW2 \
				-DUSE_G2O_DEBUG=OFF \
				-DBUILD_EXAMPLE=OFF \
				-DBUILD_TESTS=OFF

TARGET_CFLAGS += -pthread

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+glog +yaml-cpp +SuiteSparse +opencv +eigen +DBoW2 +g2o
  TITLE:=A Versatile Visual SLAM Framework
  URL:=https://github.com/xdspacelab/openvslam.git
endef

define Package/$(PKG_NAME)/description
OpenVSLAM is based on an indirect SLAM algorithm with sparse features, such as ORB-SLAM, ProSLAM, and UcoSLAM.
endef

define Build/Install
	@echo "no install"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/include/openvslam
	$(INSTALL_DIR) $(1)/usr/include/openvslam/camera
	$(INSTALL_DIR) $(1)/usr/include/openvslam/data
	$(INSTALL_DIR) $(1)/usr/include/openvslam/feature
	$(INSTALL_DIR) $(1)/usr/include/openvslam/initialize
	$(INSTALL_DIR) $(1)/usr/include/openvslam/io
	$(INSTALL_DIR) $(1)/usr/include/openvslam/match
	$(INSTALL_DIR) $(1)/usr/include/openvslam/module
	$(INSTALL_DIR) $(1)/usr/include/openvslam/optimize
	$(INSTALL_DIR) $(1)/usr/include/openvslam/publish
	$(INSTALL_DIR) $(1)/usr/include/openvslam/solve
	$(INSTALL_DIR) $(1)/usr/include/openvslam/feature
	$(INSTALL_DIR) $(1)/usr/include/openvslam/util

	$(CP) $(PKG_BUILD_DIR)/3rd/* $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/*.h $(1)/usr/include/openvslam
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/camera/*.h $(1)/usr/include/openvslam/camera
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/data/*.h $(1)/usr/include/openvslam/data
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/feature/*.h $(1)/usr/include/openvslam/feature
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/initialize/*.h $(1)/usr/include/openvslam/initialize
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/io/*.h $(1)/usr/include/openvslam/io
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/match/*.h $(1)/usr/include/openvslam/match
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/module/*.h $(1)/usr/include/openvslam/module
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/optimize/*.h $(1)/usr/include/openvslam/optimize
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/publish/*.h $(1)/usr/include/openvslam/publish
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/solve/*.h $(1)/usr/include/openvslam/solve
	$(CP) $(PKG_BUILD_DIR)/src/openvslam/util/*.h $(1)/usr/include/openvslam/util

	$(CP) $(PKG_BUILD_DIR)/3rd/json/include/* $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/3rd/popl/include/* $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/3rd/spdlog/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
