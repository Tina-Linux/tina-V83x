include $(TOPDIR)/rules.mk

PKG_NAME:=alljoyn
PKG_BASE_VERSION:=16.04
PKG_PATCHLEVEL:=
PKG_RELEASE:=1
#PKG_MD5SUM:=74ed192672f4330022f71c6f72095690
PKG_VERSION:=$(PKG_BASE_VERSION)$(PKG_PATCHLEVEL)
PKG_TARBALL_VERSION:=$(PKG_BASE_VERSION).00$(PKG_PATCHLEVEL)
PKG_BUILD_DIR:=$(COMPILE_DIR)/$(PKG_NAME)-$(PKG_TARBALL_VERSION)-src
PKG_SOURCE:=$(PKG_NAME)-$(PKG_TARBALL_VERSION)-src.tar.gz
PKG_SOURCE_URL:=https://allseenalliance.org/releases/alljoyn/$(PKG_BASE_VERSION)/

# SCons supports parallel builds but does not support make's jobserver
PKG_BUILD_PARALLEL:=$(if $(CONFIG_PKG_BUILD_USE_JOBSERVER),0,1)

PKG_CONFIG_DEPENDS:= \
	CONFIG_DEBUG \
	CONFIG_PACKAGE_alljoyn-c \
	CONFIG_PACKAGE_alljoyn-about \
	CONFIG_PACKAGE_alljoyn-config \
	CONFIG_PACKAGE_alljoyn-samples

include $(BUILD_DIR)/package.mk

ifneq ($(CONFIG_CCACHE),)
  TARGET_CC=$(TARGET_CC_NOCACHE)
  TARGET_CXX=$(TARGET_CXX_NOCACHE)
endif

define Package/alljoyn/Default
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=+libopenssl +libpthread +librt +libstdcpp +libcap
  URL:=http://www.allseenalliance.org
  MAINTAINER:=AllSeen Alliance <allseen-core@lists.allseenalliance.org>
endef

define Package/alljoyn
$(call Package/alljoyn/Default)
  TITLE:=AllJoyn Peer-to-Peer networking
  MENU:=1
endef

define Package/alljoyn/description
AllJoyn is a daemon and application framework for developing
ad hoc peer-to-peer applications.  This includes the run-time
daemon and C++ library.
endef


define Package/alljoyn-non-gw-config
$(call Package/alljoyn/Default)
  TITLE:=AllJoyn - alternate non-Gateway Config
  DEPENDS:=alljoyn
endef

define Package/alljoyn-non-gw-config/description
Alternate AllJoyn configuration with defaults suitable for
non-Gateway devices.  Do not install this on devices with a
network interface connected to an insecure network such as
the Internet.
endef


define Package/alljoyn-c
$(call Package/alljoyn/Default)
  TITLE:=AllJoyn - C binding
  DEPENDS:=alljoyn
endef

define Package/alljoyn-c/description
AllJoyn C binding.  This provides a C library for building
AllJoyn programs in C.  This is just a C callable wrapper
to the C++ library.
endef


define Package/alljoyn-about
$(call Package/alljoyn/Default)
  TITLE:=AllJoyn - About service library (deprecated)
  DEPENDS:=alljoyn
endef

define Package/alljoyn-about/description
AllJoyn About services library.  This library is deprecated.  The functionality
provided by About has been integrated into liballjoyn with a simpler API.  This
library will continue to be supported into 2015 and will be removed in 2016.
endef

define Package/alljoyn-config
$(call Package/alljoyn/Default)
	TITLE:=AllJoyn Config service library
	DEPENDS:=alljoyn \
		+PACKAGE_alljoyn-about:alljoyn-about
endef

define Package/alljoyn-config/description
AllJoyn Config service library.
endef

define Package/alljoyn-samples
$(call Package/alljoyn/Default)
  TITLE:=AllJoyn - testing samples
  DEPENDS:=alljoyn \
	+PACKAGE_alljoyn-about:alljoyn-about \
	+PACKAGE_alljoyn-about:alljoyn-config
endef

define Package/alljoyn-samples/description
Alljoyn samples.  (This is primarily for testing.  The contents of this
package vary according which other alljoyn packages are selected.)
endef

ALLJOYN_BINDINGS:=cpp,
ALLJOYN_SERVICES:=
ALLJOYN_BUILD_SERVICES_SAMPLES:=off

