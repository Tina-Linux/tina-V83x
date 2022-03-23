Package/esp8089-firmware = $(call Package/firmware-default,esp8089 firmware)
define Package/esp8089-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/esp8089/esp_init_data.bin \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/esp8089/init_data.conf \
		$(1)/$(FIRMWARE_PATH)/
endef
$(eval $(call BuildPackage,esp8089-firmware))
