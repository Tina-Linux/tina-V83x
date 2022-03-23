
//#define CONFIG_LOG_LEVEL    OPTION_LOG_LEVEL_DETAIL
//#define LOG_TAG "videoRenderComponent_newDisplay"
#include "log.h"

#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>

#include "videoRenderComponent.h"
#include "messageQueue.h"
#include "layerControl.h"
#include "memoryAdapter.h"
#include <deinterlace.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include <linux/ioctl.h>

#define USE_DETNTERLACE 1

#if(CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX)
#define SEND_PTS_TO_SF	0
#else
#define SEND_PTS_TO_SF	0
#endif

#if(CONFIG_DTV == OPTION_DTV_YES)
#define DTMB_PRODUCT	1
#else
#define DTMB_PRODUCT	0
#endif

extern LayerControlOpsT mNewLayerControlOps;

enum VIDEORENDERRESULT
{
    VIDEO_RENDER_PROCESS_MESSAGE   = 1,
    VIDEO_RENDER_DEINTERLACE_RESET = 2,
	VIDEO_RENDER_DROP_THE_PICTURE  = 3,
	VIDEO_RENDER_THREAD_CONTINUE   = 4,

    VIDEO_RENDER_RESULT_MIN = VDECODE_RESULT_UNSUPPORTED,
    VIDEO_RENDER_RESULT_MAX = VDECODE_RESULT_RESOLUTION_CHANGE,
};

typedef struct VideoRenderCompContext
{
    //* created at initialize time.
    MessageQueue*        mq;
    sem_t                startMsgReplySem;
    sem_t                stopMsgReplySem;
    sem_t                pauseMsgReplySem;
    sem_t                resetMsgReplySem;
    sem_t                eosMsgReplySem;
    sem_t                quitMsgReplySem;
    sem_t                set3DModeReplySem;
    sem_t                setWindowReplySem;
    sem_t                setHideVideoSem;
    sem_t                setHoldLastPictureSem;

    int                  nStartReply;
    int                  nStopReply;
    int                  nPauseReply;
    int                  nResetReply;
    int                  nSet3DModeReply;

    pthread_t            sRenderThread;

    enum EPLAYERSTATUS   eStatus;
    void*                pNativeWindow;
    NewLayerControlOpsT*    mNewLayerOps;
    LayerCtrl*           pLayerCtrl;
    VideoDecComp*        pDecComp;

    enum EPICTURE3DMODE  ePicture3DMode;
    enum EDISPLAY3DMODE  eDisplay3DMode;

    //* objects set by user.
    AvTimer*             pAvTimer;
    PlayerCallback       callback;
    void*                pUserData;
    int                  bEosFlag;
	int                  nRotationAngle;

    //*
    int                  bResolutionChange;

    int                  bHadSetLayerInfoFlag;
    int                  bProtectedBufferFlag;//* 1: mean the video picture is secure

    //* for 3D video stream
    int                  bVideoWithTwoStream;

    //* for deinterlace
    Deinterlace         *di;
    int                  bDeinterlaceFlag;
    FbmBufInfo           mFbmBufInfo;

	//******
	int					 bFirstPictureShowed;
    int                  bNeedResetLayerParams;
	int					 bHideVideo;
    VideoPicture*        pPicture;
    VideoPicture*        pPrePicture;
	int					 bHadGetVideoFbmBufInfoFlag;
    int                  nDeinterlaceDispNum;
	int				     nGpuBufferNum;
	int					 bHadSetBufferToDecoderFlag;
	VideoPicture*		 pCancelPicture[4];

	VideoPicture*		 pDiOutPicture;
	int					 bResetBufToDecoderFlag;
	int				     bHadRequestReleasePicFlag;
	int					 nNeedReleaseBufferNum;

}VideoRenderCompContext;

static void* VideoRenderThread(void* arg);
static void PostRenderMessage(MessageQueue* mq);
static int IsVideoWithTwoStream(VideoDecComp* pDecComp);

static inline void NotifyVideoSizeAndSetDisplayRegion(VideoRenderCompContext* p);

static inline int ProcessVideoSync(VideoRenderCompContext* p,
											 VideoPicture* pPicture,
											 Message*       msg);
static inline int QueueBufferToShow(VideoRenderCompContext* p,
											    VideoPicture* pPicture);
static inline int ProcessDeinterlace(VideoRenderCompContext* p,
											  int			nDeinterlaceTime);
static inline int RenderGetVideoFbmBufInfo(VideoRenderCompContext* p);

static inline int SetGpuBufferToDecoder(VideoRenderCompContext*p);

static inline int ResetBufToDecoder(VideoRenderCompContext*p);

VideoRenderComp* VideoRenderCompCreate(void)
{
    VideoRenderCompContext* p;
    int                     err;

    p = (VideoRenderCompContext*)malloc(sizeof(VideoRenderCompContext));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }
    memset(p, 0, sizeof(VideoRenderCompContext));

    p->mNewLayerOps = __GetNewLayerControlOps();
	p->nDeinterlaceDispNum = 1;
    p->bVideoWithTwoStream = -1;
    p->mq = MessageQueueCreate(4, "VideoRenderMq");
    if(p->mq == NULL)
    {
        loge("video render component create message queue fail.");
        free(p);
        return NULL;
    }

    sem_init(&p->startMsgReplySem, 0, 0);
    sem_init(&p->stopMsgReplySem, 0, 0);
    sem_init(&p->pauseMsgReplySem, 0, 0);
    sem_init(&p->resetMsgReplySem, 0, 0);
    sem_init(&p->eosMsgReplySem, 0, 0);
    sem_init(&p->quitMsgReplySem, 0, 0);
    sem_init(&p->set3DModeReplySem, 0, 0);
    sem_init(&p->setWindowReplySem, 0, 0);
    sem_init(&p->setHideVideoSem, 0, 0);
    sem_init(&p->setHoldLastPictureSem, 0, 0);

    p->eStatus = PLAYER_STATUS_STOPPED;

    p->di = DeinterlaceCreate();
    if (!p->di)
    {
        logw("No deinterlace...");
    }

    err = pthread_create(&p->sRenderThread, NULL, VideoRenderThread, p);
    if(err != 0)
    {
        loge("video render component create thread fail.");
        sem_destroy(&p->startMsgReplySem);
        sem_destroy(&p->stopMsgReplySem);
        sem_destroy(&p->pauseMsgReplySem);
        sem_destroy(&p->resetMsgReplySem);
        sem_destroy(&p->eosMsgReplySem);
        sem_destroy(&p->quitMsgReplySem);
        sem_destroy(&p->set3DModeReplySem);
        sem_destroy(&p->setWindowReplySem);
        sem_destroy(&p->setHideVideoSem);
        sem_destroy(&p->setHoldLastPictureSem);
        MessageQueueDestroy(p->mq);
        if (p->di)
        {
            delete p->di;
		p->di = NULL;
		}
        free(p);
        return NULL;
    }

	if(err != 0)
	{
        loge("(f:%s, l:%d) pthread_mutex_init fail!", __FUNCTION__, __LINE__);
        sem_destroy(&p->startMsgReplySem);
        sem_destroy(&p->stopMsgReplySem);
        sem_destroy(&p->pauseMsgReplySem);
        sem_destroy(&p->resetMsgReplySem);
        sem_destroy(&p->eosMsgReplySem);
        sem_destroy(&p->quitMsgReplySem);
        sem_destroy(&p->set3DModeReplySem);
        sem_destroy(&p->setWindowReplySem);
        sem_destroy(&p->setHideVideoSem);
        sem_destroy(&p->setHoldLastPictureSem);
        MessageQueueDestroy(p->mq);
        free(p);
        return NULL;
	}

    return (VideoRenderComp*)p;
}


