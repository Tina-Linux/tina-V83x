Package/qca9377-firmware-cfgfile = $(call Package/firmware-default,Qualcom qca9377 firmware & cfgfile)
define Package/qca9377-firmware-cfgfile/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)/wlan

	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/qca9377/firmware/* $(1)/$(FIRMWARE_PATH)/

	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/qca9377/cfg/qcom_cfg.ini $(1)/$(FIRMWARE_PATH)/wlan/qcom_cfg.ini
	$(INSTALL_DATA) $(TOPDIR)/package/firmware/linux-firmware/qca9377/cfg/WCNSS_qcom_cfg.ini $(1)/$(FIRMWARE_PATH)/wlan/

endef
$(eval $(call BuildPackage,qca9377-firmware-cfgfile))
