/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : cache.h
 * Description : cache
 * History :
 *
 */


#ifndef CACHE_H
#define CACHE_H

#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct CacheNode_t CacheNode;
struct CacheNode_t
{
    unsigned char* pData;
    int            nLength;
    int64_t        nPts;
    int64_t        nPcr;
    int            bIsFirstPart;
    int            bIsLastPart;
    int            eMediaType;
    int            nStreamIndex;
    int            nFlags;
    CacheNode*     pNext;
};

typedef struct StreamCache_t
{
    int             nMaxBufferSize;
    int             nStartPlaySize;
    int             nTotalDataSize;
    int             nFrameNum;
    CacheNode*      pHead;
    CacheNode*      pTail;
    pthread_mutex_t mutex;
}StreamCache;

StreamCache* StreamCacheCreate(void);

void StreamCacheDestroy(StreamCache* c);

void StreamCacheSetSize(StreamCache* c, int nStartPlaySize, int nMaxBufferSize);

int StreamCacheUnderflow(StreamCache* c);

int StreamCacheOverflow(StreamCache* c);

int StreamCacheDataEnough(StreamCache* c);

CacheNode* StreamCacheNextFrame(StreamCache* c);

void StreamCacheFlushOneFrame(StreamCache* c);

int StreamCacheAddOneFrame(StreamCache* c, CacheNode* node);

void StreamCacheFlushAll(StreamCache* c);

#endif
