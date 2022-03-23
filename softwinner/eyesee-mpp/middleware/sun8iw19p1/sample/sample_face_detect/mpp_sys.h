#ifndef __MPP_SYS__
#define __MPP_SYS__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

#include <utils/plat_log.h>
#include "media/mpi_sys.h"
#include "mm_comm_sys.h"
#include "mm_common.h"

#define MAX(x, y)   ((x) > (y) ? (x) : (y))
#define MIN(x, y)   ((x) < (y) ? (x) : (y))

typedef void* (*ProcessFuncType)(void *pThreadData);

typedef struct _THREAD_DATA
{
    ProcessFuncType ProcessFunc;
    pthread_t   mThreadID;
    void*       pPrivateData;
}THREAD_DATA;
	
int mpp_init();
int mpp_destroy();
int64_t mpp_getCurTimeUs();

#endif