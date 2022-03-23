Package/ap6236-firmware = $(call Package/firmware-default,Broadcom AP6236 firmware)
define Package/ap6236-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6236/*.bin \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6236/*.hcd \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6236/nvram_ap6236.txt \
		$(1)/$(FIRMWARE_PATH)/nvram.txt
endef
$(eval $(call BuildPackage,ap6236-firmware))
