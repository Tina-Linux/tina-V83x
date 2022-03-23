
include ../../config.mk
include ../../../Makefile.inc

################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(CFLAGS) $(LOCAL_CFLAGS)
CONFIG_FOR_LINK =


################################################################################

BuildPath      = ./build
ObjectPath     = $(BuildPath)/obj
OutputPath     = ./
DependFilePath = $(BuildPath)/dep
Target         = $(OutputPath)/demoPlayer   ## output target.

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
                    -I../../ \
                    -I../../DEMUX/BASE/include/ \
                    -I../../DEMUX/STREAM/include/ \
                    -I../../DEMUX/PARSER/include/ \
                    -I../../PLAYER/include \
					-I../../libcedarc/include \
                    -I../../EXTERNAL/include

## set compile flags
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -Wall -ldl

## set link flags
$(warning $(CONFIG_PRODUCT))
LoadFlags = $(CONFIG_FOR_LINK) -lplayer -lcdc_vdecoder -lcdc_adecoder -lcdc_sdecoder -lcdc_base \
             -lcdx_parser -lcdx_stream -lcdx_base -lcdc_ve -lcdc_memory \
               -lpthread -ldl -lstdc++ -lrt -lc

ifeq ($(CONFIG_HAVE_ZLIB), $(OPTION_HAVE_ZLIB))
LoadFlags += -lz
endif

ifeq ($(CONFIG_HAVE_SSL), $(OPTION_HAVE_SSL))
LoadFlags += -lcrypto -lssl
endif

LoadFlags += -L../../DEMUX/BASE/ \
             -L../../DEMUX/STREAM/ \
             -L../../DEMUX/PARSER/ \
             -L../../PLAYER/ \


ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
LoadFlags += -L../../EXTERNAL/lib32/lgnueabihf/
LoadFlags +=-L../../libcedarc/library/lib32/lgnueabihf
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
LoadFlags += -L../../EXTERNAL/lib32/lgnueabi/
LoadFlags +=-L../../libcedarc/library/lib32/lgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
#LoadFlags += -L/home/xuqi/release/lib
LoadFlags += -L../../EXTERNAL/lib32/uclgnueabi
LoadFlags += -L../../libcedarc/library/lib32/uclgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/linuxgnueabi
LoadFlags +=-L../../libcedarc/library/lib32/linuxgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI))
    ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
    LoadFlags +=-L../../libcedarc/library/lib32/muslgnueabi_linux310
    else
    LoadFlags +=-L../../libcedarc/library/lib32/muslgnueabi
    endif

    LoadFlags +=-L../../EXTERNAL/lib32/muslgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
    LoadFlags +=-L../../libcedarc/library/lib64/muslgnueabi_64_linux440
    LoadFlags +=-L../../EXTERNAL/lib64/muslgnueabi64
endif

ifeq ($(CONFIG_HAVE_LIVE555), $(OPTION_HAVE_LIVE555))
LoadFlags += -llive555
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
#$(ObjectPath)/%.o:%.cpp
#	$(CPP) $(CompileFlags) -o $@ -c $<
$(ObjectFiles):$(ObjectPath)/%.o:%.cpp
	@$(CPP) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

## link object files to the target share library.
$(Target):$(ObjectFiles)
#	echo $(LoadFlags)
	@$(CPP) -o $@ $^ $(LoadFlags)
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
