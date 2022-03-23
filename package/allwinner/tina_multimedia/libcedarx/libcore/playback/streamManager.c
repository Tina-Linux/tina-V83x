/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : streamManager.cpp
 * Description : stream manager
 * History :
 *
 */

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <inttypes.h>
#include "streamManager.h"
#include "cdx_log.h"

#define MIN_PREVERSE_RATIO  0.1

#ifndef UINT64_MAX
#define UINT64_MAX  0xffffffffffffffff
#endif

typedef struct StreamFifo StreamFifo;

struct StreamFifo {
    StreamFrame* pFrames;
    int          nMaxFrameNum;
    int          nReadPos;
    int          nWritePos;
};

struct StreamManager {
    pthread_mutex_t mutex;
    int             nMaxBufferSize;
    int             nValidDataSize;
    int             nStreamID;
    StreamFifo      streamFifo;
    char*           pMem;
    int             nMemSize;
    char *lostBaby;
};

/* When p->nWritePos == p->nReadPos, the valid frame num can be zero or
 * nMaxFrameNum. Since we preserve a few of old frames, valid frame num is
 * always less than nMaxFrameNum.
 */
static inline int validFrameNum(StreamFifo *p)
{
    return (p->nWritePos - p->nReadPos + p->nMaxFrameNum) % p->nMaxFrameNum;
}

static inline int invalidFrameNumTooSmall(StreamFifo *p)
{
    int invalidFrameNum = p->nMaxFrameNum - validFrameNum(p);
    int preserveFrameNum = p->nMaxFrameNum * MIN_PREVERSE_RATIO;
    if (invalidFrameNum <= preserveFrameNum)
        return 1;

    return 0;
}

/*
**********************************************************************
*                             StreamManagerCreate
*
*Description: Create Stream Manager module.
*
*Arguments  : nMaxBufferSize  max stream buffer size.
*Arguments  : nMaxFrameNum    max stream frame count.
*Arguments  : nStreamId       the ID value to identify which stream
*                             this streamBufferManager belong to.
*
*Return     : result
*               = NULL;     failed;
*              != NULL;     StreamManager handler.
*
*Summary    :
*
**********************************************************************
*/
StreamManager* StreamManagerCreate(int nMaxBufferSize, int nMaxFrameNum, int nStreamID)
{
    StreamManager* p;
    int               i;
    int               ret;

    if(nMaxFrameNum * MIN_PREVERSE_RATIO <= 0)
        return NULL;

    p = (StreamManager *)calloc(1, sizeof(*p));
    if(p == NULL)
    {
        loge("can not allocate memory for stream manager.");
        return NULL;
    }

    p->streamFifo.pFrames = (StreamFrame*)calloc(nMaxFrameNum, sizeof(StreamFrame));
    if(p->streamFifo.pFrames == NULL)
    {
        loge("can not allocate memory for stream manager.");
        free(p);
        return NULL;
    }

    pthread_mutex_init(&p->mutex, NULL);
    p->nMaxBufferSize = nMaxBufferSize;
    p->nValidDataSize = 0;

    p->streamFifo.nMaxFrameNum   = nMaxFrameNum;
    p->streamFifo.nReadPos       = 0;
    p->streamFifo.nWritePos      = 0;
    p->nStreamID = nStreamID;

    return p;
}

/*
**********************************************************************
*                             StreamManagerDestroy
*
*Description: Destroy Stream Manager module, free resource.
*
*Arguments  : p     Created by StreamManagerCreate function.
*
*Return     : NULL
*
*Summary    :
*
**********************************************************************
*/
void StreamManagerDestroy(StreamManager *p)
{
    if (p == NULL)
        return;

    pthread_mutex_destroy(&p->mutex);

    StreamFrame *pFrame = p->streamFifo.pFrames;
    int nMaxFrameNum = p->streamFifo.nMaxFrameNum;

    int i = 0;
    for (i = 0; i < nMaxFrameNum; i++)
        free(pFrame[i].pData);

    free(pFrame);
    free(p->pMem);
    free(p);
    return;
}

