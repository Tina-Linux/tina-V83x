/******************************************************************************
  Copyright (C), 2001-2017, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     : sample_virvi2fish2vo.c
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2017/1/5
  Last Modified :
  Description   : mpp component implement
  Function List :
  History       :
******************************************************************************/

#define LOG_TAG "sample_virvi2fish2vo"
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

#include <vo/hwdisplay.h>
#include "media/mpi_vi.h"
#include "media/mpi_ise.h"
#include "media/mpi_sys.h"
#include "media/mpi_isp.h"
#include "media/mpi_vo.h"

#include <confparser.h>
#include "sample_virvi2fish2vo.h"
#include "sample_virvi2fish2vo_config.h"

#define Len_110             0  //lens angle in undistort mode
#define Len_130             0
#define Fish_Len_1          0
#define Fish_Len_2          1

typedef struct awVI_pCap_S {
    VI_DEV VI_Dev;
    VI_CHN VI_Chn;
    MPP_CHN_S VI_CHN_S;
    AW_S32 s32MilliSec;
    VIDEO_FRAME_INFO_S pstFrameInfo;
    VI_ATTR_S stAttr;
} VIRVI_Cap_S;

typedef struct awISE_PortCap_S {
    MPP_CHN_S ISE_Port_S;
    FILE *OutputFilePath;
    ISE_CHN_ATTR_S PortAttr;
}ISE_PortCap_S;

typedef struct awISE_pGroupCap_S {
    ISE_GRP ISE_Group;
    MPP_CHN_S ISE_Group_S;
    ISE_GROUP_ATTR_S pGrpAttr;
    ISE_CHN ISE_Port[4];
    ISE_PortCap_S *PortCap_S[4];
}ISE_GroupCap_S;

typedef struct awVO_pCap_S {
    VO_CHN mVOChn;
    VO_DEV mVODev;
    MPP_CHN_S VO_CHN_S;
    int hlay0;
    int mUILayer;
    int mTestDuration;  //unit:s, 0 mean infinite
    cdx_sem_t mSemExit;
    MPP_SYS_CONF_S mSysConf;
    VO_LAYER mVoLayer;
    VO_VIDEO_LAYER_ATTR_S mLayerAttr;
}VO_Cap_S;

static ERRORTYPE SampleVirvi2Fish2VO_VOCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    ERRORTYPE ret = SUCCESS;
    VO_Cap_S *pVOCap = (VO_Cap_S*)cookie;
    if(MOD_ID_VOU == pChn->mModId)
    {
        if(pChn->mChnId != pVOCap->mVOChn)
        {
            aloge("fatal error! VO chnId[%d]!=[%d]", pChn->mChnId, pVOCap->mVOChn);
        }
        switch(event)
        {
            case MPP_EVENT_RELEASE_VIDEO_BUFFER:
            {
                aloge("fatal error! sample_virvi2fish2vo use binding mode!");
                break;
            }
            case MPP_EVENT_SET_VIDEO_SIZE:
            {
                SIZE_S *pDisplaySize = (SIZE_S*)pEventData;
                alogd("vo report video display size[%dx%d]", pDisplaySize->Width, pDisplaySize->Height);
                break;
            }
            case MPP_EVENT_RENDERING_START:
            {
                alogd("vo report rendering start");
                break;
            }
            default:
            {
                //postEventFromNative(this, event, 0, 0, pEventData);
                aloge("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!",
                        event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                ret = ERR_VO_ILLEGAL_PARAM;
                break;
            }
        }
    }
    else
    {
        aloge("fatal error! why modId[0x%x]?", pChn->mModId);
        ret = FAILURE;
    }
    return ret;
}

/*MPI VI*/
int aw_vipp_creat(VI_DEV ViDev, VI_ATTR_S *pstAttr)
{
    int ret = -1;
    ret = AW_MPI_VI_CreateVipp(ViDev);
    if(ret < 0)
    {
        aloge("Create vi dev[%d] falied!", ViDev);
        return ret;
    }
    ret = AW_MPI_VI_SetVippAttr(ViDev, pstAttr);
    if(ret < 0)
    {
        aloge("Set vi attr[%d] falied!", ViDev);
        return ret;
    }
    ret = AW_MPI_VI_EnableVipp(ViDev);
    if(ret < 0)
    {
        aloge("Enable vi dev[%d] falied!", ViDev);
        return ret;
    }
    AW_MPI_ISP_Init();
    if(ViDev == 0 || ViDev == 2)
        AW_MPI_ISP_Run(1); // 3A ini
    if(ViDev == 1 || ViDev == 3)
        AW_MPI_ISP_Run(0); // 3A ini
    return 0;
}

int aw_virvi_creat(VI_DEV ViDev, VI_CHN ViCh, void *pAttr)
{
    int ret = -1;
    ret = AW_MPI_VI_CreateVirChn(ViDev, ViCh, pAttr);
    if(ret < 0)
    {
        aloge("Create VI Chn failed,VIDev = %d,VIChn = %d",ViDev,ViCh);
        return ret ;
    }
    ret = AW_MPI_VI_SetVirChnAttr(ViDev, ViCh, pAttr);
    if(ret < 0)
    {
        aloge("Set VI ChnAttr failed,VIDev = %d,VIChn = %d",ViDev,ViCh);
        return ret ;
    }
    return 0;
}

