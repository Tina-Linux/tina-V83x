/*
 * =====================================================================================
 *
 *      Copyright (c) 2008-2018 Allwinner Technology Co. Ltd.
 *      All rights reserved.
 *
 *       Filename:  AwOmxVenc.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:  2018/08
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  SWC-PPD
 *      Company: Allwinner Technology Co. Ltd.
 *
 *       History :
 *        Author : AL3
 *        Date   : 2013/05/05
 *    Comment : complete the design code
 *
 * =====================================================================================
 */
#define LOG_TAG "omx_venc"
#include "log.h"
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memoryAdapter.h"
#include "CdcUtil.h"
#include "omx_venc.h"
#include "omx_venc_adapter.h"
#include <OMX_IndexExt.h>

#define ION_DEV_NAME                     "/dev/ion"//TODO: android-related!
#define DEFAULT_BITRATE (1024*1024*2)
#define PRINT_FRAME_CNT (50)
#define BUF_ALIGN_SIZE (32)
#define WIDTH_DEFAULT (176)
#define HEIGHT_DEFAULT (144)
#define ALIGN_16B(x) (((x) + (15)) & ~(15))

static unsigned int omx_venc_align(unsigned int x, int a)
{
    return (x + (a-1)) & (~(a-1));
}

#define ALIGN_32B(x) (((x) + (31)) & ~(31))
#define ROUND_DOWN_2(x)  ((x)&(~1))
static int64_t GetNowUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000000ll + tv.tv_usec;
}

/* H.263 Supported Levels & profiles */

VIDEO_PROFILE_LEVEL_TYPE SupportedH263ProfileLevels[] = {
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level10},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level20},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level30},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level40},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level50},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level60},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level70},

  {-1, -1}
};

/* AVC Supported Levels & profiles */
static VIDEO_PROFILE_LEVEL_TYPE SupportedAVCProfileLevels[] = {
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel2},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel51},

  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1 },
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel2 },
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel3 },
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel51},

  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel2 },
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3 },
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel51},

  {-1,-1}
};

/* HEVC Supported Levels & profiles */
static VIDEO_PROFILE_LEVEL_TYPE SupportedHEVCProfileLevels[] = {
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel1},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel2},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel21},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel3},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel31},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel4},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel41},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel5},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel51},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel52},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel6},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel61},
  {OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel62},

  {-1,-1}
};

const char* strThrCmd[] =
{
    "main_thread_cmd_set_state",
    "main_thread_cmd_flush",
    "main_thread_cmd_stop_port",
    "main_thread_cmd_restart_port",
    "main_thread_cmd_mark_buffer",
    "main_thread_cmd_stop",
    "main_thread_cmd_fill_buffer",
    "main_thread_cmd_empty_buffer"
};

const char* strStateType[] =
{
    "OMX_StateInvalid",
    "OMX_StateLoaded",
    "OMX_StateIdle",
    "OMX_StateExecuting",
    "OMX_StatePause",
    "OMX_StateWaitForResources"
};

const char* strVencCmd[] =
{
    "venc_cmd_open",
    "venc_cmd_close",
    "venc_cmd_stop",
    "venc_cmd_enc_idle",
    "venc_cmd_changeBitrate",
    "venc_cmd_changeColorFormat",
    "venc_cmd_requestIDRFrame",
    "venc_cmd_changeFramerate"
};

const char* strVencResult[] =
{
    "enc_result_error",//-1
    "enc_result_ok",
    "enc_result_no_frame_buffer",
    "enc_result_full_bitstream",
    "enc_result_illegal_param",
    "enc_result_no_support",
    "enc_result_no_bitstream",
    "enc_result_no_memory"
};

/*
 *     M A C R O S
 */

/*
 * Initializes a data structure using a pointer to the structure.
 * The initialization of OMX structures always sets up the nSize and nVersion fields
 *   of the structure.
 */
#define OMX_CONF_INIT_STRUCT_PTR(_variable_, _struct_name_) \
    memset((_variable_), 0x0, sizeof(_struct_name_)); \
    (_variable_)->nSize = sizeof(_struct_name_); \
    (_variable_)->nVersion.s.nVersionMajor = 0x1; \
    (_variable_)->nVersion.s.nVersionMinor = 0x1; \
    (_variable_)->nVersion.s.nRevision = 0x0; \
    (_variable_)->nVersion.s.nStep = 0x0

static VIDDEC_CUSTOM_PARAM sVideoEncCustomParams[] =
{
    {"OMX.google.android.index.storeMetaDataInBuffers",
        (OMX_INDEXTYPE)VideoEncodeCustomParamStoreMetaDataInBuffers},
    {"OMX.google.android.index.prependSPSPPSToIDRFrames",
        (OMX_INDEXTYPE)VideoEncodeCustomParamPrependSPSPPSToIDRFrames},
    {"OMX.google.android.index.enableAndroidNativeBuffers",
        (OMX_INDEXTYPE)VideoEncodeCustomParamEnableAndroidNativeBuffers},
    {"OMX.Topaz.index.param.extended_video",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideo},
    {"OMX.aw.index.param.videoSuperFrameConfig",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideoSuperframe},
    {"OMX.aw.index.param.videoSVCSkipConfig",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideoSVCSkip},
    {"OMX.aw.index.param.videoVBRConfig",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideoVBR},
    {"OMX.google.android.index.storeANWBufferInMetadata",
        (OMX_INDEXTYPE)VideoEncodeCustomParamStoreANWBufferInMetadata},
    {"OMX.aw.index.param.videoPSkipConfig",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedVideoPSkip},
    {"OMX.aw.index.param.videoAVCNaluFormatConfig",
        (OMX_INDEXTYPE)VideoEncodeCustomParamextendedAVCNaluFormat}
};

static void* ComponentThread(void* pThreadData);
static void* ComponentVencThread(void* pThreadData);

void callbackEmptyBufferDone(AwOmxVenc* impl,
                                    OMX_IN OMX_BUFFERHEADERTYPE * pInBufHdr)
{
    logv("call back of emptyBufferDone.");
    impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp,
                                                   impl->m_pAppData,
                                                   pInBufHdr);
}

void post_message_to_venc_and_wait(AwOmxVenc *impl, OMX_S32 id)
{
    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));
    msg.messageId = id;
    logv("function %s.  id %s.", __FUNCTION__, strVencCmd[id]);

    CdcMessageQueuePostMessage(impl->mqVencThread, &msg);
    omx_sem_down(&impl->m_msg_sem);
}


void post_message_to_venc(AwOmxVenc *impl, OMX_S32 id)
{
    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));
    msg.messageId = id;
    logv("function %s.  id %s.", __FUNCTION__, strVencCmd[id]);

    CdcMessageQueuePostMessage(impl->mqVencThread, &msg);
}

static OMX_ERRORTYPE post_event(AwOmxVenc* impl,
                                   OMX_IN ThrCmdType eCmdNative,
                                   OMX_IN OMX_U32         uParam1,
                                   OMX_IN OMX_PTR         pCmdData)
{
    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));
    msg.messageId = eCmdNative;
    logv("function %s.  cmd %s.", __FUNCTION__, strThrCmd[eCmdNative]);
    if (eCmdNative == MAIN_THREAD_CMD_MARK_BUF|| eCmdNative == MAIN_THREAD_CMD_EMPTY_BUF
        || eCmdNative == MAIN_THREAD_CMD_FILL_BUF)
    {
        msg.params[0] = (uintptr_t)pCmdData;
    }
    else
        msg.params[0] = (uintptr_t)uParam1;

    CdcMessageQueuePostMessage(impl->mqMainThread, &msg);

    return OMX_ErrorNone;
}

static void __AwOmxVencDestroy(OMX_HANDLETYPE pCmp_handle)
{
    AwOmxVenc *impl = (AwOmxVenc*)pCmp_handle;
    OMX_S32 nIndex;
    logd(" COMPONENT_DESTROY ");

    pthread_mutex_lock(&impl->m_inBufMutex);

    for (nIndex=0; nIndex<impl->m_sInBufList.nBufArrSize; nIndex++)
    {
        if (impl->m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
        {
            if (impl->m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
            {
                free(impl->m_sInBufList.pBufArr[nIndex].pBuffer);
                impl->m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
            }
        }
    }

    if (impl->m_sInBufList.pBufArr != NULL)
    {
        free(impl->m_sInBufList.pBufArr);
    }

    if (impl->m_sInBufList.pBufHdrList != NULL)
    {
        free(impl->m_sInBufList.pBufHdrList);
    }

    memset(&impl->m_sInBufList, 0, sizeof(struct _BufferList));
    impl->m_sInBufList.nBufArrSize = impl->m_sInPortDefType.nBufferCountActual;

    pthread_mutex_unlock(&impl->m_inBufMutex);

    pthread_mutex_lock(&impl->m_outBufMutex);

    for (nIndex=0; nIndex<impl->m_sOutBufList.nBufArrSize; nIndex++)
    {
        if (impl->m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
        {
            if (impl->m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
            {
                free(impl->m_sOutBufList.pBufArr[nIndex].pBuffer);
                impl->m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
            }
        }
    }

    if (impl->m_sOutBufList.pBufArr != NULL)
    {
        free(impl->m_sOutBufList.pBufArr);
    }

    if (impl->m_sOutBufList.pBufHdrList != NULL)
    {
        free(impl->m_sOutBufList.pBufHdrList);
    }

    memset(&impl->m_sOutBufList, 0, sizeof(struct _BufferList));
    impl->m_sOutBufList.nBufArrSize = impl->m_sOutPortDefType.nBufferCountActual;

    if(impl->memops)
        CdcMemClose(impl->memops);

    if (impl->mIonFd != -1)
    {
        int ret = 0;
        ret = CdcIonClose(impl->mIonFd);
        if(ret < 0)
            loge("CdcIonClose ion fd error\n");
        impl->mIonFd = -1;
    }

    pthread_mutex_unlock(&impl->m_outBufMutex);
    pthread_mutex_destroy(&impl->m_inBufMutex);
    pthread_mutex_destroy(&impl->m_outBufMutex);
    omx_sem_deinit(&impl->m_msg_sem);
    omx_sem_deinit(&impl->m_need_sleep_sem);
    if(impl->mqMainThread)
        CdcMessageQueueDestroy(impl->mqMainThread);

    if(impl->mqVencThread)
        CdcMessageQueueDestroy(impl->mqVencThread);

    free(impl);
    impl = NULL;

    logv("~omx_enc done!");
}

static OMX_ERRORTYPE __AwOmxVencInit(OMX_HANDLETYPE pCmp_handle, OMX_STRING pComponentName)
{
    logd(" COMPONENT_INIT ");
    AwOmxVenc* impl = (AwOmxVenc*)pCmp_handle;
    OMX_ERRORTYPE eRet = OMX_ErrorNone;
    int           err;
    OMX_U32       nIndex;
    int           i;
    impl->m_state                = OMX_StateLoaded;
    impl->m_cRole[0]             = 0;
    impl->m_cName[0]             = 0;
    impl->m_eCompressionFormat   = OMX_VIDEO_CodingUnused;
    impl->m_pAppData             = NULL;
    impl->m_thread_id            = 0;
    impl->m_venc_thread_id       = 0;
    impl->m_frameCountForCSD       = 0;
    impl->m_framerate            = 30;
    impl->mIsFromCts             = OMX_FALSE;
    impl->mIsFromVideoeditor     = OMX_FALSE;
    impl->m_useAllocInputBuffer  = OMX_FALSE;
    impl->m_useAndroidBuffer     = OMX_FALSE;
    impl->mFirstInputFrame       = OMX_TRUE;
    impl->m_usePSkip             = OMX_FALSE;
    impl->mEmptyBufCnt           = 0;
    impl->mFillBufCnt            = 0;
    impl->mIonFd                   = -1;
    impl->memops =  MemAdapterGetOpsS();
    CdcMemOpen(impl->memops);

    if(impl->bShowFrameSizeFlag)
	{
        impl->mFrameCnt = 0;
        impl->mAllFrameSize = 0;
        impl->mTimeStart = 0;
        impl->mTimeOut = 999;//ms
    }

    impl->m_encoder = NULL;

    memset(&impl->m_Callbacks, 0, sizeof(impl->m_Callbacks));
    memset(&impl->m_sInPortDefType, 0, sizeof(impl->m_sInPortDefType));
    memset(&impl->m_sOutPortDefType, 0, sizeof(impl->m_sOutPortDefType));
    memset(&impl->m_sInPortFormatType, 0, sizeof(impl->m_sInPortFormatType));
    memset(&impl->m_sOutPortFormatType, 0, sizeof(impl->m_sOutPortFormatType));
    memset(&impl->m_sPriorityMgmtType, 0, sizeof(impl->m_sPriorityMgmtType));
    memset(&impl->m_sInBufSupplierType, 0, sizeof(impl->m_sInBufSupplierType));
    memset(&impl->m_sOutBufSupplierType, 0, sizeof(impl->m_sOutBufSupplierType));
    memset(&impl->m_sInBufList, 0, sizeof(impl->m_sInBufList));
    memset(&impl->m_sOutBufList, 0, sizeof(impl->m_sOutBufList));
    memset(impl->mInputBufInfo, 0, sizeof(impl->mInputBufInfo));

    pthread_mutex_init(&impl->m_inBufMutex, NULL);
    pthread_mutex_init(&impl->m_outBufMutex, NULL);

    omx_sem_init(&impl->m_msg_sem, 0);
    omx_sem_init(&impl->m_need_sleep_sem, 0);
    impl->mqMainThread   = CdcMessageQueueCreate(64, "omx_vdec_new_mainThread");
    impl->mqVencThread   = CdcMessageQueueCreate(64, "omx_vdec_new_VencThread");
    if(impl->mqMainThread == NULL || impl->mqVencThread == NULL)
    {
        loge(" create mqMainThread[%p] or mqVdrvThread[%p]failed",
              impl->mqMainThread, impl->mqVencThread);
    }

    //init the compression format by component name.
    strncpy((char*)impl->m_cName, pComponentName, OMX_MAX_STRINGNAME_SIZE);

    if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.avc", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)impl->m_cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat = OMX_VIDEO_CodingAVC;
    }
    else if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.hevc", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)impl->m_cRole, "video_encoder.hevc", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat = OMX_VIDEO_CodingHEVC;
    }
    else if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.h263", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)impl->m_cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat = OMX_VIDEO_CodingH263;
    }
    else if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.mpeg4", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)impl->m_cRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat = OMX_VIDEO_CodingMPEG4;
    }
    else if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.mjpeg", OMX_MAX_STRINGNAME_SIZE))
    {
        strncpy((char*)impl->m_cRole, "video_encoder.mjpeg", OMX_MAX_STRINGNAME_SIZE);
        impl->m_eCompressionFormat = OMX_VIDEO_CodingMJPEG;
    }
    else
    {
        logv("\nERROR:Unknown Component\n");
        eRet = OMX_ErrorInvalidComponentName;
        return eRet;
    }

    // init codec type
    if (OMX_VIDEO_CodingAVC == impl->m_eCompressionFormat)
    {
        impl->m_vencCodecType = VENC_CODEC_H264;
    }
    else if (OMX_VIDEO_CodingVP8 == impl->m_eCompressionFormat)
    {
        impl->m_vencCodecType = VENC_CODEC_VP8;
    }
    else if (OMX_VIDEO_CodingMJPEG == impl->m_eCompressionFormat)
    {
        impl->m_vencCodecType = VENC_CODEC_JPEG;
    }
    else if (OMX_VIDEO_CodingHEVC == impl->m_eCompressionFormat)
    {
        impl->m_vencCodecType = VENC_CODEC_H265;
    }
    else
    {
        logd("need check codec type");
    }

    // Initialize component data structures to default values
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sPortParam, OMX_PORT_PARAM_TYPE);
    impl->m_sPortParam.nPorts           = 0x2;
    impl->m_sPortParam.nStartPortNumber = 0x0;

    // Initialize the video parameters for input port
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sInPortDefType, OMX_PARAM_PORTDEFINITIONTYPE);
    impl->m_sInPortDefType.nPortIndex                      = 0x0;
    impl->m_sInPortDefType.bEnabled                        = OMX_TRUE;
    impl->m_sInPortDefType.bPopulated                      = OMX_FALSE;
    impl->m_sInPortDefType.eDomain                         = OMX_PortDomainVideo;
    impl->m_sInPortDefType.format.video.nFrameWidth        = 0;//WIDTH_DEFAULT;
    impl->m_sInPortDefType.format.video.nFrameHeight       = 0;//HEIGHT_DEFAULT;
    impl->m_sInPortDefType.format.video.nStride            = 0;//WIDTH_DEFAULT;
    impl->m_sInPortDefType.eDir                            = OMX_DirInput;

//for 4k recorder @30fps
#ifdef CONF_SUPPORT_4K_30FPS_RECORDER
    impl->m_sInPortDefType.nBufferCountMin                 = 4;
    impl->m_sInPortDefType.nBufferCountActual              = 4;
#else
    impl->m_sInPortDefType.nBufferCountMin                 = NUM_IN_BUFFERS;
    impl->m_sInPortDefType.nBufferCountActual              = NUM_IN_BUFFERS;
#endif

    impl->m_sInPortDefType.nBufferSize = (OMX_U32)(impl->m_sInPortDefType.format.video.nFrameWidth *
                                             impl->m_sInPortDefType.format.video.nFrameHeight * 3 / 2);
    impl->m_sInPortDefType.format.video.eColorFormat = DEFAULT_OMX_COLOR_FORMAT;

    // Initialize the video parameters for output port
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sOutPortDefType, OMX_PARAM_PORTDEFINITIONTYPE);
    impl->m_sOutPortDefType.nPortIndex                   = 0x1;
    impl->m_sOutPortDefType.bEnabled                     = OMX_TRUE;
    impl->m_sOutPortDefType.bPopulated                   = OMX_FALSE;
    impl->m_sOutPortDefType.eDomain                      = OMX_PortDomainVideo;
    impl->m_sOutPortDefType.format.video.cMIMEType       = (OMX_STRING)"YUV420";
    impl->m_sOutPortDefType.format.video.nFrameWidth     = WIDTH_DEFAULT;
    impl->m_sOutPortDefType.format.video.nFrameHeight    = HEIGHT_DEFAULT;
    impl->m_sOutPortDefType.eDir                         = OMX_DirOutput;

    impl->mInportEnable = OMX_TRUE;
    impl->mOutportEnable = OMX_TRUE;

//for 4k recorder @30fps
#ifdef CONF_SUPPORT_4K_30FPS_RECORDER
    impl->m_sOutPortDefType.nBufferCountMin              = 6;
    impl->m_sOutPortDefType.nBufferCountActual           = 6;
#else
    impl->m_sOutPortDefType.nBufferCountMin              = NUM_OUT_BUFFERS;
    impl->m_sOutPortDefType.nBufferCountActual           = NUM_OUT_BUFFERS;