/*
**********************************************************************
*                         StreamManagerReset
*
*Description: Reset Stream Manager module.
*
*Arguments  : p     Created by StreamManagerCreate function.
*
*Return     : NULL
*
*Summary    : If succeed, Stream Manager module will be resumed to initial state,
*             stream data will be discarded.
*
**********************************************************************
*/
void StreamManagerReset(StreamManager* p)
{
    StreamFrame*      pFrame;
    int               nMaxFrameNum;
    int               nReadPos;
    int               i;

    if(p == NULL)
    {
        loge("p == NULL.");
        return;
    }

    pthread_mutex_lock(&p->mutex);

    pFrame        = p->streamFifo.pFrames;
    nMaxFrameNum   = p->streamFifo.nMaxFrameNum;

    for (i = 0; i < nMaxFrameNum; i++)
        free(pFrame[i].pData);
    memset(pFrame, 0, nMaxFrameNum * sizeof(*pFrame));

    if(p->pMem != NULL)
    {
        loge("memory leak,,,, the guy not return... '%p'", p->pMem);
        p->lostBaby = p->pMem;
//        free(p->pMem);
        p->pMem = NULL;
        p->nMemSize = 0;
    }

    p->nValidDataSize             = 0;
    p->streamFifo.nReadPos        = 0;
    p->streamFifo.nWritePos       = 0;
    pthread_mutex_unlock(&p->mutex);

    return;
}

/*
**********************************************************************
*                      StreamManagerBufferSize
*
*Description: Get the StreamManager buffer size.
*
*Arguments  : p     Created by StreamManagerCreate function.
*
*Return     : The size of StreamManager buffer, in Bytes.
*
*Summary    : The size is set when create StreamManager.
*
**********************************************************************
*/
int StreamManagerBufferSize(StreamManager* p)
{
    if(p == NULL)
    {
        loge("p == NULL.");
        return 0;
    }

    return p->nMaxBufferSize;
}

/*
**********************************************************************
*                     StreamManagerStreamFrameNum
*
*Description: Get the total frames of undecoded stream data.
*
*Arguments  : p     Created by StreamManagerCreate function.
*
*Return     : The frames of undecoded stream data.
*
*Summary    :
*
**********************************************************************
*/
int StreamManagerStreamFrameNum(StreamManager* p)
{
    if(p == NULL)
    {
        loge("p == NULL.");
        return 0;
    }

    return validFrameNum(&p->streamFifo);
}

/*
**********************************************************************
*                      StreamManagerStreamDataSize
*
*Description: Get the total size of undecoded data.
*
*Arguments  : p     Created by StreamManagerCreate function.
*
*Return     : The total size of undecoded stream data, in bytes.
*
*Summary    :
*
**********************************************************************
*/
int StreamManagerStreamDataSize(StreamManager* p)
{
    if(p == NULL)
    {
        loge("p == NULL.");
        return 0;
    }

    return p->nValidDataSize;
}

/*
**********************************************************************
*                        StreamManagerRequestBuffer
*
*Description: Request buffer from sbm module.
*
*Arguments  : p              Created by StreamManagerCreate function;
*             nRequireSize      the required size, in bytes;
*             ppBuf             store the requested buffer address;
*             pBufSize          store the requested buffer size.
*
*Return     : result;
*               = 0;    succeeded;
*               = -1;   failed.
*
*Summary    : SBM buffer is cyclic, if the  buffer turns around, there will be 2 blocks.
*
**********************************************************************
*/
int StreamManagerRequestBuffer(StreamManager* p, int nRequireSize, char** ppBuf, int* pBufSize)
{
    if (p == NULL || ppBuf == NULL || pBufSize == NULL)
    {
        loge("input error.");
        return -1;
    }

    *ppBuf    = NULL;
    *pBufSize = 0;

    pthread_mutex_lock(&p->mutex);

    if (invalidFrameNumTooSmall(&p->streamFifo))
    {
        logv("too much of frames, wait");
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }

    if (p->nValidDataSize >= p->nMaxBufferSize)
    {
        pthread_mutex_unlock(&p->mutex);
        logv("no free buffer.");

        /* Please don't do sleep in this library.
         * You can call sleep after this function return.
         */
        // usleep(100000); /* buffer full, wait 100ms... */
        return -1;
    }

    if (p->pMem != NULL)
    {
        pthread_mutex_unlock(&p->mutex);
        /* Somebody should fix this log, I don't know what does it mean. */
        logw("you not payback last buffer, not bollow now...");

        /* Please don't do sleep in this library.
         * You can call sleep after this function return.
         */
        //usleep(100000); /* buffer full, wait 100ms... */
        return -1;
/*
        if (p->nMemSize == nRequireSize)
        {
            *ppBuf    = p->pMem;
            *pBufSize = p->nMemSize;
            pthread_mutex_unlock(&p->mutex);
            return 0;
        }

        free(p->pMem);
        p->nMemSize = 0;
*/
    }

    p->pMem = (char*)malloc(nRequireSize);
    if(p->pMem != NULL)
    {
        p->nMemSize = nRequireSize;
        *ppBuf    = p->pMem;
        *pBufSize = p->nMemSize;
        pthread_mutex_unlock(&p->mutex);
        return 0;
    }
    else
    {
        loge("allocate memory fail.");
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }
}

