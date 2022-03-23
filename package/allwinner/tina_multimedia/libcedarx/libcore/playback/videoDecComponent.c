/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : videoDecComponent.cpp
 * Description : video decoder component
 * History :
 *
 */

//#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_DETAIL
//#define LOG_TAG "videoDecComponent"
#include "cdx_log.h"

#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>

#include "videoDecComponent.h"
#include "videoRenderComponent.h"
#include "AwMessageQueue.h"
#include "memoryAdapter.h"
#include "baseComponent.h"

#include <iniparserapi.h>

enum FIRST_FRAME_STATUS {
    FIRST_FRAME_NOT_READY,
    FIRST_3D_FRAME_READY,
    SECOND_3D_FRAME_READY,
    FIRST_FRAME_READY = SECOND_3D_FRAME_READY,
};

struct VideoDecComp
{
    //* created at initialize time.
    AwMessageQueue     *mq;
    BaseCompCtx         base;

    sem_t               streamDataSem;
    sem_t               frameBufferSem;

    pthread_t           sDecodeThread;

    VideoDecoder*       pDecoder;
    enum EPLAYERSTATUS  eStatus;

    //* objects set by user.
    AvTimer*            pAvTimer;
    PlayerCallback      callback;
    void*               pUserData;
    int                 bEosFlag;

    int                 bConfigDecodeKeyFrameOnly;
    int                 bConfigDropDelayFrames;

    int                 bResolutionChange;
    int                 bCrashFlag;

    VConfig             vconfig;
    VideoStreamInfo     videoStreamInfo;
    int                 nSelectStreamIndex;

    //*for new display
    VideoRenderComp*    pVideoRenderComp;
    VideoRenderCallback videoRenderCallback;
    void*               pVideoRenderUserData;
    FbmBufInfo          mFbmBufInfo;
    int                 bFbmBufInfoValidFlag;

    int                 nGpuAlignStride;

    pthread_mutex_t     videoRenderCallbackMutex;
    struct ScMemOpsS*   memOps;

    enum FIRST_FRAME_STATUS  firstFrameStatus;
};

static void handleStart(AwMessage *msg, void *arg);
static void handleStop(AwMessage *msg, void *arg);
static void handlePause(AwMessage *msg, void *arg);
static void handleReset(AwMessage *msg, void *arg);
static void handleSetEos(AwMessage *msg, void *arg);
static void handleQuit(AwMessage *msg, void *arg);
static void doDecode(AwMessage *msg, void *arg);

static void* VideoDecodeThread(void* arg);

