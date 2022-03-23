include $(TOPDIR)/rules.mk

PKG_NAME:=live
PKG_VERSION:=2019.02.27

PKG_SOURCE:=$(PKG_NAME).$(PKG_VERSION).tar.gz
PKG_SOURCE_SUBDIR:=$(PKG_NAME)
PKG_SOURCE_VERSION:=v$(PKG_VERSION)
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)

PKG_LICENSE:=LGPL
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=multimedia
  CATEGORY:=Multimedia
  DEPENDS:=+libstdcpp
  TITLE:=LIVE555 Streaming Media
  URL:=http://www.live555.com/liveMedia/
endef

define Package/$(PKG_NAME)/description
  This code forms a set of C++ libraries for multimedia streaming,
using open standard protocols (RTP/RTCP, RTSP, SIP).
endef

# static libraries : ./genMakefiles tinalinux-static-libraries
# shared libraries : ./genMakefiles tinalinux-shared-libraries
define Build/Configure
	cd $(PKG_BUILD_DIR) && \
	./genMakefiles tinalinux-shared-libraries
	$(call Build/Configure/Default, )
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/local/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_INSTALL_DIR)/usr/local/include/* $(1)/usr/include

	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/lib/* $(1)/usr/lib

	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/local/bin/* $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
