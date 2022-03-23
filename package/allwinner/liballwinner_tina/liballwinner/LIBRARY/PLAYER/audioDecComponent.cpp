
#include "log.h"

#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>

#include "audioDecComponent.h"
#include "adecoder.h"

#include "messageQueue.h"
#include "streamManager.h"

static const int MAX_AUDIO_STREAM_BUFFER_SIZE (2048*1024);
static const int MAX_AUDIO_STREAM_FRAME_COUNT (2048);

typedef struct AudioDecCompContext
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

    int                     nOffset;
    BsInFor                 bsInfo;
    AudioDecoder*           pDecoder;
    int                     nStreamCount;
    int                     nStreamSelected;

    AudioStreamInfo*        pStreamInfoArr;
    StreamManager**         pStreamManagerArr;
    pthread_mutex_t         streamManagerMutex;

    pthread_mutex_t         decoderDestroyMutex;    //* to protect decoder from destroyed.

    int                     bCrashFlag;

}AudioDecCompContext;

static void* AudioDecodeThread(void* arg);
static void PostDecodeMessage(MessageQueue* mq);
static void FlushStreamManagerBuffers(AudioDecCompContext* p, int64_t curTime, int bIncludeSeletedStream);


AudioDecComp* AudioDecCompCreate(void)
{
    AudioDecCompContext* p;
    int                  err;

    p = (AudioDecCompContext*)malloc(sizeof(AudioDecCompContext));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }
    memset(p, 0, sizeof(*p));

    p->mq = MessageQueueCreate(4, "AudioDecodeMq");
    if(p->mq == NULL)
    {
        loge("audio decoder component create message queue fail.");
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
    pthread_mutex_init(&p->decoderDestroyMutex, NULL);

    p->eStatus = PLAYER_STATUS_STOPPED;

    err = pthread_create(&p->sDecodeThread, NULL, AudioDecodeThread, p);
    if(err != 0)
    {
        loge("audio decode component create thread fail.");
        sem_destroy(&p->startMsgReplySem);
        sem_destroy(&p->stopMsgReplySem);
        sem_destroy(&p->pauseMsgReplySem);
        sem_destroy(&p->quitMsgReplySem);
        sem_destroy(&p->resetMsgReplySem);
        sem_destroy(&p->eosMsgReplySem);
        sem_destroy(&p->streamDataSem);
        sem_destroy(&p->frameBufferSem);
        pthread_mutex_destroy(&p->streamManagerMutex);
        pthread_mutex_destroy(&p->decoderDestroyMutex);
        MessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    return (AudioDecComp*)p;
}


int AudioDecCompDestroy(AudioDecComp* a)
{
    void*                status;
    AudioDecCompContext* p;
    Message              msg;
    int                  i;

    p = (AudioDecCompContext*)a;

    msg.messageId = MESSAGE_ID_QUIT;
    msg.params[0] = (uintptr_t)&p->quitMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio decode component post message fail.");
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
    pthread_mutex_destroy(&p->decoderDestroyMutex);

    if(p->pStreamInfoArr != NULL)
    {
        for(i=0; i<p->nStreamCount; i++)
        {
            if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
            {
                free(p->pStreamInfoArr[i].pCodecSpecificData);
                p->pStreamInfoArr[i].pCodecSpecificData = NULL;
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

    return 0;
}


int AudioDecCompStart(AudioDecComp* a)
{
    AudioDecCompContext* p;
    Message              msg;

    p = (AudioDecCompContext*)a;

    logv("audio decode component starting");

    msg.messageId = MESSAGE_ID_START;
    msg.params[0] = (uintptr_t)&p->startMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStartReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->startMsgReplySem, -1) < 0)
    {
        loge("audio decode component wait for start finish timeout.");
        return -1;
    }

    return p->nStartReply;
}


int AudioDecCompStop(AudioDecComp* a)
{
    AudioDecCompContext* p;
    Message              msg;

    p = (AudioDecCompContext*)a;

    logv("audio decode component stopping");

    msg.messageId = MESSAGE_ID_STOP;
    msg.params[0] = (uintptr_t)&p->stopMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStopReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->stopMsgReplySem, -1) < 0)
    {
        loge("audio decode component wait for stop finish timeout.");
        return -1;
    }

    return p->nStopReply;
}


