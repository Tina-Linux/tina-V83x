include $(TOPDIR)/rules.mk

PKG_NAME:=libpryon-lite

ifeq ($(TARGET_CPU_VARIANT), cortex-a7)
  PKG_VERSION:=2.0.0
else ifeq ($(TARGET_CPU_VARIANT), cortex-a53)
  PKG_VERSION:=1.0
endif
PKG_RELEASE:=1

PKG_LICENSE:=Proprietary
PKG_LICENSE_FILES:=LICENSE.txt

include $(BUILD_DIR)/package.mk
include $(BUILD_DIR)/cmake.mk

define Package/$(PKG_NAME)
  CATEGORY:=Thirdparty
  TITLE:=Amazon wakeword engine
endef

# The variable $(TARGET_CPU_VARIANT) would be "cortex-a7", "arm926ej-s" or "cortex-a53"
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./files/$(TARGET_CPU_VARIANT)/* $(PKG_BUILD_DIR)/
endef

#CMAKE_OPTIONS += -DTARGET_CPU_VARIANT=$(TARGET_CPU_VARIANT)

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib/ $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.a $(1)/usr/lib/
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/etc/avs/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/models/*.bin $(1)/etc/avs/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
