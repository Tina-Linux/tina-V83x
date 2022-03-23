out-dir := $(CURDIR)/out

DEV_KIT_DIR ?= $(CURDIR)/../../dev_kit
#include $(DEV_KIT_DIR)/../platform_config.mk
CA_DEV_KIT_DIR ?= $(DEV_KIT_DIR)/arm-plat-$(PLATFORM)/export-ca
TA_DEV_KIT_DIR ?= $(DEV_KIT_DIR)/arm-plat-$(PLATFORM)/export-ta_arm32

ifeq ($(ATF_EXIST),y)
CROSS_COMPILE_HOST ?= $(DEV_KIT_DIR)/../tools/toolchain/gcc-aarch64/bin/aarch64-linux-gnu-
CROSS_COMPILE_TA   ?= $(DEV_KIT_DIR)/../tools/toolchain/gcc-arm-gnueabi/bin/arm-linux-gnueabi-
else
CROSS_COMPILE_HOST ?= $(DEV_KIT_DIR)/../tools/toolchain/gcc-arm-gnueabi/bin/arm-linux-gnueabi-
CROSS_COMPILE_TA   ?= $(DEV_KIT_DIR)/../tools/toolchain/gcc-arm-gnueabi/bin/arm-linux-gnueabi-
endif

export CA_DEV_KIT_DIR
export TA_DEV_KIT_DIR

ifneq ($V,1)
	q := @
	echo := @echo
else
	q :=
	echo := @:
endif
# export 'q', used by sub-makefiles.
export q

# If _HOST or _TA specific compilers are not specified, then use CROSS_COMPILE
CROSS_COMPILE_HOST ?= $(CROSS_COMPILE)
CROSS_COMPILE_TA ?= $(CROSS_COMPILE)

CFLAGS+=$(TA_CFLAGS)
LDFLAGS+=$(TA_LDFLAGS)

.PHONY: all
ifneq ($(wildcard $(TA_DEV_KIT_DIR)/host_include/conf.mk),)
all: na ta
else
all:
	$(q)echo "TA_DEV_KIT_DIR is not correctly defined" && false
endif

.PHONY: na
na:
	$(q)$(MAKE) -C na CROSS_COMPILE="$(CROSS_COMPILE_HOST)" \
			     --no-builtin-variables \
			     O=$(out-dir)/na

.PHONY: ta
ta:
	$(q)$(MAKE) -C ta CROSS_COMPILE="$(CROSS_COMPILE_TA)" \
			     O=$(out-dir)/ta

.PHONY: clean
RMDIR := rmdir --ignore-fail-on-non-empty
define rm-build-subdir
echo `[ -d $1 ] && { cd $1; $(RMDIR) $(out-dir)/ta $(out-dir) 2> /dev/null; true; }` > /dev/null
endef

ifneq ($(wildcard $(TA_DEV_KIT_DIR)/host_include/conf.mk),)
clean:
	-$(q)$(MAKE) -C na O=$(out-dir)/na $@
	-$(q)$(MAKE) -C ta O=$(out-dir)/ta $@
	-$(q)$(call rm-build-subdir,ta)
else
clean:
	$(q)echo "TA_DEV_KIT_DIR is not correctly defined"
	$(q)echo "You can remove manually $(out-dir)"
endif
