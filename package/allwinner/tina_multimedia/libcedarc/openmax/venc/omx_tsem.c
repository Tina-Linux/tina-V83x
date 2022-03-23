/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : omx_tsem.c
 *
 * Description : pthread operation
 * History :
 *
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include "omx_tsem.h"
#include "log.h"

/** Initializes the semaphore at a given value
 *
 * @param tsem the semaphore to initialize
 * @param val the initial value of the semaphore
 *
 */
int omx_sem_init(omx_sem_t* tsem, unsigned int uVal)
{
    int nRet;

    nRet = pthread_cond_init(&tsem->sCondition, NULL);
    if (nRet!=0)
    {
        return -1;
    }

    nRet = pthread_mutex_init(&tsem->mMutex, NULL);
    if (nRet!=0)
    {
        return -1;
    }

    tsem->uSemval = uVal;

    return 0;
}

/** Destroy the semaphore
 *
 * @param tsem the semaphore to destroy
 */
void omx_sem_deinit(omx_sem_t* tsem)
{
    pthread_cond_destroy(&tsem->sCondition);
    pthread_mutex_destroy(&tsem->mMutex);
}

/** Decreases the value of the semaphore. Blocks if the semaphore
 * value is zero.
 *
 * @param tsem the semaphore to decrease
 */
void omx_sem_down(omx_sem_t* tsem)
{
    pthread_mutex_lock(&tsem->mMutex);

    while (tsem->uSemval == 0)
    {
        pthread_cond_wait(&tsem->sCondition, &tsem->mMutex);
    }

    tsem->uSemval--;
    pthread_mutex_unlock(&tsem->mMutex);
}

/** Increases the value of the semaphore
 *
 * @param tsem the semaphore to increase
 */
void omx_sem_up(omx_sem_t* tsem)
{
    pthread_mutex_lock(&tsem->mMutex);

    tsem->uSemval++;
    pthread_cond_signal(&tsem->sCondition);

    pthread_mutex_unlock(&tsem->mMutex);
}

/** Reset the value of the semaphore
 *
 * @param tsem the semaphore to reset
 */
void omx_sem_reset(omx_sem_t* tsem)
{
    CEDARC_UNUSE(omx_sem_reset);

    pthread_mutex_lock(&tsem->mMutex);

    tsem->uSemval=0;

    pthread_mutex_unlock(&tsem->mMutex);
}

/** Wait on the sCondition.
 *
 * @param tsem the semaphore to wait
 */
void omx_sem_wait(omx_sem_t* tsem)
{
    CEDARC_UNUSE(omx_sem_wait);

    pthread_mutex_lock(&tsem->mMutex);

    pthread_cond_wait(&tsem->sCondition, &tsem->mMutex);

    pthread_mutex_unlock(&tsem->mMutex);
}

/** Wait on the sCondition.
 *
 * @param tsem the semaphore to wait
 */
void omx_sem_timeddown(omx_sem_t* tsem, int64_t time_ms)
{
    struct timespec ts;
    clock_gettime(CLOCK_REALTIME, &ts);
    ts.tv_nsec += time_ms % 1000 * 1000 * 1000;
    ts.tv_sec += time_ms / 1000 + ts.tv_nsec / (1000 * 1000 * 1000);
    ts.tv_nsec = ts.tv_nsec % (1000*1000*1000);

    pthread_mutex_lock(&tsem->mMutex);

    if(tsem->uSemval == 0)
    {
        pthread_cond_timedwait(&tsem->sCondition, &tsem->mMutex, &ts);
    }
    tsem->uSemval--;
    pthread_mutex_unlock(&tsem->mMutex);
}

/** Signal the sCondition,if waiting
 *
 * @param tsem the semaphore to signal
 */
void omx_sem_signal(omx_sem_t* tsem)
{
    CEDARC_UNUSE(omx_sem_signal);

    pthread_mutex_lock(&tsem->mMutex);

    pthread_cond_signal(&tsem->sCondition);

    pthread_mutex_unlock(&tsem->mMutex);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
