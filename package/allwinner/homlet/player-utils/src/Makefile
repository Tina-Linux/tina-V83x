#export STAGING_DIR=${TINA_BUILD_TOP}/out/${TARGET_BOARD}/staging_dir/target
#CC = ${TINA_BUILD_TOP}/prebuilt/gcc/linux-x86/arm/toolchain-sunxi-glibc-gcc-531/toolchain/bin/arm-openwrt-linux-gnueabi-gcc

CDC_DIR = ${TINA_BUILD_TOP}/package/allwinner/tina_multimedia/libcedarc
CDX_DIR = ${TINA_BUILD_TOP}/package/allwinner/tina_multimedia/libcedarx

Target = libplayer_utils.so

SourceFiles = \
    $(wildcard ./*.cpp) \
    $(wildcard ./*.c)

SourceIncludePath = \
    -I.

#SourceIncludePath += \
#    -I$(STAGING_DIR)/usr/include/allwinner/include \
#    -I$(STAGING_DIR)/usr/include/allwinner

SourceIncludePath += \
    -I$(CDX_DIR) \
    -I$(CDX_DIR)/xplayer/include \
    -I$(CDX_DIR)/libcore/base/include \
    -I$(CDX_DIR)/libcore/stream/include \
    -I$(CDX_DIR)/libcore/parser/include \
    -I$(CDX_DIR)/libcore/playback/include \
    -I$(CDX_DIR)/libcore/common/iniparser \
    -I$(CDX_DIR)/external/include/adecoder \
    -I$(CDX_DIR)/external/include \
    -I$(CDC_DIR)/include

CompileFlags = $(CPPFLAGS) $(SourceIncludePath)
LoadFlags = \
    -lpthread \
    -lcdx_base -lcdx_stream -lcdx_parser -lcdx_playback -lxplayer -lcdx_common \
    -lcdc_base -lMemAdapter -lvdecoder \
    -lvideoengine -lVE \
    -ladecoder -lsubdecoder -lssl -lcrypto -lz -lasound \
    -L./arm-glibc -lAudioGain \
    -luci

.PHONY : all clean

all :
	$(CC) -fPIC -shared -o $(Target) $(SourceFiles) $(CompileFlags) $(LDFLAGS) $(LoadFlags)
	-mkdir -p $(INSTALL_PREFIX)/usr/lib
	cp -fp $(Target) $(INSTALL_PREFIX)/usr/lib
	-mkdir -p $(INSTALL_PREFIX)/etc/config
	cp -fp ./etc/config/volume $(INSTALL_PREFIX)/etc/config

clean :
	$(RM) $(Target)