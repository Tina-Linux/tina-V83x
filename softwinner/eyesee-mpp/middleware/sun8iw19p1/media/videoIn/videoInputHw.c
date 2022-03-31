/******************************************************************************
  Copyright (C), 2001-2016, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     : videoInputHw.c
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2016/06/28
  Last Modified :
  Description   : mpi functions implement
  Function List :
  History       :
******************************************************************************/

//#define LOG_NDEBUG 0
#define LOG_TAG "videoInputHw"
#include <utils/plat_log.h>

#include <errno.h>
#include <memory.h>
#include <sched.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <linux/unistd.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <sys/syscall.h>

#include "isp_dev.h"
#include "isp.h"
#include "mm_component.h"

#include <VideoVirViCompPortIndex.h>
// #include "../include/videoIn/videoInputHw.h"
#include "videoInputHw.h"
#include <mpi_videoformat_conversion.h>
#include <ChannelRegionInfo.h>
#include <BITMAP_S.h>
#include "VIPPDrawOSD_V5.h"
#include "ConfigOption.h"

#include <cdx_list.h>
#include <ConfigOption.h>

// viChnManager *gpViChnManager = NULL;
//viChnManager *gpVippManager[VI_VIPP_NUM_MAX] = {NULL}; // { NULL, NULL, NULL, NULL };
//struct hw_isp_media_dev *media;
// struct isp_video_device *video_node[HW_VIDEO_DEVICE_NUM] = {NULL, NULL};
#define ICE_THREAD_UP 0
#if ICE_THREAD_UP
#define gettid() syscall(__NR_gettid)
#define SCHED_DEADLINE	6

struct sched_attr
{
	__u32 size;
	__u32 sched_policy;
	__u64 sched_flags;
	/* SCHED_NORMAL, SCHED_BATCH */
	__s32 sched_nice;
	/* SCHED_FIFO, SCHED_RR */
	__u32 sched_priority;
	/* SCHED_DEADLINE (nsec) */
	__u64 sched_runtime;
	__u64 sched_deadline;
	__u64 sched_period;
};
int sched_setattr(pid_t pid, const struct sched_attr *attr, unsigned int flags)
{
	return syscall(__NR_sched_setattr, pid, attr, flags);
}
int sched_getattr(pid_t pid, struct sched_attr *attr,
		unsigned int size, unsigned int flags)
{
	return syscall(__NR_sched_getattr, pid, attr, size, flags);
}

#endif

VIDevManager *gpVIDevManager;

void *VideoInputHw_CapThread(void *pThreadData);

ERRORTYPE videoInputHw_Construct(int vipp_id)
{
    int i, ret;

    pthread_mutex_lock(&gpVIDevManager->mManagerLock);

    if (gpVIDevManager->gpVippManager[vipp_id] != NULL) {
        pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
        return SUCCESS;
    }
    gpVIDevManager->gpVippManager[vipp_id] = (viChnManager *)malloc(sizeof(viChnManager));
    if (NULL == gpVIDevManager->gpVippManager[vipp_id]) {
        aloge("alloc viChnManager error(%s)!", strerror(errno));
        pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
        return FAILURE;
    }
    memset(gpVIDevManager->gpVippManager[vipp_id], 0, sizeof(viChnManager));

    ret = pthread_mutex_init(&gpVIDevManager->gpVippManager[vipp_id]->mLock, NULL);
    if (ret != 0) {
        aloge("fatal error! mutex init fail");
        free(gpVIDevManager->gpVippManager[vipp_id]);
        gpVIDevManager->gpVippManager[vipp_id] = NULL;
        pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
        return FAILURE;
    }
    pthread_mutex_init(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock, NULL);
    pthread_mutex_init(&gpVIDevManager->gpVippManager[vipp_id]->mFrameListLock, NULL);
    pthread_mutex_init(&gpVIDevManager->gpVippManager[vipp_id]->mRegionLock, NULL);
    pthread_mutex_init(&gpVIDevManager->gpVippManager[vipp_id]->mLongExpLock, NULL);
    INIT_LIST_HEAD(&gpVIDevManager->gpVippManager[vipp_id]->mChnList);
    INIT_LIST_HEAD(&gpVIDevManager->gpVippManager[vipp_id]->mOverlayList);
    INIT_LIST_HEAD(&gpVIDevManager->gpVippManager[vipp_id]->mCoverList);
    INIT_LIST_HEAD(&gpVIDevManager->gpVippManager[vipp_id]->mOrlList);
    INIT_LIST_HEAD(&gpVIDevManager->gpVippManager[vipp_id]->mIdleFrameList);
    INIT_LIST_HEAD(&gpVIDevManager->gpVippManager[vipp_id]->mReadyFrameList);
    for(i=0;i<32;i++)
    {
        VippFrame *pNode = (VippFrame*)malloc(sizeof(VippFrame));
        if(NULL == pNode)
        {
            aloge("fatal error! malloc fail!");
            break;
        }
        memset(pNode, 0, sizeof(VippFrame));
        list_add_tail(&pNode->mList, &gpVIDevManager->gpVippManager[vipp_id]->mIdleFrameList);
    }
	gpVIDevManager->gpVippManager[vipp_id]->vipp_dev_id = vipp_id;
	for (i=0; i<32; i++)
		gpVIDevManager->gpVippManager[vipp_id]->refs[i] = 0;


    pthread_mutex_unlock(&gpVIDevManager->mManagerLock);

	return SUCCESS;
}
ERRORTYPE videoInputHw_Destruct(int vipp_id)
{
    int i;

    pthread_mutex_lock(&gpVIDevManager->mManagerLock);

    if (gpVIDevManager->gpVippManager[vipp_id] != NULL) {
        if (!list_empty(&gpVIDevManager->gpVippManager[vipp_id]->mChnList)) {
            aloge("fatal error! some vi channel still running when destroy vi device!");
        }
        pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mRegionLock);
        ChannelRegionInfo *pEntry, *pTmp;
        list_for_each_entry_safe(pEntry, pTmp, &gpVIDevManager->gpVippManager[vipp_id]->mOverlayList, mList)
        {
            list_del(&pEntry->mList);
            ChannelRegionInfo_Destruct(pEntry);
        }
        list_for_each_entry_safe(pEntry, pTmp, &gpVIDevManager->gpVippManager[vipp_id]->mCoverList, mList)
        {
            list_del(&pEntry->mList);
            ChannelRegionInfo_Destruct(pEntry);
        }
        list_for_each_entry_safe(pEntry, pTmp, &gpVIDevManager->gpVippManager[vipp_id]->mOrlList, mList)
        {
            list_del(&pEntry->mList);
            ChannelRegionInfo_Destruct(pEntry);
        }
        pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mRegionLock);
        pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mFrameListLock);
        if(!list_empty(&gpVIDevManager->gpVippManager[vipp_id]->mReadyFrameList))
        {
            int cnt = 0;
            VippFrame *pEntry;
            list_for_each_entry(pEntry, &gpVIDevManager->gpVippManager[vipp_id]->mReadyFrameList, mList)
            {
                aloge("fatal error! vipp[%d] frameBufId[%d] is not released?", pEntry->mVipp, pEntry->mFrameBufId);
                cnt++;
            }            
            aloge("fatal error! There is %d frame is not release in vipp[%d]!", cnt, vipp_id);
            list_splice_tail_init(&gpVIDevManager->gpVippManager[vipp_id]->mReadyFrameList, &gpVIDevManager->gpVippManager[vipp_id]->mIdleFrameList);
        }
        VippFrame *pFrameEntry, *pFrameTmp;
        list_for_each_entry_safe(pFrameEntry, pFrameTmp, &gpVIDevManager->gpVippManager[vipp_id]->mIdleFrameList, mList)
        {
            list_del(&pFrameEntry->mList);
            free(pFrameEntry);
        }
        pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mFrameListLock);

        pthread_mutex_destroy(&gpVIDevManager->gpVippManager[vipp_id]->mLongExpLock);
        pthread_mutex_destroy(&gpVIDevManager->gpVippManager[vipp_id]->mRegionLock);
        pthread_mutex_destroy(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock);
        pthread_mutex_destroy(&gpVIDevManager->gpVippManager[vipp_id]->mFrameListLock);
        pthread_mutex_destroy(&gpVIDevManager->gpVippManager[vipp_id]->mLock);
        for (i=0; i<32; i++)
        {
            if(gpVIDevManager->gpVippManager[vipp_id]->refs[i] != 0)
            {
                aloge("fatal error! vipp[%d], idx[%d], ref[%d]!=0, check code!", vipp_id, i, gpVIDevManager->gpVippManager[vipp_id]->refs[i]);
                gpVIDevManager->gpVippManager[vipp_id]->refs[i] = 0;
            }
        }
        if(gpVIDevManager->gpVippManager[vipp_id]->mpOsdGroups)
        {
            OsdGroupsDestruct(gpVIDevManager->gpVippManager[vipp_id]->mpOsdGroups);
            gpVIDevManager->gpVippManager[vipp_id]->mpOsdGroups = NULL;
        }
        free(gpVIDevManager->gpVippManager[vipp_id]);
        gpVIDevManager->gpVippManager[vipp_id] = NULL;

    }

    // cdx_sem_init(&mVideoStabilization.sync_exit, 0);
    // cdx_sem_deinit(&mVideoStabilization.sync_exit);
    pthread_mutex_unlock(&gpVIDevManager->mManagerLock);

    return SUCCESS;
}

ERRORTYPE videoInputHw_searchExistDev(VI_DEV vipp_id, viChnManager **ppViDev)
{
    ERRORTYPE ret = FAILURE;
    pthread_mutex_lock(&gpVIDevManager->mManagerLock);
    if (gpVIDevManager->gpVippManager[vipp_id] == NULL) 
    {
        pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
        return ERR_VI_UNEXIST;
    }
    if(ppViDev!=NULL)
    {
        *ppViDev = gpVIDevManager->gpVippManager[vipp_id];
        ret = SUCCESS;
    }
    else
    {
        ret = ERR_VI_INVALID_NULL_PTR;
    }
    pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
    return ret;
}

ERRORTYPE videoInputHw_RegisterCallback(int vipp_id, void *pAppData, MPPCallbackFuncType pMppCallBack)
{
    if (gpVIDevManager->gpVippManager[vipp_id] == NULL) 
    {
        return FAILURE;
    }
    pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);
    gpVIDevManager->gpVippManager[vipp_id]->mMppCallback = pMppCallBack;
    gpVIDevManager->gpVippManager[vipp_id]->pAppData = pAppData;
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);

    return SUCCESS;
}

ERRORTYPE videoInputHw_addChannel(int vipp_id, VI_CHN_MAP_S *pChn)
{
    if (gpVIDevManager->gpVippManager[vipp_id] == NULL) {
        return FAILURE;
    }
    pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);
    list_add_tail(&pChn->mList, &gpVIDevManager->gpVippManager[vipp_id]->mChnList);
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);

    return SUCCESS;
}
ERRORTYPE videoInputHw_removeChannel(int vipp_id, VI_CHN_MAP_S *pChn)
{
    if (gpVIDevManager->gpVippManager[vipp_id] == NULL) {
        return FAILURE;
    }
    pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);
    list_del(&pChn->mList);
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);
    return SUCCESS;
}

ERRORTYPE videoInputHw_searchExistDevVirChn(VI_DEV vipp_id, VI_CHN ViChn, VI_CHN_MAP_S **ppChn)
{
    ERRORTYPE ret = FAILURE;
    VI_CHN_MAP_S *pEntry;
	int mVirviComVippChn = 0;
	mVirviComVippChn = ((vipp_id << 16) & 0xFFFF0000) | (ViChn & 0x0000FFFF);
	// printf("===== dev=%d, chn=%d, mVirviComVippChn=%x.\r\n", vipp_id, ViChn, mVirviComVippChn);

    pthread_mutex_lock(&gpVIDevManager->mManagerLock);

    if (gpVIDevManager->gpVippManager[vipp_id] == NULL) {
        pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
        return FAILURE;
    }
    pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);
    list_for_each_entry(pEntry, &gpVIDevManager->gpVippManager[vipp_id]->mChnList, mList)
    {
    	// printf("-------%x,,,%x.\r\n", pEntry->mViChn , mVirviComVippChn);
        if (pEntry->mViChn == mVirviComVippChn) {
            if (ppChn) {
                *ppChn = pEntry;
            }
            ret = SUCCESS;
            break;
        }
    }
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);

    pthread_mutex_unlock(&gpVIDevManager->mManagerLock);

    return ret;
}

/*ERRORTYPE videoInputHw_initVipp(VI_DEV Vipp_id)
{
    if (gpVIDevManager->gpVippManager[Vipp_id] == NULL) {
        return FAILURE;
    }
    pthread_mutex_lock(&gpVIDevManager->gpVippManager[Vipp_id]->mLock);
    gpVIDevManager->gpVippManager[Vipp_id]->vipp_enable = -1;
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[Vipp_id]->mLock);
    return SUCCESS;
}*/
ERRORTYPE videoInputHw_setVippEnable(VI_DEV Vipp_id)
{
    if (gpVIDevManager->gpVippManager[Vipp_id] == NULL) {
        return FAILURE;
    }
    pthread_mutex_lock(&gpVIDevManager->gpVippManager[Vipp_id]->mLock);
    gpVIDevManager->gpVippManager[Vipp_id]->vipp_enable = 1;
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[Vipp_id]->mLock);
    return SUCCESS;
}
ERRORTYPE videoInputHw_setVippDisable(VI_DEV Vipp_id)
{
    if (gpVIDevManager->gpVippManager[Vipp_id] == NULL) {
        return FAILURE;
    }
    pthread_mutex_lock(&gpVIDevManager->gpVippManager[Vipp_id]->mLock);
    gpVIDevManager->gpVippManager[Vipp_id]->vipp_enable = 0;
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[Vipp_id]->mLock);
    return SUCCESS;
}

