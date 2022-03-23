include $(TOPDIR)/rules.mk
include $(BUILD_DIR)/kernel.mk
include $(TOPDIR)/package/allwinner/tina_multimedia/config_parser.mk

PKG_VERSION:=2.8
PKG_RELEASE:=1
PKG_NAME:=libcedarx

PKG_BUILD_DIR := $(COMPILE_DIR)/$(PKG_NAME)

include $(BUILD_DIR)/package.mk

#init the default value
TINA_CHIP_TYPE = G102
KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux
CEDARC_LIBS_PATH = libcedarc/library/toolchain-sunxi-arm-musl
CEDARX_LIBS_PATH = libcedarx/external/lib32/openwrt-arm-musl
VE_OFFSET = NOT_CONFIG_VE_OFFSET
KERNEL_VERSION_ION = CONF_KERNEL_CMA
COMPILE_CEDARX_LEVEL =

CEDARC_LD_FLAGS = -ldl
CEDARC_LD_FLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread)
CEDARX_LD_FLAGS = -ldl -lcrypto -lz
CEDARX_LD_FLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread)
CEDARX_EXTRA_CXXFLAGS =
CEDARC_EXTRA_CXXFLAGS =
CONFIG_PLAYREADY = --enable-playready=no
CEDARX_CONFIGURE_ARGS =

AUDIO_DEC_LIBS := AAC \
			 ALAC \
			 AMR \
			 APE \
			 ATRC \
			 COOK \
			 FLAC \
			 MP3 \
			 OGG \
			 SIPR \
			 WAV \
			 RA \
			 DSD \
			 G729 \
			 OPUS

VIDEO_DEC_LIBS := MPEG2 \
			 H264 \
			 AVS \
			 H265 \
			 MJPEG \
			 MPEG4BASE \
			 MPEG4DX \
			 MPEG4H263 \
			 MPEG4NORMAL \
			 MPEG4VP6 \
			 VP8 \
			 WMV3 \
			 VP9HW

MULTIMEDIA_LIBS := SUPPORT_AUDIO_DEC_LIBS \
				   SUPPORT_VIDEO_DEC_LIBS \
				   SUPPORT_SUBTITLE_DEC_LIBS \
				   SUPPORT_VDEC_COMMON_LIBS \
				   SUPPORT_ADEC_COMMON_LIBS \
				   SUPPORT_TPLAYER_PARSER \
				   SUPPORT_TPLAYER_STREAM \
				   SUPPORT_TPLAYER_COMMON \
				   SUPPORT_VIDEO_ENC_LIBS \
				   SUPPORT_AUDIO_ENC_LIBS \
				   SUPPORT_VENC_COMMON_LIBS \
				   SUPPORT_TRECORDER_MUXER \
				   SUPPORT_TRECORDER_COMMON \
				   SUPPORT_TMETADATA \
				   SUPPORT_JPEG_DECODER \
				   SUPPORT_OMX_LIBS

## TPLAYER related libraries
SUPPORT_AUDIO_DEC_LIBS = $(foreach c,$(AUDIO_DEC_LIBS), \
				$(if $(CONFIG_CEDAR_ADECODER_$(c)),libaw_$(shell echo $(c) | tr '[A-Z]' '[a-z]')dec.so))

SUPPORT_VIDEO_DEC_LIBS = $(foreach c,$(VIDEO_DEC_LIBS), \
				$(if $(CONFIG_CEDAR_VDECODER_$(c)),libaw$(shell echo $(c) | tr '[A-Z]' '[a-z]').so))

SUPPORT_VIDEO_DEC_LIBS += $(if $(CONFIG_CEDAR_VDECODER_MJPEG),libawmjpegplus.so)

SUPPORT_VIDEO_DEC_LIBS += $(if $(CONFIG_CEDAR_VDECODER_VP9),libawvp9Hw.so)

SUPPORT_SUBTITLE_DEC_LIBS := $(if $(CONFIG_CEDAR_SUBTITLE_SUPPORT),libsubdecoder.so)

SUPPORT_VDEC_COMMON_LIBS := $(if $(CONFIG_TPLAYER),libcdc_base.so libMemAdapter.so libvdecoder.so libVE.so libvideoengine.so)

