#ifndef __MPP_VO__
#define __MPP_VO__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>
#include <time.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <tsemaphore.h>
#include <mpi_sys.h>

#include <utils/plat_log.h>
#include "mm_comm_vo.h"
#include "vo/hwdisplay.h"
#include "mpi_vo.h"
#include "mm_comm_sys.h"
#include "mm_comm_video.h"
#include "mm_common.h"
#include <cdx_list.h>

typedef struct _VO_Frame_Manager
{
    struct list_head mIdleList;
    struct list_head mUsingList;
    int mNodeCnt;
    pthread_mutex_t mLock;
    VIDEO_FRAME_INFO_S* (*PrefetchFirstIdleFrame)(void *pThiz);
    int (*UseFrame)(void *pThiz, VIDEO_FRAME_INFO_S *pFrame);
    int (*ReleaseFrame)(void *pThiz, unsigned int nFrameId);
}VO_Frame_Manager;

typedef struct _VO_Frame_Node
{
    VIDEO_FRAME_INFO_S mFrame;
    struct list_head mList;
}VO_Frame_Node;

typedef struct _VO_Config
{
    int mPicWidth;
    int mPicHeight;
    PIXEL_FORMAT_E mPicFormat; //MM_PIXEL_FORMAT_YUV_PLANAR_420
    int mFrameRate;
}VO_Config;

typedef struct _VO_Params
{
    VO_DEV      iVoDev;
    VO_CHN      iVoChn;
    VO_LAYER    iVoLayer;
    int         iMiniGUILayer;

    int         iDispType;
    int         iDispSync;

    int         iWidth;
    int         iHeight;
    int         iFrameNum;

    VO_Frame_Manager mFrameManager;
    VO_Config        mConfigPara;
}VO_Params;

int  create_vo(VO_Params* pVOParams);
int  destroy_vo(VO_Params* pVOParams);
void open_g2d_device();
void close_g2d_device();
void rotate_frame(VIDEO_FRAME_INFO_S* pSrcFrameInfo, VIDEO_FRAME_INFO_S* pDstFrameInfo);
int  initVOFrameManager(VO_Frame_Manager *pFrameManager, int nFrameNum, VO_Config *pConfigPara);
int  destroyVOFrameManager(VO_Frame_Manager *pFrameManager);

#endif