int videoInputHw_IsLongShutterBusy(VI_DEV Vipp_id)
{
    int bIsBusy = 0;

    if (gpVIDevManager->gpVippManager[Vipp_id] == NULL) {
        return FAILURE;
    }

    pthread_mutex_lock(&gpVIDevManager->gpVippManager[Vipp_id]->mLongExpLock);
    bIsBusy = gpVIDevManager->gpVippManager[Vipp_id]->bTakeLongExpPic;
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[Vipp_id]->mLongExpLock);

    return bIsBusy;
}

ERRORTYPE videoInputHw_IncreaseLongShutterRef(VI_DEV Vipp_id)
{
    int bIsBusy = 0;

    if (gpVIDevManager->gpVippManager[Vipp_id] == NULL) {
        aloge("No such video device %d", Vipp_id);
        goto failed;
    }

    pthread_mutex_lock(&gpVIDevManager->gpVippManager[Vipp_id]->mLongExpLock);
    if (gpVIDevManager->gpVippManager[Vipp_id]->iTakeLongExpRef < VI_VIRCHN_NUM_MAX)
        gpVIDevManager->gpVippManager[Vipp_id]->iTakeLongExpRef++;
    else
        aloge("The reference has been got upper limit %d, vipp id %d", VI_VIPP_NUM_MAX, Vipp_id);
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[Vipp_id]->mLongExpLock);
    return gpVIDevManager->gpVippManager[Vipp_id]->iTakeLongExpRef;
failed:
    return FAILURE;
}

ERRORTYPE videoInputHw_DecreaseLongShutterRef(VI_DEV Vipp_id)
{
    int bIsBusy = 0;

    if (gpVIDevManager->gpVippManager[Vipp_id] == NULL) {
        aloge("No such video device %d", Vipp_id);
        goto failed;
    }

    pthread_mutex_lock(&gpVIDevManager->gpVippManager[Vipp_id]->mLongExpLock);
    if (gpVIDevManager->gpVippManager[Vipp_id]->iTakeLongExpRef > 0)
        gpVIDevManager->gpVippManager[Vipp_id]->iTakeLongExpRef--;
    else
        aloge("The reference has been got lowwer limit 0, vipp id %d", Vipp_id);
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[Vipp_id]->mLongExpLock);
    return gpVIDevManager->gpVippManager[Vipp_id]->iTakeLongExpRef;
failed:
    return FAILURE;
}

/* Long exposure is beasd on signal abstract vipp device. */
ERRORTYPE videoInputHw_SetVippShutterTime(VI_DEV Vipp_id, VI_SHUTTIME_CFG_S *pShutTime)
{
    struct isp_video_device *video = NULL;
    int iIspId;
    int time = pShutTime->iTime;

    if ((0 == time) && (VI_SHUTTIME_MODE_AUTO != pShutTime->eShutterMode)) {
        aloge("Wrong shutter time value[%d]", time);
        goto failed;
    }

    if (gpVIDevManager->gpVippManager[Vipp_id] == NULL ||
            gpVIDevManager->media->video_dev[Vipp_id] == NULL) {
        aloge("No such video device %d", Vipp_id);
        goto failed;
    }
    video = gpVIDevManager->media->video_dev[Vipp_id];
    iIspId = video_to_isp_id(video);

    int iExpNewTimeUs = 0;
    int iCurFps = 30;
    struct sensor_config stConfig;

    int iSensorGainVal = 40;
    int iSensorExpVal  = 30000;

    pthread_mutex_lock(&gpVIDevManager->gpVippManager[Vipp_id]->mLongExpLock);
    memset(&stConfig, 0, sizeof(struct sensor_config));
    if (isp_get_sensor_info(iIspId, &stConfig) < 0) {
        aloge("Get isp sensor information failed, isp id %d", iIspId);
        goto failed;
    }
    iCurFps = stConfig.fps_fixed;

    switch(pShutTime->eShutterMode) {
        case VI_SHUTTIME_MODE_AUTO: { /* auto shutter mode */
            video_set_control(video, V4L2_CID_EXPOSURE_AUTO, 0); /* auto exp */
            video_set_control(video, V4L2_CID_AUTOGAIN, 1);      /* auto gain */

            video_set_vin_reset_time(video, 0);
            if (isp_set_fps(iIspId, iCurFps) < 0) {
                aloge("Set sensor fps %d failed, isp id %d", iCurFps, iIspId);
                goto failed;
            }
            gpVIDevManager->gpVippManager[Vipp_id]->bTakeLongExpPic = 0;
        } break;

        case VI_SHUTTIME_MODE_PREVIEW: { /* preview shutter mode */
            /* get realtime gain exp value */
            video_get_control(video, V4L2_CID_GAIN, &iSensorGainVal);
            video_get_control(video, V4L2_CID_EXPOSURE_ABSOLUTE, &iSensorExpVal);

            if (iCurFps <= time) {
                video_set_vin_reset_time(video, 0);

                video_set_control(video, V4L2_CID_EXPOSURE_AUTO, 1); // 1:manual exp 2:shutter prio
                video_set_control(video, V4L2_CID_AUTOGAIN, 0); //manual gain

                iExpNewTimeUs = 1000000 / time;
                iSensorGainVal = iSensorGainVal * iSensorExpVal / iExpNewTimeUs;
                iSensorGainVal = (iSensorGainVal < 16) ? 16 : iSensorGainVal;
                iSensorGainVal = iSensorGainVal * 3; /* use 3 times of calc vaule */
                if (video_set_control(video, V4L2_CID_EXPOSURE_ABSOLUTE, iExpNewTimeUs) < 0 ||
                    video_set_control(video, V4L2_CID_GAIN, iSensorGainVal) < 0)
                {
                    aloge("Set gain %d, exposure %d failed.", iSensorGainVal, iExpNewTimeUs);
                    goto failed;
                }
            } else {
                aloge("wrong time value[%d] with <preview shutter mode>", time);
            }
        } break;
        case VI_SHUTTIME_MODE_NIGHT_VIEW: { /* night view mode */
            /* get realtime gain exp value */
            video_get_control(video, V4L2_CID_GAIN, &iSensorGainVal);
            video_get_control(video, V4L2_CID_EXPOSURE_ABSOLUTE, &iSensorExpVal);

            if (iCurFps > time) {
                video_set_control(video, V4L2_CID_EXPOSURE_AUTO, 1); // 1:manual exp 2:shutter prio
                video_set_control(video, V4L2_CID_AUTOGAIN, 0); //manual gain

                /* (time = 0) has been excluded */
                if (time > 0) {
                    iExpNewTimeUs = 1000000 / time;
                    video_set_vin_reset_time(video, 0);
                } else if (time < 0) {
                    iExpNewTimeUs = 1000000 * (0-time);
                    video_set_vin_reset_time(video, (0-time));
                }

                iSensorGainVal = iSensorGainVal * iSensorExpVal / iExpNewTimeUs;
                iSensorGainVal = (iSensorGainVal < 16) ? 16 : iSensorGainVal;
                iSensorGainVal = iSensorGainVal * 3; /* use 3 times of calc vaule */
                /* we do not care it is success or not */
                video_set_control(video, V4L2_CID_EXPOSURE_ABSOLUTE, iExpNewTimeUs);
                video_set_control(video, V4L2_CID_GAIN, iSensorGainVal);

                if (isp_set_fps(iIspId, time) < 0) {
                    goto failed;
                }
                gpVIDevManager->gpVippManager[Vipp_id]->bTakeLongExpPic = 1;
            } else {
                aloge("wrong time value[%d] with <night view mode>", time);
            }
        } break;

        default: {
            aloge("wrong shutter mode[%d], use[0~2]", pShutTime->eShutterMode);
            goto failed;
        } break;
    }

    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[Vipp_id]->mLongExpLock);
    return SUCCESS;

failed:
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[Vipp_id]->mLongExpLock);
    return FAILURE;
}

ERRORTYPE videoInputHw_searchVippStatus(VI_DEV Vipp_id, int *pStatus)
{
	int ret = -1;
    pthread_mutex_lock(&gpVIDevManager->mManagerLock);

    if (gpVIDevManager->gpVippManager[Vipp_id] == NULL) {
        pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
        return FAILURE;
    }
	pthread_mutex_lock(&gpVIDevManager->gpVippManager[Vipp_id]->mLock);
    ret = gpVIDevManager->gpVippManager[Vipp_id]->vipp_enable;
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[Vipp_id]->mLock);
	*pStatus = ret;

    pthread_mutex_unlock(&gpVIDevManager->mManagerLock);

    return ret;
}

MM_COMPONENTTYPE *videoInputHw_GetChnComp(VI_DEV ViDev, VI_CHN ViChn)
{
    VI_CHN_MAP_S *pChn;
    if (videoInputHw_searchExistDevVirChn(ViDev, ViChn, &pChn) != SUCCESS) {
        return NULL;
    }
    return pChn->mViComp;
}

VI_CHN_MAP_S *videoInputHw_CHN_MAP_S_Construct()
{
    VI_CHN_MAP_S *pChannel = (VI_CHN_MAP_S *)malloc(sizeof(VI_CHN_MAP_S));
    if (NULL == pChannel) {
        aloge("fatal error! malloc fail[%s]!", strerror(errno));
        return NULL;
    }
    memset(pChannel, 0, sizeof(VI_CHN_MAP_S));
    cdx_sem_init(&pChannel->mSemCompCmd, 0);
    return pChannel;
}
void videoInputHw_CHN_MAP_S_Destruct(VI_CHN_MAP_S *pChannel)
{
    if (pChannel->mViComp) {
        aloge("fatal error! Vi component need free before!");
        COMP_FreeHandle(pChannel->mViComp);
        pChannel->mViComp = NULL;
    }
    cdx_sem_deinit(&pChannel->mSemCompCmd);
    free(pChannel);
    //pChannel = NULL;
}
ERRORTYPE videoInputHw_Open_Media() /*Open Media+ISP+CSI Device*/
{
    if (gpVIDevManager)
    {
        if(gpVIDevManager->media)
        {
            alogd("videoInputHw already open.");
            return SUCCESS;
        }
        else
        {
            aloge("fatal error! media is not construct");
        }
    }

    gpVIDevManager = (VIDevManager *)malloc(sizeof(VIDevManager));
    if(gpVIDevManager == NULL)
    {
        aloge("error, gpVIDevManager can not be allocted");
        return FAILURE;
    }
    int index;
    pthread_mutex_init(&gpVIDevManager->mManagerLock, NULL);
    for(index = 0; index < VI_VIPP_NUM_MAX; ++index)
    {
        gpVIDevManager->gpVippManager[index] = NULL;
    }

    gpVIDevManager->media = isp_md_open(MEDIA_DEVICE);
    if (gpVIDevManager->media == NULL) {
        alogd("error: unable to open media device %s\n", MEDIA_DEVICE);
        return FAILURE;
    }

    gpVIDevManager->mSetFrequency = TRUE; //
    gpVIDevManager->mClockFrequency = -1;//deauflt

    return SUCCESS;
}
ERRORTYPE videoInputHw_Close_Media() /*Close Media+ISP+CSI Device*/
{
    if (gpVIDevManager->media) { /* Cleanup the ISP resources. */
        isp_md_close(gpVIDevManager->media);
    }
    gpVIDevManager->media = NULL;
    gpVIDevManager->mSetFrequency = TRUE;
    gpVIDevManager->mClockFrequency = -1;
    pthread_mutex_destroy(&gpVIDevManager->mManagerLock);
    free(gpVIDevManager);
    gpVIDevManager = NULL;

    return SUCCESS;
}
ERRORTYPE videoInputHw_ChnInit(int ViCh) /*Open /dev/video[0~3] node*/
{
    //return (ERRORTYPE)isp_video_open(gpVIDevManager->media, ViCh);
    //ERRORTYPE ret = -1;
    pthread_mutex_lock(&gpVIDevManager->mManagerLock);

    if(isp_video_open(gpVIDevManager->media, ViCh) < 0)
    {
        aloge("error: isp video can not open, chn[%d]!", ViCh);
        pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
        return FAILURE;
    }

    if(!gpVIDevManager->mSetFrequency)
    {
        struct isp_video_device *video = gpVIDevManager->media->video_dev[ViCh];
        if (video_set_top_clk(video, gpVIDevManager->mClockFrequency) < 0)
        {
            aloge("Cuation:can not set ISP clock frequency!");
            pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
            return FAILURE;
        }
        alogw("Attention: the ISP clock frequecy had been set %f MHZ", gpVIDevManager->mClockFrequency / 1000000.0);
        gpVIDevManager->mSetFrequency = TRUE;
    }

    pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
    return SUCCESS;

}
ERRORTYPE videoInputHw_ChnExit(int ViCh) /*Close /dev/video[0~3] node*/
{
    pthread_mutex_lock(&gpVIDevManager->mManagerLock);

    if(gpVIDevManager->media->video_dev[ViCh])
    {
        int ret = overlay_update(gpVIDevManager->media->video_dev[ViCh], 0);
        if(ret != 0)
        {
            aloge("fatal error! the vipp[%d] OSD can not closed!", ViCh);
        }
    }

    isp_video_close(gpVIDevManager->media, ViCh);

    pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
    return 0;
}

