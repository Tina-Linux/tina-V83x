#ifndef WFD_MESSAGE_QUEUE_H
#define WFD_MESSAGE_QUEUE_H

#include <stdint.h>
#include <semaphore.h>
#include <stddef.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct MessageQueueContext WFDMessageQueue;

#ifndef uintptr_t
typedef size_t uintptr_t;
#endif

typedef struct WFDMessage WFDMessage;
typedef void (*msgHandlerT)(WFDMessage *msg, void *arg);

/* Define your own struct WFDMessage and put WFDMessage_COMMON_MEMBERS at the
 * beginning
 */
#define WFDMessage_COMMON_MEMBERS \
    int          messageId; \
    msgHandlerT  execute; \

/**
 * @param nMaxMessageNum How many messages the message queue can hold
 * @param pName The name of the message queue which is used in log output
 * @param nMessageSize sizeof(struct WFDMessage)
 */
WFDMessageQueue* WFDMessageQueueCreate__(int nMaxMessageNum, const char* pName,
                                    size_t nMessageSize);
#define WFDMessageQueueCreate(nMaxMessageNum, pName) \
        WFDMessageQueueCreate__(nMaxMessageNum, pName, sizeof(WFDMessage))

void WFDMessageQueueDestroy(WFDMessageQueue* mq);

int WFDMessageQueuePostMessage(WFDMessageQueue* mq, WFDMessage* m);

int WFDMessageQueueWaitMessage(WFDMessageQueue* mq, int64_t timeout);

int WFDMessageQueueGetMessage(WFDMessageQueue* mq, WFDMessage* m);

int WFDMessageQueueTryGetMessage(WFDMessageQueue* mq, WFDMessage* m, int64_t timeout);

int WFDMessageQueueFlush(WFDMessageQueue* mq);

int WFDMessageQueueGetCount(WFDMessageQueue* mq);

//* define a semaphore timedwait method for common use.
int SemTimedWait(sem_t* sem, int64_t time_ms);

#ifdef __cplusplus
}
#endif

#endif //WFD_MESSAGE_QUEUE_H
