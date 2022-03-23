#include <stdlib.h>
#include <string.h>
#include "async_queue.h"


GList* list_prepend (GList *list,void*  data)
{
    GList *new_list;

    new_list = (GList *)malloc(sizeof(GList));
    new_list->prev = NULL;
    new_list->data = data;
    new_list->next = list;

    if (list)
    {
        new_list->prev = list->prev;
        if (list->prev)
            list->prev->next = new_list;
        list->prev = new_list;
    }
    else
        new_list->prev = NULL;

    return new_list;
}

void list_free(GList *list)
{
    if(list)
    {
        GList *current = list;
        GList *next = current->next;

        free(current);
        while(next!= NULL)
        {
            current = next;
            next = current->next;
            free(current);
        }
    }
}

AsyncQueue * async_queue_new (void)
{
    AsyncQueue *queue;

    queue = (AsyncQueue *)malloc(sizeof(AsyncQueue));

    memset(queue, 0, sizeof(AsyncQueue));

    pthread_cond_init(&queue->condition, NULL);
    pthread_mutex_init(&queue->mutex, NULL);
    queue->enabled = 1;

    return queue;
}

void async_queue_free (AsyncQueue * queue)
{
    pthread_cond_destroy(&queue->condition);
    pthread_mutex_destroy(&queue->mutex);

    list_free (queue->head);
    free(queue);
}


void async_queue_push (AsyncQueue * queue, void* data)
{
    pthread_mutex_lock(&queue->mutex);

    queue->head = list_prepend (queue->head, data);
    if (!queue->tail)
        queue->tail = queue->head;
    queue->length++;

    pthread_cond_signal(&queue->condition);

    pthread_mutex_unlock(&queue->mutex);
}

int async_queue_empty(AsyncQueue *queue)
{
    return (queue->length == 0 ? 1:0);
}

void* async_queue_pop (AsyncQueue * queue)
{
    void* data = NULL;

    if (!queue->enabled)
    {
        /* logw ("not enabled!"); */
        goto leave;
    }

    if (async_queue_empty(queue))
        goto leave;

    pthread_mutex_lock (&queue->mutex);

    if (!queue->tail)
    {
        pthread_cond_wait (&queue->condition, &queue->mutex);
    }

    if (queue->tail)
    {
        GList *node = queue->tail;
        data = node->data;

        queue->tail = node->prev;
        if (queue->tail)
            queue->tail->next = NULL;
        else
            queue->head = NULL;
        queue->length--;
        free(node);
    }

leave:
    pthread_mutex_unlock(&queue->mutex);

    return data;
}

void* async_queue_pop_forced (AsyncQueue * queue)
{
    void* data = NULL;

    pthread_mutex_lock(&queue->mutex);

    if (queue->tail)
    {
        GList *node = queue->tail;
        data = node->data;

        queue->tail = node->prev;
        if (queue->tail)
            queue->tail->next = NULL;
        else
            queue->head = NULL;
        queue->length--;
        free(node);
    }

    pthread_mutex_unlock(&queue->mutex);

    return data;
}

void async_queue_disable (AsyncQueue * queue)
{
    pthread_mutex_lock(&queue->mutex);
    queue->enabled = 0;
    pthread_cond_broadcast(&queue->condition);
    pthread_mutex_unlock(&queue->mutex);
}

void async_queue_enable (AsyncQueue * queue)
{
    pthread_mutex_lock(&queue->mutex);
    queue->enabled = 1;
    pthread_mutex_unlock(&queue->mutex);
}

void async_queue_flush (AsyncQueue * queue)
{
    pthread_mutex_lock(&queue->mutex);
    list_free (queue->head);
    queue->head = queue->tail = NULL;
    queue->length = 0;
    pthread_mutex_unlock(&queue->mutex);
}
