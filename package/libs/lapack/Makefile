include $(TOPDIR)/rules.mk

PKG_NAME:=lapack
PKG_VERSION:=3.8.0
PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_LICENSE:=BSD

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:= lapack
  DEPENDS:=+libgfortran
  URL:=http://www.netlib.org/lapack/
endef

define Package/$(PKG_NAME)/description
LAPACK is a software package provided by Univ. of Tennessee; Univ. of California, Berkeley; Univ. of Colorado Denver; and NAG Ltd..
endef

CMAKE_OPTIONS += -DBUILD_SHARED_LIBS=ON
CMAKE_OPTIONS += -DCMAKE_Fortran_COMPILER=$(TARGET_CC)

TARGET_LDFLAGS+= -lgfortran

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/lib*.so* $(1)/usr/lib/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
