
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <stdint.h>
#include "cache.h"
#include "log.h"

StreamCache* StreamCacheCreate(void)
{
    StreamCache* c;
    c = (StreamCache*)malloc(sizeof(StreamCache));
    if(c == NULL)
        return NULL;
    memset(c, 0, sizeof(StreamCache));

    pthread_mutex_init(&c->mutex, NULL);
    return c;
}


void StreamCacheDestroy(StreamCache* c)
{
    CacheNode* node;

    pthread_mutex_lock(&c->mutex);
    node = c->pHead;
    while(node != NULL)
    {
        c->pHead = node->pNext;
        if(node->pData)
            free(node->pData);
        free(node);
        node = c->pHead;
    }
    pthread_mutex_unlock(&c->mutex);

    free(c);
    return;
}


void StreamCacheSetSize(StreamCache* c, int nStartPlaySize, int nMaxBufferSize)
{
    pthread_mutex_lock(&c->mutex);
    c->nMaxBufferSize = nMaxBufferSize;
    c->nStartPlaySize = nStartPlaySize;
    pthread_mutex_unlock(&c->mutex);
    return;
}


int StreamCacheUnderflow(StreamCache* c)
{
    int bUnderFlow;
    pthread_mutex_lock(&c->mutex);
    if(c->nFrameNum > 0)
        bUnderFlow = 0;
    else
        bUnderFlow = 1;
    pthread_mutex_unlock(&c->mutex);
    return bUnderFlow;
}


int StreamCacheOverflow(StreamCache* c)
{
    int bOverFlow;
    pthread_mutex_lock(&c->mutex);
    if(c->nTotalDataSize >= c->nMaxBufferSize)
        bOverFlow = 1;
    else
        bOverFlow = 0;
    pthread_mutex_unlock(&c->mutex);
    return bOverFlow;
}


int StreamCacheDataEnough(StreamCache* c)
{
    int bDataEnough;
    pthread_mutex_lock(&c->mutex);
    if(c->nTotalDataSize >= c->nStartPlaySize)
        bDataEnough = 1;
    else
        bDataEnough = 0;
    pthread_mutex_unlock(&c->mutex);
    return bDataEnough;
}


CacheNode* StreamCacheNextFrame(StreamCache* c)
{
    CacheNode* node;
    pthread_mutex_lock(&c->mutex);
    node = c->pHead;
    pthread_mutex_unlock(&c->mutex);
    return node;
}


void StreamCacheFlushOneFrame(StreamCache* c)
{
    CacheNode* node;

    pthread_mutex_lock(&c->mutex);
    node = c->pHead;
    if(node != NULL)
    {
        c->pHead  = node->pNext;
        c->nFrameNum--;
        c->nTotalDataSize -= node->nLength;
        if(node->pData != NULL)
            free(node->pData);
        free(node);

        if(c->pHead == NULL)
            c->pTail = NULL;
    }

    pthread_mutex_unlock(&c->mutex);
    return;
}


int StreamCacheAddOneFrame(StreamCache* c, CacheNode* node)
{
    CacheNode* newNode;

    newNode = (CacheNode*)malloc(sizeof(CacheNode));
    if(newNode == NULL)
        return -1;

    newNode->pData          = node->pData;
    newNode->nLength        = node->nLength;
    newNode->nPts           = node->nPts;
    newNode->nPcr           = node->nPcr;
    newNode->bIsFirstPart   = node->bIsFirstPart;
    newNode->bIsLastPart    = node->bIsLastPart;
    newNode->eMediaType     = node->eMediaType;
    newNode->nStreamIndex   = node->nStreamIndex;
    newNode->nFlags         = node->nFlags;
    newNode->pNext          = NULL;

    pthread_mutex_lock(&c->mutex);
    if(c->pTail != NULL)
    {
        c->pTail->pNext = newNode;
        c->pTail = c->pTail->pNext;
    }
    else
    {
        c->pTail = newNode;
        c->pHead = newNode;
    }
    c->nTotalDataSize += newNode->nLength;
    c->nFrameNum++;
    pthread_mutex_unlock(&c->mutex);

    return 0;
}


void StreamCacheFlushAll(StreamCache* c)
{
    CacheNode* node;

    pthread_mutex_lock(&c->mutex);
    node = c->pHead;
    while(node != NULL)
    {
        c->pHead = node->pNext;
        if(node->pData)
            free(node->pData);
        free(node);
        node = c->pHead;
    }

    c->pTail          = NULL;
    c->nTotalDataSize = 0;
    c->nFrameNum      = 0;

    pthread_mutex_unlock(&c->mutex);
    return;
}
