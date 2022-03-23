#ifndef __MPP_VENC__
#define __MPP_VENC__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>

#include <utils/plat_log.h>
#include "media/mpi_sys.h"
#include "vencoder.h"
#include "mm_comm_sys.h"
#include "mm_comm_video.h"
#include "mpi_venc.h"
#include "mm_common.h"

typedef struct _VENC_Params
{
    int iFrameNum;
    char szOutputFile[256];

    int srcWidth;
    int srcHeight;
    int srcSize;
    int srcPixFmt;

    int dstWidth;
    int dstHeight;
    int dstSize;
    int dstPixFmt;
    
    PAYLOAD_TYPE_E mVideoEncoderFmt;
    int mEncUseProfile;
    int mField;
    int mVideoMaxKeyItl;
    int mVideoBitRate;
    int mVideoFrameRate;
    int maxKeyFrame;
    int mTimeLapseEnable;
    int mTimeBetweenFrameCapture;

    int mRcMode;
    ROTATE_E rotate;
}VENC_Params;

int create_venc(VENC_Params* pVENCParams, VENC_CHN chn, int type);
int destroy_venc(VENC_CHN chn);
int save_jpeg_venc(VENC_CHN mVeChn, VENC_CROP_CFG_S* pCrop_cfg, VIDEO_FRAME_INFO_S *pVideoFrame, char* zsPicPath);

#endif