ERRORTYPE videoInputHw_SetChnAttr(VI_DEV ViCh, VI_ATTR_S *pstAttr) /*Set /dev/video[0~3] node attr*/
{
    struct isp_video_device *video = NULL;
    struct video_fmt vfmt;
    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViCh]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViCh];
    }
    memset(&vfmt, 0, sizeof(vfmt));

    vfmt.type = pstAttr->type;
    vfmt.memtype = pstAttr->memtype;
    vfmt.format = pstAttr->format;
    vfmt.nbufs = pstAttr->nbufs;
    vfmt.nplanes = pstAttr->nplanes;
    vfmt.fps = pstAttr->fps;
    vfmt.capturemode = pstAttr->capturemode; // V4L2_MODE_VIDEO
    vfmt.use_current_win = pstAttr->use_current_win;
    vfmt.drop_frame_num  = (pstAttr->drop_frame_num >= 0) ? pstAttr->drop_frame_num : 0;
    if ((0 == pstAttr->wdr_mode) ||
        (1 == pstAttr->wdr_mode) ||
        (2 == pstAttr->wdr_mode)) {
        vfmt.wdr_mode = pstAttr->wdr_mode; /*0:normal; 1:DOL; 2:sensor commanding*/
    } else {
        vfmt.wdr_mode = 0; /* defaule value : 0 */
    }

    if (video_set_fmt(video, &vfmt) < 0) {
        aloge("video_set_fmt failed, chn[%d]", ViCh);
        return FAILURE;
    }

    return SUCCESS;
}

int videoInputHw_SetVIFreq(VI_DEV ViCh, int freq)
{
    struct isp_video_device *video = NULL;
    struct video_fmt vfmt;
   //if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
   //    ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
   //     return ERR_VI_INVALID_CHNID;
   // } else {
    //    video = media->video_dev[ViCh];
   // }
   pthread_mutex_lock(&gpVIDevManager->mManagerLock);

    if(gpVIDevManager->mClockFrequency != freq)
    {
        int video_dev_index;
        for(video_dev_index = 0; video_dev_index < HW_VIDEO_DEVICE_NUM; ++video_dev_index )
        {
            if(gpVIDevManager->media->video_dev[video_dev_index])
            {
                video = gpVIDevManager->media->video_dev[video_dev_index];
                if (video_set_top_clk(video, freq) < 0)
                {
                    aloge("Cuation:can not set ISP clock frequency!");
                    pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
                    return FAILURE;
                }
                alogw("the isp clock freq had been set %f MHz", freq / 1000000.0);
                gpVIDevManager->mSetFrequency = TRUE;
                gpVIDevManager->mClockFrequency = freq;
                pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
                return SUCCESS;
            }
        }
        gpVIDevManager->mSetFrequency = FALSE;
        gpVIDevManager->mClockFrequency = freq;
        alogw("The Device do not open, and waitting to set the isp clock freq");
    }
    else
    {
        alogw("The isp clock frequency same as you wanted, the freq is %d MHz!!", gpVIDevManager->mClockFrequency / 1000000);
    }

    pthread_mutex_unlock(&gpVIDevManager->mManagerLock);
    return 0;//
}

ERRORTYPE videoInputHw_GetChnAttr(VI_DEV ViCh, VI_ATTR_S *pstAttr) /*Get /dev/video[0~3] node attr*/
{
    struct isp_video_device *video = NULL;
    struct video_fmt vfmt;
    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViCh]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViCh];
    }
    memset(&vfmt, 0, sizeof(vfmt));
    video_get_fmt(video, &vfmt);
    pstAttr->type = vfmt.type;
    pstAttr->memtype = vfmt.memtype;
    pstAttr->format = vfmt.format;
    pstAttr->nbufs = vfmt.nbufs;
    pstAttr->nplanes = vfmt.nplanes;
    pstAttr->fps = vfmt.fps;
    pstAttr->capturemode = vfmt.capturemode;
    pstAttr->use_current_win = vfmt.use_current_win;
    pstAttr->wdr_mode = vfmt.wdr_mode;
    pstAttr->drop_frame_num = vfmt.drop_frame_num;

    return SUCCESS;
}

ERRORTYPE videoInputHw_ChnEnable(int ViVipp) /*Enable /dev/video[0~3] node*/
{
    struct isp_video_device *video = NULL;
    struct buffers_pool *pool = NULL;
    struct video_fmt vfmt;
    int i;
    if (ViVipp >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViVipp]) {
        aloge("VIN CH[%d] number is invalid!\n", ViVipp);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViVipp];
    }
    pool = buffers_pool_new(video);
    if (NULL == pool) {
        return FAILURE;
    }
    if (video_req_buffers(video, pool) < 0) {
        return FAILURE;
    }
    memset(&vfmt, 0, sizeof(vfmt));
    video_get_fmt(video, &vfmt);
    for (i = 0; i < vfmt.nbufs; i++) {
        video_queue_buffer(video, i);
    }

    if (video_stream_on(video) < 0) {
        return FAILURE;
    }
	viChnManager *pVipp = gpVIDevManager->gpVippManager[ViVipp];

	videoInputHw_setVippEnable(ViVipp);

	pthread_create(&pVipp->threadid, NULL, VideoInputHw_CapThread, (void *)pVipp);

    return SUCCESS;
}
ERRORTYPE videoInputHw_ChnDisable(int ViVipp) /*Disable /dev/video[0~3] node*/
{
    struct isp_video_device *video = NULL;

    if (ViVipp >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViVipp]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViVipp);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViVipp];
    }
	viChnManager *pVipp = gpVIDevManager->gpVippManager[ViVipp];
    /*must usleep for video capture[VideoInputHw_CapThread], must not delete!!!*/
    usleep(5000);
    pthread_mutex_lock(&pVipp->mLock);
    int cnt = 0;
    struct list_head *pList;
    list_for_each(pList, &pVipp->mChnList)
    {
        cnt++;
    }
    if(cnt > 0)
    {
        aloge("fatal error! there is [%d] vir channel exist, must destroy them first!", cnt);
    }
    pthread_mutex_unlock(&pVipp->mLock);
	videoInputHw_setVippDisable(ViVipp);
	pthread_join(pVipp->threadid, NULL);

    if (video_stream_off(video) < 0) {
        return FAILURE;
    }
    if (video_free_buffers(video) < 0) {
        return FAILURE;
    }
    buffers_pool_delete(video);
    return SUCCESS;
}
/*
ERRORTYPE videoInputHw_SetOsdMaskRegion(int *pvipp_id, VI_OsdMaskRegion *pstOsdMaskRegion)
{
    int ViCh = *pvipp_id;
    VI_OsdMaskRegion *pOsdMaskRegion = pstOsdMaskRegion;

    struct isp_video_device *video = NULL;
    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = media->video_dev[ViCh];
    }

    if (overlay_set_fmt(video, (struct osd_fmt *)pOsdMaskRegion) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_UpdateOsdMaskRegion(int *pvipp_id, int onoff)
{
    int ViCh = *pvipp_id;

    struct isp_video_device *video = NULL;
    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = media->video_dev[ViCh];
    }

    if (overlay_update(video, onoff) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}
*/

#define MAX_GLOBAL_ALPHA (16)
//ref to vipp_reg.h
#if(AWCHIP == AW_V5)
  #define MAX_OVERLAY_NUM 64
  #define MAX_COVER_NUM 8
#elif(AWCHIP == AW_V316)
  #define MAX_OVERLAY_NUM 8
  #define MAX_COVER_NUM 8
#elif(AWCHIP == AW_V459)
  #define MAX_OVERLAY_NUM 0
  #define MAX_COVER_NUM 0
  #define MAX_ORL_NUM 16
#else
  #define MAX_OVERLAY_NUM 64
  #define MAX_COVER_NUM 8
#endif

#define OVERLAY_INVERT_UNIT_WIDTH (16)
#define OVERLAY_INVERT_UNIT_HEIGHT (16)

ERRORTYPE videoInputHw_DrawOSD(VI_DEV vipp_id)
{
    ERRORTYPE ret = SUCCESS;
    viChnManager *pVipp = gpVIDevManager->gpVippManager[vipp_id];
    struct isp_video_device *video = gpVIDevManager->media->video_dev[vipp_id];
    //draw overlay
    if(!list_empty(&pVipp->mOverlayList))
    {
        struct osd_fmt stOsdFmt;
        memset(&stOsdFmt, 0, sizeof(struct osd_fmt));
        ChannelRegionInfo *pEntry;
        list_for_each_entry(pEntry, &pVipp->mOverlayList, mList)
        {
            if(pEntry->mbDraw)
            {
                if(FALSE == pEntry->mbSetBmp)
                {
                    aloge("fatal error! bmp is not set");
                }
                if(NULL == pEntry->mBmp.mpData)
                {
                    aloge("fatal error! bmpData is not set");
                }
                stOsdFmt.chromakey = map_PIXEL_FORMAT_E_to_V4L2_PIX_FMT(pEntry->mRgnAttr.unAttr.stOverlay.mPixelFmt);
                stOsdFmt.global_alpha = MAX_GLOBAL_ALPHA;
                if(pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Width%OVERLAY_INVERT_UNIT_WIDTH != 0
                    || pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Height%OVERLAY_INVERT_UNIT_HEIGHT != 0)
                {
                    aloge("fatal error! InvColArea[%dx%d] is not align to [%dx%d]", 
                        pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Width,
                        pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Height,
                        OVERLAY_INVERT_UNIT_WIDTH, OVERLAY_INVERT_UNIT_HEIGHT);
                }
                if(stOsdFmt.clipcount >= 8)
                {
                    aloge("fatal error! why elem number[%d] >= 8?", stOsdFmt.clipcount);
                }
                stOsdFmt.inv_w_rgn[stOsdFmt.clipcount] = pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Width/OVERLAY_INVERT_UNIT_WIDTH - 1;
	            stOsdFmt.inv_h_rgn[stOsdFmt.clipcount] = pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.stInvColArea.Height/OVERLAY_INVERT_UNIT_HEIGHT - 1;
                stOsdFmt.inv_th = pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.mLumThresh;
                stOsdFmt.reverse_close[stOsdFmt.clipcount] = pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn==TRUE?0:1;
                if(pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn 
                    && pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod != LESSTHAN_LUMDIFF_THRESH)
                {
                    alogd("Be careful! vipp invert color mode only support LESSTHAN_LUMDIFF_THRESH! But user set mode[0x%x]", pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.enChgMod);
                }
                stOsdFmt.bitmap[stOsdFmt.clipcount] = pEntry->mBmp.mpData;
                stOsdFmt.region[stOsdFmt.clipcount].left = pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stPoint.X;
                stOsdFmt.region[stOsdFmt.clipcount].top = pEntry->mRgnChnAttr.unChnAttr.stOverlayChn.stPoint.Y;
                stOsdFmt.region[stOsdFmt.clipcount].width = pEntry->mRgnAttr.unAttr.stOverlay.mSize.Width;
                stOsdFmt.region[stOsdFmt.clipcount].height = pEntry->mRgnAttr.unAttr.stOverlay.mSize.Height;
                stOsdFmt.clipcount++;
                if(stOsdFmt.clipcount > MAX_OVERLAY_NUM)
                {
                    aloge("fatal error! clipcount[%d] exceed!", stOsdFmt.clipcount);
                }
            }
        }
        char bitmap[100];
        if(0 == stOsdFmt.clipcount)
        {
            bitmap[0] = 'c';
            stOsdFmt.bitmap[0] = &bitmap[0];
        }
        int ret1 = overlay_set_fmt(video, &stOsdFmt);
        int ret2 = overlay_update(video, 1);
        if(ret1 != 0)
        {
            aloge("fatal error! set overlay fail[%d]", ret1);
            ret = ERR_VI_NOT_SUPPORT;
        }
        if(ret2 != 0)
        {
            aloge("fatal error! overlay update fail[%d]", ret2);
            ret = ERR_VI_NOT_SUPPORT;
        }
    }
    else
    {
        struct osd_fmt stOsdFmt;
        memset(&stOsdFmt, 0, sizeof(struct osd_fmt));
        char bitmap[100];
        bitmap[0] = 'c';
        stOsdFmt.clipcount = 0;
        stOsdFmt.bitmap[0] = &bitmap[0];
        int ret1 = overlay_set_fmt(video, &stOsdFmt);
        int ret2 = overlay_update(video, 1);
        if(ret1 != 0)
        {
            aloge("fatal error! set overlay fail[%d]", ret1);
            ret = ERR_VI_NOT_SUPPORT;
        }
        if(ret2 != 0)
        {
            aloge("fatal error! overlay update fail[%d]", ret2);
            ret = ERR_VI_NOT_SUPPORT;
        }
    }
    //draw cover
    if(!list_empty(&pVipp->mCoverList))
    {
        struct osd_fmt stOsdFmt;
        memset(&stOsdFmt, 0, sizeof(struct osd_fmt));
        ChannelRegionInfo *pEntry;
        list_for_each_entry(pEntry, &pVipp->mCoverList, mList)
        {
            if(pEntry->mbDraw)
            {
                stOsdFmt.global_alpha = MAX_GLOBAL_ALPHA;
                stOsdFmt.rgb_cover[stOsdFmt.clipcount] = pEntry->mRgnChnAttr.unChnAttr.stCoverChn.mColor;
                stOsdFmt.bitmap[stOsdFmt.clipcount] = NULL;
                if(AREA_RECT == pEntry->mRgnChnAttr.unChnAttr.stCoverChn.enCoverType)
                {
                    stOsdFmt.region[stOsdFmt.clipcount].left = pEntry->mRgnChnAttr.unChnAttr.stCoverChn.stRect.X;
                    stOsdFmt.region[stOsdFmt.clipcount].top = pEntry->mRgnChnAttr.unChnAttr.stCoverChn.stRect.Y;
                    stOsdFmt.region[stOsdFmt.clipcount].width = pEntry->mRgnChnAttr.unChnAttr.stCoverChn.stRect.Width;
                    stOsdFmt.region[stOsdFmt.clipcount].height = pEntry->mRgnChnAttr.unChnAttr.stCoverChn.stRect.Height;
                    stOsdFmt.clipcount++;
                }
                else
                {
                    aloge("fatal error! coverType[0x%x] is not rect!", pEntry->mRgnChnAttr.unChnAttr.stCoverChn.enCoverType);
                }
                if(stOsdFmt.clipcount > MAX_COVER_NUM)
                {
                    aloge("fatal error! clipcount[%d] exceed!", stOsdFmt.clipcount);
                }
            }
        }
        int ret1 = overlay_set_fmt(video, &stOsdFmt);
        int ret2 = overlay_update(video, 1);
        if(ret1 != 0)
        {
            aloge("fatal error! set cover fail[%d]", ret1);
            ret = ERR_VI_NOT_SUPPORT;
        }
        if(ret2 != 0)
        {
            aloge("fatal error! cover update fail[%d]", ret2);
            ret = ERR_VI_NOT_SUPPORT;
        }
    }
    else
    {
        struct osd_fmt stOsdFmt;
        memset(&stOsdFmt, 0, sizeof(struct osd_fmt));
        stOsdFmt.clipcount = 0;
        stOsdFmt.bitmap[0] = NULL;
        int ret1 = overlay_set_fmt(video, &stOsdFmt);
        int ret2 = overlay_update(video, 1);
        if(ret1 != 0)
        {
            aloge("fatal error! set cover fail[%d]", ret1);
            ret = ERR_VI_NOT_SUPPORT;
        }
        if(ret2 != 0)
        {
            aloge("fatal error! cover update fail[%d]", ret2);
            ret = ERR_VI_NOT_SUPPORT;
        }
    }
    //draw ORL(Object Rectangle Label)
    if(!list_empty(&pVipp->mOrlList))
    {
        struct orl_fmt stOrlFmt;
        memset(&stOrlFmt, 0, sizeof(struct orl_fmt));
        ChannelRegionInfo *pEntry;
        list_for_each_entry(pEntry, &pVipp->mOrlList, mList)
        {
            if(pEntry->mbDraw)
            {
                stOrlFmt.mThick = pEntry->mRgnChnAttr.unChnAttr.stOrlChn.mThick;
                stOrlFmt.mRgbColor[stOrlFmt.clipcount] = pEntry->mRgnChnAttr.unChnAttr.stOrlChn.mColor;
                if(AREA_RECT == pEntry->mRgnChnAttr.unChnAttr.stOrlChn.enAreaType)
                {
                    stOrlFmt.region[stOrlFmt.clipcount].left = pEntry->mRgnChnAttr.unChnAttr.stOrlChn.stRect.X;
                    stOrlFmt.region[stOrlFmt.clipcount].top = pEntry->mRgnChnAttr.unChnAttr.stOrlChn.stRect.Y;
                    stOrlFmt.region[stOrlFmt.clipcount].width = pEntry->mRgnChnAttr.unChnAttr.stOrlChn.stRect.Width;
                    stOrlFmt.region[stOrlFmt.clipcount].height = pEntry->mRgnChnAttr.unChnAttr.stOrlChn.stRect.Height;
                    stOrlFmt.clipcount++;
                }
                else
                {
                    aloge("fatal error! Orl areaType[0x%x] is not rect!", pEntry->mRgnChnAttr.unChnAttr.stOrlChn.enAreaType);
                }
                
                if(stOrlFmt.clipcount >= MAX_ORL_NUM)
                {
                    aloge("fatal error! clipcount[%d] exceed!", stOrlFmt.clipcount);
                    break;
                }
            }
        }
        int ret1 = orl_set_fmt(video, &stOrlFmt);
        int ret2 = overlay_update(video, 1);
        if(ret1 != 0)
        {
            aloge("fatal error! set orl fail[%d]", ret1);
            ret = ERR_VI_NOT_SUPPORT;
        }
        if(ret2 != 0)
        {
            aloge("fatal error! orl update fail[%d]", ret2);
            ret = ERR_VI_NOT_SUPPORT;
        }
    }
    else
    {
        struct orl_fmt stOrlFmt;
        memset(&stOrlFmt, 0, sizeof(struct orl_fmt));
        stOrlFmt.clipcount = 0;
        int ret1 = orl_set_fmt(video, &stOrlFmt);
        int ret2 = overlay_update(video, 1);
        if(ret1 != 0)
        {
            aloge("fatal error! set cover fail[%d]", ret1);
            ret = ERR_VI_NOT_SUPPORT;
        }
        if(ret2 != 0)
        {
            aloge("fatal error! cover update fail[%d]", ret2);
            ret = ERR_VI_NOT_SUPPORT;
        }
    }
    return ret;
}

