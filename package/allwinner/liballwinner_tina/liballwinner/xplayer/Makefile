

include ../LIBRARY/config.mk
include ../Makefile.inc

################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(CFLAGS) $(LOCAL_CFLAGS) -Werror
CONFIG_FOR_LINK =


################################################################################

BuildPath      = ./build
ObjectPath     = $(BuildPath)/obj
OutputPath     = ./
DependFilePath = $(BuildPath)/dep
Target         = $(OutputPath)/libxplayer.so   ## output target.

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
SvnPath = $(shell find ./ -type d | grep ".svn")
SourcePath := $(filter-out $(SvnPath) $(BuildPath) $(ObjectPath) $(DependFilePath), $(SourcePath))
VPATH := $(SourcePath)

## set the source files.
SourceFiles  = $(foreach dir,$(SourcePath),$(shell find $(dir) -maxdepth 1 -name "*.cpp"))

## set the object files.
ObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(SourceFiles)))))

## set the dependency files.
DependFiles = $(addprefix $(DependFilePath)/, $(addsuffix .d ,$(notdir $(basename $(SourceFiles)))))

################################################################################
## set flags for compile and link
################################################################################

## set the include path for compile flags.
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
                    -I../LIBRARY/ \
                    -I../LIBRARY/DEMUX/BASE/include/ \
                    -I../LIBRARY/DEMUX/STREAM/include/ \
                    -I../LIBRARY/DEMUX/PARSER/include/ \
                    -I../LIBRARY/libcedarc/include  \
                    -I../LIBRARY/PLAYER/include/ \
                    -I../LIBRARY/PLUGIN/include/ \

## set compile flags
ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -O1 -fPIC -ldl
else
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -mfpu=neon -O1 -fPIC -ldl
endif

## set link flags
LoadFlags = $(CONFIG_FOR_LINK) -ldl -shared

LoadFlags +=  -lplayer -lcdc_vdecoder -lcdc_adecoder -lcdc_sdecoder \
             -lcdx_parser -lcdx_stream -lcdx_base -lcdc_ve -lcdc_memory \
             -lpthread -ldl -lstdc++

ifeq ($(CONFIG_HAVE_SSL), $(OPTION_HAVE_SSL))
LoadFlags += -lcrypto -lssl
endif

LoadFlags += -L../LIBRARY/DEMUX/BASE/ \
             -L../LIBRARY/DEMUX/STREAM/ \
             -L../LIBRARY/DEMUX/PARSER/ \
             -L../LIBRARY/PLAYER/ \
             -L../LIBRARY/PLUGIN/manager \

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
    LoadFlags +=-L../LIBRARY/libcedarc/library/lib32/lgnueabihf_linux310
    else
    LoadFlags +=-L../LIBRARY/libcedarc/library/lib32/lgnueabihf
    endif
LoadFlags += -L../LIBRARY/EXTERNAL/lib32/lgnueabihf

endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
    LoadFlags +=-L../LIBRARY/libcedarc/library/lib32/lgnueabi_linux310
    else
    LoadFlags +=-L../LIBRARY/libcedarc/library/lib32/lgnueabi
    endif
LoadFlags += -L../LIBRARY/EXTERNAL/lib32/lgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCLIBC_ARM926))
LoadFlags += -L../LIBRARY/EXTERNAL/lib32/uclibc_arm926
LoadFlags +=-L../LIBRARY/libcedarc/library/lib32/uclibc-arm926
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
LoadFlags += -L../LIBRARY/EXTERNAL/lib32/uclgnueabi
LoadFlags +=-L../LIBRARY/libcedarc/library/lib32/uclgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
LoadFlags +=-L../LIBRARY/EXTERNAL/lib32/linuxgnueabi
LoadFlags +=-L../LIBRARY/libcedarc/library/lib32/linuxgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI))
    ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
    LoadFlags +=-L../LIBRARY/libcedarc/library/lib32/muslgnueabi_linux310
    else
    LoadFlags +=-L../LIBRARY/libcedarc/library/lib32/muslgnueabi
    endif

    LoadFlags +=-L../LIBRARY/EXTERNAL/lib32/muslgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
    LoadFlags +=-L../LIBRARY/libcedarc/library/lib64/muslgnueabi_64_linux440
    LoadFlags +=-L../LIBRARY/EXTERNAL/lib64/muslgnueabi64
endif

################################################################################
## make commands, all/clean/cleanall
################################################################################

## define commands for make, sush as all, clean
.PHONY: all clean cleantarget cleanall
all:$(Target)

clean:
	-rm -f $(ObjectPath)/*
	-rm -rf $(Target)

cleanall: clean
	-rm -f $(DependFilePath)/*
	-rm -rf $(BuildPath)


################################################################################
## define target dependencies.
################################################################################

## compile source files to object files.
#$(ObjectPath)/%.o:%.cpp
#	$(CPP) $(CompileFlags) -o $@ -c $<
$(ObjectFiles):$(ObjectPath)/%.o:%.cpp
	@$(CPP) $(CompileFlags) -o $@ -c $<
	@echo "CC   $<"

## link object files to the target share library.
$(Target):$(ObjectFiles)
	@$(LD) -o $@ $^ $(LoadFlags)
	@echo "LD    $@"

## set rules to generate .d files.
$(DependFilePath)/%.d:%.cpp
	@set -e; rm -f $@; \
	$(CPP) -MM $(CompileFlags) $< > $@.$$$$; \
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
