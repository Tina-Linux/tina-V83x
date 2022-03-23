
#include "log.h"

#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>

#include "subtitleDecComponent.h"
#include "sdecoder.h"

#include "messageQueue.h"
#include "streamManager.h"

static const int MAX_SUBTITLE_STREAM_BUFFER_SIZE (2048*1024);
static const int MAX_SUBTITLE_STREAM_FRAME_COUNT (2048);

typedef struct SubtitleDecCompContext
{
	//* created at initialize time.
    MessageQueue*           mq;
    sem_t                   startMsgReplySem;
    sem_t                   stopMsgReplySem;
    sem_t                   pauseMsgReplySem;
    sem_t                   resetMsgReplySem;
    sem_t                   eosMsgReplySem;
    sem_t                   quitMsgReplySem;
    sem_t                   streamDataSem;
    sem_t                   frameBufferSem;

    int                     nStartReply;
    int                     nStopReply;
    int                     nPauseReply;
    int                     nResetReply;

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

}SubtitleDecCompContext;


static void* SubtitleDecodeThread(void* arg);
static void PostDecodeMessage(MessageQueue* mq);
static void FlushStreamManagerBuffers(SubtitleDecCompContext* p, int64_t curTime, int bIncludeSeletedStream);


SubtitleDecComp* SubtitleDecCompCreate(void)
{
    SubtitleDecCompContext* p;
    int                     err;

    p = (SubtitleDecCompContext*)malloc(sizeof(SubtitleDecCompContext));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }
    memset(p, 0, sizeof(*p));

    p->mq = MessageQueueCreate(4, "SubtitleDecodeMq");
    if(p->mq == NULL)
    {
        loge("subtitle decoder component create message queue fail.");
        free(p);
        return NULL;
    }

    sem_init(&p->startMsgReplySem, 0, 0);
    sem_init(&p->stopMsgReplySem, 0, 0);
    sem_init(&p->pauseMsgReplySem, 0, 0);
    sem_init(&p->quitMsgReplySem, 0, 0);
    sem_init(&p->resetMsgReplySem, 0, 0);
    sem_init(&p->eosMsgReplySem, 0, 0);
    sem_init(&p->streamDataSem, 0, 0);
    sem_init(&p->frameBufferSem, 0, 0);
    pthread_mutex_init(&p->streamManagerMutex, NULL);

    p->eStatus = PLAYER_STATUS_STOPPED;

    err = pthread_create(&p->sDecodeThread, NULL, SubtitleDecodeThread, p);
    if(err != 0)
    {
        loge("subtitle decode component create thread fail.");
        sem_destroy(&p->startMsgReplySem);
        sem_destroy(&p->stopMsgReplySem);
        sem_destroy(&p->pauseMsgReplySem);
        sem_destroy(&p->quitMsgReplySem);
        sem_destroy(&p->resetMsgReplySem);
        sem_destroy(&p->eosMsgReplySem);
        sem_destroy(&p->streamDataSem);
        sem_destroy(&p->frameBufferSem);
        pthread_mutex_destroy(&p->streamManagerMutex);
        MessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    return (SubtitleDecComp*)p;
}


void SubtitleDecCompDestroy(SubtitleDecComp* s)
{
    void*                   status;
    SubtitleDecCompContext* p;
    Message                 msg;
    int                     i;

    p = (SubtitleDecCompContext*)s;

    msg.messageId = MESSAGE_ID_QUIT;
    msg.params[0] = (uintptr_t)&p->quitMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    SemTimedWait(&p->quitMsgReplySem, -1);
    pthread_join(p->sDecodeThread, &status);

    sem_destroy(&p->startMsgReplySem);
    sem_destroy(&p->stopMsgReplySem);
    sem_destroy(&p->pauseMsgReplySem);
    sem_destroy(&p->quitMsgReplySem);
    sem_destroy(&p->resetMsgReplySem);
    sem_destroy(&p->eosMsgReplySem);
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
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
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

    MessageQueueDestroy(p->mq);
    free(p);

    return;
}


int SubtitleDecCompStart(SubtitleDecComp* s)
{
    SubtitleDecCompContext* p;
    Message                 msg;

    p = (SubtitleDecCompContext*)s;

    logv("subtitle decode component starting");

    msg.messageId = MESSAGE_ID_START;
    msg.params[0] = (uintptr_t)&p->startMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStartReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->startMsgReplySem, -1) < 0)
    {
        loge("subtitle decode component wait for start finish timeout.");
        return -1;
    }

    return p->nStartReply;
}


