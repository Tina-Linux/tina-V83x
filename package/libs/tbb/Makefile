include $(TOPDIR)/rules.mk

PKG_NAME:=tbb
PKG_VERSION:=2019

PKG_SOURCE:=$(PKG_NAME)-$(PKG_NAME)_$(PKG_VERSION).zip
PKG_SOURCE_VERSION:=v$(PKG_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_NAME)_$(PKG_VERSION)
PKG_LICENSE:=Apache-2.0

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:=+libstdcpp
  TITLE:=Official Threading Building Blocks(Only glibc is supported)
  URL:=https://github.com/intel/tbb
endef

define Package/$(PKG_NAME)/description
Threading Building Blocks (TBB) lets you easily write parallel C++ programs that take full advantage of multicore performance, that are portable, composable and have future-proof scalability.
endef

define Build/Compile
	$(MAKE) -j4 -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		CXX=$(TARGET_CXX) \
		FC=$(TARGET_FORTRAN) \
		HOSTCC=$(HOSTCC) \
		PREFIX=$(PKG_INSTALL_DIR) \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		arch="armv7" \
		runtime="tina_glibc" \
		target="tina" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
