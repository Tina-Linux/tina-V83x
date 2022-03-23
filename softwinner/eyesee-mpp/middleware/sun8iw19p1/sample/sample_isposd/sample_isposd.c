//#define LOG_NDEBUG 0
#define LOG_TAG "sample_region"
#include <utils/plat_log.h>

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "media/mm_comm_vi.h"
#include "media/mpi_vi.h"
#include "vo/hwdisplay.h"
#include "log/log_wrapper.h"

#include <ClockCompPortIndex.h>
#include <mpi_videoformat_conversion.h>
#include <confparser.h>
#include "sample_isposd_config.h"
#include "sample_isposd.h"
#include "mpi_isp.h"
#include <BITMAP_S.h>
#include "media/mpi_venc.h"
#include "mm_comm_venc.h"

#define HAVE_H264

static SampleIspOsdContext *gpSampleIspOsdContext = NULL;

int initSampleIspOsdContext(SampleIspOsdContext *pContext)
{
    memset(pContext, 0, sizeof(SampleIspOsdContext));
    pContext->mUILayer = HLAY(2, 0);
    cdx_sem_init(&pContext->mSemExit, 0);
    return 0;
}

int destroySampleIspOsdContext(SampleIspOsdContext *pContext)
{    
    cdx_sem_deinit(&pContext->mSemExit);    
    return 0;
}

static int ParseCmdLine(int argc, char **argv, SampleIspOsdCmdLineParam *pCmdLinePara)
{
    //alogd("sample_region input path is : %s", argv[0]);
    int ret = 0;
    int i = 1;
    memset(pCmdLinePara, 0, sizeof(SampleIspOsdCmdLineParam));
    while(i < argc)
    {
        if(!strcmp(argv[i], "-path"))
        {
            if((++i) >= argc)
            {
                aloge("fatal error!");
                ret = -1;
                break;
            }
            if(strlen(argv[i]) >= MAX_FILE_PATH_SIZE)
            {
                aloge("fatal error!");
            }
            strncpy(pCmdLinePara->mConfigFilePath, argv[i], strlen(argv[i]));
            pCmdLinePara->mConfigFilePath[strlen(argv[i])] = '\0';
        }
        else if(!strcmp(argv[i], "-h"))
        {
             printf("CmdLine param example:\n"                
                "\t run -path /home/sample_vi2vo.conf\n");
             ret = 1;
             break;
        }
        else
        {
             printf("CmdLine param example:\n"                "\t run -path /home/sample_vi2vo.conf\n");
        }
        ++i;
    }
    return ret;
}