int VideoRenderCompDestroy(VideoRenderComp* v)
{
    logd("VideoRenderCompDestroy!!!");
    void*                   status;
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    msg.messageId = MESSAGE_ID_QUIT;
    msg.params[0] = (uintptr_t)&p->quitMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    SemTimedWait(&p->quitMsgReplySem, -1);
    pthread_join(p->sRenderThread, &status);

    sem_destroy(&p->startMsgReplySem);
    sem_destroy(&p->stopMsgReplySem);
    sem_destroy(&p->pauseMsgReplySem);
    sem_destroy(&p->resetMsgReplySem);
    sem_destroy(&p->eosMsgReplySem);
    sem_destroy(&p->quitMsgReplySem);
    sem_destroy(&p->set3DModeReplySem);
    sem_destroy(&p->setWindowReplySem);
    sem_destroy(&p->setHideVideoSem);
    sem_destroy(&p->setHoldLastPictureSem);

    if (p->di)
    {
        delete p->di;
        p->di = NULL;
    }
    MessageQueueDestroy(p->mq);
    free(p);

    return 0;
}


int VideoRenderCompStart(VideoRenderComp* v)
{
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    logv("video render component starting");

    msg.messageId = MESSAGE_ID_START;
    msg.params[0] = (uintptr_t)&p->startMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStartReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->startMsgReplySem, -1) < 0)
    {
        loge("video render component wait for start finish timeout.");
        return -1;
    }

    return p->nStartReply;
}


int VideoRenderCompStop(VideoRenderComp* v)
{
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    logv("video render component stopping");

    msg.messageId = MESSAGE_ID_STOP;
    msg.params[0] = (uintptr_t)&p->stopMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStopReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->stopMsgReplySem, -1) < 0)
    {
        loge("video render component wait for stop finish timeout.");
        return -1;
    }

    return p->nStopReply;
}


int VideoRenderCompPause(VideoRenderComp* v)
{
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    logv("video render component pausing");

    msg.messageId = MESSAGE_ID_PAUSE;
    msg.params[0] = (uintptr_t)&p->pauseMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nPauseReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->pauseMsgReplySem, -1) < 0)
    {
        loge("video render component wait for pause finish timeout.");
        return -1;
    }

    return p->nPauseReply;
}


enum EPLAYERSTATUS VideoRenderCompGetStatus(VideoRenderComp* v)
{
    VideoRenderCompContext* p;
    p = (VideoRenderCompContext*)v;
    return p->eStatus;
}


int VideoRenderCompReset(VideoRenderComp* v)
{
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    logv("video render component reseting");

    msg.messageId = MESSAGE_ID_RESET;
    msg.params[0] = (uintptr_t)&p->resetMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nResetReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->resetMsgReplySem, -1) < 0)
    {
        loge("video render component wait for reset finish timeout.");
        return -1;
    }

    return p->nResetReply;
}


int VideoRenderCompSetEOS(VideoRenderComp* v)
{
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    logv("video render component setting EOS.");

    msg.messageId = MESSAGE_ID_EOS;
    msg.params[0] = (uintptr_t)&p->eosMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->eosMsgReplySem, -1) < 0)
    {
        loge("video render component wait for setting eos finish timeout.");
        return -1;
    }

    return 0;
}


int VideoRenderCompSetCallback(VideoRenderComp* v, PlayerCallback callback, void* pUserData)
{
    VideoRenderCompContext* p;

    p = (VideoRenderCompContext*)v;

    p->callback  = callback;
    p->pUserData = pUserData;

    return 0;
}


int VideoRenderCompSetTimer(VideoRenderComp* v, AvTimer* timer)
{
    VideoRenderCompContext* p;
    p = (VideoRenderCompContext*)v;
    p->pAvTimer  = timer;
    return 0;
}


int VideoRenderCompSetWindow(VideoRenderComp* v, void* pNativeWindow)
{
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    logw("video render component setting window: %p",pNativeWindow);

    msg.messageId = MESSAGE_ID_SET_WINDOW;
    msg.params[0] = (uintptr_t)&p->setWindowReplySem;
    msg.params[1] = 0;
    msg.params[2] = (uintptr_t)pNativeWindow;
    msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->setWindowReplySem, -1) < 0)
    {
        loge("video render component wait for setting window finish timeout.");
        return -1;
    }

    return 0;
}


int VideoRenderCompSetDecodeComp(VideoRenderComp* v, VideoDecComp* d)
{
    VideoRenderCompContext* p;
    p = (VideoRenderCompContext*)v;
    p->pDecComp  = d;

    return 0;
}


int VideoRenderSet3DMode(VideoRenderComp* v,
                         enum EPICTURE3DMODE ePicture3DMode,
                         enum EDISPLAY3DMODE eDisplay3DMode)
{
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    logv("video render component setting 3d mode.");

    msg.messageId = MESSAGE_ID_SET_3D_MODE;
    msg.params[0] = (uintptr_t)&p->set3DModeReplySem;
    msg.params[1] = (uintptr_t)&p->nSet3DModeReply;
    msg.params[2] = (unsigned int)ePicture3DMode;
    msg.params[3] = (unsigned int)eDisplay3DMode;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->set3DModeReplySem, -1) < 0)
    {
        loge("video render component wait for setting 3d mode finish timeout.");
        return -1;
    }

    return p->nSet3DModeReply;
}


int VideoRenderGet3DMode(VideoRenderComp* v,
                         enum EPICTURE3DMODE* ePicture3DMode,
                         enum EDISPLAY3DMODE* eDisplay3DMode)
{
    VideoRenderCompContext* p;
    p = (VideoRenderCompContext*)v;
    *ePicture3DMode = p->ePicture3DMode;
    *eDisplay3DMode = p->eDisplay3DMode;
    return 0;
}


