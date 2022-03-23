include $(TOPDIR)/rules.mk

PKG_NAME:=tinatest
PKG_VERSION:=1
PKG_RELEASE:=2

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

ifeq ($(DUMP),)
STAMP_CONFIGURED:=$(strip $(STAMP_CONFIGURED))_$(shell $(SH_FUNC) \
	  grep '^CONFIG_TINATEST_' $(TOPDIR)/.config | md5s)
endif

define Package/tinatest
  MENU:=1
  SECTION:=test
  CATEGORY:=TestTools
  TITLE:=Test Platform For TinaSDK
  DEPENDS := +libubox +libjson-c
  DEPENDS += $(shell cat plugin/Depends.mk)
endef

define Package/tinatest/config
	source "$(SOURCE)/config/Config.in"
endef

define Package/tinatest/description
  Tinatest is a test platform for function/specification/stress test.
  It is customized in menuconfig and give birth to tinatest.json which control
  how tinatest run.
  It will also collect system info by collectd while running testcases.
  Tinatest is a strong tool to test applications but also to test tinalinux.
  You can use it to daily automated test, production function test and so on.
endef

PREFIX := $(PKG_BUILD_DIR)/src/out
TTLIB := -ltinatest -L$(PREFIX)/lib -ljson-c -pthread -ldl
TTINCLUDE := -I$(PREFIX)/include -I$(PKG_BUILD_DIR)/src/core
export CP MAKE TTLIB

ifeq ($(CONFIG_SUNXI_DISPLAY_TWO),y)
TARGET_CFLAGS+=-D__SUNXI_DISPLAY2__
endif
ifeq ($(CONFIG_SUNXI_DISPLAY_ONE),y)
TARGET_CFLAGS+=-D__SUNXI_DISPLAY__
endif

define Build/Prepare
	@mkdir -p $(PREFIX)/include
	@mkdir -p $(PREFIX)/bin
	@mkdir -p $(PREFIX)/lib
	@mkdir -p $(PREFIX)/lib/tt-module
	@$(CP) ./src $(PKG_BUILD_DIR)
	@$(CP) ./plugin $(PKG_BUILD_DIR)
	@$(CP) ./files $(PKG_BUILD_DIR)
	@$(CP) ./testcase $(PKG_BUILD_DIR)
endef

define Build/Compile
	@grep "^CONFIG_TINATEST.*" $(TOPDIR)/.config > $(PKG_BUILD_DIR)/.config
	@./tools/convert-json/convert_json.sh $(PKG_BUILD_DIR)/.config > $(PKG_BUILD_DIR)/src/tinatest.json

	@$(MAKE) --quiet -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(TTINCLUDE)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		BUILD_DIR="$(PKG_BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all

	@$(MAKE) --quiet -C $(PKG_BUILD_DIR)/plugin \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(TTINCLUDE) -fPIC" \
		LDFLAGS="$(TARGET_LDFLAGS) -shared $(TTLIB)" \
		BUILD_DIR="$(PKG_BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all

	@$(MAKE) --quiet -C $(PKG_BUILD_DIR)/testcase \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(TTINCLUDE)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(TTLIB)" \
		BUILD_DIR="$(PKG_BUILD_DIR)" \
		all

ifneq ($(CONFIG_TINATEST_SYS_BUILD_COMPLETION),)
	@mkdir -p $(PREFIX)/completion
	@./tools/complete-ash.sh $(PKG_BUILD_DIR)/.config $(PREFIX)/completion
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	@$(INSTALL_DATA) $(PREFIX)/include/* $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PREFIX)/lib/*.so $(1)/usr/lib
endef
define Package/tinatest/install
	@$(INSTALL_DIR) $(1)/etc
	@$(INSTALL_DIR) $(1)/usr/bin
	@$(INSTALL_DIR) $(1)/usr/lib
	@$(INSTALL_DIR) $(1)/usr/lib/tt-module
	@$(INSTALL_DIR) $(1)/usr/lib/tt-data
	@# library && tiantest
	@$(INSTALL_BIN)  $(PREFIX)/bin/* $(1)/usr/bin
	@$(INSTALL_DATA) $(PREFIX)/lib/*.so $(1)/usr/lib
	@$(LN) /usr/bin/tinatest $(1)/usr/bin/tt
	@$(INSTALL_DATA) $(PKG_BUILD_DIR)/src/tinatest.json $(1)/etc
	@# plugin
	@echo -e "\033[33m******** PLUGIN BIN ********\033[0m"
	@find $(PREFIX)/lib/tt-module -type f -exec basename {} \; \
		| xargs -I {} echo -e "\033[32m/usr/lib/tt-module/{}\033[0m"
	@find $(PREFIX)/lib/tt-module -type f -exec $(INSTALL_DATA) {} $(1)/usr/lib/tt-module \;
	@# testcases
	@echo -e "\033[33m******** TESTCASE BIN ********\033[0m"
	@find $(PKG_BUILD_DIR)/testcase/bin -type f -exec basename {} \; \
		| xargs -I {} echo -e "\033[32m/usr/bin/{}\033[0m"
	@find $(PKG_BUILD_DIR)/testcase/bin -type f -exec $(INSTALL_BIN) {} $(1)/usr/bin \;
	@echo -e "\033[33m******** TESTCASE DATA ********\033[0m"
	@find $(PKG_BUILD_DIR)/testcase/data -type f -exec basename {} \; \
		| xargs -I {} echo -e "\033[32m/usr/lib/tt-data/{}\033[0m"
	@find $(PKG_BUILD_DIR)/testcase/data -type f -exec $(INSTALL_DATA) {} $(1)/usr/lib/tt-data \;
ifneq ($(CONFIG_TINATEST_SYS_BUILD_BOOT_SCRIPT),)
	@$(INSTALL_DIR) $(1)/etc/init.d
	@sed -i -e 's/^START=.*/START=$(CONFIG_TINATEST_SYS_BUILD_BOOT_SCRIPT_LEVEL)/' \
		-e 's/^CMD=.*/CMD=$(CONFIG_TINATEST_SYS_BUILD_BOOT_SCRIPT_COMMAND)/' \
		-e "s/^DEPEND=.*/DEPEND=$(shell echo $(CONFIG_TINATEST_SYS_BUILD_BOOT_SCRIPT_DEPEND) | sed 's/"//g')/" \
		$(PKG_BUILD_DIR)/files/tinatest.init
	@$(INSTALL_BIN) $(PKG_BUILD_DIR)/files/tinatest.init $(1)/etc/init.d/tinatest
endif
ifneq ($(CONFIG_TINATEST_SYS_BUILD_COMPLETION),)
	@$(CP) $(PREFIX)/completion/* $(1)
endif
endef

$(eval $(call BuildPackage,tinatest))