int aw_vipp_destory(VI_DEV ViDev)
{
    int ret = -1;
    ret = AW_MPI_VI_DisableVipp(ViDev);
    if(ret < 0)
    {
        aloge("Disable vi dev[%d] falied!", ViDev);
        return ret;
    }
    ret = AW_MPI_VI_DestoryVipp(ViDev);
    if(ret < 0)
    {
        aloge("Destory vi dev[%d] falied!", ViDev);
        return ret;
    }
    return 0;
}

int aw_virvi_destory(VI_DEV ViDev, VI_CHN ViCh)
{
    int ret = -1;
    ret = AW_MPI_VI_DestoryVirChn(ViDev, ViCh);
    if(ret < 0)
    {
        aloge("Destory VI Chn failed,VIDev = %d,VIChn = %d",ViDev,ViCh);
        return ret ;
    }
    return 0;
}

/*MPI ISE*/
int aw_iseport_creat(ISE_GRP IseGrp, ISE_CHN IsePort, ISE_CHN_ATTR_S *PortAttr)
{
    int ret = 0;
    ret = AW_MPI_ISE_CreatePort(IseGrp, IsePort, PortAttr);
    if(ret < 0)
    {
        aloge("Create ISE Port failed,IseGrp= %d,IsePort = %d",IseGrp,IsePort);
        return ret ;
    }
    ret = AW_MPI_ISE_SetPortAttr(IseGrp,IsePort,PortAttr);
    if(ret < 0)
    {
        aloge("Set ISE PortAttr failed,IseGrp= %d,IsePort = %d",IseGrp,IsePort);
        return ret ;
    }
    return 0;
}

int aw_iseport_destory(ISE_GRP IseGrp, ISE_CHN IsePort)
{
    int ret = -1;
    ret = AW_MPI_ISE_DestroyPort(IseGrp,IsePort);
    if(ret < 0)
    {
        aloge("Destory ISE Port failed, IseGrp= %d,IsePort = %d",IseGrp,IsePort);
        return ret ;
    }
    return 0;
}

int aw_isegroup_creat(ISE_GRP IseGrp, ISE_GROUP_ATTR_S *pGrpAttr)
{
    int ret = -1;
    ret = AW_MPI_ISE_CreateGroup(IseGrp, pGrpAttr);
    if(ret < 0)
    {
        aloge("Create ISE Group failed, IseGrp= %d",IseGrp);
        return ret ;
    }
    ret = AW_MPI_ISE_SetGrpAttr(IseGrp, pGrpAttr);
    if(ret < 0)
    {
        aloge("Set ISE GrpAttr failed, IseGrp= %d",IseGrp);
        return ret ;
    }
    return 0;
}

int aw_isegroup_destory(ISE_GRP IseGrp)
{
    int ret = -1;
    ret = AW_MPI_ISE_DestroyGroup(IseGrp);
    if(ret < 0)
    {
        aloge("Destroy ISE Group failed, IseGrp= %d",IseGrp);
        return ret ;
    }
    return 0;
}

/*MPI VO*/
int aw_vo_dev_creat(VO_Cap_S* pVoCap)
{
    int ret = -1;
    ret = AW_MPI_VO_Enable(pVoCap->mVODev);
    if(ret < 0)
    {
        aloge("Vo Dev%d creat failed",pVoCap->mVODev);
        return ret ;
    }
    ret = AW_MPI_VO_AddOutsideVideoLayer(pVoCap->mUILayer);
    if(ret < 0)
    {
        aloge("Vo add UILayer%d failed",pVoCap->mUILayer);
        return ret ;
    }
    ret = AW_MPI_VO_CloseVideoLayer(pVoCap->mUILayer);  //close ui layer.
    if(ret < 0)
    {
        aloge("Vo close UILayer%d failed",pVoCap->mUILayer);
        return ret ;
    }
    //enable vo layer
    while(pVoCap->hlay0 < VO_MAX_LAYER_NUM)
    {
        if(SUCCESS == AW_MPI_VO_EnableVideoLayer(pVoCap->hlay0))
        {
            break;
        }
        pVoCap->hlay0++;
    }
    if(pVoCap->hlay0 >= VO_MAX_LAYER_NUM)
    {
        aloge("fatal error! enable video layer%d fail!",pVoCap->hlay0);
    }
    ret = AW_MPI_VO_SetVideoLayerAttr(pVoCap->mVoLayer, &pVoCap->mLayerAttr);
    if(ret < 0)
    {
        aloge("Vo Layer%d Set LayerAttr failed",pVoCap->mVoLayer);
        return ret;
    }
    return 0;
}

int aw_vo_chn_creat(VO_Cap_S* pVoCap)
{
    BOOL bSuccessFlag = FALSE;
    int ret = -1;
    while(pVoCap->mVOChn < VO_MAX_CHN_NUM)
    {
        ret = AW_MPI_VO_EnableChn(pVoCap->mVoLayer, pVoCap->mVOChn);
        if(SUCCESS == ret)
        {
            bSuccessFlag = TRUE;
            alogd("create vo channel[%d] success!", pVoCap->mVOChn);
            break;
        }
        else if(ERR_VO_CHN_NOT_DISABLE == ret)
        {
            alogd("vo channel[%d] is exist, find next!", pVoCap->mVOChn);
            pVoCap->mVOChn++;
        }
        else
        {
            aloge("fatal error! create vo channel[%d] ret[0x%x]!", pVoCap->mVOChn, ret);
            break;
        }
    }
    if(FALSE == bSuccessFlag)
    {
        pVoCap->mVOChn = MM_INVALID_CHN;
        aloge("fatal error! create vo channel fail!");
        return ret;
    }
    MPPCallbackInfo cbInfo;
    cbInfo.cookie = (void*)pVoCap;
    cbInfo.callback = (MPPCallbackFuncType)&SampleVirvi2Fish2VO_VOCallbackWrapper;
    ret = AW_MPI_VO_RegisterCallback(pVoCap->mVoLayer, pVoCap->mVOChn, &cbInfo);
    if(ret < 0)
    {
        aloge("fatal error! vo channel[%d] register callback failed!", pVoCap->mVOChn);
        return ret;
    }
    ret = AW_MPI_VO_SetChnDispBufNum(pVoCap->mVoLayer, pVoCap->mVOChn, 0);
    if(ret < 0)
    {
        aloge("fatal error! vo channel[%d] set dispBufNum failed!", pVoCap->mVOChn);
        return ret;
    }
    return 0;
}