SUPPORT_OMX_LIBS := $(if $(CONFIG_OMX),libOmxCore.so libOmxVdec.so libOmxVenc.so)

SUPPORT_ADEC_COMMON_LIBS := $(if $(CONFIG_TPLAYER),libadecoder.so)

SUPPORT_TPLAYER_PARSER := $(if $(CONFIG_CEDAR_PARSER_ALL),libcdx_parser.so)

SUPPORT_TPLAYER_STREAM := $(if $(CONFIG_CEDAR_STREAM_ALL),libcdx_stream.so)

SUPPORT_TPLAYER_COMMON := $(if $(CONFIG_TPLAYER),libcdx_base.so libcdx_common.so libcdx_playback.so libxplayer.so libtplayer.so)

## TRECORDER related libraries
SUPPORT_VIDEO_ENC_LIBS := $(if $(CONFIG_CEDAR_VIDEO_ENCODER_ALL),libvencoder.so libvenc_base.so libvenc_codec.so)

SUPPORT_AUDIO_ENC_LIBS := $(if $(CONFIG_CEDAR_AENCODER_ALL),libaencoder.so)

SUPPORT_VENC_COMMON_LIBS := $(if $(CONFIG_TRECORDER), libcdc_base.so libMemAdapter.so libVE.so)

SUPPORT_AENC_COMMON_LIBS :=

SUPPORT_TRECORDER_MUXER := $(if $(CONFIG_CEDAR_MUXER_ALL),libcdx_muxer.so)

SUPPORT_TRECORDER_COMMON := $(if $(CONFIG_TRECORDER),libawrecorder.so libcdx_base.so libcdx_common.so libtrecorder.so)

## TMETA related libraries
SUPPORT_TMETADATA := $(if $(CONFIG_TMETADATARETRIVER),libxmetadataretriever.so libtmetadataretriever.so)

## TJPEGDECODER related libraries
SUPPORT_JPEG_DECODER := $(if $(CONFIG_JPEGDECODER),libjpegdecode.so)

SUPPORT_LIBRARY_ALL := $(foreach c,$(MULTIMEDIA_LIBS), $($c))

