Package/ap6335-firmware = $(call Package/firmware-default,Broadcom AP6335 firmware)
define Package/ap6335-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6335/*.bin \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6335/*.hcd \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6335/nvram_ap6335.txt \
		$(1)/$(FIRMWARE_PATH)/nvram.txt
endef
$(eval $(call BuildPackage,ap6335-firmware))
