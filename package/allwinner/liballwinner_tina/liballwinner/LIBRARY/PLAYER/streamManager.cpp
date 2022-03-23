
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "streamManager.h"
#include "log.h"


typedef struct STREAMFIFO
{
    StreamFrame* pFrames;
    int          nMaxFrameNum;
    int          nValidFrameNum;
    int          nReadPos;
    int          nWritePos;
    int          nFlushPos;
}StreamFifo;

typedef struct STREAMMANAGERCONTEXT
{
    pthread_mutex_t mutex;
    int             nMaxBufferSize;
    int             nValidDataSize;
    int             nStreamID;
    StreamFifo      streamFifo;
    char*           pMem;
    int             nMemSize;
    char *lostBaby;
}StreamManagerCtx;

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
    StreamManagerCtx* p;
    int               i;
    int               ret;

    if(nMaxBufferSize <= 0)
        return NULL;

    p = (StreamManagerCtx *)malloc(sizeof(StreamManagerCtx));
    if(p == NULL)
    {
        loge("can not allocate memory for stream manager.");
        return NULL;
    }
    memset(p, 0, sizeof(StreamManagerCtx));

    p->streamFifo.pFrames = (StreamFrame*)malloc(nMaxFrameNum * sizeof(StreamFrame));
    if(p->streamFifo.pFrames == NULL)
    {
        loge("can not allocate memory for stream manager.");
        free(p);
        return NULL;
    }
    memset(p->streamFifo.pFrames, 0,  nMaxFrameNum * sizeof(StreamFrame));

    pthread_mutex_init(&p->mutex, NULL);
    p->nMaxBufferSize = nMaxBufferSize;
    p->nValidDataSize = 0;

    p->streamFifo.nMaxFrameNum   = nMaxFrameNum;
    p->streamFifo.nValidFrameNum = 0;
    p->streamFifo.nReadPos       = 0;
    p->streamFifo.nWritePos      = 0;
    p->streamFifo.nFlushPos      = 0;
    p->nStreamID = nStreamID;

    return (StreamManager*)p;
}


/*
**********************************************************************
*                             StreamManagerDestroy
*
*Description: Destroy Stream Manager module, free resource.
*
*Arguments  : pSm     Created by StreamManagerCreate function.
*
*Return     : NULL
*
*Summary    :
*
**********************************************************************
*/
void StreamManagerDestroy(StreamManager* pSm)
{
    StreamManagerCtx* p;
    StreamFrame*      pFrame;
    StreamFrame*      pFrames;
    int               nMaxFrameNum;
    int               nValidFrameNum;
    int               nReadPos;

    p = (StreamManagerCtx*)pSm;

    if(p != NULL)
    {
        pthread_mutex_destroy(&p->mutex);

        pFrames        = p->streamFifo.pFrames;
        nMaxFrameNum   = p->streamFifo.nMaxFrameNum;
        nValidFrameNum = p->streamFifo.nValidFrameNum;
        nReadPos       = p->streamFifo.nReadPos;

        while(nValidFrameNum > 0)
        {
            pFrame = &pFrames[nReadPos];
            if(pFrame->pData != NULL)
                free(pFrame->pData);
            nValidFrameNum--;
            nReadPos++;
            if(nReadPos >= nMaxFrameNum)
                nReadPos = 0;
        }

        if(pFrames != NULL)
            free(pFrames);

        if(p->pMem != NULL)
            free(p->pMem);

        free(p);
    }

    return;
}


/*
**********************************************************************
*                         StreamManagerReset
*
*Description: Reset Stream Manager module.
*
*Arguments  : pSm     Created by StreamManagerCreate function.
*
*Return     : NULL
*
*Summary    : If succeed, Stream Manager module will be resumed to initial state,
*             stream data will be discarded.
*
**********************************************************************
*/
void StreamManagerReset(StreamManager* pSm)
{
    StreamManagerCtx* p;
    StreamFrame*      pFrame;
    StreamFrame*      pFrames;
    int               nMaxFrameNum;
    int               nValidFrameNum;
    int               nReadPos;

    p = (StreamManagerCtx*)pSm;

    if(p == NULL)
    {
        loge("pSm == NULL.");
        return;
    }

    pthread_mutex_lock(&p->mutex);

    pFrames        = p->streamFifo.pFrames;
    nMaxFrameNum   = p->streamFifo.nMaxFrameNum;
    nValidFrameNum = p->streamFifo.nValidFrameNum;
    nReadPos       = p->streamFifo.nReadPos;

    while(nValidFrameNum > 0)
    {
        pFrame = &pFrames[nReadPos];
        if(pFrame->pData != NULL)
            free(pFrame->pData);
        nValidFrameNum--;
        nReadPos++;
        if(nReadPos >= nMaxFrameNum)
            nReadPos = 0;
    }

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
    p->streamFifo.nFlushPos       = 0;
    p->streamFifo.nValidFrameNum  = 0;
    pthread_mutex_unlock(&p->mutex);

    return;
}


