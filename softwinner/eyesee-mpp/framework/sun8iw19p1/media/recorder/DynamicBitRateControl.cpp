/*******************************************************************************
--                                                                            --
--                    CedarX Multimedia Framework                             --
--                                                                            --
--          the Multimedia Framework for Linux/Android System                 --
--                                                                            --
--       This software is confidential and proprietary and may be used        --
--        only as expressly authorized by a licensing agreement from          --
--                         Softwinner Products.                               --
--                                                                            --
--                   (C) COPYRIGHT 2011 SOFTWINNER PRODUCTS                   --
--                            ALL RIGHTS RESERVED                             --
--                                                                            --
--                 The entire notice above must be reproduced                 --
--                  on all copies and should not be removed.                  --
--                                                                            --
*******************************************************************************/
//#define LOG_NDEBUG 0
#define LOG_TAG "DynamicBitRateControl"
#include <utils/plat_log.h>

#include <string.h>
#include <errno.h>
#include <sys/prctl.h>
#include <vector>

#include <CDX_ErrorType.h>
#include "DynamicBitRateControl.h"

#include <media_common.h>
#include <ComponentCommon.h>
#include <mpi_venc.h>
#include <mpi_mux.h>

namespace EyeseeLinux {

ERRORTYPE setVideoEncodingBitRateToVENC_CHN_ATTR_S(VENC_CHN_ATTR_S *pChnAttr, int bitRate);

status_t DynamicBitRateControl::GetBufferState(EyeseeRecorder::BufferState &bufferState)
{
    status_t ret = NO_ERROR;
    Mutex::Autolock lock(mBufferStateLock);
    if(mbBufferStateValid)
    {
        bufferState = mBufferState;
        ret = NO_ERROR;
    }
    else
    {
        ret = UNKNOWN_ERROR;
    }
    return ret;
}

int DynamicBitRateControl::UpdateVEncBitRate(EyeseeRecorder *pRecCtx, int newBitRate, int *pOldBitRate)
{
    int ret = -1;
    VENC_CHN_ATTR_S attr;
    AW_MPI_VENC_GetChnAttr(pRecCtx->mVeChn, &attr);
    int oldBitRate = GetBitRateFromVENC_CHN_ATTR_S(&attr);
    if (pOldBitRate != NULL)
        *pOldBitRate = oldBitRate;

    if (oldBitRate != newBitRate)
    {
        //SetBitRateToVENC_CHN_ATTR_S(&attr, newBitRate);
        setVideoEncodingBitRateToVENC_CHN_ATTR_S(&attr, newBitRate);
        ret = AW_MPI_VENC_SetChnAttr(pRecCtx->mVeChn, &attr);
    }

    return ret;
}

void* DynamicBitRateControl::DynamicBitRateControlThread(void* pThreadData)
{
    DynamicBitRateControl   *pDBRC = (DynamicBitRateControl*)pThreadData;
    EyeseeRecorder *rec_ctx = pDBRC->mPriv;
    message_t msg;
    int ret = 0;
    int nDetectDuration = 500; //unit:ms
    while(1)
    {
PROCESS_MESSAGE:
        //get bitRate in CBR mode
        unsigned int nVideoBitRate = 0;
        if(rec_ctx->mVideoRCMode != rec_ctx->mVEncRcAttr.mRcMode)
        {
            aloge("fatal error! check rcMode[0x%x]!=[0x%x]", rec_ctx->mVideoRCMode, rec_ctx->mVEncRcAttr.mRcMode);
        }
        if(EyeseeRecorder::VideoRCMode_CBR==rec_ctx->mVideoRCMode)
        {
            if(PT_H264 == rec_ctx->mVEncRcAttr.mVEncType)
            {
                nVideoBitRate = rec_ctx->mVEncRcAttr.mAttrH264Cbr.mBitRate;
            }
            else if(PT_H265 == rec_ctx->mVEncRcAttr.mVEncType)
            {
                nVideoBitRate = rec_ctx->mVEncRcAttr.mAttrH265Cbr.mBitRate;
            }
            else if(PT_MJPEG == rec_ctx->mVEncRcAttr.mVEncType)
            {
                nVideoBitRate = rec_ctx->mVEncRcAttr.mAttrMjpegCbr.mBitRate;
            }
            else
            {
                aloge("fatal error! unsupport vencType[0x%x]", rec_ctx->mVEncRcAttr.mVEncType);
            }
        }
        if(!get_message(&pDBRC->mDBRCMsgQueue, &msg))
        {
            if (msg.command == Stop)
            {
                alogd("restore bitRate to [%d]kbps before exit DBRCThread", nVideoBitRate/1024);
                //ret = pDBRC->UpdateVEncBitRate(rec_ctx, nVideoBitRate, NULL);
//                if(0 != ret)
//                {
//                    aloge("fatal error! set bitRate fail[0x%x]", ret);
//                }
                // Kill thread
                goto _ExitThread;
            }
        }
        if(MEDIA_RECORDER_RECORDING==rec_ctx->mCurrentState && EyeseeRecorder::VideoRCMode_CBR==rec_ctx->mVideoRCMode)
        {
            CacheState  recRenderCacheState;
            ERRORTYPE   eError;
            eError = AW_MPI_MUX_GetCacheStatus(rec_ctx->mMuxGrp, &recRenderCacheState);
            if (SUCCESS == eError)
            {
                /*
                int oldLevel = pDBRC->mStepLevel;
                if(recRenderCacheState.mValidSizePercent >= 80)
                {
                    pDBRC->mStepLevel = DBRC_MAX_STEP_LEVEL;
                }
                else if(recRenderCacheState.mValidSizePercent >= 50)
                {
                    pDBRC->mStepLevel = 1;
                }
                else
                {
                    pDBRC->mStepLevel = 0;
                }

                if(oldLevel!=pDBRC->mStepLevel)
                {
                    int n = 1<<pDBRC->mStepLevel;
                    int newBitRate = nVideoBitRate/n;
                    int oldBitRate;
                    eError = pDBRC->UpdateVEncBitRate(rec_ctx, newBitRate, &oldBitRate);
                    if(0 == eError)
                    {
                        alogd("DBRC bitRate[%d]Mbit/s to [%d]kbit/s, validPercent[%d], stepLevel[%d]->[%d]",
                            oldBitRate/(1024*1024), newBitRate/1024,
                            recRenderCacheState.mValidSizePercent, oldLevel, pDBRC->mStepLevel);
                    }
                }
                */
                //alogd("muxer Buffer[%d],[%d]KB/[%d]KB", recRenderCacheState.mValidSizePercent, recRenderCacheState.mValidSize, recRenderCacheState.mTotalSize);
                pDBRC->mBufferStateLock.lock();
                pDBRC->mBufferState.mValidSizePercent = recRenderCacheState.mValidSizePercent;
                pDBRC->mBufferState.mValidSize = recRenderCacheState.mValidSize;
                pDBRC->mBufferState.mTotalSize = recRenderCacheState.mTotalSize;
                pDBRC->mbBufferStateValid = true;
                pDBRC->mBufferStateLock.unlock();
                //send message.
                rec_ctx->postEventFromNative(rec_ctx, MEDIA_RECORDER_EVENT_INFO, EyeseeRecorder::MEDIA_RECORDER_INFO_VENC_BUFFER_USAGE, recRenderCacheState.mValidSizePercent, NULL);
            }
            else if (ERR_MUX_SYS_NOTREADY == eError)
            {
                alogw("recRender not ready? %#x", ERR_MUX_SYS_NOTREADY);
            }
            else if (ERR_MUX_NOT_SUPPORT == eError)
            {
                alogv("recRender has not cacheManager?");
                CacheState  vbvCacheState;
                int oldLevel = pDBRC->mStepLevel;
                eError = AW_MPI_VENC_GetCacheState(rec_ctx->mVeChn, &vbvCacheState);
                if (SUCCESS == eError)
                {
                    /*
                    if (vbvCacheState.mValidSizePercent >= 50) {
                        pDBRC->mStepLevel = DBRC_MAX_STEP_LEVEL;
                    } else if (vbvCacheState.mValidSizePercent >= 30) {
                        pDBRC->mStepLevel = 1;
                    } else {
                        pDBRC->mStepLevel = 0;
                    }

                    if (oldLevel != pDBRC->mStepLevel)
                    {
                        int newBitRate = nVideoBitRate/(1<<pDBRC->mStepLevel);
                        eError = pDBRC->UpdateVEncBitRate(rec_ctx, newBitRate, NULL);
                        if (0 == eError)
                        {
                            alogd("DBRC bitRate: [%d]->[%d] kbit/s, validPercent[%d], stepLevel: [%d]->[%d]",
                                nVideoBitRate/1024, newBitRate/1024,
                                vbvCacheState.mValidSizePercent, oldLevel, pDBRC->mStepLevel);
                        }
                    }
                    */
                    //alogd("venc Buffer[%d],[%d]KB/[%d]KB", vbvCacheState.mValidSizePercent, vbvCacheState.mValidSize, vbvCacheState.mTotalSize);
                    pDBRC->mBufferStateLock.lock();
                    pDBRC->mBufferState.mValidSizePercent = vbvCacheState.mValidSizePercent;
                    pDBRC->mBufferState.mValidSize = vbvCacheState.mValidSize;
                    pDBRC->mBufferState.mTotalSize = vbvCacheState.mTotalSize;
                    pDBRC->mbBufferStateValid = true;
                    pDBRC->mBufferStateLock.unlock();
                    //send message.
                    rec_ctx->postEventFromNative(rec_ctx, MEDIA_RECORDER_EVENT_INFO, EyeseeRecorder::MEDIA_RECORDER_INFO_VENC_BUFFER_USAGE, vbvCacheState.mValidSizePercent, NULL);
                }
                else
                {
                    aloge("can NOT get vbv info, ret: [%#x]", eError);
                }
            }
            else
            {
                aloge("Query Mux CacheState, why retVal is %#x?!", eError);
            }
            TMessage_WaitQueueNotEmpty(&pDBRC->mDBRCMsgQueue, nDetectDuration);
        }
        else
        {
            TMessage_WaitQueueNotEmpty(&pDBRC->mDBRCMsgQueue, nDetectDuration);
        }
    }

_ExitThread:
    alogd("DynamicBitRateControlThread stopped");
    return (void*) CDX_OK;

}

DynamicBitRateControl::DynamicBitRateControl(EyeseeRecorder *pRecCtx):
    mPriv(pRecCtx),
    mStepLevel(0)
{
    int eError = CDX_OK;
    mbBufferStateValid = false;
    if(message_create(&mDBRCMsgQueue)<0)
    {
        aloge("message create fail!");
        eError = CDX_ERROR;
	}
    eError = pthread_create(&mDynamicBitRateControlThreadId, NULL, DynamicBitRateControlThread, this);
    if (eError!=0)
    {
        aloge("fatal error! create thread error!");
        eError = CDX_ERROR;
    }
}

DynamicBitRateControl::~DynamicBitRateControl()
{
    void *err;
    message_t   msg;
    msg.command = Stop;
    put_message(&mDBRCMsgQueue, &msg);
    // Wait for thread to exit so we can get the status into "error"
    pthread_join(mDynamicBitRateControlThreadId, (void**) &err);
    alogd("DynamicBitRateControlThread exit!");
    message_destroy(&mDBRCMsgQueue);
}

};

