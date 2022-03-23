Package/ap6212_plus_ap6212a-firmware = $(call Package/firmware-default,Broadcom compatibility firmware)
define Package/ap6212_plus_ap6212a-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6212/*.bin \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6212/*.hcd \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6212/nvram_ap6212.txt \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6212a/*.bin \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6212a/*.hcd \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/ap6212a/nvram_ap6212a.txt \
		$(1)/$(FIRMWARE_PATH)/
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/config.txt \
		$(1)/$(FIRMWARE_PATH)/
endef
$(eval $(call BuildPackage,ap6212_plus_ap6212a-firmware))
