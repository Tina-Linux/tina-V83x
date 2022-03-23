#
# Copyright (C) 2006-2010 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk

PKG_NAME:=tslib
PKG_VERSION:=1.15
PKG_RELEASE:=2

PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.bz2
PKG_MD5SUM:=6491d4fd9c7c6420690e740c4664b96b
PKG_BUILD_DIR:=$(COMPILE_DIR)/tslib-$(PKG_VERSION)
PKG_FIXUP:=libtool
PKG_INSTALL:=1

include $(BUILD_DIR)/package.mk

define Package/tslib
  SECTION:=libs
  CATEGORY:=Libraries
  TITLE:=tslib is an abstraction layer for touchscreen panel events
  URL:=https://github.com/kergoth/tslib
endef

define Package/tslib/description
  tslib is an abstraction layer for touchscreen panel events,
  as well as a filter stack for the manipulation of those events.
  Examples of implemented filters include jitter smoothing and the calibration transform.
endef

define Package/tslib/config
    menu "Configuration tslib features"
        depends on PACKAGE_tslib

        config TSLIB_ENABLE_input
        bool "Generic input raw module"
        default y
        help
           Building for generic input raw module (Linux /dev/input/eventN)

        config TSLIB_ENABLE_debounce
       bool "Debounce filter module"
       default n
       help
          Building of debounce filter

        config TSLIB_ENABLE_dejitter
       bool "Dejitter filter module"
           default n
       help
          Building of dejitter filter

        config TSLIB_ENABLE_iir
       bool "IIR filter module"
           default n
           help
              Building of IIR filter

        config TSLIB_ENABLE_linear
       bool "Linear scaling module"
           default n
           help
              Building of linear scaling

        config TSLIB_ENABLE_median
           bool "Median filter module"
           default n
           help
              Building of median filter

        config TSLIB_ENABLE_pthres
           bool "Pthres filter module"
           default n
           help
              Building of pthres filter

        config TSLIB_ENABLE_skip
           bool "Skip filter module"
           default n
           help
              Building of skip filter

        config TSLIB_ENABLE_lowpass
           bool "Lowpass filter module"
           default n
           help
              Building of lowpass filter

        config TSLIB_ENABLE_invert
           bool "Invert filter module"
           default n
           help
              Building of invert filter

        config TSLIB_ENABLE_variance
           bool "Variance filter module"
           default n
           help
              Building of variance filter

        config TSLIB_ENABLE_tatung
           bool "Tatung filter module"
           default n
       help
          Building of tatung raw module (Tatung Webpad support)

        config TSLIB_ENABLE_touchkit
           bool "Touchkit filter module"
           default n
           help
              Building of serial TouchKit raw module (Linux /dev/ttySX support)

        config TSLIB_ENABLE_waveshare
           bool "Waveshare filter module"
           default n
           help
          Building of WaveShare raw module (Linux /dev/hidrawN support)

        config TSLIB_ENABLE_ucb1x00
           bool "Ucb1x00 filter module"
           default n
           help
              Building of ucb1x00 raw module (UCB1x00 support)

        config TSLIB_ENABLE_mk712
           bool "Mk712 filter module"
           default n
           help
          Building of mk712 raw module (Hi tachi support)

        config TSLIB_ENABLE_h3600
           bool "H3600 filter module"
           default n
           help
              Building of h3600 raw module (HP iPaq H3600 support)

        config TSLIB_ENABLE_dmc
           bool "Dmc filter module"
           default n
           help
              Building of dmc raw module (HP iPaq DMC support)

        config TSLIB_ENABLE_linear-h2200
       bool "Liner-h2200 filter module"
           default n
           help
              Building of linearizing filter for iPAQ h2200

        config TSLIB_ENABLE_corgi
           bool "Corgi filter module"
           default n
           help
              Building of corgi raw module (Sharp Zaurus sl-c7x0 support)

        config TSLIB_ENABLE_collie
           bool "Collie raw module"
           default n
           help
              Building of collie raw module (Sharp Zaurus sl-5500/5000d support)

        config TSLIB_ENABLE_arctic2
           bool "Arctic2 raw module"
           default n
           help
              Building of arctic2 raw module (IBM Arctic II support)

        config TSLIB_ENABLE_dmc_dus3000
           bool "Dmc_dus3000 raw module"
           default n
           help
              Building of dmc_dus3000 raw module (DMC DUS Series support)

        config TSLIB_ENABLE_cy8mrln-palmpre
           bool "Cy8mrln-palmpre raw module"
           default n
           help
              Building of cy8mrln-palmpre raw module

        config TSLIB_ENABLE_galax
           bool "Galax raw module"
           default n
           help
              Building of HID USB eGalax raw module (Linux /dev/hiddevN support)

	config TSLIB_ENABLE_tests
	   bool "Compiling tests program"
	   default n
	   help
	      Select it to compile the tests program for tslib

	config TSLIB_ENABLE_tools
	   bool "Tools program (ts_uinput)"
	   default n
	   help
	      Wheter enable the tools
    endmenu