/*
**********************************************************************
*                        StreamManagerAddStream
*
*Description: Add one frame stream to StreamManager module.
*
*Arguments  : p              Created by StreamManagerCreate function;
*             pDataInfo        the stream info need to be added.
*
*Return     : result;
*               = 0;    succeeded;
*               = -1;   failed.
*
*Summary    :
*
**********************************************************************
*/
int StreamManagerAddStream(StreamManager* p, StreamFrame* pStreamFrame)
{

    int               nWritePos;
    char*             pNewWriteAddr;

    if(p == NULL || pStreamFrame == NULL)
    {
        loge("input error.");
        return -1;
    }

    pthread_mutex_lock(&p->mutex);

    if (invalidFrameNumTooSmall(&p->streamFifo))
    {
        loge("too much of frames");
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }

    if(p->pMem != pStreamFrame->pData || p->nMemSize < pStreamFrame->nLength)
    {
        loge("stream buffer not match. (%p :%d) (%p :%d)",
                p->pMem, p->nMemSize, pStreamFrame->pData, pStreamFrame->nLength);
        if (p->lostBaby == pStreamFrame->pData)
        {
            logw("found the lost baby, free now.'%p'", pStreamFrame->pData);
            free(pStreamFrame->pData);
        }
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }

    nWritePos = p->streamFifo.nWritePos;
    free(p->streamFifo.pFrames[nWritePos].pData);
    p->streamFifo.pFrames[nWritePos++] = *pStreamFrame;
    nWritePos %= p->streamFifo.nMaxFrameNum;

    p->streamFifo.nWritePos = nWritePos;
    p->nValidDataSize += pStreamFrame->nLength;

    p->pMem = NULL;
    p->nMemSize = 0;

    pthread_mutex_unlock(&p->mutex);
    return 0;
}

/*
**********************************************************************
*                      StreamManagerRequestStream
*
*Description: Request one frame stream data from sbm module to decoder.
*
*Arguments  : p      Created by StreamManagerCreate function;
*
*Return     : The stream information.
*
*Summary    : The stream data obeys FIFO rule.
*
**********************************************************************
*/
StreamFrame* StreamManagerRequestStream(StreamManager* p)
{
    StreamFrame*      pStreamFrame;

    if(p == NULL )
    {
        loge("p == NULL.");
        return NULL;
    }

    pthread_mutex_lock(&p->mutex);

    if(validFrameNum(&p->streamFifo) == 0)
    {
        loge("nValidFrameNum == 0.");
        pthread_mutex_unlock(&p->mutex);
        return NULL;
    }

    pStreamFrame = &p->streamFifo.pFrames[p->streamFifo.nReadPos];

    p->streamFifo.nReadPos = (p->streamFifo.nReadPos + 1) % p->streamFifo.nMaxFrameNum;
    p->nValidDataSize -= pStreamFrame->nLength;

    pthread_mutex_unlock(&p->mutex);

    return pStreamFrame;
}

/*
**********************************************************************
*                      StreamManagerGetFrameInfo
*
*Description: get the frame information of the specific frame.
*
*Arguments  : p         Created by StreamManagerCreate function;
*             nFrameIndex the frame index start counted from the first
*                         valid frame.
*
*Return     : The stream information.
*
*Summary    : The stream data obeys FIFO rule.
*
**********************************************************************
*/
StreamFrame* StreamManagerGetFrameInfo(StreamManager* p, int nFrameIndex)
{
    StreamFrame*      pStreamFrame;
    int               nPos;

    if(p == NULL )
    {
        loge("p == NULL.");
        return NULL;
    }

    pthread_mutex_lock(&p->mutex);

    if(nFrameIndex < 0 || nFrameIndex >= validFrameNum(&p->streamFifo))
    {
        pthread_mutex_unlock(&p->mutex);
        return NULL;
    }

    nPos = p->streamFifo.nReadPos + nFrameIndex;
    if(nPos >= p->streamFifo.nMaxFrameNum)
        nPos -= p->streamFifo.nMaxFrameNum;

    pStreamFrame = &p->streamFifo.pFrames[nPos];

    pthread_mutex_unlock(&p->mutex);

    return pStreamFrame;
}

