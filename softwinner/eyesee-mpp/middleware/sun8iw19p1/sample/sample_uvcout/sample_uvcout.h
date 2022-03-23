#ifndef _SAMPLE_UVC_H_
#define _SAMPLE_UVC_H_

#include <pthread.h>
#include "./include/video.h"

#include <tsemaphore.h>
#include <plat_type.h>
#include <mm_comm_vo.h>
#include <mm_common.h>
#include <mm_comm_video.h>
#include <mm_comm_venc.h>

#define SUPPORT_EVE (0)

#if (SUPPORT_EVE!=0)
#include "aw_ai_eve_type.h"
#include "aw_ai_eve_event_interface.h"
#endif

#define MAX_FILE_PATH_SIZE (256)

typedef struct SampleUVCFormat {
    unsigned int iWidth;
    unsigned int iHeight;
    unsigned int iFormat;
    unsigned int iInterval; // units:100ns
} SampleUVCFormat;

typedef struct SampleUVCFrame {
    void *pVirAddr;
    void *pPhyAddr;
    unsigned int iBufLen;
} SampleUVCFrame;

typedef struct SampleUVCDevice {
    int iDev; // must be set before open video device
    
    int iFd;

    int bIsStreaming;

    int iCtrlSetCur;
    struct uvc_streaming_control stProbe;
    struct uvc_streaming_control stCommit;

    unsigned int iWidth;
    unsigned int iHeight;
    unsigned int iFormat;
    unsigned int iInterval; // units:100ns

    SampleUVCFrame *pstFrames;
    int iBufsNum;

    void *pPrivite;
} SampleUVCDevice;

typedef struct SampleUVCCmdLineParam
{
    char mConfigFilePath[MAX_FILE_PATH_SIZE];

} SampleUVCCmdLineParam;

typedef struct SampleVencBuf {
    unsigned char *pcData;
    unsigned int iDataSize0;
    unsigned int iDataSize1;
    struct list_head mList;
} SampleVencBuf;

typedef struct SampleUVCInfo {
    pthread_t tEncTrd;
    pthread_t tFaceTrd;

    int iVippDev;
    int iVippChn;
    int iVencChn;
    int iIspDev;

    struct list_head mIdleFrm;
    struct list_head mValidFrm;
    struct list_head mUsedFrm;
    pthread_mutex_t mFrmLock;

#if (SUPPORT_EVE!=0)
    /* for eve face */
    int iFaceVippDev;
    int iFaceVippChn;
    int iFaceIspDev;
    AW_HANDLE pEveHd;
    AW_AI_EVE_EVENT_RESULT_S stEveRes;
    pthread_mutex_t mEveFaceResLock;
#endif
} SampleUVCInfo;

typedef struct SampleUVCContext
{
    int bUseEve;

    int iUVCDev;
    int iCapDev;
    int iCapWidth;
    int iCapHeight;
    int iCapFrameRate;
    PIXEL_FORMAT_E eCapFormat; //MM_PIXEL_FORMAT_YUV_PLANAR_420

    PAYLOAD_TYPE_E eEncoderType;
    int iEncWidth;
    int iEncHeight;
    int iEncBitRate;
    int iEncFrameRate;
    int iEncQuality;

    SampleUVCCmdLineParam mCmdLinePara;

    SampleUVCInfo mUVCInfo;
    SampleUVCDevice stUVCDev;
} SampleUVCContext;

int initSampleUVCContext();
int destroySampleUVCContext();

#endif  /* _SAMPLE_UVC_H_ */

