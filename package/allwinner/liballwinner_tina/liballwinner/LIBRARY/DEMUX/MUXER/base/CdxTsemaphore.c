/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : CdxTsemaphore.c
 * Description : Allwinner Muxer Writer with Thread Operation
 * History :
 *   Author  : Q Wang <eric_wang@allwinnertech.com>
 *   Date    : 2014/12/17
 *   Comment : 创建初始版本，用于V3 CedarX_1.0
 *
 *   Author  : GJ Wu <wuguanjian@allwinnertech.com>
 *   Date    : 2016/04/18
 *   Comment_: 修改移植用于CedarX_2.0
 */

#include <pthread.h>
#include <sys/time.h>
#include <errno.h>
#include <log.h>
#include "CdxTsemaphore.h"

/** Initializes the semaphore at a given value
 *
 * @param tsem the semaphore to initialize
 * @param val the initial value of the semaphore
 *
 */
int CdxSemInit(CdxSemT *tsem, unsigned int val)
{
    int i;

    i = pthread_cond_init(&tsem->condition, NULL);
    if (i!=0)
        return -1;

    i = pthread_mutex_init(&tsem->mutex, NULL);
    if (i!=0)
        return -1;

    tsem->semval = val;

    return 0;
}

/** Destroy the semaphore
 *
 * @param tsem the semaphore to destroy
 */
void CdxSemDeinit(CdxSemT *tsem)
{
    pthread_cond_destroy(&tsem->condition);
    pthread_mutex_destroy(&tsem->mutex);
}

/** Decreases the value of the semaphore. Blocks if the semaphore
 * value is zero.
 *
 * @param tsem the semaphore to decrease
 */
void CdxSemDown(CdxSemT *tsem)
{
    pthread_mutex_lock(&tsem->mutex);

    logv("semdown:%p val:%d",tsem,tsem->semval);
    while (tsem->semval == 0)
    {
        logv("semdown wait:%p val:%d",tsem,tsem->semval);
        pthread_cond_wait(&tsem->condition, &tsem->mutex);
        logv("semdown wait end:%p val:%d",tsem,tsem->semval);
    }

    tsem->semval--;
    pthread_mutex_unlock(&tsem->mutex);
}

/** Increases the value of the semaphore
 *
 * @param tsem the semaphore to increase
 */
void CdxSemUp(CdxSemT *tsem)
{
    pthread_mutex_lock(&tsem->mutex);

    tsem->semval++;
    logv("semup signal:%p val:%d",tsem,tsem->semval);
    pthread_cond_signal(&tsem->condition);

    pthread_mutex_unlock(&tsem->mutex);
}
void CdxSemUpUnique(CdxSemT *tsem)
{
    pthread_mutex_lock(&tsem->mutex);

    if(0 == tsem->semval)
    {
        tsem->semval++;
        pthread_cond_signal(&tsem->condition);
    }

    pthread_mutex_unlock(&tsem->mutex);
}

/** Reset the value of the semaphore
 *
 * @param tsem the semaphore to reset
 */
void CdxSemReset(CdxSemT *tsem)
{
    pthread_mutex_lock(&tsem->mutex);

    tsem->semval=0;

    pthread_mutex_unlock(&tsem->mutex);
}

/** Wait on the condition.
 *
 * @param tsem the semaphore to wait
 */
void CdxSemWait(CdxSemT *tsem)
{
    pthread_mutex_lock(&tsem->mutex);

    pthread_cond_wait(&tsem->condition, &tsem->mutex);

    pthread_mutex_unlock(&tsem->mutex);
}

/** Signal the condition,if waiting
 *
 * @param tsem the semaphore to signal
 */
void CdxSemSignal(CdxSemT *tsem)
{
    pthread_mutex_lock(&tsem->mutex);

    pthread_cond_signal(&tsem->condition);

    pthread_mutex_unlock(&tsem->mutex);
}

unsigned int CdxSemGetVal(CdxSemT *tsem)
{
    return tsem->semval;
}