#endif
    impl->m_sOutPortDefType.nBufferSize                 = OMX_VIDEO_ENC_INPUT_BUFFER_SIZE;

    impl->m_sOutPortDefType.format.video.eCompressionFormat = impl->m_eCompressionFormat;
    impl->m_sOutPortDefType.format.video.cMIMEType          = (OMX_STRING)"";

    // Initialize the video compression format for input port
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sInPortFormatType, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    impl->m_sInPortFormatType.nPortIndex         = 0x0;
    impl->m_sInPortFormatType.nIndex             = 0x4;
    impl->m_sInPortFormatType.eColorFormat       = DEFAULT_OMX_COLOR_FORMAT;
    //GST requries OMX_COLOR_FormatYVU420Planar, not OMX_COLOR_FormatYUV420SemiPlanar.
    impl->m_inputcolorFormats[0] = OMX_COLOR_FormatYVU420Planar;
    impl->m_inputcolorFormats[1] = OMX_COLOR_FormatAndroidOpaque;
    impl->m_inputcolorFormats[2] = OMX_COLOR_FormatYVU420SemiPlanar;
    impl->m_inputcolorFormats[3] = OMX_COLOR_FormatYUV420SemiPlanar;
    impl->m_inputcolorFormats[4] = OMX_COLOR_FormatYUV420Planar;

    // Initialize the compression format for output port
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sOutPortFormatType, OMX_VIDEO_PARAM_PORTFORMATTYPE);
    impl->m_sOutPortFormatType.nPortIndex   = 0x1;
    impl->m_sOutPortFormatType.nIndex       = 0x0;
    impl->m_sOutPortFormatType.eCompressionFormat = impl->m_eCompressionFormat;

    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sPriorityMgmtType, OMX_PRIORITYMGMTTYPE);

    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sInBufSupplierType, OMX_PARAM_BUFFERSUPPLIERTYPE );
    impl->m_sInBufSupplierType.nPortIndex = 0x0;

    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sOutBufSupplierType, OMX_PARAM_BUFFERSUPPLIERTYPE );
    impl->m_sOutBufSupplierType.nPortIndex = 0x1;

    // Initalize the output bitrate
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sOutPutBitRateType, OMX_VIDEO_PARAM_BITRATETYPE);
    impl->m_sOutPutBitRateType.nPortIndex = 0x01;
    impl->m_sOutPutBitRateType.eControlRate = OMX_Video_ControlRateDisable;
    impl->m_sOutPutBitRateType.nTargetBitrate = DEFAULT_BITRATE; //default bitrate

    // Initalize the output h264param
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sH264Type, OMX_VIDEO_PARAM_AVCTYPE);
    impl->m_sH264Type.nPortIndex                = 0x01;
    impl->m_sH264Type.nSliceHeaderSpacing       = 0;
    impl->m_sH264Type.nPFrames                  = -1;
    impl->m_sH264Type.nBFrames                  = -1;
    impl->m_sH264Type.bUseHadamard              = OMX_TRUE; /*OMX_FALSE*/
    impl->m_sH264Type.nRefFrames                = 1; /*-1;  */
    impl->m_sH264Type.nRefIdx10ActiveMinus1     = -1;
    impl->m_sH264Type.nRefIdx11ActiveMinus1     = -1;
    impl->m_sH264Type.bEnableUEP                = OMX_FALSE;
    impl->m_sH264Type.bEnableFMO                = OMX_FALSE;
    //m_sH264Type.bEnableASO                = OMX_FALSE;
    impl->m_sH264Type.bEnableRS                 = OMX_FALSE;
    impl->m_sH264Type.eProfile                  = OMX_VIDEO_AVCProfileBaseline; /*0x01;*/
    impl->m_sH264Type.eLevel                    = OMX_VIDEO_AVCLevel1; /*OMX_VIDEO_AVCLevel11;*/
    impl->m_sH264Type.nAllowedPictureTypes      = -1;
    impl->m_sH264Type.bFrameMBsOnly             = OMX_FALSE;
    impl->m_sH264Type.bMBAFF                    = OMX_FALSE;
    impl->m_sH264Type.bEntropyCodingCABAC       = OMX_FALSE;
    impl->m_sH264Type.bWeightedPPrediction      = OMX_FALSE;
    impl->m_sH264Type.nWeightedBipredicitonMode = -1;
    impl->m_sH264Type.bconstIpred               = OMX_FALSE;
    impl->m_sH264Type.bDirect8x8Inference       = OMX_FALSE;
    impl->m_sH264Type.bDirectSpatialTemporal    = OMX_FALSE;
    impl->m_sH264Type.nCabacInitIdc             = -1;
    impl->m_sH264Type.eLoopFilterMode           = OMX_VIDEO_AVCLoopFilterDisable;

    // Initalize the output h265param
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_sH265Type, OMX_VIDEO_PARAM_HEVCTYPE);
    impl->m_sH265Type.nPortIndex                = 0x01;
    impl->m_sH265Type.eProfile                  = OMX_VIDEO_HEVCProfileMain;
    impl->m_sH265Type.eLevel                    = OMX_VIDEO_HEVCMainTierLevel51;
    impl->m_sH265Type.nKeyFrameInterval         = 30;

    // Initialize the input buffer list
    memset(&(impl->m_sInBufList), 0x0, sizeof(BufferList));

    impl->m_sInBufList.pBufArr = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                                         impl->m_sInPortDefType.nBufferCountActual);
    if (impl->m_sInBufList.pBufArr == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    memset(impl->m_sInBufList.pBufArr, 0,
           sizeof(OMX_BUFFERHEADERTYPE) * impl->m_sInPortDefType.nBufferCountActual);
    for (nIndex = 0; nIndex < impl->m_sInPortDefType.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR (&impl->m_sInBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    impl->m_sInBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) *
                                                              impl->m_sInPortDefType.nBufferCountActual);
    if (impl->m_sInBufList.pBufHdrList == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    impl->m_sInBufList.nSizeOfList       = 0;
    impl->m_sInBufList.nAllocSize        = 0;
    impl->m_sInBufList.nWritePos         = 0;
    impl->m_sInBufList.nReadPos          = 0;
    impl->m_sInBufList.nAllocBySelfFlags = 0;
    impl->m_sInBufList.nSizeOfList       = 0;
    impl->m_sInBufList.nBufArrSize       = impl->m_sInPortDefType.nBufferCountActual;
    impl->m_sInBufList.eDir              = OMX_DirInput;

    // Initialize the output buffer list
    memset(&impl->m_sOutBufList, 0x0, sizeof(BufferList));

    impl->m_sOutBufList.pBufArr = (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) *
                                                          impl->m_sOutPortDefType.nBufferCountActual);
    if (impl->m_sOutBufList.pBufArr == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    memset(impl->m_sOutBufList.pBufArr, 0,
           sizeof(OMX_BUFFERHEADERTYPE) * impl->m_sOutPortDefType.nBufferCountActual);
    for (nIndex = 0; nIndex < impl->m_sOutPortDefType.nBufferCountActual; nIndex++)
    {
        OMX_CONF_INIT_STRUCT_PTR(&impl->m_sOutBufList.pBufArr[nIndex], OMX_BUFFERHEADERTYPE);
    }

    impl->m_sOutBufList.pBufHdrList = (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*) *
                                 impl->m_sOutPortDefType.nBufferCountActual);
    if (impl->m_sOutBufList.pBufHdrList == NULL)
    {
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    impl->m_sOutBufList.nSizeOfList       = 0;
    impl->m_sOutBufList.nAllocSize        = 0;
    impl->m_sOutBufList.nWritePos         = 0;
    impl->m_sOutBufList.nReadPos          = 0;
    impl->m_sOutBufList.nAllocBySelfFlags = 0;
    impl->m_sOutBufList.nSizeOfList       = 0;
    impl->m_sOutBufList.nBufArrSize       = impl->m_sOutPortDefType.nBufferCountActual;
    impl->m_sOutBufList.eDir              = OMX_DirOutput;

    OMX_CONF_INIT_STRUCT_PTR(&impl->m_avcIntraPeriod, OMX_VIDEO_CONFIG_AVCINTRAPERIOD);
    impl->m_avcIntraPeriod.nPortIndex     = 0x01;
    impl->m_avcIntraPeriod.nIDRPeriod     = 30;
    impl->m_avcIntraPeriod.nPFrames       = 29;
    OMX_CONF_INIT_STRUCT_PTR(&impl->m_avcNaluFormat, OMX_NALSTREAMFORMATTYPE);
    impl->m_avcNaluFormat.nPortIndex      = 0x01;
    impl->m_avcNaluFormat.eNaluFormat     = OMX_NaluFormatStartCodes;

    // Create the component thread
    err = pthread_create(&impl->m_thread_id, NULL, ComponentThread, pCmp_handle);
    if ( err || !impl->m_thread_id )
    {
        loge("create ComponentThread error!!!");
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // Create venc thread
    err = pthread_create(&impl->m_venc_thread_id, NULL, ComponentVencThread, pCmp_handle);
    if ( err || !impl->m_venc_thread_id )
    {
        loge("create ComponentVencThread error!!!");
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    // init some param
    impl->m_useAndroidBuffer = OMX_FALSE;
    impl->m_useMetaDataInBuffers = OMX_FALSE;
    impl->m_prependSPSPPSToIDRFrames = OMX_FALSE;

    memset(&impl->mVideoExtParams, 0, sizeof(OMX_VIDEO_PARAMS_EXTENDED));
    memset(&impl->mVideoSuperFrame, 0, sizeof(OMX_VIDEO_PARAMS_SUPER_FRAME));
    memset(&impl->mVideoSVCSkip, 0, sizeof(OMX_VIDEO_PARAMS_SVC));
    memset(&impl->mVideoVBR, 0, sizeof(OMX_VIDEO_PARAMS_VBR));

    impl->mIonFd = CdcIonOpen();
    if (impl->mIonFd < 0)
    {
        loge("open %s failed \n", ION_DEV_NAME);
        eRet = OMX_ErrorInsufficientResources;
        goto EXIT;
    }

    for(i=0; i<NUM_MAX_IN_BUFFERS; i++)
    {
        impl->mInputBufInfo[i].nAwBufId= -1;
    }
    updateOmxDebugFlag(impl);

    if(impl->bSaveStreamFlag)
    {
        impl->m_outFile = fopen("/data/camera/bitstream-enc.dat", "wb");
        if(impl->m_outFile == NULL)
        {
            loge("open save output file error. errno=%d.\n", errno);
            eRet = OMX_ErrorInsufficientResources;
        }
        else
        {
            logd("open impl->m_outFile '/data/camera/bitstream-enc.dat'\n");
        }
    }
    if(impl->bSaveInputDataFlag)
    {
        impl->mInFile = fopen("/tmp/inputEncData.dat", "wb");
        if(impl->mInFile == NULL)
        {
            loge("open save input file error, errno=%d.\n",errno);
            eRet = OMX_ErrorInsufficientResources;
        }
        else
        {
            logd("open outFile '/tmp/input.dat'\n");
        }
    }
     /*TO REMOVE: test for fix qp*/
     /*
            logd("************ test for fix qp**************");
            impl->m_fixQp.bEnable = OMX_TRUE;
            impl->m_fixQp.nIQp = 25;//pComponentParam->nQpI;
            impl->m_fixQp.nPQp = 28;//pComponentParam->nQpP;*/

EXIT:
    return eRet;
}

OMX_ERRORTYPE __AwOmxVencGetComponentVersion(OMX_IN OMX_HANDLETYPE pHComp,
                                               OMX_OUT OMX_STRING pComponentName,
                                               OMX_OUT OMX_VERSIONTYPE* pComponentVersion,
                                               OMX_OUT OMX_VERSIONTYPE* pSpecVersion,
                                               OMX_OUT OMX_UUIDTYPE* pComponentUUID)
{
    AwOmxVenc* impl = (AwOmxVenc*) pHComp;
    logd(" COMPONENT_GET_VERSION");
    CEDARC_UNUSE(pComponentUUID);

    if (!pHComp || !pComponentName || !pComponentVersion || !pSpecVersion)
    {
        return OMX_ErrorBadParameter;
    }

    strcpy((char*)pComponentName, (char*)impl->m_cName);

    pComponentVersion->s.nVersionMajor = 1;
    pComponentVersion->s.nVersionMinor = 1;
    pComponentVersion->s.nRevision     = 0;
    pComponentVersion->s.nStep         = 0;

    pSpecVersion->s.nVersionMajor = 1;
    pSpecVersion->s.nVersionMinor = 1;
    pSpecVersion->s.nRevision     = 0;
    pSpecVersion->s.nStep         = 0;

    return OMX_ErrorNone;
}
OMX_ERRORTYPE __AwOmxVencSendCommand(OMX_IN OMX_HANDLETYPE  pHComp,
                                      OMX_IN OMX_COMMANDTYPE eCmd,
                                      OMX_IN OMX_U32         uParam1,
                                      OMX_IN OMX_PTR         pCmdData)
{
    AwOmxVenc* impl = (AwOmxVenc*) pHComp;
    ThrCmdType    eCmdNative;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    CEDARC_UNUSE(pHComp);
    logv(" COMPONENT_SEND_COMMAND");

    if (impl->m_state == OMX_StateInvalid)
    {
        loge("ERROR: Send Command in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (eCmd == OMX_CommandMarkBuffer && pCmdData == NULL)
    {
        loge("ERROR: Send OMX_CommandMarkBuffer command but pCmdData invalid.");
        return OMX_ErrorBadParameter;
    }
    //logd("COMPONENT_SEND_COMMAND: %s.", strThrCmd[eCmd]);
    switch (eCmd)
    {
        case OMX_CommandStateSet:
        {
            logv(" send_command: OMX_CommandStateSet");
            eCmdNative = MAIN_THREAD_CMD_SET_STATE;
            break;
        }

        case OMX_CommandFlush:
        {
            logv(" send_command: OMX_CommandFlush");
            eCmdNative = MAIN_THREAD_CMD_FLUSH;
            if ((int)uParam1 > 1 && (int)uParam1 != -1)
            {
                loge("Error: Send OMX_CommandFlush command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            break;
         }

        case OMX_CommandPortDisable:
        {
            logv(" send_command: OMX_CommandPortDisable");
            eCmdNative = MAIN_THREAD_CMD_STOP_PORT;
            break;
        }

        case OMX_CommandPortEnable:
        {
            logv(" send_command: OMX_CommandPortEnable");
            eCmdNative = MAIN_THREAD_CMD_RESTART_PORT;
            if (uParam1 == 0x0 || (int)uParam1 == -1)
			{
		impl->m_sInPortDefType.bEnabled = OMX_TRUE;
		impl->mInportEnable = OMX_TRUE;
            }

            if (uParam1 == 0x1 || (int)uParam1 == -1)
			{
                impl->m_sOutPortDefType.bEnabled = OMX_TRUE;
                impl->mOutportEnable = OMX_TRUE;
            }
            break;
        }

        case OMX_CommandMarkBuffer:
        {
            logv(" send_command: OMX_CommandMarkBuffer");
            eCmdNative = MAIN_THREAD_CMD_MARK_BUF;
             if (uParam1 > 0)
            {
                loge("Error: Send OMX_CommandMarkBuffer command but uParam1 invalid.");
                return OMX_ErrorBadPortIndex;
            }
            break;
        }

        default:
        {
            return OMX_ErrorBadPortIndex;
        }
    }

    post_event(impl, eCmdNative, uParam1, pCmdData);

    return eError;
}
OMX_ERRORTYPE __AwOmxVencGetParameter(OMX_IN OMX_HANDLETYPE pHComp,
                                       OMX_IN OMX_INDEXTYPE  eParamIndex,
                                       OMX_INOUT OMX_PTR     pParamData)
{
    AwOmxVenc* impl = (AwOmxVenc*) pHComp;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    CEDARC_UNUSE(pHComp);

    if (impl->m_state == OMX_StateInvalid)
    {
        logv("Get Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (pParamData == NULL)
    {
        logv("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }
    logv("get_parameter: eParamIndex: 0x%x", (int)eParamIndex);

    switch (eParamIndex)
    {
        case OMX_IndexParamVideoInit:
        {
            logv(" get_parameter:: OMX_IndexParamVideoInit");
            memcpy(pParamData, &impl->m_sPortParam, sizeof(OMX_PORT_PARAM_TYPE));
            break;
        }

        case OMX_IndexParamPortDefinition:
        {
            // android::CallStack stack;
            // stack.update(1, 100);
            // stack.dump("get_parameter");
            logv(" get_parameter: OMX_IndexParamPortDefinition");
            if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex ==
                 impl->m_sInPortDefType.nPortIndex)
            {
                memcpy(pParamData, &impl->m_sInPortDefType, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                logd("InPort definition: width = %d, height = %d",
                    (int)impl->m_sInPortDefType.format.video.nFrameWidth,
                    (int)impl->m_sInPortDefType.format.video.nFrameHeight);
                logv("def init. populated? InPort %d OutPort %d.",
                    impl->m_sInPortDefType.bPopulated, impl->m_sOutPortDefType.bPopulated);
                logv("def init. enable? InPort %d OutPort %d.",
                    impl->m_sInPortDefType.bEnabled, impl->m_sOutPortDefType.bEnabled);
            }
            else if (((OMX_PARAM_PORTDEFINITIONTYPE*)(pParamData))->nPortIndex ==
                     impl->m_sOutPortDefType.nPortIndex)
            {
                memcpy(pParamData, &impl->m_sOutPortDefType, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                logd("OutPort definition: width = %d, height = %d",
                    (int)impl->m_sOutPortDefType.format.video.nFrameWidth,
                    (int)impl->m_sOutPortDefType.format.video.nFrameHeight);
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            logv("get_parameter: OMX_IndexParamVideoPortFormat");

            OMX_VIDEO_PARAM_PORTFORMATTYPE * param_portformat_type =
                (OMX_VIDEO_PARAM_PORTFORMATTYPE *)(pParamData);
            if (param_portformat_type->nPortIndex == impl->m_sInPortFormatType.nPortIndex)
            {
                if (param_portformat_type->nIndex > impl->m_sInPortFormatType.nIndex)
                {
                    logw("get_parameter erro: pParamData->nIndex > m_sInPortFormatType.nIndex,"\
                        "param index %ld. inport index %ld.",
                        param_portformat_type->nIndex, impl->m_sInPortFormatType.nIndex);
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    param_portformat_type->eCompressionFormat = (OMX_VIDEO_CODINGTYPE)0;
                    param_portformat_type->eColorFormat =
                        impl->m_inputcolorFormats[param_portformat_type->nIndex];
                }
            }
            else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nPortIndex ==
                     impl->m_sOutPortFormatType.nPortIndex)
            {
                if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex >
                    impl->m_sOutPortFormatType.nIndex)
                {
                    loge("get_parameter erro: pParamData->nIndex > m_sOutPortFormatType.nIndex");
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(pParamData, &impl->m_sOutPortFormatType,
                           sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
                }
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            logv(" get_parameter: OMX_IndexParamStandardComponentRole");
            OMX_PARAM_COMPONENTROLETYPE* comp_role;
            comp_role                    = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            comp_role->nVersion.nVersion = OMX_SPEC_VERSION;
            comp_role->nSize             = sizeof(*comp_role);
            strncpy((char*)comp_role->cRole, (const char*)impl->m_cRole, OMX_MAX_STRINGNAME_SIZE);
            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logv(" get_parameter: OMX_IndexParamPriorityMgmt");
            memcpy(pParamData, &impl->m_sPriorityMgmtType, sizeof(OMX_PRIORITYMGMTTYPE));
            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            logv(" get_parameter: OMX_IndexParamCompBufferSupplier");
            OMX_PARAM_BUFFERSUPPLIERTYPE* pBuffSupplierParam =
                (OMX_PARAM_BUFFERSUPPLIERTYPE*)pParamData;
            if (pBuffSupplierParam->nPortIndex == 1)
            {
                pBuffSupplierParam->eBufferSupplier = impl->m_sOutBufSupplierType.eBufferSupplier;
            }
            else if (pBuffSupplierParam->nPortIndex == 0)
            {
                pBuffSupplierParam->eBufferSupplier = impl->m_sInBufSupplierType.eBufferSupplier;
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoBitrate:
        {
            logv(" get_parameter: OMX_IndexParamVideoBitrate");
            if (((OMX_VIDEO_PARAM_BITRATETYPE*)(pParamData))->nPortIndex ==
                impl->m_sOutPutBitRateType.nPortIndex)
            {
                memcpy(pParamData,&impl->m_sOutPutBitRateType, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoAvc:
        {
            logv(" get_parameter: OMX_IndexParamVideoAvc");
            OMX_VIDEO_PARAM_AVCTYPE* pComponentParam = (OMX_VIDEO_PARAM_AVCTYPE*)pParamData;

            if (pComponentParam->nPortIndex == impl->m_sH264Type.nPortIndex)
            {
                memcpy(pComponentParam, &impl->m_sH264Type, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamAudioInit:
        {
            logv(" get_parameter: OMX_IndexParamAudioInit");
            OMX_PORT_PARAM_TYPE *audioPortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            audioPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            audioPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            audioPortParamType->nPorts            = 0;
            audioPortParamType->nStartPortNumber  = 0;

            break;
        }

        case OMX_IndexParamImageInit:
        {
            logv(" get_parameter: OMX_IndexParamImageInit");
            OMX_PORT_PARAM_TYPE *imagePortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            imagePortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            imagePortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            imagePortParamType->nPorts            = 0;
            imagePortParamType->nStartPortNumber  = 0;
            break;
        }

        case OMX_IndexParamOtherInit:
        {
            logv(" get_parameter: OMX_IndexParamOtherInit");
            OMX_PORT_PARAM_TYPE *otherPortParamType = (OMX_PORT_PARAM_TYPE *) pParamData;

            otherPortParamType->nVersion.nVersion = OMX_SPEC_VERSION;
            otherPortParamType->nSize             = sizeof(OMX_PORT_PARAM_TYPE);
            otherPortParamType->nPorts            = 0;
            otherPortParamType->nStartPortNumber  = 0;
            break;
        }

        case OMX_IndexParamVideoH263:
        {
            logv("get_parameter: OMX_IndexParamVideoH263, do nothing.\n");
            break;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            logv("get_parameter: OMX_IndexParamVideoMpeg4, do nothing.\n");
            break;
        }
        case OMX_IndexParamVideoProfileLevelQuerySupported:
        {
            VIDEO_PROFILE_LEVEL_TYPE* pProfileLevel = NULL;
            OMX_U32 nNumberOfProfiles = 0;
            OMX_VIDEO_PARAM_PROFILELEVELTYPE *pParamProfileLevel =
                (OMX_VIDEO_PARAM_PROFILELEVELTYPE *)pParamData;

            pParamProfileLevel->nPortIndex = impl->m_sOutPortDefType.nPortIndex;

            /* Choose table based on compression format */
            switch (impl->m_sOutPortDefType.format.video.eCompressionFormat)
            {
                case OMX_VIDEO_CodingAVC:
                {
                    pProfileLevel = SupportedAVCProfileLevels;
                    nNumberOfProfiles = sizeof(SupportedAVCProfileLevels) /
                                        sizeof (VIDEO_PROFILE_LEVEL_TYPE);
                    break;
                }
                case OMX_VIDEO_CodingHEVC:
                {
                    pProfileLevel = SupportedHEVCProfileLevels;
                    nNumberOfProfiles = sizeof(SupportedHEVCProfileLevels) /
                                        sizeof (VIDEO_PROFILE_LEVEL_TYPE);
                    break;
                }
                case OMX_VIDEO_CodingH263:
                {
                    pProfileLevel = SupportedH263ProfileLevels;
                    nNumberOfProfiles = sizeof(SupportedH263ProfileLevels) /
                                        sizeof (VIDEO_PROFILE_LEVEL_TYPE);
                    break;
                }
                default:
                {
                    return OMX_ErrorBadParameter;
                }
            }

            if (pParamProfileLevel->nProfileIndex >= (nNumberOfProfiles - 1))
            {
                return OMX_ErrorBadParameter;
            }

            /* Point to table entry based on index */
            pProfileLevel += pParamProfileLevel->nProfileIndex;

            /* -1 indicates end of table */
            if (pProfileLevel->nProfile != -1)
            {
                pParamProfileLevel->eProfile = pProfileLevel->nProfile;
                pParamProfileLevel->eLevel = pProfileLevel->nLevel;
                eError = OMX_ErrorNone;
            }
            else
            {
                eError = OMX_ErrorNoMore;
            }
            break;
        }
        case OMX_IndexParamVideoQuantization:
        {
            OMX_VIDEO_PARAM_QUANTIZATIONTYPE* pComponentParam =
                (OMX_VIDEO_PARAM_QUANTIZATIONTYPE*)pParamData;
            logv(" get_parameter: OMX_IndexParamVideoQuantization");

            pComponentParam->nVersion.nVersion = OMX_SPEC_VERSION;
            pComponentParam->nSize             = sizeof(OMX_VIDEO_PARAM_QUANTIZATIONTYPE);
            pComponentParam->nPortIndex        = 1;
            pComponentParam->nQpI              = 20;//To set default values.
            pComponentParam->nQpP              = 30;
            pComponentParam->nQpB              = 30;;
            break;
        }
        case OMX_IndexConfigVideoAVCIntraPeriod:
        {
            logv("get_parameter: OMX_IndexConfigVideoAVCIntraPeriod");
            OMX_VIDEO_CONFIG_AVCINTRAPERIOD* pData =
                (OMX_VIDEO_CONFIG_AVCINTRAPERIOD*)(pParamData);
            memcpy(pData, &impl->m_avcIntraPeriod, sizeof(OMX_VIDEO_CONFIG_AVCINTRAPERIOD));
            break;
        }

        default:
        {
            eError = getDefaultParameter(impl, eParamIndex, pParamData);
        }
    }

    return eError;
}

OMX_ERRORTYPE __AwOmxVencSetParameter(OMX_IN OMX_HANDLETYPE pHComp,
                                           OMX_IN OMX_INDEXTYPE eParamIndex,
                                           OMX_IN OMX_PTR pParamData)
{
    AwOmxVenc* impl = (AwOmxVenc*) pHComp;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    CEDARC_UNUSE(pHComp);

    if (impl->m_state == OMX_StateInvalid)
    {
        logv("Set Param in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (pParamData == NULL)
    {
        logv("Get Param in Invalid pParamData \n");
        return OMX_ErrorBadParameter;
    }

    logv("set_parameter: eParamIndex: 0x%x", (int)eParamIndex);

    switch (eParamIndex)
    {
        case OMX_IndexParamPortDefinition:
        {
            logv(" set_parameter: OMX_IndexParamPortDefinition");

            if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex ==
                impl->m_sInPortDefType.nPortIndex)
            {
                logv("set_parameter: InPort buffer count actually: %d",
                     (int)(((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual));
                if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual !=
                   impl->m_sInPortDefType.nBufferCountActual)
                {
                    int nBufCnt;
                    int nIndex;

                    pthread_mutex_lock(&impl->m_inBufMutex);

                    if (impl->m_sInBufList.pBufArr != NULL)
                        free(impl->m_sInBufList.pBufArr);

                    if (impl->m_sInBufList.pBufHdrList != NULL)
                        free(impl->m_sInBufList.pBufHdrList);

                    nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual;
                    logv(" set_parameter: allocate %d input buffers.", (int)nBufCnt);

                    impl->m_sInBufList.pBufArr =
                        (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE)* nBufCnt);
                    impl->m_sInBufList.pBufHdrList =
                        (OMX_BUFFERHEADERTYPE**)malloc(sizeof(OMX_BUFFERHEADERTYPE*)* nBufCnt);
                    for (nIndex = 0; nIndex < nBufCnt; nIndex++)
                    {
                        OMX_CONF_INIT_STRUCT_PTR (&impl->m_sInBufList.pBufArr[nIndex],
                                                  OMX_BUFFERHEADERTYPE);
                    }

                    impl->m_sInBufList.nSizeOfList       = 0;
                    impl->m_sInBufList.nAllocSize        = 0;
                    impl->m_sInBufList.nWritePos         = 0;
                    impl->m_sInBufList.nReadPos          = 0;
                    impl->m_sInBufList.nAllocBySelfFlags = 0;
                    impl->m_sInBufList.nSizeOfList       = 0;
                    impl->m_sInBufList.nBufArrSize       = nBufCnt;
                    impl->m_sInBufList.eDir              = OMX_DirInput;

                    pthread_mutex_unlock(&impl->m_inBufMutex);
                }
                memcpy(&impl->m_sInPortDefType, pParamData, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                logv("def init. populated? InPort %d OutPort %d.",
			impl->m_sInPortDefType.bPopulated, impl->m_sOutPortDefType.bPopulated);
                logv("def init. enalbe? InPort %d OutPort %d.",
                    impl->m_sInPortDefType.bEnabled, impl->m_sOutPortDefType.bEnabled);
                if(impl->m_sInPortDefType.bEnabled != impl->mInportEnable){
                    impl->m_sInPortDefType.bEnabled = impl->mInportEnable;
                    logv("reset in port enable flag.");
                }
                if(impl->m_sOutPortDefType.bEnabled != impl->mOutportEnable){
                    impl->m_sOutPortDefType.bEnabled = impl->mOutportEnable;
                    logv("reset out port enable flag.");
                }

                impl->m_sInPortDefType.format.video.nStride =
                    ALIGN_16B(impl->m_sInPortDefType.format.video.nStride);
                impl->m_sInPortDefType.nBufferSize =
                    impl->m_sInPortDefType.format.video.nFrameWidth \
                    * impl->m_sInPortDefType.format.video.nFrameHeight *3/2;
                #ifdef CONF_OMX_ENABLE_ALIGN//tina platform requires this setting!!!
                if (impl->m_vencCodecType == VENC_CODEC_JPEG
                    || impl->m_vencCodecType == VENC_CODEC_H264)
                {
                    impl->m_sInPortDefType.format.video.nFrameWidth
                        = ALIGN_16B(impl->m_sInPortDefType.format.video.nFrameWidth);
                    impl->m_sInPortDefType.format.video.nFrameHeight
                        = ALIGN_16B(impl->m_sInPortDefType.format.video.nFrameHeight);
                    impl->m_sInPortDefType.format.video.nSliceHeight
                        = impl->m_sInPortDefType.format.video.nFrameHeight;
                    impl->m_sInPortDefType.nBufferSize
                        = impl->m_sInPortDefType.format.video.nFrameWidth *
                        impl->m_sInPortDefType.format.video.nFrameHeight * 3 / 2;
                }
                #endif
                logd("init_input_port: stride = %d, width = %d, height = %d",
                     (int)impl->m_sInPortDefType.format.video.nStride,
                     (int)impl->m_sInPortDefType.format.video.nFrameWidth,
                     (int)impl->m_sInPortDefType.format.video.nFrameHeight);
            }
            else if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nPortIndex ==
                     impl->m_sOutPortDefType.nPortIndex)
            {
                logv("set_parameter: OutPort buffer count actually %d",
                     (int)((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual);
                if (((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual !=
                    impl->m_sOutPortDefType.nBufferCountActual)
                {
                    int nBufCnt;
                    int nIndex;

                    pthread_mutex_lock(&impl->m_outBufMutex);

                    if (impl->m_sOutBufList.pBufArr != NULL)
                    {
                        free(impl->m_sOutBufList.pBufArr);
                    }

                    if (impl->m_sOutBufList.pBufHdrList != NULL)
                    {
                        free(impl->m_sOutBufList.pBufHdrList);
                    }

                    nBufCnt = ((OMX_PARAM_PORTDEFINITIONTYPE *)(pParamData))->nBufferCountActual;
                    logv(" set_parameter: allocate %d output buffers.", (int)nBufCnt);
                    // Initialize the output buffer list
                    impl->m_sOutBufList.pBufArr =
                        (OMX_BUFFERHEADERTYPE*)malloc(sizeof(OMX_BUFFERHEADERTYPE) * nBufCnt);
                    impl->m_sOutBufList.pBufHdrList =
                        (OMX_BUFFERHEADERTYPE**) malloc(sizeof(OMX_BUFFERHEADERTYPE*) * nBufCnt);
                    for (nIndex = 0; nIndex < nBufCnt; nIndex++)
                    {
                        OMX_CONF_INIT_STRUCT_PTR (&impl->m_sOutBufList.pBufArr[nIndex],
                                                  OMX_BUFFERHEADERTYPE);
                    }

                    impl->m_sOutBufList.nSizeOfList       = 0;
                    impl->m_sOutBufList.nAllocSize        = 0;
                    impl->m_sOutBufList.nWritePos         = 0;
                    impl->m_sOutBufList.nReadPos          = 0;
                    impl->m_sOutBufList.nAllocBySelfFlags = 0;
                    impl->m_sOutBufList.nSizeOfList       = 0;
                    impl->m_sOutBufList.nBufArrSize       = nBufCnt;
                    impl->m_sOutBufList.eDir              = OMX_DirOutput;

                    pthread_mutex_unlock(&impl->m_outBufMutex);
                }
                memcpy(&impl->m_sOutPortDefType, pParamData, sizeof(OMX_PARAM_PORTDEFINITIONTYPE));
                impl->m_sOutPortDefType.nBufferSize =
                    impl->m_sOutPortDefType.format.video.nFrameWidth *
                    impl->m_sOutPortDefType.format.video.nFrameHeight * 3 / 2;
                impl->m_sOutPortDefType.nBufferSize = \
                    omx_venc_align(impl->m_sOutPortDefType.nBufferSize, BUF_ALIGN_SIZE);
              //Android set framerate (fps) as xFramerate >> 16 while linux as xFramerate itself.
                impl->m_framerate = (impl->m_sInPortDefType.format.video.xFramerate/*>>16 */);
                if(!impl->m_framerate)
                {
                    logw("frame rate = 0!! reset it as default 30 fps.");
                    impl->m_framerate = 30;
                }
                logd("init_output_port: framerate: %ld(>>16) %ld, output width %lu height %lu.",
                    impl->m_sInPortDefType.format.video.xFramerate,
                    impl->m_sInPortDefType.format.video.xFramerate>>16,
                    impl->m_sOutPortDefType.format.video.nFrameWidth,
                    impl->m_sOutPortDefType.format.video.nFrameHeight);
            }
            else
                eError = OMX_ErrorBadPortIndex;
            break;
        }

        case OMX_IndexParamVideoPortFormat:
        {
            logd(" set_parameter: OMX_IndexParamVideoPortFormat");

            OMX_VIDEO_PARAM_PORTFORMATTYPE * param_portformat_type =
                (OMX_VIDEO_PARAM_PORTFORMATTYPE *)(pParamData);
             if (param_portformat_type->nPortIndex == impl->m_sInPortFormatType.nPortIndex)
             {
                 if (param_portformat_type->nIndex > impl->m_sInPortFormatType.nIndex)
                 {
                     loge("set_parameter erro: pParamData->nIndex > m_sInPortFormatType.nIndex");
                     eError = OMX_ErrorNoMore;
                 }
                else
                {
                    impl->m_sInPortFormatType.eColorFormat = param_portformat_type->eColorFormat;
                    impl->m_sInPortDefType.format.video.eColorFormat = impl->m_sInPortFormatType.eColorFormat;
                    logd("set video port format 0x%x",
                         impl->m_sInPortFormatType.eColorFormat);
                }
            }
            else if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nPortIndex ==
                     impl->m_sOutPortFormatType.nPortIndex)
            {
                if (((OMX_VIDEO_PARAM_PORTFORMATTYPE*)(pParamData))->nIndex >
                    impl->m_sOutPortFormatType.nIndex)
                {
                    loge("set_parameter erro: pParamData->nIndex > m_sOutPortFormatType.nIndex");
                    eError = OMX_ErrorNoMore;
                }
                else
                {
                    memcpy(&impl->m_sOutPortFormatType, pParamData,
                           sizeof(OMX_VIDEO_PARAM_PORTFORMATTYPE));
                }
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }

            break;
        }

        case OMX_IndexParamStandardComponentRole:
        {
            logv(" set_parameter: OMX_IndexParamStandardComponentRole");
            OMX_PARAM_COMPONENTROLETYPE *comp_role;
            comp_role = (OMX_PARAM_COMPONENTROLETYPE *) pParamData;
            logv("set_parameter: OMX_IndexParamStandardComponentRole %s\n", comp_role->cRole);

            if ((OMX_StateLoaded == impl->m_state)
               /* && !BITMASK_PRESENT(&m_flags,OMX_COMPONENT_IDLE_PENDING)*/)
            {
                logv("Set Parameter called in valid state");
            }
            else
            {
                logv("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }

            if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.avc",
                         OMX_MAX_STRINGNAME_SIZE))
            {
                if (!strncmp((char*)comp_role->cRole, "video_encoder.avc",
                             OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)impl->m_cRole,"video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logv("Setparameter: unknown Index %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.hevc",
                              OMX_MAX_STRINGNAME_SIZE))
            {
                if (!strncmp((char*)comp_role->cRole, "video_encoder.hevc",
                             OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)impl->m_cRole,"video_encoder.hevc", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logv("Setparameter: unknown Index %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.mjpeg",
                              OMX_MAX_STRINGNAME_SIZE))
            {
                if (!strncmp((char*)comp_role->cRole, "video_encoder.mjpeg",
                             OMX_MAX_STRINGNAME_SIZE))
                {
                    strncpy((char*)impl->m_cRole,"video_encoder.mjpeg", OMX_MAX_STRINGNAME_SIZE);
                }
                else
                {
                    logv("Setparameter: unknown Index %s\n", comp_role->cRole);
                    eError =OMX_ErrorUnsupportedSetting;
                }
            }
            else
            {
                logv("Setparameter: unknown param %s\n", impl->m_cName);
                eError = OMX_ErrorInvalidComponentName;
            }
            break;
        }

        case OMX_IndexParamPriorityMgmt:
        {
            logv(" set_parameter: OMX_IndexParamPriorityMgmt");
            if (impl->m_state != OMX_StateLoaded)
            {
                logv("Set Parameter called in Invalid State\n");
                return OMX_ErrorIncorrectStateOperation;
            }
            OMX_PRIORITYMGMTTYPE *priorityMgmtype = (OMX_PRIORITYMGMTTYPE*) pParamData;
            impl->m_sPriorityMgmtType.nGroupID = priorityMgmtype->nGroupID;
            impl->m_sPriorityMgmtType.nGroupPriority = priorityMgmtype->nGroupPriority;
            break;
        }

        case OMX_IndexParamCompBufferSupplier:
        {
            OMX_PARAM_BUFFERSUPPLIERTYPE *bufferSupplierType =
                (OMX_PARAM_BUFFERSUPPLIERTYPE*) pParamData;
            logv("set_parameter: OMX_IndexParamCompBufferSupplier %d\n",
                 bufferSupplierType->eBufferSupplier);
            if (bufferSupplierType->nPortIndex == 0)
                impl->m_sInBufSupplierType.eBufferSupplier = bufferSupplierType->eBufferSupplier;
            else if (bufferSupplierType->nPortIndex == 1)
                impl->m_sOutBufSupplierType.eBufferSupplier = bufferSupplierType->eBufferSupplier;
            else
                eError = OMX_ErrorBadPortIndex;
            break;
        }

        case OMX_IndexParamVideoBitrate:
        {
            logv(" set_parameter: OMX_IndexParamVideoBitrate");
            OMX_VIDEO_PARAM_BITRATETYPE* pComponentParam =
                (OMX_VIDEO_PARAM_BITRATETYPE*)pParamData;
            if (pComponentParam->nPortIndex == impl->m_sOutPutBitRateType.nPortIndex)
            {
                memcpy(&impl->m_sOutPutBitRateType,pComponentParam, sizeof(OMX_VIDEO_PARAM_BITRATETYPE));

                if (!impl->m_sOutPutBitRateType.nTargetBitrate)
                {
                    impl->m_sOutPutBitRateType.nTargetBitrate = DEFAULT_BITRATE;
                }

                impl->m_sOutPortDefType.format.video.nBitrate = impl->m_sOutPutBitRateType.nTargetBitrate;

                if (impl->m_state == OMX_StateExecuting && impl->m_encoder)
                {
                    post_message_to_venc(impl, OMX_Venc_Cmd_ChangeBitrate);
                }
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoAvc:
        {
            logv(" set_parameter: OMX_IndexParamVideoAvc");
            OMX_VIDEO_PARAM_AVCTYPE* pComponentParam = (OMX_VIDEO_PARAM_AVCTYPE*)pParamData;

            if (pComponentParam->nPortIndex == impl->m_sH264Type.nPortIndex)
            {
                memcpy(&impl->m_sH264Type,pComponentParam, sizeof(OMX_VIDEO_PARAM_AVCTYPE));
                //CalculateBufferSize(pCompPortOut->pPortDef, pComponentPrivate);
            }
            else
            {
                eError = OMX_ErrorBadPortIndex;
            }
            break;
        }

        case OMX_IndexParamVideoH263:
        {
            logv(" set_parameter: OMX_IndexParamVideoH263");
            break;
        }

        case OMX_IndexParamVideoMpeg4:
        {
            logv(" set_parameter: OMX_IndexParamVideoMpeg4");
            break;
        }
        case OMX_IndexParamVideoIntraRefresh:
        {
            OMX_VIDEO_PARAM_INTRAREFRESHTYPE* pComponentParam =
                (OMX_VIDEO_PARAM_INTRAREFRESHTYPE*)pParamData;
            if (pComponentParam->nPortIndex == 1 &&
               pComponentParam->eRefreshMode == OMX_VIDEO_IntraRefreshCyclic)
            {
                int mbWidth, mbHeight;
                mbWidth = (impl->m_sInPortDefType.format.video.nFrameWidth + 15)/16;
                mbHeight = (impl->m_sInPortDefType.format.video.nFrameHeight + 15)/16;
                impl->m_vencCyclicIntraRefresh.bEnable = 1;
                impl->m_vencCyclicIntraRefresh.nBlockNumber = mbWidth*mbHeight/pComponentParam->nCirMBs;
                logd("set_parameter: m_vencCyclicIntraRefresh.nBlockNumber: %d",
                     impl->m_vencCyclicIntraRefresh.nBlockNumber);
            }
            break;
        }
        case OMX_IndexParamVideoQuantization:
        {
            OMX_VIDEO_PARAM_QUANTIZATIONTYPE* pComponentParam =
                (OMX_VIDEO_PARAM_QUANTIZATIONTYPE*)pParamData;
            impl->m_fixQp.bEnable = OMX_TRUE;
            impl->m_fixQp.nIQp = pComponentParam->nQpI;
            impl->m_fixQp.nPQp = pComponentParam->nQpP;
            logd("set_parameter: VideoQuantization. PortIdx %ld. IQp: %d. PQp: %d.",
                pComponentParam->nPortIndex, impl->m_fixQp.nIQp, impl->m_fixQp.nPQp);
            break;
        }
        case OMX_IndexConfigVideoAVCIntraPeriod:
        {
            OMX_VIDEO_CONFIG_AVCINTRAPERIOD* pComponentParam =
                (OMX_VIDEO_CONFIG_AVCINTRAPERIOD*)(pParamData);
            impl->m_enableAvcIntraPeriod = OMX_TRUE;
            memcpy(&impl->m_avcIntraPeriod, pComponentParam, sizeof(OMX_VIDEO_CONFIG_AVCINTRAPERIOD));
            logd("set_parameter: ConfigAVCIntraPeriod. PortIdx %ld. IDRPeriod: %ld. PFrames: %ld.",
                pComponentParam->nPortIndex,
                impl->m_avcIntraPeriod.nIDRPeriod, impl->m_avcIntraPeriod.nPFrames);
            break;
        }

        default:
        {
            eError = setDefaultParameter(impl, eParamIndex, pParamData);
        }
    }
    //TO REMOVE: ************test********
   //case OMX_IndexParamVideoQuantization:
   /*{
       OMX_VIDEO_PARAM_QUANTIZATIONTYPE* pComponentParam =
           (OMX_VIDEO_PARAM_QUANTIZATIONTYPE*)pParamData;
       impl->m_fixQp.bEnable = OMX_TRUE;
       impl->m_fixQp.nIQp = 15;//pComponentParam->nQpI;
       impl->m_fixQp.nPQp = 35;//pComponentParam->nQpP;
       logd("set_parameter: VideoQuantization. PortIdx %ld. IQp: %d. PQp: %d.",
           pComponentParam->nPortIndex, impl->m_fixQp.nIQp, impl->m_fixQp.nPQp);
   }*/
   //case OMX_IndexConfigVideoAVCIntraPeriod:
   /*{
       OMX_VIDEO_CONFIG_AVCINTRAPERIOD* pComponentParam =
           (OMX_VIDEO_CONFIG_AVCINTRAPERIOD*)(pParamData);
       impl->m_enableAvcIntraPeriod = OMX_TRUE;
        impl->m_avcIntraPeriod.nIDRPeriod = 20;
        impl->m_avcIntraPeriod.nPFrames = 19;
       //memcpy(&impl->m_avcIntraPeriod, pComponentParam, sizeof(OMX_VIDEO_CONFIG_AVCINTRAPERIOD));
       logd("set_parameter: ConfigAVCIntraPeriod. PortIdx %ld. IDRPeriod: %ld. PFrames: %ld.",
           pComponentParam->nPortIndex,
           impl->m_avcIntraPeriod.nIDRPeriod, impl->m_avcIntraPeriod.nPFrames);
   }*/

    return eError;
}

OMX_ERRORTYPE __AwOmxVencGetConfig(OMX_IN OMX_HANDLETYPE pHComp,
                                        OMX_IN OMX_INDEXTYPE eConfigIndex,
                                        OMX_INOUT OMX_PTR pConfigData)
{
    AwOmxVenc* impl = (AwOmxVenc*) pHComp;
    OMX_ERRORTYPE eError = OMX_ErrorNone;

    logv(" get_config: index = %d", eConfigIndex);
    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(pConfigData);

    if (impl->m_state == OMX_StateInvalid)
    {
        logv("get_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    switch (eConfigIndex)
    {
        default:
        {
            logv("get_config: unknown param %d\n",eConfigIndex);
            eError = OMX_ErrorUnsupportedIndex;
        }
    }

    return eError;
}


OMX_ERRORTYPE __AwOmxVencSetConfig(OMX_IN OMX_HANDLETYPE pHComp,
                                   OMX_IN OMX_INDEXTYPE eConfigIndex,
                                   OMX_IN OMX_PTR pConfigData)
{
    logv(" set_config: index = %d", eConfigIndex);
    AwOmxVenc* impl = (AwOmxVenc*) pHComp;

    CEDARC_UNUSE(pHComp);

    if (impl->m_state == OMX_StateInvalid)
    {
        logv("set_config in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    OMX_ERRORTYPE eError = OMX_ErrorNone;

    switch (eConfigIndex)
    {
        case OMX_IndexConfigVideoIntraVOPRefresh:
        {
            if (impl->m_state == OMX_StateExecuting && impl->m_encoder)
            {
                post_message_to_venc(impl, OMX_Venc_Cmd_RequestIDRFrame);
                logd("venc, OMX_Venc_Cmd_RequestIDRFrame");
            }
            break;
        }
        case OMX_IndexConfigVideoBitrate:
        {
            OMX_VIDEO_CONFIG_BITRATETYPE* pData = (OMX_VIDEO_CONFIG_BITRATETYPE*)(pConfigData);

            if (pData->nPortIndex == 1)
            {
                if (impl->m_state == OMX_StateExecuting && impl->m_encoder)
                {
                    impl->m_sOutPortDefType.format.video.nBitrate = pData->nEncodeBitrate;
                    post_message_to_venc(impl, OMX_Venc_Cmd_ChangeBitrate);
                    logd("function:%s, line:%d , pData->nEncodeBitrate = %lu",
                         __FUNCTION__,__LINE__,pData->nEncodeBitrate);
                }
            }
            break;
        }
        case OMX_IndexConfigVideoFramerate:
        {
            OMX_CONFIG_FRAMERATETYPE* pData = (OMX_CONFIG_FRAMERATETYPE*)(pConfigData);
            logd("function:%s, line:%d. (Microphone)", __FUNCTION__,__LINE__);
            if (pData->nPortIndex == 0)
            {
                if (impl->m_state == OMX_StateExecuting && impl->m_encoder)
                {
                    impl->m_sInPortDefType.format.video.xFramerate = pData->xEncodeFramerate;
                    impl->m_framerate = (impl->m_sInPortDefType.format.video.xFramerate/*>>16*/);
                    if(!impl->m_framerate)
                    {
                        logw("frame rate = 0!! reset it as default 30 fps.");
                        impl->m_framerate = 30;
                    }
                    post_message_to_venc(impl, OMX_Venc_Cmd_ChangeFramerate);
                    logv("function:%s, line:%d , pData->xEncodeFramerate = %lu",
                         __FUNCTION__,__LINE__,impl->m_framerate);
                }
            }
            break;
        }

        default:
        {
            logv("set_config: unknown param %d\n",eConfigIndex);
            eError = OMX_ErrorUnsupportedIndex;
            break;
        }
    }

    return eError;
}

OMX_ERRORTYPE __AwOmxVencGetExtensionIndex(OMX_IN OMX_HANDLETYPE pHComp,
                                             OMX_IN OMX_STRING pParamName,
                                             OMX_OUT OMX_INDEXTYPE* pIndexType)
{
    AwOmxVenc* impl = (AwOmxVenc*) pHComp;
    unsigned int  nIndex;
    OMX_ERRORTYPE eError = OMX_ErrorUndefined;

    logd("get extension index, param name = %s", pParamName);
    if (impl->m_state == OMX_StateInvalid)
    {
        logv("Get Extension Index in Invalid State\n");
        return OMX_ErrorInvalidState;
    }

    if (pHComp == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if(strcmp((char *)pParamName, "OMX.google.android.index.storeANWBufferInMetadata")
            == 0)
    {
        logw("do not support OMX.google.android.index.storeANWBufferInMetadata\n");
        return OMX_ErrorUnsupportedIndex;
    }

    for (nIndex = 0; nIndex < sizeof(sVideoEncCustomParams)/sizeof(VIDDEC_CUSTOM_PARAM); nIndex++)
    {
        if (strcmp((char *)pParamName, (char *)&(sVideoEncCustomParams[nIndex].cCustomParamName))
            == 0)
        {
            *pIndexType = sVideoEncCustomParams[nIndex].nCustomParamIndex;
            eError = OMX_ErrorNone;
            break;
        }
    }
    return eError;
}

OMX_ERRORTYPE __AwOmxVencGetState(OMX_IN OMX_HANDLETYPE pHComp, OMX_OUT OMX_STATETYPE* pState)
{
    logv(" COMPONENT_GET_STATE");
    AwOmxVenc* impl = (AwOmxVenc*) pHComp;
    if (pHComp == NULL || pState == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    *pState = impl->m_state;
    return OMX_ErrorNone;
}

OMX_ERRORTYPE __AwOmxVencTunnelRequest(OMX_IN    OMX_HANDLETYPE       pHComp,
                                                 OMX_IN    OMX_U32              uPort,
                                                 OMX_IN    OMX_HANDLETYPE       pPeerComponent,
                                                 OMX_IN    OMX_U32              uPeerPort,
                                                 OMX_INOUT OMX_TUNNELSETUPTYPE* pTunnelSetup)
{
    logv(" COMPONENT_TUNNEL_REQUEST");

    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pPeerComponent);
    CEDARC_UNUSE(uPeerPort);
    CEDARC_UNUSE(pTunnelSetup);

    logv("Error: component_tunnel_request Not Implemented\n");
    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE __AwOmxVencUseBuffer(OMX_IN    OMX_HANDLETYPE          hComponent,
                                    OMX_INOUT OMX_BUFFERHEADERTYPE**  ppBufferHdr,
                                    OMX_IN    OMX_U32                 nPortIndex,
                                    OMX_IN    OMX_PTR                 pAppPrivate,
                                    OMX_IN    OMX_U32                 nSizeBytes,
                                    OMX_IN    OMX_U8*                 pBuffer)
{
    AwOmxVenc* impl = (AwOmxVenc*) hComponent;
    OMX_PARAM_PORTDEFINITIONTYPE*   pPortDef;
    OMX_U32                         nIndex = 0x0;
    logd("use buffer, nPortIndex: %d, nSizeBytes: %d", (int)nPortIndex, (int)nSizeBytes);

    if (hComponent == NULL || ppBufferHdr == NULL || pBuffer == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (nPortIndex == impl->m_sInPortDefType.nPortIndex)
        pPortDef = &impl->m_sInPortDefType;
    else if (nPortIndex == impl->m_sOutPortDefType.nPortIndex)
        pPortDef = &impl->m_sOutPortDefType;
    else
        return OMX_ErrorBadParameter;

    if (!pPortDef->bEnabled)
        return OMX_ErrorIncorrectStateOperation;

    if (pPortDef->bPopulated)
        return OMX_ErrorBadParameter;

    // Find an empty position in the BufferList and allocate memory for the buffer header.
    // Use the buffer passed by the client to initialize the actual buffer
    // inside the buffer header.
    if (nPortIndex == impl->m_sInPortDefType.nPortIndex)
    {
        pthread_mutex_lock(&impl->m_inBufMutex);
        logv("vencInPort: use_buffer");

        if ((int)impl->m_sInBufList.nAllocSize >= impl->m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&impl->m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = impl->m_sInBufList.nAllocSize;
        impl->m_sInBufList.nAllocSize++;

        impl->m_sInBufList.pBufArr[nIndex].pBuffer = pBuffer;
        impl->m_sInBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
        impl->m_sInBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
        impl->m_sInBufList.pBufArr[nIndex].nInputPortIndex = nPortIndex;
        impl->m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &impl->m_sInBufList.pBufArr[nIndex];
        if (impl->m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&impl->m_inBufMutex);
    }
    else
    {
        pthread_mutex_lock(&impl->m_outBufMutex);
        logv("vencOutPort: use_buffer");

        if ((int)impl->m_sOutBufList.nAllocSize >= impl->m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&impl->m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = impl->m_sOutBufList.nAllocSize;
        impl->m_sOutBufList.nAllocSize++;
        impl->m_sOutBufList.pBufArr[nIndex].pBuffer = pBuffer;
        impl->m_sOutBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
        impl->m_sOutBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
        impl->m_sOutBufList.pBufArr[nIndex].nInputPortIndex = 0xFFFFFFFE;
        impl->m_sOutBufList.pBufArr[nIndex].nOutputPortIndex = nPortIndex;
        *ppBufferHdr = &impl->m_sOutBufList.pBufArr[nIndex];
        if (impl->m_sOutBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&impl->m_outBufMutex);
    }

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE __AwOmxVencAllocateBuffer(OMX_IN    OMX_HANDLETYPE         hComponent,
                                        OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                        OMX_IN    OMX_U32                nPortIndex,
                                        OMX_IN    OMX_PTR                pAppPrivate,
                                        OMX_IN    OMX_U32                nSizeBytes)
{
    AwOmxVenc* impl = (AwOmxVenc*) hComponent;
    OMX_S8                        nIndex = 0x0;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;
    logd("allocate buffer, nPortIndex: %s, nSizeBytes: %d", (int)nPortIndex == 0 ?
        "InPort":"OutPort", (int)nSizeBytes);

    if (hComponent == NULL || ppBufferHdr == NULL)
    {
        logv("bad parameter component 0x%p, ppBufferHdr 0x%p.",hComponent, ppBufferHdr);
        return OMX_ErrorBadParameter;
    }

    if (nPortIndex == impl->m_sInPortDefType.nPortIndex)
    {
        pPortDef = &impl->m_sInPortDefType;
    }
    else
    {
        if (nPortIndex == impl->m_sOutPortDefType.nPortIndex)
        {
            logv("port_out: nPortIndex=%d", (int)nPortIndex);
            pPortDef = &impl->m_sOutPortDefType;
        }
        else
        {
            logv("allocate_buffer fatal error! nPortIndex=%d", (int)nPortIndex);
            return OMX_ErrorBadParameter;
        }
    }
    if (nPortIndex == impl->m_sInPortDefType.nPortIndex &&
                pPortDef->bEnabled != impl->mInportEnable)
    {
        pPortDef->bEnabled = impl->mInportEnable;
        logw("reset mInportEnable flag. ");
    }

    if (nPortIndex == impl->m_sOutPortDefType.nPortIndex &&
        pPortDef->bEnabled != impl->mOutportEnable)
    {
        pPortDef->bEnabled = impl->mOutportEnable;
        logw("reset mOutportEnable flag. ");
    }

    if (!pPortDef->bEnabled)
        return OMX_ErrorIncorrectStateOperation;

    if (nPortIndex == impl->m_sInPortDefType.nPortIndex &&
            impl->m_sInBufList.nAllocSize < pPortDef->nBufferCountActual)
    {
        pPortDef->bPopulated = OMX_FALSE;
        OMX_LOGI("reset populated flag. buffer count %lu.",
            pPortDef->nBufferCountActual);
    }

    if (nPortIndex == impl->m_sOutPortDefType.nPortIndex &&
        impl->m_sOutBufList.nAllocSize < pPortDef->nBufferCountActual)
    {
        pPortDef->bPopulated = OMX_FALSE;
        OMX_LOGI("reset populated flag. buffer count %lu.",
            pPortDef->nBufferCountActual);
    }

    if (pPortDef->bPopulated)
    {
        loge("bad paramerter. port is populated. nBufferCountActual %lu",
            pPortDef->nBufferCountActual);
        return OMX_ErrorBadParameter;
    }
    // Find an empty position in the BufferList and allocate memory for the buffer header
    // and the actual buffer
    if (nPortIndex == impl->m_sInPortDefType.nPortIndex)
    {
        pthread_mutex_lock(&impl->m_inBufMutex);
        logv("vencInPort: malloc vbs");

        if ((int)impl->m_sInBufList.nAllocSize >= impl->m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&impl->m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = impl->m_sInBufList.nAllocSize;

        impl->m_sInBufList.pBufArr[nIndex].pBuffer = (OMX_U8*)malloc(nSizeBytes);

        if (!impl->m_sInBufList.pBufArr[nIndex].pBuffer)
        {
            pthread_mutex_unlock(&impl->m_inBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        impl->m_sInBufList.nAllocBySelfFlags |= (1<<nIndex);

        impl->m_sInBufList.pBufArr[nIndex].nAllocLen   = nSizeBytes;
        impl->m_sInBufList.pBufArr[nIndex].pAppPrivate = pAppPrivate;
        impl->m_sInBufList.pBufArr[nIndex].nInputPortIndex = nPortIndex;
        impl->m_sInBufList.pBufArr[nIndex].nOutputPortIndex = 0xFFFFFFFE;
        *ppBufferHdr = &impl->m_sInBufList.pBufArr[nIndex];

        impl->m_sInBufList.nAllocSize++;
        if (impl->m_sInBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&impl->m_inBufMutex);
    }
    else
    {
        logv("vencOutPort: malloc frame");
//        android::CallStack stack;
//        stack.update(1, 100);
//        stack.dump("allocate_buffer_for_frame");
        pthread_mutex_lock(&impl->m_outBufMutex);

        if ((int)impl->m_sOutBufList.nAllocSize >= impl->m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&impl->m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        nIndex = impl->m_sOutBufList.nAllocSize;

        impl->m_sOutBufList.pBufArr[nIndex].pBuffer = (OMX_U8*)malloc(nSizeBytes);

        if (!impl->m_sOutBufList.pBufArr[nIndex].pBuffer)
        {
            pthread_mutex_unlock(&impl->m_outBufMutex);
            return OMX_ErrorInsufficientResources;
        }

        impl->m_sOutBufList.nAllocBySelfFlags |= (1<<nIndex);

        impl->m_sOutBufList.pBufArr[nIndex].nAllocLen        = nSizeBytes;
        impl->m_sOutBufList.pBufArr[nIndex].pAppPrivate      = pAppPrivate;
        impl->m_sOutBufList.pBufArr[nIndex].nInputPortIndex  = 0xFFFFFFFE;
        impl->m_sOutBufList.pBufArr[nIndex].nOutputPortIndex = nPortIndex;
        *ppBufferHdr = &impl->m_sOutBufList.pBufArr[nIndex];

        impl->m_sOutBufList.nAllocSize++;
        if (impl->m_sOutBufList.nAllocSize == pPortDef->nBufferCountActual)
        {
            pPortDef->bPopulated = OMX_TRUE;
        }

        pthread_mutex_unlock(&impl->m_outBufMutex);
    }

    return OMX_ErrorNone;
}
OMX_ERRORTYPE __AwOmxVencFreeBuffer(OMX_IN  OMX_HANDLETYPE        hComponent,
                                    OMX_IN  OMX_U32               nPortIndex,
                                    OMX_IN  OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    AwOmxVenc* impl = (AwOmxVenc*) hComponent;
    OMX_PARAM_PORTDEFINITIONTYPE* pPortDef;
    OMX_S32                       nIndex;

    logv(" free buffer, nPortIndex = %d, pBufferHdr = %p",
         (int)nPortIndex, pBufferHdr);

    if (hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    // Match the pBufferHdr to the appropriate entry in the BufferList
    // and free the allocated memory
    if (nPortIndex == impl->m_sInPortDefType.nPortIndex)
    {
        pPortDef = &impl->m_sInPortDefType;

        pthread_mutex_lock(&impl->m_inBufMutex);
        logv("vencInPort: free_buffer");

        for (nIndex = 0; nIndex < impl->m_sInBufList.nBufArrSize; nIndex++)
        {
            if (pBufferHdr == &impl->m_sInBufList.pBufArr[nIndex])
            {
                break;
            }
            if(nIndex == impl->m_sInBufList.nBufArrSize - 1)
            {
                impl->m_sInPortDefType.bEnabled = OMX_FALSE;
                impl->mInportEnable = OMX_FALSE;
            }
        }

        if (nIndex == impl->m_sInBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&impl->m_inBufMutex);
            return OMX_ErrorBadParameter;
        }

        if (impl->m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
        {
            free(impl->m_sInBufList.pBufArr[nIndex].pBuffer);
            impl->m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
            impl->m_sInBufList.nAllocBySelfFlags &= ~(1<<nIndex);
        }

        impl->m_sInBufList.nAllocSize--;
        if (impl->m_sInBufList.nAllocSize == 0)
        {
            pPortDef->bPopulated = OMX_FALSE;
        }

        pthread_mutex_unlock(&impl->m_inBufMutex);
    }
    else if (nPortIndex == impl->m_sOutPortDefType.nPortIndex)
    {
        pPortDef = &impl->m_sOutPortDefType;

        pthread_mutex_lock(&impl->m_outBufMutex);
        logv("vencOutPort: free_buffer");

        for (nIndex = 0; nIndex < impl->m_sOutBufList.nBufArrSize; nIndex++)
        {
            logv("pBufferHdr = %p, &m_sOutBufList.pBufArr[%d] = %p", pBufferHdr,
                 (int)nIndex, &impl->m_sOutBufList.pBufArr[nIndex]);
            if (pBufferHdr == &impl->m_sOutBufList.pBufArr[nIndex])
                break;
            if(nIndex == impl->m_sOutBufList.nBufArrSize - 1)
            {
                impl->m_sOutPortDefType.bEnabled = OMX_FALSE;
                impl->mOutportEnable = OMX_FALSE;
            }
        }

        if (nIndex == impl->m_sOutBufList.nBufArrSize)
        {
            pthread_mutex_unlock(&impl->m_outBufMutex);
            return OMX_ErrorBadParameter;
        }

        if (impl->m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
        {
            free(impl->m_sOutBufList.pBufArr[nIndex].pBuffer);
            impl->m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
            impl->m_sOutBufList.nAllocBySelfFlags &= ~(1<<nIndex);
        }

        impl->m_sOutBufList.nAllocSize--;
        if (impl->m_sOutBufList.nAllocSize == 0)
        {
            pPortDef->bPopulated = OMX_FALSE;
        }

        pthread_mutex_unlock(&impl->m_outBufMutex);
    }
    else
    {
        return OMX_ErrorBadParameter;
    }

    return OMX_ErrorNone;
}
OMX_ERRORTYPE __AwOmxVencEmptyThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                           OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    AwOmxVenc* impl = (AwOmxVenc*) hComponent;
    ThrCmdType eCmdNative   = MAIN_THREAD_CMD_EMPTY_BUF;

    impl->mEmptyBufCnt++;
    if (impl->mEmptyBufCnt >= PRINT_FRAME_CNT)
    {
        logd("vencInPort: empty_this_buffer %d times",PRINT_FRAME_CNT);
        impl->mEmptyBufCnt = 0;
    }

    logv(" vencOutPort: function:%s, line: %d",__FUNCTION__,__LINE__);

    if (hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (!impl->m_sInPortDefType.bEnabled)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBufferHdr->nInputPortIndex != 0x0  || pBufferHdr->nOutputPortIndex != OMX_NOPORT)
    {
        return OMX_ErrorBadPortIndex;
    }

    if (impl->m_state != OMX_StateExecuting && impl->m_state != OMX_StatePause)
    {
        return OMX_ErrorIncorrectStateOperation;
    }
	#ifdef SAVE_PICTURE
    logv("empty this input buffer, pInBufHd:%p, buf_size=%ld",
        pBufferHdr, pBufferHdr->nFilledLen);
    impl->mDebugInfo.nEmptyBitCount++;
    if(impl->bSaveInputDataFlag && impl->mDebugInfo.nEmptyBitCount > 1 * 30)
    {
        fwrite(pBufferHdr->pBuffer + pBufferHdr->nOffset, 1,  pBufferHdr->nFilledLen, impl->mInFile);
        fsync(fileno(impl->mInFile));
        if(impl->mDebugInfo.nEmptyBitCount > 1000 * 30)
            impl->mDebugInfo.nEmptyBitCount = 1 * 30;
    }
	#endif

    post_event(impl, eCmdNative, MAIN_THREAD_CMD_EMPTY_BUF, (OMX_PTR)pBufferHdr);

    return OMX_ErrorNone;
}

OMX_ERRORTYPE __AwOmxVencFillThisBuffer(OMX_IN OMX_HANDLETYPE hComponent,
                                          OMX_IN OMX_BUFFERHEADERTYPE* pBufferHdr)
{
    AwOmxVenc* impl = (AwOmxVenc*) hComponent;
    ThrCmdType eCmdNative = MAIN_THREAD_CMD_FILL_BUF;

    impl->mFillBufCnt++;
    if (impl->mFillBufCnt >= PRINT_FRAME_CNT)
    {
        logd("vencOutPort: fill_this_buffer %d times",PRINT_FRAME_CNT);
        impl->mFillBufCnt = 0;
    }

    logv("vencOutPort: function:%s, line: %d",__FUNCTION__,__LINE__);

    if (hComponent == NULL || pBufferHdr == NULL)
    {
        return OMX_ErrorBadParameter;
    }

    if (!impl->m_sOutPortDefType.bEnabled)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    if (pBufferHdr->nOutputPortIndex != 0x1 || pBufferHdr->nInputPortIndex != OMX_NOPORT)
    {
        return OMX_ErrorBadPortIndex;
    }

    if (impl->m_state != OMX_StateExecuting && impl->m_state != OMX_StatePause)
    {
        return OMX_ErrorIncorrectStateOperation;
    }

    post_event(impl, eCmdNative, MAIN_THREAD_CMD_FILL_BUF, (OMX_PTR)pBufferHdr);

    return OMX_ErrorNone;
}




static OMX_ERRORTYPE __AwOmxVencSetCallbacks(OMX_IN  OMX_HANDLETYPE hComponent,
            OMX_IN  OMX_CALLBACKTYPE* pCallbacks,
            OMX_IN  OMX_PTR pAppData)
{
    AwOmxVenc *impl = (AwOmxVenc*)hComponent;
    logd("===== vdec set callbacks***************");

    if (hComponent == NULL || pCallbacks == NULL || pAppData == NULL)
    {
        return OMX_ErrorBadParameter;
    }
    memcpy(&impl->m_Callbacks, pCallbacks, sizeof(OMX_CALLBACKTYPE));
    impl->m_pAppData = pAppData;

    return OMX_ErrorNone;
}

static OMX_ERRORTYPE __AwOmxVencDeInit(OMX_IN OMX_HANDLETYPE hComponent)
{
    AwOmxVenc *impl = (AwOmxVenc*)hComponent;

    OMX_ERRORTYPE eError = OMX_ErrorNone;
    ThrCmdType    eCmdNative   = MAIN_THREAD_CMD_STOP;
    OMX_S32       nIndex = 0;

    logd("COMPONENT_DEINIT ");

    CEDARC_UNUSE(hComponent);

    // In case the client crashes, check for nAllocSize parameter.
    // If this is greater than zero, there are elements in the list that are not free'd.
    // In that case, free the elements.
    if (impl->m_sInBufList.nAllocSize > 0)
    {
        for (nIndex=0; nIndex<impl->m_sInBufList.nBufArrSize; nIndex++)
        {
            if (impl->m_sInBufList.pBufArr[nIndex].pBuffer != NULL)
            {
                if (impl->m_sInBufList.nAllocBySelfFlags & (1<<nIndex))
                {
                    free(impl->m_sInBufList.pBufArr[nIndex].pBuffer);
                    impl->m_sInBufList.pBufArr[nIndex].pBuffer = NULL;
                }
            }
        }

        if (impl->m_sInBufList.pBufArr != NULL)
        {
            free(impl->m_sInBufList.pBufArr);
        }

        if (impl->m_sInBufList.pBufHdrList != NULL)
        {
            free(impl->m_sInBufList.pBufHdrList);
        }

        memset(&impl->m_sInBufList, 0, sizeof(struct _BufferList));
        impl->m_sInBufList.nBufArrSize = impl->m_sInPortDefType.nBufferCountActual;
    }

    if (impl->m_sOutBufList.nAllocSize > 0)
    {
        for (nIndex=0; nIndex<impl->m_sOutBufList.nBufArrSize; nIndex++)
        {
            if (impl->m_sOutBufList.pBufArr[nIndex].pBuffer != NULL)
            {
                if (impl->m_sOutBufList.nAllocBySelfFlags & (1<<nIndex))
                {
                    free(impl->m_sOutBufList.pBufArr[nIndex].pBuffer);
                    impl->m_sOutBufList.pBufArr[nIndex].pBuffer = NULL;
                }
            }
        }

        if (impl->m_sOutBufList.pBufArr != NULL)
        {
            free(impl->m_sOutBufList.pBufArr);
        }

        if (impl->m_sOutBufList.pBufHdrList != NULL)
        {
            free(impl->m_sOutBufList.pBufHdrList);
        }

        memset(&impl->m_sOutBufList, 0, sizeof(struct _BufferList));
        impl->m_sOutBufList.nBufArrSize = impl->m_sOutPortDefType.nBufferCountActual;
    }

    post_event(impl, eCmdNative, eCmdNative, NULL);

    // Wait for thread to exit so we can get the status into "error"
    pthread_join(impl->m_thread_id, (void**)&eError);
    pthread_join(impl->m_venc_thread_id, (void**)&eError);

    if(impl->bSaveStreamFlag)
    {
        if (impl->m_outFile)
        {
            fclose(impl->m_outFile);
            impl->m_outFile = NULL;
        }
    }
    if(impl->bSaveInputDataFlag)
    {
        if (impl->mInFile)
        {
            fclose(impl->mInFile);
            impl->mInFile = NULL;
        }
    }

    return eError;

}

OMX_ERRORTYPE __AwOmxVencUseEGImage(OMX_IN OMX_HANDLETYPE               pHComp,
                                          OMX_INOUT OMX_BUFFERHEADERTYPE** ppBufferHdr,
                                          OMX_IN OMX_U32                   uPort,
                                          OMX_IN OMX_PTR                   pAppData,
                                          OMX_IN void*                     pEglImage)
{
    logv("Error : use_EGL_image:  Not Implemented \n");

    CEDARC_UNUSE(pHComp);
    CEDARC_UNUSE(ppBufferHdr);
    CEDARC_UNUSE(uPort);
    CEDARC_UNUSE(pAppData);
    CEDARC_UNUSE(pEglImage);

    return OMX_ErrorNotImplemented;
}

OMX_ERRORTYPE __AwOmxVencRoleEnum(OMX_IN  OMX_HANDLETYPE pHComp,
                                            OMX_OUT OMX_U8*        pRole,
                                            OMX_IN  OMX_U32        uIndex)
{
    logd(" COMPONENT_ROLE_ENUM");
    AwOmxVenc* impl = (AwOmxVenc*) pHComp;
    OMX_ERRORTYPE eRet = OMX_ErrorNone;

    CEDARC_UNUSE(pHComp);

    if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.avc", OMX_MAX_STRINGNAME_SIZE))
    {
        if ((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "video_encoder.avc", OMX_MAX_STRINGNAME_SIZE);
            logv("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.mjpeg", OMX_MAX_STRINGNAME_SIZE))
    {
        if ((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "video_encoder.mjpeg", OMX_MAX_STRINGNAME_SIZE);
            logv("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else if (!strncmp((char*)impl->m_cName, "OMX.allwinner.video.encoder.hevc", OMX_MAX_STRINGNAME_SIZE))
    {
        if ((0 == uIndex) && pRole)
        {
            strncpy((char *)pRole, "video_encoder.hevc", OMX_MAX_STRINGNAME_SIZE);
            logv("component_role_enum: pRole %s\n", pRole);
        }
        else
        {
            eRet = OMX_ErrorNoMore;
        }
    }
    else
    {
        logd("\nERROR:Querying pRole on Unknown Component\n");
        eRet = OMX_ErrorInvalidComponentName;
    }

    return eRet;
}

static OMX_COMPONENTTYPE* __AwOmxVencComponentCreate()
{
    logd(" COMPONENT_CREATE");
    AwOmxVenc *impl = (AwOmxVenc*)malloc(sizeof(AwOmxVenc));
    memset(impl, 0x00, sizeof(AwOmxVenc));

    impl->base.destroy = &__AwOmxVencDestroy;
    impl->base.init = &__AwOmxVencInit;

    impl->base.mOmxComp.nSize = sizeof(OMX_COMPONENTTYPE);
    impl->base.mOmxComp.nVersion.nVersion = OMX_SPEC_VERSION;
    impl->base.mOmxComp.pComponentPrivate = impl;

    impl->base.mOmxComp.AllocateBuffer = &__AwOmxVencAllocateBuffer;
    impl->base.mOmxComp.ComponentDeInit = &__AwOmxVencDeInit;
    impl->base.mOmxComp.ComponentRoleEnum = &__AwOmxVencRoleEnum;
    impl->base.mOmxComp.ComponentTunnelRequest = &__AwOmxVencTunnelRequest;
    impl->base.mOmxComp.EmptyThisBuffer = &__AwOmxVencEmptyThisBuffer;
    impl->base.mOmxComp.FillThisBuffer = &__AwOmxVencFillThisBuffer;
    impl->base.mOmxComp.FreeBuffer = &__AwOmxVencFreeBuffer;
    impl->base.mOmxComp.GetComponentVersion = &__AwOmxVencGetComponentVersion;
    impl->base.mOmxComp.GetConfig = &__AwOmxVencGetConfig;
    impl->base.mOmxComp.GetExtensionIndex = &__AwOmxVencGetExtensionIndex;
    impl->base.mOmxComp.GetParameter = &__AwOmxVencGetParameter;
    impl->base.mOmxComp.GetState = &__AwOmxVencGetState;
    impl->base.mOmxComp.SendCommand = &__AwOmxVencSendCommand;
    impl->base.mOmxComp.SetCallbacks = &__AwOmxVencSetCallbacks;
    impl->base.mOmxComp.SetConfig = &__AwOmxVencSetConfig;
    impl->base.mOmxComp.SetParameter = &__AwOmxVencSetParameter;
    impl->base.mOmxComp.UseBuffer = &__AwOmxVencUseBuffer;
    impl->base.mOmxComp.UseEGLImage = &__AwOmxVencUseEGImage;

    return &impl->base.mOmxComp;
}

void* AwOmxComponentCreate()
{
    return __AwOmxVencComponentCreate();
}

static inline int putOneBufferToList(AwOmxVenc* impl, BufferList* pBufferList,
                            OMX_IN uintptr_t pCmdData)
{
    if(!impl || !pBufferList || !pCmdData)
        return -1;
    OMX_BUFFERHEADERTYPE*   pOutBufHdr = NULL;
    BufferList* pRealBufferList;
    pthread_mutex_t mutex;
    if(pBufferList == &impl->m_sInBufList)
    {
        pRealBufferList = &impl->m_sInBufList;
        mutex = impl->m_inBufMutex;
    }
    else if(pBufferList == &impl->m_sOutBufList)
    {
        pRealBufferList = &impl->m_sOutBufList;
        mutex = impl->m_outBufMutex;
    }
    else
    {
        loge("neither output or input buffer list.");
        return -1;
    }

    pthread_mutex_lock(&mutex);
    if (pRealBufferList->nSizeOfList <= pRealBufferList->nAllocSize)
    {
        pRealBufferList->nSizeOfList++;
        pOutBufHdr = pRealBufferList->pBufHdrList[pRealBufferList->nWritePos++]=
            ((OMX_BUFFERHEADERTYPE*) pCmdData);

        if (pRealBufferList->nWritePos >= (int)pRealBufferList->nAllocSize)
        {
            pRealBufferList->nWritePos = 0;
        }
    }
    logv("###[Empty or FillBuf ] BufHdr=%p", pOutBufHdr);
    pthread_mutex_unlock(&mutex);
    return 0;
}

static inline int getOneBufferFromList(AwOmxVenc* impl, BufferList* pBufferList,
                                    OMX_OUT OMX_BUFFERHEADERTYPE** ppBufHdr)
{
    if(impl == NULL || pBufferList == NULL || ppBufHdr == NULL)
        return -1;
    BufferList* pRealBufferList;
    pthread_mutex_t mutex;
    if(pBufferList == &impl->m_sInBufList)
    {
        pRealBufferList = &impl->m_sInBufList;
        mutex = impl->m_inBufMutex;
    }
    else if(pBufferList == &impl->m_sOutBufList)
    {
        pRealBufferList = &impl->m_sOutBufList;
        mutex = impl->m_outBufMutex;
    }
    else
    {
        loge("neither output or input buffer list.");
        return -1;
    }

    pthread_mutex_lock(&mutex);

    if (pRealBufferList->nSizeOfList > 0)
    {
        pRealBufferList->nSizeOfList--;
        *ppBufHdr = pRealBufferList->pBufHdrList[pRealBufferList->nReadPos++];
        if (pRealBufferList->nReadPos >= (int)pRealBufferList->nAllocSize)
            pRealBufferList->nReadPos = 0;
    }
    else
    {
        *ppBufHdr = NULL;
    }

    pthread_mutex_unlock(&mutex);
    return 0;
}

static inline int flushBuffers(AwOmxVenc* impl, BufferList* pBufferList)
{
    if(impl == NULL || pBufferList == NULL)
        return -1;
    BufferList* pRealBufferList;
    pthread_mutex_t mutex;
    if(pBufferList == &impl->m_sInBufList)
    {
      pRealBufferList = &impl->m_sInBufList;
      mutex = impl->m_inBufMutex;
    }
    else if(pBufferList == &impl->m_sOutBufList)
    {
      pRealBufferList = &impl->m_sOutBufList;
      mutex = impl->m_outBufMutex;
    }
    else
    {
        loge("neither output or input buffer list.");
        return -1;
    }

    pthread_mutex_lock(&mutex);

    while (pRealBufferList->nSizeOfList > 0)
    {
        OMX_S32 pos = pRealBufferList->nReadPos++;
        pRealBufferList->nSizeOfList--;
        if(pBufferList == &impl->m_sInBufList)
            impl->m_Callbacks.EmptyBufferDone(
                &impl->base.mOmxComp, impl->m_pAppData,
                pRealBufferList->pBufHdrList[pos]);
        else if(pBufferList == &impl->m_sOutBufList)
            impl->m_Callbacks.FillBufferDone(
                &impl->base.mOmxComp, impl->m_pAppData,
                impl->m_sOutBufList.pBufHdrList[pos]);

        if (pRealBufferList->nReadPos >=
            pRealBufferList->nBufArrSize)
        {
            pRealBufferList->nReadPos = 0;
        }
    }

    pthread_mutex_unlock(&mutex);
    return 0;
}

void setSuperFrameCfg(AwOmxVenc* impl)
{
    VencSuperFrameConfig sSuperFrameCfg;

    VideoEncSetParameter(impl->m_encoder, VENC_IndexParamFramerate, &impl->m_framerate);

    switch (impl->mVideoSuperFrame.eSuperFrameMode)
    {
        case OMX_VIDEO_SUPERFRAME_NONE:
            sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_NONE;
            break;

        case OMX_VIDEO_SUPERFRAME_REENCODE:
            sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_REENCODE;
            break;

        case OMX_VIDEO_SUPERFRAME_DISCARD:
            sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_DISCARD;
            break;

        default:
            logd("the omx venc do not support the superFrame mode:%d\n", \
                impl->mVideoSuperFrame.eSuperFrameMode);
            sSuperFrameCfg.eSuperFrameMode = VENC_SUPERFRAME_NONE;
            break;
    }

    if(!impl->mVideoSuperFrame.nMaxIFrameBits && !impl->mVideoSuperFrame.nMaxPFrameBits)
    {
        sSuperFrameCfg.nMaxIFrameBits = impl->m_sOutPortDefType.format.video.nBitrate /
            impl->m_framerate*3;
        sSuperFrameCfg.nMaxPFrameBits = impl->m_sOutPortDefType.format.video.nBitrate /
            impl->m_framerate*2;
        if (impl->m_framerate <= 10)
        {
            sSuperFrameCfg.nMaxIFrameBits = impl->m_sOutPortDefType.format.video.nBitrate /
                impl->m_framerate * 2;
            sSuperFrameCfg.nMaxPFrameBits = impl->m_sOutPortDefType.format.video.nBitrate /
                impl->m_framerate*3/2;
        }
    }
    else
    {
        sSuperFrameCfg.nMaxIFrameBits = impl->mVideoSuperFrame.nMaxIFrameBits;
        sSuperFrameCfg.nMaxPFrameBits = impl->mVideoSuperFrame.nMaxPFrameBits;
    }

    logd("setSuperFrameCfg, impl->m_framerate: %d, bitrate: %d\n",
         (int)impl->m_framerate, (int)impl->m_sOutPortDefType.format.video.nBitrate);
    logd("bitRate/frameRate:%d, nMaxIFrameBits:%d, nMaxPFrameBits:%d\n",\
    (int)impl->m_sOutPortDefType.format.video.nBitrate/(int)impl->m_framerate,\
    sSuperFrameCfg.nMaxIFrameBits, sSuperFrameCfg.nMaxPFrameBits);
    VideoEncSetParameter(impl->m_encoder, VENC_IndexParamSuperFrameConfig, &sSuperFrameCfg);
}

void setSVCSkipCfg(AwOmxVenc* impl)
{
    VencH264SVCSkip SVCSkip;
    unsigned int total_ratio;
    memset(&SVCSkip, 0, sizeof(VencH264SVCSkip));

    logd("setSVCSkipCfg, impl->mVideoSVCSkip.uTemporalSVC: %d, LayerRatio[0-3]: [%d %d %d %d]\n",\
    (int)impl->mVideoSVCSkip.eSvcLayer, (int)impl->mVideoSVCSkip.uLayerRatio[0],\
    (int)impl->mVideoSVCSkip.uLayerRatio[1], (int)impl->mVideoSVCSkip.uLayerRatio[2],\
    (int)impl->mVideoSVCSkip.uLayerRatio[3]);

    SVCSkip.nTemporalSVC = NO_T_SVC;
    SVCSkip.nSkipFrame = NO_SKIP;
    switch(impl->mVideoSVCSkip.eSvcLayer)
    {
        case OMX_VIDEO_NO_SVC:
            SVCSkip.nTemporalSVC = NO_T_SVC;
            SVCSkip.nSkipFrame = NO_SKIP;
            break;
       case OMX_VIDEO_LAYER_2:
            SVCSkip.nTemporalSVC = T_LAYER_2;
            SVCSkip.nSkipFrame = SKIP_2;
            break;
       case OMX_VIDEO_LAYER_3:
            SVCSkip.nTemporalSVC = T_LAYER_3;
            SVCSkip.nSkipFrame = SKIP_4;
            break;
       case OMX_VIDEO_LAYER_4:
            SVCSkip.nTemporalSVC = T_LAYER_4;
            SVCSkip.nSkipFrame = SKIP_8;
            break;
       default:
            SVCSkip.nTemporalSVC = NO_T_SVC;
            SVCSkip.nSkipFrame = NO_SKIP;
            break;
    }
    total_ratio = impl->mVideoSVCSkip.uLayerRatio[0] + impl->mVideoSVCSkip.uLayerRatio[1] +\
    impl->mVideoSVCSkip.uLayerRatio[2] + impl->mVideoSVCSkip.uLayerRatio[3];
    if(total_ratio != 100)
    {
        logd("the set layer ratio sum is %d, not 100, so use the encoder default layer ratio\n",\
        total_ratio);
        SVCSkip.bEnableLayerRatio = 0;
    }
    else
    {
        SVCSkip.bEnableLayerRatio = 1;

        int i=0;
        for(i=0; i<4; i++)
            SVCSkip.nLayerRatio[i] = impl->mVideoSVCSkip.uLayerRatio[i];
    }

    VideoEncSetParameter(impl->m_encoder, VENC_IndexParamH264SVCSkip, &SVCSkip);
}

void init_h264_param(AwOmxVenc* impl)
{
    memset(&impl->m_vencH264Param, 0, sizeof(VencH264Param));

    //* h264 param
    impl->m_vencH264Param.bEntropyCodingCABAC = 1;
    impl->m_vencH264Param.nBitrate = impl->m_sOutPutBitRateType.nTargetBitrate; /* bps */
    impl->m_vencH264Param.nFramerate = impl->m_framerate; /* fps */
    impl->m_vencH264Param.nCodingMode = VENC_FRAME_CODING;
    impl->m_vencH264Param.nMaxKeyInterval = (impl->m_sH264Type.nPFrames + 1);
    if (impl->m_vencH264Param.nMaxKeyInterval <= 0 ||
        impl->m_vencH264Param.nMaxKeyInterval >= 500)
    {
        impl->m_vencH264Param.nMaxKeyInterval = 30;
    }

    //set profile
    switch (impl->m_sH264Type.eProfile)
    {
        case OMX_VIDEO_AVCProfileBaseline:
            impl->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileBaseline;
            break;
        case OMX_VIDEO_AVCProfileMain:
            impl->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileMain;
            break;
        case OMX_VIDEO_AVCProfileHigh:
            impl->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileHigh;
            break;

        default:
            impl->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileBaseline;
            break;
    }

    if (impl->mIsFromVideoeditor)
    {
        logd ("Called from Videoeditor,set VENC_H264ProfileBaseline");
        impl->m_vencH264Param.sProfileLevel.nProfile = VENC_H264ProfileBaseline;
    }

    logd("profile-venc=%d, profile-omx=%d, frame_rate:%d, bit_rate:%d, idr:%d, eColorFormat:%08x\n",
         impl->m_vencH264Param.sProfileLevel.nProfile,
         impl->m_sH264Type.eProfile,impl->m_vencH264Param.nFramerate,
         impl->m_vencH264Param.nBitrate,impl->m_vencH264Param.nMaxKeyInterval,
         impl->m_sInPortFormatType.eColorFormat);

    //set level
    switch (impl->m_sH264Type.eLevel)
    {
        case OMX_VIDEO_AVCLevel1:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level1;
            break;
        }
        case OMX_VIDEO_AVCLevel1b: // do not support this
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level1;
            break;
        }
        case OMX_VIDEO_AVCLevel11:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level11;
            break;
        }
        case OMX_VIDEO_AVCLevel12:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level12;
            break;
        }
        case OMX_VIDEO_AVCLevel2:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level2;
            break;
        }
        case OMX_VIDEO_AVCLevel21:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level21;
            break;
        }
        case OMX_VIDEO_AVCLevel22:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level22;
            break;
        }
        case OMX_VIDEO_AVCLevel3:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level3;
            break;
        }
        case OMX_VIDEO_AVCLevel31:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level31;
            break;
        }
        case OMX_VIDEO_AVCLevel32:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level32;
            break;
        }
        case OMX_VIDEO_AVCLevel4:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level4;
            break;
        }
        case OMX_VIDEO_AVCLevel41:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level41;
            break;
        }
        case OMX_VIDEO_AVCLevel42:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level42;
            break;
        }
        case OMX_VIDEO_AVCLevel5:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level5;
            break;
        }
        case OMX_VIDEO_AVCLevel51:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level51;
            break;
        }
        default:
        {
            impl->m_vencH264Param.sProfileLevel.nLevel = VENC_H264Level41;
            break;
        }
    }

    impl->m_vencH264Param.sQPRange.nMinqp = 6;
    impl->m_vencH264Param.sQPRange.nMaxqp = 45;

    /*For VBR setting.*/
    //CBR_VFR VBR_VFR are both not supported.
    //impl->m_sOutPutBitRateType.eControlRate = OMX_Video_ControlRateVariable;
    if(impl->m_sOutPutBitRateType.eControlRate == OMX_Video_ControlRateVariable)
    {
        impl->mVideoVBR.bEnable = OMX_TRUE;
        impl->mVideoVBR.uQpMin = 20;//kay: to select a proper vaue??
        impl->mVideoVBR.uQpMax = 45;
        impl->mVideoVBR.uBitRate = impl->m_sOutPutBitRateType.nTargetBitrate;
    }

    if (impl->mVideoVBR.bEnable)
    {
        impl->m_vencH264Param.sQPRange.nMinqp = impl->mVideoVBR.uQpMin;
        impl->m_vencH264Param.sQPRange.nMaxqp = impl->mVideoVBR.uQpMax;
        impl->m_vencH264Param.nBitrate = impl->mVideoVBR.uBitRate;

        if(impl->mVideoVBR.sMdParam.bMotionDetectEnable)
        {
            OMX_VIDEO_PARAMS_MD sMD_param;
            sMD_param.bMotionDetectEnable = impl->mVideoVBR.sMdParam.bMotionDetectEnable;
            sMD_param.nMotionDetectRatio = impl->mVideoVBR.sMdParam.nMotionDetectRatio;
            sMD_param.nStaticDetectRatio = impl->mVideoVBR.sMdParam.nStaticDetectRatio;
            sMD_param.nMaxNumStaticFrame = impl->mVideoVBR.sMdParam.nMaxNumStaticFrame;
            sMD_param.nStaticBitsRatio = impl->mVideoVBR.sMdParam.nStaticBitsRatio;
            sMD_param.dMV64x64Ratio = impl->mVideoVBR.sMdParam.dMV64x64Ratio;
            sMD_param.sMVXTh = impl->mVideoVBR.sMdParam.sMVXTh;
            sMD_param.sMVYTh = impl->mVideoVBR.sMdParam.sMVYTh;

            VideoEncSetParameter(impl->m_encoder,\
            VENC_IndexParamMotionDetectEnable, &sMD_param);
        }
        logd("Enable VBR. QpMin: %ld. QpMax: %ld. Bitrate: %ld.",
            impl->mVideoVBR.uQpMin, impl->mVideoVBR.uQpMax, impl->mVideoVBR.uBitRate);
    }

    VideoEncSetParameter(impl->m_encoder, VENC_IndexParamH264Param, &impl->m_vencH264Param);

    if(impl->m_fixQp.bEnable == OMX_TRUE)
    {
        VideoEncSetParameter(impl->m_encoder, VENC_IndexParamH264FixQP, &impl->m_fixQp);
    }

    if(impl->m_enableAvcIntraPeriod == OMX_TRUE)
    {
        logd("IDRPeriod: %ld. PFrames: %ld(not support, would be ignore!!!!).",
            impl->m_avcIntraPeriod.nIDRPeriod, impl->m_avcIntraPeriod.nPFrames);
        VideoEncSetParameter(impl->m_encoder, VENC_IndexParamMaxKeyInterval,
                            &impl->m_avcIntraPeriod.nIDRPeriod);
        impl->m_vencH264Param.nMaxKeyInterval = impl->m_avcIntraPeriod.nIDRPeriod;
    }

    if (impl->mVideoSuperFrame.bEnable)
    {
        setSuperFrameCfg(impl);
        logd("use setSuperFrameCfg");
    }

    if (impl->mVideoSVCSkip.bEnable)
    {
        setSVCSkipCfg(impl);
    }

#if 0 //* do not use this functiontion right now
    if (impl->m_vencCyclicIntraRefresh.bEnable)
    {
        VideoEncSetParameter(impl->m_encoder, VENC_IndexParamH264CyclicIntraRefresh,
                             &impl->m_vencCyclicIntraRefresh);
    }
#endif
}


void init_h265_param(AwOmxVenc* impl)
{
    memset(&impl->m_vencH265Param, 0, sizeof(VencH264Param));

    //* h265 param
    impl->m_vencH265Param.nBitrate = impl->m_sOutPutBitRateType.nTargetBitrate; /* bps */
    impl->m_vencH265Param.nFramerate = impl->m_framerate; /* fps */
    impl->m_vencH265Param.idr_period = impl->m_sH265Type.nKeyFrameInterval;
    if (impl->m_vencH265Param.idr_period <= 0)
        impl->m_vencH265Param.idr_period = 30;
    else if(impl->m_vencH265Param.idr_period > 128)
        impl->m_vencH265Param.idr_period = 128;

    impl->m_vencH265Param.nGopSize = 30;
    impl->m_vencH265Param.nIntraPeriod = impl->m_vencH265Param.idr_period;

    //set profile
    switch (impl->m_sH265Type.eProfile)
    {
        case OMX_VIDEO_HEVCProfileMain:
            impl->m_vencH265Param.sProfileLevel.nProfile = VENC_H265ProfileMain;
            break;
        default:
            impl->m_vencH265Param.sProfileLevel.nProfile = VENC_H265ProfileMain;
            break;
    }

    logd("profile-venc=%d, profile-omx=%d, frame_rate:%d, bit_rate:%d, idr:%d, eColorFormat:%08x\n",
         impl->m_vencH265Param.sProfileLevel.nProfile,
         impl->m_sH265Type.eProfile,impl->m_vencH265Param.nFramerate,
         impl->m_vencH265Param.nBitrate,impl->m_vencH265Param.idr_period,
         impl->m_sInPortFormatType.eColorFormat);

    //set level
    switch (impl->m_sH265Type.eLevel)
    {
        case OMX_VIDEO_HEVCMainTierLevel1:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level1;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel2:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level2;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel21:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level21;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel3:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level3;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel31:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level31;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel41:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level41;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel5:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level5;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel51:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level51;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel52:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level52;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel6:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level6;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel61:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level61;
            break;
        }
        case OMX_VIDEO_HEVCMainTierLevel62:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level62;
            break;
        }
        default:
        {
            impl->m_vencH265Param.sProfileLevel.nLevel = VENC_H265Level51;
            break;
        }
    }

    impl->m_vencH265Param.sQPRange.nMinqp = 6;
    impl->m_vencH265Param.sQPRange.nMaxqp = 45;
    impl->m_vencH265Param.nQPInit = 30;

    VideoEncSetParameter(impl->m_encoder, VENC_IndexParamH265Param, &impl->m_vencH265Param);

    if (impl->mVideoSuperFrame.bEnable)
    {
        setSuperFrameCfg(impl);
        logd("use setSuperFrameCfg");
    }

#if 0 //* do not use this functiontion right now
    if (impl->m_vencCyclicIntraRefresh.bEnable)
    {
        VideoEncSetParameter(impl->m_encoder, VENC_IndexParamH264CyclicIntraRefresh,
                             &impl->m_vencCyclicIntraRefresh);
    }
#endif
}

int openVencDriver(AwOmxVenc* impl)
{
    VencBaseConfig sBaseConfig;
    int result;
    memset(&sBaseConfig, 0, sizeof(VencBaseConfig));
    int size_y;
    int size_c;

    impl->m_encoder = VideoEncCreate(impl->m_vencCodecType);
    if (impl->m_encoder == NULL)
    {
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventError, OMX_ErrorHardware, 0 , NULL);
        logw("VideoEncCreate fail");
        return -1;
    }

    if (impl->m_vencCodecType == VENC_CODEC_H264)
	{
        init_h264_param(impl);
	}
    else if(impl->m_vencCodecType == VENC_CODEC_H265)
	{
        init_h265_param(impl);
	}
    else if (impl->m_vencCodecType == VENC_CODEC_JPEG)
	{
        if (impl->m_sOutPutBitRateType.nTargetBitrate != DEFAULT_BITRATE)
        {
            int jpeg_mode=1;
            VideoEncSetParameter(impl->m_encoder, VENC_IndexParamJpegEncMode, &jpeg_mode);
            VideoEncSetParameter(impl->m_encoder, VENC_IndexParamBitrate, &impl->m_sOutPutBitRateType.nTargetBitrate);
            VideoEncSetParameter(impl->m_encoder, VENC_IndexParamFramerate, &impl->m_framerate);
            VencBitRateRange bitRateRange;
            bitRateRange.bitRateMax = impl->m_sOutPutBitRateType.nTargetBitrate + 1*1024*1024;
            bitRateRange.bitRateMin = impl->m_sOutPutBitRateType.nTargetBitrate - 1*1024*1024;
            VideoEncSetParameter(impl->m_encoder, VENC_IndexParamSetBitRateRange, &bitRateRange);
        }
    }

    determineVencColorFormat(impl);

    if (!impl->m_useMetaDataInBuffers && !impl->m_useAndroidBuffer)
    {
        impl->m_useAllocInputBuffer = OMX_TRUE;
    }
    else
    {
        #if 0
        //* just work on chip-a80 and chip-a23, remove now
        if (impl->m_sInPortFormatType.eColorFormat ==
            OMX_COLOR_FormatAndroidOpaque && impl->mIsFromCts)
        {
            VENC_COLOR_SPACE colorSpace = VENC_BT601;
            if (VideoEncSetParameter(impl->m_encoder, VENC_IndexParamRgb2Yuv, &colorSpace) != 0)
            {
                impl->m_useAllocInputBuffer = OMX_TRUE;
                impl->m_vencColorFormat = VENC_PIXEL_YUV420SP;
                //use ImgRGBA2YUV420SP_neon convert argb to yuv420sp
            }
            else
            {
                logd("use bt601 ok");
                impl->m_useAllocInputBuffer = OMX_FALSE;
            }
        }
        else
        #endif
        {
            impl->m_useAllocInputBuffer = OMX_FALSE;
        }
    }

    logd("omx_venc base_config info: src_wxh:%dx%d, dis_wxh:%dx%d, stride:%d\n",
         (int)impl->m_sInPortDefType.format.video.nFrameWidth,
         (int)impl->m_sInPortDefType.format.video.nFrameHeight,
         (int)impl->m_sOutPortDefType.format.video.nFrameWidth,
         (int)impl->m_sOutPortDefType.format.video.nFrameHeight,
         (int)impl->m_sInPortDefType.format.video.nStride);
    //tina platform requires this setting!!
	if(impl->m_sOutPortDefType.format.video.nFrameWidth == 176 && impl->m_sOutPortDefType.format.video.nFrameHeight == 144)
	{
		impl->m_sOutPortDefType.format.video.nFrameWidth = impl->m_sInPortDefType.format.video.nFrameWidth;
		impl->m_sOutPortDefType.format.video.nFrameHeight = impl->m_sInPortDefType.format.video.nFrameHeight;
	}

    sBaseConfig.nInputWidth= impl->m_sInPortDefType.format.video.nFrameWidth;
    sBaseConfig.nInputHeight = impl->m_sInPortDefType.format.video.nFrameHeight;
    sBaseConfig.nStride = impl->m_sInPortDefType.format.video.nStride;

    if (impl->mVideoExtParams.bEnableScaling)
    {
       sBaseConfig.nDstWidth = impl->mVideoExtParams.ui16ScaledWidth;
       sBaseConfig.nDstHeight = impl->mVideoExtParams.ui16ScaledHeight;
    }else
    {
       sBaseConfig.nDstWidth = impl->m_sOutPortDefType.format.video.nFrameWidth;
       sBaseConfig.nDstHeight = impl->m_sOutPortDefType.format.video.nFrameHeight;
    }

    sBaseConfig.eInputFormat = impl->m_vencColorFormat;
    sBaseConfig.memops = MemAdapterGetOpsS();

    determineVencBufStrideIfNecessary(impl, &sBaseConfig);

    if(impl->m_usePSkip)
        VideoEncSetParameter(impl->m_encoder, VENC_IndexParamSetPSkip, &impl->m_usePSkip);

    if(impl->m_avcNaluFormat.eNaluFormat== OMX_NaluFormatFourByteInterleaveLength)
        sBaseConfig.bEncH264Nalu = OMX_TRUE;
    logd("AVC 4 byte interleave nalu %s.", sBaseConfig.bEncH264Nalu ? "Enable":"Disable");

    result = VideoEncInit(impl->m_encoder, &sBaseConfig);

    if (result != 0)
    {
        VideoEncDestroy(impl->m_encoder);
        impl->m_encoder = NULL;
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventError, OMX_ErrorHardware, 0 , NULL);
        logw("VideoEncInit, failed, result: %d\n", result);
        return -1;
    }

    if (impl->m_vencCodecType == VENC_CODEC_H264)
    {
        result = VideoEncGetParameter(impl->m_encoder,
                                        VENC_IndexParamH264SPSPPS, &impl->m_headdata);
    }
    else if (impl->m_vencCodecType == VENC_CODEC_H265)
    {
        result = VideoEncGetParameter(impl->m_encoder,
                                        VENC_IndexParamH265Header, &impl->m_headdata);
    }
    if(result < 0)
    {
        loge("get sps info error\n");
        VideoEncUnInit(impl->m_encoder);
        VideoEncDestroy(impl->m_encoder);
        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                        OMX_EventError, OMX_ErrorHardware, 0 , NULL);
        return -1;
    }

    size_y = sBaseConfig.nStride*((sBaseConfig.nInputHeight+15)&~15);
    size_c = size_y>>1;

    size_y = (size_y + 127)&~127;
    size_c = (size_c + 127)&~127;

    size_y += 16*1024;
    size_c += 16*1024;


    impl->m_vencAllocBufferParam.nBufferNum = impl->m_sOutPortDefType.nBufferCountActual;
    impl->m_vencAllocBufferParam.nSizeY = size_y;
    impl->m_vencAllocBufferParam.nSizeC = size_c;

    impl->mFirstInputFrame = OMX_TRUE;

    if (impl->m_useAllocInputBuffer)
    {
        result = AllocInputBuffer(impl->m_encoder, &impl->m_vencAllocBufferParam);
        if (result !=0 )
        {
            VideoEncUnInit(impl->m_encoder);
            VideoEncDestroy(impl->m_encoder);
            loge("AllocInputBuffer,error");
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventError, OMX_ErrorHardware, 0 , NULL);
            return -1;
        }
    }

    if(impl->bShowFrameSizeFlag){
        struct timeval cur_time1;
        gettimeofday(&cur_time1, NULL);
        impl->mTimeStart = cur_time1.tv_sec * 1000000LL + cur_time1.tv_usec;
    }
    return 0;
}

void closeVencDriver(AwOmxVenc* impl)
{
    if (impl->m_useAllocInputBuffer)
    {
        // ReleaseAllocInputBuffer(impl->m_encoder);
        impl->m_useAllocInputBuffer = OMX_FALSE;
    }

    VideoEncUnInit(impl->m_encoder);
    VideoEncDestroy(impl->m_encoder);
    impl->m_encoder = NULL;
}

static inline int processThreadCommand(AwOmxVenc* impl, CdcMessage* pMsg)
{
    OMX_U32                 nTimeout;
    OMX_MARKTYPE*           pMarkBuf    = NULL;
    int cmd = pMsg->messageId;
    uintptr_t  pCmdData = pMsg->params[0];
    logv("processThreadCommand cmd = %d", cmd);

    // State transition command
    if (cmd == MAIN_THREAD_CMD_SET_STATE)
    {
        logd("x set state command, cmd = %s, pCmdData = %s.", strThrCmd[(int)cmd], strStateType[(int)pCmdData]);
        if (impl->m_state == (OMX_STATETYPE)(pCmdData))
        {
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventError, OMX_ErrorSameState, 0 , NULL);
        }
        else
        {
            // transitions/callbacks made based on state transition table
            // pCmdData contains the target state
            switch ((OMX_STATETYPE)(pCmdData))
            {
                 case OMX_StateInvalid:
                 {
                     impl->m_state = OMX_StateInvalid;
                     impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                     OMX_EventError, OMX_ErrorInvalidState,
                                                     0 , NULL);
                     impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                     OMX_EventCmdComplete,
                                                     OMX_CommandStateSet,
                                                     impl->m_state, NULL);
                    break;
                 }

                case OMX_StateLoaded:
                {
                    if (impl->m_state == OMX_StateIdle ||
                        impl->m_state == OMX_StateWaitForResources)
                    {
                        nTimeout = 0x0;
                        while (1)
                        {
                            // Transition happens only when the ports are unpopulated
                            if (!impl->m_sInPortDefType.bPopulated &&
                                !impl->m_sOutPortDefType.bPopulated)
                            {
                                impl->m_state = OMX_StateLoaded;
                                impl->m_Callbacks.EventHandler(
                                    &impl->base.mOmxComp, impl->m_pAppData,
                                    OMX_EventCmdComplete,  OMX_CommandStateSet,
                                    impl->m_state, NULL);

                                break;
                             }
                            else if (nTimeout++ > OMX_MAX_TIMEOUTS)
                            {
                                impl->m_Callbacks.EventHandler(
                                    &impl->base.mOmxComp, impl->m_pAppData,
                                    OMX_EventError, OMX_ErrorInsufficientResources,
                                    0 , NULL);
                                logw("Transition to loaded failed\n");
                                break;
                            }
                            usleep(OMX_TIMEOUT*1000);
                        }
                    }
                    else
                    {
                        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp,
                                                        impl->m_pAppData,
                                                        OMX_EventError,
                                                        OMX_ErrorIncorrectStateTransition,
                                                        0 , NULL);
                    }
                    break;
                }

                case OMX_StateIdle:
                {
                    if (impl->m_state == OMX_StateInvalid)
                        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                        OMX_EventError,
                                                        OMX_ErrorIncorrectStateTransition,
                                                        0 , NULL);
                    else
                    {
                        // Return buffers if currently in pause and executing
                        if (impl->m_state == OMX_StatePause ||
                            impl->m_state == OMX_StateExecuting)
                        {
                            // venc should in idle state before stop
                            post_message_to_venc_and_wait(impl, OMX_Venc_Cmd_Enc_Idle);

                            if (impl->m_useAllocInputBuffer)
                            {
                                if (impl->mInputBufferStep == OMX_VENC_STEP_GET_ALLOCBUFFER)
                                {
                                    impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp,
                                                                       impl->m_pAppData,
                                                                       impl->mInBufHdr);
                                }
                            }
                            else
                            {
                                if (impl->mInputBufferStep == OMX_VENC_STEP_ADD_BUFFER_TO_ENC)
                                {
                                    impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp,
                                                                       impl->m_pAppData,
                                                                       impl->mInBufHdr);
                                }
                                //* check used buffer
                                while (0 == AlreadyUsedInputBuffer(impl->m_encoder,
                                                                   &impl->m_sInputBuffer_return))
                                {
                                    impl->mInBufHdr =
                                        (OMX_BUFFERHEADERTYPE*)impl->m_sInputBuffer_return.nID;
                                    impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp,
                                                                       impl->m_pAppData,
                                                                       impl->mInBufHdr);
                                }
                            }

                            flushBuffers(impl, &impl->m_sInBufList);
                            flushBuffers(impl, &impl->m_sOutBufList);
                            post_message_to_venc_and_wait(impl, OMX_Venc_Cmd_Close);
                        }
                        else
                        {
                            //* init encoder
                            //post_message_to_venc_and_wait(impl, OMX_Venc_Cmd_Open);
                        }

                        nTimeout = 0x0;
                        while (1)
                        {
                            logv("m_sInPortDefType.bPopulated:%d, "\
                                 "m_sOutPortDefType.bPopulated: %d",
                                 impl->m_sInPortDefType.bPopulated,
                                 impl->m_sOutPortDefType.bPopulated);
                            // Ports have to be populated before transition completes
                            if ((!impl->m_sInPortDefType.bEnabled &&
                                !impl->m_sOutPortDefType.bEnabled) ||
                                (impl->m_sInPortDefType.bPopulated &&
                                impl->m_sOutPortDefType.bPopulated))
                            {
                                impl->m_state = OMX_StateIdle;
                                impl->m_Callbacks.EventHandler(
                                    &impl->base.mOmxComp, impl->m_pAppData,
                                    OMX_EventCmdComplete, OMX_CommandStateSet,
                                    impl->m_state, NULL);

                                 break;
                            }
                            else if (nTimeout++ > OMX_MAX_TIMEOUTS)
                            {
                                impl->m_Callbacks.EventHandler(
                                    &impl->base.mOmxComp, impl->m_pAppData,
                                    OMX_EventError, OMX_ErrorInsufficientResources,
                                    0 , NULL);
                                logw("Idle transition failed\n");
                                break;
                            }
                            usleep(OMX_TIMEOUT*1000);
                        }
                    }
                    break;
                }

                case OMX_StateExecuting:
                {
                    // Transition can only happen from pause or idle state
                    if (impl->m_state == OMX_StateIdle || impl->m_state == OMX_StatePause)
                    {
                        // Return buffers if currently in pause
                        if (impl->m_state == OMX_StatePause)
                        {
                            loge("encode component do not support OMX_StatePause");
                            flushBuffers(impl, &impl->m_sInBufList);
                            flushBuffers(impl, &impl->m_sOutBufList);
                        }

                        impl->m_state = OMX_StateExecuting;
                        impl->m_Callbacks.EventHandler(
                            &impl->base.mOmxComp, impl->m_pAppData,
                            OMX_EventCmdComplete, OMX_CommandStateSet,
                            impl->m_state, NULL);

                        post_message_to_venc_and_wait(impl, OMX_Venc_Cmd_Open);
                    }
                    else
                    {
                        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                        OMX_EventError,
                                                        OMX_ErrorIncorrectStateTransition,
                                                        0 , NULL);
                    }
                    //nInBufEos            = OMX_FALSE; //if need it
                    impl->m_pMarkData    = NULL;
                    impl->m_hMarkTargetComponent = NULL;
                    break;
                }

                case OMX_StatePause:
                {
                    loge("encode component do not support OMX_StatePause");
                    // Transition can only happen from idle or executing state
                    if (impl->m_state == OMX_StateIdle ||
                        impl->m_state == OMX_StateExecuting)
                    {
                        impl->m_state = OMX_StatePause;
                        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandStateSet,
                                                        impl->m_state, NULL);
                    }
                    else
                    {
                        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                        OMX_EventError,
                                                        OMX_ErrorIncorrectStateTransition,
                                                        0 , NULL);
                    }
                    break;
                }

                case OMX_StateWaitForResources:
                {
                    if (impl->m_state == OMX_StateLoaded)
                    {
                        impl->m_state = OMX_StateWaitForResources;
                        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                        OMX_EventCmdComplete,
                                                        OMX_CommandStateSet,
                                                        impl->m_state, NULL);
                    }
                    else
                    {
                        impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                        OMX_EventError,
                                                        OMX_ErrorIncorrectStateTransition,
                                                        0 , NULL);
                    }
                    break;
                }
                default:
                {
                    loge("unknowed OMX_State");
                    impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                    OMX_EventError,
                                                    OMX_ErrorIncorrectStateTransition,
                                                    0 , NULL);
                    break;
                }
            }
        }
    }
    else if (cmd == MAIN_THREAD_CMD_STOP_PORT)
    {
        logd("x stop port command, pCmdData = %d.", (int)pCmdData);
        // Stop Port(s)
        // pCmdData contains the port index to be stopped.
        // It is assumed that 0 is input and 1 is output port for this component
        // The pCmdData value -1 means that both input and output ports will be stopped.
        if (pCmdData == 0x0 || (int)pCmdData == -1)
        {
            // Return all input buffers
            flushBuffers(impl, &impl->m_sInBufList);
            // Disable port
            impl->m_sInPortDefType.bEnabled = OMX_FALSE;
            impl->mInportEnable = OMX_FALSE;
        }

        if (pCmdData == 0x1 || (int)pCmdData == -1)
        {
            // Return all output buffers
            flushBuffers(impl, &impl->m_sOutBufList);
            // Disable port
            impl->m_sOutPortDefType.bEnabled = OMX_FALSE;
            impl->mOutportEnable = OMX_FALSE;
        }

        // Wait for all buffers to be freed
        nTimeout = 0x0;
        while (1)
        {
            if (pCmdData == 0x0 && !impl->m_sInPortDefType.bPopulated)
            {
                // Return cmdcomplete event if input unpopulated
                impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortDisable, 0x0, NULL);
                break;
            }

            if (pCmdData == 0x1 && !impl->m_sOutPortDefType.bPopulated)
            {
                // Return cmdcomplete event if output unpopulated
                impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortDisable, 0x1, NULL);
                break;
            }

            if ((int)pCmdData == -1 && !impl->m_sInPortDefType.bPopulated &&
                !impl->m_sOutPortDefType.bPopulated)
            {
                // Return cmdcomplete event if inout & output unpopulated
                impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortDisable, 0x0, NULL);
                impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortDisable, 0x1, NULL);
                break;
            }

            if (nTimeout++ > OMX_MAX_TIMEOUTS)
            {
                impl->m_Callbacks.EventHandler(
                    &impl->base.mOmxComp, impl->m_pAppData, OMX_EventError,
                    OMX_ErrorPortUnresponsiveDuringDeallocation, 0, NULL);
                break;
            }

            usleep(OMX_TIMEOUT*1000);
        }
    }
    else if (cmd == MAIN_THREAD_CMD_RESTART_PORT)
    {
        logd("x restart port command.");
        // Restart Port(s)
        // pCmdData contains the port index to be restarted.
        // It is assumed that 0 is input and 1 is output port for this component.
        // The pCmdData value -1 means both input and output ports will be restarted.
        if (pCmdData == 0x0 || (int)pCmdData == -1){
            impl->m_sInPortDefType.bEnabled = OMX_TRUE;
            impl->mInportEnable = OMX_TRUE;
            logw("x restart in port command.");
            }

        if (pCmdData == 0x1 || (int)pCmdData == -1){
            impl->m_sOutPortDefType.bEnabled = OMX_TRUE;
            impl->mOutportEnable = OMX_TRUE;
            logw("x restart out port command.");
        }

         // Wait for port to be populated
        nTimeout = 0x0;
        while (1)
        {
            // Return cmdcomplete event if input port populated
            if (pCmdData == 0x0 && (impl->m_state == OMX_StateLoaded ||
                impl->m_sInPortDefType.bPopulated))
            {
                impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortEnable, 0x0, NULL);
                break;
            }
            // Return cmdcomplete event if output port populated
            else if (pCmdData == 0x1 && (impl->m_state == OMX_StateLoaded ||
                     impl->m_sOutPortDefType.bPopulated))
            {
                impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                OMX_EventCmdComplete,
                                                OMX_CommandPortEnable, 0x1, NULL);
                break;
            }
            // Return cmdcomplete event if input and output ports populated
            else if ((int)pCmdData == -1 && (impl->m_state == OMX_StateLoaded ||
                     (impl->m_sInPortDefType.bPopulated &&
                      impl->m_sOutPortDefType.bPopulated)))
            {
                impl->m_Callbacks.EventHandler(
                    &impl->base.mOmxComp, impl->m_pAppData,
                    OMX_EventCmdComplete, OMX_CommandPortEnable, 0x0, NULL);
                impl->m_Callbacks.EventHandler(
                    &impl->base.mOmxComp, impl->m_pAppData,
                    OMX_EventCmdComplete, OMX_CommandPortEnable, 0x1, NULL);
                break;
            }
            else if (nTimeout++ > OMX_MAX_TIMEOUTS)
            {
                impl->m_Callbacks.EventHandler(
                    &impl->base.mOmxComp, impl->m_pAppData, OMX_EventError,
                    OMX_ErrorPortUnresponsiveDuringAllocation, 0, NULL);
                break;
            }

            usleep(OMX_TIMEOUT*1000);
        }
        closeVencDriver(impl);
        openVencDriver(impl);
        logd("********************reopen venc********************");
        if (impl->m_port_setting_match == OMX_FALSE)
            impl->m_port_setting_match = OMX_TRUE;
    }
    else if (cmd == MAIN_THREAD_CMD_FLUSH)
    {
        logd("x flush command.");
        // Flush port(s)
        // pCmdData contains the port index to be flushed.
        // It is assumed that 0 is input and 1 is output port for this component
        // The pCmdData value -1 means that both input and output ports will be flushed.
        if (pCmdData == 0x0 || (int)pCmdData == -1)
        {
            // Return all input buffers and send cmdcomplete
            flushBuffers(impl, &impl->m_sInBufList);
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete,
                                            OMX_CommandFlush, 0x0, NULL);
        }

        if (pCmdData == 0x1 || (int)pCmdData == -1)
        {
            // Return all output buffers and send cmdcomplete
            flushBuffers(impl, &impl->m_sOutBufList);
            impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                            OMX_EventCmdComplete,
                                            OMX_CommandFlush, 0x1, NULL);
        }
    }
    else if (cmd == MAIN_THREAD_CMD_STOP)
    {
        logd("x stop command.");
        post_message_to_venc_and_wait(impl, OMX_Venc_Cmd_Stop);
        // Kill thread
        return -1;//OMX_RESULT_EXIT;
    }
    else if (cmd == MAIN_THREAD_CMD_FILL_BUF)
    {
        putOneBufferToList(impl, &impl->m_sOutBufList, pCmdData);
    }
    else if (cmd == MAIN_THREAD_CMD_EMPTY_BUF)
    {
        logv("line %d", __LINE__);
        putOneBufferToList(impl, &impl->m_sInBufList, pCmdData);

        // Mark current buffer if there is outstanding command
        if (pMarkBuf)
        {
            ((OMX_BUFFERHEADERTYPE *)(pCmdData))->hMarkTargetComponent = &impl->base.mOmxComp;
            ((OMX_BUFFERHEADERTYPE *)(pCmdData))->pMarkData = pMarkBuf->pMarkData;
            pMarkBuf = NULL;
        }
    }
    else if (cmd == MAIN_THREAD_CMD_MARK_BUF)
    {
        if (!pMarkBuf)
            pMarkBuf = (OMX_MARKTYPE *)(pCmdData);
    }

    return 0;//OMX_RESULT_OK;
}
static inline void showEncodeResult(AwOmxVenc* impl, OMX_S32 result)
{
    switch (result)
    {
        case VENC_RESULT_ERROR:
        {
            impl->mDebugInfo.nErrCount++;
            break;
        }
        case VENC_RESULT_NO_FRAME_BUFFER:
        {
            impl->mDebugInfo.nNoFrameRetCount++;
            break;
        }
        case VENC_RESULT_OK:
        {
            impl->mDebugInfo.nOkRetCount++;
            break;
        }
        case VENC_RESULT_BITSTREAM_IS_EMPTY:
        {
            impl->mDebugInfo.nEmptyBitCount++;
            break;
        }
        default:
        {
            impl->mDebugInfo.nOtherRetCount++;
            break;
        }

    }
    if((impl->mDebugInfo.nErrCount + impl->mDebugInfo.nNoFrameRetCount +
        impl->mDebugInfo.nOkRetCount + impl->mDebugInfo.nEmptyBitCount
        + impl->mDebugInfo.nOtherRetCount) >= 100)
    {
        OMX_LOGI("Encode result: ok: %lu, err %lu, no frame buffer %lu, empty stream %lu...",
            impl->mDebugInfo.nOkRetCount,
            impl->mDebugInfo.nErrCount,
            impl->mDebugInfo.nNoFrameRetCount,
            impl->mDebugInfo.nEmptyBitCount);
        memset(&impl->mDebugInfo, 0, sizeof(impl->mDebugInfo));
    }
}

static void dealWithInputBuffer(AwOmxVenc* impl)
{
    int result = 0;
    if (impl->mInputBufferStep == OMX_VENC_STEP_GET_INPUTBUFFER)
     {
         getOneBufferFromList(impl, &impl->m_sInBufList, &impl->mInBufHdr);

         if (impl->mInBufHdr)
         {
             omx_sem_up(&impl->m_need_sleep_sem);

             if (impl->mInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
             {
                 // Copy flag to output buffer header
                 impl->m_inBufEos = OMX_TRUE;
                 logd(" set up impl->m_inBufEos flag.: %p", impl->mInBufHdr);
                 // Trigger event handler
                 impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                 OMX_EventBufferFlag, 0x1,
                                                 impl->mInBufHdr->nFlags, NULL);
                 // Clear flag
                 impl->mInBufHdr->nFlags = 0;
             }

             // Check for mark buffers
             if (impl->mInBufHdr->pMarkData)
             {
                 // Copy mark to output buffer header
                 if (impl->mOutBufHdr)
                 {
                     impl->m_pMarkData = impl->mInBufHdr->pMarkData;
                     // Copy handle to output buffer header
                     impl->m_hMarkTargetComponent = impl->mInBufHdr->hMarkTargetComponent;
                 }
             }

             // Trigger event handler
             if (impl->mInBufHdr->hMarkTargetComponent == &impl->base.mOmxComp && impl->mInBufHdr->pMarkData)
             {
                 impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                 OMX_EventMark, 0, 0, impl->mInBufHdr->pMarkData);
             }

             if (!impl->m_useAllocInputBuffer)
             {
                 if (impl->mInBufHdr->nFilledLen <= 0)
                 {
                     logw("skip this input buffer, impl->mInBufHdr->nTimeStamp %lld",
                          impl->mInBufHdr->nTimeStamp);
                     impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp,
                                                        impl->m_pAppData, impl->mInBufHdr);
                     impl->mInBufHdr = NULL;
                 }
                 else
                 {
                     getAndAddInputBuffer(impl, impl->mInBufHdr, &impl->m_sInputBuffer);
                 }
             }
             else
             {
                 int buffer_type =  *(int*)(impl->mInBufHdr->pBuffer+impl->mInBufHdr->nOffset);

                 //if (impl->m_useMetaDataInBuffers && buffer_type !=
                 //    kMetadataBufferTypeGrallocSource)
                 if (impl->mInBufHdr->nFilledLen <= 0)
                 {
                     logw("skip this input buffer, pInBufHd:%p,"
                         "buffer_type=%08x,buf_size=%d",
                          impl->mInBufHdr,buffer_type,
                          (int)impl->mInBufHdr->nFilledLen);
                     impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp,
                                        impl->m_pAppData, impl->mInBufHdr);
                     impl->mInBufHdr = NULL;
                 }
                 else
                 {
                     result = GetOneAllocInputBuffer(impl->m_encoder,
                                             &impl->m_sInputBuffer);

                     if (result !=0)
                     {
                         impl->mInputBufferStep =
                             OMX_VENC_STEP_GET_ALLOCBUFFER;
                     }
                     else
                     {
                         switch (impl->m_sInPortFormatType.eColorFormat)
                         {

                             case OMX_COLOR_FormatYUV420SemiPlanar:
                             {
                                 impl->mSizeY =
                                     impl->m_sInPortDefType.format.video.nStride *
                                     impl->m_sInPortDefType.format.video.nFrameHeight;
                                 impl->mSizeC = impl->mSizeY>>1;
                                 break;
                             }
                             default:
                             {
                                 impl->mSizeY = impl->m_sInPortDefType.format.video.nStride *
                                     impl->m_sInPortDefType.format.video.nFrameHeight;
                                 impl->mSizeC = impl->mSizeY>>1;
                                 break;
                             }
                         }

                         //* clear flag
                         impl->m_sInputBuffer.nFlag = 0;
                         if (impl->mInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
                         {
                             impl->m_sInputBuffer.nFlag |= VENC_BUFFERFLAG_EOS;
                         }

                         impl->m_sInputBuffer.nPts = impl->mInBufHdr->nTimeStamp;
                         impl->m_sInputBuffer.bEnableCorp = 0;
                         getInputBufferFromBufHdr(impl, impl->mInBufHdr, &impl->m_sInputBuffer);

                         impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp,
                                                            impl->m_pAppData, impl->mInBufHdr);

                        FlushCacheAllocInputBuffer(impl->m_encoder, &impl->m_sInputBuffer);
                        #ifdef CONF_OMX_ENABLE_ALIGN//tina platform requires this setting!!!
                        if (impl->m_sInPortDefType.format.video.nFrameWidth != impl->m_sOutPortDefType.format.video.nFrameWidth ||
                            impl->m_sInPortDefType.format.video.nFrameHeight != impl->m_sOutPortDefType.format.video.nFrameHeight)
                        {
                            impl->m_sInputBuffer.bEnableCorp = 1;
                            impl->m_sInputBuffer.sCropInfo.nLeft = 0;
                            /* YUV SAMPLE 4:2:0 4:2:2 4:4:4 */
                            impl->m_sInputBuffer.sCropInfo.nWidth = ROUND_DOWN_2(impl->m_sOutPortDefType.format.video.nFrameWidth);
                            impl->m_sInputBuffer.sCropInfo.nTop = 0;
                            impl->m_sInputBuffer.sCropInfo.nHeight = ROUND_DOWN_2(impl->m_sOutPortDefType.format.video.nFrameHeight);
                        }
                        #endif

                         result = AddOneInputBuffer(impl->m_encoder, &impl->m_sInputBuffer);

                         if (result!=0)
                         {
                             impl->mInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
                         }
                         else
                         {
                             impl->mInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
                         }
                     }
                 }
             }
         }
         else
         {
             //* do nothing
         }
     }
     else if (impl->mInputBufferStep == OMX_VENC_STEP_GET_ALLOCBUFFER)
     {
         result = GetOneAllocInputBuffer(impl->m_encoder, &impl->m_sInputBuffer);

         if (result !=0)
         {
             impl->mInputBufferStep = OMX_VENC_STEP_GET_ALLOCBUFFER;
         }
         else
         {
             switch (impl->m_sInPortFormatType.eColorFormat)
             {
                 case OMX_COLOR_FormatYUV420SemiPlanar:
                 {
                     impl->mSizeY = impl->m_sInPortDefType.format.video.nStride *
                         impl->m_sInPortDefType.format.video.nFrameHeight;
                     impl->mSizeC = impl->mSizeY>>1;
                     break;
                 }
                 default:
                 {
                     impl->mSizeY = impl->m_sInPortDefType.format.video.nStride *
                         impl->m_sInPortDefType.format.video.nFrameHeight;
                     impl->mSizeC= impl->mSizeY>>1;
                     break;
                 }
             }

             //* clear flag
             impl->m_sInputBuffer.nFlag = 0;
             if (impl->mInBufHdr->nFlags & OMX_BUFFERFLAG_EOS)
             {
                 impl->m_sInputBuffer.nFlag |= VENC_BUFFERFLAG_EOS;
             }

             impl->m_sInputBuffer.nPts = impl->mInBufHdr->nTimeStamp;
             impl->m_sInputBuffer.bEnableCorp = 0;

             getInputBufferFromBufHdrWithoutCrop(impl,impl->mInBufHdr,&impl->m_sInputBuffer);

             impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp, impl->m_pAppData,
                                                impl->mInBufHdr);

             result = AddOneInputBuffer(impl->m_encoder, &impl->m_sInputBuffer);
             if (result!=0)
             {
                 impl->mInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
             }
             else
             {
                 impl->mInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
             }
         }

         omx_sem_up(&impl->m_need_sleep_sem);
     }
     else if (impl->mInputBufferStep == OMX_VENC_STEP_ADD_BUFFER_TO_ENC)
     {
         result = AddOneInputBuffer(impl->m_encoder, &impl->m_sInputBuffer);
         if (result!=0)
         {
             impl->mInputBufferStep = OMX_VENC_STEP_ADD_BUFFER_TO_ENC;
         }
         else
         {
             impl->mInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;
             omx_sem_up(&impl->m_need_sleep_sem);
         }
     }
}