int VideoRenderVideoHide(VideoRenderComp* v, int bHideVideo)
{
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    logv("video render component setting video hide(%d).", bHideVideo);

    msg.messageId = MESSAGE_ID_SET_VIDEO_HIDE;
    msg.params[0] = (uintptr_t)&p->setHideVideoSem;
    msg.params[1] = 0;
    msg.params[2] = bHideVideo;
    msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->setHideVideoSem, -1) < 0)
    {
        loge("video render component wait for setting 3d mode finish timeout.");
        return -1;
    }

    return 0;
}


int VideoRenderSetHoldLastPicture(VideoRenderComp* v, int bHold)
{
    VideoRenderCompContext* p;
    Message                 msg;

    p = (VideoRenderCompContext*)v;

    logv("video render component setting hold last picture(bHold=%d).", bHold);

    msg.messageId = MESSAGE_ID_SET_HOLD_LAST_PICTURE;
    msg.params[0] = (uintptr_t)&p->setHoldLastPictureSem;
    msg.params[1] = 0;
    msg.params[2] = bHold;
    msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, video render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->setHoldLastPictureSem, -1) < 0)
    {
        loge("video render component wait for setting 3d mode finish timeout.");
        return -1;
    }

    return 0;
}

void VideoRenderCompSetProtecedFlag(VideoRenderComp* v, int bProtectedFlag)
{
    VideoRenderCompContext* p;

    p = (VideoRenderCompContext*)v;

    p->bProtectedBufferFlag = bProtectedFlag;

    return ;
}

int VideoRenderCompSetVideoStreamInfo(VideoRenderComp* v, VideoStreamInfo* pStreamInfo)
{
    CEDARX_UNUSE(v);
    CEDARX_UNUSE(pStreamInfo);
    return 0;
}

int VideoRenderCompSetSyncFirstPictureFlag(VideoRenderComp* v, int bSyncFirstPictureFlag)
{
    //*TODO
    v;
	bSyncFirstPictureFlag;
    return 0;
}

int VideoRenderCompSetFrameRateEstimater(VideoRenderComp* v, FramerateEstimater* fe)
{
	//* TODO
    v;
    fe;
    return 0;
}