/*
**********************************************************************
*                        StreamManagerReturnStream
*
*Description: Return one undecoded frame to StreamManager module.
*
*Arguments  : p          Created by StreamManagerCreate function;
*             pStreamFrame the stream info need to be returned.
*
*Return     : result;
*               = 0;    succeeded;
*               = -1;   failed.
*
*Summary    : After returned, the stream data's sequence is the same as before.
*
**********************************************************************
*/
int StreamManagerReturnStream(StreamManager* p, StreamFrame* pStreamFrame)
{
    int               nReadPos;

    if(p == NULL || pStreamFrame == NULL)
    {
        loge("input error.");
        return -1;
    }

    pthread_mutex_lock(&p->mutex);

    if(validFrameNum(&p->streamFifo) == 0)
    {
        loge("nValidFrameNum == 0.");
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }
    nReadPos = p->streamFifo.nReadPos;
    nReadPos--;
    if(nReadPos < 0)
        nReadPos = p->streamFifo.nMaxFrameNum - 1;

    if(pStreamFrame != &p->streamFifo.pFrames[nReadPos])
    {
        loge("wrong frame sequence.");
        abort();
    }

    p->streamFifo.pFrames[nReadPos] = *pStreamFrame;
    p->streamFifo.nReadPos  = nReadPos;
    p->nValidDataSize += pStreamFrame->nLength;

    pthread_mutex_unlock(&p->mutex);
    return 0;
}
/*
**********************************************************************
*                        StreamManagerFlushStream
*
*Description: free one frame whitch had submitted to decoder.
*
*Arguments  : p          Created by StreamManagerCreate function;
*             pStreamFrame the stream info need to be returned.(reserved)
*
*Return     : result;
*               = 0;    succeeded;
*               = -1;   failed.
*
*Summary    : After returned, the submitted stream data has been freed.
*
**********************************************************************
*/

int StreamManagerFlushStream(StreamManager* p, StreamFrame* pStreamFrame)
{
    int               nLastReadPos;
    (void)pStreamFrame;

    if(p == NULL )
    {
        loge("p == NULL.");
        return -1;
    }

    pthread_mutex_lock(&p->mutex);

    nLastReadPos = (p->streamFifo.nReadPos - 1 + p->streamFifo.nMaxFrameNum)%
                p->streamFifo.nMaxFrameNum;
    if(p->streamFifo.pFrames[nLastReadPos].pData != NULL)
    {
        free(p->streamFifo.pFrames[nLastReadPos].pData);
        p->streamFifo.pFrames[nLastReadPos].pData = NULL;
    }

    pthread_mutex_unlock(&p->mutex);
    return 0;
}

int StreamManagerRewind(StreamManager *p, int64_t curTime)
{
    pthread_mutex_lock(&p->mutex);

    struct StreamFifo *fifo = &p->streamFifo;
    int nReadPos = fifo->nReadPos;
    int nWritePos = fifo->nWritePos;
    int maxNum = fifo->nMaxFrameNum;

    int find = 0;
    uint64_t minDiff = UINT64_MAX;
    int n = 0;
    for (n = 0; n < maxNum; n++)
    {
        if (abs(curTime - fifo->pFrames[n].nPts) < minDiff)
        {
            find = n;
            minDiff = abs(curTime - fifo->pFrames[n].nPts);
        }
    }

    if (find == nReadPos || fifo->pFrames[find].pData == NULL)
    {
        pthread_mutex_unlock(&p->mutex);
        return 0;
    }

    logv("old read position %d, new read position %d, write position %d, "
            "frame pts %"PRId64", current time %"PRId64,
            nReadPos, find, fifo->nWritePos,
            fifo->pFrames[find].nPts, curTime);

    int validDataSize = 0;
    int i;
    for (i = find; i != nWritePos; i = (i + 1) % maxNum)
        validDataSize += fifo->pFrames[i].nLength;
    logv("valid data size before %d, after %d", p->nValidDataSize, validDataSize);

    p->nValidDataSize = validDataSize;
    fifo->nReadPos = find;

    pthread_mutex_unlock(&p->mutex);
    return 0;
}
