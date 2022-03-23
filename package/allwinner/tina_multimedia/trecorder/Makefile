
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
Target         = $(OutputPath)/libtrecorder.so   ## output target.

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


################################################################################
## set flags for compile and link
################################################################################

## set the include path for compile flags.
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
				-I../ \
				-I../tplayer \
				-I../libcedarc/include \
				-I../libcedarx/libcore/base/include \
				-I../libcedarx/libcore/muxer/include \
				-I../libcedarx/external/include/aencoder \
				-I../libcedarx/awrecorder

## set decoder offset
VE_PHY_OFFSET = 0

ifeq ($(BOARD_PLATFORM),$(filter $(BOARD_PLATFORM),r16 r40 r18))
	VE_PHY_OFFSET = 0x40000000
endif

## set compile flags
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -DCONF_VE_PHY_OFFSET=$(VE_PHY_OFFSET) -Wall -O2 -ldl

ifeq ($(BOARD_PLATFORM),$(filter $(BOARD_PLATFORM),r11,r7))
	CompileFlags += -DDETECT_SD_CARD
endif

ifeq ($(USE_VIN_ISP),y)
	CompileFlags += -D__USE_VIN_ISP__

	LIBS += -lisp -lisp_ini -lAWIspApi
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
	@echo =======$(CONFIG_FOR_COMPILE)===
	@echo =======CPP:$(CPP)====
	@echo =======CXX:$(CXX)====
	$(CC) $(CompileFlags) -o $@ -c $< -fPIC -shared

## link object files to the target share library.
$(Target):$(ObjectFiles)
	$(CC) -o $@ $^ -fPIC -shared $(LDFLAGS) $(LIBS)

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