static ERRORTYPE loadSampleIspOsdConfig(SampleIspOsdConfig *pConfig, const char *conf_path)
{
    int ret;

    if(NULL == conf_path)
    {
        alogd("user not set config file. use default test parameter!");        
        pConfig->mCaptureWidth = 1920;        
        pConfig->mCaptureHeight = 1080;        
        pConfig->mPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;        
        pConfig->mFrameRate = 30;        
        pConfig->mTestDuration = 0;

        pConfig->mBitmapFormat = MM_PIXEL_FORMAT_RGB_8888;
        pConfig->overlay_x = 32;
        pConfig->overlay_y = 960;
        pConfig->overlay_w = 100;
        pConfig->overlay_h = 100;


        pConfig->mbAttachToVi = false;
        pConfig->add_venc_channel = FALSE;
        pConfig->encoder_count = 5000;
        pConfig->bit_rate = 8388608;
        pConfig->EncoderType = PT_H264;
        strcpy(pConfig->OutputFilePath, "test.h264");
         
        return SUCCESS;
    }
    CONFPARSER_S stConfParser;
    ret = createConfParser(conf_path, &stConfParser);
    if(ret < 0)
    {
        aloge("load conf fail");
        return FAILURE;
    }
    
    memset(pConfig, 0, sizeof(SampleIspOsdConfig));

    pConfig->mCaptureWidth = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_CAPTURE_WIDTH, 1920);
    pConfig->mCaptureHeight = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_CAPTURE_HEIGHT, 1080);

    char *pStrPixelFormat = (char*)GetConfParaString(&stConfParser, SAMPLE_REGION_KEY_PIC_FORMAT, NULL);
    if(!strcmp(pStrPixelFormat, "yu12"))
    {
        pConfig->mPicFormat = MM_PIXEL_FORMAT_YUV_PLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "yv12"))
    {
        pConfig->mPicFormat = MM_PIXEL_FORMAT_YVU_PLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "nv21"))
    {
        pConfig->mPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }
    else if(!strcmp(pStrPixelFormat, "nv12"))
    {
        pConfig->mPicFormat = MM_PIXEL_FORMAT_YUV_SEMIPLANAR_420;
    }
    else
    {
        aloge("fatal error! conf file pic_format is [%s]?", pStrPixelFormat);
        pConfig->mPicFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;
    }
    pConfig->mFrameRate = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_FRAME_RATE, 30);
    pConfig->mTestDuration = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_TEST_DURATION, 0);

    char *strFormat = (char*)GetConfParaString(&stConfParser, SAMPLE_REGION_KEY_BITMAP_FORMAT, NULL);
    if(!strcmp(strFormat, "ARGB1555"))
    {
        pConfig->mBitmapFormat = MM_PIXEL_FORMAT_RGB_1555;
    }
    else
    {
        pConfig->mBitmapFormat = MM_PIXEL_FORMAT_RGB_8888;
    }
    pConfig->overlay_x = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_OVERLAY_X, 0);
    pConfig->overlay_y = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_OVERLAY_Y, 0);
    pConfig->overlay_w = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_OVERLAY_W, 0);
    pConfig->overlay_h = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_OVERLAY_H, 0);


    pConfig->mbAttachToVi = (bool)GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_ATTACH_TO_VI, 0);
    char *AddVenc = (char*)GetConfParaString(&stConfParser, SAMPLE_REGION_KEY_ADD_VENC_CHANNEL, NULL);
    if(!strcmp(AddVenc, "yes"))
    {
        pConfig->add_venc_channel = TRUE;
    }
    else
    {
        pConfig->add_venc_channel = FALSE;
    }
    pConfig->encoder_count = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_ENCODER_COUNT, 5000);
    pConfig->bit_rate = GetConfParaInt(&stConfParser, SAMPLE_REGION_KEY_BIT_RATE, 0);
    char *EncoderType = (char*)GetConfParaString(&stConfParser, SAMPLE_REGION_KEY_ENCODERTYPE, NULL);
    if(!strcmp(EncoderType, "H.264"))
    {
        pConfig->EncoderType = PT_H264;
    }
    else if(!strcmp(EncoderType, "H.265"))
    {
        pConfig->EncoderType = PT_H265;
    }
    else if(!strcmp(EncoderType, "MJPEG"))
    {
        pConfig->EncoderType = PT_MJPEG;
    }
    else
    {
        alogw("unsupported venc type:%p,encoder type turn to H.264!",EncoderType);
        pConfig->EncoderType = PT_H264;
    }
    char *pStr = (char *)GetConfParaString(&stConfParser, SAMPLE_REGION_KEY_OUTPUT_FILE_PATH, NULL);
    strncpy(pConfig->OutputFilePath, pStr, MAX_FILE_PATH_SIZE-1);
    pConfig->OutputFilePath[MAX_FILE_PATH_SIZE-1] = '\0';
    

    destroyConfParser(&stConfParser);
    return SUCCESS;
}

void handle_exit(int signo)
{
    alogd("user want to exit!");
    if(gpSampleIspOsdContext != NULL)
    {
        cdx_sem_up(&gpSampleIspOsdContext->mSemExit);
    }
}

