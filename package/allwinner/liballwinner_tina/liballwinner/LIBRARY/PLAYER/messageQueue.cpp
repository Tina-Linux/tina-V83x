
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include <malloc.h>
#include <semaphore.h>
#include <string.h>

#include "messageQueue.h"
#include "log.h"

typedef struct MessageNode
{
    Message      msg;
    MessageNode* next;
    int          valid;
}MessageNode;

typedef struct MessageQueueContext
{
    char*           pName;
    MessageNode*    pHead;
    int             nCount;
    MessageNode*    Nodes;
    int             nMaxMessageNum;
    pthread_mutex_t mutex;
    sem_t           sem;
}MessageQueueContext;


MessageQueue* MessageQueueCreate(int nMaxMessageNum, const char* pName)
{
    MessageQueueContext* mqCtx;

    mqCtx = (MessageQueueContext*)malloc(sizeof(MessageQueueContext));
    if(mqCtx == NULL)
    {
        loge("%s, allocate memory fail.", pName);
        return NULL;
    }
    memset(mqCtx, 0, sizeof(MessageQueueContext));

    if(pName != NULL)
        mqCtx->pName = strdup(pName);

    mqCtx->Nodes = (MessageNode*)malloc(nMaxMessageNum*sizeof(MessageNode));
    if(mqCtx->Nodes == NULL)
    {
        loge("%s, allocate memory for message nodes fail.", mqCtx->pName);
        free(mqCtx);
        return NULL;
    }
    memset(mqCtx->Nodes, 0, sizeof(MessageNode)*nMaxMessageNum);

    mqCtx->nMaxMessageNum = nMaxMessageNum;

    pthread_mutex_init(&mqCtx->mutex, NULL);
    sem_init(&mqCtx->sem, 0, 0);

    return (MessageQueue*)mqCtx;
}


void MessageQueueDestroy(MessageQueue* mq)
{
    MessageQueueContext* mqCtx;

    mqCtx = (MessageQueueContext*)mq;

    if(mqCtx->Nodes != NULL)
    {
        free(mqCtx->Nodes);
    }

    pthread_mutex_destroy(&mqCtx->mutex);
    sem_destroy(&mqCtx->sem);

    if(mqCtx->pName != NULL)
        free(mqCtx->pName);

    free(mqCtx);

    return;
}


int MessageQueuePostMessage(MessageQueue* mq, Message* m)
{
    MessageQueueContext* mqCtx;
    MessageNode*         node;
    MessageNode*         ptr;
    int                  i;

    mqCtx = (MessageQueueContext*)mq;

    pthread_mutex_lock(&mqCtx->mutex);

    if(mqCtx->nCount >= mqCtx->nMaxMessageNum)
    {
        loge("%s, message count exceed, current message count = %d, max message count = %d",
                mqCtx->pName, mqCtx->nCount, mqCtx->nMaxMessageNum);
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


int MessageQueueGetMessage(MessageQueue* mq, Message* m)
{
    return MessageQueueTryGetMessage(mq, m, -1);
}


int MessageQueueTryGetMessage(MessageQueue* mq, Message* m, int64_t timeout)
{
    MessageQueueContext* mqCtx;
    MessageNode*         node;

    mqCtx = (MessageQueueContext*)mq;

    if(SemTimedWait(&mqCtx->sem, timeout) < 0)
    {
        return -1;
    }

    pthread_mutex_lock(&mqCtx->mutex);

    if(mqCtx->nCount <= 0)
    {
        logv("%s, no message.", mqCtx->pName);
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
    node->valid  = 0;

    mqCtx->nCount--;

    pthread_mutex_unlock(&mqCtx->mutex);

    return 0;
}


int MessageQueueFlush(MessageQueue* mq)
{
    MessageQueueContext* mqCtx;
    int                  i;

    mqCtx = (MessageQueueContext*)mq;

    logi("%s, flush messages.", mqCtx->pName);

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


int MessageQueueGetCount(MessageQueue* mq)
{
    MessageQueueContext* mqCtx;

    mqCtx = (MessageQueueContext*)mq;

    return mqCtx->nCount;
}