static void* VideoRenderThread(void* arg)
{
    VideoRenderCompContext* p;
    Message                 msg;
    int                     ret;
    sem_t*                  pReplySem;
    int*                    pReplyValue;
    int64_t                 nCurTime;
    int                     nWaitTime;
    int                     bFirstPtsNotified;

    VideoPicture*           pSecondPictureOf3DMode;
    VideoPicture*           pLayerBuffer;
    VideoPicture*           pSecondLayerBufferOf3DMode;
    int                     bHoldLastPicture;
    VideoPicture*           pPreLayerBuffer;
    int                     nLayerBufferMode;

    p = (VideoRenderCompContext*)arg;
    bFirstPtsNotified          = 0;
    pSecondPictureOf3DMode     = NULL;
    pLayerBuffer               = NULL;
    pSecondLayerBufferOf3DMode = NULL;
    bHoldLastPicture           = 0;
    pPreLayerBuffer            = NULL;
    nLayerBufferMode           = 0;

    while(1)
    {

get_message:

        if(MessageQueueGetMessage(p->mq, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

process_message:
        pReplySem   = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == MESSAGE_ID_START)
        {
            logi("process MESSAGE_ID_START message");
            if(p->eStatus == PLAYER_STATUS_STARTED)
            {
                logw("already in started status.");
                PostRenderMessage(p->mq);
                *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            if(p->eStatus == PLAYER_STATUS_STOPPED)
            {
                p->bFirstPictureShowed = 0;
                bFirstPtsNotified   = 0;
                p->bEosFlag = 0;
            }

            //* send a render message to start decoding.
            PostRenderMessage(p->mq);

            p->eStatus = PLAYER_STATUS_STARTED;
            *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_STOP)
        {
            logi("process MESSAGE_ID_STOP message");
            if(p->eStatus == PLAYER_STATUS_STOPPED)
            {
                logw("already in stopped status.");
                *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            //* return buffers before stop.
            if(p->pPicture != NULL)
            {
                VideoDecCompReturnPicture(p->pDecComp, p->pPicture);
                p->pPicture = NULL;
            }

            if(p->pPrePicture != NULL)
            {
                VideoDecCompReturnPicture(p->pDecComp, p->pPrePicture);
                p->pPrePicture = NULL;
            }

            if(p->pDiOutPicture != NULL)
            {
                if(p->pLayerCtrl != NULL)
                {
                    p->mNewLayerOps->queueBuffer(p->pLayerCtrl, p->pDiOutPicture, 0);
                }
                p->pDiOutPicture = NULL;
            }

            if(p->pLayerCtrl != NULL)
            {
                if(bHoldLastPicture)
                    p->mNewLayerOps->ctrlHoldLastPicture(p->pLayerCtrl, 1);
                else
                {
                    p->mNewLayerOps->ctrlHoldLastPicture(p->pLayerCtrl, 0);
                    if(p->mNewLayerOps->ctrlIsVideoShow(p->pLayerCtrl) == 1)
                        p->mNewLayerOps->ctrlHideVideo(p->pLayerCtrl);
                }
            }

            //* set status to stopped.
            p->eStatus = PLAYER_STATUS_STOPPED;
            *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_PAUSE)
        {
            logi("process MESSAGE_ID_PAUSE message");
            if(p->eStatus != PLAYER_STATUS_STARTED  &&
               !(p->eStatus == PLAYER_STATUS_PAUSED && p->bFirstPictureShowed == 0))
            {
                logw("not in started status, pause operation invalid.");
                *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            //* set status to paused.
            p->eStatus = PLAYER_STATUS_PAUSED;
            if(p->bFirstPictureShowed == 0)
                PostRenderMessage(p->mq);   //* post a decode message to decode the first picture.
            *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_QUIT)
        {
            logi("process MESSAGE_ID_QUIT message");
            //* return buffers before quit.
            if(p->pPicture != NULL)
            {
                VideoDecCompReturnPicture(p->pDecComp, p->pPicture);
                p->pPicture = NULL;
            }

            if(p->pPrePicture != NULL)
            {
                VideoDecCompReturnPicture(p->pDecComp, p->pPrePicture);
                p->pPrePicture = NULL;
            }

            if(p->pDiOutPicture != NULL)
            {
                if(p->pLayerCtrl != NULL)
                {
                    p->mNewLayerOps->queueBuffer(p->pLayerCtrl, p->pDiOutPicture, 0);
                }
                p->pDiOutPicture = NULL;
            }

            sem_post(pReplySem);
            p->eStatus = PLAYER_STATUS_STOPPED;
            break;
        }
        else if(msg.messageId == MESSAGE_ID_RESET)
        {
            logi("process MESSAGE_ID_RESET message");
            //* return buffers before quit.
            if(p->pPicture != NULL)
            {
                VideoDecCompReturnPicture(p->pDecComp, p->pPicture);
                p->pPicture = NULL;
            }

            if(p->pPrePicture != NULL)
            {
                VideoDecCompReturnPicture(p->pDecComp, p->pPrePicture);
                p->pPrePicture = NULL;
            }

            if(p->pDiOutPicture != NULL)
            {
                if(p->pLayerCtrl != NULL)
                {
                    p->mNewLayerOps->queueBuffer(p->pLayerCtrl, p->pDiOutPicture, 0);
                }
                p->pDiOutPicture = NULL;
            }

            //* clear the eos flag.
            p->bEosFlag                 = 0;
            p->bFirstPictureShowed = 0;
            bFirstPtsNotified   = 0;
            *pReplyValue        = 0;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED ||
               (p->eStatus == PLAYER_STATUS_PAUSED && p->bFirstPictureShowed == 0))
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_EOS)
        {
            logi("process MESSAGE_ID_EOS message");
            p->bEosFlag = 1;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED ||
               (p->eStatus == PLAYER_STATUS_PAUSED && p->bFirstPictureShowed == 0))
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_SET_WINDOW)
        {
            logw("process MESSAGE_ID_SET_WINDOW message, p->pPicture(%p)",p->pPicture);
            //* return buffer to old layer.
            if(p->pPicture != NULL)
            {
                VideoDecCompReturnPicture(p->pDecComp, p->pPicture);
                p->pPicture = NULL;
            }

			if(p->pPrePicture != NULL)
            {
                VideoDecCompReturnPicture(p->pDecComp, p->pPrePicture);
                p->pPrePicture = NULL;
            }

            if(p->pDiOutPicture != NULL)
            {
                if(p->pLayerCtrl != NULL)
                {
                    p->mNewLayerOps->queueBuffer(p->pLayerCtrl, p->pDiOutPicture, 0);
                }
                p->pDiOutPicture = NULL;
            }

            p->pNativeWindow = (void*)msg.params[2];

            if(p->pLayerCtrl != NULL)
            {
                //* On the new-displayer of android, we not call LayerRelease,
                //* just reset nativeWindow.
#if(CONFIG_OS == OPTION_OS_ANDROID)

		VideoPicture* pPicture = NULL;
				int nWhileNum = 0;
				while(1)
		{
			nWhileNum++;
					if(nWhileNum >= 100)
					{
						loge("get pic node time more than 100, it is wrong");
						break;
					}

			pPicture = p->mNewLayerOps->getBufferOwnedByGpu(p->pLayerCtrl);
			if(pPicture == NULL)
			{
				break;
			}
			VideoDecCompReturnPicture(p->pDecComp, pPicture);
		}

				p->mNewLayerOps->resetNativeWindow(p->pLayerCtrl,p->pNativeWindow);
				VideoDecCompSetVideoFbmBufRelease(p->pDecComp);
				p->bResetBufToDecoderFlag = 1;
				p->nNeedReleaseBufferNum  = p->nGpuBufferNum;

                goto set_nativeWindow_exit;
#else
                p->mNewLayerOps->release(p->pLayerCtrl);
                p->pLayerCtrl = NULL;
#endif
            }


            //* on linux, pNativeWindow == NULL, and the LayerCtrl module will
            //* create a layer to show video picture.
#if CONFIG_OS == OPTION_OS_ANDROID
            if(p->pNativeWindow != NULL)
#endif
            {
                p->pLayerCtrl = p->mNewLayerOps->init(p->pNativeWindow,p->bProtectedBufferFlag);
            }
            p->bNeedResetLayerParams = 1;

            //* we should set layer info here if it hadn't set it
            if(p->bHadSetLayerInfoFlag == 0 && p->mFbmBufInfo.nBufNum != 0)
            {
               enum EPIXELFORMAT eDisplayPixelFormat = PIXEL_FORMAT_DEFAULT;
               FbmBufInfo* pFbmBufInfo = &p->mFbmBufInfo;
                //* we init deinterlace device here
			   if(p->di != NULL && pFbmBufInfo->bProgressiveFlag == 0 && USE_DETNTERLACE && !DTMB_PRODUCT)
			   {
				   if (p->di->init() == 0)
				   {
					   int di_flag = p->di->flag();
					   p->bDeinterlaceFlag   = 1;
					   p->nDeinterlaceDispNum   = (di_flag == DE_INTERLACE_HW) ? 2 : 1;
				   }
				   else
				   {
					   logw(" open deinterlace failed , we not to use deinterlace!");
				   }
			   }

			   if(p->bDeinterlaceFlag == 1)
			   {
                   eDisplayPixelFormat = p->di->expectPixelFormat();
			   }
			   else
			   {
                   eDisplayPixelFormat = (enum EPIXELFORMAT)pFbmBufInfo->ePixelFormat;
			   }

               p->mNewLayerOps->setDisplayPixelFormat(p->pLayerCtrl,(enum EPIXELFORMAT)pFbmBufInfo->ePixelFormat);
               p->mNewLayerOps->setVideoWithTwoStreamFlag(p->pLayerCtrl,p->bVideoWithTwoStream);
               p->mNewLayerOps->setIsSoftDecoderFlag(p->pLayerCtrl, pFbmBufInfo->bIsSoftDecoderFlag);
			   p->mNewLayerOps->setDisplayBufferSize(p->pLayerCtrl, pFbmBufInfo->nBufWidth, pFbmBufInfo->nBufHeight);
               p->nGpuBufferNum = p->mNewLayerOps->setDisplayBufferCount(p->pLayerCtrl, pFbmBufInfo->nBufNum);

			   p->bHadSetLayerInfoFlag    = 1;
            }

set_nativeWindow_exit:

            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED ||
               (p->eStatus == PLAYER_STATUS_PAUSED && p->bFirstPictureShowed == 0))
		{
		PostRenderMessage(p->mq);
		}
        }
        else if(msg.messageId == MESSAGE_ID_SET_3D_MODE)
        {
            logi("process MESSAGE_ID_SET_3D_MODE message");
            p->ePicture3DMode = (enum EPICTURE3DMODE)msg.params[2];
            p->eDisplay3DMode = (enum EDISPLAY3DMODE)msg.params[3];

            //* now , we no need to set 3D mode to nativeWindow , the app will set it
            #if 0
            if(p->pLayerCtrl != NULL)
            {
                LayerSetPicture3DMode(p->pLayerCtrl, (enum EPICTURE3DMODE)msg.params[2]);
                *pReplyValue = LayerSetDisplay3DMode(p->pLayerCtrl, (enum EDISPLAY3DMODE)msg.params[3]);
            }
            else
            {
                logw("window not set yet, can not set 3d mode.");
                *pReplyValue = -1;
            }
            #else
            *pReplyValue = 0;
            #endif

            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED ||
               (p->eStatus == PLAYER_STATUS_PAUSED && p->bFirstPictureShowed == 0))
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_SET_VIDEO_HIDE)
        {
            logi("process MESSAGE_ID_SET_VIDEO_HIDE message");
            p->bHideVideo = msg.params[2];
            if(p->bHideVideo == 1) //* hide video.
            {
                if(p->pLayerCtrl != NULL && p->mNewLayerOps->ctrlIsVideoShow(p->pLayerCtrl) == 1)
                    p->mNewLayerOps->ctrlHideVideo(p->pLayerCtrl);
            }
            else
            {
                if(p->pLayerCtrl != NULL &&
                   p->mNewLayerOps->ctrlIsVideoShow(p->pLayerCtrl) == 0 &&
                   p->bFirstPictureShowed == 1)
                    p->mNewLayerOps->ctrlShowVideo(p->pLayerCtrl);
            }
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED ||
               (p->eStatus == PLAYER_STATUS_PAUSED && p->bFirstPictureShowed == 0))
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_SET_HOLD_LAST_PICTURE)
        {
            logi("process MESSAGE_ID_SET_HOLD_LAST_PICTURE message");
            bHoldLastPicture = msg.params[2];
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED ||
               (p->eStatus == PLAYER_STATUS_PAUSED && p->bFirstPictureShowed == 0))
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_RENDER)
        {

process_render:

            logi("process MESSAGE_ID_RENDER message");

            if(p->eStatus != PLAYER_STATUS_STARTED &&
              !(p->eStatus == PLAYER_STATUS_PAUSED && p->bFirstPictureShowed == 0))
            {
                logw("not in started status, render message ignored.");
                continue;
            }

			//* when nativeWindow change ,we should reset buffer to decoder
			while(p->bResetBufToDecoderFlag == 1)
			{
				ret = ResetBufToDecoder(p);
				if(ret == VIDEO_RENDER_THREAD_CONTINUE)
				{
					ret = MessageQueueTryGetMessage(p->mq, &msg, 5); //* wait for 5ms if no message come.
					if(ret == 0)    //* new message come, quit loop to process.
						goto process_message;

					PostRenderMessage(p->mq);
					goto process_message;
				}
				else
				{
					ret = MessageQueueTryGetMessage(p->mq, &msg, 5); //* wait for 5ms if no message come.
					if(ret == 0)    //* new message come, quit loop to process.
						goto process_message;
				}
			}

			logv(" *** bHadGetVideoFbmBufInfoFlag = %d",p->bHadGetVideoFbmBufInfoFlag);
			//* get video fbm buf info
			while(p->bHadGetVideoFbmBufInfoFlag == 0)
			{
                if(p->bEosFlag == 1)
                {
                    p->callback(p->pUserData, PLAYER_VIDEO_RENDER_NOTIFY_EOS, NULL);
                    goto get_message;
                }

				if(RenderGetVideoFbmBufInfo(p) == 0)
				{
					p->bHadGetVideoFbmBufInfoFlag = 1;
				}
				else
				{
					ret = MessageQueueTryGetMessage(p->mq, &msg, 5); //* wait for 5ms if no message come.
                    if(ret == 0)    //* new message come, quit loop to process.
                        goto process_message;
				}
			}

			logv(" *** bHadSetBufferToDecoderFlag = %d",p->bHadSetBufferToDecoderFlag);
			//* set buffer to decoder
			while(p->bHadSetBufferToDecoderFlag == 0)
			{
				if(p->bHadSetLayerInfoFlag)
				{
					SetGpuBufferToDecoder(p);
					p->bHadSetBufferToDecoderFlag = 1;
				}
				else
				{
					ret = MessageQueueTryGetMessage(p->mq, &msg, 5); //* wait for 5ms if no message come.
                    if(ret == 0)    //* new message come, quit loop to process.
                        goto process_message;
				}
			}

			if(p->pPicture != NULL) //* p->pPicture or pLayerBuffer not NULL.
            {
                //* continue last process.
                if(pLayerBuffer != NULL)
                    goto step_5;
                else if(bFirstPtsNotified == 1)
                    goto step_5;
                else
                    goto step_4;
            }
            else
            {
                //*******************************
                //* 1. get picture from display queue.
                //*******************************
                while(p->pPicture == NULL)
                {
                    p->pPicture = VideoDecCompRequestPicture(p->pDecComp, 0, &p->bResolutionChange);
					logv("** get picture, picture = %p",p->pPicture);
					if(p->pPicture != NULL || (p->pPicture == NULL && p->bEosFlag))
					{
						break;
					}

					if(p->bResolutionChange)
                    {
			//* reopen the video engine.
                        VideoDecCompReopenVideoEngine(p->pDecComp);
                        //* reopen the layer.
                        if(p->pLayerCtrl != NULL)
                        {
                            if(p->bFirstPictureShowed == 1)
                                p->mNewLayerOps->ctrlHoldLastPicture(p->pLayerCtrl, 1);
                            p->mNewLayerOps->release(p->pLayerCtrl);
                            p->pLayerCtrl = NULL;

                            p->pLayerCtrl = p->mNewLayerOps->init(p->pNativeWindow,p->bProtectedBufferFlag);
                            if(p->pLayerCtrl != NULL)
                                p->bNeedResetLayerParams = 1;
                        }
                        p->bResolutionChange		  = 0;
                        p->bHadSetLayerInfoFlag		  = 0;
						p->bHadGetVideoFbmBufInfoFlag = 0;
						p->bHadSetBufferToDecoderFlag = 0;

                        goto process_render;
                    }

                    ret = MessageQueueTryGetMessage(p->mq, &msg, 5); //* wait for 5ms if no message come.
                    if(ret == 0)    //* new message come, quit loop to process.
                        goto process_message;
                }

                //*****************************************************************
                //* 2. handle EOS, p->pPicture should not be NULL except bEosFlag==1.
                //*****************************************************************
                if(p->pPicture == NULL)
                {
                    if(p->bEosFlag == 1)
                    {
                        p->callback(p->pUserData, PLAYER_VIDEO_RENDER_NOTIFY_EOS, NULL);
                        continue;
                    }
                    else
                    {
                        loge("p->pPicture=NULL but bEosFlag is not set, shouldn't run here.");
                        abort();
                    }
                }

                //********************************************
                //* 3. notify video size and set display region
                //********************************************
				if(p->bFirstPictureShowed == 0 || p->bNeedResetLayerParams == 1)
				{
					NotifyVideoSizeAndSetDisplayRegion(p);
					p->bNeedResetLayerParams = 0;
				}
                //************************************************************************************
                //* 4. notify the first sync frame to set timer. the first sync frame is the second
                //*    picture, the first picture need to be showed as soon as we can.(unsynchroized)
                //************************************************************************************
step_4:
                if(p->bFirstPictureShowed != 0 && bFirstPtsNotified == 0)
                {
                    //* this callback may block because the player need wait audio first frame to sync.
                    ret = p->callback(p->pUserData, PLAYER_VIDEO_RENDER_NOTIFY_FIRST_PICTURE, (void*)&p->pPicture->nPts);
                    if(ret == TIMER_DROP_VIDEO_DATA)
                    {
                        //* video first frame pts small (too much) than the audio,
                        //* discard this frame to catch up the audio.
                        VideoDecCompReturnPicture(p->pDecComp, p->pPicture);
                        p->pPicture = NULL;
                        PostRenderMessage(p->mq);
                        continue;
                    }
                    else if(ret == TIMER_NEED_NOTIFY_AGAIN)
                    {
                        //* waiting process for first frame sync with audio is broken by a new message to player, so the player tell us to notify again later.
                        //* post a render message to continue the rendering job after message processed.
                        ret = MessageQueueTryGetMessage(p->mq, &msg, 10); //* wait for 10ms if no message come.
                        if(ret == 0)    //* new message come, quit loop to process.
                            goto process_message;
                        PostRenderMessage(p->mq);
                        continue;
                    }
                    bFirstPtsNotified = 1;
                }

                //******************************************************
                //* 6. sync and show the picture
                //******************************************************
step_5:
                logv("** p->bDeinterlaceFlag[%d]",p->bDeinterlaceFlag);
                if(p->bDeinterlaceFlag == 0)
                {
#if DTMB_PRODUCT
			if(p->bFirstPictureShowed == 0)
			{
				QueueBufferToShow(p, p->pPicture);
                            p->pPicture = NULL;

                            VideoPicture* pReturnPicture = NULL;
                            p->mNewLayerOps->dequeueBuffer(p->pLayerCtrl, &pReturnPicture, 0);
                            VideoDecCompReturnPicture(p->pDecComp, pReturnPicture);
			}
#endif
                    //* 6.1. wait according to the presentation time stamp.
                    if(p->bFirstPictureShowed != 0)    //* the first picture is showed unsychronized.
                    {
                        ret = ProcessVideoSync(p,p->pPicture,&msg);
                        if(ret == VIDEO_RENDER_PROCESS_MESSAGE)
                        {
                            goto process_message;
                        }
                        else if(ret == VIDEO_RENDER_DROP_THE_PICTURE)
                        {
                            VideoDecCompReturnPicture(p->pDecComp, p->pPicture);
                            p->pPicture = NULL;
                        }
                        else
                        {
                            //* 6.2. queue buffer to show.
                            QueueBufferToShow(p, p->pPicture);
                            p->pPicture = NULL;

                            //* 6.3. dequeue buffer from gpu and return it to decoder
                            VideoPicture* pReturnPicture = NULL;
                            p->mNewLayerOps->dequeueBuffer(p->pLayerCtrl, &pReturnPicture, 0);
                            VideoDecCompReturnPicture(p->pDecComp, pReturnPicture);
                        }
                    }
                }
                else
                {
                    for(int nDeinterlaceTime = 0; nDeinterlaceTime < p->nDeinterlaceDispNum; nDeinterlaceTime++)
                    {
                        //*6.1. deinterlace process
						ret = ProcessDeinterlace(p, nDeinterlaceTime);
						if(ret == VIDEO_RENDER_DEINTERLACE_RESET)
						{
							nDeinterlaceTime = 0;
							continue;
						}

                        //*6.2. sync process
                        if(p->bFirstPictureShowed != 0)    //* the first picture is showed unsychronized.
                        {
				ret = ProcessVideoSync(p, p->pDiOutPicture, &msg);
							if(ret == VIDEO_RENDER_PROCESS_MESSAGE)
							{
                                p->mNewLayerOps->queueBuffer(p->pLayerCtrl, p->pDiOutPicture, 0);
                                p->pDiOutPicture = NULL;

                                if(p->pPicture != p->pPrePicture && p->pPrePicture != NULL)
                                {
                                    VideoDecCompReturnPicture(p->pDecComp, p->pPrePicture);
                                }
                                p->pPrePicture         = p->pPicture;
                                p->pPicture            = NULL;
								goto process_message;
							}
							else if(ret == VIDEO_RENDER_DROP_THE_PICTURE)
							{
                                p->mNewLayerOps->queueBuffer(p->pLayerCtrl, p->pDiOutPicture, 0);
                                p->pDiOutPicture = NULL;
								break;
							}
                        }

						//*6.3. queue buffer to show.
						QueueBufferToShow(p, p->pDiOutPicture);
						p->pDiOutPicture = NULL;

                    }

                    //* we return the pic to decoder, no need queue to gpu
                    if(p->pPicture != p->pPrePicture && p->pPrePicture != NULL)
                    {
                        VideoDecCompReturnPicture(p->pDecComp, p->pPrePicture);
                    }
                    p->pPrePicture         = p->pPicture;
                    p->pPicture            = NULL;
                }

                if(p->bFirstPictureShowed == 0)
                    p->bFirstPictureShowed = 1;

                if(p->eStatus == PLAYER_STATUS_STARTED)
                    PostRenderMessage(p->mq);
                else
                {
                    //* p->eStatus == PLAYER_STATUS_PAUSED && bFirstPictureShowed == 0
                    //* need to show the first picture as soon as we can after seek.
                    logi("first picture showed at paused status.");
                }
                continue;
            }

        }
        else
        {
            //* unknown message.
            if(pReplyValue != NULL)
                *pReplyValue = -1;
            if(pReplySem)
                sem_post(pReplySem);
        }
    }

    if(p->pLayerCtrl != NULL)
    {
        p->mNewLayerOps->release(p->pLayerCtrl);
        p->pLayerCtrl = NULL;
    }

    ret = 0;
    pthread_exit(&ret);

    return NULL;
}


