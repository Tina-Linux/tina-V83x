/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : subtitleDecComponent.cpp
 * Description : subtitle decoder component
 * History :
 *
 */

#include "cdx_log.h"

#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>

#include "subtitleDecComponent.h"
#include "sdecoder.h"

#include "AwMessageQueue.h"
#include "streamManager.h"
#include "baseComponent.h"

#define MAX_SUBTITLE_STREAM_BUFFER_SIZE (2048*1024)
#define MAX_SUBTITLE_STREAM_FRAME_COUNT (2048)

struct SubtitleDecComp {
    //* created at initialize time.
    AwMessageQueue          *mq;
    BaseCompCtx             base;

    sem_t                   streamDataSem;
    sem_t                   frameBufferSem;

    pthread_t               sDecodeThread;

    enum EPLAYERSTATUS      eStatus;

    //* objects set by user.
    AvTimer*                pAvTimer;
    PlayerCallback          callback;
    void*                   pUserData;
    int                     bEosFlag;

    SubtitleDecoder*        pDecoder;
    int                     nStreamCount;
    int                     nStreamSelected;
    int                     nExternalSubtitleNum;
    int                     nInternalSubtitleNum;

    SubtitleStreamInfo*     pStreamInfoArr;
    StreamManager**         pStreamManagerArr;
    pthread_mutex_t         streamManagerMutex;

    int                     bCrashFlag;

    int                     afterSwitchStream;
};

static void handleStart(AwMessage *msg, void *arg);
static void handleStop(AwMessage *msg, void *arg);
static void handlePause(AwMessage *msg, void *arg);
static void handleReset(AwMessage *msg, void *arg);
static void handleSetEos(AwMessage *msg, void *arg);
static void handleQuit(AwMessage *msg, void *arg);
static void doDecode(AwMessage *msg, void *arg);

static void* SubtitleDecodeThread(void* arg);
#if 0
static void FlushStreamManagerBuffers(SubtitleDecComp* p, int64_t curTime,
        int bIncludeSeletedStream);
#endif

