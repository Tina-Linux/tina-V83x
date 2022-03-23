
#ifndef CONFIG_H
#define CONFIG_H

// option of conpile tool chain for linux makefile.
// arm-linux-gnueabihf- or arm-none-linux-gnueabi- tool chain
#define OPTION_CC_GNUEABIHF 1
#define OPTION_CC_GNUEABI   2

// option for os config.
#define OPTION_OS_ANDROID   1
#define OPTION_OS_LINUX     2

// option for os version config.
#define OPTION_OS_VERSION_ANDROID_4_2   1
#define OPTION_OS_VERSION_ANDROID_4_4   2
#define OPTION_OS_VERSION_ANDROID_5_0   3

// option for momory driver config.
#define OPTION_MEMORY_DRIVER_SUNXIMEM   1
#define OPTION_MEMORY_DRIVER_ION        2

// option for product config.
#define OPTION_PRODUCT_PAD      1
#define OPTION_PRODUCT_TVBOX    2
#define OPTION_PRODUCT_OTT_CMCC 3
#define OPTION_PRODUCT_IPTV     4
#define OPTION_PRODUCT_DVB      5

// option for chip config.
#define OPTION_CHIP_1623        1
#define OPTION_CHIP_1625        2
#define OPTION_CHIP_1633        3
#define OPTION_CHIP_1651        4
#define OPTION_CHIP_1650        5
#define OPTION_CHIP_1661        6
#define OPTION_CHIP_1667        7
#define OPTION_CHIP_1639        8
#define OPTION_CHIP_1673        9
#define OPTION_CHIP_1680        10
#define OPTION_CHIP_1681        11
#define OPTION_CHIP_1689        12
#define OPTION_CHIP_1701        13



// option for dram interface.
#define OPTION_DRAM_INTERFACE_DDR1_16BITS   1
#define OPTION_DRAM_INTERFACE_DDR1_32BITS   2
#define OPTION_DRAM_INTERFACE_DDR2_16BITS   3
#define OPTION_DRAM_INTERFACE_DDR2_32BITS   4
#define OPTION_DRAM_INTERFACE_DDR3_16BITS   5
#define OPTION_DRAM_INTERFACE_DDR3_32BITS   6
#define OPTION_DRAM_INTERFACE_DDR3_64BITS   7

// option for debug level.
#define OPTION_LOG_LEVEL_CLOSE      0
#define OPTION_LOG_LEVEL_ERROR      1
#define OPTION_LOG_LEVEL_WARNING    2
#define OPTION_LOG_LEVEL_DEFAULT    3
#define OPTION_LOG_LEVEL_DETAIL     4

//## option for cmcc
#define OPTION_CMCC_NO   0
#define OPTION_CMCC_YES  1

//## option for is_camera_decoder
#define OPTION_IS_CAMERA_DECODER_NO   0
#define OPTION_IS_CAMERA_DECODER_YES  1

// option for hls seek.
#define OPTION_HLS_SEEK_IN_SEGMENT	1
#define OPTION_HLS_NOT_SEEK_IN_SEGMENT 0

// option for ve ipc.
#define OPTION_VE_IPC_DISABLE   1
#define OPTION_VE_IPC_ENABLE	2

// configuration.
#define CONFIG_CC    OPTION_CC_GNUEABI
#define CONFIG_OS    OPTION_OS_LINUX
#define CONFIG_OS_VERSION    OPTION_OS_VERSION_ANDROID_4_4
#define CONFIG_MEMORY_DRIVER    OPTION_MEMORY_DRIVER_ION
#define CONFIG_PRODUCT    OPTION_PRODUCT_TVBOX
#define CONFIG_CHIP    OPTION_CHIP_1701
#define CONFIG_DRAM_INTERFACE    OPTION_DRAM_INTERFACE_DDR3_32BITS
#ifndef CONFIG_LOG_LEVEL
#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_WARNING
#endif

#define CONFIG_VE_IPC    OPTION_VE_IPC_DISABLE

#define CONFIG_CMCC    OPTION_CMCC_NO
// <CONFIG_HLS_SEEK=1>: seek to the right request point, it may be slower a little.
// <CONFIG_HLS_SEEK=0>: seek to the beginning of this segment, it is faster a little, but not much accurat.
#define CONFIG_HLS_SEEK    OPTION_HLS_NOT_SEEK_IN_SEGMENT

//CONFIG_IS_CAMERA_DECODER is for camera plaform compiling libvdecoder,
//only compile h264 and jpeg decoder if setting yes
#define CONFIG_IS_CAMERA_DECODER    OPTION_IS_CAMERA_DECODER_NO

