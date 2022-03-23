include $(TOPDIR)/rules.mk

PKG_NAME:=procrank_linux
PKG_VERSION:=2018-08-03

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://github.com/csimmonds/procrank_linux.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=5aa6840b617ab2052c365c26b32ebc5fa96ad657
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_SOURCE_SUBDIR)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Utilities
  TITLE:=A tool to display memory usage ported from Android
  DEPENDS:=@KERNEL_PROC_PAGE_MONITOR
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		CC="$(TARGET_CC)" \
		AR="$(TARGET_AR)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		DESTDIR="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(CP) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