VideoDecComp* VideoDecCompCreate(void)
{
    VideoDecComp *p;
    int err;

    p = (VideoDecComp*)calloc(1, sizeof(*p));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }

    p->mq = AwMessageQueueCreate(4, "VideoDecodeMq");
    if(p->mq == NULL)
    {
        loge("video decoder component create message queue fail.");
        free(p);
        return NULL;
    }

    BaseMsgHandler handler = {
        .start = handleStart,
        .stop = handleStop,
        .pause = handlePause,
        .reset = handleReset,
        .setEos = handleSetEos,
        .quit = handleQuit,
        .decode = doDecode,
    };

    if (BaseCompInit(&p->base, "video decoder", p->mq, &handler))
    {
        AwMessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    p->pDecoder = CreateVideoDecoder();
    if(p->pDecoder == NULL)
    {
        loge("video decoder component create decoder fail.");
        BaseCompDestroy(&p->base);
        AwMessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    pthread_mutex_init(&p->videoRenderCallbackMutex, NULL);
    sem_init(&p->streamDataSem, 0, 0);
    sem_init(&p->frameBufferSem, 0, 0);

    p->eStatus = PLAYER_STATUS_STOPPED;

    err = pthread_create(&p->sDecodeThread, NULL, VideoDecodeThread, p);
    if(err != 0)
    {
        loge("video decode component create thread fail.");
        sem_destroy(&p->streamDataSem);
        sem_destroy(&p->frameBufferSem);
        pthread_mutex_destroy(&p->videoRenderCallbackMutex);

        DestroyVideoDecoder(p->pDecoder);
        BaseCompDestroy(&p->base);
        AwMessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    p->nGpuAlignStride = GetConfigParamterInt("gpu_align_bitwidth", 0);
    CDX_LOG_CHECK(p->nGpuAlignStride == 128 || p->nGpuAlignStride == 32 || p->nGpuAlignStride == 16,
            "invalid config '%d', pls check your cedarx.conf", p->nGpuAlignStride);

    return p;
}

static void WakeUpThread(void *arg)
{
    VideoDecComp *p = arg;
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);
}

int VideoDecCompDestroy(VideoDecComp* p)
{
    BaseCompQuit(&p->base, WakeUpThread, p);
    pthread_join(p->sDecodeThread, NULL);
    BaseCompDestroy(&p->base);

    if(p->memOps != NULL)
    {
        CdcMemClose(p->memOps);
    }

    free(p->videoStreamInfo.pCodecSpecificData);

    sem_destroy(&p->streamDataSem);
    sem_destroy(&p->frameBufferSem);

    DestroyVideoDecoder(p->pDecoder);
    AwMessageQueueDestroy(p->mq);

    pthread_mutex_destroy(&p->videoRenderCallbackMutex);

    free(p);

    return 0;
}

int VideoDecCompStart(VideoDecComp* p)
{
    return BaseCompStart(&p->base, WakeUpThread, p);
}

int VideoDecCompStop(VideoDecComp* p)
{
    return BaseCompStop(&p->base, WakeUpThread, p);
}

int VideoDecCompPause(VideoDecComp* p)
{
    return BaseCompPause(&p->base, WakeUpThread, p);
}

enum EPLAYERSTATUS VideoDecCompGetStatus(VideoDecComp* p)
{
    return p->eStatus;
}

int VideoDecCompReset(VideoDecComp* p)
{
    return BaseCompReset(&p->base, 0, WakeUpThread, p);
}

int VideoDecCompSetEOS(VideoDecComp* p)
{
    return BaseCompSetEos(&p->base, WakeUpThread, p);
}

int VideoDecCompSetCallback(VideoDecComp* p, PlayerCallback callback, void* pUserData)
{
    p->callback  = callback;
    p->pUserData = pUserData;
    return 0;
}

int VideoDecCompSetDecodeKeyFrameOnly(VideoDecComp* p, int bDecodeKeyFrameOnly)
{
    p->bConfigDecodeKeyFrameOnly  = bDecodeKeyFrameOnly;
    return 0;
}

int VideoDecCompSetDropDelayFrames(VideoDecComp* p, int bDropDelayFrames)
{
    p->bConfigDropDelayFrames  = bDropDelayFrames;
    return 0;
}

int VideoDecCompSetVideoStreamInfo(VideoDecComp* p, VideoStreamInfo* pStreamInfo, VConfig* pVconfig)
{
    int ret;
    pVconfig->nAlignStride = p->nGpuAlignStride;
    pVconfig->bGpuBufValid = 1;

    logd("++++++++ pVconfig->bGpuBufValid = %d,nGpuAlignStride = %d ",
         pVconfig->bGpuBufValid,p->nGpuAlignStride);
    //* save the config and video stream info
    if(p->videoStreamInfo.pCodecSpecificData)
    {
        free(p->videoStreamInfo.pCodecSpecificData);
        p->videoStreamInfo.pCodecSpecificData = NULL;
        p->videoStreamInfo.nCodecSpecificDataLen = 0;
    }

    p->vconfig = *pVconfig;
    p->videoStreamInfo = *pStreamInfo;

    if(pStreamInfo->nCodecSpecificDataLen > 0)
    {
        p->videoStreamInfo.pCodecSpecificData = (char*)malloc(pStreamInfo->nCodecSpecificDataLen);
        if(p->videoStreamInfo.pCodecSpecificData == NULL)
        {
            loge("malloc video codec specific data fail!");
            return -1;
        }
        memcpy(p->videoStreamInfo.pCodecSpecificData,
               pStreamInfo->pCodecSpecificData,
               pStreamInfo->nCodecSpecificDataLen);
        p->videoStreamInfo.nCodecSpecificDataLen = pStreamInfo->nCodecSpecificDataLen;
    }

    logv("pStreamInfo->bSecureStreamFlagLevel1 = %d",pStreamInfo->bSecureStreamFlagLevel1);
    pVconfig->memops = MemAdapterGetOpsS();

#if (defined(CONF_AFBC_ENABLE))
    if(pStreamInfo->eCodecFormat == VIDEO_CODEC_FORMAT_H265)
    {
        logd("enable afbc mode for H265 4k");
        pVconfig->eCtlAfbcMode = ENABLE_AFBC_JUST_BIG_SIZE;
    }
    else
    {
        logd("disable afbc mode");
        pVconfig->eCtlAfbcMode = DISABLE_AFBC_ALL_SIZE;
    }
#endif

    ret = InitializeVideoDecoder(p->pDecoder, pStreamInfo, pVconfig);

    p->memOps = MemAdapterGetOpsS();
    CdcMemOpen(p->memOps);

_exit:
    if(ret < 0)
    {
        p->bCrashFlag = 1;
        loge("VideoDecCompSetVideoStreamInfo fail");
    }

    return ret;
}

int VideoDecCompGetVideoStreamInfo(VideoDecComp* p, VideoStreamInfo* pStreamInfo)
{
    return GetVideoStreamInfo(p->pDecoder, pStreamInfo);
}

int VideoDecCompSetTimer(VideoDecComp* p, AvTimer* timer)
{
    p->pAvTimer  = timer;
    return 0;
}

int VideoDecCompRequestStreamBuffer(VideoDecComp* p,
                                    int           nRequireSize,
                                    char**        ppBuf,
                                    int*          pBufSize,
                                    char**        ppRingBuf,
                                    int*          pRingBufSize,
                                    int           nStreamIndex)
{
    int ret;

    //* we can use the same sbm interface to process secure video

    ret = RequestVideoStreamBuffer(p->pDecoder,
                                    nRequireSize,
                                    ppBuf,
                                    pBufSize,
                                    ppRingBuf,
                                    pRingBufSize,
                                    nStreamIndex);

    if(p->bCrashFlag && (ret < 0 || ((*pBufSize + *pRingBufSize) < nRequireSize)))
    {
        //* decoder crashed.
        ResetVideoDecoder(p->pDecoder); //* flush streams.
    }

    return ret;
}

int VideoDecCompSubmitStreamData(VideoDecComp*        p,
                                 VideoStreamDataInfo* pDataInfo,
                                 int                  nStreamIndex)
{
    int                  ret;
    int                  nSemCnt;

    //* don't receive input stream when decoder crashed.
    //* so the stream buffer always has empty buffer for the demux.
    //* otherwise the demux thread will blocked when video stream buffer is full.
    if(p->bCrashFlag)
    {
        return 0;
    }

    //* we can use the same sbm interface to process secure video

    ret = SubmitVideoStreamData(p->pDecoder, pDataInfo, nStreamIndex);
    if(sem_getvalue(&p->streamDataSem, &nSemCnt) == 0)
    {
        if(nSemCnt == 0)
            sem_post(&p->streamDataSem);
    }
    p->nSelectStreamIndex = nStreamIndex;

    return ret;
}

int VideoDecCompStreamBufferSize(VideoDecComp* p, int nStreamIndex)
{
    if (p == NULL)
        return 0;

    return VideoStreamBufferSize(p->pDecoder, nStreamIndex);
}

int VideoDecCompStreamDataSize(VideoDecComp* p, int nStreamIndex)
{
    if(p == NULL)
        return 0;

    return VideoStreamDataSize(p->pDecoder, nStreamIndex);
}

int VideoDecCompStreamFrameNum(VideoDecComp* p, int nStreamIndex)
{
    if (p == NULL)
        return 0;

    return VideoStreamFrameNum(p->pDecoder, nStreamIndex);
}

VideoPicture* VideoDecCompRequestPicture(VideoDecComp* p, int nStreamIndex, int* bResolutionChanged)
{
    VideoPicture *pPicture;

    pPicture = RequestPicture(p->pDecoder, nStreamIndex);
    if(bResolutionChanged != NULL)
    {
        if(pPicture != NULL || p->bResolutionChange == 0)
        {
            *bResolutionChanged = 0;
        }
        else
        {
            logd("set resolution changed.");
            *bResolutionChanged = 1;
        }
    }

    return pPicture;
}

int VideoDecCompReturnPicture(VideoDecComp* p, VideoPicture* pPicture)
{
    int ret;
    int nSemCnt;

    ret = ReturnPicture(p->pDecoder, pPicture);
    if(sem_getvalue(&p->frameBufferSem, &nSemCnt) == 0)
    {
        if(nSemCnt == 0)
            sem_post(&p->frameBufferSem);
    }

    return ret;
}

VideoPicture* VideoDecCompNextPictureInfo(VideoDecComp* p, int nStreamIndex,
        int* bResolutionChanged)
{
    VideoPicture*        pPicture;

    pPicture = NextPictureInfo(p->pDecoder, nStreamIndex);
    if(bResolutionChanged != NULL)
    {
        if(pPicture != NULL || p->bResolutionChange == 0)
            *bResolutionChanged = 0;
        else
        {
            logd("set resolution changed.");
            *bResolutionChanged = 1;
        }
    }
    return pPicture;
}

int VideoDecCompTotalPictureBufferNum(VideoDecComp* p, int nStreamIndex)
{
    return TotalPictureBufferNum(p->pDecoder, nStreamIndex);
}

int VideoDecCompEmptyPictureBufferNum(VideoDecComp* p, int nStreamIndex)
{
    return EmptyPictureBufferNum(p->pDecoder, nStreamIndex);
}

int VideoDecCompValidPictureNum(VideoDecComp* p, int nStreamIndex)
{
    return ValidPictureNum(p->pDecoder, nStreamIndex);
}

int VideoDecCompReopenVideoEngine(VideoDecComp* p)
{
    int ret;

    VideoStreamInfo* pVideoInfo =
        (VideoStreamInfo*)VideoStreamDataInfoPointer(p->pDecoder,p->nSelectStreamIndex);
    if(pVideoInfo != NULL)
    {
        if(p->videoStreamInfo.pCodecSpecificData)
            free(p->videoStreamInfo.pCodecSpecificData);
        memcpy(&p->videoStreamInfo,pVideoInfo,sizeof(VideoStreamInfo));
        if(pVideoInfo->pCodecSpecificData)
        {
            p->videoStreamInfo.pCodecSpecificData =
                (char*)malloc(pVideoInfo->nCodecSpecificDataLen);
            if(p->videoStreamInfo.pCodecSpecificData == NULL)
            {
                loge("malloc video codec specific data failed!");
                return -1;
            }
            memcpy(p->videoStreamInfo.pCodecSpecificData,
                   pVideoInfo->pCodecSpecificData,
                   pVideoInfo->nCodecSpecificDataLen);
            p->videoStreamInfo.nCodecSpecificDataLen = pVideoInfo->nCodecSpecificDataLen;

            free(pVideoInfo->pCodecSpecificData);
        }
        else
        {
            p->videoStreamInfo.pCodecSpecificData    = NULL;
            p->videoStreamInfo.nCodecSpecificDataLen = 0;
        }

        free(pVideoInfo);
    }
    else
    {
        //*if resolustionChange was detected by decoder, we should not send the
        //* specific data to decoder. or decoder will appear error.
        if(p->videoStreamInfo.pCodecSpecificData)
            free(p->videoStreamInfo.pCodecSpecificData);

        p->videoStreamInfo.pCodecSpecificData = NULL;
        p->videoStreamInfo.nCodecSpecificDataLen = 0;
    }

    p->vconfig.memops = MemAdapterGetOpsS();

    ret = ReopenVideoEngine(p->pDecoder,&p->vconfig,&p->videoStreamInfo);

    p->bResolutionChange = 0;
    BaseCompContinue(&p->base);

    return ret;
}

#if 0 /* v2.7 only support new display, do not need rotate pic in decoder */
int VideoDecCompRotatePicture(VideoDecComp* p,
                              VideoPicture* pPictureIn,
                              VideoPicture* pPictureOut,
                              int           nRotateDegree,
                              int           nGpuYAlign,
                              int           nGpuCAlign)
{
    //* on chip-1673,we use hardware to do video rotation
#if(ROTATE_PIC_HW == 1)
    CDX_PLAYER_UNUSE(nGpuYAlign);
    CDX_PLAYER_UNUSE(nGpuCAlign);
    return RotatePictureHw(p->pDecoder,pPictureIn, pPictureOut, nRotateDegree);
#else
    //* rotatePicture() should known the gpuAlign
    return RotatePicture(p->memOps, pPictureIn, pPictureOut, nRotateDegree,nGpuYAlign,nGpuCAlign);
#endif

}
#endif
/*
VideoPicture* VideoDecCompAllocatePictureBuffer(int nWidth, int nHeight,
            int nLineStride, int ePixelFormat)
{
    return AllocatePictureBuffer(nWidth, nHeight, nLineStride, ePixelFormat);
}

int VideoDecCompFreePictureBuffer(VideoPicture* pPicture)
{
    return FreePictureBuffer(pPicture);
}
*/

#if 0
//* for new display
int VideoDecCompSetVideoRenderCallback(VideoDecComp* v,
        VideoRenderCallback callback, void* pUserData)
{
    VideoDecComp* p;

    p = (VideoDecComp*)v;

    pthread_mutex_lock(&p->videoRenderCallbackMutex);
    p->videoRenderCallback  = callback;
    pthread_mutex_unlock(&p->videoRenderCallbackMutex);

    p->pVideoRenderUserData = pUserData;
    return 0;
}
#endif

//*******************************  START  **********************************//
//** for new display structure interface.
//**
FbmBufInfo* VideoDecCompGetVideoFbmBufInfo(VideoDecComp* p)
{
    if(p->pDecoder != NULL)
        return GetVideoFbmBufInfo(p->pDecoder);

    loge("the pDecoder is null when call VideoDecCompGetVideoFbmBufInfo()");
    return NULL;
}

VideoPicture* VideoDecCompSetVideoFbmBufAddress(VideoDecComp* p,
        VideoPicture* pVideoPicture, int bForbidUseFlag)
{
    if(p->pDecoder != NULL)
        return SetVideoFbmBufAddress(p->pDecoder,pVideoPicture,bForbidUseFlag);

    loge("the pDecoder is null when call VideoDecCompSetVideoFbmBufAddress()");
    return NULL;
}

int VideoDecCompSetVideoFbmBufRelease(VideoDecComp* p)
{
    if(p->pDecoder != NULL)
        return SetVideoFbmBufRelease(p->pDecoder);

    loge("the pDecoder is null when call VideoDecCompSetVideoFbmBufRelease()");
    return -1;
}

VideoPicture* VideoDecCompRequestReleasePicture(VideoDecComp* p)
{
    if(p->pDecoder != NULL)
        return RequestReleasePicture(p->pDecoder);

    loge("the pDecoder is null when call VideoDecCompRequestReleasePicture()");
    return NULL;
}

VideoPicture*  VideoDecCompReturnRelasePicture(VideoDecComp* p,
        VideoPicture* pVpicture, int bForbidUseFlag)
{
    if(p->pDecoder != NULL)
        return ReturnRelasePicture(p->pDecoder,pVpicture, bForbidUseFlag);

    loge("the pDecoder is null when call VideoDecCompReturnRelasePicture()");
    return NULL;
}

//***********************************************************************//
//*********************added by xyliu at 2015-12-23**********************//
int VideoDecCompSetExtraScaleInfo(VideoDecComp* p, int nWidthTh, int nHeightTh,
                                  int nHorizontalScaleRatio, int nVerticalScaleRatio)
{
    return ConfigExtraScaleInfo(p->pDecoder, nWidthTh, nHeightTh,
            nHorizontalScaleRatio, nVerticalScaleRatio);
}
//****************************************************************************//
//********************************  END  ***********************************//

#if 0
void VideoDecCompFreePictureBuffer(VideoDecComp* v, VideoPicture* pPicture)
{
    VideoDecComp* p;

    p = (VideoDecComp*)v;

    FreePictureBuffer(v, pPicture);
}
#endif

static void* VideoDecodeThread(void* arg)
{
    VideoDecComp *p = arg;
    AwMessage msg;

    while (AwMessageQueueGetMessage(p->mq, &msg) == 0)
    {
        if (msg.execute != NULL)
            msg.execute(&msg, p);
        else
            loge("msg with msg_id %d doesn't have a handler", msg.messageId);
    }

    return NULL;
}

static void handleStart(AwMessage *msg, void *arg)
{
    VideoDecComp *p = arg;

    if(p->eStatus == PLAYER_STATUS_STARTED)
    {
        loge("invalid start operation, already in started status.");
        if(msg->result != NULL)
            *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    if(p->eStatus == PLAYER_STATUS_PAUSED)
    {
        if(p->bCrashFlag == 0)
            BaseCompContinue(&p->base);
        p->eStatus = PLAYER_STATUS_STARTED;
        if (msg->result)
            *msg->result = 0;
        sem_post(msg->replySem);
        return;
    }

    BaseCompContinue(&p->base);
    ResetVideoDecoder(p->pDecoder);
    p->bEosFlag = 0;
    p->eStatus = PLAYER_STATUS_STARTED;

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);
}

static void handleStop(AwMessage *msg, void *arg)
{
    VideoDecComp *p = arg;

    if(p->eStatus == PLAYER_STATUS_STOPPED)
    {
        loge("invalid stop operation, already in stopped status.");
        if (msg->result)
            *msg->result = -1;
        sem_post(msg->replySem);
    }

    p->firstFrameStatus = FIRST_FRAME_NOT_READY;
    ResetVideoDecoder(p->pDecoder);
    p->eStatus = PLAYER_STATUS_STOPPED;

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);
}

