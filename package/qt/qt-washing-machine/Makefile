#
# Copyright (C) 2007-2016 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=qt-washing-machine
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)
HAVE_SOURCE := yes

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=Qt
  CATEGORY:=Qt
  DEPENDS:= +qt5-gui +qt5-core +qt5-qml +qt5-quick-2 +qt5-quick-window +qt5-drivers-touchscreen +qt5-quick-controls-2
  TITLE:=qt-washing-machine
  MAINTAINER:=anruliu<anruliu@allwinnertech.com>
endef

define Package/$(PKG_NAME)/description
  The qt washing-machine application for Demo
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/
	$(CP) -r ./src $(PKG_BUILD_DIR)/
endef

QT_NAME=qt-everywhere-opensource-src-5.10.1-prebuilt

define Build/Configure
	TARGET_CC="$(TARGET_CROSS)gcc" \
	TARGET_CXX="$(TARGET_CROSS)g++" \
	TARGET_AR="$(TARGET_CROSS)ar cqs" \
	TARGET_OBJCOPY="$(TARGET_CROSS)objcopy" \
	TARGET_RANLIB="$(TARGET_CROSS)ranlib" \
	TARGET_CFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
	TARGET_CXXFLAGS="$(TARGET_CFLAGS) $(EXTRA_CFLAGS)" \
	TARGET_LDFLAGS="$(TARGET_LDFLAGS) $(EXTRA_LDFLAGS)" \
	TARGET_INCDIRS="$(STAGING_DIR)/include -I$(STAGING_DIR)/usr/include" \
	TARGET_LIBDIRS="$(STAGING_DIR)/lib -L$(STAGING_DIR)/usr/lib" \
	$(COMPILE_DIR)/$(QT_NAME)/ipkg-install/usr/bin/qmake \
		-o $(PKG_BUILD_DIR)/src/Makefile \
		$(PKG_BUILD_DIR)/src/$(1).pro
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) \
		$(PKG_BUILD_DIR)/src/qt-washing-machine \
		$(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
