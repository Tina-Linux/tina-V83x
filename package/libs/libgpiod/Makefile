include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk

PKG_NAME:=libgpiod
PKG_VERSION:=0.0.1

PKG_SOURCE_PROTO:=git
PKG_SOURCE_URL:=https://git.kernel.org/pub/scm/libs/libgpiod/libgpiod.git
PKG_SOURCE_SUBDIR:=$(PKG_NAME)-$(PKG_VERSION)
PKG_SOURCE_VERSION:=8ff90454fc111a914802f7c4f2c4e1355ee0c16c
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

PKG_MAINTAINER:=XEL <xel1995@163.com>

PKG_RELEASE=$(PKG_SOURCE_VERSION)

PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_SOURCE_SUBDIR)

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=Gpio library and tools
endef

define Package/$(PKG_NAME)/description
	C library and tools for interacting with the linux GPIO character device
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LINUX_USER_HEADERS="$(LINUX_DIR)/user_headers" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/src/lib/libgpiod.so $(1)/usr/lib
	$(CP) $(PKG_BUILD_DIR)/include/gpiod.h $(1)/usr/include
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/lib/libgpiod.so $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tools/gpiodetect $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tools/gpiofind $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tools/gpioinfo $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tools/gpiomon $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tools/gpioset $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/tools/gpioget $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