static void *GetEncoderFrameThread(void * pArg)
{
    SampleIspOsdContext *pContext = (SampleIspOsdContext*)pArg;
    int count = 0;
    VENC_STREAM_S VencFrame;
    VENC_PACK_S venc_pack;
    VencFrame.mPackCount = 1;
    VencFrame.mpPack = &venc_pack;
    pContext->mVI2VEncChn = pContext->mVIChn + 1;
    int eRet =  AW_MPI_VI_CreateVirChn(pContext->mVIDev, pContext->mVI2VEncChn, NULL);
    if(eRet != SUCCESS)
    {
        aloge("error:AW_MPI_VI_EnableVipp failed");
    }

    MPP_CHN_S ViChn = {MOD_ID_VIU, pContext->mVIDev, pContext->mVI2VEncChn};
    MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pContext->mVEChn};
    AW_MPI_SYS_Bind(&ViChn,&VeChn);

    eRet = AW_MPI_VI_EnableVirChn(pContext->mVIDev, pContext->mVI2VEncChn);
    if(eRet != SUCCESS)
    {
        aloge("error:enablecirchn failed");
    }

    AW_MPI_VENC_StartRecvPic(pContext->mVEChn);

    //save the video
    while(FALSE == pContext->mbEncThreadExitFlag)
    {
        if(AW_MPI_VENC_GetStream(pContext->mVEChn, &VencFrame, 4000) < 0)
        {
           aloge("get first frame failed!\n");
            continue;
        }
        else
        {
            if(VencFrame.mpPack != NULL && VencFrame.mpPack->mLen0)
            {
                fwrite(VencFrame.mpPack->mpAddr0, 1, VencFrame.mpPack->mLen0, pContext->mOutputFileFp);
            }
            if(VencFrame.mpPack != NULL && VencFrame.mpPack->mLen1)
            {
                fwrite(VencFrame.mpPack->mpAddr1, 1, VencFrame.mpPack->mLen1, pContext->mOutputFileFp);
            }

            AW_MPI_VENC_ReleaseStream(pContext->mVEChn, &VencFrame);
            count++;
        }
    }

    alogd("get [%d]encoded frames!", count);

    AW_MPI_VENC_StopRecvPic(pContext->mVEChn);
    AW_MPI_VI_DisableVirChn(pContext->mVIDev, pContext->mVI2VEncChn); 

    AW_MPI_VENC_ResetChn(pContext->mVEChn);
    AW_MPI_VENC_DestroyChn(pContext->mVEChn);
    AW_MPI_VI_DestoryVirChn(pContext->mVIDev, pContext->mVI2VEncChn);
    
    return NULL;
}


static int CreateIspDebugRgb(RGB_PIC_S *ispRgb, int IspDevId)
{
    int exp_line=-1, gain=-1, lv_idx=-1, color_temp=-1, ev_idx=-1;
	int ev_pos[3] = {-1, -1, -1};
	double  gain_multiple = -1;
    char isp_debug[128];

    //   get ISP_Paramer
    AW_MPI_ISP_AE_GetExposureLine(IspDevId, &exp_line);
    AW_MPI_ISP_AE_GetGain(IspDevId, &gain);
    AW_MPI_ISP_AWB_GetCurColorT(IspDevId, &color_temp);
    AW_MPI_ISP_AE_GetEvIdx(IspDevId, &ev_idx);
    lv_idx = AW_MPI_ISP_GetEnvLV(IspDevId);
    
	ev_pos[1] = ev_idx/25;
	if(ev_pos[1] <= 0)
		ev_pos[0] = 0;
	else
		ev_pos[0] = ev_pos[1] - 1;
	if(ev_pos[1] >= 13)
		ev_pos[2] = 13;
	else
		ev_pos[2] = ev_pos[1] + 1;
	gain_multiple = (double)gain/16;	// 1 2 4 8 16 32 64

	snprintf(isp_debug, sizeof(isp_debug)-1, "exp_line=%d gain=%d(multiple:%.1f) ev_idx=%d(ev_pos:%d, %d, %d) lv_idx=%d colorT=%d",
                      exp_line, gain, gain_multiple, ev_idx, ev_pos[0], ev_pos[1], ev_pos[2], lv_idx, color_temp);

    /*      test interface       */
#if 0
    static int time_s_cnt = 0;
    static int on_off = 1;
    if(++time_s_cnt >= 5)
    {
        time_s_cnt = 0;
//        AW_MPI_ISP_SetMirror(IspDevId, on_off);
//        AW_MPI_ISP_SetFlip(IspDevId, on_off);
//        AW_MPI_ISP_SwitchIspCofig(IspDevId, on_off);
        if(++on_off > 1)
            on_off = 0;
    }
#endif

    FONT_RGBPIC_S font_pic;
    font_pic.font_type     = FONT_SIZE_32;
    font_pic.rgb_type      = OSD_RGB_32;
    font_pic.enable_bg     = 0;
    font_pic.foreground[0] = 0x6;
    font_pic.foreground[1] = 0x1;
    font_pic.foreground[2] = 0xFF;
    font_pic.foreground[3] = 0xFF;
    font_pic.background[0] = 0x21;
    font_pic.background[1] = 0x21;
    font_pic.background[2] = 0x21;
    font_pic.background[3] = 0x11;
    
    ispRgb->enable_mosaic = 0;
    ispRgb->rgb_type      = OSD_RGB_32;
    
    create_font_rectangle(isp_debug, &font_pic, ispRgb);

    return 0;
}

