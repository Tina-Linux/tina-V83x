include $(TOPDIR)/rules.mk

PKG_NAME:=tutuclear-lib

# The variable $(TARGET_CPU_VARIANT) would be "cortex-a7", "arm926ej-s" or "cortex-a53"
ifeq ($(TARGET_CPU_VARIANT), cortex-a7)
  PKG_VERSION:=1.7.1_20191127
  # The following version is authorized over the network, without expiration.
  # It will create the license file in /data directory (/data/Gemalto/aut.lic),
  # therefore we should make sure that /data can be read and written. Another
  # limitation is that no more than 50 devices can be authorized at the same time.
  #PKG_VERSION:=1.7.1_20191203_network_auth
else ifeq ($(TARGET_CPU_VARIANT), cortex-a53)
  PKG_VERSION:=16b_R18_armv8a_A
endif
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  TITLE:=GMEMS TutuClear Library
endef

define Package/$(PKG_NAME)/description
  GMEMS noise reduction, AEC and wake-up library
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./files/* $(PKG_BUILD_DIR)
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		TARGET_CPU_VARIANT="$(TARGET_CPU_VARIANT)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(CP) $(PKG_BUILD_DIR)/include/$(TARGET_CPU_VARIANT)/* $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) \
		$(PKG_BUILD_DIR)/lib/$(TARGET_CPU_VARIANT)/libtutuClear_$(PKG_VERSION)*.a \
		$(1)/usr/lib/libtutuClear.a
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtutu_tool.so $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/libtutu_tool.so $(1)/usr/lib
	# For network authorization library
	#$(INSTALL_DIR) $(1)/data
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
