Package/xr829-firmware = $(call Package/firmware-default,Xradio xr829 firmware)
define Package/xr829-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/xr829/*.bin $(1)/$(FIRMWARE_PATH)/
endef
$(eval $(call BuildPackage,xr829-firmware))