static int IspDebugRbgbUpdateThread(void)
{
    BITMAP_S stBmp;

    while(1)
    {
        CreateIspDebugRgb(&gpSampleIspOsdContext->ispDebugRgb, gpSampleIspOsdContext->mVIDev);
        memset(&stBmp, 0, sizeof(BITMAP_S));
        stBmp.mPixelFormat = MM_PIXEL_FORMAT_RGB_8888;
        stBmp.mWidth  = gpSampleIspOsdContext->ispDebugRgb.wide;
        stBmp.mHeight = gpSampleIspOsdContext->ispDebugRgb.high;
        stBmp.mpData  = gpSampleIspOsdContext->ispDebugRgb.pic_addr;
        AW_MPI_RGN_SetBitMap(gpSampleIspOsdContext->mOverlayHandle + 1, &stBmp);
        release_rgb_picture(&gpSampleIspOsdContext->ispDebugRgb);
        sleep(1);
    }

    return 0;
}

static int IspDebugRgbThreadCreate(pthread_t *threadId)
{
    int ret;

    ret = pthread_create(threadId, NULL, IspDebugRbgbUpdateThread, NULL);
    if(ret <0)
    {
        aloge("error: the IspDebugRgbUpdateThread can not be created");
        return -1;
    }else
        alogd("the IspDebugRgbUpdateThread create ok");
    
    return 0;
}

static int IspDebugRgbInit(RGB_PIC_S *ispRgb, int IspDevId)
{
    int ret;

    ret = load_font_file(FONT_SIZE_32);
    if(ret < 0)
    {
        aloge("load_font_file FONT_SIZE_32 fail! ret:%d\n", ret);
        return -1;
    }

    ret = load_font_file(FONT_SIZE_64);
    if (ret < 0)
    {
        aloge("Do load_font_file FONT_SIZE_64 fail! ret:%d\n", ret);
        return -1;
    }

    CreateIspDebugRgb(ispRgb, IspDevId);
    
    return 0;
}