int aw_vo_chn_destory(VO_Cap_S* pVoCap)
{
    int ret = -1;
    ret = AW_MPI_VO_DisableChn(pVoCap->mVoLayer, pVoCap->mVOChn);
    if(ret < 0)
    {
        aloge("Disable Vo Chn failed,VoChn = %d",pVoCap->mVOChn);
        return ret ;
    }
    ret = AW_MPI_VO_DisableVideoLayer(pVoCap->mVoLayer);
    if(ret < 0)
    {
        aloge("Disable Layer failed,Layer = %d",pVoCap->mVoLayer);
        return ret ;
    }
    ret =  AW_MPI_VO_RemoveOutsideVideoLayer(pVoCap->mUILayer);
    if(ret < 0)
    {
        aloge("Remove Layer failed,Layer = %d",pVoCap->mUILayer);
        return ret ;
    }
    return ret;
}

int aw_vo_dev_destory(VO_Cap_S* pVoCap)
{
    int ret = -1;
    ret = AW_MPI_VO_Disable(pVoCap->mVODev);
    if(ret < 0)
    {
        aloge("Disable Vo Dev failed,Dev = %d",pVoCap->mVODev);
        return ret ;
    }
    return ret;
}

static int ParseCmdLine(int argc, char **argv, SampleVirvi2Fish2VoCmdLineParam *pCmdLinePara)
{
    alogd("sample_virvi2fish2vo path:[%s], arg number is [%d]", argv[0], argc);
    int ret = 0;
    int i=1;
    memset(pCmdLinePara, 0, sizeof(SampleVirvi2Fish2VoCmdLineParam));
    while(i < argc)
    {
        if(!strcmp(argv[i], "-path"))
        {
            if(++i >= argc)
            {
                aloge("fatal error! use -h to learn how to set parameter!!!");
                ret = -1;
                break;
            }
            if(strlen(argv[i]) >= MAX_FILE_PATH_SIZE)
            {
                aloge("fatal error! file path[%s] too long: [%d]>=[%d]!", argv[i], strlen(argv[i]), MAX_FILE_PATH_SIZE);
            }
            strncpy(pCmdLinePara->mConfigFilePath, argv[i], MAX_FILE_PATH_SIZE-1);
            pCmdLinePara->mConfigFilePath[MAX_FILE_PATH_SIZE-1] = '\0';
        }
        else if(!strcmp(argv[i], "-h"))
        {
            alogd("CmdLine param:\n"
                "\t-path /home/sample_virvi2fish2vo.conf\n");
            ret = 1;
            break;
        }
        else
        {
            alogd("ignore invalid CmdLine param:[%s], type -h to get how to set parameter!", argv[i]);
        }
        i++;
    }
    return ret;
}