int SubtitleDecCompStop(SubtitleDecComp* s)
{
    SubtitleDecCompContext* p;
    Message             msg;

    p = (SubtitleDecCompContext*)s;

    logv("subtitle decode component stopping");

    msg.messageId = MESSAGE_ID_STOP;
    msg.params[0] = (uintptr_t)&p->stopMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStopReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->stopMsgReplySem, -1) < 0)
    {
        loge("subtitle decode component wait for stop finish timeout.");
        return -1;
    }

    return p->nStopReply;
}


int SubtitleDecCompPause(SubtitleDecComp* s)
{
    SubtitleDecCompContext* p;
    Message             msg;

    p = (SubtitleDecCompContext*)s;

    logv("subtitle decode component pausing");

    msg.messageId = MESSAGE_ID_PAUSE;
    msg.params[0] = (uintptr_t)&p->pauseMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nPauseReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->pauseMsgReplySem, -1) < 0)
    {
        loge("subtitle decode component wait for pause finish timeout.");
        return -1;
    }

    return p->nPauseReply;
}


enum EPLAYERSTATUS SubtitleDecCompGetStatus(SubtitleDecComp* s)
{
    SubtitleDecCompContext* p;
    p = (SubtitleDecCompContext*)s;
    return p->eStatus;
}


int SubtitleDecCompReset(SubtitleDecComp* s, int64_t nSeekTime)
{
    SubtitleDecCompContext* p;
    Message             msg;

    p = (SubtitleDecCompContext*)s;

    logv("subtitle decode component reseting");

    msg.messageId = MESSAGE_ID_RESET;
    msg.params[0] = (uintptr_t)&p->resetMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nResetReply;
    msg.params[2] = (uintptr_t)(nSeekTime & 0xffffffff);
    msg.params[3] = (uintptr_t)(nSeekTime>>32);

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->resetMsgReplySem, -1) < 0)
    {
        loge("subtitle decode component wait for reset finish timeout.");
        return -1;
    }

    return p->nResetReply;
}


int SubtitleDecCompSetEOS(SubtitleDecComp* s)
{
    SubtitleDecCompContext* p;
    Message             msg;

    p = (SubtitleDecCompContext*)s;

    logv("subtitle decode component setting EOS.");

    msg.messageId = MESSAGE_ID_EOS;
    msg.params[0] = (uintptr_t)&p->eosMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->eosMsgReplySem, -1) < 0)
    {
        loge("subtitle decode component wait for setting eos finish timeout.");
        return -1;
    }

    return 0;
}


int SubtitleDecCompSetCallback(SubtitleDecComp* s, PlayerCallback callback, void* pUserData)
{
    SubtitleDecCompContext* p;

    p = (SubtitleDecCompContext*)s;

    p->callback  = callback;
    p->pUserData = pUserData;

    return 0;
}