/**
 * @return true: first < second, false:first >= second
 */
static BOOL compareRegionPosition(const RGN_CHN_ATTR_S *pFirst, const RGN_CHN_ATTR_S *pSecond)
{
    if(pFirst->enType != pSecond->enType)
    {
        aloge("fatal error! why rgnType is not match[0x%x]!=[0x%x]", pFirst->enType, pSecond->enType);
        return FALSE;
    }
    if(OVERLAY_RGN == pFirst->enType)
    {
        if(pFirst->unChnAttr.stOverlayChn.stPoint.Y < pSecond->unChnAttr.stOverlayChn.stPoint.Y)
        {
            return TRUE;
        }
        if(pFirst->unChnAttr.stOverlayChn.stPoint.Y > pSecond->unChnAttr.stOverlayChn.stPoint.Y)
        {
            return FALSE;
        }
        if(pFirst->unChnAttr.stOverlayChn.stPoint.X < pSecond->unChnAttr.stOverlayChn.stPoint.X)
        {
            return TRUE;
        }
        if(pFirst->unChnAttr.stOverlayChn.stPoint.X > pSecond->unChnAttr.stOverlayChn.stPoint.X)
        {
            return FALSE;
        }
        return FALSE;
    }
    else if(COVER_RGN == pFirst->enType)
    {
        if(AREA_RECT == pFirst->unChnAttr.stCoverChn.enCoverType)
        {
            if(pFirst->unChnAttr.stCoverChn.stRect.Y < pSecond->unChnAttr.stCoverChn.stRect.Y)
            {
                return TRUE;
            }
            if(pFirst->unChnAttr.stCoverChn.stRect.Y > pSecond->unChnAttr.stCoverChn.stRect.Y)
            {
                return FALSE;
            }
            if(pFirst->unChnAttr.stCoverChn.stRect.X < pSecond->unChnAttr.stCoverChn.stRect.X)
            {
                return TRUE;
            }
            if(pFirst->unChnAttr.stCoverChn.stRect.X > pSecond->unChnAttr.stCoverChn.stRect.X)
            {
                return FALSE;
            }
            return FALSE;
        }
        else
        {
            aloge("fatal error! not support cover type[0x%x]", pFirst->unChnAttr.stCoverChn.enCoverType);
            return FALSE;
        }
    }
    else
    {
        aloge("fatal error! unsupport rgnType[0x%x]", pFirst->enType);
        return FALSE;
    }
}

/**
 * @return true: priority first < second, false:priority first >= second
 */
static BOOL compareRegionPriority(const RGN_CHN_ATTR_S *pFirst, const RGN_CHN_ATTR_S *pSecond)
{
    if(pFirst->enType != pSecond->enType)
    {
        aloge("fatal error! why rgnType is not match[0x%x]!=[0x%x]", pFirst->enType, pSecond->enType);
        return FALSE;
    }
    if(OVERLAY_RGN == pFirst->enType)
    {
        if(pFirst->unChnAttr.stOverlayChn.mLayer < pSecond->unChnAttr.stOverlayChn.mLayer)
        {
            return TRUE;
        }
        return FALSE;
    }
    else if(COVER_RGN == pFirst->enType)
    {
        if(AREA_RECT == pFirst->unChnAttr.stCoverChn.enCoverType)
        {
            if(pFirst->unChnAttr.stCoverChn.mLayer < pSecond->unChnAttr.stCoverChn.mLayer)
            {
                return TRUE;
            }
            return FALSE;
        }
        else
        {
            aloge("fatal error! not support cover type[0x%x]", pFirst->unChnAttr.stCoverChn.enCoverType);
            return FALSE;
        }
    }
    else if(ORL_RGN == pFirst->enType)
    {
        if(AREA_RECT == pFirst->unChnAttr.stOrlChn.enAreaType)
        {
            if(pFirst->unChnAttr.stOrlChn.mLayer < pSecond->unChnAttr.stOrlChn.mLayer)
            {
                return TRUE;
            }
            return FALSE;
        }
        else
        {
            aloge("fatal error! not support cover type[0x%x]", pFirst->unChnAttr.stOrlChn.enAreaType);
            return FALSE;
        }
    }
    else
    {
        aloge("fatal error! unsupport rgnType[0x%x]", pFirst->enType);
        return FALSE;
    }
}

BOOL checkRegionPositionValid(RGN_ATTR_S *pRgnAttr, RGN_CHN_ATTR_S *pRgnChnAttr)
{
    BOOL bValid = TRUE;
#if(AWCHIP == AW_V5)
#elif(AWCHIP == AW_V316)
    if(OVERLAY_RGN == pRgnAttr->enType)
    {
        if(pRgnAttr->unAttr.stOverlay.mSize.Width%OVERLAY_INVERT_UNIT_WIDTH != 0
            || pRgnAttr->unAttr.stOverlay.mSize.Height%OVERLAY_INVERT_UNIT_HEIGHT != 0
            || pRgnChnAttr->unChnAttr.stOverlayChn.stPoint.X%OVERLAY_INVERT_UNIT_WIDTH != 0
            || pRgnChnAttr->unChnAttr.stOverlayChn.stPoint.Y%OVERLAY_INVERT_UNIT_HEIGHT != 0)
        {
            aloge("fatal error! region position [%d,%d, %dx%d] is invalid!", 
                pRgnChnAttr->unChnAttr.stOverlayChn.stPoint.X,pRgnChnAttr->unChnAttr.stOverlayChn.stPoint.Y, 
                pRgnAttr->unAttr.stOverlay.mSize.Width, pRgnAttr->unAttr.stOverlay.mSize.Height);
            bValid = FALSE;
        }
    }
#elif(AWCHIP == AW_V459)
    if(ORL_RGN == pRgnChnAttr->enType)
    {
        if(pRgnChnAttr->unChnAttr.stOrlChn.stRect.X%2 != 0
            || pRgnChnAttr->unChnAttr.stOrlChn.stRect.Y%2 != 0
            || pRgnChnAttr->unChnAttr.stOrlChn.stRect.Width%2 != 0
            || pRgnChnAttr->unChnAttr.stOrlChn.stRect.Height%2 != 0)
        {
            aloge("fatal error! region position [%d,%d, %dx%d] is invalid!", 
                pRgnChnAttr->unChnAttr.stOrlChn.stRect.X, pRgnChnAttr->unChnAttr.stOrlChn.stRect.Y, 
                pRgnChnAttr->unChnAttr.stOrlChn.stRect.Width, pRgnChnAttr->unChnAttr.stOrlChn.stRect.Height);
            bValid = FALSE;
        }
    }
    else
    {
        aloge("fatal error! v459 donot support region type:%d!", pRgnAttr->enType);
        bValid = FALSE;
    }
#endif
    return bValid;
}

