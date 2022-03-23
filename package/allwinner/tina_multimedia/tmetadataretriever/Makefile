
################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(CFLAGS) $(LOCAL_CFLAGS)
CONFIG_FOR_LINK = -Wl,--no-undefined


################################################################################

BuildPath      = ./build
ObjectPath     = $(BuildPath)/obj
OutputPath     = ./
DependFilePath = $(BuildPath)/dep
Target         = $(OutputPath)/libtmetadataretriever.so   ## output target.

ifneq ($(BuildPath),wildcard($(BuildPath)))
a := $(shell mkdir -p $(BuildPath))
endif
ifneq ($(ObjectPath),wildcard($(ObjectPath)))
a := $(shell mkdir -p $(ObjectPath))
endif
ifneq ($(OutputPath),wildcard($(OutputPath)))
a := $(shell mkdir -p $(OutputPath))
endif
ifneq ($(DependFilePath),wildcard($(DependFilePath)))
a := $(shell mkdir -p $(DependFilePath))
endif


################################################################################
## set the source files, object files and dependency files
################################################################################
## set the source path to VPATH.
SourcePath = $(shell find ./ -type d)
SourcePath := $(filter-out $(BuildPath) $(ObjectPath) $(DependFilePath), $(SourcePath))
VPATH := $(SourcePath)

## set the source files.
SourceFiles  = $(foreach dir,$(SourcePath),$(shell find $(dir) -maxdepth 1 -name "*.c"))

## set the object files.
ObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(SourceFiles)))))

## set the dependency files.
DependFiles = $(addprefix $(DependFilePath)/, $(addsuffix .d ,$(notdir $(basename $(SourceFiles)))))

#config the arm libs path
ifeq ($(CPU_TYPE),cortex-a7)
    ifeq ($(C_LIB_TYPE),musl)
        CEDARC_LIBS_PATH = ../libcedarc/library/toolchain-sunxi-arm-musl
        CEDARX_LIBS_PATH = ../libcedarx/external/lib32/openwrt-arm-musl
    endif
    ifeq ($(C_LIB_TYPE),glibc)
        CEDARC_LIBS_PATH = ../libcedarc/library/toolchain-sunxi-arm-glibc
        CEDARX_LIBS_PATH = ../libcedarx/external/lib32/openwrt-arm-glibc
    endif
endif

#config the aarch64 libs path
ifeq ($(CPU_TYPE),cortex-a53)
    ifeq ($(KERNEL64_USER32),y)
        ifeq ($(C_LIB_TYPE),musl)
            CEDARC_LIBS_PATH = ../libcedarc/library/toolchain-sunxi-arm-musl
            CEDARX_LIBS_PATH = ../libcedarx/external/lib32/openwrt-arm-musl
        endif
        ifeq ($(C_LIB_TYPE),glibc)
            CEDARC_LIBS_PATH = ../libcedarc/library/toolchain-sunxi-arm-glibc
            CEDARX_LIBS_PATH = ../libcedarx/external/lib32/openwrt-arm-glibc
        endif
    else
        ifeq ($(C_LIB_TYPE),musl)
            CEDARC_LIBS_PATH = ../libcedarc/library/toolchain-sunxi-aarch64-musl
            CEDARX_LIBS_PATH = ../libcedarx/external/lib32/openwrt-aarch64-musl
        endif
        ifeq ($(C_LIB_TYPE),glibc)
            CEDARC_LIBS_PATH = ../libcedarc/library/toolchain-sunxi-aarch64-glibc
            CEDARX_LIBS_PATH = ../libcedarx/external/lib32/openwrt-aarch64-glibc
        endif
    endif
endif

#config the arm9 libs path
ifeq ($(CPU_TYPE),arm926ej-s)
    ifeq ($(C_LIB_TYPE),musl)
	CEDARC_LIBS_PATH = ../libcedarc/library/toolchain-sunxi-arm9-musl
        CEDARX_LIBS_PATH = ../libcedarx/external/lib32/openwrt-arm926-musl
    endif
    ifeq ($(C_LIB_TYPE),glibc)
        CEDARC_LIBS_PATH = ../libcedarc/library/toolchain-sunxi-arm9-glibc
        CEDARX_LIBS_PATH = ../libcedarx/external/lib32/openwrt-arm926-glibc
    endif
endif

LocalLibPath = -L../libcedarx/libcore/base/.libs/ -L../libcedarx/libcore/parser/base/.libs/\
			-L../libcedarx/libcore/stream/base/.libs/  -L../libcedarx/libcore/common/.libs/ -L$(CEDARX_LIBS_PATH) \
			-L../libcedarc/vdecoder/.libs/ -L../libcedarc/memory/.libs/ -L../libcedarc/base/.libs/ -L$(CEDARC_LIBS_PATH)

LocalLinkFlags =  -lcdx_parser -lcdx_base -lcdx_stream -lcdx_common -lvdecoder -ladecoder -lVE \
		-lMemAdapter -lcdc_base -ldl -lm
################################################################################
## set flags for compile and link
################################################################################

## set the include path for compile flags.
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
				-I../ \
				-I../tplayer \
				-I../libcedarc/include \
				-I../libcedarx/libcore/base/include \
				-I../libcedarx/libcore/stream/include \
				-I../libcedarx/libcore/parser/include \
				-I../libcedarx/libcore/common/iniparser \
				-I../libcedarx/external/include/adecoder \
## set compile flags
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -Wall -O2 -ldl

################################################################################
## make commands, all/clean/cleanall
################################################################################

## define commands for make, sush as all, clean
.PHONY: all clean cleantarget cleanall
all:$(Target)

clean:
	-rm -f $(ObjectPath)/*
	-rm -f $(Target)

cleanall: clean
	-rm -f $(DependFilePath)/*
	-rm -rf $(BuildPath)


################################################################################
## define target dependencies.
################################################################################

## compile source files to object files.
#$(ObjectPath)/%.o:%.c
#	$(CC) $(CompileFlags) -o $@ -c $< -fPIC -shared
$(ObjectFiles):$(ObjectPath)/%.o:%.c
	@echo =======$(CONFIG_FOR_COMPILE)===
	@echo =======CPP:$(CPP)====
	@echo =======CXX:$(CXX)====
	$(CC) $(CompileFlags) -o $@ -c $< -fPIC -shared

## link object files to the target share library.
$(Target):$(ObjectFiles)
	$(CC) -o $@ $^ -fPIC -shared $(LDFLAGS) $(LocalLibPath) $(LocalLinkFlags)

## set rules to generate .d files.
$(DependFilePath)/%.d:%.c
	set -e; rm -f $@; \
	$(CC) -MM $(CompileFlags) $< > $@.$$$$; \
	sed 's,\($*\)\.o[:]*,$(ObjectPath)/\1.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$

## include the .d files to set dependency rules.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleantarget)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(DependFiles)
endif
endif
endif
