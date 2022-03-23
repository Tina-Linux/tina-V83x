
一、linux相关说明:

1. 每套linux so对应的编译工具链如下：

# arm926-uclibc for BU1 C500
# arm-openwrt-linux-muslgnueabi for BU2 Tina2.0
# arm-openwrt-linux-muslgnueabi-v for v5
# arm-openwrt-linux-uclibc for BU2 R16
so_dir: arm926-uclibc                 host: arm-linux              toolchain_dir: uclibc_arm926_static
so_dir: arm-linux-gnueabi             host: arm-linux-gnueabi      toolchain_dir: external-toolchain
so_dir: arm-linux-gnueabihf           host: arm-linux-gnueabihf    toolchain_dir: 系统自带
so_dir: arm-none-linux-gnueabi        host: arm-none-linux-gnueabi toolchain_dir: 系统自带
so_dir: arm-openwrt-linux-muslgnueabi host: arm-openwrt-linux      toolchain_dir: OpenWrt-Toolchain-sunxi_gcc-5.2.0_musl-1.1.12_eabi.Linux-x86_64
so_dir: arm-openwrt-linux-uclibc      host: arm-openwrt-linux      toolchain_dir: toolchain_arm_uClibc
so_dir: arm-linux-gnueabihf-linaro    host: arm-linux-gnueabihf    toolchain_dir: gcc-linaro-arm-linux-gnueabihf-4.8-2014.01_linux
so_dir: arm-aarch64-openwrt-linux     host: aarch64-openwrt-linux  toolchain_dir: toolchain-sunxi-tina2.0-64
so_dir: arm-openwrt-linux-muslgnueabi-v5  host: arm-openwrt-linux-muslgnueabi    toolchain_dir: linux-x86

2.编译步骤如下（以arm926-uclibc为例）：

2.1 export编译工具链：
                     TOOLS_CHAIN=/home/user/workspace/tools_chain/
                     export PATH=${TOOLS_CHAIN}/uclibc_arm926_static/bin:$PATH

2.2. 运行automake的相关工具：./bootstrap

2.3. 配置makefile：
2.3.1 模式：./configure --prefix=INSTALL_PATH --host=HOST_NAME LDFLAGS="-LSO_PATH"
2.3.2 示例：./configure --prefix=/home/user/workspace/libcedarc/install --host=arm-linux LDFLAGS="-L/home/user/workspace/libcedarc/lib/arm926-uclibc"
2.3.3 特别说明：如果内核用的是linux3.10, 则必须加上flag：CFLAGS="-DCONF_KERNEL_VERSION_3_10" CPPFLAGS="-DCONF_KERNEL_VERSION_3_10"
	  即：./configure --prefix=/home/user/workspace/libcedarc/install --host=arm-linux CFLAGS="-DCONF_KERNEL_VERSION_3_10" CPPFLAGS="-DCONF_KERNEL_VERSION_3_10" LDFLAGS="-L/home/user/workspace/libcedarc/lib/arm926-uclibc"

2.4 编译：make ; make install

二、版本改动说明：

1). CedarC-v1.0.4

1. 特别说明：

1.1 camera模块相关：( >=android7.0的平台需要进行如下修改,其他平台保持之前的做法)
    修改的原因：a.omx和android framework都没有对NV21和NV12这两种图像格式进行细分，都是用OMX_COLOR_FormatYUV420SemiPlanar
	            进行表示，OMX_COLOR_FormatYUV420SemiPlanar即可以表示NV21，也可以表示NV12；
                b.而ACodec将OMX_COLOR_FormatYUV420SemiPlanar用作NV12，camera将OMX_COLOR_FormatYUV420SemiPlanar用于NV21，
                之前的做法是omx_venc通过进程的包名进行区分兼容，若调用者是ACodec，则用作NV12，若调用者是camera，则用作
				NV21；
				c.android7.0因为权限管理的原因，无法获取到进程的包名，所以无法在omx_venc进行兼容，只能在上层caller层进行
				兼容，而cts会对ACodec的接口进行测试，若改动ACodec，则会响应到cts测试，所以只能修改camera
				d.修改的原则为：扩展图像格式的枚举类型的成员变量OMX_COLOR_FormatYVU420SemiPlanar，用于表示NV21；
				  即 OMX_COLOR_FormatYUV420SemiPlanar --> NV12
				     OMX_COLOR_FormatYVU420SemiPlanar --> NV21

	修改的地方：
	a. 同步头文件：同步openmax/omxcore/inc/OMX_IVCommon.h 到./native/include/media/openmax/OMX_IVCommon.h
	b. camera模块在调用openmax/venc编码接口时需要进行修改：
	   修改前 NV21 --> OMX_COLOR_FormatYUV420SemiPlanar，
	   修改后 NV21 --> OMX_COLOR_FormatYVU420SemiPlanar;