int SubtitleDecCompSetSubtitleStreamInfo(SubtitleDecComp*     s,
                                         SubtitleStreamInfo*  pStreamInfo,
                                         int                  nStreamCount,
                                         int                  nDefaultStreamIndex)
{
    SubtitleDecCompContext* p;
    int                     i;

    p = (SubtitleDecCompContext*)s;

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
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
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
    p->pStreamInfoArr = (SubtitleStreamInfo*)malloc(sizeof(SubtitleStreamInfo)*nStreamCount);
    if(p->pStreamInfoArr == NULL)
    {
        loge("memory malloc fail!");
        return -1;
    }
    memset(p->pStreamInfoArr, 0, sizeof(SubtitleStreamInfo)*nStreamCount);

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
        if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
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
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
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
    p->pStreamManagerArr = (StreamManager**)malloc(nStreamCount*sizeof(StreamManager*));
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
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
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
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
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


int SubtitleDecCompAddSubtitleStream(SubtitleDecComp* s, SubtitleStreamInfo* pStreamInfo)
{
    SubtitleDecCompContext* p;

    p = (SubtitleDecCompContext*)s;

    pthread_mutex_lock(&p->streamManagerMutex);

    if(p->nStreamCount > 0)
    {
        SubtitleStreamInfo* pStreamInfoArr;
        StreamManager**     pStreamManagerArr;
        int                 nStreamCount;

        nStreamCount = p->nStreamCount + 1;
        pStreamManagerArr = (StreamManager**)malloc(sizeof(StreamManager*)*nStreamCount);
        if(pStreamManagerArr == NULL)
        {
            loge("malloc memory fail.");
            pthread_mutex_unlock(&p->streamManagerMutex);
            return -1;
        }

        pStreamInfoArr = (SubtitleStreamInfo*)malloc(sizeof(SubtitleStreamInfo)*nStreamCount);
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

        memcpy(pStreamInfoArr, p->pStreamInfoArr, p->nStreamCount*sizeof(AudioStreamInfo));
        memcpy(&pStreamInfoArr[nStreamCount-1], pStreamInfo, sizeof(AudioStreamInfo));
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
		pStreamInfoArr[nStreamCount-1].nCodecSpecificDataLen = pStreamInfo->nCodecSpecificDataLen;
		pStreamInfoArr[nStreamCount-1].pCodecSpecificData = (char*)malloc(pStreamInfo->nCodecSpecificDataLen);
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
        return SubtitleDecCompSetSubtitleStreamInfo(s, pStreamInfo, 1, 0);
    }
}


int SubtitleDecCompGetSubtitleStreamCnt(SubtitleDecComp* s)
{
    SubtitleDecCompContext* p;
    p = (SubtitleDecCompContext*)s;
    return p->nStreamCount;
}


int SubtitleDecCompCurrentStreamIndex(SubtitleDecComp* s)
{
    SubtitleDecCompContext* p;
    p = (SubtitleDecCompContext*)s;
    return p->nStreamSelected;
}


int SubtitleDecCompGetSubtitleStreamInfo(SubtitleDecComp* s, int* pStreamNum, SubtitleStreamInfo** ppStreamInfo)
{
    SubtitleDecCompContext* p;
    int                     i;
    SubtitleStreamInfo*     pStreamInfo;
    int                     nStreamCount;

    p = (SubtitleDecCompContext*)s;
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
        if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
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
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
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


int SubtitleDecCompSetTimer(SubtitleDecComp* s, AvTimer* timer)
{
    SubtitleDecCompContext* p;
    p = (SubtitleDecCompContext*)s;
    p->pAvTimer = timer;
    return 0;
}


int SubtitleDecCompRequestStreamBuffer(SubtitleDecComp* s,
                                   int          nRequireSize,
                                   char**       ppBuf,
                                   int*         pBufSize,
                                   int          nStreamIndex)
{
    SubtitleDecCompContext* p;
    StreamManager*          pSm;
    StreamFrame*            pTmpFrame;
    char*                   pBuf;
    int                     nBufSize;

    p = (SubtitleDecCompContext*)s;

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
        loge("stream index invalid, stream index = %d, audio stream num = %d",
                    nStreamIndex, p->nStreamCount);
        pthread_mutex_unlock(&p->streamManagerMutex);
        return -1;
    }

    if(p->bCrashFlag)
    {
        //* when decoder crashed, the main thread is not running,
        //* we need to flush stream buffers for demux keep going.
        if(p->pAvTimer->GetStatus(p->pAvTimer) == TIMER_STATUS_START)
        {
            int nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
            FlushStreamManagerBuffers(p, nCurTime, 1);
        }
    }

    if(p->pStreamInfoArr[nStreamIndex].bExternal)
    {
        loge("subtitle stream %d is an external subtitle, request stream buffer fail.", nStreamIndex);
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


int SubtitleDecCompSubmitStreamData(SubtitleDecComp* s, SubtitleStreamDataInfo* pDataInfo, int nStreamIndex)
{
    int                     nSemCnt;
    SubtitleDecCompContext* p;
    StreamManager*          pSm;
    StreamFrame             streamFrame;

    p = (SubtitleDecCompContext*)s;

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


SubtitleItem* SubtitleDecCompRequestSubtitleItem(SubtitleDecComp* s)
{
    SubtitleDecCompContext* p;
    SubtitleItem*           pItem;

    p = (SubtitleDecCompContext*)s;

    pItem = RequestSubtitleItem(p->pDecoder);
    return pItem;
}


void SubtitleDecCompFlushSubtitleItem(SubtitleDecComp* s, SubtitleItem* pItem)
{
    int                     ret;
    int                     nSemCnt;
    SubtitleDecCompContext* p;

    p = (SubtitleDecCompContext*)s;

    FlushSubtitleItem(p->pDecoder, pItem);
    if(sem_getvalue(&p->frameBufferSem, &nSemCnt) == 0)
    {
        if(nSemCnt == 0)
            sem_post(&p->frameBufferSem);
    }

    return;
}


SubtitleItem* SubtitleDecCompNextSubtitleItem(SubtitleDecComp* s)
{
    SubtitleDecCompContext* p;
    SubtitleItem*           pItem;

    p = (SubtitleDecCompContext*)s;

    pItem = NextSubtitleItem(p->pDecoder);
    return pItem;
}


//* must be called at stopped status.
int SubtitleDecCompSwitchStream(SubtitleDecComp* s, int nStreamIndex)
{
    SubtitleDecCompContext* p;
    p = (SubtitleDecCompContext*)s;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("can not switch status when subtitle decoder is not in stopped status.");
        return -1;
    }

    pthread_mutex_lock(&p->streamManagerMutex);
    p->nStreamSelected = nStreamIndex;
    pthread_mutex_unlock(&p->streamManagerMutex);
    return 0;
}


int SubtitleDecCompStreamBufferSize(SubtitleDecComp* s, int nStreamIndex)
{
    SubtitleDecCompContext* p;
    StreamManager*          pSm;

    p = (SubtitleDecCompContext*)s;

    pSm = p->pStreamManagerArr[nStreamIndex];
    if(pSm != NULL)
        return StreamManagerBufferSize(pSm);
    else
        return 0;   //* external subtitle.
}


int SubtitleDecCompStreamDataSize(SubtitleDecComp* s, int nStreamIndex)
{
    SubtitleDecCompContext* p;
    int                     nStreamDataSize;

    p = (SubtitleDecCompContext*)s;

    nStreamDataSize = 0;
    if(p->pStreamManagerArr[nStreamIndex] != NULL)
        nStreamDataSize = StreamManagerStreamDataSize(p->pStreamManagerArr[nStreamIndex]);

    return nStreamDataSize;
}


int SubtitleDecCompIsExternalSubtitle(SubtitleDecComp* s, int nStreamIndex)
{
    SubtitleDecCompContext* p;

    p = (SubtitleDecCompContext*)s;

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

int SubtitleDecCompGetExternalFlag(SubtitleDecComp* s)
{
	SubtitleDecCompContext* p;

    p = (SubtitleDecCompContext*)s;
	return p->pStreamInfoArr[p->nStreamSelected].bExternal;
}


static void* SubtitleDecodeThread(void* arg)
{
    SubtitleDecCompContext* p;
    Message                 msg;
    int                     ret;
    sem_t*                  pReplySem;
    int*                    pReplyValue;
    int64_t                 nCurTime;

    p = (SubtitleDecCompContext*)arg;

    while(1)
    {
        if(MessageQueueGetMessage(p->mq, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

        pReplySem   = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == MESSAGE_ID_START)
        {
            //* check status.
            if(p->eStatus == PLAYER_STATUS_STARTED)
            {
                loge("invalid start operation, already in started status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            if(p->eStatus == PLAYER_STATUS_PAUSED)
            {
                //* send a decode message to start decoding.
                //* when had internal subtitle, also send decode message
                if(p->pStreamInfoArr[p->nStreamSelected].bExternal == 0
                   || p->pStreamInfoArr[p->nStreamSelected].eCodecFormat == SUBTITLE_CODEC_IDXSUB
                   || p->nInternalSubtitleNum > 0)
                {
                    if(p->bCrashFlag == 0)
                        PostDecodeMessage(p->mq);
                }
                p->eStatus = PLAYER_STATUS_STARTED;
                if(pReplyValue != NULL)
                    *pReplyValue = 0;
                sem_post(pReplySem);
                continue;
            }

            //* create a decoder.
            p->pDecoder = CreateSubtitleDecoder(&p->pStreamInfoArr[p->nStreamSelected]);
            if(p->pDecoder == NULL)
            {
                loge("subtitle decoder component create decoder fail.");
                p->callback(p->pUserData, PLAYER_SUBTITLE_DECODER_NOTIFY_CRASH, NULL);
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            //* send a decode message.
            PostDecodeMessage(p->mq);
            p->bEosFlag = 0;
            p->eStatus  = PLAYER_STATUS_STARTED;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_STOP)
        {
            //* check status.
            if(p->eStatus == PLAYER_STATUS_STOPPED)
            {
                loge("invalid stop operation, already in stopped status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            //* destroy decoder.
            if(p->pDecoder != NULL)
            {
                DestroySubtitleDecoder(p->pDecoder);
                p->pDecoder = NULL;
            }

            p->bCrashFlag = 0;
            p->eStatus = PLAYER_STATUS_STOPPED;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_PAUSE)
        {
            //* check status.
            if(p->eStatus != PLAYER_STATUS_STARTED)
            {
                loge("invalid pause operation, component not in started status.");
                if(pReplyValue != NULL)
                    *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            p->eStatus = PLAYER_STATUS_PAUSED;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_QUIT)
        {
            //* destroy decoder and break.
            if(p->pDecoder != NULL)
            {
                DestroySubtitleDecoder(p->pDecoder);
                p->pDecoder = NULL;
            }

            p->eStatus = PLAYER_STATUS_STOPPED;
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);
            break;
        }
        else if(msg.messageId == MESSAGE_ID_RESET)
        {
            int     i;
            int64_t nSeekTime;

            nSeekTime = msg.params[2] + (((int64_t)msg.params[3])<<32);

            pthread_mutex_lock(&p->streamManagerMutex);
            for(i=0; i<p->nStreamCount; i++)
            {
                if(p->pStreamManagerArr[i] != NULL)
                    StreamManagerReset(p->pStreamManagerArr[i]);
            }
            pthread_mutex_unlock(&p->streamManagerMutex);

            p->bEosFlag = 0;
            p->bCrashFlag = 0;
            ResetSubtitleDecoder(p->pDecoder, nSeekTime);
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED)
                PostDecodeMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_EOS)
        {
            p->bEosFlag = 1;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->bCrashFlag == 0)
            {
                if(p->eStatus == PLAYER_STATUS_STARTED)
                    PostDecodeMessage(p->mq);
            }
        }
        else if(msg.messageId == MESSAGE_ID_DECODE)
        {
            StreamManager*         pSm;
            StreamFrame*           pFrame;
            SubtitleStreamDataInfo streamData;

            logv("process decode message.");
            if(p->eStatus != PLAYER_STATUS_STARTED)
            {
                logw("not in started status, ignore decode message.");
                continue;
            }

            //* flush stream manager buffers.
            pthread_mutex_lock(&p->streamManagerMutex);
            if(p->pAvTimer->GetStatus(p->pAvTimer) == TIMER_STATUS_START)
            {
                //* only flush stream when the timer is started,
                //* the timer is started at the moment the first audio frame is received.
                nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
                FlushStreamManagerBuffers(p, nCurTime, 0);
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
                    usleep(10*1000);
                    PostDecodeMessage(p->mq);
                    continue;
                }
                else
                {
                    logw("decode message for external text subtitle, ignore.");
                    continue;
                }
            }

            pSm = p->pStreamManagerArr[p->nStreamSelected];
            if(pSm == NULL &&
               p->pStreamInfoArr[p->nStreamSelected].eCodecFormat != SUBTITLE_CODEC_IDXSUB)
            {
                loge("decode message for external text subtitle, ignore.");
                continue;
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
                        continue;
                    }
                    else
                    {
                        SemTimedWait(&p->streamDataSem, 1000);
                        PostDecodeMessage(p->mq);
                        continue;
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
            if(ret == SDECODE_RESULT_OK || ret == SDECODE_RESULT_FRAME_DECODED)
            {
                if(pSm)
                {
                    pFrame = StreamManagerRequestStream(pSm);
                    StreamManagerFlushStream(pSm, pFrame);
                }
                PostDecodeMessage(p->mq);
                continue;
            }
            else if(ret == SDECODE_RESULT_NO_FRAME_BUFFER)
            {
                //* no item buffer, wait for the item buffer semaphore.
                logi("no subtitle item buffer, wait.");
                SemTimedWait(&p->frameBufferSem, 100);    //* wait for frame buffer.
                PostDecodeMessage(p->mq);
                continue;
            }
            else if(ret == SDECODE_RESULT_UNSUPPORTED)
            {
                logw("DecodeSubtitleStream() return fatal error.");
                p->bCrashFlag = 1;
                p->callback(p->pUserData, PLAYER_SUBTITLE_DECODER_NOTIFY_CRASH, NULL);
                continue;
            }
            else
            {
                logw("DecodeSubtitleStream() return %d, continue to decode", ret);
                if(pSm)
                {
                    pFrame = StreamManagerRequestStream(pSm);
                    StreamManagerFlushStream(pSm, pFrame);
                }
                PostDecodeMessage(p->mq);
            }
        }
        else
        {
            loge("unknown message in subtitle decode thread.");
            abort();
        }
    }

    ret = 0;
    pthread_exit(&ret);
    return NULL;
}


static void PostDecodeMessage(MessageQueue* mq)
{
    if(MessageQueueGetCount(mq)<=0)
    {
        Message msg;
        msg.messageId = MESSAGE_ID_DECODE;
        msg.params[0] = msg.params[1] = msg.params[2] = msg.params[3] = 0;
        if(MessageQueuePostMessage(mq, &msg) != 0)
        {
            loge("fatal error, subtitle decode component post message fail.");
            abort();
        }

        return;
    }
}


static void FlushStreamManagerBuffers(SubtitleDecCompContext* p, int64_t curTime, int bIncludeSeletedStream)
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
