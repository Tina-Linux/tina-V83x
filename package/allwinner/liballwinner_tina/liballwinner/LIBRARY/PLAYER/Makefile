
include ../config.mk
include ../../Makefile.inc


################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(CFLAGS) $(LOCAL_CFLAGS)
CONFIG_FOR_LINK = -Wl,--no-undefined

################################################################################

BuildPath      = ./build
ObjCommonPath     = $(BuildPath)/obj
OutputPath     = ./
DepCommonPath = $(BuildPath)/dep
Target         = $(OutputPath)/libplayer.so   ## output target.

################################################################################
## set the source files, object files and dependency files
################################################################################
## set the source path to VPATH.
SourcePath = $(shell find ./ -type d)
SvnPath = $(shell find ./ -type d | grep ".svn")
SourcePath := $(filter-out $(SvnPath) $(BuildPath) $(ObjectPath) $(DependFilePath), $(SourcePath))
VPATH := $(SourcePath)

## set the source files.
SourceFiles  = \
    audioDecComponent.cpp                 \
    audioRenderComponent.cpp              \
    videoDecComponent.cpp                 \
    videoRenderComponent.cpp              \
    subtitleDecComponent.cpp              \
    subtitleRenderComponent.cpp           \
    avtimer.cpp                           \
    messageQueue.cpp                      \
    bitrateEstimater.cpp                  \
    framerateEstimater.cpp                \
    streamManager.cpp                     \
    player.cpp                            \
    deinterlace.cpp                      \

SourceFiles  +=    soundControl/soundControl_null.cpp \
	           layerControl/layerControl_null.cpp

## set the object files.
ObjectFiles = $(patsubst %.cpp, $(ObjCommonPath)/%.o ,$(SourceFiles))
ObjectPath := $(dir $(ObjectFiles))

## set the dependency files.
DependFiles = $(patsubst %.cpp, $(DepCommonPath)/%.d ,$(SourceFiles))
DependFilePath := $(dir $(DependFiles))

ifneq ($(BuildPath),wildcard($(BuildPath)))
a := $(shell mkdir -p $(BuildPath))
endif
ifneq ($(ObjectPath),wildcard($(ObjectPath)))
a := $(shell mkdir -p $(ObjectPath))
endif
ifneq ($(DependFilePath),wildcard($(DependFilePath)))
a := $(shell mkdir -p $(DependFilePath))
endif
################################################################################
## set flags for compile and link
################################################################################
## set the include path for compile flags.
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
					-I./	\
                    -I../          \
                    -I./include/   \
                    -I./layerControl/1623 \
		            -I../EXTERNAL/include/ \
					-I../libcedarc/include/ \


## set compile flags
ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -O1 -fPIC -ldldif
else
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -mfpu=neon -O1 -fPIC -ldldif
endif

## set link flags
LoadFlags = $(CONFIG_FOR_LINK)  -ldl -shared

LoadFlags +=   -lcdc_adecoder \
             -lcdx_parser -lcdx_stream -lcdx_base \
              -lpthread -ldl -lstdc++ -lrt

LoadFlags += -lcdc_sdecoder -lcdc_vdecoder -lcdc_ve -lcdc_memory

LoadFlags += -L../DEMUX/BASE/ \
             -L../DEMUX/STREAM/ \
             -L../DEMUX/PARSER/ \
             -L../PLUGIN/manager \

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
    ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
    LoadFlags +=-L../libcedarc/library/lib32/lgnueabihf_linux310
    else
    LoadFlags +=-L../libcedarc/library/lib32/lgnueabihf
    endif
    LoadFlags +=-L../EXTERNAL/lib32/lgnueabihf
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCLIBC_ARM926))
LoadFlags += -L../EXTERNAL/lib32/uclibc_arm926
LoadFlags +=-L../libcedarc/library/lib32/uclibc-arm926
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
LoadFlags += -L../EXTERNAL/lib32/lgnueabi

	ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
    LoadFlags +=-L../libcedarc/library/lib32/lgnueabi_linux310
    else
    LoadFlags +=-L../libcedarc/library/lib32/lgnueabi
    endif
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
LoadFlags += -L../EXTERNAL/lib32/uclgnueabi
LoadFlags +=-L../libcedarc/library/lib32/uclgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
LoadFlags +=-L../EXTERNAL/lib32/linuxgnueabi
LoadFlags +=-L../libcedarc/library/lib32/linuxgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI))
    ifeq ($(LINUX_VERSION),$(LINUX_VERSION_3_10))
    LoadFlags +=-L../libcedarc/library/lib32/muslgnueabi_linux310
    else
    LoadFlags +=-L../libcedarc/library/lib32/muslgnueabi
    endif

    LoadFlags +=-L../EXTERNAL/lib32/muslgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
    LoadFlags +=-L../libcedarc/library/lib64/muslgnueabi_64_linux440
    LoadFlags +=-L../EXTERNAL/lib64/muslgnueabi64
endif

################################################################################
## make commands, all/clean/cleanall
################################################################################

## define commands for make, sush as all, clean
.PHONY: all clean cleantarget cleanall
all:$(Target)

clean:
	-rm -rf $(ObjCommonPath)/*
	-rm -f $(Target)

cleanall: clean
	-rm -rf $(DepCommonPath)/*
	-rm -rf $(BuildPath)



################################################################################
## define target dependencies.
################################################################################
## compile source files to object files.
$(ObjectFiles):$(ObjCommonPath)/%.o:%.cpp
	@$(CPP) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

## link object files to the target share library.
$(Target):$(ObjectFiles)
	@$(CPP) -o $@ $^ $(LoadFlags)
	@echo "LD    $@"

## set rules to generate .d files.
$(DepCommonPath)/%.d:%.cpp
	@set -e; rm -f $@; \
	$(CPP) -MM $(CompileFlags) $< > $@.$$$$; \
	sed 's,.*\.o[:]*,$(ObjCommonPath)\/$*\.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	@echo "updating    $@"

## include the .d files to set dependency rules.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleantarget)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(DependFiles)
endif
endif
endif