static void PostRenderMessage(MessageQueue* mq)
{
    if(MessageQueueGetCount(mq)<=0)
    {
        Message msg;
        msg.messageId = MESSAGE_ID_RENDER;
        msg.params[0] = msg.params[1] = msg.params[2] = msg.params[3] = 0;
        if(MessageQueuePostMessage(mq, &msg) != 0)
        {
            loge("fatal error, video render component post message fail.");
            abort();
        }

        return;
    }
}

static int IsVideoWithTwoStream(VideoDecComp* pDecComp)
{
    VideoStreamInfo videoStreamInfo;
    if(VideoDecCompGetVideoStreamInfo(pDecComp, &videoStreamInfo) == 0)
        return videoStreamInfo.bIs3DStream;
    else
        return -1;
}

static inline void NotifyVideoSizeAndSetDisplayRegion(VideoRenderCompContext* p)
{
    int size[4];

    if((p->pPicture->nRightOffset - p->pPicture->nLeftOffset) > 0 &&
       (p->pPicture->nBottomOffset - p->pPicture->nTopOffset) > 0)
	{
        size[0] = p->pPicture->nRightOffset - p->pPicture->nLeftOffset;
        size[1] = p->pPicture->nBottomOffset - p->pPicture->nTopOffset;
        size[2] = 0;
        size[3] = 0;
        p->callback(p->pUserData, PLAYER_VIDEO_RENDER_NOTIFY_VIDEO_SIZE, (void*)size);

	    size[0] = p->pPicture->nLeftOffset;
	    size[1] = p->pPicture->nTopOffset;
		size[2] = p->pPicture->nRightOffset - p->pPicture->nLeftOffset;
		size[3] = p->pPicture->nBottomOffset - p->pPicture->nTopOffset;
        p->callback(p->pUserData, PLAYER_VIDEO_RENDER_NOTIFY_VIDEO_CROP, (void*)size);

        if(p->pLayerCtrl != NULL)
        {
            p->mNewLayerOps->setDisplayRegion(p->pLayerCtrl,
			              p->pPicture->nLeftOffset,
			              p->pPicture->nTopOffset,
			              p->pPicture->nRightOffset - p->pPicture->nLeftOffset,
			              p->pPicture->nBottomOffset - p->pPicture->nTopOffset);
        }
	}
    else
    {
        logw("the offset of picture is not right, we set bufferWidht and \
              bufferHeight as video size, this is maybe wrong, offset: %d, %d, %d, %d",
              p->pPicture->nLeftOffset,p->pPicture->nRightOffset,
              p->pPicture->nTopOffset,p->pPicture->nBottomOffset);
        size[0] = p->pPicture->nWidth;
        size[1] = p->pPicture->nHeight;
        size[2] = 0;
        size[3] = 0;
        p->callback(p->pUserData, PLAYER_VIDEO_RENDER_NOTIFY_VIDEO_SIZE, (void*)size);

        if(p->pLayerCtrl != NULL)
        {
            p->mNewLayerOps->setDisplayRegion(p->pLayerCtrl,
			              0,
			              0,
			              p->pPicture->nWidth,
			              p->pPicture->nHeight);
        }
    }

	return ;
}

