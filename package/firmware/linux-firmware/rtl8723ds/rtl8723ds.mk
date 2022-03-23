Package/r8723ds-firmware = $(call Package/firmware-default,RealTek RTL8723DS firmware)
define Package/r8723ds-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)/rtlbt
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/rtl8723ds/rtl8723dsh4_fw \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723dsh4_fw
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/rtl8723ds/rtl8723dsh4_config \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723dsh4_config
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/rtl8723ds/rtl8723d_fw \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723d_fw
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/rtl8723ds/rtl8723d_config \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8723d_config
endef
$(eval $(call BuildPackage,r8723ds-firmware))
