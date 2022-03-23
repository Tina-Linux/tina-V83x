#
# Copyright (C) 2007-2015 OpenWrt.org
#
# It is licensed under the GPLv3 (see COPYING).
#

include $(TOPDIR)/rules.mk

PKG_NAME:=glmark2
PKG_VERSION:=7215c0f337dae0b232535549c37fca441747a891
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz
PKG_SOURCE_URL:=https://launchpad.net/glmark2/
#PKG_MD5SUM:=e15f173594e41afde37a09b26acc54a7

PKG_LICENSE:=GPLv3+ SGIv1
PKG_LICENSE_FILES:=COPYING COPYING.SGI

#PKG_BUILD_DEPENDS:=python/host

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)/config
  source "$(SOURCE)/Config.in"
endef

define Package/glmark2
  SECTION:=gui
  CATEGORY:=Wayland
  TITLE:=glmark2 is an OpenGL 2.0 and ES 2.0 benchmark
  URL:=https://launchpad.net/glmark2/
  MAINTAINER:=
  DEPENDS:= +libstdcpp +libpng +libjpeg +libump +GLMARK2_WAYLAND:wayland
ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r16 r40 r18 r311 t7))
	DEPENDS+= +GLMARK2_FBDEV:mali400-um +GLMARK2_WAYLAND:mali400-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r30))
	DEPENDS+= +GLMARK2_FBDEV:mali-t760-um +GLMARK2_WAYLAND:mali-t760-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), r58))
	DEPENDS+= +GLMARK2_FBDEV:sgx544-um +GLMARK2_WAYLAND:sgx544-um
else ifeq ($(TARGET_BOARD_PLATFORM), $(filter $(TARGET_BOARD_PLATFORM), mr813))
        DEPENDS+= +GLMARK2_FBDEV:ge8300-um +GLMARK2_WAYLAND:ge8300-um
endif
endef

define Package/glmark2/description
  glmark2 is developed by Alexandros Frantzis and Jesse Barker based on the
original glmark benchmark by Ben Smith.
endef

GLMARK2_FLAVORS_FBDEV += fbdev-glesv2
GLMARK2_FLAVORS_WAYLAND += wayland-glesv2 drm-glesv2

ifeq ($(CONFIG_GLMARK2_FBDEV), y)
CONFIGURE_ARGS = \
	--with-flavors=$(subst $(space),$(comma),$(GLMARK2_FLAVORS_FBDEV)) \
	--for-fbdev \
	--prefix=$(CONFIGURE_PREFIX)
endif
ifeq ($(CONFIG_GLMARK2_WAYLAND), y)
CONFIGURE_ARGS = \
	--with-flavors=$(subst $(space),$(comma),$(GLMARK2_FLAVORS_WAYLAND)) \
	--prefix=$(CONFIGURE_PREFIX)
endif

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM), r58 mr813))
CONFIGURE_ARGS += \
       --img_gpu
endif

define Build/Configure
	(cd $(PKG_BUILD_DIR); $(MAKE_FLAGS) ./waf configure $(CONFIGURE_ARGS) XXFLAGS=-std=c++11)
endef

define Build/Compile
	(cd $(PKG_BUILD_DIR); ./waf)
endef

define Build/Install
	(cd $(PKG_BUILD_DIR); ./waf install --destdir=$(PKG_INSTALL_DIR))
endef

define Package/glmark2/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/share/glmark2
	$(CP) $(PKG_INSTALL_DIR)/usr/share/glmark2/* $(1)/usr/share/glmark2
endef

$(eval $(call BuildPackage,glmark2))