int AudioDecCompPause(AudioDecComp* a)
{
    AudioDecCompContext* p;
    Message              msg;

    p = (AudioDecCompContext*)a;

    logv("audio decode component pausing");

    msg.messageId = MESSAGE_ID_PAUSE;
    msg.params[0] = (uintptr_t)&p->pauseMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nPauseReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->pauseMsgReplySem, -1) < 0)
    {
        loge("audio decode component wait for pause finish timeout.");
        return -1;
    }

    return p->nPauseReply;
}


enum EPLAYERSTATUS AudioDecCompGetStatus(AudioDecComp* a)
{
    AudioDecCompContext* p;
    p = (AudioDecCompContext*)a;
    return p->eStatus;
}


int AudioDecCompReset(AudioDecComp* a, int64_t nSeekTime)
{
    AudioDecCompContext* p;
    Message              msg;

    p = (AudioDecCompContext*)a;

    logv("audio decode component reseting");

    msg.messageId = MESSAGE_ID_RESET;
    msg.params[0] = (uintptr_t)&p->resetMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nResetReply;
    msg.params[2] = (uintptr_t)(nSeekTime & 0xffffffff);
    msg.params[3] = (uintptr_t)(nSeekTime>>32);
    //msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->resetMsgReplySem, -1) < 0)
    {
        loge("audio decode component wait for reset finish timeout.");
        return -1;
    }

    return p->nResetReply;
}


int AudioDecCompSetEOS(AudioDecComp* a)
{
    AudioDecCompContext* p;
    Message              msg;

    p = (AudioDecCompContext*)a;

    logv("audio decode component setting EOS.");

    msg.messageId = MESSAGE_ID_EOS;
    msg.params[0] = (uintptr_t)&p->eosMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio decode component post message fail.");
        abort();
    }

    //* wake up the thread if it is pending for stream data or frame buffer.
    sem_post(&p->streamDataSem);
    sem_post(&p->frameBufferSem);

    if(SemTimedWait(&p->eosMsgReplySem, -1) < 0)
    {
        loge("audio decode component wait for setting eos finish timeout.");
        return -1;
    }

    return 0;
}


int AudioDecCompSetCallback(AudioDecComp* a, PlayerCallback callback, void* pUserData)
{
    AudioDecCompContext* p;

    p = (AudioDecCompContext*)a;

    p->callback  = callback;
    p->pUserData = pUserData;

    return 0;
}


