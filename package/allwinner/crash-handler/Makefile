include $(TOPDIR)/rules.mk

PKG_NAME:=crash-handler
PKG_VERSION:=1
PKG_RELEASE:=1


PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  SECTION:=utils
  CATEGORY:=Allwinner
  DEPENDS:= +libunwind +libunwind-arm +libunwind-ptrace +elfutils +libdw +libelf +libstdcpp +liballwinner-base
  TITLE:=Tina crash hanndler
endef

CMAKE_OPTIONS += -DCMAKE_SYSTEM_PROCESSOR=armv7l

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/crash-handler/* $(PKG_BUILD_DIR)/
endef


define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/libexec/* $(1)/usr/bin
endef

define Package/crash-handler/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/libexec/* $(1)/bin
endef


$(eval $(call BuildPackage,$(PKG_NAME)))