#config the kernel version,which depends on $(KERNEL_PATCHVER)
ifeq ($(KERNEL_PATCHVER),3.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
endif

ifeq ($(KERNEL_PATCHVER),3.10)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
endif

ifeq ($(KERNEL_PATCHVER),4.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_4
endif

ifeq ($(KERNEL_PATCHVER),4.9)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_9
endif
#end config the kernel version

#config the toolchain,which depends on arm or arm64
#notice:arm musl,arm glibc,arm9 musl,arm9 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"arm")
    CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
    CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux
endif
#notice:aarch64 musl,aarch64 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"aarch64")
    CEDARC_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
    CEDARX_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
endif
#end config the toolchain

#config the cma type
ifeq ($(CONFIG_SUNXI_ALLOC_CMA),y)
    KERNEL_VERSION_ION = CONF_KERNEL_CMA
endif
ifeq ($(CONFIG_SUNXI_ALLOC_IOMMU),y)
    KERNEL_VERSION_ION = CONF_KERNEL_IOMMU
endif

#config the arm libs path
ifeq ($(TARGET_CPU_VARIANT),cortex-a7)
    ifeq ($(CONFIG_LIBC),"musl")
        CEDARC_LIBS_PATH = libcedarc/library/toolchain-sunxi-arm-musl
        CEDARX_LIBS_PATH = libcedarx/external/lib32/openwrt-arm-musl
        COMPILE_CEDARX_LEVEL = -O0
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARC_LIBS_PATH = libcedarc/library/toolchain-sunxi-arm-glibc
        CEDARX_LIBS_PATH = libcedarx/external/lib32/openwrt-arm-glibc
    endif
endif

#config the aarch64 libs path
ifeq ($(TARGET_CPU_VARIANT),cortex-a53)
    ifeq ($(CONFIG_COMPLILE_KERNEL64_USER32),y)
        CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
	CEDARX_COMPILE_TOOL_CHAIN = arm-openwrt-linux
        ifeq ($(CONFIG_LIBC),"musl")
            CEDARC_LIBS_PATH = libcedarc/library/toolchain-sunxi-arm-musl
            CEDARX_LIBS_PATH = libcedarx/external/lib32/openwrt-arm-musl
            COMPILE_CEDARX_LEVEL = -O0
        endif
        ifeq ($(CONFIG_LIBC),"glibc")
            CEDARC_LIBS_PATH = libcedarc/library/toolchain-sunxi-arm-glibc
            CEDARX_LIBS_PATH = libcedarx/external/lib32/openwrt-arm-glibc
        endif
    else
        ifeq ($(CONFIG_LIBC),"musl")
            CEDARC_LIBS_PATH = libcedarc/library/toolchain-sunxi-aarch64-musl
            CEDARX_LIBS_PATH = libcedarx/external/lib32/openwrt-aarch64-musl
            COMPILE_CEDARX_LEVEL = -O0
        endif
        ifeq ($(CONFIG_LIBC),"glibc")
            CEDARC_LIBS_PATH = libcedarc/library/toolchain-sunxi-aarch64-glibc
            CEDARX_LIBS_PATH = libcedarx/external/lib32/openwrt-aarch64-glibc
        endif
    endif
endif

#config the arm9 libs path
ifeq ($(TARGET_CPU_VARIANT),arm926ej-s)
    ifeq ($(CONFIG_LIBC),"musl")
	CEDARC_LIBS_PATH = libcedarc/library/toolchain-sunxi-arm9-musl
        CEDARX_LIBS_PATH = libcedarx/external/lib32/openwrt-arm926-musl
        COMPILE_CEDARX_LEVEL = -O0
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARC_LIBS_PATH = libcedarc/library/toolchain-sunxi-arm9-glibc
        CEDARX_LIBS_PATH = libcedarx/external/lib32/openwrt-arm926-glibc
    endif
endif

ifeq ($(TARGET_BOARD_PLATFORM),r16)
    TINA_CHIP_TYPE = r16
    NOT_SUPPORT_VIDEO_LIBS += libawmjpegplus.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r58)
    TINA_CHIP_TYPE = r58
    NOT_SUPPORT_VIDEO_LIBS += libawmjpegplus.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r40)
    TINA_CHIP_TYPE = r40
    NOT_SUPPORT_VIDEO_LIBS += libawmjpegplus.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r18)
    TINA_CHIP_TYPE = r18
    NOT_SUPPORT_VIDEO_LIBS += libawmjpeg.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r30)
    TINA_CHIP_TYPE = r30
    NOT_SUPPORT_VIDEO_LIBS += libawmjpegplus.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r311)
    TINA_CHIP_TYPE = r311
    VE_OFFSET = CONFIG_VE_OFFSET_0
    NOT_SUPPORT_VIDEO_LIBS += libawmjpegplus.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),mr133)
    TINA_CHIP_TYPE = mr133
    VE_OFFSET = CONFIG_VE_OFFSET_0
    NOT_SUPPORT_VIDEO_LIBS += libawmjpegplus.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r6)
    TINA_CHIP_TYPE = r6
    NOT_SUPPORT_VIDEO_LIBS += libawmjpegplus.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),c600)
    TINA_CHIP_TYPE = c600
    CEDARX_LD_FLAGS = -lz
    NOT_SUPPORT_VIDEO_LIBS += libawmjpegplus.so
endif
ifeq ($(TARGET_BOARD_PLATFORM),c200s)
    TINA_CHIP_TYPE = c200s
    CEDARX_LD_FLAGS = -lz
    NOT_SUPPORT_VIDEO_LIBS += libawmjpegplus.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),g102)
    TINA_CHIP_TYPE = G102
    NOT_SUPPORT_VIDEO_LIBS += libawmjpeg.so \
				   libawmjpegplus.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r11)
    TINA_CHIP_TYPE = R11
    CEDARX_LD_FLAGS = -lz
    NOT_SUPPORT_VIDEO_LIBS += libawmjpeg.so