/*
**********************************************************************
*                      StreamManagerBufferSize
*
*Description: Get the StreamManager buffer size.
*
*Arguments  : pSm     Created by StreamManagerCreate function.
*
*Return     : The size of StreamManager buffer, in Bytes.
*
*Summary    : The size is set when create StreamManager.
*
**********************************************************************
*/
int StreamManagerBufferSize(StreamManager* pSm)
{
    StreamManagerCtx* p;
    p = (StreamManagerCtx*)pSm;

    if(p == NULL)
    {
        loge("pSm == NULL.");
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
*Arguments  : pSm     Created by StreamManagerCreate function.
*
*Return     : The frames of undecoded stream data.
*
*Summary    :
*
**********************************************************************
*/
int StreamManagerStreamFrameNum(StreamManager* pSm)
{
    StreamManagerCtx* p;
    p = (StreamManagerCtx*)pSm;

    if(p == NULL)
    {
        loge("pSm == NULL.");
        return 0;
    }

    return p->streamFifo.nValidFrameNum;
}


/*
**********************************************************************
*                      StreamManagerStreamDataSize
*
*Description: Get the total size of undecoded data.
*
*Arguments  : pSm     Created by StreamManagerCreate function.
*
*Return     : The total size of undecoded stream data, in bytes.
*
*Summary    :
*
**********************************************************************
*/
int StreamManagerStreamDataSize(StreamManager* pSm)
{
    StreamManagerCtx* p;
    p = (StreamManagerCtx*)pSm;

    if(p == NULL)
    {
        loge("pSm == NULL.");
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
*Arguments  : pSm              Created by StreamManagerCreate function;
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
int StreamManagerRequestBuffer(StreamManager* pSm, int nRequireSize, char** ppBuf, int* pBufSize)
{
    StreamManagerCtx* p;

    p = (StreamManagerCtx*)pSm;

    if (p == NULL || ppBuf == NULL || pBufSize == NULL)
    {
        loge("input error.");
        return -1;
    }

    pthread_mutex_lock(&p->mutex);

    if (p->streamFifo.nValidFrameNum >= p->streamFifo.nMaxFrameNum)
    {
        logv("nValidFrameNum >= nMaxFrameNum.");
        pthread_mutex_unlock(&p->mutex);
        *ppBuf    = NULL;
        *pBufSize = 0;
        return -1;
    }

    if (p->nValidDataSize >= p->nMaxBufferSize)
    {
		//change to logv, it is not an error.
        logw("no free buffer.");
        usleep(100000); /* buffer full, wait 100ms... */
        pthread_mutex_unlock(&p->mutex);
        *ppBuf    = NULL;
        *pBufSize = 0;
        return -1;
    }

    if (p->pMem != NULL)
    {
        logw("you not payback last buffer, not bollow now...");
        usleep(100000); /* buffer full, wait 100ms... */
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
        *ppBuf    = NULL;
        *pBufSize = 0;
        return -1;
    }
}


/*
**********************************************************************
*                        StreamManagerAddStream
*
*Description: Add one frame stream to StreamManager module.
*
*Arguments  : pSm              Created by StreamManagerCreate function;
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
int StreamManagerAddStream(StreamManager* pSm, StreamFrame* pStreamFrame)
{

    int               nWritePos;
    char*             pNewWriteAddr;
    StreamManagerCtx* p;

    p = (StreamManagerCtx*)pSm;

    if(p == NULL || pStreamFrame == NULL)
    {
        loge("input error.");
        return -1;
    }

    pthread_mutex_lock(&p->mutex);

    if(p->streamFifo.nValidFrameNum >= p->streamFifo.nMaxFrameNum)
    {
        loge("nValidFrameNum > nMaxFrameNum.");
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }

    if(p->pMem != pStreamFrame->pData || p->nMemSize < pStreamFrame->nLength)
    {
        loge("stream buffer not match. (%p :%d) (%p :%d)", p->pMem, p->nMemSize, pStreamFrame->pData, pStreamFrame->nLength);
        if (p->lostBaby == pStreamFrame->pData)
        {
            logw("found the lost baby, free now.'%p'", pStreamFrame->pData);
            free(pStreamFrame->pData);
        }
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }

    nWritePos = p->streamFifo.nWritePos;
    memcpy(&p->streamFifo.pFrames[nWritePos], pStreamFrame, sizeof(StreamFrame));
    nWritePos++;
    if(nWritePos >= p->streamFifo.nMaxFrameNum)
        nWritePos = 0;

    p->streamFifo.nWritePos = nWritePos;
    p->streamFifo.nValidFrameNum++;
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
*Arguments  : pSm      Created by StreamManagerCreate function;
*
*Return     : The stream information.
*
*Summary    : The stream data obeys FIFO rule.
*
**********************************************************************
*/
StreamFrame* StreamManagerRequestStream(StreamManager* pSm)
{
    StreamFrame*      pStreamFrame;
    StreamManagerCtx* p;

    p = (StreamManagerCtx*)pSm;

    if(p == NULL )
    {
        loge("pSm == NULL.");
        return NULL;
    }

    pthread_mutex_lock(&p->mutex);

    if(p->streamFifo.nValidFrameNum == 0)
    {
        loge("nValidFrameNum == 0.");
        pthread_mutex_unlock(&p->mutex);
        return NULL;
    }

    pStreamFrame = &p->streamFifo.pFrames[p->streamFifo.nReadPos];

    p->streamFifo.nReadPos++;
    if(p->streamFifo.nReadPos >= p->streamFifo.nMaxFrameNum)
        p->streamFifo.nReadPos = 0;

    pthread_mutex_unlock(&p->mutex);

    return pStreamFrame;
}


/*
**********************************************************************
*                      StreamManagerGetFrameInfo
*
*Description: get the frame information of the specific frame.
*
*Arguments  : pSm         Created by StreamManagerCreate function;
*             nFrameIndex the frame index start counted from the first
*                         valid frame.
*
*Return     : The stream information.
*
*Summary    : The stream data obeys FIFO rule.
*
**********************************************************************
*/
StreamFrame* StreamManagerGetFrameInfo(StreamManager* pSm, int nFrameIndex)
{
    StreamFrame*      pStreamFrame;
    int               nPos;
    StreamManagerCtx* p;

    p = (StreamManagerCtx*)pSm;

    if(p == NULL )
    {
        loge("pSm == NULL.");
        return NULL;
    }

    pthread_mutex_lock(&p->mutex);

    if(nFrameIndex < 0 || nFrameIndex >= p->streamFifo.nValidFrameNum)
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
*Arguments  : pSm          Created by StreamManagerCreate function;
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
int StreamManagerReturnStream(StreamManager* pSm, StreamFrame* pStreamFrame)
{
    int               nReadPos;
    StreamManagerCtx* p;

    p = (StreamManagerCtx*)pSm;

    if(p == NULL || pStreamFrame == NULL)
    {
        loge("input error.");
        return -1;
    }

    pthread_mutex_lock(&p->mutex);

    if(p->streamFifo.nValidFrameNum == 0)
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

    pthread_mutex_unlock(&p->mutex);
    return 0;
}


/*
**********************************************************************
*                       StreamManagerFlushStream
*
*Description: Flush one frame which is requested from StreamManager.
*
*Arguments  : pSm          Created by StreamManagerCreate function;
*             pStreamFrame the stream info need to be flushed.
*
*Return     : result;
*               = 0;    succeeded;
*               = -1;   failed.
*
*Summary    : After flushed, the buffer can be used to store new stream.
*
**********************************************************************
*/
int StreamManagerFlushStream(StreamManager* pSm, StreamFrame* pStreamFrame)
{
    int               nFlushPos;
    StreamManagerCtx* p;

    p = (StreamManagerCtx*)pSm;

    if(p == NULL)
    {
        loge("pSm == NULL.");
        return -1;
    }

    pthread_mutex_lock(&p->mutex);

    if(p->streamFifo.nValidFrameNum == 0)
    {
        loge("no valid frame.");
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }

    nFlushPos = p->streamFifo.nFlushPos;
    if(pStreamFrame != &p->streamFifo.pFrames[nFlushPos])
    {
        loge("flush frame not match.");
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }

    if(pStreamFrame->pData != NULL)
        free(pStreamFrame->pData);

    nFlushPos++;
    if(nFlushPos >= p->streamFifo.nMaxFrameNum)
        nFlushPos = 0;

    p->streamFifo.nValidFrameNum--;
    p->nValidDataSize      -= pStreamFrame->nLength;
    p->streamFifo.nFlushPos = nFlushPos;
    pthread_mutex_unlock(&p->mutex);
    return 0;
}