1.2 加载解码库的说明：
    之前加载解码库的操作在vdecoder.c这一层进行，为了提高灵活性，从v1.0.4后
把加载解码库的操作放到上层调用者进行，上层可调用vdecoder.c的AddVDPlugin接口默认
加载所有的子解码，或者参照如下代码按需加载：

static void InitVDLib(const char *lib)
{
    void *libFd = NULL;
    if(lib == NULL)
    {
        loge(" open lib == NULL ");
        return;
    }

    libFd = dlopen(lib, RTLD_NOW);

    VDPluginFun *PluginInit = NULL;

    if (libFd == NULL)
    {
        loge("dlopen '%s' fail: %s", lib, dlerror());
        return ;
    }

    PluginInit = (VDPluginFun*)dlsym(libFd, "CedarPluginVDInit");
    if (PluginInit == NULL)
    {
        logw("Invalid plugin, CedarPluginVDInit not found.");
        return;
    }
    logd("vdecoder open lib: %s", lib);
    PluginInit(); /* init plugin */
    return ;
}

static void AddVDLib(void)
{
    InitVDLib("/system/lib/libawh265.so");
    InitVDLib("/system/lib/libawh265soft.so");
    InitVDLib("/system/lib/libawh264.so");
    InitVDLib("/system/lib/libawmjpeg.so");
    InitVDLib("/system/lib/libawmjpegplus.so");
    InitVDLib("/system/lib/libawmpeg2.so");
    InitVDLib("/system/lib/libawmpeg4base.so");
    InitVDLib("/system/lib/libawmpeg4normal.so");
    InitVDLib("/system/lib/libawmpeg4vp6.so");
    InitVDLib("/system/lib/libawmpeg4dx.so");
    InitVDLib("/system/lib/libawmpeg4h263.so");
    InitVDLib("/system/lib/libawvp8.so");

    return;
}

2. 改动点如下：
2.1 openmax:venc add p_skip interface
2.2 h265:fix the HevcDecodeNalSps and HevcInitialFBM
2.3 h264:refactor the H264ComputeOffset
2.4 mjpeg scale+rotate的修正
2.5 vdcoder/h265: add the code of parser HDR info
2.6 vdecoder/h265: add the process of error-frame
2.7 vdecoder/h264: make sure pMbNeighborInfoBuf is 16K-align to fix mbaff function
2.8 openmax/vdec: remove cts-flag
2.9 openmax/venc: remove cts-flag
2.10 detection a complete frame bitstream and crop the stuff zero data
2.11 vdecoder/h265:fix the bug that the pts of keyFrame is error when seek
2.12 openmax/inc: adjust the define of struct
2.13 vencoder: add lock for vencoderOpen()
2.14 vdecoder/ALMOST decoders:fix rotate and scaledown
2.15 vdecoder/h265:fix the process of searching the start-code when sbm cycles
2.16 vdecoder/h265:fix the bug when request fbm fail after fbm initial
2.17 vdecoder/h265:improve the process when poc is abnormal
2.18 cedarc: unify the release of android and linux
2.19 vdecoder/avs: make mbInfoBuf to 16K align

2).CedarC-v1.0.5

1. 改动点如下：
1.1.configure.ac:fix the config for linux compiling
1.2.openmax/venc: revert mIsFromCts on less than android_7.0 platfrom
1.3.vdecoder/h265soft:make h265soft be compatible with AndroidN
1.4.cedarc: merge the submit of cedarc-release
1.5.vdecoder/h265:use the flag "bSkipBFrameIfDelay"
1.6.vdecoder:fix the buffer size for thumbnail mode
1.7.cedarc: fix compile error of linux
1.8.omx:venc add fence for androidN
1.9.openmax:fix some erros
1.10.omx_venc: add yvu420sp for omx_venc init
1.11.videoengine:add 2k limit for h2
1.12.cedarc: add the toolschain of arm-openwrt-linux-muslgnueabi for v5
1.13.cedarc: 解决0x1663 mpeg4 播放花屏的问题
1.14.vdecoder: fix compile error of soft decoder for A83t
1.15.omx_vdec: fix the -1 bug for cts
1.16.修改mpeg2 获取ve version的方式
1.17.vencoder: fix for input addrPhy check
1.18.cedarc: merge the submit of cedarc-release