endif
ifeq ($(TARGET_BOARD_PLATFORM),r7)
    TINA_CHIP_TYPE = r7
    CEDARX_LD_FLAGS = -lz
    NOT_SUPPORT_VIDEO_LIBS += libawmjpeg.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r333)
    TINA_CHIP_TYPE = R333
    CEDARX_LD_FLAGS = -lz
    NOT_SUPPORT_VIDEO_LIBS += libawmjpeg.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r331)
    TINA_CHIP_TYPE = R331
    CEDARX_LD_FLAGS = -lz
    NOT_SUPPORT_VIDEO_LIBS += libawmjpeg.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r7s)
    TINA_CHIP_TYPE = R7s
    CEDARX_LD_FLAGS = -lz
    NOT_SUPPORT_VIDEO_LIBS += libawmjpeg.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r332)
    TINA_CHIP_TYPE = R332
    CEDARX_LD_FLAGS = -lz
    NOT_SUPPORT_VIDEO_LIBS += libawmjpeg.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),t7)
    TINA_CHIP_TYPE = T7
    NOT_SUPPORT_VIDEO_LIBS += libawmjpeg.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),v306)
    TINA_CHIP_TYPE = V3
    CEDARX_LD_FLAGS = -lz
    NOT_SUPPORT_VIDEO_LIBS += libcdc_vd_mjpeg.so
endif

ifeq ($(TARGET_BOARD_PLATFORM),r328s2)
    TINA_CHIP_TYPE = R328
endif

ifeq ($(TARGET_BOARD_PLATFORM),r328s3)
    TINA_CHIP_TYPE = R328
endif

ifeq ($(TARGET_BOARD_PLATFORM),h6)
    CEDARC_EXTRA_CXXFLAGS = -DCONF_SURPPORT_METADATA_BUFFER -DCONF_VE_FREQ_ENABLE_SETUP
    CEDARX_EXTRA_CXXFLAGS = -DCONF_AFBC_ENABLE  -DCONF_H264_4K_P2P -DCONF_H265_4K -DCONF_H265_4K_P2P -DCONF_HIGH_DYNAMIC_RANGE_ENABLE  -DCONF_PRODUCT_STB
    CEDARX_CONFIGURE_ARGS += --enable-xplayerdemo-de2=yes
endif

ifeq ($(CONFIG_CEDAR_FILE_STREAM),y)
	CONFIG_STREAM += --enable-file-stream=yes
else
	CONFIG_STREAM += --enable-file-stream=no
endif

ifeq ($(CONFIG_CEDAR_HTTP_STREAM),y)
	CONFIG_STREAM += --enable-http-stream=yes
else
	CONFIG_STREAM += --enable-http-stream=no
endif

ifeq ($(CONFIG_CEDAR_HTTPS_STREAM),y)
	CONFIG_STREAM += --enable-https-stream=yes
else
	CONFIG_STREAM += --enable-https-stream=no
endif

ifeq ($(CONFIG_CEDAR_RTSP_STREAM),y)
	CONFIG_STREAM += --enable-rtsp-stream=yes
else
	CONFIG_STREAM += --enable-rtsp-stream=no
endif

ifeq ($(CONFIG_CEDAR_AAC_MUXER),y)
	CONFIG_MUXER += --enable-aac-muxer=yes
else
	CONFIG_MUXER += --enable-aac-muxer=no
endif

ifeq ($(CONFIG_CEDAR_TS_MUXER),y)
	CONFIG_MUXER += --enable-ts-muxer=yes
else
	CONFIG_MUXER += --enable-ts-muxer=no
endif

ifeq ($(CONFIG_CEDAR_MP4_MUXER),y)
	CONFIG_MUXER += --enable-mp4-muxer=yes
else
	CONFIG_MUXER += --enable-mp4-muxer=no
endif

ifeq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	CONF_ONLY_ENABLE_AUDIO += --enable-only-audio=yes
else
	CONF_ONLY_ENABLE_AUDIO += --enable-only-audio=no
endif

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),r16 r58 r40 r18 r6 c200s g102 r11 r7 r30 r311 r333 r331 r7s t7 r332 v306 dolphin h3 h6 mr133 r328s2 r328s3 mr813))

