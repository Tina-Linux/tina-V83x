#ifndef __OMX_SEM_H__
#define __OMX_SEM_H__
#include <semaphore.h>
#include "OMX_Types.h"
#ifdef __cplusplus
extern "C" {
#endif

struct OmxSem_obj
{
    sem_t     mSem;
    char      *mName;
    OMX_BOOL  bShowLog;
};

typedef struct OmxSem_obj *OmxSemHandle;
OmxSemHandle OmxCreateSem(const char* name, int pShared, unsigned int value,
                                 OMX_BOOL bShowlogFlag);
int  OmxDestroySem(OmxSemHandle* pHandler);
int  OmxTimedWaitSem(OmxSemHandle handler, OMX_S64 time_ms);
void OmxTryPostSem(OmxSemHandle handler);

#ifdef __cplusplus
}
#endif

#endif
