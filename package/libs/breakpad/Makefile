include $(TOPDIR)/rules.mk

PKG_NAME:=breakpad

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://chromium.googlesource.com/breakpad/breakpad.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE_VERSION:=cb94b71d28d0c12ba9edbbead8c152d29061e140
PKG_SOURCE:=$(PKG_NAME)-$(PKG_SOURCE_VERSION).tar.gz

PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_LICENSE:=BSD-3-Clause

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/breakpad-libs
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Libraries of breakpad
  DEPENDS:=+libstdcpp
endef

define Package/breakpad-libs/description
 A set of client and server components which implement a crash-reporting system
endef

define Package/breakpad-utils
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=Utilities provided by breakpad
  DEPENDS:=+libstdcpp +breakpad-libs
endef

define Package/breakpad-utils/description
 A set of client and server components which implement a crash-reporting system
endef

define Build/Configure
	$(call Build/Configure/Default, \
		--disable-tools \
		--host=arm-openwrt-linux \
		CXXFLAGS='-D__WORDSIZE=32' \
	)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/breakpad-libs/install
	:
endef

define Package/breakpad-utils/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,breakpad-libs))
$(eval $(call BuildPackage,breakpad-utils))