// When hls live tv download ts slice failed, we will try to reconnect http server
// from the last read point. But the reconnection may fail with http 404 error.
// Then we will try to reconnect the same server with 30s timeout. If this
// reconnection is also timeout and (CONFIG_HLS_TRY_NEXT_TS == 1), we try to
// request next ts slice. (for yst icntv)
// If CONFIG_HLS_TRY_NEXT_TS == 0, after 30s timeout, we will return IO ERROR.
#define CONFIG_HLS_TRY_NEXT_TS     0

#define CONFIG_ENABLE_DEMUX_ASF    1
#define CONFIG_ENABLE_DEMUX_AVI    1
#define CONFIG_ENABLE_DEMUX_BLUERAYDISK    1
#define CONFIG_ENABLE_DEMUX_MPEGDASH    1
#define CONFIG_ENABLE_DEMUX_FLV    1
#define CONFIG_ENABLE_DEMUX_HLS    1
#define CONFIG_ENABLE_DEMUX_MKV    1
#define CONFIG_ENABLE_DEMUX_MMS    1
#define CONFIG_ENABLE_DEMUX_MOV    1
#define CONFIG_ENABLE_DEMUX_MPG    1
#define CONFIG_ENABLE_DEMUX_PMP    1
#define CONFIG_ENABLE_DEMUX_OGG    1
#define CONFIG_ENABLE_DEMUX_RX    1
#define CONFIG_ENABLE_DEMUX_TS    1
#define CONFIG_ENABLE_DEMUX_M3U9	 1
#define CONFIG_ENABLE_DEMUX_PLAYLIST     1
#define CONFIG_ENABLE_DEMUX_APE			 1
#define CONFIG_ENABLE_DEMUX_FLAC	 1
#define CONFIG_ENABLE_DEMUX_AMR			 1
#define CONFIG_ENABLE_DEMUX_ATRAC	 1
#define CONFIG_ENABLE_DEMUX_MP3			 1
#define CONFIG_ENABLE_DEMUX_DTS			 1
#define CONFIG_ENABLE_DEMUX_AAC			 1
#define CONFIG_ENABLE_DEMUX_WAV			 1
#define CONFIG_ENABLE_DEMUX_REMUX	 1
#define CONFIG_ENABLE_DEMUX_WVM			 1
#define CONFIG_ENABLE_DEMUX_MMSHTTP	 1
#define CONFIG_ENABLE_DEMUX_AWTS	 1
#define CONFIG_ENABLE_DEMUX_SSTR	 1
#define CONFIG_ENABLE_DEMUX_CAF			 1

//* other global define
#if(CONFIG_CHIP == OPTION_CHIP_1667 \
    || (CONFIG_CHIP == OPTION_CHIP_1673 && CONFIG_PRODUCT == OPTION_PRODUCT_PAD&& CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_5_0) \
    || (CONFIG_CHIP == OPTION_CHIP_1639 && CONFIG_PRODUCT == OPTION_PRODUCT_PAD&& CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_5_0) \
    || (CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX && WIDEVINE_OEMCRYPTO_LEVEL == 1)\
	|| (CONFIG_CHIP == OPTION_CHIP_1639 && CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX && CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_5_0) \
	|| (CONFIG_CHIP == OPTION_CHIP_1689 && CONFIG_PRODUCT == OPTION_PRODUCT_PAD&& CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_5_0))
    #define USE_NEW_DISPLAY 1
#else
    #define USE_NEW_DISPLAY 0
#endif

#if(CONFIG_CHIP == OPTION_CHIP_1680 || CONFIG_CHIP == OPTION_CHIP_1667 || CONFIG_CHIP == OPTION_CHIP_1689)
    #define GPU_TYPE_MALI 1
#else
    #define GPU_TYPE_MALI 0
#endif

#if(USE_NEW_DISPLAY == 1) && (CONFIG_PRODUCT == OPTION_PRODUCT_PAD)
    #define DROP_3D_SECOND_VIDEO_STREAM 1
#else
    #define DROP_3D_SECOND_VIDEO_STREAM 0
#endif

#if((CONFIG_PRODUCT == OPTION_PRODUCT_PAD) && CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_5_0)
    #define MUTE_DRM_WHEN_HDMI_FLAG 1
#else
    #define MUTE_DRM_WHEN_HDMI_FLAG 0
#endif

//*We surpport display subtitle in cedarx on android4.2 and 4.4.
//*but the APIs of skia on android5.0 are much more different,
//*so it don't work on android5.0
#define ENABLE_SUBTITLE_DISPLAY_IN_CEDARX (0)

#endif