static inline int ProcessVideoSync(VideoRenderCompContext* p,
											 VideoPicture* pPicture,
											 Message*       msg)
{
	int                 nWaitTime;
	int ret;

    nWaitTime = p->callback(p->pUserData, PLAYER_VIDEO_RENDER_NOTIFY_PICTURE_PTS, (void*)&pPicture->nPts);

    if (SEND_PTS_TO_SF == 1)
    {
        if (nWaitTime > 120)
        {
            nWaitTime -= 90;
        }
        else if(nWaitTime >= 0)
        {
            nWaitTime = 0;
        }
    }
    logv("nWaitTime=%d", nWaitTime);

    if(nWaitTime > 0)
    {
        int nWaitTimeOnce;
        while(nWaitTime > 0)
        {
            //* wait for 100ms if no message come.
            nWaitTimeOnce = (nWaitTime>100 ? 100 : nWaitTime);
            ret = MessageQueueTryGetMessage(p->mq, msg, nWaitTimeOnce);
            if(ret == 0)    //* new message come, quit loop to process.
            {
                logw("**** detect message coming, nWaitTime = %d",nWaitTime);
                //* if pLayerCtrl==null, we should return picture before process message
                //* or the picture will never be returned.
                #if 0
                if(p->pLayerCtrl == NULL)
                {
                    VideoDecCompReturnPicture(p->pDecComp, p->pPicture);
                    p->pPicture = NULL;
                    pLayerBuffer = NULL;
                }
                #endif
				return VIDEO_RENDER_PROCESS_MESSAGE;
            }
            nWaitTime -= nWaitTimeOnce;
        }
    }
	else if(nWaitTime < -10 && p->bDeinterlaceFlag == 1)
    {
		//* if it is deinterlace and expired, we should drop it
		return VIDEO_RENDER_DROP_THE_PICTURE;
    }
#if (CONFIG_PRODUCT == OPTION_PRODUCT_TVBOX)
    else if(nWaitTime < -100)
    {
		 int nDispFPS = p->mNewLayerOps->getDisplayFPS(p->pLayerCtrl);
		 if(nDispFPS <= 30)
		 {
		/* when disp in 24fps/30fps, SurfaceFlinger could not drop-frame correctly when video framerate larger than 2x */
		/* so we drop it here */
		logd("drop frame nWaitTime=%d, nDispFPS=%d, nFrameRate=%d.", nWaitTime, nDispFPS, pPicture->nFrameRate);
		return VIDEO_RENDER_DROP_THE_PICTURE;
		}
    }
#endif
	return 0;
}

