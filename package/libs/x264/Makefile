#
#Copyright (C) 2006-2016 OpenWrt.org
#
#This is free software, licensed under the GNU General Public License v2.
#See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=x264
PKG_RELEASE:=1

PKG_SOURCE:=$(PKG_NAME).zip

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_REMOVE_FILES:=autogen.sh aclocal.m4

include $(BUILD_DIR)/package.mk

define Package/x264
 SECTION:=libs
 CATEGORY:=Libraries
 DEPENDS:=+libpthread
 TITLE:=x264
endef

define Package/x264/description
	X264
endef

TARGET_CFLAGS += $(FPIC) -I$(PKG_BUILD_DIR) -D_GNU_SOURCE

CONFIGURE_ARGS+= \
		--disable-cli \
		--disable-static \
		--enable-shared \
		--disable-opencl \
		--disable-asm \

MAKE_FLAGS += \
	LD="$(TARGET_CC) -o " \

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/lib/pkgconfig

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/* $(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libx264* $(1)/usr/lib/
endef

define Package/x264/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include

	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libx264*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/

endef

$(eval $(call BuildPackage,x264))
