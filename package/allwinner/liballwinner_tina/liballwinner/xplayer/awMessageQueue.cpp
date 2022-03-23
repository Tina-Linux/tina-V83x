/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : awMessageQueue.cpp
 * Description : MessageQueue
 * History :
 *
 */


#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>
#include <semaphore.h>
#include <sys/time.h>
#include <stdint.h>
#include "awMessageQueue.h"
#include "log.h"

typedef struct AwMessageNode
{
    AwMessage      msg;
    AwMessageNode* next;
    int            valid;
}AwMessageNode;

typedef struct AwMessageQueueContext
{
    AwMessageNode*  pHead;
    int             nCount;
    AwMessageNode*  Nodes;
    int             nMaxMessageNum;
    pthread_mutex_t mutex;
    sem_t           sem;
}AwMessageQueueContext;


AwMessageQueue* AwMessageQueueCreate(int nMaxMessageNum)
{
    AwMessageQueueContext* mqCtx;

    logv("create message queue.");

    mqCtx = (AwMessageQueueContext*)malloc(sizeof(AwMessageQueueContext));
    if(mqCtx == NULL)
    {
        loge("allocate memory fail.");
        return NULL;
    }
    memset(mqCtx, 0, sizeof(AwMessageQueueContext));

    mqCtx->Nodes = (AwMessageNode*)malloc(nMaxMessageNum*sizeof(AwMessageNode));
    if(mqCtx->Nodes == NULL)
    {
        loge("allocate memory for message nodes fail.");
        free(mqCtx);
        return NULL;
    }
    memset(mqCtx->Nodes, 0, sizeof(AwMessageNode)*nMaxMessageNum);

    mqCtx->nMaxMessageNum = nMaxMessageNum;

    pthread_mutex_init(&mqCtx->mutex, NULL);
    sem_init(&mqCtx->sem, 0, 0);

    return (AwMessageQueue*)mqCtx;
}


void AwMessageQueueDestroy(AwMessageQueue* mq)
{
    AwMessageQueueContext* mqCtx;

    logv("destroy message queue.");

    mqCtx = (AwMessageQueueContext*)mq;

    if(mqCtx->Nodes != NULL)
    {
        free(mqCtx->Nodes);
    }

    pthread_mutex_destroy(&mqCtx->mutex);
    sem_destroy(&mqCtx->sem);

    free(mqCtx);

    return;
}


int AwMessageQueuePostMessage(AwMessageQueue* mq, AwMessage* m)
{
    AwMessageQueueContext* mqCtx;
    AwMessageNode*         node;
    AwMessageNode*         ptr;
    int                    i;

    mqCtx = (AwMessageQueueContext*)mq;

    logv("post message.");

    pthread_mutex_lock(&mqCtx->mutex);

    if(mqCtx->nCount >= mqCtx->nMaxMessageNum)
    {
        loge("message count exceed, current message count = %d, max message count = %d",
                mqCtx->nCount, mqCtx->nMaxMessageNum);
        pthread_mutex_unlock(&mqCtx->mutex);
        return -1;
    }

    node = NULL;
    ptr  = mqCtx->Nodes;
    for(i=0; i<mqCtx->nMaxMessageNum; i++, ptr++)
    {
        if(ptr->valid == 0)
        {
            node = ptr;
            break;
        }
    }

    node->msg.messageId = m->messageId;
    node->msg.params[0] = m->params[0];
    node->msg.params[1] = m->params[1];
    node->msg.params[2] = m->params[2];
    node->msg.params[3] = m->params[3];
    node->msg.params[4] = m->params[4];
    node->msg.params[5] = m->params[5];
    node->msg.params[6] = m->params[6];
    node->msg.params[7] = m->params[7];
    node->valid         = 1;
    node->next          = NULL;

    ptr = mqCtx->pHead;
    if(ptr == NULL)
        mqCtx->pHead = node;
    else
    {
        while(ptr->next != NULL)
            ptr = ptr->next;

        ptr->next = node;
    }

    mqCtx->nCount++;

    pthread_mutex_unlock(&mqCtx->mutex);

    sem_post(&mqCtx->sem);

    return 0;
}