static inline int QueueBufferToShow(VideoRenderCompContext* p,
											    VideoPicture* pPicture)
{
	if(p->pLayerCtrl != NULL)
    {
        if (SEND_PTS_TO_SF == 1)
        {
            int64_t ptsAbs = p->pAvTimer->PtsToSystemTime(p->pAvTimer, pPicture->nPts);
            p->mNewLayerOps->setBufferTimeStamp(p->pLayerCtrl, ptsAbs);
        }
	p->mNewLayerOps->queueBuffer(p->pLayerCtrl, pPicture, 1);
    }
    else
    {
        //* if pLayerCtrl==null , we should return picture to decoder immediately
        #if 0
        VideoDecCompReturnPicture(p->pDecComp, p->pPicture);
        p->pPicture = NULL;
        pLayerBuffer = NULL;
        #endif
    }

    if(p->pLayerCtrl != NULL &&
       p->mNewLayerOps->ctrlIsVideoShow(p->pLayerCtrl) == 0 &&
       p->bHideVideo == 0)
    {
        p->mNewLayerOps->ctrlShowVideo(p->pLayerCtrl);
    }

	return 0;
}

static inline int ProcessDeinterlace(VideoRenderCompContext* p,
											  int			nDeinterlaceTime)
{
	//* deinterlace process
	int ret = -1;

    if(p->pPrePicture == NULL)
    {
        p->pPrePicture          = p->pPicture;
    }
	ret = p->mNewLayerOps->dequeueBuffer(p->pLayerCtrl, &p->pDiOutPicture, 0);
	if(ret != 0)
	{
		loge("** dequeue buffer failed when process deinterlace");
		return -1;
	}

	int diret = p->di->process(p->pPrePicture,
					  p->pPicture,
					  p->pDiOutPicture,
					  nDeinterlaceTime);
	if (diret != 0)
	{
		p->di->reset();

        VideoDecCompReturnPicture(p->pDecComp, p->pPrePicture);
		p->mNewLayerOps->queueBuffer(p->pLayerCtrl, p->pDiOutPicture, 0);
		p->pPrePicture = NULL;
		p->pDiOutPicture = NULL;
		return VIDEO_RENDER_DEINTERLACE_RESET;
	}
	return 0;
}

