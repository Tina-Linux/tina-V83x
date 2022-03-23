include $(TOPDIR)/rules.mk

PKG_NAME:=alsa-plugins-sona
PKG_VERSION:=20190909
PKG_RELEASE:=1

PKG_FIXUP:=autoreconf
PKG_INSTALL:=1
PKG_USE_MIPS16:=0
PKG_CHECK_FORMAT_SECURITY:=0

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  TITLE:=ALSA plugins from Sona
  DEPENDS:=+alsa-lib
endef

CONFIGURE_ARGS += --enable-audioaef

# Remove the double quotation marks ("") in $(CONFIG_TOOLCHAIN_LIBC).
# Use the $(shell ...) function instead of the $(subst ...) function only
# because the quotation mark in $(subst ...) may affect the syntax highlight
# in some editors. Either of them can achieve the same effect.
TOOLCHAIN_LIBC_STRING:=$(shell echo -n $(CONFIG_TOOLCHAIN_LIBC))
#TOOLCHAIN_LIBC_STRING:=$(subst ",,$(CONFIG_TOOLCHAIN_LIBC))

TARGET_LDFLAGS += -L$(PKG_BUILD_DIR)/audioaef/lib/$(TOOLCHAIN_LIBC_STRING)

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/audioaef/lib/$(TOOLCHAIN_LIBC_STRING)/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/alsa-lib
	$(INSTALL_BIN) \
		$(PKG_INSTALL_DIR)/usr/lib/alsa-lib/libasound_module*.so \
		$(1)/usr/lib/alsa-lib
	$(INSTALL_BIN) \
		$(PKG_BUILD_DIR)/audioaef/lib/$(TOOLCHAIN_LIBC_STRING)/*.so \
		$(1)/usr/lib
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
