Package/cyw43438-firmware = $(call Package/firmware-default,cypress 43438 firmware)
define Package/cyw43438-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/cyw43438/*.bin $(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/cyw43438/*.txt $(1)/$(FIRMWARE_PATH)/nvram.txt
endef
$(eval $(call BuildPackage,cyw43438-firmware))
