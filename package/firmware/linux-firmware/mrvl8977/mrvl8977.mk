Package/mrvl8977-firmware-cfgfile = $(call Package/firmware-default,Marvell 8977 firmware & cfgfile)
define Package/mrvl8977-firmware-cfgfile/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)/mrvl

	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/mrvl8977/*.bin $(1)/$(FIRMWARE_PATH)/mrvl/

	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/mrvl8977/WlanCalData_ext_QFN_RD_V1_P82.conf $(1)/$(FIRMWARE_PATH)/mrvl/

endef
$(eval $(call BuildPackage,mrvl8977-firmware-cfgfile))
