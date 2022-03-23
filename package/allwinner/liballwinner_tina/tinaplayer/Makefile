
include ../LIBRARY/config.mk
include ../Makefile.inc

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
Target         = $(OutputPath)/libtinaplayer.so   ## output target.

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
					-I../LIBRARY/DEMUX/MUXER/include/ \
                    -I../LIBRARY/DEMUX/STREAM/include/ \
                    -I../LIBRARY/DEMUX/PARSER/include/ \
                    -I../LIBRARY/libcedarc/include \
                    -I../LIBRARY/PLAYER/include/ \
                    -I../LIBRARY/PLUGIN/include/ \
					-I../xplayer

## set compile flags
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -Wall -O2 -ldl

##set link flag path
ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
LocalAudioGain += ./MUSLGNUEABI64/libAudioGain.a
else
LocalAudioGain += ./MUSLGNUEABI/libAudioGain.a
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
#	$(CPP) $(CompileFlags) -o $@ -c $< -fPIC -shared
$(ObjectFiles):$(ObjectPath)/%.o:%.cpp
	$(CPP) $(CompileFlags) -o $@ -c $< -fPIC -shared

## link object files to the target share library.
$(Target):$(ObjectFiles)
	$(CPP) -o $@ $^ -fPIC -shared $(LDFLAGS) $(LocalAudioGain)

## set rules to generate .d files.
$(DependFilePath)/%.d:%.cpp
	set -e; rm -f $@; \
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