ifdef CONFIG_PACKAGE_alljoyn-c
  ALLJOYN_BINDINGS+=c,
endif

ifdef CONFIG_PACKAGE_alljoyn-about
  ALLJOYN_SERVICES+=about,
endif

ifdef CONFIG_PACKAGE_alljoyn-config
  ALLJOYN_SERVICES+=config,
endif

ifeq ($(CONFIG_DEBUG),y)
  ALLJOYN_BUILD_VARIANT:=debug
else
  ALLJOYN_BUILD_VARIANT:=release
endif


PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/build/openwrt/openwrt/$(ALLJOYN_BUILD_VARIANT)/dist
PKG_INSTALL_TEST_DIR:=$(PKG_BUILD_DIR)/build/openwrt/openwrt/$(ALLJOYN_BUILD_VARIANT)/test

TARGET_CFLAGS+=-fPIC
TARGET_CPPFLAGS+=-fPIC

ifdef CONFIG_PACKAGE_$(PKG_NAME)-samples
  ifdef CONFIG_PACKAGE_alljoyn-about
    ALLJOYN_BUILD_SERVICES_SAMPLES:=on
  endif
endif

define Build/Configure
# Override default to do nothing
endef

define Build/Compile
	scons -C $(PKG_BUILD_DIR) \
		$(PKG_JOBS) \
		WS=off \
		CPU=openwrt \
		OS=openwrt \
		"BINDINGS=$(ALLJOYN_BINDINGS)" \
		"SERVICES=$(ALLJOYN_SERVICES)" \
		"BUILD_SERVICES_SAMPLES=$(ALLJOYN_BUILD_SERVICES_SAMPLES)" \
		"VARIANT=$(ALLJOYN_BUILD_VARIANT)" \
		BR=on \
		ICE=off \
		POLICYDB=on \
		"TARGET_CC=$$(TARGET_CC)" \
		"TARGET_CXX=$$(TARGET_CXX)" \
		"TARGET_CFLAGS=$$(TARGET_CFLAGS)" \
		"TARGET_CPPFLAGS=$$(TARGET_CPPFLAGS)" \
		"TARGET_PATH=$$(TARGET_PATH)" \
		"TARGET_LINKFLAGS=$$(TARGET_LDFLAGS)" \
		"TARGET_LINK=$$(TARGET_CC)" \
		"TARGET_AR=$$(TARGET_AR)" \
		"TARGET_RANLIB=$$(TARGET_RANLIB)" \
		"STAGING_DIR=$$(STAGING_DIR)"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/include/alljoyn
	$(INSTALL_DIR) $(1)/usr/include/qcc
	$(INSTALL_DIR) $(1)/usr/include/qcc/posix
	$(CP) $(PKG_INSTALL_DIR)/cpp/inc/alljoyn/*.h $(1)/usr/include/alljoyn/
	$(CP) $(PKG_INSTALL_DIR)/cpp/inc/qcc/*.h $(1)/usr/include/qcc/
	$(CP) $(PKG_INSTALL_DIR)/cpp/inc/qcc/posix/*.h $(1)/usr/include/qcc/posix/
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/cpp/lib/liballjoyn.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/cpp/lib/liballjoyn.so $(1)/usr/lib/

ifdef CONFIG_PACKAGE_alljoyn-about
	$(INSTALL_DIR) $(1)/usr/include/alljoyn/about
	$(CP) $(PKG_INSTALL_DIR)/cpp/inc/alljoyn/about/*.h $(1)/usr/include/alljoyn/about
	$(CP) $(PKG_INSTALL_DIR)/cpp/lib/liballjoyn_about.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/cpp/lib/liballjoyn_about.so $(1)/usr/lib/
endif

ifdef CONFIG_PACKAGE_alljoyn-config
	$(INSTALL_DIR) $(1)/usr/include/alljoyn/config
	$(CP) $(PKG_INSTALL_DIR)/cpp/inc/alljoyn/config/*.h $(1)/usr/include/alljoyn/config
	$(CP) $(PKG_INSTALL_DIR)/cpp/lib/liballjoyn_config.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/cpp/lib/liballjoyn_config.so $(1)/usr/lib/
endif

ifdef CONFIG_PACKAGE_alljoyn-c
	$(INSTALL_DIR) $(1)/usr/include/alljoyn_c
	$(CP) $(PKG_INSTALL_DIR)/c/inc/alljoyn_c/*.h $(1)/usr/include/alljoyn_c/
	$(CP) $(PKG_INSTALL_DIR)/c/lib/liballjoyn_c.a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/c/lib/liballjoyn_c.so $(1)/usr/lib/
endif
endef

define Package/alljoyn/conffiles
/etc/alljoyn/alljoyn.conf
/etc/config/alljoyn
endef

define Package/alljoyn-non-gw-config/conffiles
/etc/config/alljoyn
endef

define Package/alljoyn/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/alljoyn-daemon $(1)/usr/bin
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/cpp/lib/liballjoyn.so $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/etc/init.d
	$(INSTALL_BIN) ./files/alljoyn.init $(1)/etc/init.d/alljoyn
	$(INSTALL_DIR) $(1)/etc/uci-defaults
	$(INSTALL_BIN) ./files/alljoyn.defaults $(1)/etc/uci-defaults/alljoyn
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/alljoyn.config $(1)/etc/config/alljoyn
	$(INSTALL_DIR) $(1)/etc/alljoyn
	$(INSTALL_DATA) ./files/alljoyn.conf $(1)/etc/alljoyn/alljoyn.conf
endef

define Package/alljoyn-non-gw-config/install
	$(INSTALL_DIR) $(1)/etc/config
	$(INSTALL_CONF) ./files/alljoyn-non-gw.config $(1)/etc/config/alljoyn
endef

define Package/alljoyn-c/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/c/lib/liballjoyn_c.so $(1)/usr/lib/
endef

define Package/alljoyn-about/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/cpp/lib/liballjoyn_about.so $(1)/usr/lib/
endef

define Package/alljoyn-config/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/cpp/lib/liballjoyn_config.so $(1)/usr/lib/
endef

define Package/alljoyn-samples/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/aes_ccm $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/bastress $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/bignum $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/bbjitter $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/marshal $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/names $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/remarshal $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/socktest $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/srp $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_TEST_DIR)/cpp/bin/unpack $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/advtunnel $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/bastress2 $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/bbclient $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/bbjoin $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/bbservice $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/bbsig $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/ns $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/rawclient $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/rawservice $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/sessions $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/samples/AboutClient $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/samples/AboutService $(1)/usr/bin/
ifdef CONFIG_PACKAGE_alljoyn-about
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/samples/sample_rule_app $(1)/usr/bin/
endif

ifdef CONFIG_PACKAGE_alljoyn-config
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/samples/ConfigClient $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/cpp/bin/samples/ConfigService $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/alljoyn-config
	$(CP) $(PKG_BUILD_DIR)/services/config/cpp/samples/ConfigServiceSample/ConfigService.conf $(1)/etc/alljoyn-config/
endif
endef

define Package/alljoyn/postinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    . /etc/uci-defaults/alljoyn
    rm -f /etc/uci-defaults/alljoyn
fi
exit 0
endef

define Package/alljoyn/prerm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    /etc/init.d/alljoyn stop
    /etc/init.d/alljoyn disable
    uci batch <<EOF
del_list ucitrack.@network[-1].affects=alljoyn
delete ucitrack.@alljoyn[-1]
commit ucitrack
EOF
fi
exit 0
endef


define Package/alljoyn-non-gw-config/preinst
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    echo 'Saving original alljoyn config file to alljoyn-orig'
    mv /etc/config/alljoyn /etc/config/alljoyn-orig
fi
exit 0
endef

define Package/alljoyn-non-gw-config/postrm
#!/bin/sh
if [ -z "$${IPKG_INSTROOT}" ]; then
    if [ -e /etc/config/alljoyn-orig ]; then
        echo 'Restoring original alljoyn config file from alljoyn-orig'
        mv /etc/config/alljoyn-orig /etc/config/alljoyn
    fi
fi
exit 0
endef

$(eval $(call BuildPackage,alljoyn))
$(eval $(call BuildPackage,alljoyn-non-gw-config))
$(eval $(call BuildPackage,alljoyn-c))
$(eval $(call BuildPackage,alljoyn-about))
$(eval $(call BuildPackage,alljoyn-config))
$(eval $(call BuildPackage,alljoyn-samples))
