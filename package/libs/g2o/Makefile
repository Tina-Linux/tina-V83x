include $(TOPDIR)/rules.mk

PKG_NAME:=g2o
PKG_VERSION:=master

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_LICENSE:=BSD

PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

CMAKE_OPTIONS +=-DBUILD_UNITTESTS=OFF \
				-DBUILD_WITH_MARCH_NATIVE=OFF \
				-DG2O_USE_CHOLMOD=ON \
				-DG2O_USE_CSPARSE=ON \
				-DG2O_USE_OPENGL=OFF \
				-DG2O_USE_OPENMP=ON \
				-DG2O_USE_QGLVIEWER=OFF \
				-DG2O_BUILD_APPS=OFF \
				-DG2O_BUILD_EXAMPLES=OFF \
				-DBUILD_UNITTESTS=OFF

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp +libgomp +lapack +OpenBLAS +SuiteSparse
  TITLE:=A General Framework for Graph Optimization
  URL:=https://github.com/RainerKuemmerle/g2o.git
endef

define Package/$(PKG_NAME)/description
g2o is an open-source C++ framework for optimizing graph-based nonlinear error functions.
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