endef

define Build/Prepare
	if [ -d "patches" ]; then \
		cp patches/ patches_tmp -rfd; \
	fi

	if [ -d "${TARGET_PLATFORM}-patches" ]; then \
		mkdir -p patches; \
		cp ${TARGET_PLATFORM}-patches/* patches/; \
	fi
	$(Build/Prepare/Default)

	rm patches/ -rf
	if [ -d "patches_tmp" ]; then \
		mv patches_tmp patches; \
	fi
endef

define Package/tslib/conffiles
	/etc/tslib-env.sh
	/etc/ts.conf
	/etc/pointercal
endef

EXTRA_CFLAGS+= -D__linux__

CONFIGURE_ARGS += \
        $(if $(CONFIG_TSLIB_ENABLE_galax), --enable-galax, --disable-galax) \
        $(if $(CONFIG_TSLIB_ENABLE_cy8mrln-palmpre), --enable-cy8mrln-palmpre, --disable-cy8mrln-palmpre) \
        $(if $(CONFIG_TSLIB_ENABLE_dmc_dus3000), --enable-dmc_dus3000, --disable-dmc_dus3000) \
        $(if $(CONFIG_TSLIB_ENABLE_arctic2), --enable-arctic2, --disable-arctic2) \
        $(if $(CONFIG_TSLIB_ENABLE_collie), --enable-collie, --disable-collie) \
        $(if $(CONFIG_TSLIB_ENABLE_corgi), --enable-corgi, --disable-corgi) \
        $(if $(CONFIG_TSLIB_ENABLE_linear-h2200), --enable-linear-h2200, --disable-linear-h2200) \
        $(if $(CONFIG_TSLIB_ENABLE_dmc), --enable-dmc, --disable-dmc) \
        $(if $(CONFIG_TSLIB_ENABLE_h3600), --enable-h3600, --disable-h3600) \
        $(if $(CONFIG_TSLIB_ENABLE_mk712), --enable-mk712, --disable-mk712) \
        $(if $(CONFIG_TSLIB_ENABLE_ucb1x00), --enable-ucb1x00, --disable-ucb1x00) \
        $(if $(CONFIG_TSLIB_ENABLE_waveshare), --enable-waveshare, --disable-waveshare) \
        $(if $(CONFIG_TSLIB_ENABLE_touchkit), --enable-touchkit, --disable-touchkit) \
        $(if $(CONFIG_TSLIB_ENABLE_tatung), --enable-tatung, --disable-tatung) \
        $(if $(CONFIG_TSLIB_ENABLE_variance), --enable-variance, --disable-variance) \
        $(if $(CONFIG_TSLIB_ENABLE_invert), --enable-invert, --disable-invert) \
        $(if $(CONFIG_TSLIB_ENABLE_lowpass), --enable-lowpass, --disable-lowpass) \
        $(if $(CONFIG_TSLIB_ENABLE_skip), --enable-skip, --disable-skip) \
        $(if $(CONFIG_TSLIB_ENABLE_pthres), --enable-pthres, --disable-pthres) \
        $(if $(CONFIG_TSLIB_ENABLE_median), --enable-median, --disable-median) \
        $(if $(CONFIG_TSLIB_ENABLE_linear), --enable-linear, --disable-linear) \
        $(if $(CONFIG_TSLIB_ENABLE_iir), --enable-iir, --disable-iir) \
        $(if $(CONFIG_TSLIB_ENABLE_dejitter), --enable-dejitter, --disable-dejitter) \
        $(if $(CONFIG_TSLIB_ENABLE_debounce), --enable-debounce, --disable-debounce) \
        $(if $(CONFIG_TSLIB_ENABLE_input), --enable-input, --disable-input)  \
	$(if $(CONFIG_TSLIB_ENABLE_tools), --enable-tools, --disable-tools) \
	$(if $(CONFIG_TSLIB_ENABLE_tests), --enable-tests, --disable-tests) \
	--enable-shared

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include \
		$(1)/usr/lib/pkgconfig
	$(CP) $(PKG_INSTALL_DIR)/usr/include/* $(1)/usr/include/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/
	$(CP) $(PKG_INSTALL_DIR)/usr/lib/pkgconfig/** $(1)/usr/lib/pkgconfig
endef

define Package/tslib/install
	$(INSTALL_DIR) $(1)/usr/lib/ts \
		$(1)/usr/bin \
		$(1)/etc
	{  \
	    set +e; \
	    $(CP) $(PKG_INSTALL_DIR)/usr/bin/ts_* $(1)/usr/bin/;  \
	    $(CP) $(PKG_INSTALL_DIR)/usr/lib/*.so* $(1)/usr/lib/;  \
	    $(CP) $(PKG_INSTALL_DIR)/usr/lib/ts/*.so* $(1)/usr/lib/ts/; \
	    $(CP) ./files/ts.conf $(1)/etc/ts.conf; \
	    $(CP) ./files/tslib-env.sh $(1)/etc/; \
	}
endef

$(eval $(call BuildPackage,tslib))