static ERRORTYPE loadSampleVirvi2Fish2VoConfig(SampleVirvi2Fish2VoConfig *pConfig, const char *conf_path)
{
    int ret;
    char *ptr = NULL;
    char *pStrPixelFormat = NULL,*EncoderType = NULL;
    int i = 0,ISEPortNum = 0;
    CONFPARSER_S stConfParser;
    char name[256];
    ret = createConfParser(conf_path, &stConfParser);
    if(ret < 0)
    {
        aloge("load conf fail");
        return FAILURE;
    }
    memset(pConfig, 0, sizeof(SampleVirvi2Fish2VoConfig));
    pConfig->AutoTestCount = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_Auto_Test_Count, 0);

    /* Get VI parameter*/
    pConfig->VIDevConfig.DevId = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_Dev_ID, 0);
    pConfig->VIDevConfig.SrcFrameRate = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_Src_Frame_Rate, 0);
    pConfig->VIDevConfig.SrcWidth = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_Src_Width, 0);
    pConfig->VIDevConfig.SrcHeight = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_Src_Height, 0);
    if(pConfig->VIDevConfig.SrcWidth % 32 != 0 || pConfig->VIDevConfig.SrcHeight % 32 != 0)
    {
        aloge("fatal error! vi src width and src height must multiple of 32,width = %d,height = %d\n",
                pConfig->VIDevConfig.SrcWidth,pConfig->VIDevConfig.SrcHeight);
        return FAILURE;
    }
    printf("VI Parameter:dev_id = %d, src_width = %d, src_height = %d, src_frame_rate = %d\n",
       pConfig->VIDevConfig.DevId,pConfig->VIDevConfig.SrcWidth,
       pConfig->VIDevConfig.SrcHeight,pConfig->VIDevConfig.SrcFrameRate);

    /* Get ISE parameter*/
    pConfig->ISEGroupConfig.ISE_Dewarp_Mode = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_ISE_Dewarp_Mode, 0);
    pConfig->ISEGroupConfig.Lens_Parameter_P = pConfig->VIDevConfig.SrcWidth/3.1415;
    pConfig->ISEGroupConfig.Lens_Parameter_Cx = pConfig->VIDevConfig.SrcWidth/2;
    pConfig->ISEGroupConfig.Lens_Parameter_Cy = pConfig->VIDevConfig.SrcHeight/2;
    pConfig->ISEGroupConfig.Mount_Mode = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_ISE_Mount_Mode, 0);
    if(pConfig->ISEGroupConfig.ISE_Dewarp_Mode == WARP_NORMAL)
    {
        pConfig->ISEGroupConfig.normal_pan = (float)GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_ISE_NORMAL_Pan, 0);
        pConfig->ISEGroupConfig.normal_tilt = (float)GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_ISE_NORMAL_Tilt, 0);
        pConfig->ISEGroupConfig.normal_zoom = (float)GetConfParaInt(&stConfParser,   SAMPLE_Virvi2Fish2Vo_ISE_NORMAL_Zoom, 0);
        printf("ISE Group Parameter:dewarp_mode = %d,Lens_Parameter_p = %f,Lens_Parameter_cx = %d,Lens_Parameter_cy = %d,"
                "mount_mode = %d,pan = %f,tilt = %f,zoom = %f\n",pConfig->ISEGroupConfig.ISE_Dewarp_Mode,
                pConfig->ISEGroupConfig.Lens_Parameter_P,pConfig->ISEGroupConfig.Lens_Parameter_Cx,
                pConfig->ISEGroupConfig.Lens_Parameter_Cy,pConfig->ISEGroupConfig.Mount_Mode,
                pConfig->ISEGroupConfig.normal_pan,pConfig->ISEGroupConfig.normal_tilt,pConfig->ISEGroupConfig.normal_zoom);
    }
    else if(pConfig->ISEGroupConfig.ISE_Dewarp_Mode == WARP_PTZ4IN1)
    {
        for(i = 0;i < 4;i++)
        {
            snprintf(name, 256, "ise_ptz4in1_pan_%d", i);
            pConfig->ISEGroupConfig.ptz4in1_pan[i] = (float)GetConfParaInt(&stConfParser, name, 0);
            snprintf(name, 256, "ise_ptz4in1_tilt_%d", i);
            pConfig->ISEGroupConfig.ptz4in1_tilt[i] = (float)GetConfParaInt(&stConfParser, name, 0);
            snprintf(name, 256, "ise_ptz4in1_zoom_%d", i);
            pConfig->ISEGroupConfig.ptz4in1_zoom[i] = (float)GetConfParaInt(&stConfParser, name, 0);
            alogd("ptz4in1_Sub%d:pan = %f,tilt = %f,zoom = %f",i,
                  pConfig->ISEGroupConfig.ptz4in1_pan[i],pConfig->ISEGroupConfig.ptz4in1_tilt[i],
                  pConfig->ISEGroupConfig.ptz4in1_zoom[i]);
        }
    }
    alogd("ISE Group Parameter:dewarp_mode = %d,mount_mode = %d,Lens_Parameter_p = %f,"
          "Lens_Parameter_cx = %f,Lens_Parameter_cy = %f",
          pConfig->ISEGroupConfig.ISE_Dewarp_Mode,pConfig->ISEGroupConfig.Mount_Mode,
          pConfig->ISEGroupConfig.Lens_Parameter_P,pConfig->ISEGroupConfig.Lens_Parameter_Cx,
          pConfig->ISEGroupConfig.Lens_Parameter_Cy);

    pConfig->ISEPortConfig[0].ISEWidth = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_ISE_Width, 0);
    pConfig->ISEPortConfig[0].ISEHeight = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_ISE_Height, 0);
    pConfig->ISEPortConfig[0].ISEStride = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_ISE_Stride, 0);
    pConfig->ISEPortConfig[0].flip_enable = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_ISE_Flip, 0);
    pConfig->ISEPortConfig[0].mirror_enable = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_ISE_Mirror, 0);
    printf("ISE Port0 Parameter:ISE_Width = %d,ISE_Height = %d,flip_enable = %d,mirror_enable = %d\n",
            pConfig->ISEPortConfig[0].ISEWidth,pConfig->ISEPortConfig[0].ISEHeight,
            pConfig->ISEPortConfig[0].flip_enable,pConfig->ISEPortConfig[0].mirror_enable);

    /* Get VO parameter*/
    pConfig->VOChnConfig.display_width = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_Display_Width, 0);
    pConfig->VOChnConfig.display_height = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_Display_Height, 0);
    pConfig->VOChnConfig.mTestDuration = GetConfParaInt(&stConfParser, SAMPLE_Virvi2Fish2Vo_Vo_Test_Duration, 0);
    printf("VO Parameter:display_width = %d, display_height = %d, mTestDuration = %d\n",
            pConfig->VOChnConfig.display_width,pConfig->VOChnConfig.display_height,
            pConfig->VOChnConfig.mTestDuration);
    destroyConfParser(&stConfParser);
    return SUCCESS;
}

void Virvi2Fish2Vo_HELP()
{
    printf("Run CSI0/CSI1+ISE+VO command: ./sample_virvi2fish2vo -path ./sample_virvi2fish2vo.conf\r\n");
}