ERRORTYPE videoInputHw_SetRegion(VI_DEV vipp_id, RGN_HANDLE RgnHandle, RGN_ATTR_S *pRgnAttr, const RGN_CHN_ATTR_S *pRgnChnAttr, BITMAP_S *pBmp)
{
    ERRORTYPE ret = SUCCESS;
    if (vipp_id >= HW_VIDEO_DEVICE_NUM || vipp_id < 0)
    {
        aloge("vipp[%d] is invalid!", vipp_id);
        return ERR_VI_INVALID_CHNID;
    }

    if (gpVIDevManager->gpVippManager[vipp_id] == NULL)
    {
        return ERR_VI_INVALID_NULL_PTR;
    }
    viChnManager *pVipp = gpVIDevManager->gpVippManager[vipp_id];

    //check if region position and size fulfill the align request.
    if(FALSE == checkRegionPositionValid(pRgnAttr, pRgnChnAttr))
    {
        aloge("fatal error! region position is invalid, ignore this region!");
        return ERR_VI_INVALID_PARA;
    }

    pthread_mutex_lock(&pVipp->mRegionLock);
    //if handle is exist, return.
    ChannelRegionInfo *pEntry;
    list_for_each_entry(pEntry, &pVipp->mOverlayList, mList)
    {
        if(RgnHandle == pEntry->mRgnHandle)
        {
            aloge("fatal error! RgnHandle[%d] is already exist!", RgnHandle);
            ret = ERR_VI_EXIST;
            goto _err0;
        }
    }
    list_for_each_entry(pEntry, &pVipp->mCoverList, mList)
    {
        if(RgnHandle == pEntry->mRgnHandle)
        {
            aloge("fatal error! RgnHandle[%d] is already exist!", RgnHandle);
            ret = ERR_VI_EXIST;
            goto _err0;
        }
    }
    list_for_each_entry(pEntry, &pVipp->mOrlList, mList)
    {
        if(RgnHandle == pEntry->mRgnHandle)
        {
            aloge("fatal error! RgnHandle[%d] is already exist!", RgnHandle);
            ret = ERR_VI_EXIST;
            goto _err0;
        }
    }
    ChannelRegionInfo *pRegion = ChannelRegionInfo_Construct();
    if(NULL == pRegion)
    {
        aloge("fatal error! malloc fail!");
        ret = ERR_VI_NOMEM;
        goto _err0;
    }
    pRegion->mRgnHandle = RgnHandle;
    pRegion->mRgnAttr = *pRgnAttr;
    pRegion->mRgnChnAttr = *pRgnChnAttr;
    if(pBmp)
    {
        pRegion->mbSetBmp = TRUE;
        pRegion->mBmp = *pBmp;
        int nSize = BITMAP_S_GetdataSize(&pRegion->mBmp);
        pRegion->mBmp.mpData = malloc(nSize);
        if(NULL == pRegion->mBmp.mpData)
        {
            aloge("fatal error! malloc fail!");
            free(pRegion);
            ret = ERR_VI_NOMEM;
            goto _err0;
        }
        memcpy(pRegion->mBmp.mpData, pBmp->mpData, nSize);
    }
    else
    {
        pRegion->mbSetBmp = FALSE;
    }
    if(OVERLAY_RGN == pRegion->mRgnAttr.enType)
    {
        //sort from small priority to large priority.
        if(!list_empty(&pVipp->mOverlayList))
        {
            BOOL bInsert = FALSE;
            ChannelRegionInfo *pEntry;
            list_for_each_entry(pEntry, &pVipp->mOverlayList, mList)
            {
                if(TRUE == compareRegionPriority(&pRegion->mRgnChnAttr, &pEntry->mRgnChnAttr))
                {
                    list_add_tail(&pRegion->mList, &pEntry->mList);
                    bInsert = TRUE;
                    break;
                }
            }
            if(!bInsert)
            {
                list_add_tail(&pRegion->mList, &pVipp->mOverlayList);
            }
        }
        else
        {
            list_add_tail(&pRegion->mList, &pVipp->mOverlayList);
        }
    }
    else if(COVER_RGN == pRegion->mRgnAttr.enType)
    {
        //sort from small priority to large priority.
        if(!list_empty(&pVipp->mCoverList))
        {
            BOOL bInsert = FALSE;
            ChannelRegionInfo *pEntry;
            list_for_each_entry(pEntry, &pVipp->mCoverList, mList)
            {
                if(TRUE == compareRegionPriority(&pRegion->mRgnChnAttr, &pEntry->mRgnChnAttr))
                {
                    list_add_tail(&pRegion->mList, &pEntry->mList);
                    bInsert = TRUE;
                    break;
                }
            }
            if(!bInsert)
            {
                list_add_tail(&pRegion->mList, &pVipp->mCoverList);
            }
        }
        else
        {
            list_add_tail(&pRegion->mList, &pVipp->mCoverList);
        }
    }
    else if(ORL_RGN == pRegion->mRgnAttr.enType)
    {
        //sort from small priority to large priority.
        if(!list_empty(&pVipp->mOrlList))
        {
            BOOL bInsert = FALSE;
            ChannelRegionInfo *pEntry;
            list_for_each_entry(pEntry, &pVipp->mOrlList, mList)
            {
                if(TRUE == compareRegionPriority(&pRegion->mRgnChnAttr, &pEntry->mRgnChnAttr))
                {
                    list_add_tail(&pRegion->mList, &pEntry->mList);
                    bInsert = TRUE;
                    break;
                }
            }
            if(!bInsert)
            {
                list_add_tail(&pRegion->mList, &pVipp->mOrlList);
            }
        }
        else
        {
            list_add_tail(&pRegion->mList, &pVipp->mOrlList);
        }
    }
    else
    {
        aloge("fatal error! unsupport rgnType[0x%x]", pRegion->mRgnAttr.enType);
        if(pRegion->mBmp.mpData)
        {
            free(pRegion->mBmp.mpData);
            pRegion->mBmp.mpData = NULL;
        }
        free(pRegion);
        goto _err0;
    }
    //decide if draw this region
    if(pRegion->mRgnChnAttr.bShow)
    {
        if(OVERLAY_RGN == pRegion->mRgnAttr.enType)
        {
            if(pRegion->mbSetBmp)
            {
                pRegion->mbDraw = TRUE;
            }
            else
            {
                pRegion->mbDraw = FALSE;
            }
        }
        else
        {
            pRegion->mbDraw = TRUE;
        }
    }
    else
    {
        pRegion->mbDraw = FALSE;
    }

    if(pRegion->mbDraw)
    {
        pthread_mutex_lock(&pVipp->mLock);
        if(pVipp->vipp_enable)
        {
          #if(AWCHIP == AW_V5)
            videoInputHw_DrawOSD_V5(vipp_id);
          #elif(AWCHIP == AW_V316 || AWCHIP == AW_V459)
            videoInputHw_DrawOSD(vipp_id);
          #else
            videoInputHw_DrawOSD(vipp_id);
          #endif
        }
        else
        {
            alogw("Be careful! can't draw osd during vipp disable!");
        }
        pthread_mutex_unlock(&pVipp->mLock);
    }
    pthread_mutex_unlock(&pVipp->mRegionLock);
    return ret;

_err1:
_err0:
    pthread_mutex_unlock(&pVipp->mRegionLock);
    return ret;
}

ERRORTYPE videoInputHw_DeleteRegion(VI_DEV vipp_id, RGN_HANDLE RgnHandle)
{
    ERRORTYPE ret = SUCCESS;
    if (vipp_id >= HW_VIDEO_DEVICE_NUM || vipp_id < 0)
    {
        aloge("vipp[%d] is invalid!", vipp_id);
        return ERR_VI_INVALID_CHNID;
    }

    if (gpVIDevManager->gpVippManager[vipp_id] == NULL)
    {
        return ERR_VI_INVALID_NULL_PTR;
    }
    viChnManager *pVipp = gpVIDevManager->gpVippManager[vipp_id];
    pthread_mutex_lock(&pVipp->mRegionLock);
    //find handle
    BOOL bFind = FALSE;
    ChannelRegionInfo *pRegion;
    if(!list_empty(&pVipp->mOverlayList))
    {
        list_for_each_entry(pRegion, &pVipp->mOverlayList, mList)
        {
            if(RgnHandle == pRegion->mRgnHandle)
            {
                bFind = TRUE;
                break;
            }
        }
    }
    if(FALSE == bFind && !list_empty(&pVipp->mCoverList))
    {
        list_for_each_entry(pRegion, &pVipp->mCoverList, mList)
        {
            if(RgnHandle == pRegion->mRgnHandle)
            {
                bFind = TRUE;
                break;
            }
        }
    }
    if(FALSE == bFind && !list_empty(&pVipp->mOrlList))
    {
        list_for_each_entry(pRegion, &pVipp->mOrlList, mList)
        {
            if(RgnHandle == pRegion->mRgnHandle)
            {
                bFind = TRUE;
                break;
            }
        }
    }
    if(FALSE == bFind)
    {
        aloge("fatal error! can't find rgnHandle[%d]", RgnHandle);
        ret = ERR_VI_UNEXIST;
        goto _err0;
    }
    list_del(&pRegion->mList);
    if(pRegion->mbDraw)
    {
        //need redraw osd
        pthread_mutex_lock(&pVipp->mLock);
        if(pVipp->vipp_enable)
        {
          #if(AWCHIP == AW_V5)
            videoInputHw_DrawOSD_V5(vipp_id);
          #elif(AWCHIP == AW_V316 || AWCHIP == AW_V459)
            videoInputHw_DrawOSD(vipp_id);
          #else
            videoInputHw_DrawOSD(vipp_id);
          #endif
        }
        else
        {
            alogw("Be careful! can't draw osd during vipp disable!");
        }
        pthread_mutex_unlock(&pVipp->mLock);
    }
    ChannelRegionInfo_Destruct(pRegion);
    pthread_mutex_unlock(&pVipp->mRegionLock);
    return ret;

_err0:
    pthread_mutex_unlock(&pVipp->mRegionLock);
    return ret;
}

ERRORTYPE videoInputHw_UpdateOverlayBitmap(VI_DEV vipp_id, RGN_HANDLE RgnHandle, BITMAP_S *pBitmap)
{
    ERRORTYPE ret = SUCCESS;
    if (vipp_id >= HW_VIDEO_DEVICE_NUM || vipp_id < 0)
    {
        aloge("vipp[%d] is invalid!", vipp_id);
        return ERR_VI_INVALID_CHNID;
    }

    if (gpVIDevManager->gpVippManager[vipp_id] == NULL)
    {
        return ERR_VI_INVALID_NULL_PTR;
    }
    viChnManager *pVipp = gpVIDevManager->gpVippManager[vipp_id];
    pthread_mutex_lock(&pVipp->mRegionLock);
    //find handle
    BOOL bFind = FALSE;
    ChannelRegionInfo *pRegion;
    list_for_each_entry(pRegion, &pVipp->mOverlayList, mList)
    {
        if(RgnHandle == pRegion->mRgnHandle)
        {
            bFind = TRUE;
            break;
        }
    }
    if(FALSE == bFind)
    {
        ret = ERR_VI_UNEXIST;
        goto _err0;
    }
    if(pRegion->mRgnAttr.enType != OVERLAY_RGN)
    {
        aloge("fatal error! rgn type[0x%x] is not overlay!", pRegion->mRgnAttr.enType);
        ret = ERR_VI_INVALID_PARA;
        goto _err0;
    }
    int size0 = 0;
    int size1 = BITMAP_S_GetdataSize(pBitmap);
    if(pRegion->mbSetBmp)
    {
        size0 = BITMAP_S_GetdataSize(&pRegion->mBmp);
        if(size0 != size1)
        {
            aloge("fatal error! bmp size[%d]!=[%d]", size0, size1);
            free(pRegion->mBmp.mpData);
            pRegion->mBmp.mpData = NULL;
            pRegion->mbSetBmp = FALSE;
        }
    }
    if(FALSE == pRegion->mbSetBmp)
    {
        pRegion->mBmp = *pBitmap;
        pRegion->mBmp.mpData = malloc(size1);
        if(NULL == pRegion->mBmp.mpData)
        {
            aloge("fatal error! malloc fail!");
        }
        pRegion->mbSetBmp = TRUE;
    }
    memcpy(pRegion->mBmp.mpData, pBitmap->mpData, size1);
    if(pBitmap->mWidth != pRegion->mRgnAttr.unAttr.stOverlay.mSize.Width || pBitmap->mHeight != pRegion->mRgnAttr.unAttr.stOverlay.mSize.Height)
    {
        alogw("Be careful! bitmap size[%dx%d] != region size[%dx%d], need update region size!", pBitmap->mWidth, pBitmap->mHeight, pRegion->mRgnAttr.unAttr.stOverlay.mSize.Width, pRegion->mRgnAttr.unAttr.stOverlay.mSize.Height);
        pRegion->mRgnAttr.unAttr.stOverlay.mSize.Width = pBitmap->mWidth;
        pRegion->mRgnAttr.unAttr.stOverlay.mSize.Height = pBitmap->mHeight;
    }
    //decide if draw this region
    if(pRegion->mRgnChnAttr.bShow)
    {
        pRegion->mbDraw = TRUE;
    }
    else
    {
        pRegion->mbDraw = FALSE;
    }

    if(pRegion->mbDraw)
    {
        pthread_mutex_lock(&pVipp->mLock);
        if(pVipp->vipp_enable)
        {
          #if(AWCHIP == AW_V5)
            videoInputHw_DrawOSD_V5(vipp_id);
          #elif(AWCHIP == AW_V316 || AWCHIP == AW_V459)
            videoInputHw_DrawOSD(vipp_id);
          #else
            videoInputHw_DrawOSD(vipp_id);
          #endif
        }
        else
        {
            alogw("Be careful! can't draw osd during vipp disable!");
        }
        pthread_mutex_unlock(&pVipp->mLock);
    }
    pthread_mutex_unlock(&pVipp->mRegionLock);
    return ret;

_err0:
    pthread_mutex_unlock(&pVipp->mRegionLock);
    return ret;
}

