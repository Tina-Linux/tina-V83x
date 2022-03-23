

include ../../config.mk
include ../../../Makefile.inc

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
Target         = $(OutputPath)/encodedemo   ## output target.

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
SourceFiles  = $(foreach dir,$(SourcePath),$(shell find $(dir) -maxdepth 1 -name "*.c"))

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
		   -I../../DEMUX/MUXER/include/ \
                    -I../../DEMUX/STREAM/include/ \
                    -I../../DEMUX/PARSER/include/ \
                    -I../../libcedarc/include  \
                    -I../../PLAYER/include/ \
                    -I../../PLUGIN/include/ \

## set compile flags
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -Wall -O2 -ldl

## set link flags
LoadFlags = $(CONFIG_FOR_LINK)

LoadFlags +=   -lcdc_vencoder  -lcdc_base\
               -lcdc_ve -lcdc_memory \
               -lpthread -ldl -lstdc++ -lrt

LoadFlags += -L../../DEMUX/BASE/ \
             -L../../DEMUX/STREAM/ \
             -L../../DEMUX/PARSER/ \
             -L../../PLAYER/ \
             -L../../PLUGIN/manager \

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
	ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
	LoadFlags += -L../../libcedarc/library/lib32/lgnueabihf_linux310
	else
	LoadFlags += -L../../libcedarc/library/lib32/lgnueabihf
	endif
LoadFlags += -L../../EXTERNAL/lib32/lgnueabihf/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
	ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
	LoadFlags += -L../../libcedarc/library/lib32/lgnueabi_linux310
	else
	LoadFlags += -L../../libcedarc/library/lib32/lgnueabi
	endif
LoadFlags += -L../../EXTERNAL/lib32/lgnueabi/
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCLIBC_ARM926))
LoadFlags += -L../../EXTERNAL/lib32/uclibc_arm926
LoadFlags +=-L../../libcedarc/library/lib32/uclibc-arm926
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
LoadFlags += -L../../EXTERNAL/lib32/uclgnueabi/
LoadFlags += -L../../libcedarc/library/lib32/uclgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/linuxgnueabi
LoadFlags += -L../../libcedarc/library/lib32/linuxgnueabi
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
$(ObjectFiles):$(ObjectPath)/%.o:%.c
	@$(CC) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

## link object files to the target share library.
$(Target):$(ObjectFiles)
	@$(CC) -o $@ $^ $(LoadFlags)
	@echo "LD    $@"

## set rules to generate .d files.
$(DependFilePath)/%.d:%.c
	@set -e; rm -f $@; \
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
