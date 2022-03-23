
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
Target         = $(OutputPath)/libtplayer.so   ## output target.

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
ifeq ($(ONLY_ENABLE_AUDIO),y)
SourceFiles  = ./tplayer.c ./awsink/tsound_ctrl.c
else
SourceFiles  = $(foreach dir,$(SourcePath),$(shell find $(dir) -maxdepth 1 -name "*.c"))
endif

## set the object files.
ObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(SourceFiles)))))

## set the dependency files.
DependFiles = $(addprefix $(DependFilePath)/, $(addsuffix .d ,$(notdir $(basename $(SourceFiles)))))


################################################################################
## set flags for compile and link
################################################################################

## set the include path for compile flags.
ifeq ($(ONLY_ENABLE_AUDIO),y)
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
				-I ../ \
			        -I../libcedarx/ \
			        -I../libcedarx/external/include/adecoder \
			        -I../libcedarx/libcore/playback/include \
			        -I../libcedarx/libcore/common/iniparser\
			        -I../libcedarx/libcore/parser/include/ \
			        -I../libcedarx/libcore/stream/include/ \
			        -I../libcedarx/libcore/base/include/ \
			        -I../libcedarx/xplayer/include
else
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
				-I ../ \
				-I../libcedarc/include \
			        -I../libcedarc/vdecoder/include \
			        -I../libcedarx/ \
			        -I../libcedarx/external/include/adecoder \
			        -I../libcedarx/external/include/sdecoder \
			        -I../libcedarx/libcore/playback/include \
			        -I../libcedarx/libcore/common/iniparser\
			        -I../libcedarx/libcore/parser/include/ \
			        -I../libcedarx/libcore/stream/include/ \
			        -I../libcedarx/libcore/base/include/ \
			        -I../libcedarx/xplayer/include
endif

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

## set compile flags
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -Wall -O2 -ldl
ifeq ($(ONLY_ENABLE_AUDIO),y)
LocalLibPath = -L../libcedarx/xplayer/.libs/ -L../libcedarx/libcore/base/.libs/ -L../libcedarx/libcore/parser/base/.libs/\
			-L../libcedarx/libcore/playback/.libs/ -L../libcedarx/libcore/stream/base/.libs/ -L../libcedarx/libcore/common/.libs/ \
			-L$(CEDARX_LIBS_PATH)
else
LocalLibPath = -L../libcedarx/xplayer/.libs/ -L../libcedarx/libcore/base/.libs/ -L../libcedarx/libcore/parser/base/.libs/\
			-L../libcedarx/libcore/playback/.libs/ -L../libcedarx/libcore/stream/base/.libs/ -L../libcedarx/libcore/common/.libs/ \
			-L$(CEDARX_LIBS_PATH) \
			-L../libcedarc/vdecoder/.libs/ -L../libcedarc/memory/.libs/ -L../libcedarc/base/.libs/ -L$(CEDARC_LIBS_PATH)
endif

ifeq ($(ONLY_ENABLE_AUDIO),y)
LocalLinkFlags = -lxplayer -ladecoder \
               -lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base \
              -lpthread -ldl  -lrt -lm -lc -lz -lasound
else
LocalLinkFlags =  -lxplayer -lvdecoder -ladecoder -lsubdecoder -lcdc_base -lVE -lMemAdapter \
               -lcdx_parser  -lcdx_playback -lcdx_stream -lcdx_base \
              -lpthread -ldl -luapi -lrt -lm -lc -lz -lasound
endif

ifeq ($(ONLY_ENABLE_AUDIO),y)
CompileFlags += -DONLY_ENABLE_AUDIO
endif
##set link flag path
ifeq ($(BOARD_PLATFORM),$(filter $(BOARD_PLATFORM),r18 r30 mr813))
    ifeq ($(C_LIB_TYPE),musl)
        ifeq ($(KERNEL64_USER32),y)
            LocalAudioGain += ./arm-musl/libAudioGain.a
        else
            LocalAudioGain += ./aarch64-musl/libAudioGain.a
        endif
    endif
    ifeq ($(C_LIB_TYPE),glibc)
        ifeq ($(KERNEL64_USER32),y)
            LocalAudioGain += ./arm-glibc/libAudioGain.a
        else
            LocalAudioGain += ./aarch64-glibc/libAudioGain.a
        endif
    endif
endif

ifeq ($(BOARD_PLATFORM),$(filter $(BOARD_PLATFORM),r16 r58 r40 r311 r11 r7 r333 r331 r7s g102 t7 r332 mr133 r328s2 r328s3))
    ifeq ($(C_LIB_TYPE),musl)
        LocalAudioGain += ./arm-musl/libAudioGain.a
    endif
    ifeq ($(C_LIB_TYPE),glibc)
        LocalAudioGain += ./arm-glibc/libAudioGain.a
    endif
endif

ifeq ($(BOARD_PLATFORM),$(filter $(BOARD_PLATFORM),r6 c200s))
    ifeq ($(C_LIB_TYPE),musl)
        LocalAudioGain += ./arm9-musl/libAudioGain.a
    endif
    ifeq ($(C_LIB_TYPE),glibc)
        LocalAudioGain += ./arm9-glibc/libAudioGain.a
    endif
endif

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
	$(CC) $(CompileFlags) -o $@ -c $< -fPIC -shared

## link object files to the target share library.
$(Target):$(ObjectFiles)
	$(CC) -o $@ $^ -fPIC -shared $(LDFLAGS) $(LocalAudioGain) $(LocalLibPath) $(LocalLinkFlags)

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
