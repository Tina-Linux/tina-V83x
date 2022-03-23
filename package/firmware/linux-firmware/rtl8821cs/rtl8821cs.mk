Package/rtl8821cs-firmware = $(call Package/firmware-default,RealTek RTL8821CS firmware)

define Package/rtl8821cs-firmware/install
	$(INSTALL_DIR) $(1)/$(FIRMWARE_PATH)/rtlbt
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/rtl8821cs/rtl8821c_fw \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8821c_fw
	$(INSTALL_DATA) \
		$(TOPDIR)/package/firmware/linux-firmware/rtl8821cs/rtl8821c_config \
		$(1)/$(FIRMWARE_PATH)/rtlbt/rtl8821c_config
endef

$(eval $(call BuildPackage,rtl8821cs-firmware))
