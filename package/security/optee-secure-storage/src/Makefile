O:= $(CURDIR)/out

TA_DIR := ta
NA_DIR := na
DEV_KIT_DIR ?= $(CURDIR)/../../dev_kit

export DEV_KIT_DIR

ifneq ($V,1)
	q := @
	echo := @echo
else
	q :=
	echo := @:
endif
export q

.PHONY: all
all: ta na demo

.PHONY: demo
demo:na
	$(q)$(MAKE) demo -C $(NA_DIR) CROSS_COMPILE="$(CROSS_COMPILE_HOST)" O=$(O)/na

.PHONY: na
na:
	$(q)$(MAKE) na -C $(NA_DIR) CROSS_COMPILE="$(CROSS_COMPILE_HOST)" O=$(O)/na

.PHONY: ta
ta:
	$(q)$(foreach dir,$(TA_DIR), $(MAKE) -C $(dir) CROSS_COMPILE="$(CROSS_COMPILE_TA)" O=$(O)/$(dir) &&) true

.PHONY: clean
clean:
	$(q)$(foreach dir,$(TA_DIR), $(MAKE) -C $(dir) O=$(O)/$(dir) $@ &&) true
	$(q)$(MAKE) -C $(NA_DIR) O=$(O)/na $@
	$(q)rm -rf $(O)/na
	$(q)rm -rf $(O)
