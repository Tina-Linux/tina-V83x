include $(TOPDIR)/rules.mk

PKG_NAME:=libatomic_ops
PKG_VERSION:=7.4.8
PKG_RELEASE:=1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/ivmai/libatomic_ops
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=3978e9d7010d724bbd88b51e27cb5ee5315a694a
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.xz

PKG_LICENSE_FILES:=doc/LICENSING.txt

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_VERSION)
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=The atomic_ops library
endef

define Package/$(PKG_NAME)/description
  This package provides semi-portable access to hardware-provided atomic memory
  update operations on a number architectures.
endef

define Build/Prepare
	$(Build/Prepare/Default)
	(cd $(PKG_BUILD_DIR) && ./autogen.sh)
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	:
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
