/*
 * =====================================================================================
 *
 *       Filename:  omx_sem.c
 *
 *      Description: pack the sem with name, and use a flag to show the log
 *
 *        Version:  1.0
 *        Created:  08/02/2018 04:18:11 PM
 *       Revision:  none
 *
 *         Author:  Gan Qiuye
 *        Company:
 *
 * =====================================================================================
 */

#define LOG_TAG "omx_sem"
#include "log.h"
#include "omx_sem.h"
#include <sys/prctl.h>
#include <malloc.h>
#include <sys/resource.h>
#include <string.h>
#include <time.h>

OmxSemHandle OmxCreateSem(const char* name, int pShared, unsigned int value,
                                OMX_BOOL bShowlogFlag)
{
    struct OmxSem_obj * pHandler = NULL;
    pHandler = (struct OmxSem_obj *)malloc(sizeof(struct OmxSem_obj));
    if(pHandler != NULL)
    {
        if(sem_init(&pHandler->mSem, pShared, value) == -1)
        {
            loge("failed to create the OmxSemHandle!");
            free(pHandler);
            pHandler = NULL;
        }
        else
        {
            pHandler->mName = strdup(name);
            pHandler->bShowLog = bShowlogFlag;
        }
    }
    return (OmxSemHandle)pHandler;
}

int  OmxDestroySem(OmxSemHandle* pHandler)
{
    OmxSemHandle handler = *pHandler;
    if(handler == NULL)
    {
        loge("handle is null");
        return -1;
    }

    sem_destroy(&handler->mSem);
    free(handler->mName);
    free(handler);
    return 0;

}

/*
*    NOTE: one sem just only has one OmxTimedWaitSem when use the OmxTryPostSem to post sem
*/
int OmxTimedWaitSem(OmxSemHandle handler, OMX_S64 time_ms)
{
    int err;
    if(handler == NULL)
    {
        loge("handle is null");
        return -1;
    }
    if(handler->bShowLog)
        logd("wait for sem:%s begin", handler->mName);

    if(time_ms == -1)
    {

        err = sem_wait(&handler->mSem);
        if(handler->bShowLog)
            logd("wait for sem:%s done!", handler->mName);

    }
    else
    {
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME, &ts);
        ts.tv_nsec += time_ms % 1000 * 1000 * 1000;
        ts.tv_sec += time_ms / 1000 + ts.tv_nsec / (1000 * 1000 * 1000);
        ts.tv_nsec = ts.tv_nsec % (1000*1000*1000);

        err = sem_timedwait(&handler->mSem, &ts);
        if(handler->bShowLog)
            logd("wait for sem:%s %lldms done!", handler->mName, time_ms);
    }

    return err;
}

/*
*   NOTE:
*   if several threads use sem_timedwait , this fn is  not correct to run
*   use this fn after reset the condition
*/
void OmxTryPostSem(OmxSemHandle handler)
{
    int nSemVal;
    int nRetSemGetValue;
    nRetSemGetValue=sem_getvalue(&handler->mSem, &nSemVal);
    if(0 == nRetSemGetValue)
    {
        if(0 == nSemVal)//mean one or more threads are blocked waiting to lock the smeaphore with sem_wait
        {
            sem_post(&handler->mSem);
            if(handler->bShowLog)
                logd("post sem:%s", handler->mName);
        }
        else
        {
            logv("Be careful, sem:%s, sema frame_output[%d]!=0", handler->mName, nSemVal);
        }
    }
    else
    {
        logw("fatal error, why sem getvalue of sem:%s m_sem_frame_output[%d] fail!",
             handler->mName, nRetSemGetValue);
    }
}
