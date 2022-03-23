/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : audioRenderComponent.cpp
 * Description : audio render component
 * History :
 *
 */

//#define CONFIG_LOG_LEVEL LOG_LEVEL_VERBOSE
#define LOG_TAG "audioRender"
#include "cdx_log.h"
#include <stdlib.h>
#include <string.h>
#include "audioRenderComponent.h"
#include "AwMessageQueue.h"
#include "baseComponent.h"
#include "CdxTime.h"
#include <inttypes.h>

/* All local variables on the render thread which are used across different
 * handlers should be put in this struct.
 */
typedef struct RenderThreadCtx RenderThreadCtx;

struct AudioRenderComp {
    //* created at initialize time.
    AwMessageQueue*             mq;
    BaseCompCtx                 base;

    pthread_t                   sRenderThread;

    enum EPLAYERSTATUS          eStatus;
    SoundCtrl*                  pSoundCtrl;
    AudioDecComp*               pDecComp;

    //* objects set by user.
    AvTimer*                    pAvTimer;
    PlayerCallback              callback;
    void*                       pUserData;
    int                         bEosFlag;

    /* audio balance.
     * 1 means left channel, 2 means right channel, 3 means stereo, 0 means
     * default.
     */
    int                         nConfigOutputBalance;
    //* audio mute, 1 means mute the audio.
    int                         bMute;

    int                         bForceWriteToDeviceFlag;

    // raw data configure
    CdxPlaybkCfg                cfg;

    RenderThreadCtx            *threadCtx;
};

static void handleStart(AwMessage *msg, void *arg);
static void handleStop(AwMessage *msg, void *arg);
static void handlePause(AwMessage *msg, void *arg);
static void handleReset(AwMessage *msg, void *arg);
static void handleSetEos(AwMessage *msg, void *arg);
static void handleQuit(AwMessage *msg, void *arg);
static void doRender(AwMessage *msg, void *arg);

static void handleSetAudioSink(AwMessage *msg, void *arg);

static void* AudioRenderThread(void* arg);
static void ProcessBalance(unsigned char* pData, int nDataLen,
        int nBitsPerSample, int nChannelCount, int nOutBalance);
static void ProcessMute(unsigned char* pData, int nDataLen, int bMute);

