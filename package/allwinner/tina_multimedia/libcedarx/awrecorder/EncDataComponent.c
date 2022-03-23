/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : EncDataComponent.cpp
 * Description : EncDataComponent
 * History :
 *
 */

#define LOG_TAG "EncDataComponent"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>

#include <cdx_log.h>
#include "AwMessageQueue.h"
#include "EncDataComponent.h"
#include "VideoEncodeComponent.h"

#define SAVE_FRAME (0)

static const int MUXER_STATUS_STARTED     = 1;   //* parsing and sending data.
static const int MUXER_STATUS_PAUSED      = 1<<1;   //* sending job paused.
static const int MUXER_STATUS_STOPPED     = 1<<2;   //* parser closed.

//* command id in muxer thread
static const int MUXER_COMMAND_MUX           = 0x101;
static const int MUXER_COMMAND_START         = 0x102;
static const int MUXER_COMMAND_STOP          = 0x103;
static const int MUXER_COMMAND_PAUSE         = 0x104;
static const int MUXER_COMMAND_RESET         = 0x105;
static const int MUXER_COMMAND_QUIT          = 0x106;

struct AwMessage {
    AWMESSAGE_COMMON_MEMBERS
    uintptr_t params[4];
};

typedef struct EncDataCompContext
{
    AudioEncodeComp*        pAudioEncComp;
    VideoEncodeComp*        pVideoEncComp;
    AwMessageQueue*           mMessageQueue;
    pthread_t                mThreadId;
    int                     mThreadCreated;

    AwMessageQueue*           mAudioMessageQueue;
    pthread_t                mAudioThreadId;
    int                     mAudioThreadCreated;
    int                       mAudioStatus;
    int                       mStatus;
    MuxerCallback             mCallback;
    void*                   mUserData;

    AudioEncOutBuffer      pAudioOutBuf;
    VencOutputBuffer       pVideoOutBuf;

    VideoEncodeConfig*     pVideoConfig;
    AudioEncodeConfig*     pAudioConfig;

    int                     mGetAudioFrame;
    int                     mGetVideoFrame;

    sem_t                   mSemStart;
    sem_t                   mSemStop;
    sem_t                   mSemPause;
    sem_t                   mSemReset;
    sem_t                   mSemQuit;

    int                     mStartReply;
    int                     mStopReply;
    int                     mResetReply;
    int                     mPauseReply;

    sem_t                   mAudioSemStart;
    sem_t                   mAudioSemStop;
    sem_t                   mAudioSemPause;
    sem_t                   mAudioSemReset;
    sem_t                   mAudioSemQuit;

    int                     mAudioStartReply;
    int                     mAudioStopReply;
    int                     mAudioResetReply;
    int                     mAudioPauseReply;

    void *                  mApp;
    EncDataCallBackOps*     mEncDataCallBackOps;

#if SAVE_FRAME
    FILE                    *pVideoFile;
#endif
}EncDataCompContext;

static void PostMuxerMessage(AwMessageQueue* mq)
{
    if(AwMessageQueueGetCount(mq)<=0)
    {
        AwMessage msg;
        msg.messageId = MUXER_COMMAND_MUX;
        msg.params[0] = msg.params[1] = msg.params[2] = msg.params[3] = 0;
        if(AwMessageQueuePostMessage(mq, &msg) != 0)
        {
            loge("fatal error, audio decode component post message fail.");
            abort();
        }

        return;
    }
}

