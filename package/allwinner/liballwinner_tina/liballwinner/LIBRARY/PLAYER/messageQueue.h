
#ifndef MESSAGE_QUEUE_H
#define MESSAGE_QUEUE_H

#include <stdint.h>
#include <semaphore.h>
#include <sys/time.h>

typedef void* MessageQueue;

typedef struct Message
{
    int          messageId;
    uintptr_t    params[4];
}Message;

MessageQueue* MessageQueueCreate(int nMaxMessageNum, const char* pName = "unknown");

void MessageQueueDestroy(MessageQueue* mq);

int MessageQueuePostMessage(MessageQueue* mq, Message* m);

int MessageQueueGetMessage(MessageQueue* mq, Message* m);

int MessageQueueTryGetMessage(MessageQueue* mq, Message* m, int64_t timeout);

int MessageQueueFlush(MessageQueue* mq);

int MessageQueueGetCount(MessageQueue* mq);

//* define a semaphore timedwait method for common use.
static int SemTimedWait(sem_t* sem, int64_t time_ms)
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

#endif
