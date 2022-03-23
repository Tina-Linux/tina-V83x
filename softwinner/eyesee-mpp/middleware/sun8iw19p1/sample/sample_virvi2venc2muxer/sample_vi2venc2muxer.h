#ifndef __SAMPLE_VI2VENC2MUXER_H__
#define __SAMPLE_VI2VENC2MUXER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <cdx_list.h>

#include <pthread.h>

#include "mm_comm_sys.h"
#include "mpi_sys.h"

#include "mm_comm_vi.h"
#include "mpi_vi.h"
#include <mpi_isp.h>

#include "vencoder.h"
#include "mpi_venc.h"
#include "mm_comm_video.h"

#include "mm_comm_mux.h"
#include "mpi_mux.h"

#include "tmessage.h"
#include "tsemaphore.h"

#include <memoryAdapter.h>
#include "sc_interface.h"

#include <confparser.h>


#define MAX_FILE_PATH_LEN  (128)

typedef struct output_sink_info_s
{
    int mMuxerId;
    MEDIA_FILE_FORMAT_E mOutputFormat;
    int mOutputFd;
    int mFallocateLen;
    BOOL mCallbackOutFlag;
}OUTSINKINFO_S, *PTR_OUTSINKINFO_S;

typedef struct mux_chn_info_s
{
    OUTSINKINFO_S mSinkInfo;
    MUX_CHN_ATTR_S mMuxChnAttr;
    MUX_CHN mMuxChn;
    struct list_head mList;
}MUX_CHN_INFO_S, *PTR_MUX_CHN_INFO_S;

typedef struct venc_in_frame_s
{
    VIDEO_FRAME_INFO_S  mFrame;
    struct list_head mList;
}VENC_IN_FRAME_S, *PTR_VENC_IN_FRAME_S;


typedef enum RecordState
{
    REC_NOT_PREPARED = 0,
    REC_PREPARED,
    REC_RECORDING,
    REC_STOP,
    REC_ERROR,
}RECSTATE_E;

typedef struct Vi2Venc2Muxer_CmdLineParam
{
    char mConfigFilePath[MAX_FILE_PATH_LEN];
}VI2VENC2MUXER_CMDLINEPARAM_S;

typedef struct Vi2Venc2Muxer_Config
{
    char dstVideoFile[MAX_FILE_PATH_LEN];
    int mDstFileMaxCnt;

    int srcSize;
    int dstSize;
    int srcWidth;
    int srcHeight;
    int srcPixFmt;
    //int dstPixFmt;
    int dstWidth;
    int dstHeight;

    int mDevNo;

    int mField;
    int mVideoEncoderFmt;
    int mVideoFrameRate;
    int mVideoBitRate;
    int mMaxFileDuration;
    int mTestDuration;

    int mRcMode;
    int mGopMode;
    int mAdvancedRef_Base;
    int mAdvancedRef_Enhance;
    int mAdvancedRef_RefBaseEn;
    int mEnableFastEnc;
    BOOL mbEnableSmart;
    int mSVCLayer;  //0, 2, 3, 4
    int mEncodeRotate;  //clockwise.

    BOOL mColor2Grey;
    int m3DNR;
    int mRoiNum;
    BOOL mbRoiBgFrameRate;
    int mIntraRefreshBlockNum;
    int mOrlNum;

    int mVbvBufferSize;  //unit:Byte
    int mVbvThreshSize;  //unit:Byte
    
    
}VI2VENC2MUXER_CONFIG_S;

typedef struct 
{
    char strFilePath[MAX_FILE_PATH_LEN];
    struct list_head mList;
}FilePathNode;

typedef struct sample_vi2venc2muxer_s
{
    VI2VENC2MUXER_CONFIG_S mConfigPara;
    VI2VENC2MUXER_CMDLINEPARAM_S mCmdLinePara;

    char mDstDir[MAX_FILE_PATH_LEN];    //tail don't contain '/', e.g.,/mnt/extsd/sample_virvi2venc2muxer_Files
    char mFirstFileName[MAX_FILE_PATH_LEN];

    cdx_sem_t mSemExit;

    MPP_SYS_CONF_S mSysConf;

    RECSTATE_E mCurrentState;

    VI_ATTR_S mViAttr;
    ISP_DEV mIspDev;
    VI_DEV mViDev;
    VI_CHN mViChn;

    MUX_GRP_ATTR_S mMuxGrpAttr;
    MUX_GRP mMuxGrp;

    VENC_CHN_ATTR_S mVencChnAttr;
    VENC_CHN mVeChn;

    int mMuxId[2];
    struct list_head mMuxerFileListArray[2];    //FilePathNode
    int mMuxerIdCounter;


    pthread_mutex_t mMuxChnListLock;
    struct list_head mMuxChnList;   //MUX_CHN_INFO_S

}SAMPLE_VI2VENC2MUXER_S;




#endif //#define __SAMPLE_VI2VENC2MUXER_H__
