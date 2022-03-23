include $(TOPDIR)/rules.mk

PKG_NAME:=benchmarks
PKG_RELEASE:=1

include $(BUILD_DIR)/package.mk

define Package/benchmarks
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:= Benchmark program
endef

define Package/benchmarks/description
 This package contains the several benchmark test.
 Contains Dhrystone, Linpack and Coremark 
endef

define Package/benchmarks/config
menu "Configuration"
	depends on PACKAGE_benchmarks
config BENCHMARKS_DHRYSTONE
	bool "compile Dhrystone"
	default n
config BENCHMARKS_LINPACK
	bool "compile Linpack"
	default y
config BENCHMARKS_COREMARK
	bool "compile Coremark"
	default y
config BENCHMARKS_STATIC_COMPILE
	bool "static compile"
	default n
endmenu
endef

define Build/Prepare
	$(INSTALL_DIR) $(PKG_BUILD_DIR)
	$(CP) ./src/ $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		MAKE="$(MAKE)" \
		CC="$(TARGET_CC)" \
		CFLAGS="$(TARGET_CFLAGS)"  \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		ARCH="$(CONFIG_ARCH)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		BENCHMARKS_DHRYSTONE=$(CONFIG_BENCHMARKS_DHRYSTONE) \
		BENCHMARKS_LINPACK=$(CONFIG_BENCHMARKS_LINPACK) \
		BENCHMARKS_COREMARK=$(CONFIG_BENCHMARKS_COREMARK) \
		BENCHMARKS_STATIC_COMPILE=$(CONFIG_BENCHMARKS_STATIC_COMPILE) \
		all
endef

define Package/benchmarks/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,benchmarks))
