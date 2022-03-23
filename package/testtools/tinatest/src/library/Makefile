TARGET := libtinatest.so

SUBLIBS := $(shell ls | sed 's/Makefile//g')

SCRIPTS := $(basename $(shell find . -type f -regex '.*/scripts/.*'))

.PHONY: all $(SUBLIBS) $(SCRIPTS)

all: $(TARGET) $(SCRIPTS)

INCLUDES := $(addprefix -I../, $(SUBLIBS))

$(SUBLIBS):
	@echo -e "\033[32mBuilding Library: $@\033[0m"
	@$(MAKE) -C $(BUILD_DIR)/src/library/$@ \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS) -c -Wall -fPIC $(INCLUDES)" \
		BUILD_DIR="$(BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all

$(TARGET): $(SUBLIBS)
	@echo -e "\033[32mMerging Library: $@\033[0m"
	@$(CC) -fPIC -shared $(shell find . -name "*.o") -o $(PREFIX)/lib/$@

$(SCRIPTS): $(TARGET)
	@echo -e "\033[32mBuilding Scripts Library : $(notdir $@)\033[0m"
	@$(MAKE) -C $(BUILD_DIR)/src/library/$(dir $(shell dirname $@)) \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(TTLIB)" \
		PREFIX="$(PREFIX)" \
		$(notdir $@)