3). CedarC-v1.1

1. 特别说明：
1.1 v1.1集成了H6-dev开发分支的修改成果；增加了h265 10bit和afbc功能，此功能需
    cedarx匹配修改，否则无法独立生效；
1.2 v1.1 关闭了用于申请物理连续内存的memory接口，上层若需要申请物理内存，需模块
    内部实现memory的接口或调用其他接口；
1.3 增加了VideoDecoderGetVeIommuAddr和VideoDecoderFreeVeIommuAddr这两个接口，用于
    对iommu的buffer进行绑定与解绑定操作。

2. 改动点如下：
2.1 增加h265 10bit和afbc功能的支持；
2.2 增加vp9 硬件解码驱动；
2.3 增加对iommu buffer管理的支持；
2.4 对ve模块进行了重构；
2.5 对sbm模块进行了重构；
2.6 memory接口不再对外开放。

4). CedarC-v1.1.1

1. 改动点如下：
1.1 调整cedarc-release，目前release出来的cedarc可适用于所有android和linxu平台；
1.2 vdecoder/Vp8:process the case of showFrm
1.3 vdecoder/h265: increase the size of HEVC_LOCAL_SBM_BUF_SIZE
1.4 vdecoder/videoengine: add the function of checkAlignStride
1.5 vdecoder/h265: set proc info
1.6 vdecoder/sbm: 修改H265 sbmFrame 访问越界的bug

5). CedarC-v1.1.2

1. 改动点如下：
1.1 vdecoder/h264: set proc info
1.2 vdecoder/mjpge: set proc info
1.3 vdecoder: improve function of savePicture
1.4 vencoder: fix for jpeg get phy_addr and androidN get chroma addr
1.5 在fbmInfo中添加offset的信息
1.6 openmax/vdec: set mCropEnable to false on linux
1.7 采用属性检测的方式来确定内存的使用方式
1.8 unmap the fbm buffer when native window changed
1.9 vp8 return the alterframe error for '新仙鹤神针.mkv'

6). CedarC-v1.1.3
1.1 vdecoder: change the 6k range
1.2 openmax:venc: fix for recorder
1.3 vdecoder: fix the bug for initializeVideoDecoder fails
1.4 vdecoder/h265Soft: fix the bug: crash when seek the video H265_test.mkv
1.5 ve: control phyOffset in ve module
1.6 use iomem.type to check iommu mode
1.7 vdecoder: add nBufFd when call FbmReturnReleasePicture
1.8 openmax: load the libawwmv3.so when init
1.9 vdecoder/VP9:reset some parameter for Vp9HwdecoderReset()
1.10 ve: dynamic set dram high channal

7). CedarC-v1.1.4
1.1 vencoder: add for thumb write back func
1.2 vencoder:fix for set thumb write back scaler dynamic
1.3 vencoder: fix for only thumb write back no encode
1.4 修改检测硬件busy的状态位的等待时间
1.5 vdecoder/sbmFrame: fix the error video 720P_V_.HEVC_AAC_2M_15F.mkv

8). CedarC-v1.1.6

1. 改动点如下：
1.1 vdecoder/h264:after reset,the first frame pts is same to the last bitstream
1.2 h265:fix the bug of parse-extradata
1.3 修改H264 跳播后pts异常的bug
1.4 vdecoder: add lock for VideoEngineCreate
1.5 fix gts test fail
1.6 vdecoder/vc1: fix the bug: error when seek
1.7 调整H264代码架构，清除最后一帧没有解码的bug
1.8 openmax/vdec: not support metadata buffer
1.9 ve: fix for getIcVersion when other process is reseting ve
1.10 openmax/vdec: open mem-ops when use
1.11 vdecoder/h264/distinguish SbmReturnStream of stream and frame for resolution change
1.12 omx:venc: fix for recorder of h6-kk44-iommu
1.13 openmax/vdec: support afbc function
1.14 h264:fix the progress of erro
1.15 openmax/vdec: plus timeout to 5s

2.特别说明:
2.1 mediacodec通路对afbc功能的支持
    afbc功能的支持涉及到两个模块，其一是cedarc/openmax模块的修改，此模块的修改已完成，将
 cedarc的代码更新到cedarc-v1.1.6或更新版本即可；
    其二是framework层的修改，目前framework层的patch只集成到H6的方案上，若其他方案要集成afbc
 的功能，可找AL3的王喜望提供framework_patch.

9). CedarC-v1.1.7

