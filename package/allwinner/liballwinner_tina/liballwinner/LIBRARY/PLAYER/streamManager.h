
#ifndef STREAM_MANAGER_H
#define STREAM_MANAGER_H

#include <pthread.h>
#include <stdint.h>

typedef struct STREAMFRAME
{
    void*   pData;
    int     nLength;
    int64_t nPts;
    int64_t nPcr;
    int64_t nDuration;
}StreamFrame;

typedef void* StreamManager;


StreamManager* StreamManagerCreate(int nMaxBufferSize, int nMaxFrameNum, int nStreamID);

void StreamManagerDestroy(StreamManager* pSm);

void StreamManagerReset(StreamManager* pSm);

int StreamManagerBufferSize(StreamManager* pSm);

int StreamManagerStreamFrameNum(StreamManager* pSm);

int StreamManagerStreamDataSize(StreamManager* pSm);

int StreamManagerRequestBuffer(StreamManager* pSm, int nRequireSize, char** ppBuf, int* pBufSize);

int StreamManagerAddStream(StreamManager* pSm, StreamFrame* pStreamFrame);

StreamFrame* StreamManagerRequestStream(StreamManager* pSm);

StreamFrame* StreamManagerGetFrameInfo(StreamManager* pSm, int nFrameIndex);

int StreamManagerReturnStream(StreamManager* pSm, StreamFrame* pStreamFrame);

int StreamManagerFlushStream(StreamManager* pSm, StreamFrame* pStreamFrame);


#endif