static void handlePause(AwMessage *msg, void *arg)
{
    VideoDecComp *p = arg;

    if(p->eStatus != PLAYER_STATUS_STARTED  &&
       !(p->eStatus == PLAYER_STATUS_PAUSED && p->firstFrameStatus != FIRST_FRAME_READY))
    {
        loge("invalid pause operation, component not in started status.");
        if (msg->result)
            *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    p->eStatus = PLAYER_STATUS_PAUSED;
    if(p->firstFrameStatus != FIRST_FRAME_READY)
        BaseCompContinue(&p->base);

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);
}

static void handleQuit(AwMessage *msg, void *arg)
{
    VideoDecComp *p = arg;

    ResetVideoDecoder(p->pDecoder);
    p->eStatus = PLAYER_STATUS_STOPPED;

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);
    pthread_exit(NULL);
}

static void handleReset(AwMessage *msg, void *arg)
{
    VideoDecComp *p = arg;

    p->bEosFlag = 0;
    p->firstFrameStatus = FIRST_FRAME_NOT_READY;
    ResetVideoDecoder(p->pDecoder);

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);

    //* send a message to continue the thread.
    if(p->eStatus != PLAYER_STATUS_STOPPED)
        BaseCompContinue(&p->base);
}

static void handleSetEos(AwMessage *msg, void *arg)
{
    VideoDecComp *p = arg;

    p->bEosFlag = 1;
    sem_post(msg->replySem);

    if(p->bCrashFlag == 0)
    {
        //* send a message to continue the thread.
        if(p->eStatus == PLAYER_STATUS_STARTED ||
           (p->eStatus == PLAYER_STATUS_PAUSED && p->firstFrameStatus != FIRST_FRAME_READY))
            BaseCompContinue(&p->base);
    }
    else
    {
        logv("video decoder notify eos.");
        p->callback(p->pUserData, PLAYER_VIDEO_DECODER_NOTIFY_EOS, NULL);
    }
}