1. 改动点如下：
1.1. vdecoder/avs:the case of diff pts > 2s for TvStream
1.2. vencoder:jpeg fix for exif buffer memory leak
1.3. h264:fix the bug of frameStream-end
1.4. vdecoder: fix the bug when get memops
1.5. vdecoder/h264:u16 to s32
1.6. vdecoder: add iptv-info for h264 and h265
1.7. vdecoder/fbm: avoid memory leak
1.8. 解决H8 大白鲨.mp4播放花屏的问题
1.9. vdecoder/sbmH264: surpport secure video
1.10. 按照H264的分辨率来申请Dram buffer 的大小
1.11. 修正surface 切换时，播放A026.mpeg4挂住的问题
1.12. secure 模式下支持iommu
1.13. H265 的ve 频率在H6上调整为696MHZ
1.14. memory: fix for get phy_addr 0 when is iommu mode
1.15. H264 添加软件解头信息的代码
1.16. openmax/vdec: increase the input-buffer-size to 6 MB
1.17. vdecoder: optimize the policy of set vefreq
1.18. 在mpeg2解码器中添加错误帧的识别
1.19. omx:venc: fix for gpu buf
1.20. vdecoder/avs/fix the flag of bIsProgressive
1.21. openmax/vdec: support native-handle
1.22. 添加ve 频率配置项
1.23. vdecoder: not get the veLock with the softDecoder case

10). CedarC-v1.1.7 -- patch-002

1. 改动点如下：
1.1. 修改H264计算dram buffer的方式
1.2. 修改T3 mpeg4v2的解码方式,此格式VE不支持硬解
1.3. 兼容widewine 模式下extradata 的处理
1.4. vdecoder/H264/decoder one frame then return
1.5. 兼容parser传错width和height的case
1.6. openmax/vdec: add the policy of LIMIT_STREAM_NUM
1.7. videoengine:fix the specificdat value
1.8. videoengine: fix the ve unlock in VideoEngineReopen
1.9. openmax/vdec: add the function of di
1.10. vdecoder/H264:resolution change for online video
1.11. 修正没有获取到fbm 信息时,解码器就接收到eos标记而不能正常退出的bug
1.12. openmax/vdec: remove bCalledByOmxFlag
1.13. h265: fix the bug decoding the slcieRps as numOfRefs is outoff range
1.14. openmax/vdec: not init decoder in the status of idle

10). CedarC-v1.1.7 -- patch-003

1. 改动点如下：
1.1 修复h264 cts failed
1.2 openmax/vdec: reset MAX_NUM_OUT_BUFFERS from 15 to 4 as it consume too much buffer
1.3 h265: fix the pts of eos frame for gts
1.4 fbm: fix the value of pMetaData
1.5 修改mpeg2 pt2.vob 崩溃的bug

10). CedarC-v1.1.7 -- patch-004

1. 改动点如下：
1.1 openmax/vdec: fix the process of decoding the last frame which size changes
1.2 h265: fix the bug of decoding extraData
1.3 fbm: add code for allocating metadata buffer for linux
1.4 di not support 4K stream, for 4K interlace stream, ve does scaledown
1.5 vdecoder: limit nVbvBufferSize to [1 MB, 32MB]
1.6 openmax/vdec: increase OMX_VIDEO_DEC_INPUT_BUFFER_SIZE_SECURE from 256 KB to 1 MB
1.7 修改H264因丢帧导致的pts 计算出错的问题
1.8 vdecoder:add the decIpVersion for T7
1.9 vdecoder/h264:add reset parameters of H264ResetDecoderParams()
1.10 openMAX: Adapt DI process with two input di pictures to the platform of H3
1.11 openmax/vdec: just set nv21 format in di-function case
1.12 demo: add vencDem to cedarc
1.13 support the field structure of vc1 frame packed mode
1.14 demo:demoVencoder: fix for style error
1.15 config: add config file of T7 platform

11). CedarC-v1.1.8

