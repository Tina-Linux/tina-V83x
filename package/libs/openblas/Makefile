include $(TOPDIR)/rules.mk

PKG_NAME:=OpenBLAS
PKG_VERSION:=0.2.19
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_LICENSE:=BSD

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:= an optimized BLAS library
endef

define Package/$(PKG_NAME)/description
	OpenBLAS is an optimized BLAS library based on GotoBLAS2 1.13 BSD version.
	Please read the documents on OpenBLAS wiki pages http://github.com/xianyi/OpenBLAS/wiki.
endef

define Build/Compile
	+$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		CC=$(TARGET_CC) \
		FC=$(TARGET_FORTRAN) \
		HOSTCC=$(HOSTCC) \
		PREFIX=$(PKG_INSTALL_DIR) \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		TARGET='ARMV7' \
		all install
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/openblas $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/include/* $(1)/usr/include/openblas/
	$(CP) $(PKG_INSTALL_DIR)/lib/libopenblas*.{so*,a} $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/libopenblas*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
