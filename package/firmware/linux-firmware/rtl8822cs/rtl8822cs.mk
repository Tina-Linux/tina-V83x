Package/r8822cs-firmware = $(call Package/firmware-default,RealTek RTL8822CS firmware)
define Package/r8822cs-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)/rtlbt
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/rtl8822cs/rtl8822cs_config \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8822cs_config
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/rtl8822cs/rtl8822cs_fw \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8822cs_fw
endef
$(eval $(call BuildPackage,r8822cs-firmware))