AudioRenderComp* AudioRenderCompCreate(void)
{
    AudioRenderComp* p;
    int err;

    p = calloc(1, sizeof(*p));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }

    p->mq = AwMessageQueueCreate(4, "AudioRenderMq");
    if(p->mq == NULL)
    {
        loge("audio render component create message queue fail.");
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
        .decode = doRender,
    };

    if (BaseCompInit(&p->base, "audio render", p->mq, &handler))
    {
        AwMessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    p->eStatus = PLAYER_STATUS_STOPPED;

    err = pthread_create(&p->sRenderThread, NULL, AudioRenderThread, p);
    if(err != 0)
    {
        loge("audio render component create thread fail.");
        BaseCompDestroy(&p->base);
        AwMessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    return p;
}

int AudioRenderCompDestroy(AudioRenderComp* p)
{
    BaseCompQuit(&p->base, NULL, NULL);
    pthread_join(p->sRenderThread, NULL);
    BaseCompDestroy(&p->base);

    AwMessageQueueDestroy(p->mq);
    free(p);

    return 0;
}

int AudioRenderCompStart(AudioRenderComp* p)
{
    return BaseCompStart(&p->base, NULL, NULL);
}

int AudioRenderCompStop(AudioRenderComp* p)
{
    return BaseCompStop(&p->base, NULL, NULL);
}

int AudioRenderCompPause(AudioRenderComp* p)
{
    return BaseCompPause(&p->base, NULL, NULL);
}

enum EPLAYERSTATUS AudioRenderCompGetStatus(AudioRenderComp* p)
{
    return p->eStatus;
}

int AudioRenderCompReset(AudioRenderComp* p)
{
    return BaseCompReset(&p->base, 0, NULL, NULL);
}

int AudioRenderCompSetEOS(AudioRenderComp* p)
{
    return BaseCompSetEos(&p->base, NULL, NULL);
}

int AudioRenderCompSetCallback(AudioRenderComp* p, PlayerCallback callback, void* pUserData)
{
    p->callback  = callback;
    p->pUserData = pUserData;

    return 0;
}

int AudioRenderCompSetTimer(AudioRenderComp* p, AvTimer* timer)
{
    p->pAvTimer = timer;
    return 0;
}

int AudioRenderCompSetAudioSink(AudioRenderComp* p, SoundCtrl* pAudioSink)
{
    sem_t replySem;
    sem_init(&replySem, 0, 0);

    AwMessage msg = {
        .messageId = MESSAGE_ID_SET_AUDIO_SINK,
        .replySem = &replySem,
        .opaque = pAudioSink,
        .execute = handleSetAudioSink,
    };

    logd("audio render component setting AudioSink");

    if(AwMessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio render component post message fail.");
        abort();
    }

    if(SemTimedWait(&replySem, -1) < 0)
    {
        loge("audio render component wait for setting window finish failed.");
        sem_destroy(&replySem);
        return -1;
    }

    sem_destroy(&replySem);
    return 0;
}


int AudioRenderCompSetPlaybackRate(AudioRenderComp* p,const XAudioPlaybackRate* rate)
{
    int ret = 0;

    if (rate->mSpeed > 0.f)
        ret = SoundDeviceSetPlaybackRate(p->pSoundCtrl,rate);

    return ret;
}


int AudioRenderCompSetDecodeComp(AudioRenderComp* p, AudioDecComp* d)
{
    p->pDecComp = d;
    return 0;
}

int64_t AudioRenderCompCacheTimeUs(AudioRenderComp* p)
{
    int64_t nCachedTimeUs;

    if(p->pSoundCtrl == NULL)
        return 0;

    nCachedTimeUs = SoundDeviceGetCachedTime(p->pSoundCtrl);
    return nCachedTimeUs;
}

struct RenderThreadCtx {
    int bFirstFrameSend;
    int audioInfoNotified;
    unsigned nSampleRate;
    unsigned nChannelNum;
    unsigned nFrameSize;
    unsigned nBitRate;
    unsigned nBitsPerSample;
    int      needDirectOut;//Direct out put or not
    enum AUIDO_RAW_DATA_TYPE    dataType;//Tpye of data --- pcm or iec61937 or muti pcm

    int64_t nPts;
    uint8_t *pPcmData;
    unsigned nPcmDataLen;
};

CdxPlaybkCfg initPlybkCfg =
{
    .nRoutine = 0,
    .nNeedDirect = 0,
    .nChannels = 2,
    .nSamplerate = 44100,
    .nBitpersample = 16,
    .nDataType = AUDIO_RAW_DATA_PCM,
};

static void* AudioRenderThread(void* arg)
{
    AudioRenderComp *p = arg;
    AwMessage msg;
    RenderThreadCtx threadCtx = {
        .nBitsPerSample = 16,
        .dataType = AUDIO_RAW_DATA_PCM,
    };

    p->threadCtx = &threadCtx;

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
    AudioRenderComp *p = arg;

    logi("audio render process start message.");

    if (p->eStatus == PLAYER_STATUS_STARTED)
    {
        logw("already in started status.");
        *msg->result = -1;
        sem_post(msg->replySem);
        BaseCompContinue(&p->base);
        return;
    }
    else if (p->eStatus == PLAYER_STATUS_STOPPED)
    {
        p->threadCtx->bFirstFrameSend = 0;
        p->threadCtx->nSampleRate     = 0;
        p->threadCtx->nChannelNum     = 0;
        p->threadCtx->nBitsPerSample  = 16;
        p->threadCtx->dataType        = AUDIO_RAW_DATA_PCM;
        p->threadCtx->needDirectOut   = 0;
        p->bEosFlag     = 0;
    }
    else
    {
        //* resume from pause status.
        if(p->pSoundCtrl)
        {
            logd("start sound devide.");
            SoundDeviceStart(p->pSoundCtrl);
        }
    }

    p->eStatus = PLAYER_STATUS_STARTED;
    *msg->result = 0;
    sem_post(msg->replySem);

    BaseCompContinue(&p->base);
}

static void handleStop(AwMessage *msg, void *arg)
{
    AudioRenderComp *p = arg;

    logi("audio render process stop message.");

    if (p->eStatus == PLAYER_STATUS_STOPPED)
    {
        logw("already in stopped status.");
        *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    //* stop the sound device.
    if(p->pSoundCtrl)
    {
        logd("stop sound devide.");
        SoundDeviceStop(p->pSoundCtrl);
    }

    //* set status to stopped.
    p->eStatus = PLAYER_STATUS_STOPPED;
    *msg->result = 0;
    sem_post(msg->replySem);
}

static void handlePause(AwMessage *msg, void *arg)
{
    AudioRenderComp *p = arg;

    logi("audio render process pause message.");

    if (p->eStatus != PLAYER_STATUS_STARTED)
    {
        logw("not in started status, pause operation invalid.");
        *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    //* pause the sound device.
    if(p->pSoundCtrl)
    {
        logd("pause sound devide.");
        SoundDevicePause(p->pSoundCtrl);
    }

    //* set status to paused.
    p->eStatus = PLAYER_STATUS_PAUSED;
    *msg->result = 0;
    sem_post(msg->replySem);
}

static void handleQuit(AwMessage *msg, void *arg)
{
    AudioRenderComp *p = arg;

    logd("audio render process quit message.");
    if(p->pSoundCtrl)
    {
        logd("stop sound devide.");
        SoundDeviceStop(p->pSoundCtrl);
        p->pSoundCtrl = NULL;
    }

    sem_post(msg->replySem);
    p->eStatus = PLAYER_STATUS_STOPPED;
    pthread_exit(NULL);
}

static void handleReset(AwMessage *msg, void *arg)
{
    AudioRenderComp *p = arg;

    logi("audio render process reset message.");

    //* reset the sound device to flush data.
    if(p->pSoundCtrl)
    {
        logd("reset sound devide.");
        SoundDeviceReset(p->pSoundCtrl);
    }

    //* clear the eos flag.
    p->bEosFlag = 0;
    p->threadCtx->bFirstFrameSend = 0;
    *msg->result = 0;
    sem_post(msg->replySem);

    //* send a message to continue the thread.
    if(p->eStatus == PLAYER_STATUS_STARTED)
        BaseCompContinue(&p->base);
}

static void handleSetEos(AwMessage *msg, void *arg)
{
    AudioRenderComp *p = arg;

    logi("audio render process set_eos message.");
    p->bEosFlag = 1;
    sem_post(msg->replySem);


    //* send a message to continue the thread.
    if(p->eStatus == PLAYER_STATUS_STARTED)
        BaseCompContinue(&p->base);
}

static void handleSetAudioSink(AwMessage *msg, void *arg)
{
    AudioRenderComp *p = arg;

    logv("audio render process set_audiosink message.");
    p->pSoundCtrl = msg->opaque;
    sem_post(msg->replySem);

    //* send a message to continue the thread.
    if(p->eStatus == PLAYER_STATUS_STARTED)
        BaseCompContinue(&p->base);
}

static int initSoundDevice(AudioRenderComp *p)
{
    RenderThreadCtx *threadCtx = p->threadCtx;
    int ret;
    CdxPlaybkCfg cfg = initPlybkCfg;

    do {
        ret = AudioDecCompGetAudioSampleRate(p->pDecComp, &threadCtx->nSampleRate,
                                        &threadCtx->nChannelNum, &threadCtx->nBitRate);
        if (ret == -1 || threadCtx->nSampleRate == 0 || threadCtx->nChannelNum == 0)
        {
            //* check whether stream end.
            if(p->bEosFlag)
            {
                p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_EOS, NULL);
                return -1;
            }

            //* get stream info fail, decoder not initialized yet.
            if (AwMessageQueueWaitMessage(p->mq, 10) > 0)
                return -1;
        }
        else
        {
            logd("init sound device.");
            if (p->pSoundCtrl == NULL)
            {
                loge("can not initialize the sound device.");
                abort();
            }

            logd("set sound devide param, sample rate = %d, channel num = %d.",
                    threadCtx->nSampleRate, threadCtx->nChannelNum);
            cfg.nSamplerate = threadCtx->nSampleRate;
            cfg.nChannels   = threadCtx->nChannelNum;
            SoundDeviceSetFormat(p->pSoundCtrl, &cfg);
            threadCtx->nFrameSize = ((threadCtx->nChannelNum * threadCtx->nBitsPerSample)>>3);
            // / 8;
            return 0;
        }
    } while (1);

    return 0;
}

static inline int requestPcmData(AudioRenderComp *p)
{
    RenderThreadCtx *threadCtx = p->threadCtx;
    int ret = 0;

    uint8_t *pPcmData = NULL;;
    unsigned nPcmDataLen;

    do
    {
        //* pcm data of 40ms
        nPcmDataLen = (threadCtx->nSampleRate * 40 / 1000) * threadCtx->nFrameSize;
        pPcmData    = NULL;
        memset(&p->cfg, 0, sizeof(CdxPlaybkCfg));

        ret = AudioDecCompRequestPcmData(p->pDecComp, &pPcmData,
                        &nPcmDataLen, &threadCtx->nPts, &p->cfg);
        logv("audio render, ret = %d, pPcmData = %p, nPcmDataLen=%u, %u",
            ret, pPcmData, nPcmDataLen, threadCtx->nFrameSize);
        if(ret < 0 || pPcmData == NULL || nPcmDataLen < threadCtx->nFrameSize)
        {
            //* check whether stream end.
            if(p->bEosFlag)
            {
                //check we how much cache we have outside cedarx
                int cacheout = SoundDeviceGetCachedTime(p->pSoundCtrl);
                int timeout = cacheout*2/1000;
                int if_gapless_play = 0;
                cdx_int64 starttime = CdxGetNowUs();
                logd("we have %d ms",cacheout/1000);
                int framecount;
                SoundDeviceControl(p->pSoundCtrl, SOUND_CONTROLL_SET_CLBK_EOS, NULL);
                if_gapless_play = SoundDeviceControl(p->pSoundCtrl,
                    SOUND_CONTROLL_QUERY_IF_GAPLESS_PLAY, NULL);

                while(!if_gapless_play)
                {
                    ret = AwMessageQueueWaitMessage(p->mq, 5);
                    if (ret > 0)
                        return -1;

                    cacheout = SoundDeviceGetCachedTime(p->pSoundCtrl);
                    framecount = SoundDeviceGetFrameCount(p->pSoundCtrl);
                    cdx_int64 currenttime = CdxGetNowUs();
                    cdx_int64 passtime = (currenttime - starttime)/1000;
                    logd("we still have %d ms and pass %"PRId64" ms",cacheout/1000,passtime);

                    if(cacheout <= 10*1000){
                        logw("we quit but still have %d ms",cacheout/1000);
                        break;
                    }
                    //to avoid bug when audiotrack no consumed buffer by some unknow issue
                    if (passtime > timeout)
                    {
                        logw("Pass too long, maybe something happen!!");
                        break;
                    }

                }

                if(if_gapless_play)
                {
                    SoundDeviceStop(p->pSoundCtrl);
                }
                p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_EOS, NULL);
                return -1;
            }

            //* if no pcm data, wait some time.
            ret = AwMessageQueueWaitMessage(p->mq, 10);
            if (ret > 0)
                return -1;
        }
        else
        {
            break;  //* get frame success.
        }
    }while(1);

    threadCtx->pPcmData = pPcmData;
    threadCtx->nPcmDataLen = nPcmDataLen;

    return 0;
}

static inline int checkSampleRate(AudioRenderComp *p)
{
    RenderThreadCtx *threadCtx = p->threadCtx;
    unsigned nNewSampleRate = 0;
    unsigned nNewChannelNum = 0;
    unsigned nNewBitRate = 0;
    unsigned nNewBitPerSample = 16;
    int      nNewneedDirectOut = 0;
    enum AUIDO_RAW_DATA_TYPE    nNewDataType  = AUDIO_RAW_DATA_PCM;
    CdxPlaybkCfg PlybkCfg = initPlybkCfg;

    int ret;

    ret = AudioDecCompGetAudioSampleRate(p->pDecComp, &nNewSampleRate,
                                    &nNewChannelNum, &nNewBitRate);
    if (ret != 0)
    {
        loge("get audio sample rate failed");
        return -1;
    }

    if(nNewChannelNum == 1){
      logd("the channel num is 1, maybe something is wrong!");
      return -1;
    }

    if(p->cfg.nNeedDirect != 0)
    {
        //Direct out put start
        PlybkCfg.nBitpersample = nNewBitPerSample = p->cfg.nBitpersample;
        PlybkCfg.nChannels = nNewChannelNum = p->cfg.nChannels;
        PlybkCfg.nSamplerate = nNewSampleRate = p->cfg.nSamplerate;
        PlybkCfg.nDataType = nNewDataType = p->cfg.nDataType;
        PlybkCfg.nNeedDirect = nNewneedDirectOut = 1;
    }
    else//did not need direct out put, mix output
    {
        PlybkCfg.nChannels     = nNewChannelNum;
        PlybkCfg.nSamplerate   = nNewSampleRate;
    }

    if ((nNewSampleRate != threadCtx->nSampleRate) ||
        (nNewChannelNum != threadCtx->nChannelNum) ||
        (nNewBitRate != threadCtx->nBitRate) ||
        !threadCtx->audioInfoNotified)
    {
        int nAudioInfo[3];
        nAudioInfo[0] = nNewSampleRate;
        nAudioInfo[1] = nNewChannelNum;
        nAudioInfo[2] = threadCtx->nBitRate = nNewBitRate;

        ret = p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_AUDIO_INFO,
                (void*)nAudioInfo);
        threadCtx->audioInfoNotified = 1;
    }

    if ((nNewSampleRate != threadCtx->nSampleRate) ||
        (nNewChannelNum != threadCtx->nChannelNum) ||
        (nNewBitPerSample != threadCtx->nBitsPerSample) ||
        (nNewneedDirectOut != threadCtx->needDirectOut) ||
        (nNewDataType != threadCtx->dataType))
    {
        logw("sample rate change from %d to %d.",
                threadCtx->nSampleRate, nNewSampleRate);
        logw("channel num change from %d to %d.",
                threadCtx->nChannelNum, nNewChannelNum);
        logw("bitPerSample num change from %d to %d.",
                threadCtx->nBitsPerSample, nNewBitPerSample);
        logw("if need direct out put flag change from %d to %d.",
                threadCtx->needDirectOut, nNewneedDirectOut);
        logw("data type change from %d to %d.",
                threadCtx->dataType, nNewDataType);
        ret = SoundDeviceStop(p->pSoundCtrl);
        if (ret != 0)
        {
            loge("SoundDeviceStop failed");
            return -1;
        }

        SoundDeviceSetFormat(p->pSoundCtrl, &PlybkCfg);
        threadCtx->nBitsPerSample = nNewBitPerSample;
        threadCtx->needDirectOut = nNewneedDirectOut;
        threadCtx->nSampleRate = nNewSampleRate;
        threadCtx->nChannelNum = nNewChannelNum;
        threadCtx->nFrameSize  = ((nNewChannelNum * threadCtx->nBitsPerSample)>>3);
        // / 8;
        threadCtx->dataType  = nNewDataType;
        if(threadCtx->bFirstFrameSend == 1)
        {
            logw("start sound devide again because samplaRate "
                    "or channelNum change");
            SoundDeviceStart(p->pSoundCtrl);
        }
    }

    return 0;
}