int main(int argc, char **argv)
{
    int result = 0;
    BOOL bAddVencChannel = FALSE;
    ERRORTYPE eRet = SUCCESS;


    
    SampleIspOsdContext *pContext = (SampleIspOsdContext*)malloc(sizeof(SampleIspOsdContext));
    if(NULL == pContext)
    {
        alogd("malloc fail!");
        return -1;
    }
    initSampleIspOsdContext(pContext);
    gpSampleIspOsdContext = pContext;

    
    if(ParseCmdLine(argc, argv, &pContext->mCmdLinePara) != 0)
    {
        result = -1;
        goto _exit;
        
    }
    char *pConfigFilePath = NULL;
    if(strlen(pContext->mCmdLinePara.mConfigFilePath) > 0)
    {
        pConfigFilePath = pContext->mCmdLinePara.mConfigFilePath;
    }
    if(loadSampleIspOsdConfig(&pContext->mConfigPara, pConfigFilePath) != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        result = -1;
        goto _exit;
    }
    alogw("=============================================>");
    bAddVencChannel = pContext->mConfigPara.add_venc_channel;
    if(bAddVencChannel)
    {
        alogd("Had add the VencChanel, add VI_To_Venc");
    }
    alogw("=============================================>");


    memset(&pContext->mSysConf, 0, sizeof(MPP_SYS_CONF_S));
    pContext->mSysConf.nAlignWidth = 32;
    AW_MPI_SYS_SetConf(&pContext->mSysConf);
    AW_MPI_SYS_Init();

    pContext->mVIDev = 0;
    pContext->mVIChn = 0;
    eRet = AW_MPI_VI_CreateVipp(pContext->mVIDev);
    if(eRet != SUCCESS)
    {
        aloge("error:AW_MPI_VI_CreateVipp failed");
    }

    VI_ATTR_S attr;
    eRet = AW_MPI_VI_GetVippAttr(pContext->mVIDev, &attr);
    if(eRet != SUCCESS)
    {
        aloge("error:AW_MPI_VI_GetVippAttr failed");
    }
    memset(&attr, 0, sizeof(VI_ATTR_S));
    attr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
    attr.memtype = V4L2_MEMORY_MMAP;
    attr.format.pixelformat = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(pContext->mConfigPara.mPicFormat);
    attr.format.field = V4L2_FIELD_NONE;
    attr.format.width = pContext->mConfigPara.mCaptureWidth;
    attr.format.height = pContext->mConfigPara.mCaptureHeight;
    attr.nbufs = 4;//10;
    attr.nplanes = 2;
    attr.fps = 30;
    attr.wdr_mode = 0;
    eRet = AW_MPI_VI_SetVippAttr(pContext->mVIDev, &attr);
    if(eRet != SUCCESS)
    {
        aloge("error:AW_MPI_SetVippAttr failed");
    }

    eRet = AW_MPI_VI_EnableVipp(pContext->mVIDev);
    if(eRet != SUCCESS)
    {
        aloge("error:AW_MPI_VI_EnableVipp failed");
    }

    AW_MPI_ISP_Init();
    AW_MPI_ISP_Run(0);

    eRet = AW_MPI_VI_CreateVirChn(pContext->mVIDev, pContext->mVIChn, NULL);
    if(eRet != SUCCESS)
    {
        aloge("error:AW_MPI_VI_CreateVirChn failed ");
    }


    IspDebugRgbInit(&pContext->ispDebugRgb, pContext->mVIDev);
    
    
    if(bAddVencChannel)
    {
        pContext->mVEChn = 0;
        int VIFrameRate = pContext->mConfigPara.mFrameRate;
        int VencFrameRate = pContext->mConfigPara.mFrameRate;
        memset(&pContext->mVEncChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
        SIZE_S wantedVideoSize = {pContext->mConfigPara.mCaptureWidth, pContext->mConfigPara.mCaptureHeight};
        SIZE_S videosize = {pContext->mConfigPara.mCaptureWidth, pContext->mConfigPara.mCaptureHeight};
        PAYLOAD_TYPE_E videoCodec = pContext->mConfigPara.EncoderType;
        PIXEL_FORMAT_E wantedPreviewFormat = pContext->mConfigPara.mPicFormat;
        int wantedFrameRate = pContext->mConfigPara.mFrameRate;
        pContext->mVEncChnAttr.VeAttr.Type = videoCodec;
        pContext->mVEncChnAttr.VeAttr.MaxKeyInterval = wantedFrameRate;
        pContext->mVEncChnAttr.VeAttr.SrcPicWidth = videosize.Width;
        pContext->mVEncChnAttr.VeAttr.SrcPicHeight = videosize.Height;
        pContext->mVEncChnAttr.VeAttr.Field = VIDEO_FIELD_FRAME;
        pContext->mVEncChnAttr.VeAttr.PixelFormat = wantedPreviewFormat;
        int wantedVideoBitRate = pContext->mConfigPara.bit_rate;
        if(PT_H264 == pContext->mVEncChnAttr.VeAttr.Type)
        {
            pContext->mVEncChnAttr.VeAttr.AttrH264e.bByFrame = TRUE;
            pContext->mVEncChnAttr.VeAttr.AttrH264e.Profile = 2;
            pContext->mVEncChnAttr.VeAttr.AttrH264e.mLevel = VENC_H264Level51;
            pContext->mVEncChnAttr.VeAttr.AttrH264e.PicWidth = wantedVideoSize.Width;
            pContext->mVEncChnAttr.VeAttr.AttrH264e.PicHeight = wantedVideoSize.Height;
            pContext->mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
            pContext->mVEncChnAttr.RcAttr.mAttrH264Cbr.mBitRate = wantedVideoBitRate;
            pContext->mVEncChnAttr.RcAttr.mAttrH264Cbr.mMaxQp = 51;
            pContext->mVEncChnAttr.RcAttr.mAttrH264Cbr.mMinQp = 1;
        }
        else if(PT_H265 == pContext->mVEncChnAttr.VeAttr.Type)
        {
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mbByFrame = TRUE;
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mProfile = 0;
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mLevel = VENC_H265Level62;
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mPicWidth = wantedVideoSize.Width;
            pContext->mVEncChnAttr.VeAttr.AttrH265e.mPicHeight = wantedVideoSize.Height;
            pContext->mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
            pContext->mVEncChnAttr.RcAttr.mAttrH265Cbr.mBitRate = wantedVideoBitRate;
            pContext->mVEncChnAttr.RcAttr.mAttrH265Cbr.mMaxQp = 51;
            pContext->mVEncChnAttr.RcAttr.mAttrH265Cbr.mMinQp = 1;
        }
            else if(PT_MJPEG== pContext->mVEncChnAttr.VeAttr.Type)
            {
                pContext->mVEncChnAttr.VeAttr.AttrMjpeg.mbByFrame = TRUE;
                pContext->mVEncChnAttr.VeAttr.AttrMjpeg.mPicWidth = wantedVideoSize.Width;
                pContext->mVEncChnAttr.VeAttr.AttrMjpeg.mPicHeight = wantedVideoSize.Height;
                pContext->mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
                pContext->mVEncChnAttr.RcAttr.mAttrMjpegeCbr.mBitRate = wantedVideoBitRate;
            }

        eRet =  AW_MPI_VENC_CreateChn(pContext->mVEChn, &pContext->mVEncChnAttr);
        if(eRet != SUCCESS)
        {
            aloge("error:AW_MPI_VENC_CreateChn failed");
            goto _exit;
        
        }

        VENC_FRAME_RATE_S stFrameRate;
        stFrameRate.SrcFrmRate = VIFrameRate;
        stFrameRate.DstFrmRate = VencFrameRate;
        AW_MPI_VENC_SetFrameRate(pContext->mVEChn, &stFrameRate);

        VencHeaderData vencheader;
        pContext->mOutputFileFp = fopen(pContext->mConfigPara.OutputFilePath,"wb+");
        if(!pContext->mOutputFileFp)
        {
            aloge("error:can not open the file");
            goto _exit;
        }
        if(PT_H264 == pContext->mVEncChnAttr.VeAttr.Type)
        {
            AW_MPI_VENC_GetH264SpsPpsInfo(pContext->mVEChn, &vencheader);
            if(vencheader.nLength)
            {
                fwrite(vencheader.pBuffer, vencheader.nLength, 1, pContext->mOutputFileFp);
            }
        }
        else if(PT_H265 == pContext->mVEncChnAttr.VeAttr.Type)
        {
            AW_MPI_VENC_GetH265SpsPpsInfo(pContext->mVEChn, &vencheader);
            if(vencheader.nLength)
            {
                fwrite(vencheader.pBuffer, vencheader.nLength, 1, pContext->mOutputFileFp);
            }
        }
    
    }
     
    //bind vechn 	    
    MPP_CHN_S VeChn = {MOD_ID_VENC, 0, pContext->mVEChn};

    if(bAddVencChannel)
    {
        if(pthread_create(&pContext->mEncThreadId, NULL, GetEncoderFrameThread, pContext) < 0)
        {
            aloge("error: the pthread can not be created");
        }
        alogw("the pthread[0x%x] of venc had created", pContext->mEncThreadId);
    }

    IspDebugRgbThreadCreate(&pContext->ispThreadId);

    //test the region
    if(pContext->mConfigPara.overlay_h != 0 && pContext->mConfigPara.overlay_w != 0)
    {
        RGN_ATTR_S stRegion;
        BITMAP_S stBmp;
        RGN_CHN_ATTR_S stRgnChnAttr ;

        if(bAddVencChannel)
        {
            //add to venchn
            memset(&stRegion, 0, sizeof(RGN_ATTR_S));
            stRegion.enType = OVERLAY_RGN;
            stRegion.unAttr.stOverlay.mPixelFmt = MM_PIXEL_FORMAT_RGB_8888;   //MM_PIXEL_FORMAT_RGB_8888;
            stRegion.unAttr.stOverlay.mSize.Width = pContext->ispDebugRgb.wide;
            stRegion.unAttr.stOverlay.mSize.Height = pContext->ispDebugRgb.high;
            AW_MPI_RGN_Create(pContext->mOverlayHandle + 1, &stRegion);
                     
            memset(&stBmp, 0, sizeof(BITMAP_S));
            stBmp.mPixelFormat = stRegion.unAttr.stOverlay.mPixelFmt;
            stBmp.mWidth = stRegion.unAttr.stOverlay.mSize.Width;
            stBmp.mHeight = stRegion.unAttr.stOverlay.mSize.Height;
            stBmp.mWidth  = pContext->ispDebugRgb.wide;
            stBmp.mHeight = pContext->ispDebugRgb.high;
            stBmp.mpData  = pContext->ispDebugRgb.pic_addr;            // ALien
            if(NULL == stBmp.mpData)        
            {            
                aloge("fatal error! malloc fail!");        
            }        
            AW_MPI_RGN_SetBitMap(pContext->mOverlayHandle + 1, &stBmp);
            stRgnChnAttr.bShow = TRUE;
            stRgnChnAttr.enType = stRegion.enType;
            stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.X = 32;
            stRgnChnAttr.unChnAttr.stOverlayChn.stPoint.Y = 960;
            stRgnChnAttr.unChnAttr.stOverlayChn.mLayer = 0;
            stRgnChnAttr.unChnAttr.stOverlayChn.mFgAlpha = 0x5C; // global alpha mode value for ARGB1555
            stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Width = 16;
            stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Height = 16;
            stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.mLumThresh = 60;
            stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod = LESSTHAN_LUMDIFF_THRESH;
            stRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn = FALSE;
            AW_MPI_RGN_AttachToChn(pContext->mOverlayHandle + 1,  &VeChn, &stRgnChnAttr);
        }
    }


    if(pContext->mConfigPara.mTestDuration > 0)
    {
        cdx_sem_down_timedwait(&pContext->mSemExit, pContext->mConfigPara.mTestDuration * 1000);
    }
    else
    {
        cdx_sem_down(&pContext->mSemExit);
    }

    if(bAddVencChannel)
    {
        pContext->mbEncThreadExitFlag = TRUE;
        int eError = 0;
        pthread_join(pContext->mEncThreadId, (void*) &eError);
    }


    AW_MPI_RGN_DetachFromChn(pContext->mOverlayHandle + 1, &VeChn);
    AW_MPI_RGN_Destroy(pContext->mOverlayHandle + 1);
    
    
_exit: 
	AW_MPI_ISP_Stop(0);
	AW_MPI_ISP_Exit();

    //stop vo channel, clock channel, vi channel,
    AW_MPI_VENC_StopRecvPic(pContext->mVEChn);
 	AW_MPI_VI_DisableVirChn(pContext->mVIDev, pContext->mVIChn);
    AW_MPI_VI_DisableVirChn(pContext->mVIDev, pContext->mVIChn + 1);    
    AW_MPI_VENC_ResetChn(pContext->mVEChn);

 	pContext->mClockChn = MM_INVALID_CHN;
    AW_MPI_VENC_DestroyChn(pContext->mVEChn);
    AW_MPI_VI_DestoryVirChn(pContext->mVIDev, pContext->mVIChn);
    AW_MPI_VI_DestoryVirChn(pContext->mVIDev, pContext->mVIChn + 1);
    AW_MPI_VI_DisableVipp(pContext->mVIDev);
    AW_MPI_VI_DestoryVipp(pContext->mVIDev);
    pContext->mVIDev = MM_INVALID_DEV;
    pContext->mVIChn = MM_INVALID_CHN;
    pContext->mVEChn = MM_INVALID_CHN;

    //exit mpp system
    AW_MPI_SYS_Exit();
    fclose(pContext->mOutputFileFp);

    destroySampleIspOsdContext(pContext);
    free(pContext);
    pContext = NULL;
    if (result == 0) {
        printf("sample_region exit!\n");
    }
    return result;
}