ERRORTYPE videoInputHw_UpdateRegionChnAttr(VI_DEV vipp_id, RGN_HANDLE RgnHandle, const RGN_CHN_ATTR_S *pRgnChnAttr)
{
    ERRORTYPE ret = SUCCESS;
    if (vipp_id >= HW_VIDEO_DEVICE_NUM || vipp_id < 0)
    {
        aloge("vipp[%d] is invalid!", vipp_id);
        return ERR_VI_INVALID_CHNID;
    }

    if (gpVIDevManager->gpVippManager[vipp_id] == NULL)
    {
        return ERR_VI_INVALID_NULL_PTR;
    }
    viChnManager *pVipp = gpVIDevManager->gpVippManager[vipp_id];
    pthread_mutex_lock(&pVipp->mRegionLock);
    //find handle
    BOOL bFind = FALSE;
    ChannelRegionInfo *pRegion;
    if(OVERLAY_RGN == pRgnChnAttr->enType)
    {
        list_for_each_entry(pRegion, &pVipp->mOverlayList, mList)
        {
            if(RgnHandle == pRegion->mRgnHandle)
            {
                bFind = TRUE;
                break;
            }
        }
    }
    else if(COVER_RGN == pRgnChnAttr->enType)
    {
        list_for_each_entry(pRegion, &pVipp->mCoverList, mList)
        {
            if(RgnHandle == pRegion->mRgnHandle)
            {
                bFind = TRUE;
                break;
            }
        }
    }
    else if(ORL_RGN == pRgnChnAttr->enType)
    {
        list_for_each_entry(pRegion, &pVipp->mOrlList, mList)
        {
            if(RgnHandle == pRegion->mRgnHandle)
            {
                bFind = TRUE;
                break;
            }
        }
    }
    if(FALSE == bFind)
    {
        ret = ERR_VI_UNEXIST;
        goto _err0;
    }

    if(OVERLAY_RGN == pRgnChnAttr->enType)
    {
        BOOL bUpdate = FALSE;
        if(pRegion->mRgnChnAttr.bShow != pRgnChnAttr->bShow)
        {
            alogd("bShow change [%d]->[%d]", pRegion->mRgnChnAttr.bShow, pRgnChnAttr->bShow);
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.stPoint.X != pRgnChnAttr->unChnAttr.stOverlayChn.stPoint.X
            || pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.stPoint.Y != pRgnChnAttr->unChnAttr.stOverlayChn.stPoint.Y)
        {
            alogd("stPoint change [%d,%d]->[%d,%d]",
                pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.stPoint.X,
                pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.stPoint.Y,
                pRgnChnAttr->unChnAttr.stOverlayChn.stPoint.X,
                pRgnChnAttr->unChnAttr.stOverlayChn.stPoint.Y
                );
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.mFgAlpha != pRgnChnAttr->unChnAttr.stOverlayChn.mFgAlpha)
        {
            alogd("FgAlpha change [%d]->[%d]", pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.mFgAlpha, pRgnChnAttr->unChnAttr.stOverlayChn.mFgAlpha);
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.mLayer != pRgnChnAttr->unChnAttr.stOverlayChn.mLayer)
        {
            alogd("overlay priority(mLayer) change [%d]->[%d]", pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.mLayer, pRgnChnAttr->unChnAttr.stOverlayChn.mLayer);
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn != pRgnChnAttr->unChnAttr.stOverlayChn.stInvertColor.bInvColEn)
        {
            alogd("overlay InvColEn change [%d]->[%d]", pRegion->mRgnChnAttr.unChnAttr.stOverlayChn.stInvertColor.bInvColEn, pRgnChnAttr->unChnAttr.stOverlayChn.stInvertColor.bInvColEn);
            bUpdate = TRUE;
        }
        pRegion->mRgnChnAttr = *pRgnChnAttr;
        //decide if draw this region
        if(pRegion->mRgnChnAttr.bShow && pRegion->mbSetBmp)
        {
            pRegion->mbDraw = TRUE;
        }
        else
        {
            pRegion->mbDraw = FALSE;
        }
        if(bUpdate)
        {
            if(pRegion->mbSetBmp)
            {
                pthread_mutex_lock(&pVipp->mLock);
                if(pVipp->vipp_enable)
                {
                  #if(AWCHIP == AW_V5)
                    videoInputHw_DrawOSD_V5(vipp_id);
                  #elif(AWCHIP == AW_V316 || AWCHIP == AW_V459)
                    videoInputHw_DrawOSD(vipp_id);
                  #else
                    videoInputHw_DrawOSD(vipp_id);
                  #endif
                }
                else
                {
                    alogw("Be careful! can't draw osd during vipp disable!");
                }
                pthread_mutex_unlock(&pVipp->mLock);
            }
        }
    }
    else if(COVER_RGN == pRgnChnAttr->enType)
    {
        BOOL bUpdate = FALSE;
        if(pRegion->mRgnChnAttr.bShow != pRgnChnAttr->bShow)
        {
            alogd("bShow change [%d]->[%d]", pRegion->mRgnChnAttr.bShow, pRgnChnAttr->bShow);
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stCoverChn.enCoverType != pRgnChnAttr->unChnAttr.stCoverChn.enCoverType)
        {
            aloge("fatal error! cover type change [0x%x]->[0x%x]", pRegion->mRgnChnAttr.unChnAttr.stCoverChn.enCoverType, pRgnChnAttr->unChnAttr.stCoverChn.enCoverType);
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stCoverChn.stRect.X != pRgnChnAttr->unChnAttr.stCoverChn.stRect.X
            || pRegion->mRgnChnAttr.unChnAttr.stCoverChn.stRect.Y != pRgnChnAttr->unChnAttr.stCoverChn.stRect.Y
            || pRegion->mRgnChnAttr.unChnAttr.stCoverChn.stRect.Width != pRgnChnAttr->unChnAttr.stCoverChn.stRect.Width
            || pRegion->mRgnChnAttr.unChnAttr.stCoverChn.stRect.Height != pRgnChnAttr->unChnAttr.stCoverChn.stRect.Height)
        {
            alogd("cover rect change [%d,%d,%d,%d]->[%d,%d,%d,%d]",
                pRegion->mRgnChnAttr.unChnAttr.stCoverChn.stRect.X, pRegion->mRgnChnAttr.unChnAttr.stCoverChn.stRect.Y,
                pRegion->mRgnChnAttr.unChnAttr.stCoverChn.stRect.Width, pRegion->mRgnChnAttr.unChnAttr.stCoverChn.stRect.Height,
                pRgnChnAttr->unChnAttr.stCoverChn.stRect.X, pRgnChnAttr->unChnAttr.stCoverChn.stRect.Y,
                pRgnChnAttr->unChnAttr.stCoverChn.stRect.Width, pRgnChnAttr->unChnAttr.stCoverChn.stRect.Height);
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stCoverChn.mColor != pRgnChnAttr->unChnAttr.stCoverChn.mColor)
        {
            alogd("cover color change [0x%x]->[0x%x]", pRegion->mRgnChnAttr.unChnAttr.stCoverChn.mColor, pRgnChnAttr->unChnAttr.stCoverChn.mColor);
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stCoverChn.mLayer != pRgnChnAttr->unChnAttr.stCoverChn.mLayer)
        {
            alogd("cover priority(mLayer) change [%d]->[%d]", pRegion->mRgnChnAttr.unChnAttr.stCoverChn.mLayer, pRgnChnAttr->unChnAttr.stCoverChn.mLayer);
            bUpdate = TRUE;
        }
        pRegion->mRgnChnAttr = *pRgnChnAttr;
        //decide if draw this region
        if(pRegion->mRgnChnAttr.bShow)
        {
            pRegion->mbDraw = TRUE;
        }
        else
        {
            pRegion->mbDraw = FALSE;
        }
        if(bUpdate)
        {
            pthread_mutex_lock(&pVipp->mLock);
            if(pVipp->vipp_enable)
            {
              #if(AWCHIP == AW_V5)
                videoInputHw_DrawOSD_V5(vipp_id);
              #elif(AWCHIP == AW_V316 || AWCHIP == AW_V459)
                videoInputHw_DrawOSD(vipp_id);
              #else
                videoInputHw_DrawOSD(vipp_id);
              #endif
            }
            else
            {
                alogw("Be careful! can't draw osd during vipp disable!");
            }
            pthread_mutex_unlock(&pVipp->mLock);
        }
    }
    else if(ORL_RGN == pRgnChnAttr->enType)
    {
        BOOL bUpdate = FALSE;
        if(pRegion->mRgnChnAttr.unChnAttr.stOrlChn.enAreaType != pRgnChnAttr->unChnAttr.stOrlChn.enAreaType)
        {
            aloge("fatal error! orl type change [0x%x]->[0x%x], ignore this region!", pRegion->mRgnChnAttr.unChnAttr.stOrlChn.enAreaType, pRgnChnAttr->unChnAttr.stOrlChn.enAreaType);
            bUpdate = FALSE;
            goto _update;
        }
        //check if region position and size fulfill the align request.
        if(FALSE == checkRegionPositionValid(NULL, pRgnChnAttr))
        {
            aloge("fatal error! region position is invalid, ignore this region!");
            bUpdate = FALSE;
            goto _update;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stOrlChn.stRect.X != pRgnChnAttr->unChnAttr.stOrlChn.stRect.X
            || pRegion->mRgnChnAttr.unChnAttr.stOrlChn.stRect.Y != pRgnChnAttr->unChnAttr.stOrlChn.stRect.Y
            || pRegion->mRgnChnAttr.unChnAttr.stOrlChn.stRect.Width != pRgnChnAttr->unChnAttr.stOrlChn.stRect.Width
            || pRegion->mRgnChnAttr.unChnAttr.stOrlChn.stRect.Height != pRgnChnAttr->unChnAttr.stOrlChn.stRect.Height)
        {
            alogd("orl rect change [%d,%d,%d,%d]->[%d,%d,%d,%d]",
                pRegion->mRgnChnAttr.unChnAttr.stOrlChn.stRect.X, pRegion->mRgnChnAttr.unChnAttr.stOrlChn.stRect.Y,
                pRegion->mRgnChnAttr.unChnAttr.stOrlChn.stRect.Width, pRegion->mRgnChnAttr.unChnAttr.stOrlChn.stRect.Height,
                pRgnChnAttr->unChnAttr.stOrlChn.stRect.X, pRgnChnAttr->unChnAttr.stOrlChn.stRect.Y,
                pRgnChnAttr->unChnAttr.stOrlChn.stRect.Width, pRgnChnAttr->unChnAttr.stOrlChn.stRect.Height);
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stOrlChn.mColor != pRgnChnAttr->unChnAttr.stOrlChn.mColor)
        {
            alogd("orl color change [0x%x]->[0x%x]", pRegion->mRgnChnAttr.unChnAttr.stOrlChn.mColor, pRgnChnAttr->unChnAttr.stOrlChn.mColor);
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.unChnAttr.stOrlChn.mThick != pRgnChnAttr->unChnAttr.stOrlChn.mThick)
        {
            alogd("orl thick change [0x%x]->[0x%x]", pRegion->mRgnChnAttr.unChnAttr.stOrlChn.mThick, pRgnChnAttr->unChnAttr.stOrlChn.mThick);
            bUpdate = TRUE;
        }
        
        if(pRegion->mRgnChnAttr.unChnAttr.stOrlChn.mLayer != pRgnChnAttr->unChnAttr.stOrlChn.mLayer)
        {
            alogd("orl priority(mLayer) change [%d]->[%d]", pRegion->mRgnChnAttr.unChnAttr.stOrlChn.mLayer, pRgnChnAttr->unChnAttr.stOrlChn.mLayer);
            bUpdate = TRUE;
        }
        if(pRegion->mRgnChnAttr.bShow != pRgnChnAttr->bShow)
        {
            alogd("bShow change [%d]->[%d]", pRegion->mRgnChnAttr.bShow, pRgnChnAttr->bShow);
            bUpdate = TRUE;
        }
        else
        {
            if(FALSE == pRgnChnAttr->bShow)
            {
                alogd("this region remain unshow, so need not update!");
                bUpdate = FALSE;
            }
        }
        pRegion->mRgnChnAttr = *pRgnChnAttr;
        //decide if draw this region
        if(pRegion->mRgnChnAttr.bShow)
        {
            pRegion->mbDraw = TRUE;
        }
        else
        {
            pRegion->mbDraw = FALSE;
        }
      _update:
        if(bUpdate)
        {
            pthread_mutex_lock(&pVipp->mLock);
            if(pVipp->vipp_enable)
            {
              #if(AWCHIP == AW_V5)
                videoInputHw_DrawOSD_V5(vipp_id);
              #elif(AWCHIP == AW_V316 || AWCHIP == AW_V459)
                videoInputHw_DrawOSD(vipp_id);
              #else
                videoInputHw_DrawOSD(vipp_id);
              #endif
            }
            else
            {
                alogw("Be careful! can't draw osd during vipp disable!");
            }
            pthread_mutex_unlock(&pVipp->mLock);
        }
    }
    else
    {
        aloge("fatal error! rgn type[0x%x]", pRgnChnAttr->enType);
    }
    pthread_mutex_unlock(&pVipp->mRegionLock);
    return ret;

_err0:
    pthread_mutex_unlock(&pVipp->mRegionLock);
    return ret;
}