int AudioDecCompSetAudioStreamInfo(AudioDecComp*    a,
                                   AudioStreamInfo* pStreamInfo,
                                   int              nStreamCount,
                                   int              nDefaultStreamIndex)
{
    AudioDecCompContext* p;
    int                  i;

    p = (AudioDecCompContext*)a;

    if(p->pStreamInfoArr != NULL && p->nStreamCount > 0)
    {
        for(i=0; i<p->nStreamCount; i++)
        {
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

    p->pStreamInfoArr = (AudioStreamInfo*)malloc(sizeof(AudioStreamInfo)*nStreamCount);
    if(p->pStreamInfoArr == NULL)
    {
        loge("memory malloc fail!");
        return -1;
    }
    memset(p->pStreamInfoArr, 0, sizeof(AudioStreamInfo)*nStreamCount);

    for(i=0; i<nStreamCount; i++)
    {
        memcpy(&p->pStreamInfoArr[i], &pStreamInfo[i], sizeof(AudioStreamInfo));
        if(pStreamInfo[i].pCodecSpecificData != NULL && pStreamInfo[i].nCodecSpecificDataLen > 0)
        {
            p->pStreamInfoArr[i].pCodecSpecificData = (char*)malloc(pStreamInfo[i].nCodecSpecificDataLen);
            if(p->pStreamInfoArr[i].pCodecSpecificData == NULL)
            {
                loge("malloc memory fail.");
                p->pStreamInfoArr[i].nCodecSpecificDataLen = 0;
                break;
            }
            memcpy(p->pStreamInfoArr[i].pCodecSpecificData,
                   pStreamInfo[i].pCodecSpecificData,
                   pStreamInfo[i].nCodecSpecificDataLen);
        }
    }

    if(i != nStreamCount)
    {
        //* memory alloc fail break.
        i--;
        for(; i>=0; i--)
        {
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

    p->pStreamManagerArr = (StreamManager**)malloc(nStreamCount*sizeof(StreamManager*));
    if(p->pStreamManagerArr == NULL)
    {
        loge("malloc memory fail.");
        for(i=0; i<nStreamCount; i++)
        {
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
        p->pStreamManagerArr[i] = StreamManagerCreate(MAX_AUDIO_STREAM_BUFFER_SIZE,
                                                      MAX_AUDIO_STREAM_FRAME_COUNT,
                                                      i);
        if(p->pStreamManagerArr[i] == NULL)
        {
            loge("create stream manager for audio stream %d fail", i);
            break;
        }
    }

    if(i != nStreamCount)
    {
        //* memory alloc fail break.
        i--;
        for(; i>=0; i--)
        {
            StreamManagerDestroy(p->pStreamManagerArr[i]);
            p->pStreamManagerArr[i] = NULL;
        }
        free(p->pStreamManagerArr);
        p->pStreamManagerArr = NULL;

        for(i=0; i<nStreamCount; i++)
        {
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

    p->nStreamSelected = nDefaultStreamIndex;
    p->nStreamCount = nStreamCount;

    return 0;
}


int AudioDecCompAddAudioStream(AudioDecComp* a, AudioStreamInfo* pStreamInfo)
{
    AudioDecCompContext* p;

    p = (AudioDecCompContext*)a;

    pthread_mutex_lock(&p->streamManagerMutex);

    if(p->nStreamCount > 0)
    {
        AudioStreamInfo*  pStreamInfoArr;
        StreamManager**   pStreamManagerArr;
        int               nStreamCount;

        nStreamCount = p->nStreamCount + 1;
        pStreamManagerArr = (StreamManager**)malloc(sizeof(StreamManager*)*nStreamCount);
        if(pStreamManagerArr == NULL)
        {
            loge("malloc memory fail.");
            pthread_mutex_unlock(&p->streamManagerMutex);
            return -1;
        }

        pStreamInfoArr = (AudioStreamInfo*)malloc(sizeof(AudioStreamInfo)*nStreamCount);
        if(pStreamInfoArr == NULL)
        {
            loge("malloc memory fail.");
            free(pStreamManagerArr);
            pthread_mutex_unlock(&p->streamManagerMutex);
            return -1;
        }

        memcpy(pStreamManagerArr, p->pStreamManagerArr, p->nStreamCount * sizeof(StreamManager*));
        pStreamManagerArr[nStreamCount-1] = StreamManagerCreate(MAX_AUDIO_STREAM_BUFFER_SIZE,
                                                                MAX_AUDIO_STREAM_FRAME_COUNT,
                                                                nStreamCount-1);
        if(pStreamManagerArr[nStreamCount-1] == NULL)
        {
            loge("create stream manager fail.");
            free(pStreamManagerArr);
            free(pStreamInfoArr);
            pthread_mutex_unlock(&p->streamManagerMutex);
            return -1;
        }

        memcpy(pStreamInfoArr, p->pStreamInfoArr, p->nStreamCount*sizeof(AudioStreamInfo));
        memcpy(&pStreamInfoArr[nStreamCount-1], pStreamInfo, sizeof(AudioStreamInfo));
        if(pStreamInfo->pCodecSpecificData != NULL && pStreamInfo->nCodecSpecificDataLen > 0)
        {
            pStreamInfoArr[nStreamCount-1].pCodecSpecificData = (char*)malloc(pStreamInfo->nCodecSpecificDataLen);
            if(pStreamInfoArr[nStreamCount-1].pCodecSpecificData == NULL)
            {
                loge("malloc memory fail.");
                free(pStreamManagerArr);
                free(pStreamInfoArr);
                pthread_mutex_unlock(&p->streamManagerMutex);
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
        return AudioDecCompSetAudioStreamInfo(a, pStreamInfo, 1, 0);
    }
}


int AudioDecCompGetAudioStreamCnt(AudioDecComp* a)
{
    AudioDecCompContext* p;
    p = (AudioDecCompContext*)a;
    return p->nStreamCount;
}


int AudioDecCompCurrentStreamIndex(AudioDecComp* a)
{
    AudioDecCompContext* p;
    p = (AudioDecCompContext*)a;
    return p->nStreamSelected;
}


int AudioDecCompGetAudioSampleRate(AudioDecComp* a,
                                   unsigned int* pSampleRate,
                                   unsigned int* pChannelNum,
                                   unsigned int* pBitRate)
{
    AudioDecCompContext* p;
    int                  i;

    p = (AudioDecCompContext*)a;
    if(p->nStreamCount <= 0)
        return -1;
    i = p->nStreamSelected;
    *pSampleRate = p->pStreamInfoArr[i].nSampleRate;
    *pChannelNum = p->pStreamInfoArr[i].nChannelNum;
    *pBitRate    = p->pStreamInfoArr[i].nAvgBitrate;
    return 0;
}


int AudioDecCompGetAudioStreamInfo(AudioDecComp* a, int* pStreamNum, AudioStreamInfo** ppStreamInfo)
{
    AudioDecCompContext* p;
    int                  i;
    AudioStreamInfo*     pStreamInfo;
    int                  nStreamCount;

    p = (AudioDecCompContext*)a;
    nStreamCount = p->nStreamCount;

    pStreamInfo = (AudioStreamInfo*)malloc(sizeof(AudioStreamInfo)*nStreamCount);
    if(pStreamInfo == NULL)
    {
        loge("memory malloc fail!");
        return -1;
    }
    memset(pStreamInfo, 0, sizeof(AudioStreamInfo)*nStreamCount);

    for(i=0; i<nStreamCount; i++)
    {
        memcpy(&pStreamInfo[i], &p->pStreamInfoArr[i], sizeof(AudioStreamInfo));
        if(p->pStreamInfoArr[i].pCodecSpecificData != NULL && p->pStreamInfoArr[i].nCodecSpecificDataLen > 0)
        {
            pStreamInfo[i].pCodecSpecificData = (char*)malloc(p->pStreamInfoArr[i].nCodecSpecificDataLen);
            if(pStreamInfo[i].pCodecSpecificData == NULL)
            {
                loge("malloc memory fail.");
                pStreamInfo[i].nCodecSpecificDataLen = 0;
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
            if(pStreamInfo[i].pCodecSpecificData != NULL && pStreamInfo[i].nCodecSpecificDataLen > 0)
            {
                free(pStreamInfo[i].pCodecSpecificData);
                pStreamInfo[i].pCodecSpecificData = NULL;
                pStreamInfo[i].nCodecSpecificDataLen = 0;
            }
        }
        free(pStreamInfo);
        return -1;
    }

    *pStreamNum = nStreamCount;
    *ppStreamInfo = pStreamInfo;

    return 0;
}


int AudioDecCompSetTimer(AudioDecComp* a, AvTimer* timer)
{
    AudioDecCompContext* p;
    p = (AudioDecCompContext*)a;
    p->pAvTimer = timer;
    return 0;
}


int AudioDecCompRequestStreamBuffer(AudioDecComp* a,
                                    int           nRequireSize,
                                    char**        ppBuf,
                                    int*          pBufSize,
                                    char**        ppRingBuf,
                                    int*          pRingBufSize,
                                    int           nStreamIndex)
{
    AudioDecCompContext* p;
    StreamManager*       pSm;
    char*                pStreamBufEnd;
    StreamFrame*         pTmpFrame;
    char*                pBuf0;
    char*                pBuf1;
    int                  nBufSize0;
    int                  nBufSize1;

    p = (AudioDecCompContext*)a;

    *ppBuf        = NULL;
    *ppRingBuf    = NULL;
    *pBufSize     = 0;
    *pRingBufSize = 0;

    pBuf0      = NULL;
    pBuf1      = NULL;
    nBufSize0  = 0;
    nBufSize1  = 0;

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
            int64_t nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
            FlushStreamManagerBuffers(p, nCurTime, 1);
        }
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
        if(StreamManagerRequestBuffer(pSm, nRequireSize, &pBuf0, &nBufSize0) < 0)
        {
            pthread_mutex_unlock(&p->streamManagerMutex);
            logv("request buffer fail.");
            return -1;
        }
    }
    else
    {
        while(StreamManagerRequestBuffer(pSm, nRequireSize, &pBuf0, &nBufSize0) < 0)
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
    *ppBuf    = pBuf0;
    *pBufSize = nBufSize0;

    pthread_mutex_unlock(&p->streamManagerMutex);
    return 0;
}


int AudioDecCompSubmitStreamData(AudioDecComp*        a,
                                 AudioStreamDataInfo* pDataInfo,
                                 int                  nStreamIndex)
{
    int                  nSemCnt;
    AudioDecCompContext* p;
    StreamManager*       pSm;
    StreamFrame          streamFrame;

    p = (AudioDecCompContext*)a;

    //* submit data to stream manager
    pthread_mutex_lock(&p->streamManagerMutex);

    pSm = p->pStreamManagerArr[nStreamIndex];

    streamFrame.pData   = pDataInfo->pData;
    streamFrame.nLength = pDataInfo->nLength;
    if(pDataInfo->bIsFirstPart)
    {
        streamFrame.nPts = pDataInfo->nPts;
        streamFrame.nPcr = pDataInfo->nPcr;
    }
    else
    {
        streamFrame.nPts = -1;
        streamFrame.nPcr = -1;
    }

    StreamManagerAddStream(pSm, &streamFrame);

    pthread_mutex_unlock(&p->streamManagerMutex);

    if(sem_getvalue(&p->streamDataSem, &nSemCnt) == 0)
    {
        if(nSemCnt == 0)
            sem_post(&p->streamDataSem);
    }

    return 0;
}


int AudioDecCompStreamBufferSize(AudioDecComp* a, int nStreamIndex)
{
    AudioDecCompContext* p;
    StreamManager*       pSm;

    p = (AudioDecCompContext*)a;

    pSm = p->pStreamManagerArr[nStreamIndex];

    return StreamManagerBufferSize(pSm);
}


int AudioDecCompStreamDataSize(AudioDecComp* a, int nStreamIndex)
{
    AudioDecCompContext* p;
    int                  nStreamDataSize;

    p = (AudioDecCompContext*)a;

    pthread_mutex_lock(&p->streamManagerMutex);
	nStreamDataSize = 0;
	if(p->pStreamManagerArr[nStreamIndex] != NULL)
    {
        nStreamDataSize = StreamManagerStreamDataSize(p->pStreamManagerArr[nStreamIndex]);

        //* this method is called by the demux thread, the decoder may be destroyed when
        //* switching audio, so we should lock the decoderDestroyMutex to protect the
        //* decoder from destroyed.
        pthread_mutex_lock(&p->decoderDestroyMutex);
        if(p->pDecoder != NULL)
            nStreamDataSize += AudioStreamDataSize(p->pDecoder);
        pthread_mutex_unlock(&p->decoderDestroyMutex);
    }
    pthread_mutex_unlock(&p->streamManagerMutex);

    return nStreamDataSize;
}


int AudioDecCompStreamFrameNum(AudioDecComp* a, int nStreamIndex)
{
    AudioDecCompContext* p;
    int                  nStreamFrameNum;

    p = (AudioDecCompContext*)a;

    pthread_mutex_lock(&p->streamManagerMutex);
	nStreamFrameNum = 0;
	if(p->pStreamManagerArr[nStreamIndex] != NULL)
    {
        nStreamFrameNum = StreamManagerStreamFrameNum(p->pStreamManagerArr[nStreamIndex]);

        //* this method is called by the demux thread, the decoder may be destroyed when
        //* switching audio, so we should lock the decoderDestroyMutex to protect the
        //* decoder from destroyed.
//        pthread_mutex_lock(&p->decoderDestroyMutex);
//        if(p->pDecoder != NULL)
//            nStreamDataSize += AudioStreamDataSize(p->pDecoder);
//        pthread_mutex_unlock(&p->decoderDestroyMutex);
    }
    pthread_mutex_unlock(&p->streamManagerMutex);

    return nStreamFrameNum;
}


int AudioDecCompRequestPcmData(AudioDecComp*   a,
                               unsigned char** ppData,
                               unsigned int*   pSize,
                               int64_t*        pPts,
                               cedar_raw_data*  raw_data)
{
    AudioDecCompContext* p;
    p = (AudioDecCompContext*)a;
	AudioStreamInfo pStreamInfo = p->pStreamInfoArr[p->nStreamSelected];
	//* this method is called by the audio render thread,
	//* the audio render thread is paused or stop before the audio decoder thread,
	//* so here we do not need to lock the decoderDestroyMutex.
	if(p->pDecoder != NULL)
	{
        *pPts = PlybkRequestPcmPts(p->pDecoder);
		memcpy(raw_data,&(pStreamInfo.raw_data),sizeof(cedar_raw_data));
        return  PlybkRequestPcmBuffer(p->pDecoder, ppData, (int*)pSize);
    }
    else
    {
        *ppData = NULL;
        *pPts = -1;
		memset(raw_data,0,sizeof(cedar_raw_data));
        return -1;
    }
}


int AudioDecCompReleasePcmData(AudioDecComp* a, int nReleaseSize)
{
    int ret;
    int nSemCnt;
    AudioDecCompContext* p;
    p = (AudioDecCompContext*)a;

	//* this method is called by the audio render thread,
	//* the audio render thread is paused or stop before the audio decoder thread,
	//* so here we do not need to lock the decoderDestroyMutex.
    ret = PlybkUpdatePcmBuffer(p->pDecoder, nReleaseSize);

    if(sem_getvalue(&p->frameBufferSem, &nSemCnt) == 0)
    {
        if(nSemCnt == 0)
            sem_post(&p->frameBufferSem);
    }

    return ret;
}

int AudioDecCompPcmDataSize(AudioDecComp* a, int nStreamIndex)
{
    int                  nPcmDataSize;
    AudioDecCompContext* p;

    p            = (AudioDecCompContext*)a;
    nPcmDataSize = 0;

	CEDARX_UNUSE(nStreamIndex);

    //* this method is called by the demux thread, the decoder may be destroyed when
    //* switching audio, so we should lock the decoderDestroyMutex to protect the
    //* decoder from destroyed.
    pthread_mutex_lock(&p->decoderDestroyMutex);
	if(p->pDecoder != NULL)
        nPcmDataSize = AudioPCMDataSize(p->pDecoder);
    pthread_mutex_unlock(&p->decoderDestroyMutex);

    return nPcmDataSize;
}

//* must be called at stopped status.
int AudioDecCompSwitchStream(AudioDecComp* a, int nStreamIndex)
{
    AudioDecCompContext* p;
    p = (AudioDecCompContext*)a;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("can not switch status when audio decoder is not in stopped status.");
        return -1;
    }

    pthread_mutex_lock(&p->streamManagerMutex);
    p->nStreamSelected = nStreamIndex;
    pthread_mutex_unlock(&p->streamManagerMutex);
    return 0;
}

void AudioDecRawSendCmdToHalClbk(void *pself,void *param)
{
    AudioDecCompContext* p = (AudioDecCompContext*)pself;
	if(p->callback)
		p->callback(p->pUserData, PLAYER_AUDIO_DECODER_NOTIFY_AUDIORAWPLAY, param);
}
static void* AudioDecodeThread(void* arg)
{
    AudioDecCompContext* p;
    Message              msg;
    int                  ret;
    sem_t*               pReplySem;
    int*                 pReplyValue;
    int64_t              nCurTime;
    int                  bFirstFramePtsValid;

    p = (AudioDecCompContext*)arg;

    bFirstFramePtsValid = 0;

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
            logv("process start message.");
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
                if(p->bCrashFlag == 0)
                    PostDecodeMessage(p->mq);
                p->eStatus = PLAYER_STATUS_STARTED;
		        if(pReplyValue != NULL)
			        *pReplyValue = 0;
		        sem_post(pReplySem);
		        continue;
            }

            //* create a decoder.
            //* lock the decoderDestroyMutex to prevend the demux thread from getting
            //* stream data size or pcm data size when the decoder is being created.
            //* see AudioDecCompStreamDataSize() and AudioDecCompPcmDataSize().
            pthread_mutex_lock(&p->decoderDestroyMutex);

            p->pDecoder = CreateAudioDecoder();
            if(p->pDecoder == NULL)
            {
                pthread_mutex_unlock(&p->decoderDestroyMutex);
                loge("audio decoder component create decoder fail.");
                p->bCrashFlag = 1;
                p->callback(p->pUserData, PLAYER_AUDIO_DECODER_NOTIFY_CRASH, NULL);
		        if(pReplyValue != NULL)
			        *pReplyValue = -1;
		        sem_post(pReplySem);
		        continue;
            }

            memset(&p->bsInfo, 0, sizeof(BsInFor));
            if(InitializeAudioDecoder(p->pDecoder,
                                      &p->pStreamInfoArr[p->nStreamSelected],
                                      &p->bsInfo) != 0)
            {
                loge("initialize audio decoder fail.");
                DestroyAudioDecoder(p->pDecoder);
                p->pDecoder = NULL;
                pthread_mutex_unlock(&p->decoderDestroyMutex);
                p->bCrashFlag = 1;
                p->callback(p->pUserData, PLAYER_AUDIO_DECODER_NOTIFY_CRASH, NULL);
		        if(pReplyValue != NULL)
			        *pReplyValue = -1;
		        sem_post(pReplySem);
		        continue;
            }
            SetRawPlayParam(p->pDecoder,(void*)p);
            pthread_mutex_unlock(&p->decoderDestroyMutex);     //* demux thread can use the decoder now.

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
            logv("process stop message.");
            //* check status.
            if(p->eStatus == PLAYER_STATUS_STOPPED)
            {
                loge("invalid stop operation, already in stopped status.");
                if(p->bCrashFlag == 1)
                {
                    p->bCrashFlag = 0;
                }

		        if(pReplyValue != NULL)
			        *pReplyValue = -1;
		        sem_post(pReplySem);
		        continue;
            }

            //* destroy decoder.
            //* lock the decoderDestroyMutex to prevend the demux thread from getting
            //* stream data size or pcm data size when the decoder is being created.
            //* see AudioDecCompStreamDataSize() and AudioDecCompPcmDataSize().
            pthread_mutex_lock(&p->decoderDestroyMutex);
            bFirstFramePtsValid = 0;
            if(p->pDecoder != NULL)
            {
                DestroyAudioDecoder(p->pDecoder);
                p->pDecoder = NULL;
            }
            pthread_mutex_unlock(&p->decoderDestroyMutex);
            memset(&p->bsInfo, 0, sizeof(BsInFor));

            p->bCrashFlag = 0;
            p->eStatus = PLAYER_STATUS_STOPPED;
		    if(pReplyValue != NULL)
			    *pReplyValue = 0;
		    sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_PAUSE)
        {
            logv("process pause message.");
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
            logv("process quit message.");
            //* destroy decoder and break.
            //* lock the decoderDestroyMutex to prevend the demux thread from getting
            //* stream data size or pcm data size when the decoder is being created.
            //* see AudioDecCompStreamDataSize() and AudioDecCompPcmDataSize().
            pthread_mutex_lock(&p->decoderDestroyMutex);
            if(p->pDecoder != NULL)
            {
                DestroyAudioDecoder(p->pDecoder);
                p->pDecoder = NULL;
            }
            pthread_mutex_unlock(&p->decoderDestroyMutex);

            p->eStatus = PLAYER_STATUS_STOPPED;
		    if(pReplyValue != NULL)
			    *pReplyValue = 0;
		    sem_post(pReplySem);
		    break;
        }
        else if(msg.messageId == MESSAGE_ID_RESET)
        {
            logv("process reset message.");
            int i;
			int64_t nSeekTime;

			bFirstFramePtsValid = 0;
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
            ResetAudioDecoder(p->pDecoder, nSeekTime);
            if(pReplyValue != NULL)
                *pReplyValue = 0;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED)
                PostDecodeMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_EOS)
        {
            logv("process eos message.");
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
            char*          pOutputBuf  = NULL;
            int            nPcmDataLen = 0;
            StreamManager* pSm         = p->pStreamManagerArr[p->nStreamSelected];
            StreamFrame*   pFrame      = NULL;
            unsigned char* pBuf0       = NULL;
            unsigned char* pBuf1       = NULL;
            int            nBufSize0   = 0;
            int            nBufSize1   = 0;

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
                //* to prevent the pts loop back at the very beginning
                //* before the first audio frame is send to the audio render.
                //* the timer is started at the moment the first audio frame is received.
                nCurTime = p->pAvTimer->GetTime(p->pAvTimer);
                FlushStreamManagerBuffers(p, nCurTime, 0);
            }
            pthread_mutex_unlock(&p->streamManagerMutex);

            if(DecRequestPcmBuffer(p->pDecoder, &pOutputBuf) < 0)
            {
                //* no pcm buffer, wait for the pcm buffer semaphore.
                logi("no pcm buffer, wait.");
                SemTimedWait(&p->frameBufferSem, 20);    //* wait for frame buffer.
                PostDecodeMessage(p->mq);
                continue;
            }

            //* Add stream to decoder.
            pFrame = StreamManagerGetFrameInfo(pSm, 0);
            if(pFrame != NULL)
            {
		if((bFirstFramePtsValid==0) && (pFrame->nPts==-1))
		{
			pFrame = StreamManagerRequestStream(pSm);
			StreamManagerFlushStream(pSm, pFrame);

                    if(p->bEosFlag && StreamManagerStreamFrameNum(p->pStreamManagerArr[p->nStreamSelected]) == 0)
                    {
			logd("audio decoder notify eos.");
                        p->callback(p->pUserData, PLAYER_AUDIO_DECODER_NOTIFY_EOS, NULL);
                    }
                    else
                    {
			PostDecodeMessage(p->mq);
                    }
                    continue;
		}
                ret = ParserRequestBsBuffer(p->pDecoder,
                                            pFrame->nLength,
                                            &pBuf0,
                                            &nBufSize0,
                                            &pBuf1,
                                            &nBufSize1,
                                            &p->nOffset);
                if((nBufSize0+nBufSize1)>=pFrame->nLength)
                {
                    pFrame = StreamManagerRequestStream(pSm);
                    if(nBufSize0 >= pFrame->nLength)
                        memcpy(pBuf0, pFrame->pData, pFrame->nLength);
                    else
                    {
                        memcpy(pBuf0, pFrame->pData, nBufSize0);
                        memcpy(pBuf1, (char*)pFrame->pData + nBufSize0, pFrame->nLength - nBufSize0);
                    }
                    ParserUpdateBsBuffer(p->pDecoder,
                                         pFrame->nLength,
                                         pFrame->nPts,
                                         p->nOffset);
                    StreamManagerFlushStream(pSm, pFrame);
                    bFirstFramePtsValid = 1;
                }
            }

            ret = DecodeAudioStream(p->pDecoder,
                                    &p->pStreamInfoArr[p->nStreamSelected],
                                    pOutputBuf,
                                    &nPcmDataLen);
            logv("DecodeAudioStream, ret = %d",ret);
            if(ret == ERR_AUDIO_DEC_NONE)
            {
                if(p->pStreamInfoArr[p->nStreamSelected].nSampleRate != p->bsInfo.out_samplerate ||
                    p->pStreamInfoArr[p->nStreamSelected].nChannelNum != p->bsInfo.out_channels)
                {
                    p->pStreamInfoArr[p->nStreamSelected].nSampleRate = p->bsInfo.out_samplerate;
                    p->pStreamInfoArr[p->nStreamSelected].nChannelNum = p->bsInfo.out_channels;
                }
                DecUpdatePcmBuffer(p->pDecoder, nPcmDataLen);
                PostDecodeMessage(p->mq);
                continue;
            }
            else if(ret == ERR_AUDIO_DEC_NO_BITSTREAM || ret == ERR_AUDIO_DEC_ABSEND)
            {
                if(p->bEosFlag &&
                   StreamManagerStreamFrameNum(p->pStreamManagerArr[p->nStreamSelected]) == 0)
                {
                    logv("audio decoder notify eos.");
                    p->callback(p->pUserData, PLAYER_AUDIO_DECODER_NOTIFY_EOS, NULL);
                    continue;
                }
                else
                {
                    if(StreamManagerStreamFrameNum(p->pStreamManagerArr[p->nStreamSelected]) == 0)
                        SemTimedWait(&p->streamDataSem, 50);
                    PostDecodeMessage(p->mq);
                    continue;
                }
            }
            else if(ret == ERR_AUDIO_DEC_EXIT || ret == ERR_AUDIO_DEC_ENDINGCHKFAIL)
            {
                p->bCrashFlag = 1;
                p->callback(p->pUserData, PLAYER_AUDIO_DECODER_NOTIFY_CRASH, NULL);
                continue;
            }
            else
            {
                logw("DecodeAudioStream() return %d, continue to decode", ret);
                PostDecodeMessage(p->mq);
            }
        }
        else
        {
            loge("unknown message in audio decode thread.");
            abort();
        }
	}

    ret = 0;
    pthread_exit(&ret);
	return NULL;
}


static void PostDecodeMessage(MessageQueue* mq)
{
    if(MessageQueueGetCount(mq) <= 0)
    {
        Message msg;
        msg.messageId = MESSAGE_ID_DECODE;
        msg.params[0] = msg.params[1] = msg.params[2] = msg.params[3] = 0;
        if(MessageQueuePostMessage(mq, &msg) != 0)
        {
            loge("fatal error, audio decode component post message fail.");
            abort();
        }

        return;
    }
}


static void FlushStreamManagerBuffers(AudioDecCompContext* p, int64_t curTime, int bIncludeSeletedStream)
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
