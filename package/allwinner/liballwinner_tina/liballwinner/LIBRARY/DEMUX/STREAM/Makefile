

include ../../config.mk
include ../../../Makefile.inc


################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(CFLAGS) $(LOCAL_CFLAGS) -Werror
CONFIG_FOR_LINK = -Wl,--no-undefined


################################################################################

BuildPath      = ./build
ObjectPath     = $(BuildPath)/obj
OutputPath     = ./
DependFilePath = $(BuildPath)/dep
Target         = $(OutputPath)/libcdx_stream.so   ## output target.

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
## set the source files, object files and dependency files
################################################################################
## set the source path to VPATH.
SourcePath =       \
            ./include \
            ./base \
            ./tcp  \
            ./file \
            ./udp  \
            ./http \

ifeq ($(CONFIG_HAVE_LIVE555), $(OPTION_HAVE_LIVE555))
SourcePath +=       ./rtsp \
					./rtmp
endif

ifeq ($(CONFIG_HAVE_SSL), $(OPTION_HAVE_SSL))
SourcePath +=       ./ssl
endif

ifeq ($(CONFIG_MMS), $(OPTION_MMS_ENABLE))
SourcePath +=       ./mms
endif

VPATH := $(SourcePath)
# set the source files.
SourceFiles  = $(foreach dir,$(SourcePath),$(shell find $(dir) -maxdepth 1 -name "*.c"))
CppSourceFiles  = $(foreach dir,$(SourcePath),$(shell find $(dir) -maxdepth 1 -name "*.cpp"))

## set the object files.
ObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(SourceFiles)))))
CppObjectFiles = $(addprefix $(ObjectPath)/, $(addsuffix .o ,$(basename $(notdir $(CppSourceFiles)))))

## set the dependency files.
DependFiles = $(addprefix $(DependFilePath)/, $(addsuffix .d ,$(notdir $(basename $(SourceFiles)))))
CppDependFiles = $(addprefix $(DependFilePath)/, $(addsuffix .d ,$(notdir $(basename $(CppSourceFiles)))))


################################################################################
## set flags for compile and link
################################################################################

## set the include path for compile flags.
SourceIncludePath = $(foreach dir,$(SourcePath),-I$(dir)) \
                    -I../BASE/include \
                    -I../../          \
                    -I../PARSER/include \
                    -I../../libcedarc/include \
                    -I../../EXTERNAL/include/zlib/ \
                    -I../../EXTERNAL/include/openssl/ \
                    -I../../EXTERNAL/include/libxml \
                    -I../../EXTERNAL/include/zlib  \
                    -I../../EXTERNAL/include/live  \
                    -I../../EXTERNAL/include


ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
SourceIncludePath += -I../../EXTERNAL/build-by-arm-linux-gnueabihf/openssl/include/
endif
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
SourceIncludePath += -I../../EXTERNAL/build-by-arm-none-linux-gnueabi/openssl/include/
endif

## set compile flags
ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -O1 -fPIC -ldl
else
CompileFlags = $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -mfpu=neon -O1 -fPIC -ldl
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
#CompileFlags += -mfloat-abi=softfp -mfpu=neon
endif

## set link flags
LoadFlags = $(CONFIG_FOR_LINK) -ldl -shared -lpthread -lrt

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
LoadFlags += -L../../EXTERNAL/lib32/lgnueabihf
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/lgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/uclgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCLIBC_ARM926))
LoadFlags += -L../../EXTERNAL/lib32/uclibc_arm926
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_UCGNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/linuxgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/muslgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_LINUX_MUSLGNUEABI64))
LoadFlags +=-L../../EXTERNAL/lib64/muslgnueabi64
endif

ifeq ($(CONFIG_HAVE_ZLIB), $(OPTION_HAVE_ZLIB))
LoadFlags += -lz
endif

ifeq ($(CONFIG_HAVE_LIVE555), $(OPTION_HAVE_LIVE555))
LoadFlags += -llive555
endif

ifeq ($(CONFIG_HAVE_SSL), $(OPTION_HAVE_SSL))
LoadFlags += -lssl -lcrypto
endif

LoadFlags += -L../BASE/
LoadFlags += -lcdx_base

#LoadFlags += -L../PARSER/
#LoadFlags += -lcdx_parser
#LoadFlags += -Wl,--no-undefined

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
$(ObjectFiles):$(ObjectPath)/%.o:%.c
	@$(CC) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

$(CppObjectFiles):$(ObjectPath)/%.o:%.cpp
	@$(CPP) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

## link object files to the target share library.
$(Target):$(ObjectFiles) $(CppObjectFiles)
	@$(CPP) -o $@ $^ $(LoadFlags)
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
