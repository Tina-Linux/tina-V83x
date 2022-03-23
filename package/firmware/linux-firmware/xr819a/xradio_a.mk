Package/xr819a-firmware = $(call Package/firmware-default,Xradio xr819a firmware)
define Package/xr819a-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/xr819a/*_xr819.bin $(1)/$(FIRMWARE_PATH)/
endef
$(eval $(call BuildPackage,xr819a-firmware))
