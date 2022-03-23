Package/ap6212z-firmware = $(call Package/firmware-default,Broadcom AP6212z firmware)
define Package/ap6212z-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6212z/*.bin \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6212z/*.hcd \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/ap6212z/nvram_ap6212z.txt \
		$(1)/$(FIRMWARE_PATH)/nvram.txt
endef
$(eval $(call BuildPackage,ap6212z-firmware))