SubtitleDecComp* SubtitleDecCompCreate(void)
{
    SubtitleDecComp* p;
    int err;

    p = calloc(1, sizeof(*p));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }

    p->mq = AwMessageQueueCreate(4, "SubtitleDecodeMq");
    if(p->mq == NULL)
    {
        loge("subtitle decoder component create message queue fail.");
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

    if (BaseCompInit(&p->base, "subtitle decoder", p->mq, &handler))
    {
        AwMessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    sem_init(&p->streamDataSem, 0, 0);
    sem_init(&p->frameBufferSem, 0, 0);
    pthread_mutex_init(&p->streamManagerMutex, NULL);

    p->eStatus = PLAYER_STATUS_STOPPED;

    err = pthread_create(&p->sDecodeThread, NULL, SubtitleDecodeThread, p);
    if(err != 0)
    {
        loge("subtitle decode component create thread fail.");
        BaseCompDestroy(&p->base);
        sem_destroy(&p->streamDataSem);
        sem_destroy(&p->frameBufferSem);
        pthread_mutex_destroy(&p->streamManagerMutex);
        AwMessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    return (SubtitleDecComp*)p;
}

static void WakeUpThread(void *arg)
{
    SubtitleDecComp *p = arg;
    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);
}

void SubtitleDecCompDestroy(SubtitleDecComp* p)
{
    int i;

    BaseCompQuit(&p->base, WakeUpThread, p);

    pthread_join(p->sDecodeThread, NULL);

    BaseCompDestroy(&p->base);

    sem_destroy(&p->streamDataSem);
    sem_destroy(&p->frameBufferSem);
    pthread_mutex_destroy(&p->streamManagerMutex);

    if(p->pStreamInfoArr != NULL)
    {
        for(i=0; i<p->nStreamCount; i++)
        {
            if(p->pStreamInfoArr[i].pUrl != NULL)
            {
                free(p->pStreamInfoArr[i].pUrl);
                p->pStreamInfoArr[i].pUrl = NULL;
            }
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL &&
                    p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
            {
                free(p->pStreamInfoArr[i].pCodecSpecificData);
                p->pStreamInfoArr[i].pCodecSpecificData = NULL;
                p->pStreamInfoArr[i].nCodecSpecificDataLen = 0;
            }
        }
        free(p->pStreamInfoArr);
    }

    if(p->pStreamManagerArr != NULL)
    {
        for(i=0; i<p->nStreamCount; i++)
        {
            if(p->pStreamManagerArr[i] != NULL)
            {
                StreamManagerDestroy(p->pStreamManagerArr[i]);
                p->pStreamManagerArr[i] = NULL;
            }
        }
        free(p->pStreamManagerArr);
    }

    AwMessageQueueDestroy(p->mq);
    free(p);

    return;
}

int SubtitleDecCompStart(SubtitleDecComp* p)
{
    return BaseCompStart(&p->base, WakeUpThread, p);
}

int SubtitleDecCompStop(SubtitleDecComp* p)
{
    return BaseCompStop(&p->base, WakeUpThread, p);
}

int SubtitleDecCompPause(SubtitleDecComp* p)
{
    return BaseCompPause(&p->base, WakeUpThread, p);
}

enum EPLAYERSTATUS SubtitleDecCompGetStatus(SubtitleDecComp* p)
{
    return p->eStatus;
}

int SubtitleDecCompReset(SubtitleDecComp* p, int64_t nSeekTime)
{
    return BaseCompReset(&p->base, nSeekTime, WakeUpThread, p);
}

int SubtitleDecCompSetEOS(SubtitleDecComp* p)
{
    return BaseCompSetEos(&p->base, WakeUpThread, p);
}

int SubtitleDecCompSetCallback(SubtitleDecComp* p, PlayerCallback callback, void* pUserData)
{
    p->callback  = callback;
    p->pUserData = pUserData;

    return 0;
}

int SubtitleDecCompSetSubtitleStreamInfo(SubtitleDecComp*     p,
                                         SubtitleStreamInfo*  pStreamInfo,
                                         int                  nStreamCount,
                                         int                  nDefaultStreamIndex)
{
    int i;

    //* free old SubtitleStreamInfo.
    if(p->pStreamInfoArr != NULL && p->nStreamCount > 0)
    {
        for(i=0; i<p->nStreamCount; i++)
        {
            if(p->pStreamInfoArr[i].pUrl != NULL)
            {
                free(p->pStreamInfoArr[i].pUrl);
                p->pStreamInfoArr[i].pUrl = NULL;
            }
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL &&
                    p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
            {
                free(p->pStreamInfoArr[i].pCodecSpecificData);
                p->pStreamInfoArr[i].pCodecSpecificData = NULL;
                p->pStreamInfoArr[i].nCodecSpecificDataLen = 0;
            }
        }

        free(p->pStreamInfoArr);
        p->pStreamInfoArr = NULL;
    }

    //* free old StreamManager.
    if(p->pStreamManagerArr != NULL)
    {
        for(i=0; i<p->nStreamCount; i++)
        {
            if(p->pStreamManagerArr[i] != NULL)
            {
                StreamManagerDestroy(p->pStreamManagerArr[i]);
                p->pStreamManagerArr[i] = NULL;
            }
        }
        free(p->pStreamManagerArr);
    }

    p->nStreamSelected = 0;
    p->nStreamCount = 0;

    //* set SubtitleStreamInfo.
    p->pStreamInfoArr = calloc(nStreamCount, sizeof(*p->pStreamInfoArr));
    if(p->pStreamInfoArr == NULL)
    {
        loge("memory malloc fail!");
        return -1;
    }

    for(i=0; i<nStreamCount; i++)
    {
        memcpy(&p->pStreamInfoArr[i], &pStreamInfo[i], sizeof(SubtitleStreamInfo));
        if(pStreamInfo[i].pUrl != NULL)
        {
            p->pStreamInfoArr[i].pUrl = strdup(pStreamInfo[i].pUrl);
            if(p->pStreamInfoArr[i].pUrl == NULL)
            {
                loge("malloc memory fail.");
                p->pStreamInfoArr[i].pUrl = 0;
                break;
            }
        }
        if(p->pStreamInfoArr[i].pCodecSpecificData != NULL &&
                p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
        {
            pStreamInfo[i].nCodecSpecificDataLen = p->pStreamInfoArr[i].nCodecSpecificDataLen;
            pStreamInfo[i].pCodecSpecificData = (char*)malloc(pStreamInfo[i].nCodecSpecificDataLen);
            if(pStreamInfo[i].pCodecSpecificData == NULL)
            {
                loge("malloc memory fail.");
                pStreamInfo[i].pCodecSpecificData = 0;
                break;
            }
            memcpy(pStreamInfo[i].pCodecSpecificData,
                p->pStreamInfoArr[i].pCodecSpecificData,
                p->pStreamInfoArr[i].nCodecSpecificDataLen);
        }
    }

    if(i != nStreamCount)
    {
        //* memory alloc fail break.
        i--;
        for(; i>=0; i--)
        {
            if(p->pStreamInfoArr[i].pUrl != NULL)
            {
                free(p->pStreamInfoArr[i].pUrl);
                p->pStreamInfoArr[i].pUrl = NULL;
            }
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL &&
                    p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
            {
                free(p->pStreamInfoArr[i].pCodecSpecificData);
                p->pStreamInfoArr[i].pCodecSpecificData = NULL;
                p->pStreamInfoArr[i].nCodecSpecificDataLen = 0;
            }
        }
        free(p->pStreamInfoArr);
        return -1;
    }

    //* allocate StreamManager for each stream.
    p->pStreamManagerArr = malloc(nStreamCount*sizeof(StreamManager*));
    if(p->pStreamManagerArr == NULL)
    {
        loge("malloc memory fail.");
        for(i=0; i<nStreamCount; i++)
        {
            if(p->pStreamInfoArr[i].pUrl != NULL)
            {
                free(p->pStreamInfoArr[i].pUrl);
                p->pStreamInfoArr[i].pUrl = NULL;
            }
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL &&
                    p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
            {
                free(p->pStreamInfoArr[i].pCodecSpecificData);
                p->pStreamInfoArr[i].pCodecSpecificData = NULL;
                p->pStreamInfoArr[i].nCodecSpecificDataLen = 0;
            }
        }
        free(p->pStreamInfoArr);
        return -1;
    }

    for(i=0; i<nStreamCount; i++)
    {
        if(p->pStreamInfoArr[i].bExternal)
        {
            p->pStreamManagerArr[i] = NULL;
            continue;
        }

        p->pStreamManagerArr[i] = StreamManagerCreate(MAX_SUBTITLE_STREAM_BUFFER_SIZE,
                                                      MAX_SUBTITLE_STREAM_FRAME_COUNT,
                                                      i);
        if(p->pStreamManagerArr[i] == NULL)
        {
            loge("create stream manager for subtitle stream %d fail", i);
            break;
        }
    }

    if(i != nStreamCount)
    {
        //* memory alloc fail break.
        i--;
        for(; i>=0; i--)
        {
            if(p->pStreamManagerArr[i] != NULL)
            {
                StreamManagerDestroy(p->pStreamManagerArr[i]);
                p->pStreamManagerArr[i] = NULL;
            }
        }
        free(p->pStreamManagerArr);
        p->pStreamManagerArr = NULL;

        for(i=0; i<nStreamCount; i++)
        {
            if(p->pStreamInfoArr[i].pUrl != NULL)
            {
                free(p->pStreamInfoArr[i].pUrl);
                p->pStreamInfoArr[i].pUrl = NULL;
            }
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL &&
                    p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
            {
                free(p->pStreamInfoArr[i].pCodecSpecificData);
                p->pStreamInfoArr[i].pCodecSpecificData = NULL;
                p->pStreamInfoArr[i].nCodecSpecificDataLen = 0;
            }
        }
        free(p->pStreamInfoArr);
        return -1;
    }

    //* check the external subtitle number
    for(i=0;i<nStreamCount;i++)
    {
        if(p->pStreamInfoArr[i].bExternal==1)
            p->nExternalSubtitleNum++;
        else
            p->nInternalSubtitleNum++;
    }

    p->nStreamSelected = nDefaultStreamIndex;
    p->nStreamCount = nStreamCount;

    return 0;
}

int SubtitleDecCompAddSubtitleStream(SubtitleDecComp* p, SubtitleStreamInfo* pStreamInfo)
{
    pthread_mutex_lock(&p->streamManagerMutex);

    if(p->nStreamCount > 0)
    {
        SubtitleStreamInfo* pStreamInfoArr;
        StreamManager**     pStreamManagerArr;
        int                 nStreamCount;

        nStreamCount = p->nStreamCount + 1;
        pStreamManagerArr = malloc(sizeof(StreamManager*)*nStreamCount);
        if(pStreamManagerArr == NULL)
        {
            loge("malloc memory fail.");
            pthread_mutex_unlock(&p->streamManagerMutex);
            return -1;
        }

        pStreamInfoArr = malloc(sizeof(SubtitleStreamInfo)*nStreamCount);
        if(pStreamInfoArr == NULL)
        {
            loge("malloc memory fail.");
            free(pStreamManagerArr);
            pthread_mutex_unlock(&p->streamManagerMutex);
            return -1;
        }

        memcpy(pStreamManagerArr, p->pStreamManagerArr, p->nStreamCount * sizeof(StreamManager*));
        if(pStreamInfo->bExternal)
            pStreamManagerArr[nStreamCount-1] = NULL;
        else
        {
            pStreamManagerArr[nStreamCount-1] = StreamManagerCreate(MAX_SUBTITLE_STREAM_BUFFER_SIZE,
                                                                    MAX_SUBTITLE_STREAM_FRAME_COUNT,
                                                                    nStreamCount-1);
            if(pStreamManagerArr[nStreamCount-1] == NULL)
            {
                loge("create stream manager fail.");
                free(pStreamManagerArr);
                free(pStreamInfoArr);
                pthread_mutex_unlock(&p->streamManagerMutex);
                return -1;
            }
        }

        memcpy(pStreamInfoArr, p->pStreamInfoArr, p->nStreamCount*sizeof(*pStreamInfoArr));
        memcpy(&pStreamInfoArr[nStreamCount-1], pStreamInfo, sizeof(*pStreamInfoArr));
        if(pStreamInfo->pUrl != NULL)
        {
            pStreamInfoArr[nStreamCount-1].pUrl = strdup(pStreamInfo->pUrl);
            if(pStreamInfoArr[nStreamCount-1].pUrl == NULL)
            {
                loge("malloc memory fail.");
                free(pStreamManagerArr);
                free(pStreamInfoArr);
                pthread_mutex_unlock(&p->streamManagerMutex);
                return -1;
            }
        }
        if(pStreamInfo->pCodecSpecificData != NULL && pStreamInfo->nCodecSpecificDataLen > 0)
        {
            pStreamInfoArr[nStreamCount-1].nCodecSpecificDataLen =
                pStreamInfo->nCodecSpecificDataLen;
            pStreamInfoArr[nStreamCount-1].pCodecSpecificData =
                (char*)malloc(pStreamInfo->nCodecSpecificDataLen);
            if(pStreamInfoArr[nStreamCount-1].pCodecSpecificData == NULL)
            {
                loge("malloc memory fail.");
                free(pStreamManagerArr);
                free(pStreamInfoArr);
                return -1;
            }
            memcpy(pStreamInfoArr[nStreamCount-1].pCodecSpecificData,
                pStreamInfo->pCodecSpecificData,
                pStreamInfo->nCodecSpecificDataLen);
        }

        free(p->pStreamInfoArr);
        free(p->pStreamManagerArr);
        p->pStreamInfoArr    = pStreamInfoArr;
        p->pStreamManagerArr = pStreamManagerArr;
        p->nStreamCount      = nStreamCount;

        pthread_mutex_unlock(&p->streamManagerMutex);

        return 0;
    }
    else
    {
        pthread_mutex_unlock(&p->streamManagerMutex);
        return SubtitleDecCompSetSubtitleStreamInfo(p, pStreamInfo, 1, 0);
    }
}

int SubtitleDecCompGetSubtitleStreamCnt(SubtitleDecComp* p)
{
    return p->nStreamCount;
}

int SubtitleDecCompCurrentStreamIndex(SubtitleDecComp* p)
{
    return p->nStreamSelected;
}

int SubtitleDecCompGetSubtitleStreamInfo(SubtitleDecComp* p, int* pStreamNum,
        SubtitleStreamInfo** ppStreamInfo)
{
    int                     i;
    SubtitleStreamInfo*     pStreamInfo;
    int                     nStreamCount;

    nStreamCount = p->nStreamCount;

    pStreamInfo = (SubtitleStreamInfo*)malloc(sizeof(SubtitleStreamInfo)*nStreamCount);
    if(pStreamInfo == NULL)
    {
        loge("memory malloc fail!");
        return -1;
    }
    memset(pStreamInfo, 0, sizeof(SubtitleStreamInfo)*nStreamCount);

    for(i=0; i<nStreamCount; i++)
    {
        memcpy(&pStreamInfo[i], &p->pStreamInfoArr[i], sizeof(SubtitleStreamInfo));
        if(p->pStreamInfoArr[i].pUrl != NULL)
        {
            pStreamInfo[i].pUrl = strdup(p->pStreamInfoArr[i].pUrl);
            if(pStreamInfo[i].pUrl == NULL)
            {
                loge("malloc memory fail.");
                break;
            }
        }
        if(p->pStreamInfoArr[i].pCodecSpecificData != NULL &&
                p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
        {
            pStreamInfo[i].nCodecSpecificDataLen = p->pStreamInfoArr[i].nCodecSpecificDataLen;
            pStreamInfo[i].pCodecSpecificData = (char*)malloc(pStreamInfo[i].nCodecSpecificDataLen);
            if(pStreamInfo[i].pCodecSpecificData == NULL)
            {
                loge("malloc memory fail.");
                break;
            }
            memcpy(pStreamInfo[i].pCodecSpecificData,
                p->pStreamInfoArr[i].pCodecSpecificData,
                p->pStreamInfoArr[i].nCodecSpecificDataLen);
        }
    }

    if(i != nStreamCount)
    {
        //* memory alloc fail break.
        i--;
        for(; i>=0; i--)
        {
            if(pStreamInfo[i].pUrl != NULL)
            {
                free(pStreamInfo[i].pUrl);
                pStreamInfo[i].pUrl = NULL;
            }
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL &&
                    p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
            {
                free(p->pStreamInfoArr[i].pCodecSpecificData);
                p->pStreamInfoArr[i].pCodecSpecificData = NULL;
                p->pStreamInfoArr[i].nCodecSpecificDataLen = 0;
            }
        }
        free(pStreamInfo);
        return -1;
    }

    *pStreamNum = nStreamCount;
    *ppStreamInfo = pStreamInfo;

    return 0;
}

int SubtitleDecCompSetTimer(SubtitleDecComp* p, AvTimer* timer)
{
    p->pAvTimer = timer;
    return 0;
}

int SubtitleDecCompRequestStreamBuffer(SubtitleDecComp* p,
                                   int          nRequireSize,
                                   char**       ppBuf,
                                   int*         pBufSize,
                                   int          nStreamIndex)
{
    StreamManager*          pSm;
    StreamFrame*            pTmpFrame;
    char*                   pBuf;
    int                     nBufSize;

    //* the nStreamIndex is pass form demux ,not the index in pStreamInfoArr.
    //* Because pStreamInfoArr include external and internal subtitle,and the
    //* external subtitle is in front of intenalSub, we should skip nExternalNums
    //* Be careful: if internalSub is in front of externalSub, we should remove the next line code
    //nStreamIndex += p->nExternalSubtitleNum;

    *ppBuf    = NULL;
    *pBufSize = 0;

    pBuf     = NULL;
    nBufSize = 0;

    pthread_mutex_lock(&p->streamManagerMutex);

    if(nStreamIndex < 0 || nStreamIndex >= p->nStreamCount)
    {
        loge("stream index invalid, stream index = %d, subtitle stream num = %d",
                    nStreamIndex, p->nStreamCount);
        pthread_mutex_unlock(&p->streamManagerMutex);
        return -1;
    }

    /* This is a waste of time. If decoder is crashed, p->bCrashFlag != 0,
     * we will call StreamManagerRequestStream in this function when necessary,
     * and free some space.
     *
     * 2016-05-17
     * You can remove this block of code after, say half a year.
     */
#if 0
    //* when decoder crashed, the main thread is not running,
    //* we need to flush stream buffers for demux keep going.
    if (p->bCrashFlag &&
            p->pAvTimer->GetStatus(p->pAvTimer) == TIMER_STATUS_START)
    {
        int nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
        for (int i = 0; i < p->nStreamCount; i++)
        {
            pSm = p->pStreamManagerArr[i];
            if (pSm != NULL)
                StreamManagerRewind(pSm, nCurTime);
        }
    }
#endif

    if(p->pStreamInfoArr[nStreamIndex].bExternal)
    {
        loge("subtitle stream %d is an external subtitle, request stream buffer fail.",
                nStreamIndex);
        pthread_mutex_unlock(&p->streamManagerMutex);
        return -1;
    }

    pSm = p->pStreamManagerArr[nStreamIndex];

    if(pSm == NULL)
    {
        loge("buffer for selected stream is not created, request buffer fail.");
        pthread_mutex_unlock(&p->streamManagerMutex);
        return -1;
    }

    if(nRequireSize > StreamManagerBufferSize(pSm))
    {
        loge("require size too big.");
        pthread_mutex_unlock(&p->streamManagerMutex);
        return -1;
    }

    if(nStreamIndex == p->nStreamSelected && p->bCrashFlag == 0)
    {
        if(StreamManagerRequestBuffer(pSm, nRequireSize, &pBuf, &nBufSize) < 0)
        {
            pthread_mutex_unlock(&p->streamManagerMutex);
            logd("request buffer fail.");
            return -1;
        }
    }
    else
    {
        while(StreamManagerRequestBuffer(pSm, nRequireSize, &pBuf, &nBufSize) < 0)
        {
            pTmpFrame = StreamManagerRequestStream(pSm);
            if(pTmpFrame != NULL)
                StreamManagerFlushStream(pSm, pTmpFrame);
            else
            {
                loge("all stream flushed but still can not allocate buffer.");
                pthread_mutex_unlock(&p->streamManagerMutex);
                return -1;
            }
        }
    }

    //* output the buffer.
    *ppBuf    = pBuf;
    *pBufSize = nBufSize;

    pthread_mutex_unlock(&p->streamManagerMutex);
    return 0;
}

int SubtitleDecCompSubmitStreamData(SubtitleDecComp* p,
        SubtitleStreamDataInfo* pDataInfo, int nStreamIndex)
{
    int                     nSemCnt;
    StreamManager*          pSm;
    StreamFrame             streamFrame;

    //* the nStreamIndex is pass form demux ,not the index in pStreamInfoArr.
    //* Because pStreamInfoArr include external and internal subtitle,and the
    //* external subtitle is in front of intenalSub, we should skip nExternalNums
    //* Be careful: if internalSub is in front of externalSub, we should remove the next line code
    //nStreamIndex += p->nExternalSubtitleNum;

    //* submit data to stream manager
    pthread_mutex_lock(&p->streamManagerMutex);

    pSm = p->pStreamManagerArr[nStreamIndex];

    streamFrame.pData     = pDataInfo->pData;
    streamFrame.nLength   = pDataInfo->nLength;
    streamFrame.nPts      = pDataInfo->nPts;
    streamFrame.nPcr      = pDataInfo->nPcr;
    streamFrame.nDuration = pDataInfo->nDuration;

    StreamManagerAddStream(pSm, &streamFrame);

    pthread_mutex_unlock(&p->streamManagerMutex);

    if(sem_getvalue(&p->streamDataSem, &nSemCnt) == 0)
    {
        if(nSemCnt == 0)
            sem_post(&p->streamDataSem);
    }

    return 0;
}

SubtitleItem* SubtitleDecCompRequestSubtitleItem(SubtitleDecComp* p)
{
    return RequestSubtitleItem(p->pDecoder);
}

void SubtitleDecCompFlushSubtitleItem(SubtitleDecComp* p, SubtitleItem* pItem)
{
    int ret;
    int nSemCnt;

    FlushSubtitleItem(p->pDecoder, pItem);
    if(sem_getvalue(&p->frameBufferSem, &nSemCnt) == 0)
    {
        if(nSemCnt == 0)
            sem_post(&p->frameBufferSem);
    }

    return;
}

SubtitleItem* SubtitleDecCompNextSubtitleItem(SubtitleDecComp* p)
{
    return NextSubtitleItem(p->pDecoder);
}

//* must be called at stopped status.
int SubtitleDecCompSwitchStream(SubtitleDecComp* p, int nStreamIndex)
{
    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("can not switch status when subtitle decoder is not in stopped status.");
        return -1;
    }

    pthread_mutex_lock(&p->streamManagerMutex);
    p->nStreamSelected = nStreamIndex;
    p->afterSwitchStream = 1;
    pthread_mutex_unlock(&p->streamManagerMutex);
    return 0;
}

int SubtitleDecCompStreamBufferSize(SubtitleDecComp* p, int nStreamIndex)
{
    StreamManager *pSm;

    pSm = p->pStreamManagerArr[nStreamIndex];
    if(pSm != NULL)
        return StreamManagerBufferSize(pSm);
    else
        return 0;   //* external subtitle.
}

int SubtitleDecCompStreamDataSize(SubtitleDecComp* p, int nStreamIndex)
{
    int nStreamDataSize = 0;

    if(p->pStreamManagerArr[nStreamIndex] != NULL)
        nStreamDataSize = StreamManagerStreamDataSize(p->pStreamManagerArr[nStreamIndex]);

    return nStreamDataSize;
}

int SubtitleDecCompIsExternalSubtitle(SubtitleDecComp* p, int nStreamIndex)
{
    if(nStreamIndex < 0 || nStreamIndex >= p->nStreamCount)
    {
        logw("invalid subtitle stream index.");
        return 0;
    }

    if(p->pStreamInfoArr[nStreamIndex].bExternal)
        return 1;
    else
        return 0;
}

int SubtitleDecCompGetExternalFlag(SubtitleDecComp* p)
{
    return p->pStreamInfoArr[p->nStreamSelected].bExternal;
}

static void* SubtitleDecodeThread(void* arg)
{
    SubtitleDecComp* p = arg;
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
    SubtitleDecComp* p = arg;

    if (p->eStatus == PLAYER_STATUS_STARTED)
    {
        loge("invalid start operation, already in started status.");
        if (msg->result)
            *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    if (p->eStatus == PLAYER_STATUS_PAUSED)
    {
        //* send a decode message to start decoding.
        //* when had internal subtitle, also send decode message
        if(p->pStreamInfoArr[p->nStreamSelected].bExternal == 0
           || p->pStreamInfoArr[p->nStreamSelected].eCodecFormat == SUBTITLE_CODEC_IDXSUB
           || p->nInternalSubtitleNum > 0)
        {
            if (p->bCrashFlag == 0)
                BaseCompContinue(&p->base);
        }
        p->eStatus = PLAYER_STATUS_STARTED;
        if (msg->result)
            *msg->result = 0;
        sem_post(msg->replySem);
        return;
    }

    //* create a decoder.
    p->pDecoder = CreateSubtitleDecoder(&p->pStreamInfoArr[p->nStreamSelected]);
    if(p->pDecoder == NULL)
    {
        loge("subtitle decoder component create decoder fail.");
        p->callback(p->pUserData, PLAYER_SUBTITLE_DECODER_NOTIFY_CRASH, NULL);
        if (msg->result)
            *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    //* send a decode message.
    BaseCompContinue(&p->base);
    p->bEosFlag = 0;
    p->eStatus  = PLAYER_STATUS_STARTED;

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);
}

static void handleStop(AwMessage *msg, void *arg)
{
    SubtitleDecComp* p = arg;

    if(p->eStatus == PLAYER_STATUS_STOPPED)
    {
        loge("invalid stop operation, already in stopped status.");
        if (msg->result)
            *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    if (p->pDecoder != NULL)
    {
        DestroySubtitleDecoder(p->pDecoder);
        p->pDecoder = NULL;
    }

    p->bCrashFlag = 0;
    p->eStatus = PLAYER_STATUS_STOPPED;

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);
}

static void handlePause(AwMessage *msg, void *arg)
{
    SubtitleDecComp* p = arg;

    if (p->eStatus != PLAYER_STATUS_STARTED)
    {
        loge("invalid pause operation, component not in started status.");
        if (msg->result)
            *msg->result = -1;
        sem_post(msg->replySem);
        return;
    }

    p->eStatus = PLAYER_STATUS_PAUSED;

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);
}

static void handleQuit(AwMessage *msg, void *arg)
{
    SubtitleDecComp* p = arg;

    if(p->pDecoder != NULL)
    {
        DestroySubtitleDecoder(p->pDecoder);
        p->pDecoder = NULL;
    }

    p->eStatus = PLAYER_STATUS_STOPPED;

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);

    pthread_exit(NULL);
}

static void handleReset(AwMessage *msg, void *arg)
{
    SubtitleDecComp* p = arg;
    int i;

    pthread_mutex_lock(&p->streamManagerMutex);
    for(i=0; i<p->nStreamCount; i++)
    {
        if(p->pStreamManagerArr[i] != NULL)
            StreamManagerReset(p->pStreamManagerArr[i]);
    }
    pthread_mutex_unlock(&p->streamManagerMutex);

    p->bEosFlag = 0;
    p->bCrashFlag = 0;
    ResetSubtitleDecoder(p->pDecoder, msg->seekTime);

    if (msg->result)
        *msg->result = 0;
    sem_post(msg->replySem);

    //* send a message to continue the thread.
    if(p->eStatus == PLAYER_STATUS_STARTED)
        BaseCompContinue(&p->base);
}

static void handleSetEos(AwMessage *msg, void *arg)
{
    SubtitleDecComp* p = arg;

    p->bEosFlag = 1;
    sem_post(msg->replySem);

    //* send a message to continue the thread.
    if(p->bCrashFlag == 0 && p->eStatus == PLAYER_STATUS_STARTED)
        BaseCompContinue(&p->base);
}

static void doDecode(AwMessage *msg, void *arg)
{
    int ret = 0;
    SubtitleDecComp* p = arg;
    (void)msg;

    StreamManager*         pSm;
    StreamFrame*           pFrame;
    SubtitleStreamDataInfo streamData;

    if(p->eStatus != PLAYER_STATUS_STARTED)
    {
        logw("not in started status, ignore decode message.");
        return;
    }

    pSm = p->pStreamManagerArr[p->nStreamSelected];
    pthread_mutex_lock(&p->streamManagerMutex);
    if (p->afterSwitchStream &&
            p->pAvTimer->GetStatus(p->pAvTimer) == TIMER_STATUS_START)
    {
        p->afterSwitchStream = 0;
        if (pSm != NULL)
        {
            int64_t nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
            StreamManagerRewind(pSm, nCurTime);
        }
    }
    pthread_mutex_unlock(&p->streamManagerMutex);

    if(p->pStreamInfoArr[p->nStreamSelected].bExternal == 1 &&
       p->pStreamInfoArr[p->nStreamSelected].eCodecFormat != SUBTITLE_CODEC_IDXSUB)
    {
        logv("p->nInternalSubtitleNum = %d, nExternalSubtitleNum = %d",
             p->nInternalSubtitleNum, p->nExternalSubtitleNum);
        //* we should call FlushStreamManagerBuffers in the thread
        //* when had internal subtitle
        if(p->nInternalSubtitleNum > 0)
        {
            if (AwMessageQueueWaitMessage(p->mq, 10) <= 0)
                BaseCompContinue(&p->base);
            return;
        }
        else
        {
            logw("decode message for external text subtitle, ignore.");
            return;
        }
    }

    if(pSm == NULL &&
               p->pStreamInfoArr[p->nStreamSelected].eCodecFormat != SUBTITLE_CODEC_IDXSUB)
    {
        loge("decode message for external text subtitle, ignore.");
        return;
    }

    if(pSm != NULL)    //* for index sub, pSm == NULL.
    {
        pFrame = StreamManagerGetFrameInfo(pSm, 0);
        if(pFrame == NULL)
        {
            if(p->bEosFlag)
            {
                logv("subtitle decoder notify eos.");
                p->callback(p->pUserData, PLAYER_SUBTITLE_DECODER_NOTIFY_EOS, NULL);
                return;
            }
            else
            {
                SemTimedWait(&p->streamDataSem, 1000);
                BaseCompContinue(&p->base);
                return;
            }
        }

        streamData.pData     = (char*)pFrame->pData;
        streamData.nLength   = pFrame->nLength;
        streamData.nPts      = pFrame->nPts;
        streamData.nPcr      = pFrame->nPcr;
        streamData.nDuration = pFrame->nDuration;
    }
    else
    {
        streamData.pData     = NULL;
        streamData.nLength   = 0;
        streamData.nPts      = -1;
        streamData.nPcr      = -1;
        streamData.nDuration = 0;
    }

    ret = DecodeSubtitleStream(p->pDecoder, &streamData);
    if (ret == SDECODE_RESULT_OK || ret == SDECODE_RESULT_FRAME_DECODED)
    {
        if(pSm)
        {
            pFrame = StreamManagerRequestStream(pSm);
            StreamManagerFlushStream(pSm, pFrame);
        }
        BaseCompContinue(&p->base);
        return;
    }
    else if(ret == SDECODE_RESULT_NO_FRAME_BUFFER)
    {
        //* no item buffer, wait for the item buffer semaphore.
        logi("no subtitle item buffer, wait.");
        SemTimedWait(&p->frameBufferSem, 100);    //* wait for frame buffer.
        BaseCompContinue(&p->base);
        return;
    }
    else if(ret == SDECODE_RESULT_UNSUPPORTED)
    {
        logw("DecodeSubtitleStream() return fatal error.");
        p->bCrashFlag = 1;
        p->callback(p->pUserData, PLAYER_SUBTITLE_DECODER_NOTIFY_CRASH, NULL);
        return;
    }
    else
    {
        logw("DecodeSubtitleStream() return %d, continue to decode", ret);
        if(pSm)
        {
            pFrame = StreamManagerRequestStream(pSm);
            StreamManagerFlushStream(pSm, pFrame);
        }
        BaseCompContinue(&p->base);
    }
}

#if 0
static void FlushStreamManagerBuffers(SubtitleDecComp* p,
        int64_t curTime, int bIncludeSeletedStream)
{
    //* to prevent from flush incorrectly when pts loop back,
    //* we find the frame who's pts is near the current timer value,
    //* and flush frames before this frame.

    int            i;
    int            nFrameIndex;
    int            nFlushPos;
    int            nFrameCount;
    StreamFrame*   pFrame;
    StreamManager* pSm;
    int64_t        nMinPtsDiff;
    int64_t        nPtsDiff;

    for(i=0; i<p->nStreamCount; i++)
    {
        if(i == p->nStreamSelected && bIncludeSeletedStream == 0)
            continue;

        pSm = p->pStreamManagerArr[i];
        if(pSm == NULL)
            continue;   //* external subtitle.

        nFrameCount = StreamManagerStreamFrameNum(pSm);
        nMinPtsDiff = 0x7fffffffffffffffLL; //* set it to the max value.
        nFlushPos   = nFrameCount;
        for(nFrameIndex=0; nFrameIndex<nFrameCount; nFrameIndex++)
        {
            pFrame = StreamManagerGetFrameInfo(pSm, nFrameIndex);
            if(pFrame->nPts == -1)
                continue;

            nPtsDiff = pFrame->nPts - curTime;
            if(nPtsDiff >= 0 && nPtsDiff < nMinPtsDiff)
            {
                nMinPtsDiff = nPtsDiff;
                nFlushPos   = nFrameIndex;
            }
        }

        //* flush frames before nFlushPos.
        for(nFrameIndex=0; nFrameIndex<nFlushPos; nFrameIndex++)
        {
            pFrame = StreamManagerRequestStream(pSm);
            StreamManagerFlushStream(pSm, pFrame);
        }
    }

    return;
}
#endif