static void doDecode(AwMessage *msg, void *arg)
{
    VideoDecComp *p = arg;
    (void)msg;

    if(p->eStatus != PLAYER_STATUS_STARTED &&
       !(p->eStatus == PLAYER_STATUS_PAUSED && p->firstFrameStatus != FIRST_FRAME_READY))
    {
        loge("not in started status, ignore decode message.");
        return;
    }

    if(p->bCrashFlag)
    {
        logw("video decoder has already crashed,  MESSAGE_ID_DECODE will not be processe.");
        return;
    }

    int64_t nCurTime = p->pAvTimer->GetTime(p->pAvTimer);

    int ret = DecodeVideoStream(p->pDecoder,
                                p->bEosFlag,
                                p->bConfigDecodeKeyFrameOnly,
                                p->bConfigDropDelayFrames,
                                nCurTime);

    logv("DecodeVideoStream return = %d, p->bCrashFlag(%d)", ret, p->bCrashFlag);

    if(ret == VDECODE_RESULT_NO_BITSTREAM)
    {
        if(p->bEosFlag)
        {
            logv("video decoder notify eos.");
            p->callback(p->pUserData, PLAYER_VIDEO_DECODER_NOTIFY_EOS, NULL);
            return;
        }
        else
        {
            SemTimedWait(&p->streamDataSem, 20);    //* wait for stream data.
            BaseCompContinue(&p->base);
            return;
        }
    }
    else if(ret == VDECODE_RESULT_NO_FRAME_BUFFER)
    {
        SemTimedWait(&p->frameBufferSem, 20);    //* wait for frame buffer.
        BaseCompContinue(&p->base);
        return;
    }
    else if(ret == VDECODE_RESULT_RESOLUTION_CHANGE)
    {
        logv("decode thread detect resolution change.");
        p->bResolutionChange = 1;
        //*for new display, we should callback the message to videoRender
        if(p->videoRenderCallback != NULL)
        {
            int msg = VIDEO_RENDER_RESOLUTION_CHANGE;
            p->videoRenderCallback(p->pVideoRenderUserData, msg,
                    (void*)(&p->bResolutionChange));
        }
        return;
    }
    else if(ret < 0)    //* ret == VDECODE_RESULT_UNSUPPORTED
    {
        logw("video decoder notify crash.");
        p->bCrashFlag = 1;
        p->callback(p->pUserData, PLAYER_VIDEO_DECODER_NOTIFY_CRASH, NULL);
        return;
    }

    if(p->firstFrameStatus != FIRST_FRAME_READY)
    {
        if(ret == VDECODE_RESULT_FRAME_DECODED ||
                ret == VDECODE_RESULT_KEYFRAME_DECODED)
        {
            VideoStreamInfo videoStreamInfo;
            GetVideoStreamInfo(p->pDecoder, &videoStreamInfo);
            if(videoStreamInfo.bIs3DStream)
            {
                if(p->firstFrameStatus == FIRST_3D_FRAME_READY)
                    p->firstFrameStatus = SECOND_3D_FRAME_READY;
                else
                    //* decode one picture of a 3d frame(with two picture).
                    p->firstFrameStatus = FIRST_3D_FRAME_READY;
            }
            else
                p->firstFrameStatus = FIRST_FRAME_READY;
        }
    }

    if(p->eStatus == PLAYER_STATUS_STARTED || p->firstFrameStatus != FIRST_FRAME_READY)
    {
        BaseCompContinue(&p->base);
        return;
    }
}

