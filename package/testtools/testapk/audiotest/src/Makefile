EXCLUDE_DIRS := local

SUB_DIRS := $(shell find . -maxdepth 1 -type d)
SUB_DIRS := $(basename $(patsubst ./%,%,$(SUB_DIRS)))
SUB_DIRS := $(filter-out $(EXCLUDE_DIRS),$(SUB_DIRS))

# to avoid that the target names of "clean" operation are the same as
# $(SUB_DIRS), add prefix to CLEAN_DIRS
CLEAN_DIRS := $(addprefix _clean_,$(SUB_DIRS))

all: $(SUB_DIRS)

$(SUB_DIRS):
	$(MAKE) -C $@ \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		CXXFLAGS="$(CXXFLAGS)" \
		LDFLAGS="$(LDFLAGS)" \
		INSTALL_PREFIX="$(INSTALL_PREFIX)" \
		all

$(CLEAN_DIRS):
	$(MAKE) -C $(patsubst _clean_%,%,$@) clean

clean: $(CLEAN_DIRS)

.PHONY: all clean $(SUB_DIRS) $(CLEAN_DIRS)