1. 改动点如下：
1.1. 修正mp4normal 缩略图模式下没有specialdata时,无法解码的bug
1.2. 修正mpeg4Normal 届缩略图的bug
1.3. 支持mjpeg444的解码
1.4. vdecoder: fix the bug when sbm inits failed
1.5. vdecoder:fix for VC1. Be compatible to 64 bit system
1.6. 修复0x1663播放mpeg4文件花屏的问题
1.7. cdcUtil: fix ion handle for linux4.4
1.8. 恢复vbv buffer size 的设置方式
1.9. omx:venc: fix for h265 enc error
1.10. vdecoder:A63 upgrade,Mpeg1/2/4 addr register
1.11. vdecoder:catch DDR value for H265
1.12. 修正播放05_100M.ts 视频pts 出错的问题
1.13. cedarc:avs:add support for sun8iw7p1
1.14. 1708 芯片的mjpeg 调用mjpegPlus
1.15. vdecoder/h265:protecting nFrameDuration
1.16. vdecoder/sbm: fix bug: crash when length of stream is 0
1.17. 添加andoido 的支持
1.18. 修改H265的丢帧机制
1.19. cedarc:avs_plus:fix avs_plus unsupport error on chip-1680
1.20. cedarc/log: dynamic show log by property_get
1.21. cedarc: compile so in system/lib/ and system/vendor/lib
1.22. vdecoder/fbm: reduce frmbuf_c_size of afbc

11). CedarC-v1.1.9

1. 改动点如下：

1.1 修改linux目录库名称：

    修改前                                 修改后
arm-linux-gnueabi                    arm-linux-gnueabi
arm-linux-gnueabihf                  arm-linux-gnueabihf
arm-none-linux-gnueabi               arm-none-linux-gnueabi
arm-linux-gnueabihf-linaro           toolchain-sunxi-aarch64-glibc
arm926-uclibc                        toolchain-sunxi-aarch64-musl
arm-aarch64-openwrt-linux            toolchain-sunxi-arm9-glibc
arm-openwrt-linux-muslgnueabi        toolchain-sunxi-arm9-musl
arm-openwrt-linux-muslgnueabi-v5     toolchain-sunxi-arm-glibc
arm-openwrt-linux-uclibc             toolchain-sunxi-arm-musl

说明：修改原因为：根据BU1最新提供的工具链进行整理；
      前面3套为用得比较多的编译工具链，适用于各个BU的方案开发；
      后面6套为BU1提供的工具链编译出来的so，如果其他BU需要使用，建议先与cedarc驱动组的同事（如王喜望/刘小燕）进行沟通了解；

1.2 修改linux so名称：

	修改前                            修改后
libcdc_vd_avs.so                   libawavs.so
libcdc_vd_h264.so                  libawh264.so
libcdc_vd_h265.so                  libawh265.so
libcdc_vd_mjpeg.so                 libawmjpeg.so
libcdc_vd_mjpegs.so                libawmjpegplus.so
libcdc_vd_mpeg2.so                 libawmpeg2.so
libcdc_vd_mpeg4base.so             libawmpeg4base.so
libcdc_vd_mpeg4dx.so               libawmpeg4dx.so
libcdc_vd_mpeg4h263.so             libawmpeg4h263.so
libcdc_vd_mpeg4normal.so           libawmpeg4normal.so
libcdc_vd_mpeg4vp6.so              libawmpeg4vp6.so
libcdc_vd_vp8.so                   libawvp8.so
libcdc_vd_vp9Hw.so                 libawvp9Hw.so
libcdc_vd_wmv3.so                  libawwmv3.so
libcdc_vencoder.so                 libvencoder.so
libcdc_ve.so                       libVE.so
libcdc_videoengine.so              libvideoengine.so

说明：修改linux so的名称的原因主要是为了与 android平台的so名称一致。

12). CedarC-v1.1.10

1. 改动点如下：
1.1 支持 android P 系统；
1.2 兼容 android P gms测试；
1.3 兼容重构后的openmax模块；
1.4 fix bugs。

2. 特别说明：

2.1 将编码器的接口层代码进行开源:
    闭源：将整个encoder模块编译成一个so: libvendoer.so;
    开源：将encoder模块分割成三个子模块：libvendoer.so / libvenc_base.so / libvenc_codec.so;
          libvendoer.so / libvenc_base.so　两个子模块进行开源，libvenc_codec.so为闭源部分。

　　使用方法的改变：闭源时适用编码器只需链接一个so即可，开源后需要链接三个so .
12). CedarC-v1.2.0
1. 改动点如下：
1.1 添加CdcMalloc.c，用于检测malloc的内存是否存在泄漏现象；
1.2 添加sysfs机制，可通过cat ve的节点实时查看sbm/fbm/vdecoder/内存等信息；
1.3 添加动态debug机制，通过修改配置文件cedarc.conf实现(修改配置文件后，需要kill掉相关的进程如mediaserver才会生效)；
1.4 独立出编码器isp的功能接口，使用者可单独调用VideoEncIspFunction接口；
1.5 fix bugs。