/*
 *  Component Thread
 *    The ComponentThread functiontion is exeuted in a separate pThread and
 *    is used to implement the actual component functiontions.
 */
 /*****************************************************************************/
static void* ComponentThread(void* pThreadData)
{
    // Recover the pointer to my component specific data
    AwOmxVenc* impl = (AwOmxVenc*)pThreadData;
    OMX_U32                 pCmdData;
    impl->mInBufHdr   = NULL;
    impl->mOutBufHdr  = NULL;
    impl->m_port_setting_match   = OMX_TRUE;
    impl->m_inBufEos            = OMX_FALSE;
    //nVencNotifyEosFlag   = OMX_FALSE;
    impl->m_pMarkData            = NULL;
    impl->m_hMarkTargetComponent = NULL;

    CdcMessage msg;
    memset(&msg, 0, sizeof(CdcMessage));

    impl->mInputBufferStep = OMX_VENC_STEP_GET_INPUTBUFFER;

    while (1)
    {
        if(CdcMessageQueueTryGetMessage(impl->mqMainThread, &msg, 0) == 0)
        {
            pCmdData = (OMX_U32) msg.params[0];
            if(processThreadCommand(impl, &msg) == -1)
            {
                goto EXIT;
            }
        }

        // Buffer processing
        // Only happens when the component is in executing state.
        if(impl->m_encoder == NULL)
            goto ENCODER_ERROR;
        if (impl->m_state == OMX_StateExecuting  &&
            impl->m_sInPortDefType.bEnabled      &&
            impl->m_sOutPortDefType.bEnabled     &&
            impl->m_port_setting_match)
        {
            //* check input buffer.
            omx_sem_reset(&impl->m_need_sleep_sem);

            if (impl->m_inBufEos && (impl->mInputBufferStep == OMX_VENC_STEP_GET_INPUTBUFFER))
            {
                post_message_to_venc_and_wait(impl, OMX_Venc_Cmd_Enc_Idle);

                if (ValidBitstreamFrameNum(impl->m_encoder) <= 0)
                {
                    getOneBufferFromList(impl, &impl->m_sOutBufList, &impl->mOutBufHdr);

                    //* if no output buffer, wait for some time.
                    if (impl->mOutBufHdr == NULL)
                    {
                    }
                    else
                    {
                        impl->mOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_CODECCONFIG;
                        impl->mOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_SYNCFRAME;

                        impl->mOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                        impl->mOutBufHdr->nFilledLen = 0;

                        impl->m_Callbacks.FillBufferDone(&impl->base.mOmxComp, impl->m_pAppData,
                                                          impl->mOutBufHdr);
                        impl->mOutBufHdr = NULL;
                        impl->m_inBufEos = OMX_FALSE;
                    }
                }
            }

            dealWithInputBuffer(impl);

            //* check used buffer
            if (0==AlreadyUsedInputBuffer(impl->m_encoder, &impl->m_sInputBuffer_return))
            {
                omx_sem_up(&impl->m_need_sleep_sem);

                if (impl->m_useAllocInputBuffer)
                {
                    ReturnOneAllocInputBuffer(impl->m_encoder, &impl->m_sInputBuffer_return);
                }
                else
                {
                    impl->mInBufHdr = (OMX_BUFFERHEADERTYPE*)impl->m_sInputBuffer_return.nID;
                    impl->m_Callbacks.EmptyBufferDone(&impl->base.mOmxComp, impl->m_pAppData,
                                                       impl->mInBufHdr);
                }
            }

            if (ValidBitstreamFrameNum(impl->m_encoder) > 0)
            {
                //* check output buffer
                getOneBufferFromList(impl, &impl->m_sOutBufList, &impl->mOutBufHdr);

                impl->m_frameCountForCSD++;
                if (impl->mOutBufHdr)
                {
                    impl->mOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_CODECCONFIG;
                    impl->mOutBufHdr->nFlags &= ~OMX_BUFFERFLAG_SYNCFRAME;

                    if ((impl->m_vencCodecType == VENC_CODEC_H264
                        || impl->m_vencCodecType == VENC_CODEC_H265)
                        && (impl->m_frameCountForCSD % impl->m_vencH264Param.nMaxKeyInterval) == 1
                        && (impl->m_frameCountForCSD < impl->m_vencH264Param.nMaxKeyInterval * 10))
                    {
                        impl->mOutBufHdr->nTimeStamp = 0; //fixed it later;
                        impl->mOutBufHdr->nFilledLen = impl->m_headdata.nLength;
                        impl->mOutBufHdr->nOffset = 0;

                        memcpy(impl->mOutBufHdr->pBuffer, impl->m_headdata.pBuffer,
                               impl->mOutBufHdr->nFilledLen);
                        impl->mOutBufHdr->nFlags |= OMX_BUFFERFLAG_CODECCONFIG;

                        if(impl->bSaveStreamFlag)
                        {
                            if (impl->m_outFile)
                            {
                                fwrite(impl->mOutBufHdr->pBuffer, 1, impl->mOutBufHdr->nFilledLen, impl->m_outFile);
                            }
                            else
                            {
                                logw("open impl->m_outFile failed");
                            }
                        }

                        impl->m_Callbacks.FillBufferDone(&impl->base.mOmxComp, impl->m_pAppData,
                                                          impl->mOutBufHdr);
                        logv("submit output buffer with length %ld.", impl->mOutBufHdr->nFilledLen);

                        impl->mOutBufHdr = NULL;
                    }
                    else
                    {
                        GetOneBitstreamFrame(impl->m_encoder, &impl->m_sOutputBuffer);

                        impl->mOutBufHdr->nTimeStamp = impl->m_sOutputBuffer.nPts;
                        impl->mOutBufHdr->nFilledLen = impl->m_sOutputBuffer.nSize0 + impl->m_sOutputBuffer.nSize1;
                        impl->mOutBufHdr->nOffset = 0;

                        if(impl->bShowFrameSizeFlag)
						{

                            impl->mFrameCnt++;
                            impl->mAllFrameSize += impl->mOutBufHdr->nFilledLen;

                            struct timeval cur_time;
                            gettimeofday(&cur_time, NULL);
                            const uint64_t now_time = cur_time.tv_sec * 1000000LL + cur_time.tv_usec;
                            if (((now_time-impl->mTimeStart)/1000) >= impl->mTimeOut)
                            {
                                int bitrate_real = (impl->mAllFrameSize) /
                                    ((float)(now_time-impl->mTimeStart) / 1000000) * 8;
                                if (bitrate_real)
                                {
                                    logd("venc bitrate real:%d,set:%ld , framerate real:%d,set:%ld , \
                                             avg framesize real:%d,set:%ld    ",
                                         bitrate_real,
                                         impl->m_sOutPortDefType.format.video.nBitrate,
                                         impl->mFrameCnt,
                                         impl->m_sInPortDefType.format.video.xFramerate/*>>16*/,
                                         bitrate_real/impl->mFrameCnt,
                                         impl->m_sOutPortDefType.format.video.nBitrate /
                                             (impl->m_sInPortDefType.format.video.xFramerate/*>>16*/));
                                }
                                impl->mTimeStart = now_time;
                                impl->mFrameCnt = 0;
                                impl->mAllFrameSize = 0;
                            }
                        }

                        impl->mOutBufHdr->nFlags |= OMX_BUFFERFLAG_ENDOFFRAME;

                        if (impl->m_sOutputBuffer.nFlag & VENC_BUFFERFLAG_KEYFRAME)
                        {
                            impl->mOutBufHdr->nFlags |= OMX_BUFFERFLAG_SYNCFRAME;
                        }

                        if (impl->m_sOutputBuffer.nFlag & VENC_BUFFERFLAG_EOS)
                        {
                            impl->mOutBufHdr->nFlags |= OMX_BUFFERFLAG_EOS;
                        }

                        if (impl->m_prependSPSPPSToIDRFrames ==
                            OMX_TRUE && (impl->m_sOutputBuffer.nFlag & VENC_BUFFERFLAG_KEYFRAME))
                        {
                            memcpy(impl->mOutBufHdr->pBuffer, impl->m_headdata.pBuffer,
                                   impl->m_headdata.nLength);
                            memcpy(impl->mOutBufHdr->pBuffer + impl->m_headdata.nLength,
                                   impl->m_sOutputBuffer.pData0, impl->m_sOutputBuffer.nSize0);
                            if (impl->m_sOutputBuffer.nSize1)
                            {
                                memcpy((impl->mOutBufHdr->pBuffer + impl->m_headdata.nLength +
                                        impl->m_sOutputBuffer.nSize0),
                                       impl->m_sOutputBuffer.pData1, impl->m_sOutputBuffer.nSize1);
                            }

                            impl->mOutBufHdr->nFilledLen += impl->m_headdata.nLength;
                        }
                        else
                        {
                            memcpy(impl->mOutBufHdr->pBuffer, impl->m_sOutputBuffer.pData0,
                                   impl->m_sOutputBuffer.nSize0);
                            if (impl->m_sOutputBuffer.nSize1)
                            {
                                memcpy(impl->mOutBufHdr->pBuffer + impl->m_sOutputBuffer.nSize0,
                                       impl->m_sOutputBuffer.pData1, impl->m_sOutputBuffer.nSize1);
                            }
                        }

                        if(impl->bSaveStreamFlag)
                        {
                            if (impl->m_outFile)
                            {
                                fwrite(impl->mOutBufHdr->pBuffer, 1, impl->mOutBufHdr->nFilledLen, impl->m_outFile);
                            }
                            else
                            {
                                logw("open impl->m_outFile failed");
                            }
                        }

                        FreeOneBitStreamFrame(impl->m_encoder, &impl->m_sOutputBuffer);

                        // Check for mark buffers
                        if (impl->m_pMarkData != NULL && impl->m_hMarkTargetComponent != NULL)
                        {
                            if (ValidBitstreamFrameNum(impl->m_encoder) == 0)
                            {
                                // Copy mark to output buffer header
                                impl->mOutBufHdr->pMarkData = impl->mInBufHdr->pMarkData;
                                // Copy handle to output buffer header
                                impl->mOutBufHdr->hMarkTargetComponent = impl->mInBufHdr->hMarkTargetComponent;

                                impl->m_pMarkData = NULL;
                                impl->m_hMarkTargetComponent = NULL;
                            }
                         }

                        impl->m_Callbacks.FillBufferDone(&impl->base.mOmxComp, impl->m_pAppData,
                                                          impl->mOutBufHdr);
                        logv("call back of fillBufferDone, submit output buffer with length %ld.",
                            impl->mOutBufHdr->nFilledLen);
                        impl->mOutBufHdr = NULL;
                    }
                }
                else
                {
                    //* do nothing
                }
            }

ENCODER_ERROR:

            omx_sem_timeddown(&impl->m_need_sleep_sem, 10);
        }
    }

EXIT:

    return (void*)OMX_ErrorNone;
}

