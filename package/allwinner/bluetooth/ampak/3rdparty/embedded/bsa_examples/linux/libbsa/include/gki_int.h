/****************************************************************************
 **
 **  Name        gki_int.h
 **
 **  Function    This file contains GKI private definitions
 **
 **
 **  Copyright (c) 1999-2012, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef GKI_INT_H
#define GKI_INT_H

#include "gki_common.h"
#include <pthread.h>

/**********************************************************************
** OS specific definitions
*/

typedef struct
{
    pthread_t           id;
    pthread_mutex_t     evt_mutex;
    pthread_cond_t      evt_cond;
    TASKPTR             start;
} tOS_THREAD;

typedef struct
{
    tOS_THREAD          thread[GKI_MAX_TASKS];
    pthread_mutex_t     mutex;
    pthread_t           timer_thread_id;
} tGKI_OS;


/* Contains common control block as well as OS specific variables */
typedef struct
{
    tGKI_OS     os;
    tGKI_COM_CB com;
} tGKI_CB;


#ifdef __cplusplus
extern "C" {
#endif

#if GKI_DYNAMIC_MEMORY == FALSE
GKI_API extern tGKI_CB gki_cb;
#else
GKI_API extern tGKI_CB *gki_cb_ptr;
#define gki_cb (*gki_cb_ptr)
#endif

#ifndef GKI_SYSTEM_TIMESTAMP
#define GKI_SYSTEM_TIMESTAMP TRUE
#endif
#ifndef GKI_TIMESTAMP_CLOCK
#define GKI_TIMESTAMP_CLOCK CLOCK_MONOTONIC
#endif
#define GKI_MAX_TIMESTAMP_BUF_SIZE 40

#if ((defined(__FreeBSD__)) || (defined(CLOCK_NANOSLEEP_UNDEFINED)))
/*need to implement clock_nanosleep for FreeBSD;
 * some toolchain may not implement it as well, so need to implemen here  */
/*
 * NOTE:
 *   If the thread is preempted after getting the current time,
 *   it may sleep more than the specified time.
 */
#include <time.h>
#include <errno.h>
static inline int
clock_nanosleep(clockid_t clock_id, int flags,
        const struct timespec *request, struct timespec *remain)
{
    int error;
    const struct timespec *tsp;
    struct timespec ts;
    if (flags == TIMER_ABSTIME) {
        error = clock_gettime(clock_id, &ts);
        if (error != 0) {
            return (error);
        }
        ts.tv_sec = request->tv_sec - ts.tv_sec;
        ts.tv_nsec = request->tv_nsec - ts.tv_nsec;
        if (ts.tv_nsec < 0) {
            ts.tv_sec--;
            ts.tv_nsec += 1000000000;
        }
        tsp = (const struct timespec *)&ts;
    }
    else {
        tsp = request;
    }

    if (tsp->tv_sec < 0 || tsp->tv_nsec < 0 || tsp->tv_nsec > 999999999L) {
        errno = EINVAL;
        return (-1);
    }

    error = nanosleep(tsp, remain);
    return (error);
}
#endif

#ifdef __cplusplus
}

#endif
#endif
