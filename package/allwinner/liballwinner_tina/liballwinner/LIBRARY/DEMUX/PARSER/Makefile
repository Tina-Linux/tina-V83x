

include ../../config.mk
include ../../../Makefile.inc

################################################################################
## set flags for golobal compile and link setting.
################################################################################

CONFIG_FOR_COMPILE = $(CFLAGS) $(LOCAL_CFLAGS)
CONFIG_FOR_LINK =


################################################################################

BuildPath      = ./build
ObjCommonPath     = $(BuildPath)/obj
OutputPath     = ./
DepCommonPath = $(BuildPath)/dep
Target         = $(OutputPath)/libcdx_parser.so   ## output target.

################################################################################
## set the source files, object files and dependency files
################################################################################
## set the source path to VPATH.
SourcePath =    ./                 \
                ./asf              \
                ./mpg              \
                ./include          \
                ./id3              \
                ./aac              \
                ./base             \
                ./amr              \
                ./ts               \
                ./AwSpecialStream  \
                ./mkv              \
                ./ape              \
                ./mov              \
                ./wav              \
                ./flac             \
                ./flv              \
                ./mp3              \
                ./caf              \
                ./ogg              \
                ./avi              \
				./playlist

ifeq ($(CONFIG_HAVE_SSL), $(OPTION_HAVE_SSL))
SourcePath += ./hls
endif

SvnPath = $(shell find ./ -type d | grep ".svn")
EnvPath = $(shell find ./ -type d | grep "env")
SstrPath = $(shell find ./ -type d | grep "sstr")
DashPath = $(shell find ./ -type d | grep "dash")
AviPath = $(shell find ./ -type d | grep "avi")
AwRawStreamPath = $(shell find ./ -type d | grep "AwRawStream")
AwSpecialStreamPath = $(shell find ./ -type d | grep "AwSpecialStream")
AwtsPath = $(shell find ./ -type d | grep "awts")
BdPath = $(shell find ./ -type d | grep "bd")
FlvPath = $(shell find ./ -type d | grep "flv")
HlsPath = $(shell find ./ -type d | grep "hls")
M3u9Path = $(shell find ./ -type d | grep "m3u9")
MkvPath = $(shell find ./ -type d | grep "mkv")
MmsPath = $(shell find ./ -type d | grep "mms")
MmshttpPath = $(shell find ./ -type d | grep "mmshttp")
MovPath = $(shell find ./ -type d | grep "mov")
MpgPath = $(shell find ./ -type d | grep "mpg")
PlaylistPath = $(shell find ./ -type d | grep "playlist")
PmpPath = $(shell find ./ -type d | grep "pmp")
RemuxPath = $(shell find ./ -type d | grep "remux")
TsPath = $(shell find ./ -type d | grep "ts")
WvmPath = $(shell find ./ -type d | grep "wvm")
MpgPath = $(shell find ./ -type d | grep "mpg")
CafPath = $(shell find ./ -type d | grep "caf")
ApePath = $(shell find ./ -type d | grep "ape")
FlacPath = $(shell find ./ -type d | grep "flac")
AmrPath = $(shell find ./ -type d | grep "amr")
OggPath = $(shell find ./ -type d | grep "ogg")
AsfPath = $(shell find ./ -type d | grep "asf")

ifeq ($(CONFIG_PRODUCT),$(OPTION_PRODUCT_LOUDSPEAKER))
SourcePath := $(filter-out $(SvnPath) $(EnvPath) $(SstrPath) $(DashPath) $(AviPath) $(AwRawStreamPath) $(AwSpecialStreamPath) $(AwtsPath)  $(BdPath) $(FlvPath) \
                       $(M3u9Path)  $(MkvPath) $(MmsPath)  $(MpgPath)  $(PlaylistPath) \
                       $(PmpPath) $(RemuxPath)  $(TsPath)  $(WvmPath) $(BuildPath), $(SourcePath))
