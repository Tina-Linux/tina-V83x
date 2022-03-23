TARGET := tinatest
.PHONY: all build-library $(TARGET)

$(TARGET): build-library
	@echo -e "\033[33m******** BUILD CORE ********\033[0m"
	@$(MAKE) -C $(BUILD_DIR)/src/core \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS) $(TTLIB)" \
		BUILD_DIR="$(BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all

all: build-library $(TARGET)

build-library:
	@echo -e "\033[33m******** BUILD LIBRARY ********\033[0m"
	@$(MAKE) -C $(BUILD_DIR)/src/library \
		ARCH="$(ARCH)" \
		AR="$(AR)" \
		CC="$(CC)" \
		CXX="$(CXX)" \
		CFLAGS="$(CFLAGS)" \
		LDFLAGS="$(LDFLAGS)" \
		BUILD_DIR="$(BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all
