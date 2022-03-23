/*
 * =====================================================================================
 *
 *       Filename:  omx_pub_def.h
 *
 *    Description:
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:
 *
 *         Author:  Gan Qiuye(ganqiuye@allwinnertech.com)
 *        Company:  Allwinnertech.com
 *
 * =====================================================================================
 */

#ifndef __OMX_PUB_DEF_H__
#define __OMX_PUB_DEF_H__

#include "omx_macros.h"
#include "AWOMX_VideoIndexExtension.h"
#include "OMX_Video.h"
#include "OMX_VideoExt.h"
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>


#define VERSION "V2.0"
#define REPO_TAG ""
#define REPO_PATCH ""
#define REPO_BRANCH "dev-cedarc_v1.2"
#define REPO_COMMIT ""
#define REPO_DATE "Wed Aug 1  2018 +0800"
#define RELEASE_AUTHOR "MPD"

static inline void OmxVersionInfo(void)
{
    logd("\n"
         ">>>>>>>>>>>>>>>>>>>>>>>>>>>>> Openmax Info <<<<<<<<<<<<<<<<<<<<<<<<<<<<\n"
         "ver   : %s\n"
         "tag   : %s\n"
         "branch: %s\n"
         "commit: %s\n"
         "date  : %s\n"
         "author: %s\n"
         "patch : %s\n"
         "----------------------------------------------------------------------\n",
         VERSION, REPO_TAG, REPO_BRANCH, REPO_COMMIT, REPO_DATE, RELEASE_AUTHOR, REPO_PATCH);
}

static inline const char *OmxState2String(OMX_STATETYPE state)
{
    switch(state)
    {
        STRINGIFY(OMX_StateInvalid);
        STRINGIFY(OMX_StateLoaded);
        STRINGIFY(OMX_StateIdle);
        STRINGIFY(OMX_StateExecuting);
        STRINGIFY(OMX_StatePause);
        STRINGIFY(OMX_StateWaitForResources);
        STRINGIFY(OMX_StateKhronosExtensions);
        STRINGIFY(OMX_StateVendorStartUnused);
        STRINGIFY(OMX_StateMax);
        default: return "state - unknown";
    }
}

static inline int64_t OmxGetNowUs()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    return (int64_t)tv.tv_sec * 1000000ll + tv.tv_usec;
}

static inline OMX_U32 OmxAlign(unsigned int nOriginValue, int nAlign)
{
    return (nOriginValue + (nAlign-1)) & (~(nAlign-1));
}

static inline void AsserFailed(const char* expr, const char*fn, unsigned int line)
{
    loge("!!! Assert '%s' Failed at %s:%d", expr, fn, line);
    abort();
}

#define OMX_ASSERT(expr) (expr)?(void)0:AsserFailed(#expr, __FUNCTION__, __LINE__)

enum {
    kInputPortIndex  = 0x0,
    kOutputPortIndex = 0x1,
    kInvalidPortIndex = 0xFFFFFFFE,
};

/* H.263 Supported Levels & profiles */
static VIDEO_PROFILE_LEVEL_TYPE SupportedH263ProfileLevels[] = {
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level10},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level20},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level30},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level40},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level45},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level50},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level60},
  {OMX_VIDEO_H263ProfileBaseline, OMX_VIDEO_H263Level70},
  {-1, -1}};

/* MPEG4 Supported Levels & profiles */
static VIDEO_PROFILE_LEVEL_TYPE SupportedMPEG4ProfileLevels[] ={
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level0b},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level1},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level2},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level3},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level4a},
  {OMX_VIDEO_MPEG4ProfileSimple, OMX_VIDEO_MPEG4Level5},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level0},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level0b},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level1},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level2},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level3},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level4},
  {OMX_VIDEO_MPEG4ProfileAdvancedSimple, OMX_VIDEO_MPEG4Level5},
  {-1,-1}};

/* AVC Supported Levels & profiles */
static VIDEO_PROFILE_LEVEL_TYPE SupportedAVCProfileLevels[] ={
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel2},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel3},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileBaseline, OMX_VIDEO_AVCLevel51},

  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1 },
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel2 },
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel3 },
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileMain, OMX_VIDEO_AVCLevel51},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel1b},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel11},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel12},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel13},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel2 },
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel21},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel22},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel3 },
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel31},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel32},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel4},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel41},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel42},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel5},
  {OMX_VIDEO_AVCProfileHigh, OMX_VIDEO_AVCLevel51},


  {-1,-1}};

static VIDEO_PROFILE_LEVEL_TYPE SupportedHEVCProfileLevels[] = {
  { OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel1  },
  { OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel2  },
  { OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel21 },
  { OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel3  },
  { OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel31 },
  { OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel4  },
  { OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel41 },
  { OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel5  },
  { OMX_VIDEO_HEVCProfileMain, OMX_VIDEO_HEVCMainTierLevel51 },
  {-1,-1}};

static VIDDEC_CUSTOM_PARAM sVideoDecCustomParams[] =
{
    {VIDDEC_CUSTOMPARAM_ENABLE_ANDROID_NATIVE_BUFFER,
     (OMX_INDEXTYPE)AWOMX_IndexParamVideoEnableAndroidNativeBuffers},
    {VIDDEC_CUSTOMPARAM_GET_ANDROID_NATIVE_BUFFER_USAGE,
     (OMX_INDEXTYPE)AWOMX_IndexParamVideoGetAndroidNativeBufferUsage},
    {VIDDEC_CUSTOMPARAM_USE_ANDROID_NATIVE_BUFFER2,
     (OMX_INDEXTYPE)AWOMX_IndexParamVideoUseAndroidNativeBuffer2},
    {VIDDEC_CUSTOMPARAM_STORE_META_DATA_IN_BUFFER,
     (OMX_INDEXTYPE)AWOMX_IndexParamVideoUseStoreMetaDataInBuffer},
    {VIDDEC_CUSTOMPARAM_PREPARE_FOR_ADAPTIVE_PLAYBACK,
     (OMX_INDEXTYPE)AWOMX_IndexParamVideoUsePrepareForAdaptivePlayback},
    {VIDDEC_CUSTOMPARAM_GET_AFBC_HDR_FLAG,
     (OMX_INDEXTYPE)AWOMX_IndexParamVideoGetAfbcHdrFlag},
    {VIDDEC_CUSTOMPARAM_ALLOCATE_NATIVE_HANDLE,
     (OMX_INDEXTYPE)AWOMX_IndexParamVideoAllocateNativeHandle},
    {VIDDEC_CUSTOMPARAM_DESCRIBE_COLORASPECTS,
     (OMX_INDEXTYPE)AWOMX_IndexParamVideoDescribeColorAspects},
    {VIDDEC_CUSTOMPARAM_DESCRIBE_HDR_STATIC_INFO,
     (OMX_INDEXTYPE)AWOMX_IndexParamVideoDescribeHDRStaticInfo}
};


#endif
