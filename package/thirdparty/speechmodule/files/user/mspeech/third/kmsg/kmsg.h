#ifndef _KMSG_H_
#define _KMSG_H_

#ifdef __cplusplus
extern "C"
{
#endif

/*
 * kmsg.h
 *
 * 多线程安全的消息队列实现
 */

#include <pthread.h>

#define MAX_MSG_NUMBER  30

struct kmsg {
    unsigned int size;   // buffer capcacity
    unsigned int out;
    unsigned int in;
    unsigned char *data;
    int msg_number;
    int msg_part_len[MAX_MSG_NUMBER];
    pthread_mutex_t mutex;
};


int kmsg_new(struct kmsg *buf, int size);

int kmsg_delete(struct kmsg *buf);

int kmsg_push(struct kmsg *buf, const char *data, int len);

int kmsg_check(struct kmsg *buf);

int kmsg_pop(struct kmsg *buf, char *data);

#ifdef __cplusplus
}
#endif

#endif
