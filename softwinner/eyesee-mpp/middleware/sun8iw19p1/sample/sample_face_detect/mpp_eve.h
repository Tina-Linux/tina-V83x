#ifndef __MPP_EVE__
#define __MPP_EVE__

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
#include "aw_ai_eve_event_interface.h"

typedef struct _EVE_Params
{
    int            iFrameNum;
    
    PIXEL_FORMAT_E ePixFmt;
    int            iPicWidth;
    int            iPicHeight;
    int            iFrmRate;
    
    AW_HANDLE      pEVEHd;
    AW_IMAGE_S*    pImage;
}EVE_Params;

extern pthread_mutex_t          g_stResult_lock;
extern AW_AI_EVE_EVENT_RESULT_S g_stResult;
extern int                      g_eve_ready;

int create_eve(EVE_Params*     pEVEParams);
int destroy_eve(EVE_Params*     pEVEParams);
void ShowFaceDetectResult(int iFrameIndex, AW_AI_EVE_EVENT_RESULT_S* pstEVEResult, int bShowDetail);

#endif