else
SourcePath := $(filter-out $(SvnPath) $(EnvPath) $(SstrPath) $(DashPath) $(BuildPath), $(SourcePath))
endif

ifeq ($(CONFIG_HAVE_LIVE555), $(OPTION_HAVE_LIVE555))
SourcePath +=       ./remux
endif

ifeq ($(CONFIG_MMS), $(OPTION_MMS_ENABLE))
SourcePath +=       ./mms \
                    ./mmshttp
endif

VPATH := $(SourcePath)

## set the source files.
SourceFiles  = $(foreach dirTmp,$(SourcePath),$(shell find $(dirTmp) -maxdepth 1 -name "*.c"))

## set the object files.
ObjectFiles = $(patsubst ./%.c, $(ObjCommonPath)/%.o ,$(SourceFiles))
ObjectPath := $(dir $(ObjectFiles))

## set the dependency files.
DependFiles = $(patsubst ./%.c, $(DepCommonPath)/%.d ,$(SourceFiles))
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
                    -I../BASE/include \
                    -I../STREAM/include \
                    -I../STREAM/rtsp \
                    -I../../          \
					-I../../libcedarc/include/ \
                    -I../../PLAYER/include/ \
                    -I../../EXTERNAL/include/zlib/ \
                    -I../../EXTERNAL/include/openssl/ \
                    -I../../EXTERNAL/include/libxml \
                    -I../../EXTERNAL/include/zlib  \
                    -I../../EXTERNAL/include

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
#LoadFlags = $(CONFIG_FOR_LINK) -lGetAudio_format -ldl -shared
LoadFlags = $(CONFIG_FOR_LINK) -ldl -shared -lpthread

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
LoadFlags += -L../../EXTERNAL/lib32/lgnueabihf
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/lgnueabi
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCLIBC_ARM926))
LoadFlags += -L../../EXTERNAL/lib32/uclibc_arm926
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_UCGNUEABI))
LoadFlags +=-L../../EXTERNAL/lib32/uclgnueabi
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

LoadFlags += -L../BASE/
LoadFlags += -lcdx_base

#LoadFlags += -L../STREAM/
#LoadFlags += -lcdx_stream
#LoadFlags += -Wl,--no-undefined

ifeq ($(CONFIG_HAVE_ZLIB), $(OPTION_HAVE_ZLIB))
LoadFlags += -lz
endif

ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABIHF))
LoadFlags += -L../../CODEC/AUDIO/DECODER/libs_for_linux/build-by-arm-linux-gnueabihf/static
endif
ifeq ($(CONFIG_CC),$(OPTION_CC_GNUEABI))
LoadFlags += -L../../CODEC/AUDIO/DECODER/libs_for_linux/build-by-arm-none-linux-gnueabi/static
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
#$(ObjectPath)/%.o:%.c
#	$(CC) $(CompileFlags) -o $@ -c $<
$(ObjectFiles):$(ObjCommonPath)/%.o:%.c
	@$(CC) $(CompileFlags) -o $@ -c $<
	@echo "CC    $<"

## link object files to the target share library.
$(Target):$(ObjectFiles)
	@$(CC) -o $@ $^ $(LoadFlags)
	@echo "LD    $@"

## set rules to generate .d files.
$(DependFiles):$(DepCommonPath)/%.d:%.c
	@set -e; rm -f $@; \
	$(CC) -MM $(CompileFlags) $< > $@.$$$$; \
	sed 's,.*\.o[:]*,$(ObjCommonPath)\/$*\.o $@: ,g' < $@.$$$$ > $@; \
	rm -f $@.$$$$
#	@echo "updating    $@"

## include the .d files to set dependency rules.
ifneq ($(MAKECMDGOALS),clean)
ifneq ($(MAKECMDGOALS),cleantarget)
ifneq ($(MAKECMDGOALS),cleanall)
-include $(DependFiles)
endif
endif
endif