#if 0
//* for new display callback form decoder
static int CallbackProcess(void* pUserData, int eMessageId, void* param)
{
    logv("DecoderCallback, msg = %d",eMessageId);

    VideoDecComp* p;
    int msg;
    int ret;

    p = (VideoDecComp*)pUserData;

    //* The p->videoRenderCallback maybe is null when decoder callback
    //* message VIDEO_DEC_BUFFER_INFO.
    //* If it happen, we should save it and send it to VideoRender next time.
    pthread_mutex_lock(&p->videoRenderCallbackMutex);

    if(p->videoRenderCallback == NULL)
    {
        if(eMessageId == VIDEO_DEC_BUFFER_INFO)
        {
            FbmBufInfo* pFbmBufInfo = (FbmBufInfo*)param;
            memcpy(&p->mFbmBufInfo, pFbmBufInfo, sizeof(FbmBufInfo));
            p->bFbmBufInfoValidFlag = 1;
            pthread_mutex_unlock(&p->videoRenderCallbackMutex);
            return 0;
        }
        else
        {
            logw("the p->videoRenderCallback is null");
            pthread_mutex_unlock(&p->videoRenderCallbackMutex);
            return -1;
        }
    }

    if(p->bFbmBufInfoValidFlag == 1)
    {
        int msg;
        msg = VIDEO_RENDER_VIDEO_INFO;
        p->videoRenderCallback(p->pVideoRenderUserData,msg,(void*)(&p->mFbmBufInfo));
        p->bFbmBufInfoValidFlag = 0;
    }

    switch(eMessageId)
    {
        case VIDEO_DEC_BUFFER_INFO:
             msg = VIDEO_RENDER_VIDEO_INFO;
             break;
        case VIDEO_DEC_REQUEST_BUFFER:
             msg = VIDEO_RENDER_REQUEST_BUFFER;
             break;
        case VIDEO_DEC_DISPLAYER_BUFFER:
             msg = VIDEO_RENDER_DISPLAYER_BUFFER;
             break;
        case VIDEO_DEC_RETURN_BUFFER:
             msg = VIDEO_RENDER_RETURN_BUFFER;
             break;
        default:
             loge("the callback message is not support! msg = %d",eMessageId);
             pthread_mutex_unlock(&p->videoRenderCallbackMutex);
             return -1;
    }

    ret = p->videoRenderCallback(p->pVideoRenderUserData,msg,param);

    pthread_mutex_unlock(&p->videoRenderCallbackMutex);

    return ret;
}

#endif
