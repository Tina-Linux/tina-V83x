/*
 * =====================================================================================
 *
 *       Filename:  omx_mutx.c
 *
 *    Description: the mutex class is adaptive with used or not used
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

#define LOG_TAG "omx_mutex"
#include "log.h"
#include "omx_mutex.h"
#include <malloc.h>

int OmxCreateMutex(OmxMutexHandle* pHandler, OMX_BOOL bUsed)
{
    *pHandler = (OmxMutexHandle)malloc(sizeof(**pHandler));
    if(NULL == *pHandler)
        return -1;
    if(pthread_mutex_init(&(*pHandler)->mutex, NULL))
    {
        loge("failed to create the OmxMutexHandle!");
        free(*pHandler);
        *pHandler = NULL;
        return -1;
    }
    else
    {
        (*pHandler)->bUsed = bUsed;
        return 0;
    }
}

int OmxDestroyMutex(OmxMutexHandle* pHandler)
{
    OmxMutexHandle handler = *pHandler;
    if(handler == NULL)
    {
        loge("handle is null");
        return -1;
    }

    pthread_mutex_destroy(&handler->mutex);
    free(handler);
    return 0;
}

int OmxTryAcquireMutex(OmxMutexHandle handler)
{
    int rc;
    if(handler == NULL)
    {
        loge("handle is null");
        return -1;
    }

    if(handler->bUsed)
    {
        rc = pthread_mutex_trylock(&handler->mutex);
        if(0 == rc)
            return 0;
        else
        {
            loge("OmxTryAcquireMutex failed!!");
            return -1;
        }
    }
    return 0;
}

int OmxAcquireMutex(OmxMutexHandle handler)
{
    int rc;
    if(handler == NULL)
    {
        loge("handle is null");
        return -1;
    }
    if(handler->bUsed)
    {
        rc = pthread_mutex_lock(&handler->mutex);
        if(0 == rc)
            return 0;
        loge("OmxAcquireMutex failed!!");
        return -1;
    }
    return 0;
}
int OmxReleaseMutex(OmxMutexHandle handler)
{
    int rc;
    if(handler == NULL)
    {
        loge("handle is null");
        return -1;
    }
    if(handler->bUsed)
    {
        rc = pthread_mutex_unlock(&handler->mutex);
        if(0 == rc)
            return 0;
        else
        {
            loge("OmxReleaseMutex failed!!");
            return -1;
        }
    }
    return 0;
}
