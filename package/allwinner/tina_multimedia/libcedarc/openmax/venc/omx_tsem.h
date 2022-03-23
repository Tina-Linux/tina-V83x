/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : omx_tsem.h
 *
 * Description : pthread operation
 * History :
 *
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#ifndef __OMX_TSEM_H__
#define __OMX_TSEM_H__

#include <pthread.h>
#include <stdint.h>
typedef struct omx_sem_t
{
    pthread_cond_t  sCondition;
    pthread_mutex_t mMutex;
    unsigned int    uSemval;
}omx_sem_t;

int omx_sem_init(omx_sem_t* tsem, unsigned int val);
void omx_sem_deinit(omx_sem_t* tsem);
void omx_sem_down(omx_sem_t* tsem);
void omx_sem_up(omx_sem_t* tsem);
void omx_sem_reset(omx_sem_t* tsem);
void omx_sem_wait(omx_sem_t* tsem);
void omx_sem_signal(omx_sem_t* tsem);

void omx_sem_timeddown(omx_sem_t* tsem, int64_t time_ms);

#endif //__OMX_TSEM_H__

#ifdef __cplusplus
}
#endif /* __cplusplus */