int main(int argc, char *argv[])
{
    int ret = 0, i = 0, j = 0,count = 0;
    int result = 0;
    int AutoTestCount = 0;
    printf("sample_virvi2fish2vo build time = %s, %s.\r\n", __DATE__, __TIME__);
    if (argc != 3)
    {
        Virvi2Fish2Vo_HELP();
        exit(0);
    }

    VIRVI_Cap_S    pVICap[MAX_VIR_CHN_NUM];
    ISE_GroupCap_S pISEGroupCap[ISE_MAX_GRP_NUM];
    ISE_PortCap_S  pISEPortCap[ISE_MAX_CHN_NUM];
    VO_Cap_S       pVOCap[VO_MAX_CHN_NUM];

    SampleVirvi2Fish2VoConfparser stContext;
    //parse command line param,read sample_virvi2fish2vo.conf
    if(ParseCmdLine(argc, argv, &stContext.mCmdLinePara) != 0)
    {
        aloge("fatal error! command line param is wrong, exit!");
        result = -1;
        goto _exit;
    }
    char *pConfigFilePath;
    if(strlen(stContext.mCmdLinePara.mConfigFilePath) > 0)
    {
        pConfigFilePath = stContext.mCmdLinePara.mConfigFilePath;
    }
    else
    {
        pConfigFilePath = DEFAULT_SAMPLE_VIRVI2FISH2VO_CONF_PATH;
    }
    //parse config file.
    if(loadSampleVirvi2Fish2VoConfig(&stContext.mConfigPara, pConfigFilePath) != SUCCESS)
    {
        aloge("fatal error! no config file or parse conf file fail");
        result = -1;
        goto _exit;
    }
    AutoTestCount = stContext.mConfigPara.AutoTestCount;

    while (count != AutoTestCount)
    {
        /*Set VI Channel Attribute*/
        memset(&pVICap[0], 0, sizeof(VIRVI_Cap_S));
        pVICap[0].stAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
        pVICap[0].stAttr.memtype = V4L2_MEMORY_MMAP;
        pVICap[0].stAttr.format.pixelformat = V4L2_PIX_FMT_NV21M;
        pVICap[0].stAttr.format.field = V4L2_FIELD_NONE;
        pVICap[0].stAttr.format.colorspace = V4L2_COLORSPACE_JPEG;
        pVICap[0].stAttr.format.width = stContext.mConfigPara.VIDevConfig.SrcWidth;
        pVICap[0].stAttr.format.height = stContext.mConfigPara.VIDevConfig.SrcHeight;
        pVICap[0].stAttr.nbufs = 5;
        pVICap[0].stAttr.nplanes = 2;
        pVICap[0].stAttr.fps = stContext.mConfigPara.VIDevConfig.SrcFrameRate;
        pVICap[0].VI_Dev = stContext.mConfigPara.VIDevConfig.DevId;
        pVICap[0].VI_Chn = 0;
        pVICap[0].s32MilliSec = 5000;
        /*vi bind channel attr*/
        pVICap[0].VI_CHN_S.mChnId = 0;
        pVICap[0].VI_CHN_S.mDevId = stContext.mConfigPara.VIDevConfig.DevId;
        pVICap[0].VI_CHN_S.mModId = MOD_ID_VIU;

        /*Set ISE Group Attribute*/
        memset(&pISEGroupCap[0], 0, sizeof(ISE_GroupCap_S));
        for(j = 0;j < 1;j++)
        {
            pISEGroupCap[j].ISE_Group = j;   //Group ID
            pISEGroupCap[j].pGrpAttr.iseMode = ISEMODE_ONE_FISHEYE;
            /*ise group bind channel attr*/
            pISEGroupCap[j].ISE_Group_S.mChnId = 0;
            pISEGroupCap[j].ISE_Group_S.mDevId = j;
            pISEGroupCap[j].ISE_Group_S.mModId = MOD_ID_ISE;
            for(i = 0;i < 1;i++)
            {
                memset(&pISEPortCap[i], 0, sizeof(ISE_PortCap_S));
                pISEGroupCap[j].ISE_Port[i] = i;  //Port ID
                /*ise port bind channel attr*/
                pISEPortCap[i].ISE_Port_S.mChnId = i;
                pISEPortCap[i].ISE_Port_S.mDevId = j;
                pISEPortCap[i].ISE_Port_S.mModId = MOD_ID_ISE;
                /*Set ISE Port Attribute*/
                if(i == 0)  //fish arttr
                {
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.dewarp_mode = stContext.mConfigPara.ISEGroupConfig.ISE_Dewarp_Mode;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.in_w = stContext.mConfigPara.VIDevConfig.SrcWidth;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.in_h = stContext.mConfigPara.VIDevConfig.SrcHeight;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.in_luma_pitch = stContext.mConfigPara.VIDevConfig.SrcWidth;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.in_chroma_pitch = stContext.mConfigPara.VIDevConfig.SrcWidth;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.in_yuv_type = 0;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.out_yuv_type = 0;
#if Fish_Len_1
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.p = (868.445*stContext.mConfigPara.VIDevConfig.SrcHeight/2048)*2/3.1415;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cx = 1018.4*stContext.mConfigPara.VIDevConfig.SrcWidth/2048;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cy = 1007.3*stContext.mConfigPara.VIDevConfig.SrcHeight/2048;
#endif
#if Fish_Len_2
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.p = (875.6041*stContext.mConfigPara.VIDevConfig.SrcHeight/2048)*2/3.1415;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cx = 1032.1  *stContext.mConfigPara.VIDevConfig.SrcWidth/2048;
                    pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cy = 1023.2*stContext.mConfigPara.VIDevConfig.SrcHeight/2048;
#endif
                    if(pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.dewarp_mode == WARP_NORMAL)
                    {
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.mount_mode = stContext.mConfigPara.ISEGroupConfig.Mount_Mode;;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.pan = stContext.mConfigPara.ISEGroupConfig.normal_pan;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.tilt = stContext.mConfigPara.ISEGroupConfig.normal_tilt;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.zoom = stContext.mConfigPara.ISEGroupConfig.normal_zoom;
                    }
                    if(pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.dewarp_mode == WARP_PANO360)
                    {
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.mount_mode = stContext.mConfigPara.ISEGroupConfig.Mount_Mode;
                    }
                    if(pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.dewarp_mode == WARP_180WITH2)
                    {
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.mount_mode = stContext.mConfigPara.ISEGroupConfig.Mount_Mode;
                    }
                    if(pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.dewarp_mode == WARP_UNDISTORT)
                    {
#if Len_110
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cx = 918.18164 * (stContext.mConfigPara.VIDevConfig.SrcWidth*1.0/1920);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cy = 479.86784 * (stContext.mConfigPara.VIDevConfig.SrcHeight*1.0/1080);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.fx  = 1059.11146 * (stContext.mConfigPara.VIDevConfig.SrcWidth*1.0/1920);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.fy = 1053.26240 * (stContext.mConfigPara.VIDevConfig.SrcHeight*1.0/1080);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cxd = 876.34066 * (stContext.mConfigPara.VIDevConfig.SrcWidth*1.0/1920);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cyd = 465.93162 * (stContext.mConfigPara.VIDevConfig.SrcHeight*1.0/1080);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.fxd = 691.76196 * (stContext.mConfigPara.VIDevConfig.SrcWidth*1.0/1920);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.fyd = 936.82897 * (stContext.mConfigPara.VIDevConfig.SrcHeight*1.0/1080);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[0] = 0.182044494560808;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[1] = -0.1481043082174997;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[2] = -0.005128687334715951;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[3] = 0.567926713301489;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[4] = -0.1789466261819578;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[5] = -0.03561367966855939;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.p_undis[0] = 0.000649146914880072;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.p_undis[1] = 0.0002534155740808075;
#endif

#if Len_130
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cx = 889.50411 * (stContext.mConfigPara.VIDevConfig.SrcWidth*1.0/1920);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cy = 522.42100 * (stContext.mConfigPara.VIDevConfig.SrcHeight*1.0/1080);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.fx  = 963.98364 * (stContext.mConfigPara.VIDevConfig.SrcWidth*1.0/1920);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.fy = 965.10729 * (stContext.mConfigPara.VIDevConfig.SrcHeight*1.0/1080);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cxd = 801.04388 * (stContext.mConfigPara.VIDevConfig.SrcWidth*1.0/1920);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.cyd = 518.79163 * (stContext.mConfigPara.VIDevConfig.SrcHeight*1.0/1080);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.fxd = 580.58685 * (stContext.mConfigPara.VIDevConfig.SrcWidth*1.0/1920);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.fyd = 850.71746 * (stContext.mConfigPara.VIDevConfig.SrcHeight*1.0/1080);
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[0] = 0.5874970340655806;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[1] = 0.01263866896598456;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[2] = -0.003440797814786819;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[3] = 0.9486826799125321;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[4] = 0.1349250696268053;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.k[5] = -0.01052234728693081;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.p_undis[0] = 0.000362407777916013;
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.p_undis[1] = -0.0001245920435755955;

#endif
                    }
                    if(pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.dewarp_mode == WARP_PTZ4IN1)
                    {
                        pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.mount_mode = stContext.mConfigPara.ISEGroupConfig.Mount_Mode;;
                        pISEPortCap->PortAttr.mode_attr.mFish.ise_cfg.mount_mode = stContext.mConfigPara.ISEGroupConfig.Mount_Mode;
                        for(int sub_num = 0;sub_num < 4;sub_num++)
                        {
                            pISEPortCap->PortAttr.mode_attr.mFish.ise_cfg.pan_sub[sub_num] = stContext.mConfigPara.ISEGroupConfig.ptz4in1_pan[sub_num];
                            pISEPortCap->PortAttr.mode_attr.mFish.ise_cfg.tilt_sub[sub_num] = stContext.mConfigPara.ISEGroupConfig.ptz4in1_tilt[sub_num];;
                            pISEPortCap->PortAttr.mode_attr.mFish.ise_cfg.zoom_sub[sub_num] = stContext.mConfigPara.ISEGroupConfig.ptz4in1_zoom[sub_num];;
                        }
                    }
                }
                pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.out_en[i] = 1;
                pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.out_w[i] =  stContext.mConfigPara.ISEPortConfig[i].ISEWidth;
                pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.out_h[i] =  stContext.mConfigPara.ISEPortConfig[i].ISEHeight;
                pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.out_flip[i] =  stContext.mConfigPara.ISEPortConfig[i].flip_enable;
                pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.out_mirror[i] =  stContext.mConfigPara.ISEPortConfig[i].mirror_enable;
                pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.out_luma_pitch[i] = stContext.mConfigPara.ISEPortConfig[i].ISEStride;
                pISEPortCap[i].PortAttr.mode_attr.mFish.ise_cfg.out_chroma_pitch[i] = stContext.mConfigPara.ISEPortConfig[i].ISEStride;
                pISEGroupCap[j].PortCap_S[i] = &pISEPortCap[i];
            }
        }

        /*Set VO Channel Attribute*/
        memset(&pVOCap[0], 0, sizeof(VO_Cap_S));
        pVOCap[0].mUILayer = HLAY(2, 0);
        pVOCap[0].mVODev = 0;
        pVOCap[0].mVOChn = 0;
        pVOCap[0].hlay0 = 0;
        pVOCap[0].mVoLayer = 0;
        pVOCap[0].mLayerAttr.stDispRect.X = 0;
        pVOCap[0].mLayerAttr.stDispRect.Y = 0;
        pVOCap[0].mLayerAttr.stDispRect.Width = stContext.mConfigPara.VOChnConfig.display_width;
        pVOCap[0].mLayerAttr.stDispRect.Height = stContext.mConfigPara.VOChnConfig.display_height;
        pVOCap[0].mTestDuration = stContext.mConfigPara.VOChnConfig.mTestDuration;
        /*vo chn bind channel attr*/
        pVOCap[0].VO_CHN_S.mChnId = 0;
        pVOCap[0].VO_CHN_S.mDevId = 0;
        pVOCap[0].VO_CHN_S.mModId = MOD_ID_VOU;
        cdx_sem_init(&pVOCap[0].mSemExit, 0);

        /* start mpp systerm */
        MPP_SYS_CONF_S mSysConf;
        memset(&mSysConf,0,sizeof(MPP_SYS_CONF_S));
        mSysConf.nAlignWidth = 32;
        AW_MPI_SYS_SetConf(&mSysConf);
        ret = AW_MPI_SYS_Init();
        if(ret < 0)
        {
            aloge("sys init failed");
            result = -1;
            goto sys_exit;
        }

        /* creat vi component */
        ret = aw_vipp_creat(pVICap[0].VI_Dev, &pVICap[0].stAttr);
        if(ret < 0)
        {
            aloge("vipp creat failed,VIDev = %d",pVICap[0].VI_Dev);
            result = -1;
            goto vipp_exit;
        }
        /*AW_MPI_ISP_Init(0);
        AW_MPI_ISP_Run(0);*/
        ret = aw_virvi_creat(pVICap[0].VI_Dev,pVICap[0].VI_Chn, NULL);
        if(ret < 0)
        {
            aloge("virvi creat failed,VIDev = %d,VIChn = %d",pVICap[0].VI_Dev,pVICap[0].VI_Chn);
            result = -1;
            goto virvi_exit;
        }

        /* creat ise component */
        ret = aw_isegroup_creat(pISEGroupCap[0].ISE_Group, &pISEGroupCap[0].pGrpAttr);
        if(ret < 0)
        {
            aloge("ISE Group %d creat failed",pISEGroupCap[0].ISE_Group);
            result = -1;
            goto ise_group_exit;
        }
        ret = aw_iseport_creat(pISEGroupCap[0].ISE_Group,pISEGroupCap[0].ISE_Port[0],&(pISEGroupCap[0].PortCap_S[0]->PortAttr));
        if(ret < 0)
        {
            aloge("ISE Port%d creat failed",pISEGroupCap[0].ISE_Port[0]);
            result = -1;
            goto ise_port_exit;
        }

        /* creat vo component */
        ret = aw_vo_dev_creat(&pVOCap[0]);
        if(ret < 0)
        {
            aloge("Vo Dev%d enable failed",pVOCap[0].mVODev);
            result = -1;
            goto vo_dev_exit;
        }
        ret = aw_vo_chn_creat(&pVOCap[0]);
        if(ret < 0)
        {
            aloge("Vo Chn%d enable failed",pVOCap[0].mVOChn);
            result = -1;
            goto vo_chn_exit;
        }

        /* bind component */
        if (pVICap[0].VI_CHN_S.mDevId >= 0 && pISEGroupCap[0].ISE_Group_S.mDevId >= 0)
        {
           ret = AW_MPI_SYS_Bind(&pVICap[0].VI_CHN_S,&pISEGroupCap[0].ISE_Group_S);
           if(ret !=SUCCESS)
           {
               aloge("error!!! VI dev%d can not bind ISE Group%d!!",
                       pVICap[0].VI_CHN_S.mDevId,pISEGroupCap[0].ISE_Group_S.mDevId);
               result = -1;
               goto vi_bind_ise_exit;
           }
        }
        if (pISEGroupCap[0].PortCap_S[0]->ISE_Port_S.mChnId >= 0 && pVOCap[0].VO_CHN_S.mChnId >= 0)
        {
            ret = AW_MPI_SYS_Bind(&pISEGroupCap[0].PortCap_S[0]->ISE_Port_S,&pVOCap[0].VO_CHN_S);
            if(ret !=SUCCESS)
            {
                aloge("error!!! ISE port%d can not bind VO chn%d!!!",
                      pISEGroupCap[0].PortCap_S[0]->ISE_Port_S.mChnId, pVOCap[0].VO_CHN_S.mChnId);
                result = -1;
                goto  ise_bind_vo_exit;
            }
        }

        /* start component */
        ret = AW_MPI_VI_EnableVirChn(pVICap[0].VI_Dev, pVICap[0].VI_Chn);
        if (ret < 0)
        {
            aloge("VI enable error! VIDev = %d,VIChn = %d",
                    pVICap[0].VI_Dev, pVICap[0].VI_Chn);
            result = -1;
            goto vi_stop;
        }
        ret = AW_MPI_ISE_Start(pISEGroupCap[0].ISE_Group);
        if (ret < 0)
        {
            aloge("ISE Group%d Start error!",pISEGroupCap[0].ISE_Group);
            result = -1;
            goto ise_stop;
        }
        ret = AW_MPI_VO_StartChn(pVOCap[0].mVoLayer, pVOCap[0].mVOChn);
        if(ret < 0)
        {
            aloge("Vo Chn%d Start error!",pVOCap[0].mVOChn);
            result = -1;
            goto vo_stop;
        }
        if(pVOCap[0].mTestDuration > 0)
        {
            cdx_sem_down_timedwait(&pVOCap[0].mSemExit, pVOCap[0].mTestDuration*1000);
        }
        else
        {
            cdx_sem_down(&pVOCap[0].mSemExit);
        }

        if(pVICap[0].VI_Dev == 0 || pVICap[0].VI_Dev == 2)
            AW_MPI_ISP_Stop(1);
        if(pVICap[0].VI_Dev == 1 || pVICap[0].VI_Dev == 3)
            AW_MPI_ISP_Stop(0);
        AW_MPI_ISP_Exit();

vo_stop:
        /* stop component */
        ret = AW_MPI_VO_StopChn(pVOCap[0].mVoLayer, pVOCap[0].mVOChn);
        if(ret < 0)
        {
            aloge("Vo Chn%d Stop error!",pVOCap[0].mVOChn);
            result = -1;
            goto _exit;
        }

ise_stop:
        ret = AW_MPI_ISE_Stop(pISEGroupCap[0].ISE_Group);
        if(ret < 0)
        {
            aloge("ISE Group%d Stop error!",pISEGroupCap[0].ISE_Group);
            result = -1;
            goto _exit;
        }

vi_stop:
        ret = AW_MPI_VI_DisableVirChn(pVICap[0].VI_Dev, pVICap[0].VI_Chn);
        if(ret < 0)
        {
            aloge("Disable VI Chn failed,VIDev = %d,VIChn = %d",pVICap[0].VI_Dev, pVICap[0].VI_Chn);
            result = -1;
            goto _exit;
        }

vi_bind_ise_exit:
        /* ubind component */
        if (pVICap[0].VI_CHN_S.mDevId >= 0 && pISEGroupCap[0].ISE_Group_S.mDevId >= 0)
        {
            ret = AW_MPI_SYS_UnBind(&pVICap[0].VI_CHN_S,&pISEGroupCap[0].ISE_Group_S);
            if (ret)
            {
                aloge("error!!! Unbind VI dev%d ISE Group%d failed!!!",
                       pVICap[0].VI_CHN_S.mDevId,pISEGroupCap[0].ISE_Group_S.mDevId);
                result = -1;
                goto _exit;
            }
        }
ise_bind_vo_exit:
        if (pISEGroupCap[0].PortCap_S[0]->ISE_Port_S.mChnId >= 0 && pVOCap[0].VO_CHN_S.mChnId >= 0)
        {
           ret = AW_MPI_SYS_UnBind(&pISEGroupCap[0].PortCap_S[0]->ISE_Port_S,&pVOCap[0].VO_CHN_S);
           if(ret !=SUCCESS)
           {
               aloge("error!!! Unbind ISE port%d VO chn%d failed!!!",
                     pISEGroupCap[0].PortCap_S[0]->ISE_Port_S.mChnId, pVOCap[0].VO_CHN_S.mChnId);
               result = -1;
               goto _exit;
           }
        }

vo_chn_exit:
        /* destory vo component */
        ret = aw_vo_chn_destory(&pVOCap[0]);
        if (ret < 0)
        {
            aloge("VO Chn%d distory failed!",pVOCap[0].mVOChn);
            result = -1;
            goto _exit;
        }

vo_dev_exit:
        ret = aw_vo_dev_destory(&pVOCap[0]);
        if (ret < 0)
        {
            aloge("VO Dev%d distory failed!",pVOCap[0].mVODev);
            result = -1;
            goto _exit;
        }

ise_port_exit:
        /* destory ise component */
        ret = aw_iseport_destory(pISEGroupCap[0].ISE_Group, pISEGroupCap[0].ISE_Port[0]);
        if (ret < 0)
        {
            aloge("ISE Port%d distory error!",pISEGroupCap[0].ISE_Port[0]);
            result = -1;
            goto _exit;
        }

ise_group_exit:
        ret = aw_isegroup_destory(pISEGroupCap[0].ISE_Group);
        if (ret < 0)
        {
            aloge("ISE Destroy Group%d error!",pISEGroupCap[0].ISE_Group);
            result = -1;
            goto _exit;
        }

virvi_exit:
        /* destory vi component */
        ret = aw_virvi_destory(pVICap[0].VI_Dev, pVICap[0].VI_Chn);
        if (ret < 0)
        {
            aloge("virvi end error! VIDev = %d,VIChn = %d",
                       pVICap[0].VI_Dev, pVICap[0].VI_Chn);
            result = -1;
            goto _exit;
        }

vipp_exit:
        ret = aw_vipp_destory(pVICap[0].VI_Dev);
        if (ret < 0)
        {
            aloge("vipp end error! VIDev = %d",pVICap[0].VI_Dev);
            result = -1;
            goto _exit;
        }

sys_exit:
        /* exit mpp systerm */
        ret = AW_MPI_SYS_Exit();
        if(ret < 0)
        {
            aloge("sys exit failed");
            result = -1;
            goto _exit;
        }
        printf("======================================.\r\n");
        printf("Auto Test count end: %d. (MaxCount==1000).\r\n", count);
        printf("======================================.\r\n");
        count ++;
    }
    printf("sample_virvi2fish2vo exit!\n");
    return 0;
_exit:
        return result;
}
