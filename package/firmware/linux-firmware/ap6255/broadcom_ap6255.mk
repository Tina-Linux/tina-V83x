Package/ap6255-firmware = $(call Package/firmware-default,Broadcom AP6255 firmware)
define Package/ap6255-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6255/*.bin \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6255/*.hcd \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6255/nvram_ap6255.txt \
		$(1)/$(FIRMWARE_PATH)/nvram.txt
endef
$(eval $(call BuildPackage,ap6255-firmware))
