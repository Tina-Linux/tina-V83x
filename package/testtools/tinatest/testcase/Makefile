TESTCASES_ENABLE := $(shell \
	for one in `find . -name "private.conf" | xargs -I {} dirname {} | sed 's/^\.//g'`; \
	do \
		echo $${one} | sed '{s/[a-z]/\u&/g; s/[^ [:alnum:]]/_/g}' \
		| xargs -I {} grep "CONFIG_TINATEST{}_ENABLE" $(BUILD_DIR)/.config 1>/dev/null 2>&1 \
			&& echo $${one}; \
	done \
)

PREFIX := $(BUILD_DIR)/testcase/bin
PREFIX_DATA := $(BUILD_DIR)/testcase/data

.PHONY: all

all: preinit $(TESTCASES_ENABLE)

define BuildTestcase

$(1):
ifneq ($$(wildcard .$(1)/Makefile),)
	@$(MAKE) -C .$(1) \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" \
		PREFIX="$(PREFIX)" \
		PREFIX_DATA="$(PREFIX_DATA)" \
		all
else
	@echo -e "\033[32mBuilding $(1)\033[0m"
	@for one in `ls .$(1)`; \
		do \
			case $$$${one} in \
				private.conf) \
					;; \
				*.c) \
					! $(CC) $(CFLAGS) $(LDFLAGS) \
					.$(1)/$$$${one} -o $(PREFIX)/`echo $$$${one%.*}` \
					&& echo -e "\033[31mAuto-Build-[$(1)]: $$$${one} Failed\033[0m" && exit 1; \
					;; \
				*.cpp) \
					! $(CXX) $(CFLAGS) $(LDFLAGS) \
					.$(1)/$$$${one} -o $(PREFIX)/`echo $$$${one%.*}` \
					&& echo -e "\033[31mAuto-Build-[$(1)]: $$$${one} Failed\033[0m" && exit 1; \
					;; \
				*.sh) \
					cp .$(1)/$$$${one} $(PREFIX); \
					;; \
				*) \
					grep "^DATA *=" .$(1)/private.conf | grep -Fw "$$$${one}" >/dev/null \
					&& cp .$(1)/$$$${one} $(PREFIX_DATA); \
					;; \
			esac; \
			continue; \
		done
endif

endef

$(foreach one,$(TESTCASES_ENABLE),$(eval $(call BuildTestcase,$(one))))

preinit:
	@echo -e "\033[33m******** BUILD TESTCASES ********\033[0m"
	@rm -rf $(PREFIX) $(PREFIX_DATA)
	@mkdir -p $(PREFIX) $(PREFIX_DATA)