static void* ComponentVencThread(void* pThreadData)
{
    int                     result = 0;
    OMX_VENC_COMMANDTYPE    cmd;
    int nStopFlag = 0;
    int nWaitIdle = 0;

    // Recover the pointer to my component specific data
   AwOmxVenc* impl = (AwOmxVenc*)pThreadData;

    while (1)
    {
        CdcMessage msg;
        memset(&msg, 0, sizeof(CdcMessage));

        if(CdcMessageQueueTryGetMessage(impl->mqVencThread, &msg, 0) == 0)
        {
            process_venc_cmd:
            cmd = (OMX_VENC_COMMANDTYPE)msg.messageId;
            logd("(vdrvThread receive cmd[%s]", strVencCmd[cmd]);
            // State transition command

            switch (cmd)
            {
                case OMX_Venc_Cmd_Open:
                {
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    if (!impl->m_encoder)
                    {
                        openVencDriver(impl);
                    }
                    omx_sem_up(&impl->m_msg_sem);
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);

                    break;
                }

                case OMX_Venc_Cmd_Close:
                {
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    if (impl->m_encoder)
                    {
                        int ret;
                        ret = deparseEncInputBuffer(impl->mIonFd,
                                                        impl->m_encoder,
                                                        impl->mInputBufInfo);
                        if(ret < 0)
                            loge("deparse_omx_enc_input_buffer error\n");

                        closeVencDriver(impl);
                    }
                    omx_sem_up(&impl->m_msg_sem);
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    break;
                }

                case OMX_Venc_Cmd_Stop:
                {
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    nStopFlag = 1;
                    omx_sem_up(&impl->m_msg_sem);
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    break;
                }

                case OMX_Venc_Cmd_Enc_Idle:
                {
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    nWaitIdle = 1;
                    logv("(f:%s, l:%d) vencThread receive cmd[0x%x]", __FUNCTION__, __LINE__, cmd);
                    break;
                }

                case OMX_Venc_Cmd_ChangeBitrate:
                {
                    logd("ChangeBitrate cmd. bitrate: %d, frame rate: %d.",
                         (int)impl->m_sOutPortDefType.format.video.nBitrate, (int)impl->m_framerate);
                    VideoEncSetParameter(impl->m_encoder, VENC_IndexParamBitrate,
                                         &impl->m_sOutPortDefType.format.video.nBitrate);
                    if (impl->mVideoSuperFrame.bEnable)
                    {
                        setSuperFrameCfg(impl);
                    }
                    break;
                }

                case OMX_Venc_Cmd_ChangeColorFormat:
                {
                    VideoEncSetParameter(impl->m_encoder, VENC_IndexParamColorFormat,
                                         &impl->m_vencColorFormat);
                    omx_sem_up(&impl->m_msg_sem);
                    break;
                }

                case OMX_Venc_Cmd_RequestIDRFrame:
                {
                    int value = 1;
                    VideoEncSetParameter(impl->m_encoder,VENC_IndexParamForceKeyFrame, &value);
                    logd("(f:%s, l:%d) OMX_Venc_Cmd_RequestIDRFrame[0x%x]",
                         __FUNCTION__, __LINE__, cmd);
                    break;
                }

                case OMX_Venc_Cmd_ChangeFramerate:
                {
                    if(!impl->m_framerate)
                    {
                        loge("frame rate = 0!! reset it as default 30 fps.");
                        impl->m_framerate = 30;
                        break;
                    }
                    logd("ChangeFramerate. bitrate: %d, framerate: %d",
                         (int)impl->m_sOutPortDefType.format.video.nBitrate,
                         (int)impl->m_framerate);
                    VideoEncSetParameter(impl->m_encoder, VENC_IndexParamFramerate,
                                         &impl->m_framerate);
                    if (impl->mVideoSuperFrame.bEnable)
                        setSuperFrameCfg(impl);
                    setSVCSkipCfg(impl);
                    break;
                }
                default:
                {
                    logw("unknown cmd: %d", cmd);
                    break;
                }
            }
        }

        if (nStopFlag)
        {
            logd("vencThread detect nStopFlag[%d], exit!", (int)nStopFlag);
            goto EXIT;
        }

        if (impl->m_state == OMX_StateExecuting && impl->m_encoder)
        {
            if(impl->bOpenStatisticFlag)
			{
                impl->mTimeUs1 = GetNowUs();
            }
            result = VideoEncodeOneFrame(impl->m_encoder);

            showEncodeResult(impl, result);

            if(impl->bOpenStatisticFlag)
			{
                impl->mTimeUs2 = GetNowUs();
                logv("MicH264Enc, VideoEncodeOneFrame time %ld."\
                    " buffer by encoder %ld and by decoder %ld.",
                    (impl->mTimeUs2-impl->mTimeUs1), impl->m_outBufferCountOwnedByEncoder,
                    impl->m_outBufferCountOwnedByRender);
            }

            if (result == VENC_RESULT_ERROR)
            {
                impl->m_Callbacks.EventHandler(&impl->base.mOmxComp, impl->m_pAppData,
                                                OMX_EventError, OMX_ErrorHardware,
                                                0 , NULL);
                loge("VideoEncodeOneFrame, failed, result: %s\n", strVencResult[result+1]);
            }

            if (nWaitIdle && result == VENC_RESULT_NO_FRAME_BUFFER)
            {
                logv("input buffer idle \n");
                omx_sem_up(&impl->m_msg_sem);
                nWaitIdle = 0;
            }

            if (result != VENC_RESULT_OK)
            {
                if(CdcMessageQueueTryGetMessage(impl->mqVencThread, &msg, 10) == 0)
                    goto process_venc_cmd;
            }
            //logv("encoding..... result %s.", strVencResult[result+1]);
        }
        else
        {
            if(CdcMessageQueueTryGetMessage(impl->mqVencThread, &msg, 10) == 0)
                goto process_venc_cmd;
        }
    }

EXIT:
    return (void*)OMX_ErrorNone;
}
