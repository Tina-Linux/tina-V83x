include $(TOPDIR)/rules.mk

PKG_NAME:=audiotest
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

define Package/audiotest
  SECTION:=utils
  CATEGORY:=TestTools
  SUBMENU:=testapk
  TITLE:=Audio test utilities
  DEPENDS:=+alsa-lib
  MENU:=1
endef

define Package/audiotest/description
  It provides some utilities for audio test.
endef

define Package/audiotest-utils/Default
  SECTION:=utils
  CATEGORY:=TestTools
  DEPENDS:=+alsa-lib
endef

define Package/audiotest/config
  if PACKAGE_audiotest

      comment "Utilities"

      $(foreach util,$(AUDIOTEST_UTILS), \
        config PACKAGE_audiotest_$(util)
            prompt"audiotest_$(util)"

      )
  endif
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		CXXFLAGS="$(TARGET_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/audiotest/install
	/bin/true
endef

# 1: package name suffix
# 2: title
# 3: dependencies on other packages
define BuildAudiotestUtil
  AUDIOTEST_UTILS += $(1)

  define Package/audiotest_$(1)
    $(call Package/audiotest-utils/Default)
    TITLE:=$(2)
    DEPENDS+=$(3)
    HIDDEN:=1
  endef

  define Package/audiotest_$(1)/description
    audiotest utilities: audiotest_$(1)
  endef

  define Package/audiotest_$(1)/install
	$(INSTALL_DIR) $$(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/audiotest_$(1) $$(1)/usr/bin
  endef

  $$(eval $$(call BuildPackage,audiotest_$(1)))
endef

$(eval $(call BuildAudiotestUtil,wav_header,Parse and print wav header,,))
$(eval $(call BuildAudiotestUtil,wav_play_time,Get wav playing time,,))

$(eval $(call BuildPackage,audiotest))