#if 1
/* ==================================================== */
/* Isp set api. start */
/* ==================================================== */
ERRORTYPE videoInputHw_IspAe_SetMode(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id; // 0, 1
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if (0 == value) {
		printf("auto ae.\r\n");
		video_set_control(video, V4L2_CID_EXPOSURE_AUTO, 0); // auto ae
		video_set_control(video, V4L2_CID_AUTOGAIN, 1);
	} else if (1 == value) {
		printf("manual ae.\r\n");
		video_set_control(video, V4L2_CID_EXPOSURE_AUTO, 1); // manual ae
		video_set_control(video, V4L2_CID_AUTOGAIN, 0);
	} else {
		return ERR_VI_INVALID_CHNID;
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAe_SetExposureBias(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if ((value < 0) || (value > 8))
		return ERR_VI_INVALID_CHNID;
	// auto ae
    if (video_set_control(video, V4L2_CID_AUTO_EXPOSURE_BIAS, value) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}
ERRORTYPE videoInputHw_IspAe_SetExposure(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if (video_set_control(video, V4L2_CID_EXPOSURE_ABSOLUTE, value) < 0) {// manual ae,300000
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAe_SetISOSensitiveMode(int *pvipp_id, int mode)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
    if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    mode = !!mode;
    if (video_set_control(video, V4L2_CID_ISO_SENSITIVITY_AUTO, mode) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAe_SetISOSensitive(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
    if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if (value == 0) {
        return video_set_control(video, V4L2_CID_ISO_SENSITIVITY_AUTO, 1); // auto mode
    }

    if (value < 0 || value > 7) {
        aloge("value range should be [1~7], value(%d)", value);
        return ERR_VI_INVALID_PARA;
    }

    if ((video_set_control(video, V4L2_CID_ISO_SENSITIVITY_AUTO, 0/*manual*/) < 0) ||
        (video_set_control(video, V4L2_CID_ISO_SENSITIVITY, value-1) < 0)) {
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAe_SetMetering(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if ((value < 0) || (value > 3)) {
        aloge("out of range, you shoule use [0~3], value(%d)", value);
        return FAILURE;
    }

    /* 0:average, 1:center, 2:spot, 3:matrix */
    if (video_set_control(video, V4L2_CID_EXPOSURE_METERING, value) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}
ERRORTYPE videoInputHw_IspAe_SetGain(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if (video_set_control(video, V4L2_CID_GAIN, value) < 0) {// manual ae,8000
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_SetMode(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
    if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
    if ((value != 0) && (value != 1))
            return ERR_VI_INVALID_CHNID;

    if (0 == value) {
        video_set_control(video, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, 1);// auto awb
    } else if (1 == value) {
        video_set_control(video, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, 0);// manual awb
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_SetColorTemp(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
    if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if (value == 0 || value == 1) {
        printf("Please use <videoInputHw_IspAwb_SetMode> to set AWB mode, value = [%d]\r\n", value);
        return -1;
    }

    if (value < 0 || value > 9) {
        printf("Please use <2~9> to set color temperature, value = [%d]\r\n", value);
        return ERR_VI_INVALID_CHNID;
    }

    if (video_set_control(video, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, value) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_SetFlicker(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value == 0) || (value == 1) || (value == 2) || (value == 3)) {
		if (video_set_control(video, V4L2_CID_POWER_LINE_FREQUENCY, value) < 0) {
	        return FAILURE;
	    }
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_SetMirror(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;

    struct isp_video_device *video = NULL;
    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViCh]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViCh];
    }

	if ((value == 0) || (value == 1)) {
		if (video_set_control(video, V4L2_CID_HFLIP, value) < 0) {
	        return FAILURE;
	    }
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_SetFlip(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;

    struct isp_video_device *video = NULL;
    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViCh]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViCh];
    }

	if ((value == 0) || (value == 1)) {
		if (video_set_control(video, V4L2_CID_VFLIP, value) < 0) {
        	return FAILURE;
    	}
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_SetBrightness(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= -126) && (value <= 126)) {
		if (video_set_control(video, V4L2_CID_BRIGHTNESS, value) < 0) {
        	return FAILURE;
    	}
	}

    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_SetContrast(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= -64) && (value <= 64)) {
		if (video_set_control(video, V4L2_CID_CONTRAST, value) < 0) {
        	return FAILURE;
    	}
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_SetSaturation(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if ((value >= -256) && (value <= 512)) {
        if (video_set_control(video, V4L2_CID_SATURATION, value) < 0) {
            return FAILURE;
        }
    } else {
        ISP_ERR("invalid parameter range, [-256, 512]");
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_SetSharpness(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if ((value < -32) || (value > 32)) {
        aloge("out of range, should be[-32~32], value(%d)", value);
        return FAILURE;
    }

    if (video_set_control(video, V4L2_CID_SHARPNESS, value) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_SetHue(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= 0) && (value <= 255)) {
		if (video_set_control(video, V4L2_CID_HUE, value) < 0) {
        	return FAILURE;
    	}
	}

    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_SetScene(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if (video_set_control(video, V4L2_CID_SCENE_MODE, !!value) < 0) {
		return FAILURE;
	}

    return SUCCESS;
}

/* ==================================================== */
/* Isp set api. end */
/* ==================================================== */
#endif

#if 1
/* ==================================================== */
/* Isp get api. start */
/* ==================================================== */
ERRORTYPE videoInputHw_IspAe_GetMode(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
    if (video_get_control(video, V4L2_CID_EXPOSURE_AUTO, value) < 0) {
        return FAILURE;
    }
    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAe_GetExposureBias(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
    if (video_get_control(video, V4L2_CID_AUTO_EXPOSURE_BIAS, value) < 0) {
        return FAILURE;
    }
    return SUCCESS;
}
ERRORTYPE videoInputHw_IspAe_GetExposure(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
    if (video_get_control(video, V4L2_CID_EXPOSURE_ABSOLUTE, value) < 0) {
        return FAILURE;
    }
    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAe_GetISOSensitiveMode(int *pvipp_id, int *mode)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
    if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if (video_get_control(video, V4L2_CID_ISO_SENSITIVITY_AUTO, mode) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAe_GetISOSensitive(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
    if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if (video_get_control(video, V4L2_CID_ISO_SENSITIVITY_AUTO, value) < 0) {
        return FAILURE;
    } else {
        if (1 == *value) {
            *value = 0;
            return SUCCESS;
        }
    }

    if (video_get_control(video, V4L2_CID_ISO_SENSITIVITY, value) < 0) {
        return FAILURE;
    }
    *value = *value + 1;

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAe_GetMetering(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
    if (video_get_control(video, V4L2_CID_EXPOSURE_METERING, value) < 0) {
        return FAILURE;
    }
    return SUCCESS;
}
ERRORTYPE videoInputHw_IspAe_GetGain(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    if (video_get_control(video, V4L2_CID_GAIN, value) < 0) {
        return FAILURE;
    }
    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_GetMode(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
    if (video_get_control(video, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, value) < 0) {
        return FAILURE;
    }
	if (*value == 0)
		*value = 1;
	else if (*value == 1)
		*value = 0;

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_GetColorTemp(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
    if (video_get_control(video, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, value) < 0) {
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_GetFlicker(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (video_get_control(video, V4L2_CID_POWER_LINE_FREQUENCY, value) < 0) {
        return FAILURE;
    }
    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_GetMirror(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;

    struct isp_video_device *video = NULL;
    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViCh]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViCh];
    }
	if (video_get_control(video, V4L2_CID_HFLIP, value) < 0) {
     	return FAILURE;
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_GetFlip(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;

    struct isp_video_device *video = NULL;
    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViCh]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViCh];
    }
	if (video_get_control(video, V4L2_CID_VFLIP, value) < 0) {
    	return FAILURE;
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_GetBrightness(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (video_get_control(video, V4L2_CID_BRIGHTNESS, value) < 0) {
    	return FAILURE;
	}
    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_GetContrast(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }

    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (video_get_control(video, V4L2_CID_CONTRAST, value) < 0) {
    	return FAILURE;
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_Isp_GetSaturation(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (video_get_control(video, V4L2_CID_SATURATION, value) < 0) {
    	return FAILURE;
	}

    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_GetSharpness(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (video_get_control(video, V4L2_CID_SHARPNESS, value) < 0) {
    	return FAILURE;
	}
    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_GetHue(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (video_get_control(video, V4L2_CID_HUE, value) < 0) {
    	return FAILURE;
	}
    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_GetScene(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (video_get_control(video, V4L2_CID_SCENE_MODE, value) < 0) {
    	return FAILURE;
	}
    return SUCCESS;
}

#include "isp_tuning_priv.h"
#include "isp_tuning.h"
#include "isp.h"
ERRORTYPE videoInputHw_Isp_SetWDR(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= 0) && (value <= 255)) {
		if (isp_set_attr_cfg(isp_id, ISP_CTRL_PLTMWDR_STR, &value) < 0) {
        	return FAILURE;
    	}
	}

    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_GetWDR(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (isp_get_attr_cfg(isp_id, ISP_CTRL_PLTMWDR_STR, value) < 0) {
    	return FAILURE;
	}
    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_SetNR(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= 0) && (value <= 1000)) {
		if (isp_set_attr_cfg(isp_id, ISP_CTRL_DN_STR, &value) < 0) {
        	return FAILURE;
    	}
	}

    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_GetNR(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (isp_get_attr_cfg(isp_id, ISP_CTRL_DN_STR, value) < 0) {
    	return FAILURE;
	}
    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_Set3DNR(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= 0) && (value <= 100)) {
		if (isp_set_attr_cfg(isp_id, ISP_CTRL_3DN_STR, &value) < 0) {
        	return FAILURE;
    	}
	}

    return SUCCESS;
}
ERRORTYPE videoInputHw_Isp_Get3DNR(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                alogd("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (isp_get_attr_cfg(isp_id, ISP_CTRL_3DN_STR, value) < 0) {
    	return FAILURE;
	}
    return SUCCESS;
}
ERRORTYPE videoInputHw_IspAwb_SetRGain(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;
    struct isp_wb_gain wb_gain;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= 256) && (value <= 256 * 64)) {
		memset(&wb_gain, 0, sizeof(struct isp_wb_gain));
		if (isp_get_attr_cfg(isp_id, ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
			return FAILURE;
		}
		if(wb_gain.r_gain != value){
			wb_gain.r_gain = value;
			if (isp_set_attr_cfg(isp_id,
					ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
				return FAILURE;
			}
		}
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_GetRGain(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;
    struct isp_wb_gain wb_gain;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    memset(&wb_gain, 0, sizeof(struct isp_wb_gain));
    if (isp_get_attr_cfg(isp_id, ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
	    return FAILURE;
    }
    *value = wb_gain.r_gain;


    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_SetBGain(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;
    struct isp_wb_gain wb_gain;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= 256) && (value <= 256 * 64)) {
		memset(&wb_gain, 0, sizeof(struct isp_wb_gain));
		if (isp_get_attr_cfg(isp_id, ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
			return FAILURE;
		}
		if(wb_gain.b_gain != value){
			wb_gain.b_gain = value;
			if (isp_set_attr_cfg(isp_id,
					ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
				return FAILURE;
			}
		}

	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_GetBGain(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;
    struct isp_wb_gain wb_gain;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    memset(&wb_gain, 0, sizeof(struct isp_wb_gain));
    if (isp_get_attr_cfg(isp_id, ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
	return FAILURE;
    }
    *value = wb_gain.b_gain;

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_SetGrGain(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;
    struct isp_wb_gain wb_gain;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= 256) && (value <= 256 * 64)) {
		memset(&wb_gain, 0, sizeof(struct isp_wb_gain));
		if (isp_get_attr_cfg(isp_id, ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
			return FAILURE;
		}
		if(wb_gain.gr_gain != value){
			wb_gain.gr_gain = value;
			if (isp_set_attr_cfg(isp_id,
					ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
				return FAILURE;
			}
		}
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_GetGrGain(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;
    struct isp_wb_gain wb_gain;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    memset(&wb_gain, 0, sizeof(struct isp_wb_gain));
    if (isp_get_attr_cfg(isp_id, ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
	    return FAILURE;
    }
    *value = wb_gain.gr_gain;

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_SetGbGain(int *pvipp_id, int value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;
    struct isp_wb_gain wb_gain;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

	if ((value >= 256) && (value <= 256 * 64)) {
		memset(&wb_gain, 0, sizeof(struct isp_wb_gain));
		if (isp_get_attr_cfg(isp_id, ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
			return FAILURE;
		}
		if(wb_gain.gb_gain != value){
			wb_gain.gb_gain = value;
			if (isp_set_attr_cfg(isp_id,
					ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
				return FAILURE;
			}
		}
	}

    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_GetGbGain(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;
    struct isp_wb_gain wb_gain;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }

    memset(&wb_gain, 0, sizeof(struct isp_wb_gain));
    if (isp_get_attr_cfg(isp_id, ISP_CTRL_WB_MGAIN, &wb_gain) < 0) {
	    return FAILURE;
    }
    *value = wb_gain.gb_gain;

    return SUCCESS;
}

//add by jason

ERRORTYPE videoInputHw_IspAe_GetExposureLine(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                // printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
    if (video_get_control(video, V4L2_CID_EXPOSURE, value) < 0) {
        return FAILURE;
    }
    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAwb_GetCurColorT(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                // printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (isp_get_attr_cfg(isp_id, ISP_CTRL_COLOR_TEMP, value) < 0) {
    	return FAILURE;
	}
    return SUCCESS;
}

ERRORTYPE videoInputHw_IspAe_GetEvIdx(int *pvipp_id, int *value)
{
    int ViCh = *pvipp_id;
    int i, isp_id, found = 0;

    struct isp_video_device *video = NULL;
	if ((0 != ViCh) && (1 != ViCh))  {
        ISP_ERR("ISP ID[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        for (i = 0; i < MAX_VIPP_DEV_NUM; i++) {
            video = gpVIDevManager->media->video_dev[i];
            if (NULL == video) {
                continue ;
            }
            isp_id = video_to_isp_id(gpVIDevManager->media->video_dev[i]);
            if (isp_id == ViCh) {
                // printf("isp[%d]2vipp[%d].\r\n", ViCh, i);
                found = 1;
                break;
            }
        }
    }
    if (0 == found) {
        printf("No find video open @ isp[%d].\r\n", ViCh);
        return -1;
    }
	if (isp_get_attr_cfg(isp_id, ISP_CTRL_EV_IDX, value) < 0) {
    	return FAILURE;
	}
    return SUCCESS;
}




/* ==================================================== */
/* Isp get api. end */
/* ==================================================== */
#endif

static ERRORTYPE videoInputHw_GetData(int *pvipp_id, VIDEO_FRAME_INFO_S *pstFrameInfo, int nMilliSec)
{
    int ViCh = *pvipp_id;
    viChnManager *pVippInfo = gpVIDevManager->gpVippManager[ViCh];
    struct isp_video_device *video = NULL;
    struct video_buffer buffer;
    struct video_fmt vfmt;
    int i;

    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViCh]) {
        ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViCh];
    }
    if (video_wait_buffer(video, nMilliSec) < 0) {
        return FAILURE;
    }
    if (video_dequeue_buffer(video, &buffer) < 0) {
        return FAILURE;
    }
    memset(&vfmt, 0, sizeof(vfmt));
    video_get_fmt(video, &vfmt);
    for (i = 0; i < vfmt.nplanes; i++) {
		pstFrameInfo->VFrame.mpVirAddr[i] = buffer.planes[i].mem;
		pstFrameInfo->VFrame.mStride[i] = buffer.planes[i].size;//buffer.planes[i].size, 0
		pstFrameInfo->VFrame.mPhyAddr[i] = buffer.planes[i].mem_phy;
    }
    pstFrameInfo->VFrame.mWidth = vfmt.format.width;
    pstFrameInfo->VFrame.mHeight = vfmt.format.height;
    pstFrameInfo->VFrame.mOffsetTop = 0;
    pstFrameInfo->VFrame.mOffsetBottom = pstFrameInfo->VFrame.mHeight;
    pstFrameInfo->VFrame.mOffsetLeft = 0;
    pstFrameInfo->VFrame.mOffsetRight = pstFrameInfo->VFrame.mWidth;
    pstFrameInfo->VFrame.mField = vfmt.format.field;
    pstFrameInfo->VFrame.mPixelFormat = map_V4L2_PIX_FMT_to_PIXEL_FORMAT_E(vfmt.format.pixelformat);// V4L2_PIX_FMT_SBGGR12;
    pstFrameInfo->VFrame.mpts = (int64_t)buffer.timestamp.tv_sec*1000*1000 + buffer.timestamp.tv_usec;
    pstFrameInfo->VFrame.mFramecnt = buffer.frame_cnt;
    pstFrameInfo->VFrame.mExposureTime = buffer.exp_time / 1000;
    pstFrameInfo->mId = buffer.index;

    pthread_mutex_lock(&pVippInfo->mFrameListLock);
    if(list_empty(&pVippInfo->mIdleFrameList))
    {
        alogw("impossible, idle frame list is empty, malloc one");
        VippFrame *pNode = (VippFrame*)malloc(sizeof(VippFrame));
        if(pNode != NULL)
        {
            memset(pNode, 0, sizeof(VippFrame));
            list_add_tail(&pNode->mList, &pVippInfo->mIdleFrameList);
        }
        else
        {
            aloge("fatal error! malloc fail!");
        }
    }
    if(!list_empty(&pVippInfo->mIdleFrameList))
    {
        VippFrame *pNode = list_first_entry(&pVippInfo->mIdleFrameList, VippFrame, mList);
        pNode->mVipp = ViCh;
        pNode->mFrameBufId = pstFrameInfo->mId;
        list_move_tail(&pNode->mList, &pVippInfo->mReadyFrameList);
    }
    pthread_mutex_unlock(&pVippInfo->mFrameListLock);

    return 0;
}

static ERRORTYPE videoInputHw_ReleaseData(int *pvipp_id, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
    int ViCh = *pvipp_id;

    struct isp_video_device *video = NULL;
    if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == gpVIDevManager->media->video_dev[ViCh]) {
        ISP_ERR("fatal error! VIN CH[%d] number is invalid!\n", ViCh);
        return ERR_VI_INVALID_CHNID;
    } else {
        video = gpVIDevManager->media->video_dev[ViCh];
    }
	if (video_queue_buffer(video, pstFrameInfo->mId) < 0) {
        aloge("fatal error! vipp[%d] queue bufferId[%d] fail!", ViCh, pstFrameInfo->mId);
        return FAILURE;
    }
    pthread_mutex_lock(&gpVIDevManager->gpVippManager[ViCh]->mFrameListLock);
    int nMatchNum = 0;
    VippFrame *pEntry, *pTmp;
    list_for_each_entry_safe(pEntry, pTmp, &gpVIDevManager->gpVippManager[ViCh]->mReadyFrameList, mList)
    {
        if(pEntry->mFrameBufId == pstFrameInfo->mId)
        {
            if(pEntry->mVipp != ViCh)
            {
                aloge("fatal error! vipp[%d]!=[%d], check code!", pEntry->mVipp, ViCh);
            }
            nMatchNum++;
            list_move_tail(&pEntry->mList, &gpVIDevManager->gpVippManager[ViCh]->mIdleFrameList);
        }
    }
    if(nMatchNum != 1)
    {
        aloge("fatal error! matchNum[%d]!=1, vipp[%d]frameBufId[%d]", nMatchNum, ViCh, pstFrameInfo->mId);
    }
    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[ViCh]->mFrameListLock);

    return SUCCESS;
}

ERRORTYPE videoInputHw_RefsIncrease(int vipp_id, VIDEO_FRAME_INFO_S *pstFrameInfo)
{ // it will be not called.
	gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo->mId]++;
	return SUCCESS;
}

ERRORTYPE videoInputHw_RefsReduceAndRleaseData(int vipp_id, VIDEO_FRAME_INFO_S *pstFrameInfo)
{
	int refs = 0, ret = -1;
    pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock);
    if(gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo->mId] <= 0)
    {
        aloge("fatal error! vipp[%d], idx[%d]: ref=[%d] when reduce refs, check code!", vipp_id, pstFrameInfo->mId,
            gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo->mId]);
    }
    gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo->mId]--;
	refs = gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo->mId];
	pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock);

	if (0 == refs)
	{
		ret = videoInputHw_ReleaseData(&vipp_id, pstFrameInfo);
	}
    return ret;
}

void * VideoInputHw_CapThread(void *pThreadData)
{
	int ret = -1;
	int status;
	viChnManager *pManager = (viChnManager *)pThreadData;
	int vipp_id = pManager->vipp_dev_id;
	VIDEO_FRAME_INFO_S	pstFrameInfo;
    VI_ATTR_S attr;
    int num_buf= 0;
    int nMilliSec = 2000;   //unit:ms; default == 5000; icekirin fix me
    if(pManager != gpVIDevManager->gpVippManager[vipp_id])
    {
        aloge("fatal error! vipp[%d] viChnManager is not match[%p!=%p], check code!", vipp_id, pManager, gpVIDevManager->gpVippManager[vipp_id]);
    }
    videoInputHw_GetChnAttr(vipp_id, &attr);
    num_buf = attr.nbufs;

    if(attr.fps > VI_HIGH_FRAMERATE_STANDARD)
    {
        //alogd("high frame rate[%d], use burst policy!", attr.fps);
        //nMilliSec = 0;
        //alogw("Be careful! high frame rate[%d], but don't use burst policy!", attr.fps);
    }
    int i;
    int iDropFrameNum = attr.drop_frame_num;

	prctl(PR_SET_NAME, "VICaptureThread", 0, 0, 0);
	printf("loop VideoInputHw_CapThread vipp_id = %d, buf_num=%d.\r\n", vipp_id, num_buf);
    while ( 1 ) {
		videoInputHw_searchVippStatus(vipp_id, &status);
		if (0 == status) {
            while (TRUE) {
                if (list_empty(&gpVIDevManager->gpVippManager[vipp_id]->mChnList)) {
                    break;
                }
                usleep(10000);
                int cnt = 0;
                struct list_head *pList;
                list_for_each(pList, &gpVIDevManager->gpVippManager[vipp_id]->mChnList)
                {
                    cnt++;
                }            
                aloge("fatal error! Virvi Com not exit, vipp[%d], chnNum[%d]!!! wait !!!", vipp_id, cnt);
            }

            for (i = 0; i < num_buf; i++) {
                if (0 != gpVIDevManager->gpVippManager[vipp_id]->refs[num_buf]) {
                    alogd("fatal error! Virvi Com not return all yuv frame !!!, frame id(%d)", num_buf);
                    videoInputHw_ReleaseData(&vipp_id,
                        &gpVIDevManager->gpVippManager[vipp_id]->VideoFrameInfo[num_buf]);
                    gpVIDevManager->gpVippManager[vipp_id]->refs[num_buf] = 0;
                    memset(&gpVIDevManager->gpVippManager[vipp_id]->VideoFrameInfo[num_buf], 0, sizeof(pstFrameInfo));
                }
            }
			break ;
		}
		memset(&pstFrameInfo, 0x0, sizeof(pstFrameInfo));

        // gpVIDevManager->gpVippManager[ViVipp]->mProcessStep = 2;
		if (0 == videoInputHw_GetData(&vipp_id, &pstFrameInfo, nMilliSec)) { /* success : get yuv data */
            pManager->mVippTimeoutCnt = 0;
			// printf("addr = %p, vipp_id = %d.\r\n", pstFrameInfo.VFrame.mpVirAddr[0], vipp_id);
			/*printf("VideoInputHw_CapThread:%d,%d,%d,%d;%d,%d",
				pstFrameInfo.VFrame.mOffsetTop,
				pstFrameInfo.VFrame.mOffsetLeft,
				pstFrameInfo.VFrame.mOffsetRight,
				pstFrameInfo.VFrame.mOffsetBottom,
				pstFrameInfo.VFrame.mWidth,
				pstFrameInfo.VFrame.mHeight);*/
            if (iDropFrameNum > 0) {
                alogd("should drop %d frames, now still has %d frames should be droped",
                    attr.drop_frame_num, iDropFrameNum);
                iDropFrameNum--;
                videoInputHw_ReleaseData(&vipp_id, &pstFrameInfo);
                continue;
            }

			VI_CHN_MAP_S *pEntry;
			pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);// Fix me mID
			/*
			1. normal video have this mID , it from videoX buffer.
            2. stabilization video no this mID
            */
			if (0 == gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo.mId]) {
				if (list_empty(&gpVIDevManager->gpVippManager[vipp_id]->mChnList)) {
					alogw("VIPP[%d], No Virvi Component, drop this one yuv data.\r\n", vipp_id);
					videoInputHw_ReleaseData(&vipp_id, &pstFrameInfo);
					pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);
					continue ;
				}
                pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock);
                gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo.mId]++;
                pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock);
				list_for_each_entry(pEntry, &gpVIDevManager->gpVippManager[vipp_id]->mChnList, mList) {
                    memcpy(&gpVIDevManager->gpVippManager[vipp_id]->VideoFrameInfo[pstFrameInfo.mId], &pstFrameInfo, sizeof(pstFrameInfo));
					COMP_BUFFERHEADERTYPE bufferHeader;
		            bufferHeader.nInputPortIndex = VI_CHN_PORT_INDEX_CAP_IN; // VI_CHN_PORT_INDEX_CAP_IN;
		            bufferHeader.pOutputPortPrivate = &pstFrameInfo;
					// alogv("VideoInputHw_CapThread, %p.\r\n", pstFrameInfo.VFrame.mpVirAddr[0]);
					pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock);
					gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo.mId]++;
                    pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock);
		            ret = pEntry->mViComp->EmptyThisBuffer(pEntry->mViComp, &bufferHeader);
					if (-1 == ret) {
                        pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock);
						gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo.mId]--;
                        pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mRefsLock);
					}

				}
//                if(0 == gpVippManager[vipp_id]->refs[pstFrameInfo.mId])
//                {
//                    aloge("fatal error, call virvi component emptybuffer fail, buf id = %d.\r\n",
//                        gpVippManager[vipp_id]->refs[pstFrameInfo.mId]);
//                    videoInputHw_ReleaseData(&vipp_id, &pstFrameInfo);
//                }
                videoInputHw_RefsReduceAndRleaseData(vipp_id, &pstFrameInfo);
			} else {
				alogw("fatal error, buf not return, refs id = %d, y_addr=%p, drop this yuv data, buffer index=%d .\r\n",
                    gpVIDevManager->gpVippManager[vipp_id]->refs[pstFrameInfo.mId],
                    pstFrameInfo.VFrame.mpVirAddr[0], pstFrameInfo.mId);
				videoInputHw_ReleaseData(&vipp_id, &pstFrameInfo);
			}
			pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mLock);
		} else { /* failed : get yuv data. & drop yuv frame. */
            pthread_mutex_lock(&gpVIDevManager->gpVippManager[vipp_id]->mFrameListLock);
            int num = 0;
            VippFrame *pEntry;
            list_for_each_entry(pEntry, &gpVIDevManager->gpVippManager[vipp_id]->mReadyFrameList, mList)
            {
                if(pEntry->mVipp != vipp_id)
                {
                    aloge("fatal error! vipp[%d]!=[%d], check code!", pEntry->mVipp, vipp_id);
                }
                num++;
                alogw("vipp[%d] get frame fail! frameBufId[%d] is not release", pEntry->mVipp, pEntry->mFrameBufId);
            }
            if(num > 0)
            {
                alogw("vipp[%d] get frame fail! [%d]frames are not release", vipp_id, num);
            }
            else
            {
                aloge("fatal error! vipp[%d] get frame fail, but all frames are release!", vipp_id);
            }
            pthread_mutex_unlock(&gpVIDevManager->gpVippManager[vipp_id]->mFrameListLock);
            //usleep(40000);
            pManager->mVippTimeoutCnt++;
            if (pManager->mVippTimeoutCnt%5 == 0)
            {
                aloge("vipp[%d]: timeout for [%d]s when vipp GetFrame!", vipp_id, pManager->mVippTimeoutCnt*nMilliSec/1000);
                if(pManager->mMppCallback)
                {
                    MPP_CHN_S stChn = {MOD_ID_VIU, pManager->vipp_dev_id, MM_INVALID_CHN};
                    pManager->mMppCallback(
                        pManager->pAppData,
                        &stChn,
                        MPP_EVENT_VI_TIMEOUT,
                        NULL);
                }
            }
			continue ;
		}
    }

	/* release all cap buffer ? */
/*
	for ()
		videoInputHw_ReleaseData(&vipp_id, &pstFrameInfo);
*/
    // printf("debug exit virvi thread.\r\n");fflush(NULL);sleep(2);
	return NULL;
}