int AwMessageQueueGetMessage(AwMessageQueue* mq, AwMessage* m)
{
    return AwMessageQueueTryGetMessage(mq, m, -1);
}


int AwMessageQueueTryGetMessage(AwMessageQueue* mq, AwMessage* m, int64_t timeout)
{
    AwMessageQueueContext* mqCtx;
    AwMessageNode*         node;

    mqCtx = (AwMessageQueueContext*)mq;

    logv("try get message.");

    if(SemTimedWait(&mqCtx->sem, timeout) < 0)
    {
        return -1;
    }

    pthread_mutex_lock(&mqCtx->mutex);

    if(mqCtx->nCount <= 0)
    {
        logv("no message.");
        pthread_mutex_unlock(&mqCtx->mutex);
        return -1;
    }

    node = mqCtx->pHead;
    mqCtx->pHead = node->next;

    m->messageId = node->msg.messageId;
    m->params[0] = node->msg.params[0];
    m->params[1] = node->msg.params[1];
    m->params[2] = node->msg.params[2];
    m->params[3] = node->msg.params[3];
    m->params[4] = node->msg.params[4];
    m->params[5] = node->msg.params[5];
    m->params[6] = node->msg.params[6];
    m->params[7] = node->msg.params[7];
    node->valid  = 0;

    mqCtx->nCount--;

    pthread_mutex_unlock(&mqCtx->mutex);

    return 0;
}


int AwMessageQueueFlush(AwMessageQueue* mq)
{
    AwMessageQueueContext* mqCtx;
    int                    i;

    mqCtx = (AwMessageQueueContext*)mq;

    logv("flush messages.");

    pthread_mutex_lock(&mqCtx->mutex);

    mqCtx->pHead  = NULL;
    mqCtx->nCount = 0;
    for(i=0; i<mqCtx->nMaxMessageNum; i++)
    {
        mqCtx->Nodes[i].valid = 0;
    }

    do
    {
        if(sem_getvalue(&mqCtx->sem, &i) != 0 || i == 0)
            break;

        sem_trywait(&mqCtx->sem);

    }while(1);

    pthread_mutex_unlock(&mqCtx->mutex);

    return 0;
}


int AwMessageQueueGetCount(AwMessageQueue* mq)
{
    AwMessageQueueContext* mqCtx;

    mqCtx = (AwMessageQueueContext*)mq;

    logv("get message count.");

    return mqCtx->nCount;
}


int SemTimedWait(sem_t* sem, int64_t time_ms)
{
    int err;

    struct timeval  tv;
    struct timespec ts;

    if(time_ms == -1)
    {
        err = sem_wait(sem);
    }
    else
    {
        gettimeofday(&tv, NULL);
        ts.tv_sec  = tv.tv_sec;
        ts.tv_nsec = tv.tv_usec*1000 + time_ms*1000000;
        ts.tv_sec += ts.tv_nsec/(1000*1000*1000);
        ts.tv_nsec = ts.tv_nsec % (1000*1000*1000);

        err = sem_timedwait(sem, &ts);
    }

    return err;
}


void setMessage(AwMessage* msg,
                int        cmd,
                uintptr_t        param0,
                uintptr_t        param1,
                uintptr_t        param2,
                uintptr_t        param3,
                uintptr_t        param4,
                uintptr_t        param5,
                uintptr_t        param6,
                uintptr_t        param7)
{
    msg->messageId = cmd;
    msg->params[0] = param0;
    msg->params[1] = param1;
    msg->params[2] = param2;
    msg->params[3] = param3;
    msg->params[4] = param4;
    msg->params[5] = param5;
    msg->params[6] = param6;
    msg->params[7] = param7;
}
