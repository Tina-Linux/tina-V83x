#include ./LIBRARY/config.mk

#PATH_TOOLCHAIN=$(shell pwd)/../doninux/rootfs/toolchain/arm-20100950
#PATH_TOOLCHAIN=/home/bzchen/a10s_dongle/doninux/rootfs/toolchain/arm-20100950

#CC   = $(PATH_TOOLCHAIN)/bin/arm-none-linux-gnueabi-gcc
#CPP  = $(PATH_TOOLCHAIN)/bin/arm-none-linux-gnueabi-g++
#STRIP= $(PATH_TOOLCHAIN)/bin/arm-none-linux-gnueabi-strip
#AR   = $(PATH_TOOLCHAIN)/bin/arm-none-linux-gnueabi-ar

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
# set compile tool chains to be arm-linux-gnueabihf- set.
CC    = arm-linux-gnueabihf-gcc
LD    = arm-linux-gnueabihf-gcc
CPP   = arm-linux-gnueabihf-g++
STRIP = arm-linux-gnueabihf-strip
AR    = arm-linux-gnueabihf-ar
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
# set compile tool chains to be arm-none-linux-gnueabi- set.
CC    = @echo " CC  $<";arm-none-linux-gnueabi-gcc
LD    = @echo " LD  $@";arm-none-linux-gnueabi-gcc
CPP   = @echo " CPP  $<";arm-none-linux-gnueabi-g++
STRIP = @echo " STRIP  $<";arm-none-linux-gnueabi-strip
AR    = @echo " LD  $@";arm-none-linux-gnueabi-ar

#CFLAGS = -isystem /arm/arm-2010.09/arm-none-linux-gnueabi/libc/usr/include
endif
CFLAGS += -D_GNU_SOURCE

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
# set compile tool chains to be arm-none-linux-gnueabi- set.
CC    = arm-openwrt-linux-uclibcgnueabi-gcc
LD    = arm-openwrt-linux-uclibcgnueabi-gcc
CPP   = arm-openwrt-linux-uclibcgnueabi-g++
STRIP = arm-openwrt-linux-uclibcgnueabi-strip
AR    = arm-openwrt-linux-uclibcgnueabi-ar
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
# set compile tool chains to be arm-none-linux-gnueabi- set.
#PATH_TOOLCHAIN=/home1/liweihai/a20/MerriiLinux/out/linux/common/buildroot/external-toolchain/bin
CC    = arm-linux-gnueabi-gcc
LD    = arm-linux-gnueabi-gcc
CPP   = arm-linux-gnueabi-g++
STRIP = arm-linux-gnueabi-strip
AR    = arm-linux-gnueabi-ar
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI))
CFLAGS  =-Os -pipe -march=armv7-a -mtune=cortex-a7 -mfpu=neon -fno-caller-saves \
			-fno-plt -fhonour-copts -Wno-error=unused-but-set-variable \
			-Wno-error=unused-result -mfloat-abi=hard \
			 -Wformat -Werror=format-security -fstack-protector -D_FORTIFY_SOURCE=1 \
			 -Wl,-z,now -Wl,-z,relro
CC    = arm-openwrt-linux-muslgnueabi-gcc
LD    = arm-openwrt-linux-muslgnueabi-gcc
CPP   = arm-openwrt-linux-muslgnueabi-g++
STRIP = arm-openwrt-linux-muslgnueabi-strip
AR    = arm-openwrt-linux-muslgnueabi-ar
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
CC    = aarch64-openwrt-linux-musl-gcc
LD    = aarch64-openwrt-linux-musl-gcc
CPP   = aarch64-openwrt-linux-musl-g++
STRIP = aarch64-openwrt-linux-musl-strip
AR    = aarch64-openwrt-linux-musl-ar
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCLIBC_ARM926))
CC    = arm-buildroot-linux-uclibcgnueabi-gcc
LD    = arm-buildroot-linux-uclibcgnueabi-gcc
CPP   = arm-buildroot-linux-uclibcgnueabi-g++
STRIP = arm-buildroot-linux-uclibcgnueabi-strip
AR    = arm-buildroot-linux-uclibcgnueabi-ar
endif