static int startSoundDevice(AudioRenderComp *p)
{
    int ret;
    RenderThreadCtx *threadCtx = p->threadCtx;

    ret = p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_FIRST_FRAME,
            (void*)&threadCtx->nPts);
    if(ret == TIMER_DROP_AUDIO_DATA)
    {
        AudioDecCompReleasePcmData(p->pDecComp, threadCtx->nPcmDataLen);
        //* post a render message to continue the rendering job after message processed.
        BaseCompContinue(&p->base);
        return -1;
    }
    else if(ret == TIMER_NEED_NOTIFY_AGAIN)
    {
        /* waiting process for first frame sync with video is
         * broken by a new message to player, so the player tell us
         * to notify again later.
         */
        ret = AwMessageQueueWaitMessage(p->mq, 10);
        if(ret <= 0)
            BaseCompContinue(&p->base);
        return -1;
    }

    logd("start sound device.");
    SoundDeviceStart(p->pSoundCtrl);

    threadCtx->bFirstFrameSend = 1;
    return 0;
}

static inline int notifyAudioPts(AudioRenderComp *p)
{
    if(p->threadCtx->nPts < 0)
        return 0;

    int64_t callbackParam[2];

    callbackParam[0] = p->threadCtx->nPts;
    callbackParam[1]  = SoundDeviceGetCachedTime(p->pSoundCtrl);

    int ret = p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_PTS_AND_CACHETIME,
            (void*)callbackParam);
    if(ret == TIMER_DROP_AUDIO_DATA)
    {
        AudioDecCompReleasePcmData(p->pDecComp, p->threadCtx->nPcmDataLen);
        BaseCompContinue(&p->base);
        return -1;
    }

    return 0;
}

