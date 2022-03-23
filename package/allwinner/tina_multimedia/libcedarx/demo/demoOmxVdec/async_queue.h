#ifndef ASYNC_QUEUE_H
#define ASYNC_QUEUE_H
#include <pthread.h>

typedef struct _GList GList;

struct _GList
{
    void* data;
    GList *next;
    GList *prev;
};

typedef struct AsyncQueue
{
    pthread_mutex_t mutex;
    pthread_cond_t condition;
    GList *head;
    GList *tail;
    unsigned int length;
    int enabled;
} AsyncQueue;

AsyncQueue *async_queue_new (void);
void async_queue_free (AsyncQueue * queue);
void async_queue_push (AsyncQueue * queue, void* data);
void* async_queue_pop (AsyncQueue * queue);
int async_queue_empty(AsyncQueue *queue);
void* async_queue_pop_forced (AsyncQueue * queue);
void async_queue_disable (AsyncQueue * queue);
void async_queue_enable (AsyncQueue * queue);
void async_queue_flush (AsyncQueue * queue);

#endif /* ASYNC_QUEUE_H */
