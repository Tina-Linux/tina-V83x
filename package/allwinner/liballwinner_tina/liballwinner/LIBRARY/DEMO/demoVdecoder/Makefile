
include ../../config.mk
include ../../../Makefile.inc
################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(LOCAL_CFLAGS)
CONFIG_FOR_LINK =

###############################################################################
###############################################################################
BuildPath      = ./build
OutputPath     = ./
CurrentPath     = ./
ObjCommonPath  = ./build/obj
DepCommonPath  = ./build/dep
Target         = $(OutputPath)/demoVdecoder   ## output target.

SourceFiles    := demoVdecoder.c

ObjectFiles	:= $(patsubst %.c,$(ObjCommonPath)/%.o,$(SourceFiles))
ObjectPath := $(dir $(ObjectFiles))
DependFiles	:= $(patsubst %.c,$(ObjCommonPath)/%.d,$(SourceFiles))
DependFilePath := $(dir $(DependFiles))

DemoVdecoderInstallPath := ./app
DemoVdecoderLibPath := $(DemoVdecoderInstallPath)/lib

ifneq ($(BuildPath),wildcard($(BuildPath)))
a := $(shell mkdir -p $(BuildPath))
endif
ifneq ($(ObjectPath),wildcard($(ObjectPath)))
a := $(shell mkdir -p $(ObjectPath))
endif
ifneq ($(DependFilePath),wildcard($(DependFilePath)))
a := $(shell mkdir -p $(DependFilePath))
endif

SourceIncludePath :=	-I../../DEMUX/BASE/include/ \
	                    -I../../DEMUX/STREAM/include/ \
	                    -I../../DEMUX/PARSER/include/ \
						-I../../libcedarc/include \
	                    -I../../


CompileFlags := $(CONFIG_FOR_COMPILE) $(SourceIncludePath) -O2  -Wall -fPIC -ldl

LoadFlags := -L../../DEMUX/BASE/ \
             -L../../DEMUX/STREAM/ \
             -L../../DEMUX/PARSER/ \
             -L../.././PLUGIN/manager/ \
             -L../../PLAYER/

LoadFlags += -lcdc_vdecoder -lcdc_ve -lcdc_memory -lcdc_base -lcdx_base -lcdx_parser -lcdx_stream -lpthread

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

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
LoadFlags += -L../../EXTERNAL/lib32/uclgnueabi/
LoadFlags += -L../../libcedarc/library/lib32/uclgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCLIBC_ARM926))
LoadFlags += -L../../EXTERNAL/lib32/uclibc_arm926
LoadFlags +=-L../../libcedarc/library/lib32/uclibc-arm926
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

ifeq ($(CONFIG_HAVE_ZLIB), $(OPTION_HAVE_ZLIB))
    LoadFlags += -lz
endif

ifeq ($(CONFIG_HAVE_SSL), $(OPTION_HAVE_SSL))
LoadFlags += -lcrypto -lssl
endif

ifeq ($(CONFIG_HAVE_LIVE555), $(OPTION_HAVE_LIVE555))
LoadFlags += -llive555
endif

LoadFlags += -Wl,--no-undefined

###############################################################################
###############################################################################
.PHONY: all clean cleanall
all:$(Target)

clean:
	-rm -rf $(ObjCommonPath)/*
	-rm -f $(Target)

cleanall: clean
	-rm -rf $(DepCommonPath)/*
	-rm -rf $(DemoVdecoderInstallPath)
	-rm -rf $(BuildPath)


###############################################################################
###############################################################################
$(ObjectFiles):$(ObjCommonPath)/%.o:%.c
	@$(CC) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

## link object files to the target share library.
$(Target):$(ObjectFiles)
	@$(CC) -o $@ $^ $(LoadFlags)
	@echo "LD    $@"

## set rules to generate .d files.
$(DepCommonPath)/%.d:%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CompileFlags) $< > $@.$$$$; \
	sed 's,.*\.o[:]*,$(ObjCommonPath)\/$*\.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
	@echo "updating    $@"


###############################################################################
###############################################################################
	#copy libraries
	#-mkdir -p install/lib/
	#-cp DEMUX/BASE/libcdx_base.so install/lib/
install:

ifneq ($(DemoVdecoderInstallPath),wildcard($(DemoVdecoderInstallPath)))
	$(shell mkdir -p $(DemoVdecoderInstallPath))
endif
ifneq ($(DemoVdecoderLibPath),wildcard($(DemoVdecoderLibPath)))
	$(shell mkdir -p $(DemoVdecoderLibPath))
endif
	-rm -f ${DemoVdecoderLibPath}/*

	-cp ../../VE/*.so ${DemoVdecoderLibPath}/
	-cp ../../MEMORY/*.so ${DemoVdecoderLibPath}/
	-cp ../../CODEC/VIDEO/DECODER/*.so ${DemoVdecoderLibPath}/
	-cp ../../DEMUX/STREAM/*.so ${DemoVdecoderLibPath}/
	-cp ../../DEMUX/PARSER/*.so ${DemoVdecoderLibPath}/
	-cp ../../DEMUX/BASE/*.so ${DemoVdecoderLibPath}/
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
	-cp ../../EXTERNAL/lib32/lgnueabihf/*.so ${DemoVdecoderLibPath}/
endif
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
	-cp ../../EXTERNAL/lib32/lgnueabi/*.so ${DemoVdecoderLibPath}/
endif
	-cd ${DemoVdecoderLibPath};ln -s libxml2.so libxml2.so.2;cd -
	-chmod a+x ${DemoVdecoderLibPath}/*.so
	-cp $(Target) $(DemoVdecoderInstallPath)/
	-cp $(CurrentPath)/doc/readme.txt $(DemoVdecoderInstallPath)/

## include the .d files to set dependency rules.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(DependFiles)
endif
endif