define Package/$(PKG_NAME)/config
source "$(SOURCE)/Config.in"
endef

define Package/$(PKG_NAME)
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=$(PKG_NAME)  for allwinner
	DEPENDS:=+libpthread \
	   +(!ONLY_ENABLE_AUDIO):libuapi \
	   +zlib \
	   +alsa-lib \
	   +CEDAR_HTTPS_STREAM:libopenssl \
	   +CEDAR_PARSER_HLS:libopenssl \
	   +CEDAR_RTSP_STREAM:live \
	   +TR_YES_USE_VIN_ISP:libAWIspApi \
	   +CEDARX_YES_USE_PLAYREADY:playready \
	   +OMX:libstdcpp
endef

ifeq ($(CONFIG_PACKAGE_xplayerdemo),y)
    CEDARX_CONFIGURE_ARGS += --enable-xplayerdemo=yes
endif

ifeq ($(CONFIG_CEDARX_YES_USE_PLAYREADY),y)
	CONFIG_PLAYREADY = --enable-playready=yes
	COMPILE_PLAYREADY = \
				-DADJUST_ADDRESS_FOR_SECURE_OS_OPTEE \
				-DBOARD_USE_PLAYREADY=1 \
				-DDRM_BUILD_PROFILE=900 \
				-DARM=1
endif

define Package/$(PKG_NAME)/Default
  TITLE:=$(PKG_NAME) for all
  URL:=http://www.allwinner.com/
endef

define Package/$(PKG_NAME)/description
	$(PKG_NAME)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./tina_multimedia_version.h $(PKG_BUILD_DIR)/
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(CP) -r ./libcedarc $(PKG_BUILD_DIR)/
endif
	$(CP) -r ./libcedarx $(PKG_BUILD_DIR)/
	$(CP) -r ./tplayer $(PKG_BUILD_DIR)/tplayer
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(CP) -r ./trecorder $(PKG_BUILD_DIR)/trecorder
	$(CP) -r ./jpegdecode $(PKG_BUILD_DIR)/jpegdecode
	$(CP) -r ./tmetadataretriever $(PKG_BUILD_DIR)/tmetadataretriever
endif
	$(CP) -r ./tplayer/configs/$(TARGET_BOARD_PLATFORM)_linux_cedarx.conf $(PKG_BUILD_DIR)/
endef

define Build/Configure
	@echo "********target_board_platform = $TARGET_BOARD_PLATFORM***********"
	@echo "********chip type = $(TINA_CHIP_TYPE)*********"
	@echo "********kernel version = $(KERNEL_PATCHVER)*********"
	@echo "********toolchain = $(CONFIG_ARCH)_$(CONFIG_LIBC)*********"
	@echo "********cpu arch = $(TARGET_CPU_VARIANT)*********"
	@echo "********parser: $(CONFIG_PARSER)*********"
	@echo "********stream: $(CONFIG_STREAM)*********"
	@echo "********muxer: $(CONFIG_MUXER)*********"
	@echo "********only_enable_audio: $(CONFIG_ONLY_ENABLE_AUDIO)*********"
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	cd $(PKG_BUILD_DIR)/libcedarc; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/libcedarc/install --host=$(CEDARC_COMPILE_TOOL_CHAIN) CFLAGS="-D__ENABLE_ZLIB__ -D$(VE_OFFSET) -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION)" CPPFLAGS="-D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION)" LDFLAGS="$(CEDARC_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)  -L$(PKG_BUILD_DIR)/libcedarc/install/lib"
endif
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	cd $(PKG_BUILD_DIR)/libcedarx; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/libcedarx/install --host=$(CEDARX_COMPILE_TOOL_CHAIN)  $(CONFIG_PLAYREADY) $(CONFIG_PARSER) $(CONFIG_STREAM) $(CONFIG_MUXER)  CFLAGS="-D__ENABLE_ZLIB__ -D$(TINA_CHIP_TYPE) -Wno-error=format" CPPFLAGS="-D__ENABLE_ZLIB__  -D$(TINA_CHIP_TYPE) -Wno-error=format" LDFLAGS="$(CEDARX_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)  -L$(PKG_BUILD_DIR)/libcedarx/install/lib -L$(PKG_BUILD_DIR)/libcedarc/install/lib -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)" $(CEDARX_CONFIGURE_ARGS)
else
	cd $(PKG_BUILD_DIR)/libcedarx; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/libcedarx/install --host=$(CEDARX_COMPILE_TOOL_CHAIN)  $(CONFIG_PLAYREADY) $(CONFIG_PARSER) $(CONFIG_STREAM) $(CONFIG_MUXER) $(CONF_ONLY_ENABLE_AUDIO) CFLAGS="-D__ENABLE_ZLIB__ -D$(TINA_CHIP_TYPE) -DONLY_ENABLE_AUDIO -Wno-error=format" CPPFLAGS="-D__ENABLE_ZLIB__  -D$(TINA_CHIP_TYPE) -Wno-error=format" LDFLAGS="$(CEDARX_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)  -L$(PKG_BUILD_DIR)/libcedarx/install/lib" $(CEDARX_CONFIGURE_ARGS)
