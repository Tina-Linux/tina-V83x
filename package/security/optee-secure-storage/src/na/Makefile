# Normally this makefile shouldn't be called directly and we expect the output
# path to be on a certain location to fit together with the other OP-TEE
# gits and helper scripts.
ifeq ($O,)
$(error output path should be specified when calling this makefile)
endif

#include $(DEV_KIT_DIR)/../platform_config.mk
#ifeq ($(ATF_EXIST),y)
#CROSS_COMPILE = $(DEV_KIT_DIR)/../tools/toolchain/gcc-aarch64/bin/aarch64-linux-gnu-
#else
#CROSS_COMPILE = $(DEV_KIT_DIR)/../tools/toolchain/gcc-arm-gnueabi/bin/arm-linux-gnueabi-
#endif

# By default we expect optee_client exported folder to be on a certain relative
# path, but if the client specifies the OPTEE_CLIENT_EXPORT then that path will
# be used instead.
OPTEE_CLIENT_EXPORT ?= $(DEV_KIT_DIR)/arm-plat-$(PLATFORM)/export-ca

CC		= $(CROSS_COMPILE)gcc
CPP		= $(CROSS_COMPILE)cpp
LD		= $(CROSS_COMPILE)ld
AR		= $(CROSS_COMPILE)ar
NM		= $(CROSS_COMPILE)nm
OBJCOPY		= $(CROSS_COMPILE)objcopy
OBJDUMP		= $(CROSS_COMPILE)objdump
READELF		= $(CROSS_COMPILE)readelf

#LIB here is the NA
LIB_OUT_DIR = $(O)/lib
LIB_CFLAGS = $(CFLAGS)
LIB_CFLAGS += -I./
LIB_CFLAGS += -I./../ta/include
LIB_CFLAGS += -I$(OPTEE_CLIENT_EXPORT)/include

LIB_LDFLAGS = $(LDFLAGS)
LIB_LDFLAGS += -L$(OPTEE_CLIENT_EXPORT)/exportlib -lteec

ifneq ($(DYNAMIC_LIBSTORAGE),1)
LIB_NAME = libstorage.a
LIB_CFLAGS += -c -o $(LIB_OUT_DIR)/libstorage.o
else
LIB_NAME = libstorage.so
LIB_CFLAGS += --no-builtin-variables
LIB_CFLAGS += -fPIC
LIB_CFLAGS += -o $(LIB_OUT_DIR)/$(LIB_NAME)
LIB_LDFLAGS += -lpthread -lm
LIB_LDFLAGS += -Wl,-soname,libstorage.so
LIB_LDFLAGS += -shared
endif

demo_srcs := demo.c
demo_objs := $(patsubst %.c,%.o,$(demo_srcs))

DEMO_OUT_DIR = $(O)/demo
DEMO_CFLAGS = $(CFLAGS)
DEMO_CFLAGS += -I$(OPTEE_CLIENT_EXPORT)/include
DEMO_CFLAGS += -I./../ta/include
DEMO_LDFLAGS = $(LDFLAGS)
DEMO_LDFLAGS += -L$(OPTEE_CLIENT_EXPORT)/exportlib -lteec
.PHONY: all
all: na demo

.PHONY:na
na:libstorage

libstorage:
	mkdir -p $(LIB_OUT_DIR)
	$(q)$(CC) $(LIB_CFLAGS) $(LIB_LDFLAGS)  libstorage.c
ifneq ($(DYNAMIC_LIBSTORAGE),1)
	$(q)$(AR) -crv $(LIB_OUT_DIR)/$(LIB_NAME) $(LIB_OUT_DIR)/libstorage.o
endif

.PHONY:demo
demo: $(DEMO_OUT_DIR)/$(demo_objs) $(LIB_OUT_DIR)/$(LIB_NAME)
	mkdir -p $(DEMO_OUT_DIR)
	$(q)$(CC) -o $(DEMO_OUT_DIR)/$@ $+ $(DEMO_LDFLAGS)

$(DEMO_OUT_DIR)/$(demo_objs): $(demo_srcs)
	mkdir -p $(DEMO_OUT_DIR)
	$(q)$(CC) $(DEMO_CFLAGS) -c $< -o $@

.PHONY:clean
clean:
	@echo '  CLEAN $(O)'
	$(q)rm -f $(LIB_OUT_DIR)/libstorage
	$(q)rm -f $(DEMO_OUT_DIR)/demo
	$(q)$(foreach obj,$(demo_objs), rm -f $(DEMO_OUT_DIR)/$(obj))
	$(q)rm -rf $(LIB_OUT_DIR); true
	$(q)rm -rf $(DEMO_OUT_DIR); true