static inline int writeToSoundDevice(AudioRenderComp *p)
{
    RenderThreadCtx *threadCtx = p->threadCtx;
    uint8_t *pPcmData = threadCtx->pPcmData;
    unsigned nPcmDataLen = threadCtx->nPcmDataLen;
    unsigned nWritten;

    if(!threadCtx->needDirectOut)
    {
        ProcessBalance(pPcmData, nPcmDataLen,
            threadCtx->nBitsPerSample, threadCtx->nChannelNum, p->nConfigOutputBalance);

        ProcessMute(pPcmData, nPcmDataLen, p->bMute);

        if((nPcmDataLen % threadCtx->nFrameSize != 0)&&(threadCtx->nChannelNum > 2))
        {
            logw("invalid pcm length, nPcmDataLen %d, nFrameSize %d",
                    nPcmDataLen, threadCtx->nFrameSize);
            nPcmDataLen = nPcmDataLen / threadCtx->nFrameSize * threadCtx->nFrameSize;
        }
    }

    //* we set data to 0 when the bForceWriteToDeviceFlag is 1
    if(p->bForceWriteToDeviceFlag == 1)
        memset(pPcmData, 0, nPcmDataLen);

    while(nPcmDataLen > 0)
    {
        nWritten = SoundDeviceWrite(p->pSoundCtrl,
                                       pPcmData, nPcmDataLen);
        if(nWritten > 0)
        {
            nPcmDataLen -= nWritten;
            pPcmData    += nWritten;
            AudioDecCompReleasePcmData(p->pDecComp, nWritten);
            usleep(1000);
        }
        else
        {
            //* if no buffer to write, wait some time.
            if (AwMessageQueueWaitMessage(p->mq, 10) > 0)
            {
                AudioDecCompReleasePcmData(p->pDecComp, nPcmDataLen);
                return -1;
            }
        }
    }

    return 0;
}

static void doRender(AwMessage *msg, void *arg)
{
    int ret = 0;

    logv("audio render process render message.");

    AudioRenderComp *p = arg;
    (void)msg;

    if (p->eStatus != PLAYER_STATUS_STARTED)
    {
        logw("not in started status, render message ignored.");
        return;
    }

    RenderThreadCtx *threadCtx = p->threadCtx;
    //************************************************************************
    //* get the audio sample rate and channel num, set it to the sound device.
    //************************************************************************
    if (threadCtx->bFirstFrameSend == 0 &&
            (threadCtx->nSampleRate == 0 || threadCtx->nChannelNum == 0))
    {
        if (initSoundDevice(p) != 0)
            return;
    }

    if (-1 == checkSampleRate(p))
    {
        BaseCompContinue(&p->base);
        return;
    }

    //*******************************************
    //* 1. request pcm data and pts from decoder.
    //*******************************************
    if (requestPcmData(p) != 0)
        return;

    //**********************************************************
    //* 2. If first frame, call back and start the sound device.
    //**********************************************************
    if(threadCtx->bFirstFrameSend == 0 && startSoundDevice(p) != 0)
        return;

    //******************************************************************
    //* 3. report timer difference for the player to adjust timer speed.
    //******************************************************************
    if (notifyAudioPts(p) != 0)
        return;


    //********************************
    //* 4. Write data to sound device.
    //********************************
    writeToSoundDevice(p);

    BaseCompContinue(&p->base);
}

int AudioRenderCompGetAudioOutBalance(AudioRenderComp *p)
{
    int nBalance;

    nBalance = p->nConfigOutputBalance;
    if(nBalance == 0)
        nBalance = 3;

    return nBalance;
}

//* set audio balance, 1 means left channel, 2 means right channel, 3 means stereo.
int AudioRenderCompSetAudioOutBalance(AudioRenderComp* p, int nBalance)
{
    if(nBalance < 0 || nBalance > 3)
        return -1;

    p->nConfigOutputBalance = nBalance;
    return 0;
}

int AudioRenderCompSetAudioMute(AudioRenderComp *p, int bMute)
{
    p->bMute = bMute;
    return 0;
}

int AudioRenderCompGetAudioMuteFlag(AudioRenderComp *p)
{
    return p->bMute;
}

int AudioRenderCompSetForceWriteToDeviceFlag(AudioRenderComp* p, int bForceFlag)
{
    p->bForceWriteToDeviceFlag = bForceFlag;

    return 0;
}

static void ProcessBalance(unsigned char* pData, int nDataLen,
            int nBitsPerSample, int nChannelCount, int nOutBalance)
{
    int    nSampleCount;
    int    nBytesPerSample;
    int    i;
    short* pShortData;

    if(nOutBalance == 0 || nOutBalance == 3)
        return;

    if(nChannelCount < 2)
        return;

    //* don't know how to do with data more than 2 channels.
    //* currently decoder mix all channel to 2 channel for render.
    if(nChannelCount > 2)
        return;

    //* currently decoder only output 16 bits width data.
    if(nBitsPerSample != 16)
        return;

    nBytesPerSample = nBitsPerSample>>3;
    nSampleCount = nDataLen/nBytesPerSample;
    pShortData   = (short*)pData;

    if(nOutBalance == 1)
    {
        for(i=0; i<nSampleCount; i+=2)
            pShortData[i+1] = pShortData[i];
    }
    else
    {
        for(i=0; i<nSampleCount; i+=2)
            pShortData[i] = pShortData[i+1];
    }
}

static void ProcessMute(unsigned char* pData, int nDataLen, int bMute)
{
    if(bMute != 0)
        memset(pData, 0, nDataLen);
    return;
}