static inline int RenderGetVideoFbmBufInfo(VideoRenderCompContext* p)
{
    enum EPIXELFORMAT eDisplayPixelFormat = PIXEL_FORMAT_DEFAULT;
	FbmBufInfo* pFbmBufInfo =  VideoDecCompGetVideoFbmBufInfo(p->pDecComp);

	logv("pFbmBufInfo = %p",pFbmBufInfo);

	if(pFbmBufInfo != NULL)
	{
		memcpy(&p->mFbmBufInfo, pFbmBufInfo, sizeof(FbmBufInfo));
		 //* We check whether it is a 3D stream here,
        //* because Layer must know whether it 3D stream at the beginning;
        p->bVideoWithTwoStream = IsVideoWithTwoStream(p->pDecComp);

        if(p->bVideoWithTwoStream == -1)
             p->bVideoWithTwoStream = 0;

        if(p->pLayerCtrl != NULL)
        {
            logd("video buffer info: nWidth[%d],nHeight[%d],nBufferCount[%d],ePixelFormat[%d]",
                  pFbmBufInfo->nBufWidth,pFbmBufInfo->nBufHeight,
                  pFbmBufInfo->nBufNum,pFbmBufInfo->ePixelFormat);
            logd("video buffer info: nAlignValue[%d],bProgressiveFlag[%d],bIsSoftDecoderFlag[%d]",
                  pFbmBufInfo->nAlignValue,pFbmBufInfo->bProgressiveFlag,
                  pFbmBufInfo->bIsSoftDecoderFlag);

            //* we init deinterlace device here
            if(p->di != NULL && pFbmBufInfo->bProgressiveFlag == 0 && USE_DETNTERLACE && !DTMB_PRODUCT)
            {
                if (p->di->init() == 0)
                {
                    int di_flag = p->di->flag();
                    p->bDeinterlaceFlag   = 1;
                    p->nDeinterlaceDispNum   = (di_flag == DE_INTERLACE_HW) ? 2 : 1;
                }
                else
                {
                    logw(" open deinterlace failed , we not to use deinterlace!");
                }
            }

            if(p->bDeinterlaceFlag == 1)
            {
                eDisplayPixelFormat = p->di->expectPixelFormat();
            }
            else
            {
                eDisplayPixelFormat = (enum EPIXELFORMAT)pFbmBufInfo->ePixelFormat;
            }

            p->mNewLayerOps->setDisplayPixelFormat(p->pLayerCtrl,eDisplayPixelFormat);
            p->mNewLayerOps->setDisplayBufferSize(p->pLayerCtrl, pFbmBufInfo->nBufWidth, pFbmBufInfo->nBufHeight);
            p->mNewLayerOps->setVideoWithTwoStreamFlag(p->pLayerCtrl,p->bVideoWithTwoStream);
            p->mNewLayerOps->setIsSoftDecoderFlag(p->pLayerCtrl, pFbmBufInfo->bIsSoftDecoderFlag);
            p->nGpuBufferNum = p->mNewLayerOps->setDisplayBufferCount(p->pLayerCtrl, pFbmBufInfo->nBufNum);

            p->bHadSetLayerInfoFlag  = 1;
        }

		return 0;
	}
	return -1;
}

static inline int SetGpuBufferToDecoder(VideoRenderCompContext*p)
{
	VideoPicture mTmpVideoPicture;
	VideoPicture* pTmpVideoPicture = &mTmpVideoPicture;
	int nLayerBufferNum = p->mNewLayerOps->getBufferNumHoldByGpu(p->pLayerCtrl);
	memset(pTmpVideoPicture, 0, sizeof(VideoPicture));
	for(int i = 0; i< p->nGpuBufferNum; i++)
	{
		int ret = p->mNewLayerOps->dequeueBuffer(p->pLayerCtrl, &pTmpVideoPicture, 1);
		if(ret == 0)
		{
			if (i >= p->nGpuBufferNum - nLayerBufferNum)
			{
				p->pCancelPicture[i-(p->nGpuBufferNum-nLayerBufferNum)] = VideoDecCompSetVideoFbmBufAddress(p->pDecComp, pTmpVideoPicture, 1);
			}
			else
			{
				VideoDecCompSetVideoFbmBufAddress(p->pDecComp, pTmpVideoPicture, 0);
			}
		}
		else
		{
			loge("*** dequeue buffer failed when set-buffer-to-decoder");
			abort();
		}
	}

	for (int i = 0; i < nLayerBufferNum; ++i)
	{
		p->mNewLayerOps->queueBuffer(p->pLayerCtrl, p->pCancelPicture[i], 0);
	}
	return 0;
}

static inline int ResetBufToDecoder(VideoRenderCompContext*p)
{
	int ret = 0;
	VideoPicture* pReleasePicture = NULL;

	if(p->bHadRequestReleasePicFlag == 0)
	{
		pReleasePicture = VideoDecCompRequestReleasePicture(p->pDecComp);

		logv("*** pReleasePicture(%p),nNeedReleaseBufferNum(%d)",
			 pReleasePicture,p->nNeedReleaseBufferNum);

		if(pReleasePicture != NULL)
		{
			p->mNewLayerOps->releaseBuffer(p->pLayerCtrl, pReleasePicture);
		}
		else
		{
			//* we drop the picture here, or the decoder will block and
			//* can not return all the ReleasePicture
			VideoPicture* pRequestPic = NULL;
			int nResolutionChange = 0;
			pRequestPic = VideoDecCompRequestPicture(p->pDecComp, 0, &nResolutionChange);
			if(pRequestPic != NULL)
			{
				VideoDecCompReturnPicture(p->pDecComp, pRequestPic);
			}
		}
	}

	if(p->bHadRequestReleasePicFlag == 1 || pReleasePicture != NULL)
	{
		VideoPicture mTmpReturnPicture;
		memset(&mTmpReturnPicture, 0, sizeof(VideoPicture));
		VideoPicture* pTmpReturnPicture = &mTmpReturnPicture;
		ret = p->mNewLayerOps->dequeueBuffer(p->pLayerCtrl, &pTmpReturnPicture, 1);
		if(ret == 0)
		{
			if(p->nNeedReleaseBufferNum == 2 || p->nNeedReleaseBufferNum == 1)
			{
				pTmpReturnPicture = VideoDecCompReturnRelasePicture(p->pDecComp, pTmpReturnPicture, 1);
				p->mNewLayerOps->queueBuffer(p->pLayerCtrl, pTmpReturnPicture, 0);
			}
			else
			{
				VideoDecCompReturnRelasePicture(p->pDecComp, pTmpReturnPicture, 0);
			}

			if(p->bHadRequestReleasePicFlag == 1)
			{
				p->bHadRequestReleasePicFlag = 0;
			}

			p->nNeedReleaseBufferNum--;

			if(p->nNeedReleaseBufferNum <= 0)
			{
				p->bResetBufToDecoderFlag = 0;
			}
		}
		else
		{
			p->bHadRequestReleasePicFlag = 1;
			return VIDEO_RENDER_THREAD_CONTINUE;
		}
	}
	return 0;
}
