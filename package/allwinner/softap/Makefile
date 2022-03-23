include $(TOPDIR)/rules.mk

PKG_NAME:=softap
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/softap/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  KCONFIG:=\
    CONFIG_IP_NF_IPTABLES=y \
    CONFIG_IP_NF_MANGLE=y \
    CONFIG_IP_NF_TARGET_MASQUERADE=y \
    CONFIG_IP_NF_TARGET_NETMAP=y \
    CONFIG_IP_NF_TARGET_REDIRECT=y \
    CONFIG_NETFILTER=y \
    CONFIG_NETFILTER_ADVANCED=y \
    CONFIG_NETFILTER_NETLINK=y \
    CONFIG_NETFILTER_XTABLES=y \
    CONFIG_NETFILTER_XT_CONNMARK=y \
    CONFIG_NETFILTER_XT_MATCH_CONNMARK=y \
    CONFIG_NETFILTER_XT_MATCH_CONNTRACK=y \
    CONFIG_NETFILTER_XT_MATCH_STATE=y \
    CONFIG_NETFILTER_XT_TARGET_NETMAP=y \
    CONFIG_NETFILTER_XT_TARGET_REDIRECT=y \
    CONFIG_NF_REJECT_IPV4=y \
    CONFIG_NF_CONNTRACK=y \
    CONFIG_NF_CONNTRACK_EVENTS=y \
    CONFIG_NF_CONNTRACK_IPV4=y \
    CONFIG_NF_CONNTRACK_MARK=y \
    CONFIG_NF_CONNTRACK_SECMARK=y \
    CONFIG_NF_CONNTRACK_TIMEOUT=y \
    CONFIG_NF_CONNTRACK_TIMESTAMP=y \
    CONFIG_NF_CT_NETLINK=y \
    CONFIG_NF_CT_NETLINK_TIMEOUT=y \
    CONFIG_NF_DEFRAG_IPV4=y \
    CONFIG_NF_NAT=y \
    CONFIG_NF_NAT_IPV4=y \
    CONFIG_NF_NAT_NEEDED=y \
    CONFIG_NETFILTER_XT_TARGET_CONNSECMARK=n \
    CONFIG_NETFILTER_XT_TARGET_SECMARK=n \
    CONFIG_NETFILTER_XT_MATCH_QUOTA2=n \
    CONFIG_IP_NF_TARGET_REJECT_SKERR=n
  TITLE:=Tina softap manager
  DEPENDS:=+libpthread +libstdcpp +liballwinner-base +libopenssl +iw +hostapd +dnsmasq +iptables
endef


define Package/softap
  $(call Package/softap/Default)
  MENU:=1
  DEFAULT:=1
endef

define Package/softap-demo
  $(call Package/softap/Default)
  TITLE:=Tina softap app demo
  DEPENDS := softap +softap
endef

define Package/softap/description
	Tina softap manager
endef

define Package/softap-demo/description
	Tina softap app demo
endef

define Package/softap-demo/config
choice
    prompt "wifi module"
	depends on PACKAGE_softap-demo
    default BCMDHD
    help
      Select wifi modules.
    config BCMDHD
        bool "ampark"
    config XRADIO
        bool "xradio"
    config REALTEK
        bool "Realtek"
    config ESP8089
        bool "esp8089"
endchoice
endef

ifeq ($(CONFIG_BCMDHD),y)
TARGET_CFLAGS += -DBCMDHD
endif
ifeq ($(CONFIG_XRADIO),y)
TARGET_CFLAGS += -DXRADIO
endif
ifeq ($(CONFIG_REALTEK),y)
TARGET_CFLAGS += -DREALTEK
endif
ifeq ($(CONFIG_ESP8089),y)
TARGET_CFLAGS += -DESP8089
endif

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src $(PKG_BUILD_DIR)/
	$(CP) -r ./demo $(PKG_BUILD_DIR)/
endef

define Build/Configure
endef
define Build/Compile
	#libsoftap.so
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all

	#softap_test
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libsoftap.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
endef

define Package/softap/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/libsoftap.so $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DATA) ./src/hostapd.conf $(1)/etc/wifi
endef

define Package/softap-demo/install
	$(INSTALL_DIR) $(1)/bin
	$(INSTALL_DIR) $(1)/etc/wifi
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/softap_test $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/softap_up $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/softap_down $(1)/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/softap_longtime_test $(1)/bin
endef

$(eval $(call BuildPackage,softap))
$(eval $(call BuildPackage,softap-demo))