endif

endef

define Build/Compile

ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(MAKE) -C $(PKG_BUILD_DIR)/libcedarc \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -D$(VE_OFFSET) -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) $(CEDARC_EXTRA_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) $(CEDARC_EXTRA_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(CEDARC_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH) -L$(PKG_BUILD_DIR)/libcedarc/install/lib" \
		install
endif

 ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(MAKE) -C $(PKG_BUILD_DIR)/libcedarx \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -D$(TINA_CHIP_TYPE) -D$(KERNEL_VERSION_ION) -Wno-error=format  $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__  -D$(TINA_CHIP_TYPE) -D$(KERNEL_VERSION_ION) -Wno-error=format $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(CEDARX_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)  -L$(PKG_BUILD_DIR)/libcedarx/install/lib -L$(PKG_BUILD_DIR)/libcedarc/install/lib -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)" \
		BOARD_PLATFORM="$(TARGET_BOARD_PLATFORM)" \
		install
else
	$(MAKE) -C $(PKG_BUILD_DIR)/libcedarx \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -D$(TINA_CHIP_TYPE)  -DONLY_ENABLE_AUDIO -D$(KERNEL_VERSION_ION) -Wno-error=format  $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS)" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__  -D$(TINA_CHIP_TYPE) -D$(KERNEL_VERSION_ION) -Wno-error=format $(COMPILE_CEDARX_LEVEL) $(CEDARX_EXTRA_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(CEDARX_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)  -L$(PKG_BUILD_DIR)/libcedarx/install/lib" \
		BOARD_PLATFORM="$(TARGET_BOARD_PLATFORM)" \
		install
endif

	$(MAKE) -C $(PKG_BUILD_DIR)/tplayer \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D$(KERNEL_VERSION_ION)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		BOARD_PLATFORM="$(TARGET_BOARD_PLATFORM)" \
		CPU_TYPE="$(TARGET_CPU_VARIANT)" \
		C_LIB_TYPE="$(CONFIG_LIBC)" \
		KERNEL64_USER32="$(CONFIG_COMPLILE_KERNEL64_USER32)" \
		ONLY_ENABLE_AUDIO="$(CONFIG_ONLY_ENABLE_AUDIO)"

ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(MAKE) -C $(PKG_BUILD_DIR)/trecorder \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)"	\
		BOARD_PLATFORM="$(TARGET_BOARD_PLATFORM)" \
		USE_VIN_ISP="$(CONFIG_TR_YES_USE_VIN_ISP)"

	$(MAKE) -C $(PKG_BUILD_DIR)/jpegdecode \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		KERNEL64_USER32="$(CONFIG_COMPLILE_KERNEL64_USER32)" \
		CPU_TYPE="$(TARGET_CPU_VARIANT)" \
		C_LIB_TYPE="$(CONFIG_LIBC)"

	$(MAKE) -C $(PKG_BUILD_DIR)/tmetadataretriever \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		KERNEL64_USER32="$(CONFIG_COMPLILE_KERNEL64_USER32)" \
		CPU_TYPE="$(TARGET_CPU_VARIANT)" \
		C_LIB_TYPE="$(CONFIG_LIBC)"
