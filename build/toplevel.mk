# Makefile for OpenWrt
#
# Copyright (C) 2007-2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

RELEASE:=Designated Driver
PREP_MK= OPENWRT_BUILD= QUIET=0

export IS_TTY=$(shell tty -s && echo 1 || echo 0)

include $(TOPDIR)/build/verbose.mk

ifeq ($(SDK),1)
  include $(TOPDIR)/build/version.mk
else
  REVISION:=$(shell $(TOPDIR)/scripts/getver.sh -id)
endif

HOSTCC ?= $(CC)
export RELEASE
export REVISION
export GIT_CONFIG_PARAMETERS='core.autocrlf=false'
export MAKE_JOBSERVER=$(filter --jobserver%,$(MAKEFLAGS))
export SOURCE_DATE_EPOCH:=$(shell $(TOPDIR)/scripts/get_source_date_epoch.sh)

# prevent perforce from messing with the patch utility
unexport P4PORT P4USER P4CONFIG P4CLIENT

# prevent user defaults for quilt from interfering
unexport QUILT_PATCHES QUILT_PATCH_OPTS

unexport C_INCLUDE_PATH CROSS_COMPILE ARCH

# prevent distro default LPATH from interfering
unexport LPATH

# make sure that a predefined CFLAGS variable does not disturb packages
export CFLAGS=
export LDFLAGS=

empty:=
space:= $(empty) $(empty)
path:=$(subst :,$(space),$(PATH))
path:=$(filter-out .%,$(path))
path:=$(subst $(space),:,$(path))
export PATH:=$(path)

unexport TAR_OPTIONS

ifneq ($(shell $(HOSTCC) 2>&1 | grep clang),)
  export HOSTCC_REAL?=$(HOSTCC)
  export HOSTCC_WRAPPER:=$(TOPDIR)/scripts/clang-gcc-wrapper
else
  export HOSTCC_WRAPPER:=$(HOSTCC)
endif

ifeq ($(FORCE),)
  .config scripts/config/conf scripts/config/mconf: out/host/.prereq-build
endif

SCAN_COOKIE?=$(shell echo $$$$)
export SCAN_COOKIE

SUBMAKE:=umask 022; $(SUBMAKE)

ULIMIT_FIX=_limit=`ulimit -n`; [ "$$_limit" = "unlimited" -o "$$_limit" -ge 1024 ] || ulimit -n 1024;

prepare-mk: FORCE ;

ifdef SDK
  IGNORE_PACKAGES = allwinner
endif

ifeq ($(TARGET_CONFIG),)
  TARGET_CONFIG=target/allwinner/$(TARGET_BOARD)/defconfig
endif
_ignore = $(foreach p,$(IGNORE_PACKAGES),--ignore $(p))

prepare-tmpinfo: FORCE
	@+$(MAKE) -r -s out/host/.prereq-build $(PREP_MK)
	mkdir -p tmp/info
	$(_SINGLE)$(NO_TRACE_MAKE) -j1 -r -s -f build/scan.mk SCAN_TARGET="packageinfo" SCAN_DIR="package" SCAN_NAME="package" SCAN_DEPS="$(TOPDIR)/build/package*.mk $(TOPDIR)/overlay/*/*.mk" SCAN_DEPTH=5 SCAN_EXTRA=""
	$(_SINGLE)$(NO_TRACE_MAKE) -j1 -r -s -f build/scan.mk SCAN_TARGET="targetinfo" SCAN_DIR="target/allwinner" SCAN_NAME="target" SCAN_DEPS="*.mk $(TOPDIR)/build/kernel*.mk $(TOPDIR)/build/target.mk" SCAN_DEPTH=2 SCAN_EXTRA="" SCAN_MAKEOPTS="TARGET_BUILD=1"
	for type in package target; do \
		f=tmp/.$${type}info; t=tmp/.config-$${type}.in; \
		[ "$$t" -nt "$$f" ] || ./scripts/metadata.pl $(_ignore) $${type}_config "$$f" > "$$t" || { rm -f "$$t"; echo "Failed to build $$t"; false; break; }; \
	done
	[ tmp/.config-feeds.in -nt tmp/.packagesubdirs ] || ./scripts/feeds feed_config > tmp/.config-feeds.in
	./scripts/metadata.pl package_mk tmp/.packageinfo > tmp/.packagedeps || { rm -f tmp/.packagedeps; false; }
	./scripts/metadata.pl package_subdirs tmp/.packageinfo > tmp/.packagesubdirs || { rm -f tmp/.packagesubdirs; false; }
	touch $(TOPDIR)/tmp/.build

