
#ifndef _SAMPLE_VIRVI_H_
#define _SAMPLE_VIRVI_H_

#include <plat_type.h>
#include <tsemaphore.h>

#define MAX_FILE_PATH_SIZE  (256)
typedef struct awVirVi_PrivCap_S {
    pthread_t thid;
    VI_DEV Dev;
    VI_CHN Chn;
    AW_S32 s32MilliSec;
    VIDEO_FRAME_INFO_S pstFrameInfo;
} VirVi_Cap_S;

typedef struct SampleVirViCmdLineParam
{
    char mConfigFilePath[MAX_FILE_PATH_SIZE];
}SampleVirViCmdLineParam;

typedef struct SampleVirViConfig
{
    int AutoTestCount;
    int GetFrameCount;
    int DevNum;
    int PicWidth;
    int PicHeight;
    int FrameRate;
    __u32 PicFormat; //MM_PIXEL_FORMAT_YUV_PLANAR_420
}SampleVirViConfig;

typedef struct SampleVirViConfparser
{
    SampleVirViCmdLineParam mCmdLinePara;
    SampleVirViConfig mConfigPara;
}SampleVirViConfparser;

#endif  /* _SAMPLE_VIRVI_H_ */

