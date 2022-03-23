Package/ap6212-firmware = $(call Package/firmware-default,Broadcom AP6212 firmware)
define Package/ap6212-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6212/*.bin \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6212/*.hcd \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6212/nvram_ap6212.txt \
		$(1)/$(FIRMWARE_PATH)/nvram.txt
endef
$(eval $(call BuildPackage,ap6212-firmware))