.config: ./scripts/config/conf $(if $(CONFIG_HAVE_DOT_CONFIG),,prepare-tmpinfo)
	@+if [ \! -e .config ] || ! grep CONFIG_HAVE_DOT_CONFIG .config >/dev/null; then \
		if [ "x$(TARGET_BUILD_VARIANT)" = "xtina" ]; then \
			[ -e $(TARGET_CONFIG) ] && \
			cp $(TARGET_CONFIG) .config; \
		fi \
	elif [ -e .config -a "x$(TARGET_BUILD_VARIANT)" = "xtina" ]; then \
		cp $(TARGET_CONFIG) .config; \
	fi

scripts/config/mconf:
	@$(_SINGLE)$(SUBMAKE) -s -C scripts/config all CC="$(HOSTCC_WRAPPER)"

$(eval $(call rdep,scripts/config,scripts/config/mconf))

scripts/config/conf:
	@$(_SINGLE)$(SUBMAKE) -s -C scripts/config conf CC="$(HOSTCC_WRAPPER)"

config: scripts/config/conf prepare-tmpinfo FORCE
	$< Config.in

config-clean: FORCE
	$(_SINGLE)$(NO_TRACE_MAKE) -C scripts/config clean

defconfig: scripts/config/conf prepare-tmpinfo FORCE
	touch .config
	@if [ -z $(TARGET_BOARD) ]; then \
		echo "please run command:"; \
		echo "$ source build/envsetup.sh"; \
		echo "$ make defconfig"; \
	else \
		[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
		[ -e target/allwinner/$(TARGET_BOARD)/defconfig ] && \
		cp target/allwinner/$(TARGET_BOARD)/defconfig .config; \
		$< --defconfig=.config Config.in; \
		if cat .config|grep CONFIG_TARGET_BOARD=\"$(TARGET_BOARD)\" >> /dev/null; then \
			[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
			cp .config target/allwinner/$(TARGET_BOARD)/defconfig; \
			echo ; \
		else \
			rm .config; \
		fi \
	fi

oldconfig: scripts/config/conf prepare-tmpinfo FORCE
	@if [ -z $(TARGET_BOARD) ]; then \
		echo "please run command:"; \
		echo "$ source build/envsetup.sh"; \
		echo "$ make oldconfig"; \
	else \
		[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
		[ -e target/allwinner/$(TARGET_BOARD)/defconfig ] && \
		cp target/allwinner/$(TARGET_BOARD)/defconfig .config; \
		$< --allnoconfig Config.in; \
		if cat .config|grep CONFIG_TARGET_BOARD=\"$(TARGET_BOARD)\" >> /dev/null; then \
			[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
			cp .config target/allwinner/$(TARGET_BOARD)/defconfig; \
			echo ; \
		else \
			rm .config; \
		fi \
	fi

menuconfig: scripts/config/mconf prepare-tmpinfo FORCE
	if [ -z $(TARGET_BOARD) ]; then \
		echo "please run command:"; \
		echo "$ source build/envsetup.sh"; \
		echo "$ make menuconfig"; \
	else \
		[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
		[ -e target/allwinner/$(TARGET_BOARD)/defconfig ] && \
		cp target/allwinner/$(TARGET_BOARD)/defconfig .config; \
		$< Config.in; \
		if cat .config|grep CONFIG_TARGET_BOARD=\"$(TARGET_BOARD)\" >> /dev/null; then \
			[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
			cp .config target/allwinner/$(TARGET_BOARD)/defconfig; \
			echo ; \
		else \
			rm .config; \
		fi \
	fi

ota_menuconfig: scripts/config/mconf prepare-tmpinfo FORCE
	if [ -z $(TARGET_BOARD) ]; then \
		echo "please run command:"; \
		echo "$ source build/envsetup.sh"; \
		echo "$ make menuconfig"; \
	else \
		[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
		[ -e target/allwinner/$(TARGET_BOARD)/defconfig_ota ] && \
		cp target/allwinner/$(TARGET_BOARD)/defconfig_ota .config; \
		$< Config.in; \
		if cat .config|grep CONFIG_TARGET_BOARD=\"$(TARGET_BOARD)\" >> /dev/null; then \
			[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
			cp .config target/allwinner/$(TARGET_BOARD)/defconfig_ota; \
			echo ; \
		else \
			rm .config; \
		fi \
	fi

recovery_menuconfig: scripts/config/mconf prepare-tmpinfo FORCE
	if [ -z $(TARGET_BOARD) ]; then \
		echo "please run command:"; \
		echo "$ source build/envsetup.sh"; \
		echo "$ make menuconfig"; \
	else \
		[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
		[ -e target/allwinner/$(TARGET_BOARD)/defconfig_recovery ] && \
		cp target/allwinner/$(TARGET_BOARD)/defconfig_recovery .config; \
		$< Config.in; \
		if cat .config|grep CONFIG_TARGET_BOARD=\"$(TARGET_BOARD)\" >> /dev/null; then \
			[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
			cp .config target/allwinner/$(TARGET_BOARD)/defconfig_recovery; \
			echo ; \
		else \
			rm .config; \
		fi \
	fi

ramfs_menuconfig: scripts/config/mconf prepare-tmpinfo FORCE
	if [ -z $(TARGET_BOARD) ]; then \
		echo "please run command:"; \
		echo "$ source build/envsetup.sh"; \
		echo "$ make menuconfig"; \
	else \
		[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
		[ -e target/allwinner/$(TARGET_BOARD)/defconfig_ramfs ] && \
		cp target/allwinner/$(TARGET_BOARD)/defconfig_ramfs .config; \
		$< Config.in; \
		if cat .config|grep CONFIG_TARGET_BOARD=\"$(TARGET_BOARD)\" >> /dev/null; then \
			[ "x$(TARGET_BUILD_VARIANT)" = "xtina" ] && \
			cp .config target/allwinner/$(TARGET_BOARD)/defconfig_ramfs; \
			echo ; \
		else \
			rm .config; \
		fi \
	fi

prepare_kernel_conf: .config FORCE

ifeq ($(wildcard out/host/bin/quilt),)
  prepare_kernel_conf:
	@+$(SUBMAKE) -r tools/quilt/install
else
  prepare_kernel_conf: ;
endif

rtos_menuconfig:
	$(_SINGLE)$(NO_TRACE_MAKE) -C rtos menuconfig

kernel_oldconfig: prepare_kernel_conf
	$(_SINGLE)$(NO_TRACE_MAKE) -C target/allwinner oldconfig

kernel_menuconfig: prepare_kernel_conf
	$(_SINGLE)$(NO_TRACE_MAKE) -C target/allwinner menuconfig

recovery_kernel_menuconfig: prepare_kernel_conf
	$(_SINGLE)$(NO_TRACE_MAKE) -C target/allwinner recovery_menuconfig

ramfs_kernel_menuconfig: prepare_kernel_conf
	$(_SINGLE)$(NO_TRACE_MAKE) -C target/allwinner ramfs_menuconfig

kernel_nconfig: prepare_kernel_conf
	$(_SINGLE)$(NO_TRACE_MAKE) -C target/allwinner nconfig

out/host/.prereq-build: build/prereq-build.mk
	mkdir -p tmp
	rm -f tmp/.host.mk
	@$(_SINGLE)$(NO_TRACE_MAKE) -j1 -r -s -f $(TOPDIR)/build/prereq-build.mk prereq 2>/dev/null || { \
		echo "Prerequisite check failed. Use FORCE=1 to override."; \
		false; \
	}
ifneq ($(realpath $(TOPDIR)/build/prepare.mk),)
	@$(_SINGLE)$(NO_TRACE_MAKE) -j1 -r -s -f $(TOPDIR)/build/prepare.mk prepare 2>/dev/null || { \
		echo "Preparation failed."; \
		false; \
	}
endif
	touch $@

printdb: FORCE
	@$(_SINGLE)$(NO_TRACE_MAKE) -p $@ V=99 DUMP_TARGET_DB=1 2>&1

download: .config FORCE
	@+$(SUBMAKE) tools/download
	@+$(SUBMAKE) toolchain/download
	@+$(SUBMAKE) package/download
	@+$(SUBMAKE) target/download

clean dirclean: .config
	@+$(SUBMAKE) -r $@

prereq:: prepare-tmpinfo .config
	@+$(NO_TRACE_MAKE) -r -s $@

WARN_PARALLEL_ERROR = $(if $(BUILD_LOG),,$(and $(filter -j,$(MAKEFLAGS)),$(findstring s,$(OPENWRT_VERBOSE))))

ifeq ($(SDK),1)

%::
	@+$(PREP_MK) $(NO_TRACE_MAKE) -r -s prereq
	@./scripts/config/conf --defconfig=.config Config.in
	@+$(ULIMIT_FIX) $(SUBMAKE) -r $@

else

%::
	@+$(PREP_MK) $(NO_TRACE_MAKE) -r -s prereq
	@( \
		cp .config tmp/.config; \
		./scripts/config/conf --defconfig=tmp/.config -w tmp/.config Config.in > /dev/null 2>&1; \
		if ./scripts/kconfig.pl '>' .config tmp/.config | grep -q CONFIG; then \
			printf "$(_R)WARNING: your configuration is out of sync. Please run make menuconfig, oldconfig or defconfig!$(_N)\n" >&2; \
		fi \
	)
	@+$(ULIMIT_FIX) $(SUBMAKE) -r $@ $(if $(WARN_PARALLEL_ERROR), || { \
		printf "$(_R)Build failed - please re-run with -j1 to see the real error message$(_N)\n" >&2; \
		false; \
	} )

endif

# update all feeds, re-create index files, install symlinks
package/symlinks:
	./scripts/feeds update -a
	./scripts/feeds install -a

# re-create index files, install symlinks
package/symlinks-install:
	./scripts/feeds update -i
	./scripts/feeds install -a

# remove all symlinks, don't touch ./feeds
package/symlinks-clean:
	./scripts/feeds uninstall -a

help:
	cat ./build/README

docs docs/compile: FORCE
	@$(_SINGLE)$(SUBMAKE) -C docs compile

docs/clean: FORCE
	@$(_SINGLE)$(SUBMAKE) -C docs clean

distclean:
	rm -rf .config* feeds key-build* logs package/feeds package/openwrt-packages out tmp
	@$(_SINGLE)$(SUBMAKE) -C scripts/config clean

ifeq ($(findstring v,$(DEBUG)),)
  .SILENT: symlinkclean clean dirclean distclean config-clean download help tmpinfo-clean .config scripts/config/mconf scripts/config/conf menuconfig out/host/.prereq-build tmp/.prereq-package prepare-tmpinfo
endif
.PHONY: help FORCE
.NOTPARALLEL:

