/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleRenderComponent.cpp
 * Description : subtitle render component
 * History :
 *
 */

#include "cdx_log.h"

#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>

#include "subtitleRenderComponent.h"
#include "subtitleDecComponent.h"
#include "AwMessageQueue.h"
#include "baseComponent.h"

#define MAX_SUBTITLE_ITEM (32)  //* support 32 subtitle items show at the same time at most.

//* record subtitle time info.
typedef struct SubtitleItemInfo {
    int     nId;
    int     bValid;
    int64_t nPts;
    int64_t nDuration;
}SubtitleItemInfo;

typedef struct RenderThreadCtx RenderThreadCtx;

struct SubtitleRenderComp {
    //* created at initialize time.
    AwMessageQueue*     mq;
    BaseCompCtx         base;

    pthread_t           sRenderThread;

    enum EPLAYERSTATUS  eStatus;
    SubtitleDecComp*    pDecComp;

    //* objects set by user.
    AvTimer*            pAvTimer;
    PlayerCallback      callback;
    void*               pUserData;
    int                 bEosFlag;

    int64_t             nTimeShiftUs;  //* adjustment set by user.
    int64_t             nVideoOrAudioFirstPts;
    int                 bExternalFlag;

#if defined(CONF_YUNOS)
    //* AliYUNOS Idx+Sub subtitle render in CedarX
    SubtitleStreamInfo* pStreamInfo;
    int                 nStreamCount;
    int                 nStreamSelected;
    int                 bIdxSubFlag;
#endif

    SubCtrl*            pSubCtrl;

    RenderThreadCtx    *threadCtx;
};

static void handleStart(AwMessage *msg, void *arg);
static void handleStop(AwMessage *msg, void *arg);
static void handlePause(AwMessage *msg, void *arg);
static void handleReset(AwMessage *msg, void *arg);
static void handleSetEos(AwMessage *msg, void *arg);
static void handleQuit(AwMessage *msg, void *arg);
static void doRender(AwMessage *msg, void *arg);

static void handleSetSubRender(AwMessage *msg, void *arg);
static void handleTimeShift(AwMessage *msg, void *arg);

static void* SubtitleRenderThread(void* arg);
static void PostRenderMessage(AwMessageQueue* mq);
static void FlushExpiredItems(SubtitleRenderComp* p,
        SubtitleItemInfo* pItemInfo, int64_t nTimeDelayUs);
static void FlushAllItems(SubtitleRenderComp* p, SubtitleItemInfo* pItemInfo);
static int AddItemInfo(SubtitleItemInfo* pItemInfo, SubtitleItem* pItem, int nId);

SubtitleRenderComp* SubtitleRenderCompCreate(void)
{
    SubtitleRenderComp* p;
    int err;

    p = calloc(1, sizeof(*p));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }

    p->mq = AwMessageQueueCreate(4, "SubtitleRenderMq");
    if(p->mq == NULL)
    {
        loge("subtitle render component create message queue fail.");
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

    if (BaseCompInit(&p->base, "subtitle render", p->mq, &handler))
    {
        AwMessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    p->pSubCtrl = NULL;

    p->eStatus = PLAYER_STATUS_STOPPED;

    err = pthread_create(&p->sRenderThread, NULL, SubtitleRenderThread, p);
    if(err != 0)
    {
        loge("subtitle render component create thread fail.");
        BaseCompDestroy(&p->base);
        AwMessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    return p;
}

int SubtitleRenderCompDestroy(SubtitleRenderComp* p)
{
    BaseCompQuit(&p->base, NULL, NULL);
    pthread_join(p->sRenderThread, NULL);
    BaseCompDestroy(&p->base);

#if defined(CONF_YUNOS)
    if(p->pStreamInfo != NULL)
    {
        free(p->pStreamInfo);
        p->pStreamInfo = NULL;
    }
#endif

    AwMessageQueueDestroy(p->mq);
    free(p);

    return 0;
}

int SubtitleRenderCompSetSubControl(SubtitleRenderComp* p, SubCtrl* pSubCtrl)
{
    sem_t replySem;
    sem_init(&replySem, 0, 0);

    AwMessage msg = {
        .messageId = MESSAGE_ID_SET_SUB_RENDER,
        .replySem = &replySem,
        .opaque = pSubCtrl,
        .execute = handleSetSubRender,
    };

    logd("=== subtitle render component set SubControl");

    if(AwMessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle render component post message fail.");
        abort();
    }

    if(SemTimedWait(&replySem, -1) < 0)
    {
        loge("subtitle render component wait for start finish failed.");
        sem_destroy(&replySem);
        return -1;
    }
    logd("=== subtitle render component set SubControl end");

    sem_destroy(&replySem);
    return 0;
}

int SubtitleRenderCompStart(SubtitleRenderComp* p)
{
    return BaseCompStart(&p->base, NULL, NULL);
}

int SubtitleRenderCompStop(SubtitleRenderComp* p)
{
    return BaseCompStop(&p->base, NULL, NULL);
}

int SubtitleRenderCompPause(SubtitleRenderComp* p)
{
    return BaseCompPause(&p->base, NULL, NULL);
}

enum EPLAYERSTATUS SubtitleRenderCompGetStatus(SubtitleRenderComp* p)
{
    return p->eStatus;
}

int SubtitleRenderCompReset(SubtitleRenderComp* p)
{
    return BaseCompReset(&p->base, 0, NULL, NULL);
}

int SubtitleRenderCompSetEOS(SubtitleRenderComp* p)
{
    return BaseCompSetEos(&p->base, NULL, NULL);
}

int SubtitleRenderCompSetCallback(SubtitleRenderComp* p, PlayerCallback callback, void* pUserData)
{
    p->callback  = callback;
    p->pUserData = pUserData;

    return 0;
}

int SubtitleRenderCompSetTimer(SubtitleRenderComp* p, AvTimer* timer)
{
    p->pAvTimer = timer;
    return 0;
}

int SubtitleRenderCompSetDecodeComp(SubtitleRenderComp* p, SubtitleDecComp* d)
{
    p->pDecComp = d;
    return 0;
}

int SubtitleRenderSetShowTimeAdjustment(SubtitleRenderComp* p, int nTimeMs)
{
    sem_t replySem;
    sem_init(&replySem, 0, 0);

    AwMessage msg = {
        .messageId = MESSAGE_ID_SET_TIMESHIFT,
        .replySem = &replySem,
        .execute = handleTimeShift,
        .seekTime = nTimeMs,
    };

    if(AwMessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle render component post message fail.");
        abort();
    }

    if(SemTimedWait(&replySem, -1) < 0)
    {
        loge("subtitle render component wait for time shift setting finish failed.");
        sem_destroy(&replySem);
        return -1;
    }

    sem_destroy(&replySem);
    return 0;
}

int SubtitleRenderGetShowTimeAdjustment(SubtitleRenderComp* p)
{
    return p->nTimeShiftUs;
}

int SubtitleRenderCompSetVideoOrAudioFirstPts(SubtitleRenderComp* p,int64_t nFirstPts)
{
    p->nVideoOrAudioFirstPts = nFirstPts;
    return 0;
}

#if defined(CONF_YUNOS)
int SubtitleRenderCompSetSubtitleStreamInfo(SubtitleRenderComp*    p,
                                         SubtitleStreamInfo*  pStreamInfo,
                                         int                  nStreamCount,
                                         int                  nDefaultStreamIndex)
{
    int i;

    // Todo: check pCodecSpecificData

    //* free old SubtitleStreamInfo.
    if(p->pStreamInfo != NULL)
    {
        free(p->pStreamInfo);
        p->pStreamInfo = NULL;
    }

    p->nStreamSelected = 0;
    p->nStreamCount = 0;
    p->bIdxSubFlag = 0;

    //* set SubtitleStreamInfo.
    p->pStreamInfo = calloc(nStreamCount, sizeof(SubtitleStreamInfo));
    if(p->pStreamInfo == NULL)
    {
        loge("memory malloc fail!");
        return -1;
    }

    memcpy(p->pStreamInfo, pStreamInfo, nStreamCount * sizeof(SubtitleStreamInfo));

    p->nStreamSelected = nDefaultStreamIndex;
    p->nStreamCount = nStreamCount;

    if(p->pStreamInfo[p->nStreamSelected].eCodecFormat == SUBTITLE_CODEC_IDXSUB)
    {
        p->bIdxSubFlag = 1;
        logi("This subtitle is IdxSub.");
    }

    return 0;
}

int SubtitleRenderCompSwitchStream(SubtitleRenderComp* p, int nStreamIndex)
{
    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("can not switch status when subtitle decoder is not in stopped status.");
        return -1;
    }

    p->bIdxSubFlag = 0;
    p->nStreamSelected = nStreamIndex;

    if(p->pStreamInfo[p->nStreamSelected].eCodecFormat == SUBTITLE_CODEC_IDXSUB)
    {
        p->bIdxSubFlag = 1;
        logi("This subtitle is IdxSub.");
    }

    return 0;
}
#endif

struct RenderThreadCtx {
    unsigned nIdCounter;
    SubtitleItemInfo pItemInfo[MAX_SUBTITLE_ITEM];
};

static void* SubtitleRenderThread(void* arg)
{
    SubtitleRenderComp* p = arg;
    AwMessage msg;
    RenderThreadCtx threadCtx = {
        .nIdCounter = 0,
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
    SubtitleRenderComp* p = arg;

    if(p->eStatus == PLAYER_STATUS_STARTED)
    {
        logw("already in started status.");
        *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    if(p->eStatus == PLAYER_STATUS_STOPPED)
        p->bEosFlag = 0;

    if(p->pDecComp != NULL)
        p->bExternalFlag = SubtitleDecCompGetExternalFlag(p->pDecComp);

    p->eStatus = PLAYER_STATUS_STARTED;
    *msg->result = 0;
    sem_post(msg->replySem);

    BaseCompContinue(&p->base);
}

static void handleStop(AwMessage *msg, void *arg)
{
    SubtitleRenderComp* p = arg;

    if (p->eStatus == PLAYER_STATUS_STOPPED)
    {
        logw("already in stopped status.");
        *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    if(p->pSubCtrl)
        CdxSubRenderHide(p->pSubCtrl, 0);

    FlushAllItems(p, p->threadCtx->pItemInfo);
    p->threadCtx->nIdCounter = 0;

    p->eStatus = PLAYER_STATUS_STOPPED;
    *msg->result = 0;
    sem_post(msg->replySem);
}

static void handlePause(AwMessage *msg, void *arg)
{
    SubtitleRenderComp* p = arg;

    if (p->eStatus != PLAYER_STATUS_STARTED)
    {
        logw("not in started status, pause operation invalid.");
        *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    p->eStatus = PLAYER_STATUS_PAUSED;
    *msg->result = 0;
    sem_post(msg->replySem);
}

static void handleSetSubRender(AwMessage *msg, void *arg)
{
    SubtitleRenderComp* p = arg;

    p->pSubCtrl = msg->opaque;
    if (p->pSubCtrl == NULL)
        logw("sub render is NULL");

    sem_post(msg->replySem);

    if (p->eStatus == PLAYER_STATUS_STARTED)
        BaseCompContinue(&p->base);
}

static void handleQuit(AwMessage *msg, void *arg)
{
    SubtitleRenderComp* p = arg;

    FlushAllItems(p, p->threadCtx->pItemInfo);

    sem_post(msg->replySem);
    p->eStatus = PLAYER_STATUS_STOPPED;
    pthread_exit(NULL);
}

static void handleReset(AwMessage *msg, void *arg)
{
    SubtitleRenderComp* p = arg;

    FlushAllItems(p, p->threadCtx->pItemInfo);
    p->threadCtx->nIdCounter = 0;
    p->bEosFlag = 0;

    *msg->result = 0;
    sem_post(msg->replySem);

    if(p->eStatus == PLAYER_STATUS_STARTED)
        BaseCompContinue(&p->base);
}

static void handleSetEos(AwMessage *msg, void *arg)
{
    SubtitleRenderComp* p = arg;

    p->bEosFlag = 1;
    sem_post(msg->replySem);

    if(p->eStatus == PLAYER_STATUS_STARTED)
        BaseCompContinue(&p->base);
}

static void handleTimeShift(AwMessage *msg, void *arg)
{
    SubtitleRenderComp* p = arg;

    p->nTimeShiftUs = msg->seekTime * 1000; //* transform ms to us.

    sem_post(msg->replySem);

    if(p->eStatus == PLAYER_STATUS_STARTED)
        BaseCompContinue(&p->base);
}

static void doRender(AwMessage *msg, void *arg)
{
    int ret;
    SubtitleRenderComp* p = arg;
    SubtitleItem *pCurItem = NULL;
    (void)msg;

    if (p->eStatus != PLAYER_STATUS_STARTED)
    {
        logw("not in started status, render message ignored.");
        return;
    }

    RenderThreadCtx *threadCtx = p->threadCtx;

    //* clear expired subtitle items and return buffer to decoder.
    int64_t shiftPts = p->nTimeShiftUs;
    if(p->bExternalFlag == 1)
        shiftPts += p->nVideoOrAudioFirstPts;
    FlushExpiredItems(p, threadCtx->pItemInfo, shiftPts);

    pCurItem = SubtitleDecCompNextSubtitleItem(p->pDecComp);
    if (pCurItem == NULL)
    {
        //* check whether stream end.
        if(p->bEosFlag)
        {
            p->callback(p->pUserData, PLAYER_SUBTITLE_RENDER_NOTIFY_EOS, NULL);
            return;
        }

        if (AwMessageQueueWaitMessage(p->mq, 100) <= 0)
            BaseCompContinue(&p->base);
        return;
    }

    //* check whether it is time to show the item.
    int64_t callbackParam[3];
    callbackParam[0] = pCurItem->nPts + shiftPts;
    callbackParam[1] = pCurItem->nDuration;
    callbackParam[2] = 0; //* this item not showed yet.
    ret = p->callback(p->pUserData, PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_PTS_AND_DURATION,
            (void*)callbackParam);

    if (ret > 0)
    {
        //* not the time to show this item, wait.
        //* don't wait in a loop, just wait no more than 100ms, because we need to
        //* notify expired message of other subtitle items, and also we need to
        //* callback again to check pts again.
        //* (there may be a pts jump event which make the nWaitTimeMs jump.)
        int nWaitTimeMs = (ret < 100) ? ret : 100;
        if (AwMessageQueueWaitMessage(p->mq, nWaitTimeMs) > 0)
            return;
    }
    else if (ret < 0)
    {
        //* item expired, flush it.
        logw("skip one subtitle item, start time = %lld us, duration = %lld us.",
                pCurItem->nPts, pCurItem->nDuration);
        pCurItem = SubtitleDecCompRequestSubtitleItem(p->pDecComp);
        SubtitleDecCompFlushSubtitleItem(p->pDecComp, pCurItem);
    }
    else
    {
        //* time to show the item.
        pCurItem = SubtitleDecCompRequestSubtitleItem(p->pDecComp);

        //* check whether a clear command.
        if(pCurItem->pText == NULL && pCurItem->pBitmapData == NULL)
        {
            //* it is a subtitle clear command, release all the subtitle item.
            FlushAllItems(p, threadCtx->pItemInfo);
            SubtitleDecCompFlushSubtitleItem(p->pDecComp, pCurItem);
        }
        else
        {
            if(p->pSubCtrl)
            {
                CdxSubRenderShow(p->pSubCtrl, pCurItem, threadCtx->nIdCounter);
            }

            /************************begin****************************/

            //* add the item to item info array.
            ret = AddItemInfo(threadCtx->pItemInfo, pCurItem, threadCtx->nIdCounter++);
            if(ret < 0)
            {
                loge("more than 32 items showed, should not run here.");
                abort();
            }

            //* return the item.
            SubtitleDecCompFlushSubtitleItem(p->pDecComp, pCurItem);
        }
    }

    BaseCompContinue(&p->base);
}

static void FlushExpiredItems(SubtitleRenderComp* p, SubtitleItemInfo* pItemInfo,
        int64_t nTimeDelayUs)
{
    int     i;
    int64_t nPts;
    int64_t nDuration;
    int     ret;
    int64_t callbackParam[3];

    for(i=0; i<MAX_SUBTITLE_ITEM; i++)
    {
        if(pItemInfo[i].bValid == 0)
            continue;

        nPts      = pItemInfo[i].nPts;
        nDuration = pItemInfo[i].nDuration;

        callbackParam[0] = nPts + nTimeDelayUs;
        callbackParam[1] = nDuration;
        //* this item has been showed, we just check whether it is time to clear it.
        callbackParam[2] = 1;
        ret = p->callback(p->pUserData, PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_PTS_AND_DURATION,
                (void*)callbackParam);

        if(ret < 0)
        {
            pItemInfo[i].bValid = 0;

            if(p->pSubCtrl)
            {
                CdxSubRenderHide(p->pSubCtrl, pItemInfo[i].nId);
            }
        }
    }

    return;
}

static void FlushAllItems(SubtitleRenderComp* p, SubtitleItemInfo* pItemInfo)
{
    int     i;
    int     ret;
    int64_t callbackParam[2];

    for(i=0; i<MAX_SUBTITLE_ITEM; i++)
    {
        if(pItemInfo[i].bValid)
        {
            pItemInfo[i].bValid = 0;

            if(p->pSubCtrl)
            {
                CdxSubRenderHide(p->pSubCtrl, pItemInfo[i].nId);
            }
        }
    }

    return;
}

static int AddItemInfo(SubtitleItemInfo* pItemInfo, SubtitleItem* pItem, int nId)
{
    int i;

    for(i=0; i<MAX_SUBTITLE_ITEM; i++)
    {
        if(pItemInfo[i].bValid == 0)
        {
            pItemInfo[i].bValid    = 1;
            pItemInfo[i].nId       = nId;
            pItemInfo[i].nPts      = pItem->nPts;
            pItemInfo[i].nDuration = pItem->nDuration;
            break;
        }
    }

    if(i == MAX_SUBTITLE_ITEM)
        return -1;

    return 0;
}
