AJ_SERVICE:=notification
AJ_SERVICE_NAME:=Notification

include $(TOPDIR)/rules.mk

PKG_NAME:=alljoyn-$(AJ_SERVICE)
PKG_BASE_VERSION:=16.04
PKG_PATCHLEVEL:=.00
PKG_RELEASE:=1
#PKG_MD5SUM:=389c2623619281e6849da25cee7e1bb7
PKG_VERSION:=$(PKG_BASE_VERSION)$(PKG_PATCHLEVEL)
PKG_TARBALL_VERSION:=$(PKG_VERSION)
PKG_BUILD_DIR:=$(COMPILE_DIR)/alljoyn-services-$(PKG_TARBALL_VERSION)-src
PKG_SOURCE:=alljoyn-base-$(PKG_TARBALL_VERSION).tar.gz
PKG_SOURCE_URL:=https://allseenalliance.org/releases/alljoyn/$(PKG_BASE_VERSION)/

# SCons supports parallel builds but does not support make's jobserver
PKG_BUILD_PARALLEL:=$(if $(CONFIG_PKG_BUILD_USE_JOBSERVER),0,1)

PKG_CONFIG_DEPENDS:= CONFIG_PACKAGE_$(PKG_NAME)-samples

include $(BUILD_DIR)/package.mk

ifneq ($(CONFIG_CCACHE),)
  TARGET_CC=$(TARGET_CC_NOCACHE)
  TARGET_CXX=$(TARGET_CXX_NOCACHE)
endif

ALLJOYN_BINDINGS:=cpp,
ALLJOYN_BUILD_SERVICES_SAMPLES:=off

define Package/$(PKG_NAME)
  TITLE:=AllJoyn $(AJ_SERVICE_NAME) service library
  SECTION:=net
  CATEGORY:=Network
  DEPENDS:=alljoyn \
	+alljoyn-about
  URL:=http://www.allseenalliance.org
  MAINTAINER:=AllSeen Alliance <allseen-core@lists.allseenalliance.org>
  MENU:=1
endef

define Package/$(PKG_NAME)/description
AllJoyn $(AJ_SERVICE_NAME) service library.
endef


define Package/$(PKG_NAME)-samples
$(call Package/$(PKG_NAME))
  TITLE+=- testing samples
  DEPENDS:=$(PKG_NAME) \
	+alljoyn-services_common
  MENU:=0
endef

define Package/$(PKG_NAME)-samples/description
Alljoyn $(AJ_SERVICE_NAME) service library samples.  (This is primarily for testing.
The contents of this package vary according which other alljoyn packages
are selected.)
endef


ifeq ($(CONFIG_DEBUG),y)
  ALLJOYN_BUILD_VARIANT:=debug
else
  ALLJOYN_BUILD_VARIANT:=release
endif


ALLJOYN_DISTDIR:=$(COMPILE_DIR)/alljoyn-$(PKG_VERSION)-src/build/openwrt/openwrt/$(ALLJOYN_BUILD_VARIANT)/dist
PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/$(AJ_SERVICE)/build/openwrt/openwrt/$(ALLJOYN_BUILD_VARIANT)/dist/$(AJ_SERVICE)

TARGET_CFLAGS+=-fPIC
TARGET_CPPFLAGS+=-fPIC

ifdef CONFIG_PACKAGE_$(PKG_NAME)-samples
  ifdef CONFIG_PACKAGE_alljoyn-about
    ALLJOYN_BUILD_SERVICES_SAMPLES:=on
  endif
endif

define Build/Configure/Default
# Override default to do nothing
endef

define Build/Compile
	scons -C $(PKG_BUILD_DIR)/$(AJ_SERVICE) \
		$(PKG_JOBS) \
		WS=off \
		CPU=openwrt \
		OS=openwrt \
		"BINDINGS=$(ALLJOYN_BINDINGS)" \
		"BUILD_SERVICES_SAMPLES=$(ALLJOYN_BUILD_SERVICES_SAMPLES)" \
		"VARIANT=$(ALLJOYN_BUILD_VARIANT)" \
		BR=off \
		BT=off \
		ICE=off \
		"TARGET_CC=$$(TARGET_CC)" \
		"TARGET_CXX=$$(TARGET_CXX)" \
		"TARGET_CFLAGS=$$(TARGET_CFLAGS)" \
		"TARGET_CPPFLAGS=$$(TARGET_CPPFLAGS)" \
		"TARGET_PATH=$$(TARGET_PATH)" \
		"TARGET_LINKFLAGS=$$(TARGET_LDFLAGS)" \
		"TARGET_LINK=$$(TARGET_CC)" \
		"TARGET_AR=$$(TARGET_AR)" \
		"TARGET_RANLIB=$$(TARGET_RANLIB)" \
		"STAGING_DIR=$$(STAGING_DIR)" \
		"ALLJOYN_DISTDIR=$$(ALLJOYN_DISTDIR)"
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include/alljoyn/$(AJ_SERVICE)
	$(CP) $(PKG_INSTALL_DIR)/inc/alljoyn/$(AJ_SERVICE)/*.h $(1)/usr/include/alljoyn/$(AJ_SERVICE)
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/liballjoyn_$(AJ_SERVICE).a $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/lib/liballjoyn_$(AJ_SERVICE).so $(1)/usr/lib/
endef


define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib
	$(CP) $(PKG_INSTALL_DIR)/lib/liballjoyn_$(AJ_SERVICE).so $(1)/usr/lib/
endef


define Package/$(PKG_NAME)-samples/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/bin/* $(1)/usr/bin/
endef


$(eval $(call BuildPackage,$(PKG_NAME)))
$(eval $(call BuildPackage,$(PKG_NAME)-samples))