static void* AudioEncDataThread(void* arg)
{
    EncDataCompContext *p = (EncDataCompContext*)arg;
    AwMessage             msg;
    sem_t*                 pReplySem;
    int*                    pReplyValue;
    CdxMuxerPacketT          packet;
    int ret;

    while(1)
    {
        //logd("==== AudioEncDataThread");
        if(AwMessageQueueGetMessage(p->mAudioMessageQueue, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

        pReplySem    = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == MUXER_COMMAND_START)
        {
            if(p->mAudioStatus == MUXER_STATUS_STARTED)
            {
                loge("invalid start operation, already in started status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            if(p->mAudioStatus == MUXER_STATUS_PAUSED)
            {
                PostMuxerMessage(p->mAudioMessageQueue);
                p->mAudioStatus = MUXER_STATUS_STARTED;
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            PostMuxerMessage(p->mAudioMessageQueue);
            // reset encoder ******
            p->mAudioStatus = MUXER_STATUS_STARTED;
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end ENC_COMMAND_START.
        else if(msg.messageId == MUXER_COMMAND_MUX)
        {
            //logd("=========== audio mux thread");
            //* 1. get the audio
            if(!p->mGetAudioFrame && p->pAudioConfig)
            {
                p->mGetAudioFrame = 1;
                ret = AudioEncodeCompRequestBuffer(p->pAudioEncComp, &p->pAudioOutBuf);
                if(ret < 0)
                {
                    logv(" request audio encode frame failed");
                    p->mGetAudioFrame = 0;
                }
            }

            if(!p->mGetAudioFrame)
            {
                //loge("cannot request video and audio frame");
                usleep(10*1000);
                PostMuxerMessage(p->mAudioMessageQueue);
                continue;
            }

            //* 2. choose a buf send to muxer
            packet.buflen = p->pAudioOutBuf.len;
            packet.length = p->pAudioOutBuf.len;
            packet.buf = p->pAudioOutBuf.pBuf;
            packet.pts = p->pAudioOutBuf.pts;
            packet.type = CDX_MEDIA_AUDIO;
            packet.streamIndex = 1;
            if(p->pAudioConfig->nInSamplerate > 0)
                packet.duration = 1024000 / p->pAudioConfig->nInSamplerate;
            else
                packet.duration = 23;

            //logd("audio packet length(%d)", packet.length);
            if (p->mEncDataCallBackOps && p->mEncDataCallBackOps->onAudioDataEnc)
                p->mEncDataCallBackOps->onAudioDataEnc(p->mApp,&packet);

            // set audioBuf to NULL
            AudioEncodeCompReturnBuffer(p->pAudioEncComp, &p->pAudioOutBuf);
            p->mGetAudioFrame = 0;

            PostMuxerMessage(p->mAudioMessageQueue);
            continue;
        } //* end ENC_COMMAND_ENCODE.
        else if(msg.messageId == MUXER_COMMAND_STOP)
        {
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;    //* break the thread.
        } //* end ENC_COMMAND_STOP.
        else if(msg.messageId == MUXER_COMMAND_PAUSE)
        {
            if(p->mAudioStatus == MUXER_STATUS_PAUSED)
            {
                loge("invalid pause operation, already in pause status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }
            p->mAudioStatus = MUXER_STATUS_PAUSED;
            PostMuxerMessage(p->mAudioMessageQueue);

            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
            continue;
        } //* end ENC_COMMAND_PAUSE.
        else if(msg.messageId == MUXER_COMMAND_RESET)
        {
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == MUXER_COMMAND_QUIT)
        {
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            break;    //* break the thread.
        }
        else
        {
            logw("unknow message with id %d, ignore.", msg.messageId);
        }
    }

    return NULL;
}

static void* EncDataThread(void* arg)
{
    EncDataCompContext *p = (EncDataCompContext*)arg;
    AwMessage             msg;
    sem_t*                 pReplySem;
    int*                    pReplyValue;
    CdxMuxerPacketT          packet;
    int ret;

    while(1)
    {
        if(AwMessageQueueGetMessage(p->mMessageQueue, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

        pReplySem    = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == MUXER_COMMAND_START)
        {
            if(p->mStatus == MUXER_STATUS_STARTED)
            {
                loge("invalid start operation, already in started status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            if(p->mStatus == MUXER_STATUS_PAUSED)
            {
                PostMuxerMessage(p->mMessageQueue);
                p->mStatus = MUXER_STATUS_STARTED;
                if(pReplyValue != NULL)
                    *pReplyValue = (int)0;
                if(pReplySem != NULL)
                    sem_post(pReplySem);
                continue;
            }

            PostMuxerMessage(p->mMessageQueue);
            // reset encoder ******
            p->mStatus = MUXER_STATUS_STARTED;
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;
        } //* end ENC_COMMAND_START.
        else if(msg.messageId == MUXER_COMMAND_MUX)
        {
            //* 1. get the video out buf;
            if(!p->mGetVideoFrame && p->pVideoConfig)
            {
                p->mGetVideoFrame = 1;
                ret = VideoEncodeCompRequestVideoFrame(p->pVideoEncComp, &p->pVideoOutBuf);
                if(ret < 0)
                {
                    //loge("request video encode frame failed");
                    p->mGetVideoFrame = 0;
                }
            }

            if(!p->mGetVideoFrame)
            {
                //loge("cannot request video and audio frame");
                usleep(1*1000);
                PostMuxerMessage(p->mMessageQueue);
                continue;
            }

            packet.buflen = p->pVideoOutBuf.nSize0 + p->pVideoOutBuf.nSize1;
            packet.length = packet.buflen;

            if(p->pVideoOutBuf.nSize1 > 0)
            {
                packet.buf = malloc(packet.buflen);
                memcpy(packet.buf, p->pVideoOutBuf.pData0, p->pVideoOutBuf.nSize0);
                memcpy((char*)packet.buf + p->pVideoOutBuf.nSize0,
                        p->pVideoOutBuf.pData1, p->pVideoOutBuf.nSize1);
            }
            else
            {
                packet.buf = p->pVideoOutBuf.pData0;
            }
            packet.pts = p->pVideoOutBuf.nPts;
            packet.type = CDX_MEDIA_VIDEO;
            packet.streamIndex = 0;
            packet.duration = 1000 / p->pVideoConfig->nFrameRate ;// 33;
            packet.keyFrameFlag = p->pVideoOutBuf.nFlag;

            if (p->mEncDataCallBackOps && p->mEncDataCallBackOps->onVideoDataEnc)
                 p->mEncDataCallBackOps->onVideoDataEnc(p->mApp,&packet);

            #if SAVE_FRAME
            if(fwrite(packet.buf, 1, packet.buflen, p->pVideoFile) < (unsigned int)packet.buflen)
            {
                loge("+++++++++++++++ fwrite err(%d)", errno);
            }
            #endif

            VideoEncodeCompReturnVideoFrame(p->pVideoEncComp, &p->pVideoOutBuf);
            p->mGetVideoFrame = 0;

            if(p->pVideoOutBuf.nSize1 > 0)
                free(packet.buf);

            PostMuxerMessage(p->mMessageQueue);
            continue;
        } //* end ENC_COMMAND_ENCODE.
        else if(msg.messageId == MUXER_COMMAND_STOP)
        {
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            continue;    //* break the thread.
        } //* end ENC_COMMAND_STOP.
        else if(msg.messageId == MUXER_COMMAND_PAUSE)
        {
            if(p->mStatus == MUXER_STATUS_PAUSED)
            {
                loge("invalid pause operation, already in pause status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }
            p->mStatus = MUXER_STATUS_PAUSED;
            PostMuxerMessage(p->mMessageQueue);

            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
            continue;
        } //* end ENC_COMMAND_PAUSE.
        else if(msg.messageId == MUXER_COMMAND_RESET)
        {
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
            continue;
        }
        else if(msg.messageId == MUXER_COMMAND_QUIT)
        {
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
                sem_post(pReplySem);
            break;    //* break the thread.
        }
        else
        {
            logw("unknow message with id %d, ignore.", msg.messageId);
        }
    }

    return NULL;
}

EncDataComp* EncDataCompCreate(void * app)
{
    EncDataCompContext *p;
    int ret;

    p = (EncDataCompContext*)malloc(sizeof(EncDataCompContext));
    if(NULL == p)
    {
        loge("malloc failed ");
        return NULL;
    }
    memset(p, 0x00, sizeof(EncDataCompContext));

    p->mApp = app;

    p->mMessageQueue = AwMessageQueueCreate(4, "EncDataComp");
    if(NULL == p->mMessageQueue)
    {
        loge("create message queue failed");
        free(p);
        return NULL;
    }

    sem_init(&p->mSemStart, 0, 0);
    sem_init(&p->mSemStop, 0, 0);
    sem_init(&p->mSemReset, 0, 0);
    sem_init(&p->mSemPause, 0, 0);
    sem_init(&p->mSemQuit, 0, 0);
    ret = pthread_create(&p->mThreadId, NULL, EncDataThread, p);
    if(ret != 0)
    {
        p->mThreadCreated = 0;
        AwMessageQueueDestroy(p->mMessageQueue);
        sem_destroy(&p->mSemStart);
        sem_destroy(&p->mSemStop);
        sem_destroy(&p->mSemReset);
        sem_destroy(&p->mSemPause);
        sem_destroy(&p->mSemQuit);
        free(p);
        return NULL;
    }
    p->mThreadCreated = 1;

    p->mAudioMessageQueue = AwMessageQueueCreate(4, "AudioEncData");
    if(NULL == p->mAudioMessageQueue)
    {
        loge("create message queue failed");
        AwMessageQueueDestroy(p->mMessageQueue);
        sem_destroy(&p->mSemStart);
        sem_destroy(&p->mSemStop);
        sem_destroy(&p->mSemReset);
        sem_destroy(&p->mSemPause);
        sem_destroy(&p->mSemQuit);
        free(p);
        return NULL;
    }

    sem_init(&p->mAudioSemStart, 0, 0);
    sem_init(&p->mAudioSemStop, 0, 0);
    sem_init(&p->mAudioSemReset, 0, 0);
    sem_init(&p->mAudioSemPause, 0, 0);
    sem_init(&p->mAudioSemQuit, 0, 0);
    ret = pthread_create(&p->mAudioThreadId, NULL, AudioEncDataThread, p);
    if(ret != 0)
    {
        p->mAudioThreadCreated = 0;
        AwMessageQueueDestroy(p->mMessageQueue);
        AwMessageQueueDestroy(p->mAudioMessageQueue);
        sem_destroy(&p->mSemStart);
        sem_destroy(&p->mSemStop);
        sem_destroy(&p->mSemReset);
        sem_destroy(&p->mSemPause);
        sem_destroy(&p->mSemQuit);
        sem_destroy(&p->mAudioSemStart);
        sem_destroy(&p->mAudioSemStop);
        sem_destroy(&p->mAudioSemReset);
        sem_destroy(&p->mAudioSemPause);
        sem_destroy(&p->mAudioSemQuit);
        free(p);
        return NULL;
    }
    p->mAudioThreadCreated = 1;

    #if SAVE_FRAME
    p->pVideoFile = fopen("/mnt/sdcard/save.es", "wb");
    if(!p->pVideoFile)
    {
        loge("open file failed");
    }
    #endif

    return (EncDataComp*)p;
}

int EncDataCompInit(EncDataComp* v,
                            VideoEncodeConfig* videoConfig,
                            AudioEncodeConfig* audioConfig,
                            EncDataCallBackOps *ops)
{
    EncDataCompContext *p = (EncDataCompContext*)v;

    p->mEncDataCallBackOps = ops;

    p->pVideoConfig = videoConfig;
    p->pAudioConfig = audioConfig;

    return 0;
}

void EncDataCompDestory(EncDataComp * v)
{
    EncDataCompContext *p = (EncDataCompContext*)v;
    AwMessage msg;

    if(!p)
    {
        return ;
    }

    if(p->mThreadCreated)
    {
        void* status;

        //* send a quit message to quit the main thread.
        memset(&msg, 0, sizeof(AwMessage));
        msg.messageId = MUXER_COMMAND_QUIT;
        msg.params[0] = (uintptr_t)&p->mSemQuit;
        msg.params[1] = msg.params[2] = msg.params[3] = 0;

        AwMessageQueuePostMessage(p->mMessageQueue, &msg);
        SemTimedWait(&p->mSemQuit, -1);
        pthread_join(p->mThreadId, &status);
    }

    if(p->mAudioThreadCreated)
    {
        void* status;

        //* send a quit message to quit the main thread.
        memset(&msg, 0, sizeof(AwMessage));
        msg.messageId = MUXER_COMMAND_QUIT;
        msg.params[0] = (uintptr_t)&p->mAudioSemQuit;
        msg.params[1] = msg.params[2] = msg.params[3] = 0;
        AwMessageQueuePostMessage(p->mAudioMessageQueue, &msg);
        SemTimedWait(&p->mAudioSemQuit, -1);
        pthread_join(p->mAudioThreadId, &status);
    }

    sem_destroy(&p->mSemStart);
    sem_destroy(&p->mSemStop);
    sem_destroy(&p->mSemReset);
    sem_destroy(&p->mSemPause);
    sem_destroy(&p->mSemQuit);

    sem_destroy(&p->mAudioSemStart);
    sem_destroy(&p->mAudioSemStop);
    sem_destroy(&p->mAudioSemReset);
    sem_destroy(&p->mAudioSemPause);
    sem_destroy(&p->mAudioSemQuit);

    AwMessageQueueDestroy(p->mMessageQueue);
    AwMessageQueueDestroy(p->mAudioMessageQueue);

    #if SAVE_FRAME
    fclose(p->pVideoFile);
    #endif

    free(p);
}

int EncDataCompStart(EncDataComp *v)
{
    EncDataCompContext *p ;
    AwMessage msg;

    p = (EncDataCompContext*)v;

    logd("start");

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = MUXER_COMMAND_START;
    msg.params[0] = (uintptr_t)&p->mSemStart;
    msg.params[1] = (uintptr_t)&p->mStartReply;
    msg.params[2] = msg.params[3] = 0;
    AwMessageQueuePostMessage(p->mMessageQueue, &msg);
    SemTimedWait(&p->mSemStart, -1);

    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = MUXER_COMMAND_START;
    msg.params[0] = (uintptr_t)&p->mAudioSemStart;
    msg.params[1] = (uintptr_t)&p->mAudioStartReply;
    msg.params[2] = msg.params[3] = 0;
    AwMessageQueuePostMessage(p->mAudioMessageQueue, &msg);
    SemTimedWait(&p->mAudioSemStart, -1);

    return (int)p->mStartReply;
}

int EncDataCompStop(EncDataComp *v)
{
    EncDataCompContext *p;
    AwMessage msg;

    p = (EncDataCompContext*)v;
    logd("+++ MuxerCompStop");

    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = MUXER_COMMAND_STOP;
    msg.params[0] = (uintptr_t)&p->mSemStop;
    msg.params[1] = (uintptr_t)&p->mStopReply;
    msg.params[2] = msg.params[3] = 0;
    AwMessageQueuePostMessage(p->mMessageQueue, &msg);
    SemTimedWait(&p->mSemStop, -1);

    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = MUXER_COMMAND_STOP;
    msg.params[0] = (uintptr_t)&p->mAudioSemStop;
    msg.params[1] = (uintptr_t)&p->mAudioStopReply;
    msg.params[2] = msg.params[3] = 0;
    AwMessageQueuePostMessage(p->mAudioMessageQueue, &msg);
    SemTimedWait(&p->mAudioSemStop, -1);

    return (int)p->mStopReply;
}

int EncDataCompReset(EncDataComp *v)
{
    EncDataCompContext *p;
    AwMessage msg;

    p = (EncDataCompContext*)v;
    logd("+++ MuxerCompReset");
    //* send a start message.
    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = MUXER_COMMAND_RESET;
    msg.params[0] = (uintptr_t)&p->mSemReset;
    msg.params[1] = (uintptr_t)&p->mResetReply;
    msg.params[2] = msg.params[3] = 0;
    AwMessageQueuePostMessage(p->mMessageQueue, &msg);
    SemTimedWait(&p->mSemReset, -1);

    memset(&msg, 0, sizeof(AwMessage));
    msg.messageId = MUXER_COMMAND_RESET;
    msg.params[0] = (uintptr_t)&p->mAudioSemReset;
    msg.params[1] = (uintptr_t)&p->mAudioResetReply;
    msg.params[2] = msg.params[3] = 0;
    AwMessageQueuePostMessage(p->mAudioMessageQueue, &msg);
    SemTimedWait(&p->mAudioSemReset, -1);
    return (int)p->mResetReply;
}

int EncDataCompSetAudioEncodeComp(EncDataComp *v, AudioEncodeComp* pAudioEncoder)
{
    EncDataCompContext *p = (EncDataCompContext*)v;

    p->pAudioEncComp = pAudioEncoder;
    return 0;
}

int EncDataCompSetVideoEncodeComp(EncDataComp *v, VideoEncodeComp* pVideoEncoder)
{
    EncDataCompContext *p = (EncDataCompContext*)v;

    p->pVideoEncComp = pVideoEncoder;
    return 0;
}

int EncDataCompSetCallback(EncDataComp* v, MuxerCallback callback, void* pUserData)
{
    EncDataCompContext *p = (EncDataCompContext*)v;

    p->mCallback  = callback;
    p->mUserData = pUserData;
    return 0;
}