endif
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	## copy libraries
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(CP) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/*.so							$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/libcedarc/install/lib/*.so								$(PKG_INSTALL_DIR)/usr/lib/
endif
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/*.so							$(PKG_INSTALL_DIR)/usr/lib/
ifeq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	rm $(PKG_INSTALL_DIR)/usr/lib/libsubdecoder.so
endif
	$(CP) $(PKG_BUILD_DIR)/libcedarx/install/lib/*.so								$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tplayer/libtplayer.so									$(PKG_INSTALL_DIR)/usr/lib/
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(CP) $(PKG_BUILD_DIR)/trecorder/libtrecorder.so								$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/jpegdecode/libjpegdecode.so							$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tmetadataretriever/libtmetadataretriever.so					$(PKG_INSTALL_DIR)/usr/lib/
endif
	cd $(PKG_INSTALL_DIR)/usr/lib/

	## copy header files.
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	## log.h
	$(CP) $(PKG_BUILD_DIR)/libcedarc/base/include/log.h								$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## libcedarc vdecoder/vencoder
	-cp -r $(PKG_BUILD_DIR)/libcedarc/include											$(PKG_INSTALL_DIR)/usr/include/allwinner/
endif
	##cdx_config.h
	-cp -r $(PKG_BUILD_DIR)/libcedarx/cdx_config.h										$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## libcedarx adecoder/aencoder/sdecoder
	-cp -r $(PKG_BUILD_DIR)/libcedarx/external/include/adecoder/adecoder.h					$(PKG_INSTALL_DIR)/usr/include/allwinner/
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	-cp -r $(PKG_BUILD_DIR)/libcedarx/external/include/aencoder/aencoder.h					$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcedarx/external/include/sdecoder/sdecoder.h					$(PKG_INSTALL_DIR)/usr/include/allwinner/
endif
	## libcedarx:libcore
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/base/include/*.h								$(PKG_INSTALL_DIR)/usr/include/allwinner/
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/muxer/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
endif
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/parser/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/stream/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/libcedarx/libcore/playback/include/*.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## xplayer
	-cp -r $(PKG_BUILD_DIR)/libcedarx/xplayer/include/*.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## tplayer
	-cp $(PKG_BUILD_DIR)/tplayer/tplayer.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	## trecorder
	-cp $(PKG_BUILD_DIR)/trecorder/Trecorder.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp $(PKG_BUILD_DIR)/trecorder/dataqueue.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
	## jpegdecode
	-cp $(PKG_BUILD_DIR)/jpegdecode/jpegdecode.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## tmetadataretriever
	-cp $(PKG_BUILD_DIR)/tmetadataretriever/tmetadataretriever.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
	## awrecorder
	-cp $(PKG_BUILD_DIR)/libcedarx/awrecorder/awencoder.h							$(PKG_INSTALL_DIR)/usr/include/allwinner/
endif
	echo "**InstallDev path:$(1)***"
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/

	mkdir -p $(PKG_BUILD_DIR)/all_libs
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(CP) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/*.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/libcedarc/install/lib/*.so $(PKG_BUILD_DIR)/all_libs
endif
	$(CP) $(PKG_BUILD_DIR)/$(CEDARX_LIBS_PATH)/*.so $(PKG_BUILD_DIR)/all_libs
ifeq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	rm $(PKG_BUILD_DIR)/all_libs/libsubdecoder.so
endif
	$(CP) $(PKG_BUILD_DIR)/libcedarx/install/lib/*.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/tplayer/libtplayer.so $(PKG_BUILD_DIR)/all_libs
ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(CP) $(PKG_BUILD_DIR)/trecorder/libtrecorder.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/jpegdecode/libjpegdecode.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/tmetadataretriever/libtmetadataretriever.so $(PKG_BUILD_DIR)/all_libs
endif

ifeq ($(CONFIG_PACKAGE_xplayerdemo),y)
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/libcedarx/install/bin/* $(1)/usr/bin/
endif

ifneq ($(CONFIG_ONLY_ENABLE_AUDIO),y)
	$(CP) $(PKG_BUILD_DIR)/trecorder/recorder.cfg									$(1)/etc/
endif

	echo "**install path:$(1)***"

	for n in $(SUPPORT_LIBRARY_ALL); \
	do \
		$(CP) $(PKG_BUILD_DIR)/all_libs/$$$$n $(1)/usr/lib; \
	done
## deal with cedarx memory config
	$(CP) $(PKG_BUILD_DIR)/$(TARGET_BOARD_PLATFORM)_linux_cedarx.conf $(PKG_BUILD_DIR)/all_libs/cedarx.conf
	$(CP) $(PKG_BUILD_DIR)/libcedarc/config/cedarc.conf $(PKG_BUILD_DIR)/all_libs/cedarc.conf

ifeq ($(CONFIG_CUSTOM_FBM_NUM_ENABLE),y)
	./set_cedar_conf.sh pic_4list_num $(CONFIG_FBM_DISPLAY_HOLDING) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_FBM_NUM_ENABLE)
	./set_cedar_conf.sh pic_4di_num $(CONFIG_FBM_DEINTERLEACED_USED) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_FBM_NUM_ENABLE)
	./set_cedar_conf.sh pic_4rotate_num $(CONFIG_FBM_ROTATE_USED) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_FBM_NUM_ENABLE)
	./set_cedar_conf.sh pic_4smooth_num $(CONFIG_FBM_SMOOTHLY_USED) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_FBM_NUM_ENABLE)
endif

ifeq ($(CONFIG_CUSTOM_VBV_ENABLE),y)
	./set_cedar_conf.sh vbv_buffer_size $(CONFIG_VBVBUFSIZE) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_VBV_ENABLE)
endif

ifeq ($(CONFIG_SCALE_DOWN_ENABLE),y)
	./set_cedar_conf.sh scaledown_large_video_flag 1 $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_SCALE_DOWN_ENABLE)
	./set_cedar_conf.sh scaledown_width_limit $(CONFIG_SCALE_DOWN_WIDTH_LIMIT) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_SCALE_DOWN_ENABLE)
	./set_cedar_conf.sh scaledown_ratio $(CONFIG_SCALE_DOWN_RATIO) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_SCALE_DOWN_ENABLE)
endif

ifeq ($(CONFIG_CUSTOM_CACHE_ENABLE),y)
	./set_cedar_conf.sh cache_buffer_size  $(CONFIG_CACHE_BUFFER_SIZE) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_CACHE_ENABLE)
	./set_cedar_conf.sh cache_buffer_size_live  $(CONFIG_LIVE_CACHE_BUFFER_SIZE) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_CACHE_ENABLE)
	./set_cedar_conf.sh start_play_cache_time  $(CONFIG_START_PLAY_CACHE_TIME) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_CACHE_ENABLE)
	./set_cedar_conf.sh max_http_stream_buf_size  $(CONFIG_HTTP_STREAM_CACHE_SIZE) $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_CUSTOM_CACHE_ENABLE)
endif

ifeq ($(CONFIG_USE_ION_BUF_ENABLE),y)
	./set_cedar_conf.sh use_ion_buf_flag 1 $(PKG_BUILD_DIR)/all_libs/cedarx.conf $(CONFIG_USE_ION_BUF_ENABLE)
endif

	$(CP)          $(PKG_BUILD_DIR)/all_libs/cedarx.conf						 $(1)/etc/cedarx.conf
	$(CP)          $(PKG_BUILD_DIR)/all_libs/cedarc.conf						 $(1)/etc/cedarc.conf
	cd $(1)/usr/lib/; rm -rf $(NOT_SUPPORT_VIDEO_LIBS)

endef

endif

ifeq ($(TARGET_BOARD_PLATFORM),$(filter $(TARGET_BOARD_PLATFORM),r16 r58 r40 r18 r6 c200s g102 r11 r7 r30 r311 r333 r331 r7s t7 r332 v306 dolphin h3 h6 mr133 r328s2 r328s3 mr813))
$(eval $(call BuildPackage,$(PKG_NAME)))
endif
