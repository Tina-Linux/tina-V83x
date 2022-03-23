include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=af-alg
PKG_VERSION:=2012-02-29
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_MD5SUM:=92ffb33315e744134ea2358b638ba2e7

include $(BUILD_DIR)/package.mk

define Package/af-alg
  SECTION:=libs
  CATEGORY:=Libraries
  DEPENDS:= +libopenssl
  TITLE:= AF_ALG for OpenSSL
endef

define Package/af-alg/description
  AF_ALG for OpenSSL
endef


define Build/Configure
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LIBS="-lcrypto" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/af-alg/install
	$(INSTALL_DIR) $(1)/usr/lib/engines
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/engines/libaf_alg.so \
		$(1)/usr/lib/engines/libaf_alg.so
endef

$(eval $(call BuildPackage,af-alg))
