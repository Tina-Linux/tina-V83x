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
#define LOG_TAG "RecSink"
#include <utils/plat_log.h>

//ref platform headers
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/prctl.h>
#include <math.h>

#include "plat_type.h"
#include "plat_errno.h"
#include "plat_defines.h"
#include "plat_math.h"
#include "cdx_list.h"

//media api headers to app
#include "mm_common.h"
#include "mm_comm_video.h"
#include "mm_comm_venc.h"
#include <mm_comm_mux.h>

//media internal common headers.
#include "media_common.h"
#include "mm_component.h"
#include "ComponentCommon.h"
#include <SystemBase.h>
#include "tmessage.h"
#include "tsemaphore.h"
#include <aenc_sw_lib.h>

#include "RecRenderSink.h"
#include <mp4_mux_lib.h>
#include <sa_config.h>
#include <dup2SeldomUsedFd.h>

//#include <ConfigOption.h>

#define NOTIFY_NEEDNEXTFD_IN_ADVANCE (10*1000)  //ms

extern ERRORTYPE RecSinkMuxerInit(RecSink *pSinkInfo);

static ERRORTYPE RecSinkIncreaseIdleRSPacketList(RecSink* pThiz)
{
    ERRORTYPE   eError = SUCCESS;
    RecSinkPacket   *pRSPacket;
    int i;
    for(i=0;i<RECSINK_MAX_PACKET_NUM;i++)
    {
        pRSPacket = (RecSinkPacket*)malloc(sizeof(RecSinkPacket));
        if(NULL == pRSPacket)
        {
            aloge("fatal error! malloc fail");
            eError = ERR_MUX_NOMEM;
            break;
        }
        list_add_tail(&pRSPacket->mList, &pThiz->mIdleRSPacketList);
    }
    return eError;
}

static ERRORTYPE RSSetRecSinkPacket(PARAM_IN RecSinkPacket *pDes, PARAM_IN RecSinkPacket *pSrc)
{
    pDes->mId           = pSrc->mId;
    pDes->mStreamType   = pSrc->mStreamType;
    pDes->mFlags        = pSrc->mFlags;
    pDes->mPts          = pSrc->mPts;
    pDes->mpData0       = pSrc->mpData0;
    pDes->mSize0        = pSrc->mSize0;
    pDes->mpData1       = pSrc->mpData1;
    pDes->mSize1        = pSrc->mSize1;
    pDes->mCurrQp       = pSrc->mCurrQp;
    pDes->mavQp         = pSrc->mavQp;
	pDes->mnGopIndex    = pSrc->mnGopIndex;
	pDes->mnFrameIndex  = pSrc->mnFrameIndex;
	pDes->mnTotalIndex  = pSrc->mnTotalIndex;
    pDes->mSourceType   = pSrc->mSourceType;
    pDes->mRefCnt       = 0;
    return SUCCESS;
}

void RecSinkResetSomeMembers(RecSink *pThiz)
{
    pThiz->mMuxerId = -1;
    pThiz->nMuxerMode = MUXER_MODE_MP4;
    //pThiz->nOutputFd = -1;
    pThiz->nFallocateLen = 0;
    pThiz->nCallbackOutFlag = FALSE;
    //pThiz->pOutputFile = NULL;
    pThiz->pWriter = NULL;
    pThiz->pMuxerCtx = NULL;
    pThiz->mbMuxerInit = FALSE;
    int j;
    for(j=0;j<MAX_TRACK_COUNT;j++)
    {
        pThiz->mbTrackInit[j] = FALSE;
        pThiz->mPrevPts[j] = -1;
        pThiz->mBasePts[j] = -1;
        pThiz->mInputPrevPts[j] = -1;
        pThiz->mOrigBasePts[j] = -1;
    }
    pThiz->mDuration = 0;
    pThiz->mDurationAudio = 0;
    pThiz->mDurationText = 0;
    pThiz->mLoopDuration = 0;
    pThiz->mPrevDuration = 0;
    pThiz->mPrevDurationAudio = 0;
    pThiz->mPrevDurationText = 0;
    pThiz->mCurFileEndTm = 0;
    pThiz->mFileSizeBytes = 0;
    pThiz->mVideoFrameCounter = 0;
    //pThiz->nSwitchFd = -1;
    pThiz->nSwitchFdFallocateSize = 0;
    //pThiz->mSwitchFdImpactFlag = 0;
    pThiz->reset_fd_flag = FALSE;
    pThiz->need_set_next_fd = FALSE;
    pThiz->rec_file = FILE_NORMAL;
    pThiz->mRecordMode = RECORDER_MODE_NONE;
    pThiz->mFileDurationPolicy = RecordFileDurationPolicy_MinDuration;
    pThiz->mFsWriteMode = FSWRITEMODE_SIMPLECACHE;
    pThiz->mFsSimpleCacheSize = 0;
    //pThiz->mStatus = COMP_StateLoaded;
    //pThiz->mImpactStartTm = -1;
    //pThiz->mImpactAgainTm = -1;
    //pThiz->mImpactPrevTm = -1;
    pThiz->mCurMaxFileDuration = -1;
    pThiz->mMaxFileDuration = -1;
    //pThiz->mImpactFileDuration = -1;
    pThiz->mNoInputPacketFlag = 0;
    pThiz->mpMediaInf = NULL;
    pThiz->mpVencExtraData = NULL;
    pThiz->mpCallbackWriter = NULL;
    pThiz->mbSdCardState = TRUE;
    pThiz->mpCallbacks = NULL;
    pThiz->mpAppData = NULL;
    pThiz->mPrefetchFlag = FALSE; 
    pThiz->mPrefetchFlagAudio = FALSE;
    pThiz->mbShutDownNowFlag = FALSE;

    
    pThiz->bNeedSw = FALSE;
    pThiz->bNeedSwAudio = FALSE; 
    pThiz->bTimeMeetAudio = FALSE;
    pThiz->mVideoFrmCntWriteMore = 0;
    pThiz->mVideoPtsWriteMoreSt = -1;
}

static RecSinkPacket* RecSinkGetRSPacket_l(RecSink *pRecSink)
{
    RecSinkPacket   *pRSPacket = NULL;
    if(!list_empty(&pRecSink->mValidRSPacketList))
    {
        pRSPacket = list_first_entry(&pRecSink->mValidRSPacketList, RecSinkPacket, mList);
        list_del(&pRSPacket->mList);
    }
    return pRSPacket;
}

static RecSinkPacket* RecSinkGetRSPacket(RecSink *pRecSink)
{
    RecSinkPacket   *pRSPacket = NULL;
    pthread_mutex_lock(&pRecSink->mRSPacketListMutex);
    pRSPacket = RecSinkGetRSPacket_l(pRecSink);
    pthread_mutex_unlock(&pRecSink->mRSPacketListMutex);
    return pRSPacket;
}

static void RecSinkMovePrefetchRSPackets(RecSink *pRecSink)
{
    pthread_mutex_lock(&pRecSink->mRSPacketListMutex);
    list_splice_init(&pRecSink->mPrefetchRSPacketList, &pRecSink->mValidRSPacketList);
    pthread_mutex_unlock(&pRecSink->mRSPacketListMutex);
}

/*******************************************************************************
Function name: RecSinkWriteRSPacket
Description: 
    AVPacket,
    mbTrackInit[]
    mBasePts[]
    mPrevPts[]
    mDuration
    mDurationAudio
    mVideoFrameCounter
Parameters: 
    
Return: 
    
Time: 2014/2/18
*******************************************************************************/
static ERRORTYPE RecSinkWriteRSPacket(RecSink *pRecSink, RecSinkPacket *pSrcPkt)
{
    int ret;
    AVPacket packet;
    AVPacket *pDesPkt = &packet;
    memset(pDesPkt, 0, sizeof(AVPacket));
	pDesPkt->dts = -1;
    pDesPkt->data0 = pSrcPkt->mpData0;
    pDesPkt->size0 = pSrcPkt->mSize0;
    pDesPkt->data1 = pSrcPkt->mpData1;
    pDesPkt->size1 = pSrcPkt->mSize1;
    pDesPkt->stream_index = (int)pSrcPkt->mStreamType;
    pDesPkt->flags = pSrcPkt->mFlags;
    pDesPkt->duration = -1;
    pDesPkt->pos = -1;
    pDesPkt->CurrQp = pSrcPkt->mCurrQp;
    pDesPkt->avQp = pSrcPkt->mavQp;
	pDesPkt->nGopIndex = pSrcPkt->mnGopIndex;
	pDesPkt->nFrameIndex = pSrcPkt->mnFrameIndex;
	pDesPkt->nTotalIndex = pSrcPkt->mnTotalIndex;
    if (!pRecSink->mbTrackInit[pDesPkt->stream_index])
	{
		pRecSink->mbTrackInit[pDesPkt->stream_index] = TRUE;
        pRecSink->mBasePts[pDesPkt->stream_index] = pSrcPkt->mPts;
        alogd("streamIdx[%d] BasePts[%lld]us!", pDesPkt->stream_index, pSrcPkt->mPts);
        if(-1 == pRecSink->mOrigBasePts[pDesPkt->stream_index])
        {
            pRecSink->mOrigBasePts[pDesPkt->stream_index] = pSrcPkt->mPts;
            alogd("streamIdx[%d] OrigBasePts[%lld]us!", pDesPkt->stream_index, pSrcPkt->mPts);
        }
        //if(pRecSink->mInputPrevPts[pDesPkt->stream_index] != -1)
        //{
        //    pDesPkt->duration = (pSrcPkt->mPts - pRecSink->mInputPrevPts[pDesPkt->stream_index])/1000;
        //    if(pDesPkt->duration <= 0)
        //    {
        //        aloge("fatal error! stream[%d], curPts[%lld]us<=prevPts[%lld]us!", pDesPkt->stream_index, pSrcPkt->mPts, pRecSink->mInputPrevPts[pDesPkt->stream_index]);
        //    }
        //}
        //if(pDesPkt->duration < 0)
        //{
        //    if(pSrcPkt->mStreamType == CODEC_TYPE_VIDEO)
        //    {
        //        pDesPkt->duration = 1000*1000 / pRecSink->mpMediaInf->uVideoFrmRate;
        //    }
        //    else if (pSrcPkt->mStreamType == CODEC_TYPE_AUDIO)
        //    {
        //        pDesPkt->duration = MAXDECODESAMPLE*1000 / pRecSink->mpMediaInf->sample_rate;
        //    }
        //    else if (pSrcPkt->mStreamType == CODEC_TYPE_TEXT)
        //    {
        //        pDesPkt->duration = 1000;
        //    }
        //    alogd("first packet of stream[%d], duration[%d]ms!", pSrcPkt->mStreamType, pDesPkt->duration);
        //}
	}
//    int64_t nBasePts;
//    if(pRecSink->mRecordMode & RECORDER_MODE_VIDEO)
//    {
//        if(pRecSink->mBasePts[CODEC_TYPE_VIDEO] >= 0)
//        {
//            nBasePts = pRecSink->mBasePts[CODEC_TYPE_VIDEO];
//        }
//        else
//        {
//            alogw("fatal error! videoBasePts[%lld]<0, cur stream_index[%d], check code!", pRecSink->mBasePts[CODEC_TYPE_VIDEO], pDesPkt->stream_index);
//            nBasePts = pRecSink->mBasePts[pDesPkt->stream_index];
//        }
//    }
//    else
//    {
//        nBasePts = pRecSink->mBasePts[pDesPkt->stream_index];
//    }
//    pDesPkt->pts = pSrcPkt->mPts - nBasePts;
//    if(pDesPkt->pts < 0)
//    {
//        alogw("Be careful! streamIdx[%d] pts[%lld]<0, [%lld][%lld][%lld], change to 0, check code!", 
//            pDesPkt->stream_index, pDesPkt->pts, pSrcPkt->mPts, pRecSink->mBasePts[pDesPkt->stream_index], nBasePts);
//        pDesPkt->pts = 0;
//    }
    pDesPkt->pts = pSrcPkt->mPts - pRecSink->mBasePts[pDesPkt->stream_index];
    //if(pRecSink->mPrevPts[pDesPkt->stream_index] >= 0)
    //{
    //    pDesPkt->duration = (int)((pDesPkt->pts - pRecSink->mPrevPts[pDesPkt->stream_index])/1000);
    //}
    pRecSink->mPrevPts[pDesPkt->stream_index] = pDesPkt->pts;
    pRecSink->mInputPrevPts[pDesPkt->stream_index] = pSrcPkt->mPts;

    if(pSrcPkt->mStreamType == CODEC_TYPE_VIDEO)   //if base on videoBasePts, then video duration is accurate.
    {
        //pRecSink->mDuration += pDesPkt->duration;
        //pRecSink->mLoopDuration += pDesPkt->duration;
        pRecSink->mDuration = lround((double)pDesPkt->pts/1000 + 1000*1000/pRecSink->mpMediaInf->uVideoFrmRate);
        pDesPkt->duration = pRecSink->mDuration - pRecSink->mPrevDuration;
        pRecSink->mPrevDuration = pRecSink->mDuration;
        pRecSink->mLoopDuration = llround((double)(pSrcPkt->mPts - pRecSink->mOrigBasePts[pDesPkt->stream_index])/1000 + 
                                                1000*1000/pRecSink->mpMediaInf->uVideoFrmRate);
        pRecSink->mVideoFrameCounter++;
        pRecSink->mFileSizeBytes += (int64_t)(pDesPkt->size0 + pDesPkt->size1 + 32);
    }
    else if (pSrcPkt->mStreamType == CODEC_TYPE_AUDIO)   //if base on videoBasePts, audio duration is not accurate.
    {
        //pRecSink->mDurationAudio += pDesPkt->duration;
        pRecSink->mDurationAudio = pDesPkt->pts/1000 + MAXDECODESAMPLE*1000 / pRecSink->mpMediaInf->sample_rate;
        pDesPkt->duration = pRecSink->mDurationAudio - pRecSink->mPrevDurationAudio;
        pRecSink->mPrevDurationAudio = pRecSink->mDurationAudio;
        pRecSink->mFileSizeBytes += (int64_t)(pDesPkt->size0 + pDesPkt->size1);
        pRecSink->mLoopDurationAudio = (pSrcPkt->mPts - pRecSink->mOrigBasePts[pDesPkt->stream_index])/1000 +
                                            MAXDECODESAMPLE*1000 / pRecSink->mpMediaInf->sample_rate;
        if(0 == (pRecSink->mRecordMode & RECORDER_MODE_VIDEO))
        {
            pRecSink->mDuration = pRecSink->mDurationAudio;
            pRecSink->mPrevDuration = pRecSink->mPrevDurationAudio;
            pRecSink->mLoopDuration = pRecSink->mLoopDurationAudio;
        }
    }
	else if (pSrcPkt->mStreamType == CODEC_TYPE_TEXT)
	{
        //pRecSink->mDurationText += pDesPkt->duration;
        pRecSink->mDurationText = pDesPkt->pts/1000 + 1000;
        pDesPkt->duration = pRecSink->mDurationText - pRecSink->mPrevDurationText;
        pRecSink->mPrevDurationText = pRecSink->mDurationText;
        pRecSink->mFileSizeBytes += (int64_t)(pDesPkt->size0 + pDesPkt->size1);
    }

    if(pRecSink->mbSdCardState || pRecSink->nCallbackOutFlag)
    {
		if (pRecSink->pWriter != NULL) 
        {
            ret = pRecSink->pWriter->MuxerWritePacket(pRecSink->pMuxerCtx, pDesPkt);
            if (ret != 0)
            {
                aloge("fatal error! muxerId[%d]muxerWritePacket FAILED", pRecSink->mMuxerId);
                pRecSink->mpCallbacks->EventHandler(
                        pRecSink, 
                        pRecSink->mpAppData,
                        COMP_EventError, 
                        ERR_MUX_NOT_PERM, 
                        COMP_StateInvalid, 
                        NULL);
                pRecSink->mStatus = COMP_StateInvalid;   //OMX_StateIdle;
            }
		} 
        else 
        {
			alogw("pWriter=NULL, muxer not initialize");
		}
    }
    else
    {
        alogw("Sdcard is not exist, skip MuxerWritePacket()!");
    }
    return SUCCESS;
}

static ERRORTYPE RecSinkReleaseRSPacket_l(RecSink *pRecSink, RecSinkPacket *pRSPacket)
{
    ERRORTYPE omxRet;
    omxRet = pRecSink->mpCallbacks->EmptyBufferDone(pRecSink, pRecSink->mpAppData, pRSPacket);
    if(omxRet != SUCCESS)
    {
        aloge("fatal error! emptyBufferDone fail[0x%x]", omxRet);
    }
    list_add_tail(&pRSPacket->mList, &pRecSink->mIdleRSPacketList);
    return omxRet;
}

static ERRORTYPE RecSinkReleaseRSPacket(RecSink *pRecSink, RecSinkPacket *pRSPacket)
{
    ERRORTYPE omxRet;
    omxRet = pRecSink->mpCallbacks->EmptyBufferDone(pRecSink, pRecSink->mpAppData, pRSPacket);
    if(omxRet != SUCCESS)
    {
        aloge("fatal error! emptyBufferDone fail[0x%x]", omxRet);
    }
    pthread_mutex_lock(&pRecSink->mRSPacketListMutex);
    list_add_tail(&pRSPacket->mList, &pRecSink->mIdleRSPacketList);
    pthread_mutex_unlock(&pRecSink->mRSPacketListMutex);
    return omxRet;
}

static ERRORTYPE RecSinkDrainAllRSPackets(RecSink *pRecSink, BOOL bIgnore)
{
    int cnt = 0;
    int sendCnt = 0;
    RecSinkPacket   *pEntry, *pTmp;
    RecSinkPacket   *pRSPacket;
    pthread_mutex_lock(&pRecSink->mRSPacketListMutex);
    alogd("muxerId[%d]muxerMode[%d] begin to drain packets, bIgnore[%d]", pRecSink->mMuxerId, pRecSink->nMuxerMode, bIgnore);
    while(1)
    {
        if(bIgnore)
        {
            break;
        }
        if(!pRecSink->mbSdCardState)
        {
            alogd("sdcard is pull out");
            break;
        }
        pRSPacket = RecSinkGetRSPacket_l(pRecSink);
        if(pRSPacket)
        {
            cnt++;
            if(!pRecSink->mbShutDownNowFlag)
            {
                if(pRecSink->mbMuxerInit == FALSE)
                {
                    BOOL bGrant = TRUE;
                    if(pRecSink->mRecordMode & RECORDER_MODE_VIDEO)
                    {
                        if(!(pRSPacket->mFlags & AVPACKET_FLAG_KEYFRAME))
                        {
                            bGrant = FALSE;
                        }
                    }
                    if(bGrant)
                    {
                        pthread_mutex_lock(&pRecSink->mutex_reset_writer_lock);
                        if(pRecSink->nCallbackOutFlag==TRUE || (pRecSink->nOutputFd>=0 || pRecSink->mPath!=NULL) || pRecSink->reset_fd_flag==TRUE)
                        {
                            if(RecSinkMuxerInit(pRecSink) != SUCCESS) 
                            {
                                aloge("fatal error! muxerId[%d][%p]ValidMuxerInit Error!", pRecSink->mMuxerId, pRecSink);
                                pRecSink->mStatus = COMP_StateInvalid;   //OMX_StateIdle;
                                pthread_mutex_unlock(&pRecSink->mutex_reset_writer_lock);
                                RecSinkReleaseRSPacket_l(pRecSink, pRSPacket);
                                break;
                            }
                        }
                        pthread_mutex_unlock(&pRecSink->mutex_reset_writer_lock);
                    }
                }
                if(pRecSink->mbMuxerInit)
                {
                    RecSinkWriteRSPacket(pRecSink, pRSPacket);
                }
            }
            //release RSPacket
            RecSinkReleaseRSPacket_l(pRecSink, pRSPacket);
        }
        else
        {
            break;
        }
    }
    if(!list_empty(&pRecSink->mValidRSPacketList))
    {
        list_for_each_entry_safe(pEntry, pTmp, &pRecSink->mValidRSPacketList, mList)
        {
            pRecSink->mpCallbacks->EmptyBufferDone(pRecSink, pRecSink->mpAppData, pEntry);
            list_move_tail(&pEntry->mList, &pRecSink->mIdleRSPacketList);
            cnt++;
            sendCnt++;
        }
        alogd("left [%d]packets to send out immediately", sendCnt);
    }
    pthread_mutex_unlock(&pRecSink->mRSPacketListMutex);
    alogd("muxerId[%d]muxerMode[%d] drain [%d]packets, ShutDownNow[%d]", pRecSink->mMuxerId, pRecSink->nMuxerMode, cnt, pRecSink->mbShutDownNowFlag);
    return SUCCESS;
}

ERRORTYPE RecSinkStreamCallback(void *hComp, int event)
{
    RecSink *pRecSink = (RecSink*)hComp;
    alogd("WriteDisk fail: muxerId[%d], muxerFileType[0x%x], fd[%d]", pRecSink->mMuxerId, pRecSink->nMuxerMode, pRecSink->nOutputFd);
    pRecSink->mpCallbacks->EventHandler(
                                pRecSink,
                                pRecSink->mpAppData, 
                                COMP_EventWriteDiskError, 
                                pRecSink->mMuxerId, 
                                0, 
                                NULL);
    return SUCCESS;
}

ERRORTYPE RecSinkMuxerInit(RecSink *pSinkInfo)
{
    int ret = 0;
    alogd("sinkInfo[%p]muxerId[%d],fd[%d], nCallbackOutFlag[%d]", pSinkInfo, pSinkInfo->mMuxerId, pSinkInfo->nOutputFd, pSinkInfo->nCallbackOutFlag);
    if(pSinkInfo->rec_file == FILE_NORMAL)
    {
        pSinkInfo->mCurMaxFileDuration = pSinkInfo->mMaxFileDuration;
        pSinkInfo->mCurFileEndTm += pSinkInfo->mCurMaxFileDuration;
    }
	else if (pSinkInfo->rec_file == FILE_IMPACT_RECDRDING)
	{
        aloge("fatal error! not care impact!");
		//pSinkInfo->mCurMaxFileDuration = pSinkInfo->mImpactFileDuration;
        //pSinkInfo->mCurFileEndTm = pSinkInfo->mLoopDuration + pSinkInfo->mCurMaxFileDuration;
	}
    else if (pSinkInfo->rec_file == FILE_NEED_SWITCH_TO_IMPACT)
	{
        aloge("fatal error! not care impact!");
//        alogd("rec file state FILE_NEED_SWITCH_TO_IMPACT, so directly impact file.");
//        pSinkInfo->rec_file = FILE_IMPACT_RECDRDING;
//		pSinkInfo->mCurMaxFileDuration = pSinkInfo->mImpactFileDuration;
//        pSinkInfo->mCurFileEndTm += pSinkInfo->mCurMaxFileDuration;
	}
    else
    {
        aloge("fatal error! wrong rec file state[%d]", pSinkInfo->rec_file);
        pSinkInfo->mCurMaxFileDuration = pSinkInfo->mMaxFileDuration;
    }
    if(FALSE == pSinkInfo->nCallbackOutFlag)
    {
        if(pSinkInfo->nOutputFd<0 && NULL==pSinkInfo->mPath)
        {
            alogw("Be careful! nOutputFd<0 when call RecRender MuxerInit, have last chance to get fd!");
            if(pSinkInfo->reset_fd_flag)
            {
                if(pSinkInfo->nSwitchFd >= 0)
                {
                    if(pSinkInfo->mSwitchFilePath)
                    {
                        aloge("fatal error! fd[%d] and path[%s] only use one, check code!", pSinkInfo->nSwitchFd, pSinkInfo->mSwitchFilePath);
                    }
                    pSinkInfo->nOutputFd = pSinkInfo->nSwitchFd;
                    pSinkInfo->nSwitchFd = -1;
                    pSinkInfo->nFallocateLen = pSinkInfo->nSwitchFdFallocateSize;
                    pSinkInfo->nSwitchFdFallocateSize = 0;
//                    if(pSinkInfo->mSwitchFdImpactFlag)
//                    {
//                        alogd("to write impact fd!");
//                        pSinkInfo->mSwitchFdImpactFlag = 0;
//                    }
                    pSinkInfo->reset_fd_flag = FALSE;
                }
                else if(pSinkInfo->mSwitchFilePath)
                {
                    if(pSinkInfo->mPath)
                    {
                        aloge("fatal error! mPath[%s] should be null", pSinkInfo->mPath);
                        free(pSinkInfo->mPath);
                        pSinkInfo->mPath = NULL;
                    }
                    pSinkInfo->mPath = strdup(pSinkInfo->mSwitchFilePath);
                    free(pSinkInfo->mSwitchFilePath);
                    pSinkInfo->mSwitchFilePath = NULL;
                    pSinkInfo->nFallocateLen = pSinkInfo->nSwitchFdFallocateSize;
                    pSinkInfo->nSwitchFdFallocateSize = 0;
//                    if(pSinkInfo->mSwitchFdImpactFlag)
//                    {
//                        alogd("to write impact fd!");
//                        pSinkInfo->mSwitchFdImpactFlag = 0;
//                    }
                    pSinkInfo->reset_fd_flag = FALSE;
                }
                else
                {
                    alogd("pSinkInfo->nSwitchFd[%d] < 0", pSinkInfo->nSwitchFd);
                }
            }
            else
            {
                aloge("fatal error! reset_fd flag != TRUE");
            }
        }
        if(pSinkInfo->nOutputFd>=0)
        {
            if(pSinkInfo->nOutputFd == 0)
            {
                alogd("Be careful! fd == 0");
            }
//            pSinkInfo->pOutputFile = fdopen(pSinkInfo->nOutputFd, "wb");
//            if(pSinkInfo->pOutputFile==NULL) 
//            {
//                aloge("fatal error! get file fd failed, test");
//            }
        }
        else if(pSinkInfo->mPath!=NULL)
        {
        }
        else
        {
            alogd("RecSink[%p] is not set nOutputFd[%d]", pSinkInfo, pSinkInfo->nOutputFd);
//            if(pSinkInfo->pOutputFile!=NULL)
//            {
//                aloge("fatal error! RecSink[%p] pOutputFile[%p] is not NULL!", pSinkInfo, pSinkInfo->pOutputFile);
//                pSinkInfo->pOutputFile = NULL;
//            }
        }
    }
    pSinkInfo->mDuration = 0;
    pSinkInfo->mDurationAudio = 0;
    pSinkInfo->mDurationText = 0;
    pSinkInfo->mPrevDuration = 0;
    pSinkInfo->mPrevDurationAudio = 0;
    pSinkInfo->mPrevDurationText = 0;
    pSinkInfo->mFileSizeBytes = 0;
    int i;
    for(i=0;i<MAX_TRACK_COUNT;i++)
    {
        pSinkInfo->mbTrackInit[i] = FALSE;
        pSinkInfo->mPrevPts[i] = -1;
        pSinkInfo->mBasePts[i] = -1;
    }
    pSinkInfo->mVideoFrameCounter = 0;
	if (pSinkInfo->pWriter == NULL) 
    {
        pSinkInfo->pWriter = cedarx_record_writer_create(pSinkInfo->nMuxerMode);
        if (NULL == pSinkInfo->pWriter)
        {
            aloge("fatal error! cedarx_record_writer_create failed");
            return ERR_MUX_NOMEM;
        }
        pSinkInfo->pMuxerCtx = pSinkInfo->pWriter->MuxerOpen((int*)&ret);
        if (ret != SUCCESS) 
        {
            aloge("fatal error! [%p]MuxerOpen failed", pSinkInfo);
            goto MUXER_OPEN_ERR;
        }
        else 
        {
            alogv("MuxerOpen OK");
        }
        //TODO: now, write fd and callback mode are mutex. In the future, we will improve it if necessary.
        if (FALSE == pSinkInfo->nCallbackOutFlag) 
        {
            if(pSinkInfo->nOutputFd>=0)
            {
                pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SETFALLOCATELEN, (unsigned int)pSinkInfo->nFallocateLen, NULL);
                if (pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SETCACHEFD2, (unsigned int)pSinkInfo->nOutputFd, NULL) != 0)
                {
                    aloge("fatal error! SETCACHEFD2 failed");
                    goto SETCACHEFD_ERR;
                }
            }
            else if(pSinkInfo->mPath!=NULL)
            {
                pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SETFALLOCATELEN, (unsigned int)pSinkInfo->nFallocateLen, NULL);
                if (pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SETCACHEFD, 0, (void*)pSinkInfo->mPath) != 0)
                {
                    aloge("fatal error! SETCACHEFD failed");
                    goto SETCACHEFD_ERR;
                }
            }
            //alogd("use fsWriteMode[%d], simpleCacheSize[%d]KB", pSinkInfo->mFsWriteMode, pSinkInfo->mFsSimpleCacheSize/1024);
            alogd("(f:%s, l:%d) FileDurationPolicy[0x%x], use fsWriteMode[%d], simpleCacheSize[%ld]KB", __FUNCTION__, __LINE__, pSinkInfo->mFileDurationPolicy, pSinkInfo->mFsWriteMode, pSinkInfo->mFsSimpleCacheSize/1024);
            if(FSWRITEMODE_CACHETHREAD == pSinkInfo->mFsWriteMode)
            {
                aloge("fatal error! not use cacheThread mode now!");
            }
            pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SET_FS_WRITE_MODE, pSinkInfo->mFsWriteMode, NULL);
            pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SET_FS_SIMPLE_CACHE_SIZE, pSinkInfo->mFsSimpleCacheSize, NULL);

            cdx_write_callback_t  callback;
            callback.hComp = pSinkInfo;
            callback.cb = RecSinkStreamCallback;
            pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SET_STREAM_CALLBACK, 0, (void*)&callback);
        }
        else 
        {
            //pRecRenderData->writer->MuxerIoctrl(pRecRenderData->p_muxer_ctx, SETOUTURL, (unsigned int)pRecRenderData->url);
            if (pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, REGISTER_WRITE_CALLBACK, 0, (void*)pSinkInfo->mpCallbackWriter) != 0)
            {
                aloge("fatal error! REGISTER_WRITE_CALLBACK failed");
                goto SETCACHEFD_ERR;
            }
        }
	}
    else
    {
        aloge("fatal error! sinkInfo[%p]muxerId[%d] pWriter[%p] is not NULL!", pSinkInfo, pSinkInfo->mMuxerId, pSinkInfo->pWriter);
    }

	// mjpeg source form camera
//	if(pRecRenderData->is_compress_source == 1) {  /* gushiming compressed source */
//		pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SET_VIDEO_CODEC_ID, CODEC_ID_MJPEG);
//	}

    pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SETAVPARA, 0, (void*)(pSinkInfo->mpMediaInf));

    if(pSinkInfo->mRecordMode & RECORDER_MODE_VIDEO)
    {
        if(pSinkInfo->mpVencExtraData && pSinkInfo->mpVencExtraData->nLength > 0)
        {
            pSinkInfo->pWriter->MuxerWriteExtraData(pSinkInfo->pMuxerCtx, (unsigned char *)pSinkInfo->mpVencExtraData->pBuffer, pSinkInfo->mpVencExtraData->nLength, 0);
        }
    }
    if(pSinkInfo->mRecordMode & RECORDER_MODE_AUDIO)
    {
        __extra_data_t  AudioExtraDataForMp4;
        MuxerGenerateAudioExtraData(&AudioExtraDataForMp4, pSinkInfo->mpMediaInf);
        pSinkInfo->pWriter->MuxerWriteExtraData(pSinkInfo->pMuxerCtx, AudioExtraDataForMp4.extra_data, AudioExtraDataForMp4.extra_data_len, 1);
    }
    if(pSinkInfo->mRecordMode & RECORDER_MODE_TEXT)
    {   // do nothing
        pSinkInfo->pWriter->MuxerWriteExtraData(pSinkInfo->pMuxerCtx, NULL, 0, 2);
    }


    ret = pSinkInfo->pWriter->MuxerWriteHeader(pSinkInfo->pMuxerCtx);
    if (ret != 0)
    {
        aloge("write header failed");
        goto SETCACHEFD_ERR;
    }
    else
    {
        alogv("write header ok");
    }
//    if(FALSE==pSinkInfo->reset_fd_flag)
//    {
//        aloge("fatal error! why reset fd flag = 0?");
//    }
//    pSinkInfo->reset_fd_flag = FALSE;
    pSinkInfo->mbMuxerInit = TRUE;
    return SUCCESS;

SETCACHEFD_ERR:
    pSinkInfo->pWriter->MuxerClose(pSinkInfo->pMuxerCtx);
MUXER_OPEN_ERR:
    cedarx_record_writer_destroy(pSinkInfo->pWriter);
    return ERR_MUX_ILLEGAL_PARAM;
}

/*******************************************************************************
Function name: RecSinkMuxerClose
Description: 
    1. now, one muxer can only support one method: fwrite or CallbackOut.
        In future, we will change it if necessary.
Parameters: 
    
Return: 
    
Time: 2014/7/12
*******************************************************************************/
ERRORTYPE RecSinkMuxerClose(RecSink *pSinkInfo, int clrFile)
{
    ERRORTYPE ret = SUCCESS;
	if (pSinkInfo->pWriter != NULL) 
    {
        alogw("avsync_muxer_close:%d-%d-%lld-%lld-%d",pSinkInfo->mDuration,pSinkInfo->mDurationAudio,
            pSinkInfo->mLoopDuration,pSinkInfo->mLoopDurationAudio,pSinkInfo->mpMediaInf->nWidth);
        
        if(FALSE == pSinkInfo->nCallbackOutFlag)
        {
            if(pSinkInfo->mbSdCardState)
            {
        		ret = pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SETTOTALTIME, pSinkInfo->mDuration, NULL);
                if(ret != 0)
                {
                    aloge("writer->MuxerIoctrl setTOTALTIME FAILED, ret: %d", ret);
                }
        		ret = pSinkInfo->pWriter->MuxerWriteTrailer(pSinkInfo->pMuxerCtx);
                if(ret!=0)
                {
                    aloge("muxerWriteTrailer ret:[%d]", ret);
                }
        		ret = pSinkInfo->pWriter->MuxerClose(pSinkInfo->pMuxerCtx);
                if (ret != 0)
                {
                    aloge("muxerClose failed, ret = %d", ret);
                }
                //fflush(pSinkInfo->pOutputFile);
                if (clrFile == 1) 
                {
                    aloge("fatal error! clrFile[%d]!=0", clrFile);
//                    #if (CDXCFG_FILE_SYSTEM==OPTION_FILE_SYSTEM_DIRECT_FATFS)
//                        fat_sync((int)pSinkInfo->pOutputFile);
//                        fat_lseek((int)pSinkInfo->pOutputFile, 0);
//                        fat_truncate((int)pSinkInfo->pOutputFile);
//                    #else
//                        fflush(pSinkInfo->pOutputFile);
//                        ftruncate(fileno(pSinkInfo->pOutputFile), 0);
//                        rewind(pSinkInfo->pOutputFile);
//                    #endif
                }
                else
                {
//                    alogd("before fflush");
//                    fflush(pSinkInfo->pOutputFile);
//                    alogd("before fsync");
//                    fsync(pSinkInfo->nOutputFd);
//                    alogd("after fsync");
                }
            }
            else
            {
                alogd("mbSdCardState[%d], sdcard is pull out", pSinkInfo->mbSdCardState);
                pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SETSDCARDSTATE, pSinkInfo->mbSdCardState, NULL);
                pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SETTOTALTIME, pSinkInfo->mDuration, NULL);
                pSinkInfo->pWriter->MuxerClose(pSinkInfo->pMuxerCtx);
                alogd("mbSdCardState[%d], sdcard is pull out, muxer close done!", pSinkInfo->mbSdCardState);
            }
            if(pSinkInfo->nOutputFd < 0 && NULL==pSinkInfo->mPath)
            {
                aloge("fatal error! sinkInfo[%p]->nOutputFd[%d]<0", pSinkInfo, pSinkInfo->nOutputFd);
            }
            if(pSinkInfo->nOutputFd>=0)
            {
                close(pSinkInfo->nOutputFd);
                pSinkInfo->nOutputFd = -1;
            }
            if(pSinkInfo->mPath)
            {
                free(pSinkInfo->mPath);
                pSinkInfo->mPath = NULL;
            }
            if(FSWRITEMODE_CACHETHREAD == pSinkInfo->mFsWriteMode)
            {
                aloge("fatal error! not use cacheThread mode now!");
            }
            pSinkInfo->mpCallbacks->EventHandler(pSinkInfo, pSinkInfo->mpAppData, COMP_EventRecordDone, pSinkInfo->mMuxerId, 0, NULL);
        }
        else
        {
            pSinkInfo->pWriter->MuxerIoctrl(pSinkInfo->pMuxerCtx, SETTOTALTIME, pSinkInfo->mDuration, NULL);
    		pSinkInfo->pWriter->MuxerWriteTrailer(pSinkInfo->pMuxerCtx);
    		pSinkInfo->pWriter->MuxerClose(pSinkInfo->pMuxerCtx);
            //pSinkInfo->nCallbackOutFlag = FALSE;
            pSinkInfo->mpCallbacks->EventHandler(pSinkInfo, pSinkInfo->mpAppData, COMP_EventRecordDone, pSinkInfo->mMuxerId, 0, NULL);
        }
		cedarx_record_writer_destroy(pSinkInfo->pWriter);
		pSinkInfo->pWriter = NULL;
		pSinkInfo->pMuxerCtx = NULL;
	}
    pSinkInfo->mbMuxerInit = FALSE;
    int videoStreamIndex = (int)CODEC_TYPE_VIDEO;
    int audioStreamIndex = (int)CODEC_TYPE_AUDIO;
    alogd("TOTAL duration: %d(ms)", pSinkInfo->mDuration);
    alogd("TOTAL duration audio: %d(ms)", pSinkInfo->mDurationAudio);
    alogd("TOTAL duration text: %d(ms)", pSinkInfo->mDurationText);
    alogd("LOOP duration: %lld(ms), lastPts v[%lld]-a[%lld]=[%lld]ms, curInputPts v[%lld]-a[%lld]=[%lld]ms",
        pSinkInfo->mLoopDuration,
        (pSinkInfo->mPrevPts[videoStreamIndex] + pSinkInfo->mBasePts[videoStreamIndex])/1000,
        (pSinkInfo->mPrevPts[audioStreamIndex] + pSinkInfo->mBasePts[audioStreamIndex])/1000,
        (pSinkInfo->mPrevPts[videoStreamIndex] + pSinkInfo->mBasePts[videoStreamIndex] - (pSinkInfo->mPrevPts[audioStreamIndex] + pSinkInfo->mBasePts[audioStreamIndex]))/1000,
        pSinkInfo->mDebugInputPts[videoStreamIndex]/1000,
        pSinkInfo->mDebugInputPts[audioStreamIndex]/1000,
        (pSinkInfo->mDebugInputPts[videoStreamIndex] - pSinkInfo->mDebugInputPts[audioStreamIndex])/1000);
    alogd("MuxerId: %d, TOTAL file size: %lld(bytes)",pSinkInfo->mMuxerId, pSinkInfo->mFileSizeBytes);
    return SUCCESS;
}

BOOL RecSinkIfNeedSwitchFile(RecSink *pRecSink,int *need_switch_audio)
{
    BOOL bNeedSwitch = FALSE;
    if(TRUE == pRecSink->bNeedSw)
    {
        return pRecSink->bNeedSw;
    }
    else if(TRUE == pRecSink->bNeedSwAudio)
    {
        *need_switch_audio = 1;
        return FALSE;
    }
    
    if(pRecSink->mbMuxerInit /* && pRecRenderData->reset_fd_flag == TRUE */ && !pRecSink->nCallbackOutFlag) // no network
    {
        if(pRecSink->mCurMaxFileDuration > 0)   //user set max duration, then segment file base on duration
        {
            //if(pRecSink->mLoopDuration >= pRecSink->mCurFileEndTm)
            //{
            //    bNeedSwitch = TRUE;
            //}
            int nErrorInterval = 1000*1000/pRecSink->mpMediaInf->uVideoFrmRate/3;   //unit:ms
            if(pRecSink->mFileDurationPolicy == RecordFileDurationPolicy_MinDuration || 
                pRecSink->mFileDurationPolicy == RecordFileDurationPolicy_AccurateDuration)
            {
                if(pRecSink->mDuration + nErrorInterval >= pRecSink->mCurMaxFileDuration)
                {
                    bNeedSwitch = TRUE;
                    pRecSink->bNeedSw = TRUE;
                }
            }
            else if(pRecSink->mFileDurationPolicy == RecordFileDurationPolicy_AverageDuration)
            {
                if(pRecSink->mLoopDuration + nErrorInterval >= pRecSink->mCurFileEndTm)
                {
                    bNeedSwitch = TRUE;
                    pRecSink->bNeedSw = TRUE;
                }
            }
            else
            {
                aloge("(f:%s, l:%d) fatal error! unknown FileDurationPolicy[0x%x], check code!", __FUNCTION__, __LINE__, pRecSink->mFileDurationPolicy);
            }

            if(FALSE == bNeedSwitch && (pRecSink->mRecordMode & RECORDER_MODE_AUDIO))    // to check audio status
            { 
                int nErrorInterval = MAXDECODESAMPLE*1000 / pRecSink->mpMediaInf->sample_rate/3;   //unit:ms
                *need_switch_audio = 0;
                if(pRecSink->mFileDurationPolicy == RecordFileDurationPolicy_MinDuration || 
                    pRecSink->mFileDurationPolicy == RecordFileDurationPolicy_AccurateDuration)
                {
                    if(pRecSink->mDurationAudio + nErrorInterval >= pRecSink->mCurMaxFileDuration)
                    {
                        *need_switch_audio = 1;
                        pRecSink->bNeedSwAudio = TRUE;
                    }
                }
                else if(pRecSink->mFileDurationPolicy == RecordFileDurationPolicy_AverageDuration)
                {
                    if(pRecSink->mLoopDurationAudio + nErrorInterval >= pRecSink->mCurFileEndTm)
                    {
                        *need_switch_audio = 1;
                        pRecSink->bNeedSwAudio = TRUE;
                    }
                }
                else
                {
                    aloge("(f:%s, l:%d) fatal error! unknown FileDurationPolicy[0x%x], check code!", __FUNCTION__, __LINE__, pRecSink->mFileDurationPolicy);
                }
            } 
            
        }
        else if(pRecSink->mMaxFileSizeBytes > 0)   //user set max file size, then segment file base on fileSize.
        {
            if(pRecSink->mFileSizeBytes >= pRecSink->mMaxFileSizeBytes)
            {
                double fileSizeMB = (double)pRecSink->mFileSizeBytes/(1024*1024);
                alogv("fileSize[%lld]Bytes([%7.3lf]MB) >= max[%7.3lf]MB, rec_file[%d], need switch file", pRecSink->mFileSizeBytes, fileSizeMB, (double)pRecSink->mMaxFileSizeBytes/(1024*1024), pRecSink->rec_file);
                bNeedSwitch = TRUE;
                pRecSink->bNeedSw = TRUE;
            }
        }
        else if(pRecSink->rec_file == FILE_NEED_SWITCH_TO_IMPACT || pRecSink->rec_file == FILE_NEED_SWITCH_TO_NORMAL)
        {
            bNeedSwitch = TRUE;
            pRecSink->bNeedSw = TRUE;
        }
        else    //not switch file
        {
            bNeedSwitch = FALSE;
            pRecSink->bNeedSw = FALSE;
        }
    }
    else
    {
        bNeedSwitch = FALSE;
    }
    return bNeedSwitch;
}

BOOL RecSinkIfNeedRequestNextFd(RecSink *pRecSink)
{
    if(pRecSink->need_set_next_fd == FALSE)
    {
        return FALSE;
    }
    BOOL bNeedRequest = FALSE;
    if(pRecSink->mCurMaxFileDuration > 0)   //user set max duration, then segment file base on duration
    {
        //if(pRecSink->mLoopDuration + NOTIFY_NEEDNEXTFD_IN_ADVANCE >= pRecSink->mCurFileEndTm)
        //{
        //    bNeedRequest = TRUE;
        //}
        if(pRecSink->mFileDurationPolicy == RecordFileDurationPolicy_MinDuration ||
            pRecSink->mFileDurationPolicy == RecordFileDurationPolicy_AccurateDuration)
        {
            if((pRecSink->mDuration + NOTIFY_NEEDNEXTFD_IN_ADVANCE) >= pRecSink->mCurMaxFileDuration) 
            {
                bNeedRequest = TRUE;
            }
        }
        else if(pRecSink->mFileDurationPolicy == RecordFileDurationPolicy_AverageDuration)
        {
            if((pRecSink->mLoopDuration + NOTIFY_NEEDNEXTFD_IN_ADVANCE) >= pRecSink->mCurFileEndTm)
            {
                bNeedRequest = TRUE;
            }
            if((pRecSink->mLoopDurationAudio + NOTIFY_NEEDNEXTFD_IN_ADVANCE) >= pRecSink->mCurFileEndTm)
            {                

                bNeedRequest = TRUE;
            }
        }
        else
        {
            aloge("(f:%s, l:%d) fatal error! unknown FileDurationPolicy[0x%x], check code!", __FUNCTION__, __LINE__, pRecSink->mFileDurationPolicy);
        }
    }
    else if(pRecSink->mMaxFileSizeBytes > 0)   //user set max file size, then segment file base on fileSize.
    {
        if(pRecSink->mFileSizeBytes + pRecSink->mMaxFileSizeBytes/10 >= pRecSink->mMaxFileSizeBytes)
        {
            double fileSizeMB = (double)pRecSink->mFileSizeBytes/(1024*1024);
            alogd("fileSize[%lld]Bytes([%7.3lf]MB) < max[%7.3lf]MB, rec_file[%d], need request next fd", pRecSink->mFileSizeBytes, fileSizeMB, (double)pRecSink->mMaxFileSizeBytes/(1024*1024), pRecSink->rec_file);
            bNeedRequest = TRUE;
        }
    }
    else
    {
        bNeedRequest = FALSE;
    }
    return bNeedRequest;
}

BOOL RecSinkGrantSwitchFileAudio(RecSink *pRecSink, RecSinkPacket *pRSPacket)
{
    BOOL bGrant = TRUE;
    if(RecordFileDurationPolicy_AccurateDuration == pRecSink->mFileDurationPolicy)
    {
        bGrant = TRUE;
    }
    else if(RecordFileDurationPolicy_MinDuration == pRecSink->mFileDurationPolicy || RecordFileDurationPolicy_AverageDuration == pRecSink->mFileDurationPolicy)
    {
        if(pRecSink->mPrefetchFlagAudio)     // need to cache and wait
        {
            int videoStreamIndex = (int)CODEC_TYPE_VIDEO;
            int audioStreamIndex = (int)CODEC_TYPE_AUDIO;
            int VFrameDuration = 1000*1000 / pRecSink->mpMediaInf->uVideoFrmRate;
            int AFrameDuration = MAXDECODESAMPLE*1000 / pRecSink->mpMediaInf->sample_rate;
            
            if((pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000+AFrameDuration <= 
                (pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000+VFrameDuration)
            {
                alogd("RecSink[%d] can switch file. aSrcPts[%lld]ms, vSrcPts[%lld]ms, ADur[%lld]-VDur[%lld]=[%lld]ms, rec_file[%d]",
                             pRecSink->mMuxerId,
                             (pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000,
                             (pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000,
                             (pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000+AFrameDuration,
                             (pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000+VFrameDuration,
                             ((pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000+AFrameDuration) - ((pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000+VFrameDuration),
                             pRecSink->rec_file);

                             
                pRecSink->bTimeMeetAudio = TRUE;
                if(pRSPacket->mStreamType==CODEC_TYPE_VIDEO && pRSPacket->mFlags&AVPACKET_FLAG_KEYFRAME)
                {

                    bGrant = TRUE;          // calculation end
                }
                else
                {
                    bGrant = FALSE;         // A->cache;v ->push,and calculation start
                }
            }
            else
            {
                bGrant = FALSE;             // A ->cahe;v->push
                
                if(CODEC_TYPE_VIDEO == pRSPacket->mStreamType)
                {
                    alogv("RecSink[%d] streamType[%d] wait switch file in prefetch state, prevAudioPts[%lld]ms should less than prevVideoPts[%lld]ms(total prevAudioPts[%lld]ms<[%lld]ms), rec_file[%d]",
                        pRecSink->mMuxerId, pRSPacket->mStreamType,
                        pRecSink->mPrevPts[audioStreamIndex]/1000, pRecSink->mPrevPts[videoStreamIndex]/1000,
                        (pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000, (pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000,
                        pRecSink->rec_file);
                }
            }
        }
        else    // need not to cache and wait
        {
            if(pRecSink->mRecordMode & RECORDER_MODE_VIDEO)
            {
                if(pRSPacket->mStreamType==CODEC_TYPE_VIDEO && pRSPacket->mFlags&AVPACKET_FLAG_KEYFRAME)
                {
                    bGrant = TRUE;
                }
                else
                {
                    bGrant = FALSE;
                }
            }
            else
            {
                bGrant = TRUE;
            }
        }
    }
    return bGrant;
}



BOOL RecSinkGrantSwitchFile(RecSink *pRecSink, RecSinkPacket *pRSPacket)
{
    BOOL bGrant = TRUE;
    if(RecordFileDurationPolicy_AccurateDuration == pRecSink->mFileDurationPolicy)
    {
        bGrant = TRUE;
    }
    else if(RecordFileDurationPolicy_MinDuration == pRecSink->mFileDurationPolicy || RecordFileDurationPolicy_AverageDuration == pRecSink->mFileDurationPolicy)
    {
        if(pRecSink->mPrefetchFlag)
        {
            int videoStreamIndex = (int)CODEC_TYPE_VIDEO;
            int audioStreamIndex = (int)CODEC_TYPE_AUDIO;
            int VFrameDuration = 1000*1000 / pRecSink->mpMediaInf->uVideoFrmRate;
            int AFrameDuration = MAXDECODESAMPLE*1000 / pRecSink->mpMediaInf->sample_rate;
            
            if((pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000+AFrameDuration >= 
                (pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000+VFrameDuration)
            {
                alogd("RecSink[%d] can switch file. aSrcPts[%lld]ms, vSrcPts[%lld]ms, ADur[%lld]-VDur[%lld]=[%lld]ms, rec_file[%d]",
                             pRecSink->mMuxerId,
                             (pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000,
                             (pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000,
                             (pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000+AFrameDuration,
                             (pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000+VFrameDuration,
                             ((pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000+AFrameDuration) - ((pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000+VFrameDuration),
                             pRecSink->rec_file);
                bGrant = TRUE;
            }
            else
            {
                if(CODEC_TYPE_AUDIO == pRSPacket->mStreamType)
                {
                    alogv("RecSink[%d] streamType[%d] wait switch file in prefetch state, prevAudioPts[%lld]ms should less than prevVideoPts[%lld]ms(total prevAudioPts[%lld]ms<[%lld]ms), rec_file[%d]",
                        pRecSink->mMuxerId, pRSPacket->mStreamType,
                        pRecSink->mPrevPts[audioStreamIndex]/1000, pRecSink->mPrevPts[videoStreamIndex]/1000,
                        (pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex])/1000, (pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])/1000,
                        pRecSink->rec_file);
                }
                bGrant = FALSE;
            }
        }
        else
        {
            if(pRecSink->mRecordMode & RECORDER_MODE_VIDEO)
            {
                if(pRSPacket->mStreamType==CODEC_TYPE_VIDEO && pRSPacket->mFlags&AVPACKET_FLAG_KEYFRAME)
                {
                    bGrant = TRUE;
                }
                else
                {
                    bGrant = FALSE;
                }
            }
            else
            {
                bGrant = TRUE;
            }
        }
    }
    return bGrant;
}

void RecSinkSwitchFile(RecSink *pSinkInfo, int clrFile)
{
    if(0 == pSinkInfo->nOutputFd)
    {
        alogd("Be careful, fd == 0, sinkInfo[%p] muxerId[%d]", pSinkInfo, pSinkInfo->mMuxerId);
    }
//    alogd("TOTAL duration: %d(ms)", pSinkInfo->mDuration);
//    alogd("TOTAL duration audio: %d(ms)", pSinkInfo->mDurationAudio);
//    alogd("TOTAL file size: %lld(bytes)", pSinkInfo->mFileSizeBytes);

    if(TRUE==pSinkInfo->bTimeMeetAudio && 0<pSinkInfo->mVideoFrmCntWriteMore)   // more video frame was sent to file
    {
        int64_t videoLen = pSinkInfo->mVideoPtsWriteMoreEnd - pSinkInfo->mVideoPtsWriteMoreSt + 1000*1000/pSinkInfo->mpMediaInf->uVideoFrmRate*1000;
        int64_t audioLen = 0;
        int64_t audio_st = -1;
        int nErrorInterval = MAXDECODESAMPLE*1000 / pSinkInfo->mpMediaInf->sample_rate/2*1000;  // unit:us
        RecSinkPacket   *pRSPacket = NULL;
        RecSinkPacket   *pRSPacketNext = NULL;
        int cnt = 0;
        struct list_head *pList;
        list_for_each(pList, &pSinkInfo->mPrefetchRSPacketList)
        {
            cnt++;
        } 
        alogw("avsync_a_list:%d-%d-%lld-%lld-%d",cnt,pSinkInfo->mVideoFrmCntWriteMore,
                                pSinkInfo->mVideoPtsWriteMoreSt,pSinkInfo->mVideoPtsWriteMoreEnd,pSinkInfo->mpMediaInf->nWidth);
        
        for(int i=0;i<cnt-1;i++)
        {
            if(pSinkInfo->mVideoFrmCntWriteMore <= 2 && pSinkInfo->mpMediaInf->sample_rate<16000)  // for condition that audio sample<16000
            {
                alogw("avsync_bk0");
                break;
            }
            if(!list_empty(&pSinkInfo->mPrefetchRSPacketList))
            {
                pRSPacket = list_first_entry(&pSinkInfo->mPrefetchRSPacketList, RecSinkPacket, mList);
                list_del(&pRSPacket->mList);
            } 
            if(NULL != pRSPacket)
            {
                if(-1 == audio_st)
                {
                    audio_st = pRSPacket->mPts;
                }
                audioLen = pRSPacket->mPts - audio_st + MAXDECODESAMPLE*1000 / pSinkInfo->mpMediaInf->sample_rate*1000;

                
                RecSinkWriteRSPacket(pSinkInfo, pRSPacket);
                RecSinkReleaseRSPacket(pSinkInfo, pRSPacket);
                
                if(audioLen<videoLen && videoLen-audioLen <= nErrorInterval)    
                {
                    alogw("avsync_bk1");
                    break;
                }
                else if(audioLen>videoLen && audioLen-videoLen <= nErrorInterval)
                {                   
                    alogw("avsync_bk2");
                    break;
                }
                
            }
        }
        
    }

    int64_t tm1, tm2;
    tm1 = CDX_GetSysTimeUsMonotonic();
    RecSinkMuxerClose(pSinkInfo, clrFile);
    tm2 = CDX_GetSysTimeUsMonotonic();
    alogd("muxerId[%d]recRender_MuxerClose[%lld]MB itl[%lld]ms", pSinkInfo->mMuxerId, pSinkInfo->mFileSizeBytes/(1024*1024), (tm2-tm1)/1000);
    pSinkInfo->mDuration = 0;
    pSinkInfo->mDurationAudio = 0;
    pSinkInfo->mDurationText = 0;
    pSinkInfo->mPrevDuration = 0;
    pSinkInfo->mPrevDurationAudio = 0;
    pSinkInfo->mPrevDurationText = 0;
    pSinkInfo->mFileSizeBytes = 0;
    pSinkInfo->bNeedSw = FALSE;
    pSinkInfo->bNeedSwAudio = FALSE;

    pSinkInfo->bTimeMeetAudio = FALSE;
    pSinkInfo->mVideoFrmCntWriteMore = 0;
    pSinkInfo->mVideoPtsWriteMoreSt = -1;
    
    int i;
    for(i=0;i<MAX_TRACK_COUNT;i++)
    {
        pSinkInfo->mbTrackInit[i] = FALSE;
        pSinkInfo->mPrevPts[i] = -1;
        pSinkInfo->mBasePts[i] = -1;
//        if(pSinkInfo->mSwitchFdImpactFlag)
//        {
//            pSinkInfo->mInputPrevPts[i] = -1;
//        }
    }
    if(pSinkInfo->mPrefetchFlag)
    {
        alogd("switchFile, reset prefetchFlag to false!");
        pSinkInfo->mPrefetchFlag = FALSE;
    }

    if(pSinkInfo->mPrefetchFlagAudio)
    {
        alogd("switchFile, reset prefetchFlagAudio to false!");
        pSinkInfo->mPrefetchFlagAudio = FALSE;
    }

    
    RecSinkMovePrefetchRSPackets(pSinkInfo);
    pSinkInfo->mVideoFrameCounter = 0;
    //change fd.
    if(pSinkInfo->nSwitchFd >= 0)
    {
        if(pSinkInfo->mSwitchFilePath)
        {
            aloge("fatal error! fd[%d] and path[%s] only use one, check code!", pSinkInfo->nSwitchFd, pSinkInfo->mSwitchFilePath);
        }
        pSinkInfo->nOutputFd = pSinkInfo->nSwitchFd;
        pSinkInfo->nSwitchFd = -1;
        pSinkInfo->nFallocateLen = pSinkInfo->nSwitchFdFallocateSize;
        pSinkInfo->nSwitchFdFallocateSize = 0;
//        if(pSinkInfo->mSwitchFdImpactFlag)
//        {
//            alogd("to write impact fd!");
//            pSinkInfo->mSwitchFdImpactFlag = 0;
//        }
        pSinkInfo->reset_fd_flag = FALSE;
    }
    else if(pSinkInfo->mSwitchFilePath!=NULL)
    {
        if(pSinkInfo->mPath)
        {
            aloge("fatal error! mPath[%s] should be null", pSinkInfo->mPath);
            free(pSinkInfo->mPath);
            pSinkInfo->mPath = NULL;
        }
        pSinkInfo->mPath = strdup(pSinkInfo->mSwitchFilePath);
        free(pSinkInfo->mSwitchFilePath);
        pSinkInfo->mSwitchFilePath = NULL;
        pSinkInfo->nFallocateLen = pSinkInfo->nSwitchFdFallocateSize;
        pSinkInfo->nSwitchFdFallocateSize = 0;
//        if(pSinkInfo->mSwitchFdImpactFlag)
//        {
//            alogd("to write impact fd!");
//            pSinkInfo->mSwitchFdImpactFlag = 0;
//        }
        pSinkInfo->reset_fd_flag = FALSE;
    }
    else
    {
        alogd("Be careful. pRecRenderData->nSwitchFd[%d] < 0", pSinkInfo->nSwitchFd);
    }

	if (pSinkInfo->rec_file == FILE_NEED_SWITCH_TO_IMPACT)
	{
        aloge("fatal error! check code!");
		pSinkInfo->rec_file = FILE_IMPACT_RECDRDING;
	}
	else if (pSinkInfo->rec_file == FILE_IMPACT_RECDRDING)
	{
        aloge("fatal error! check code!");
		pSinkInfo->rec_file = FILE_NORMAL;
	}
    else if(pSinkInfo->rec_file == FILE_NEED_SWITCH_TO_NORMAL)
    {
        pSinkInfo->rec_file = FILE_NORMAL;
    }
    else
    {
    }
}

static void* RecSinkThread(void* pThreadData) 
{
    unsigned int cmddata;
    CompInternalMsgType cmd;
    RecSink *pRecSink = (RecSink*) pThreadData;
    message_t cmd_msg;
    alogv("RecSink thread start run...\n");
    char threadName[20];
    snprintf(threadName, 20, "RecSink[%d]", pRecSink->mMuxerId);
    prctl(PR_SET_NAME, (unsigned long)threadName, 0, 0, 0);
    while (1) { 
PROCESS_MESSAGE:
        if(get_message(&pRecSink->mMsgQueue, &cmd_msg) == 0)
        {
            cmd = cmd_msg.command;
            cmddata = (unsigned int)cmd_msg.para0;

            alogv("get_message cmd:%d", cmd);

            // State transition command
            if (cmd == SetState) 
            {
                // If the parameter states a transition to the same state
                // raise a same state transition error.
                if (pRecSink->mStatus == (COMP_STATETYPE) (cmddata))
                {
                    pRecSink->mpCallbacks->EventHandler(
                            pRecSink,
                            pRecSink->mpAppData, 
                            COMP_EventError, 
                            ERR_MUX_SAMESTATE, 
                            0, 
                            NULL);
                }
                else 
                {
                    // transitions/callbacks made based on state transition table
                    // cmddata contains the target state
                    switch ((COMP_STATETYPE) (cmddata)) 
                    {
                        case COMP_StateInvalid:
                        {
                            pRecSink->mVideoPtsWriteMoreSt = -1;
                            pRecSink->mStatus = COMP_StateInvalid;
                            pRecSink->mpCallbacks->EventHandler(
                                    pRecSink,
                                    pRecSink->mpAppData, 
                                    COMP_EventError, 
                                    ERR_MUX_INCORRECT_STATE_TRANSITION, 
                                    0, 
                                    NULL);
                            pRecSink->mpCallbacks->EventHandler(
                                    pRecSink,
                                    pRecSink->mpAppData, 
                                    COMP_EventCmdComplete, 
                                    COMP_CommandStateSet, 
                                    pRecSink->mStatus, 
                                    NULL);
                            break;
                        }
                        
                        case COMP_StateLoaded:
                        {
                            alogv("set state LOADED");
                            if (pRecSink->mStatus == COMP_StateExecuting || pRecSink->mStatus == COMP_StatePause)
                            {
                                RecSinkMovePrefetchRSPackets(pRecSink);
                                RecSinkDrainAllRSPackets(pRecSink, FALSE);
                                if(pRecSink->mbMuxerInit)
                                {
                                    int64_t tm1, tm2;
                                    tm1 = CDX_GetSysTimeUsMonotonic();
                                    RecSinkMuxerClose(pRecSink, 0);
                                    tm2 = CDX_GetSysTimeUsMonotonic();
                                    alogd("muxerId[%d] recRender_MuxerClose[%lld]MB itl[%lld]ms", pRecSink->mMuxerId, pRecSink->mFileSizeBytes/(1024*1024), (tm2-tm1)/1000);
                                }
                                pRecSink->mPrefetchFlag = FALSE;
                            }
                            else
                            {
                                aloge("fatal error! muxerId[%d]muxerMode[%d] wrong status[%d]", pRecSink->mMuxerId, pRecSink->nMuxerMode, pRecSink->mStatus);
                            }
                            pRecSink->mStatus = COMP_StateLoaded;
                            pRecSink->mpCallbacks->EventHandler(
                                    pRecSink, 
                                    pRecSink->mpAppData,
                                    COMP_EventCmdComplete, 
                                    COMP_CommandStateSet,
                                    pRecSink->mStatus, 
                                    NULL);
                            cdx_sem_up(&pRecSink->mSemStateComplete);
                            alogv("RecRender set state LOADED ok");
                            break;
                        }
                        case COMP_StateExecuting:
                        {
                            if (pRecSink->mStatus == COMP_StateLoaded || pRecSink->mStatus == COMP_StatePause)
                            {
                                pRecSink->mStatus = COMP_StateExecuting;
                                pRecSink->mpCallbacks->EventHandler(
                                        pRecSink, 
                                        pRecSink->mpAppData,
                                        COMP_EventCmdComplete, 
                                        COMP_CommandStateSet,
                                        pRecSink->mStatus, 
                                        NULL);
                            }
                            else
                            {
                                aloge("fatal error! Set wrong status[%d]->Executing", pRecSink->mStatus);
                                pRecSink->mpCallbacks->EventHandler(
                                        pRecSink, 
                                        pRecSink->mpAppData,
                                        COMP_EventError, 
                                        ERR_MUX_INCORRECT_STATE_TRANSITION, 
                                        0, 
                                        NULL);
                            }
                            cdx_sem_up(&pRecSink->mSemStateComplete);
                            break;
                        }
                        case COMP_StatePause:
                        {
                            // Transition can only happen from idle or executing state
                            if (pRecSink->mStatus == COMP_StateLoaded || pRecSink->mStatus == COMP_StateExecuting) 
                            {
                                pRecSink->mStatus = COMP_StatePause;
                                pRecSink->mpCallbacks->EventHandler(
                                        pRecSink, 
                                        pRecSink->mpAppData,
                                        COMP_EventCmdComplete, 
                                        COMP_CommandStateSet,
                                        pRecSink->mStatus, 
                                        NULL);
                            } 
                            else
                            {
                                aloge("fatal error! Set wrong status[%d]->Pause", pRecSink->mStatus);
                                pRecSink->mpCallbacks->EventHandler(
                                        pRecSink, 
                                        pRecSink->mpAppData,
                                        COMP_EventError, 
                                        ERR_MUX_INCORRECT_STATE_TRANSITION, 
                                        0, 
                                        NULL);
                            }
                            break;
                        }
                        default:
                        {
                            aloge("fatal error! wrong desStatus[%d], current status[%d]", (COMP_STATETYPE)cmddata, pRecSink->mStatus);
                            break;
                        }
                    }
                }
            }
            else if (cmd == Stop) 
            {
                // Kill thread
                goto EXIT;
            }
            else if (SwitchFileNormal == cmd)
            {
                if(pRecSink->rec_file != FILE_NORMAL)
                {
                    aloge("fatal error! Switch file normal, but rec_file=%d", pRecSink->rec_file);
                    goto PROCESS_MESSAGE;
                }
                ERRORTYPE switchRet = ERR_MUX_NOT_SUPPORT;
                pRecSink->rec_file = FILE_NEED_SWITCH_TO_NORMAL;
                if(FALSE == pRecSink->mbMuxerInit)
                {
                    alogd("not muxerInit when Switch file rec_file=%d, so close nOutputFd[%d]", pRecSink->rec_file, pRecSink->nOutputFd);
                    if(pRecSink->nOutputFd>=0)
                    {
                        if(pRecSink->mPath)
                        {
                            aloge("fatal error! fd and path only use one! check code!");
                        }
                        close(pRecSink->nOutputFd);
                        pRecSink->nOutputFd = -1;
                        pRecSink->mpCallbacks->EventHandler(
                                pRecSink, 
                                pRecSink->mpAppData, 
                                COMP_EventRecordDone, 
                                pRecSink->mMuxerId, 
                                0, 
                                NULL);
                    }
                    if(pRecSink->mPath)
                    {
                        free(pRecSink->mPath);
                        pRecSink->mPath = NULL;
                    }
                    pthread_mutex_lock(&pRecSink->mutex_reset_writer_lock);
                    if(pRecSink->reset_fd_flag)
                    {
                        if(pRecSink->nSwitchFd < 0 && NULL==pRecSink->mSwitchFilePath)
                        {
                            aloge("fatal error! reset__fd_flag is true but switchFd[%d]<0, check code!", pRecSink->nSwitchFd);
                            pRecSink->reset_fd_flag = FALSE;
                        }
                    }
                    pthread_mutex_unlock(&pRecSink->mutex_reset_writer_lock);
                }
                switchRet = SUCCESS;
                pRecSink->mpCallbacks->EventHandler(
                        pRecSink, 
                        pRecSink->mpAppData, 
                        COMP_EventCmdComplete, 
                        SwitchFileNormal, 
                        pRecSink->mMuxerId, 
                        (void*)switchRet);
            }

            
            else if (RecSink_InputPacketAvailable == cmd)
            {
                alogv("input packet available");
                //pRecSink->mNoInputPacketFlag = FALSE;
            }

            //precede to process message
            goto PROCESS_MESSAGE;
        }
        if (pRecSink->mStatus == COMP_StateExecuting) 
        {
            RecSinkPacket   *pRSPacket = RecSinkGetRSPacket(pRecSink);
            if(pRSPacket)
            {
                pthread_mutex_lock(&pRecSink->mutex_reset_writer_lock);

                int need_switch_audio = 0;      // audio duration meet the maxduration
                bool need_switch_video = FALSE; 
                need_switch_video = RecSinkIfNeedSwitchFile(pRecSink,&need_switch_audio);

                if(TRUE == need_switch_video)   // time duration meet setting
                {
                    //if not prefetch, decide whether can prefetch.
                    if(pRecSink->mPrefetchFlag == FALSE)
                    {
                        if(pRecSink->mRecordMode & RECORDER_MODE_VIDEO)
                        {
                            if(pRSPacket->mStreamType==CODEC_TYPE_VIDEO && pRSPacket->mFlags&AVPACKET_FLAG_KEYFRAME)
                            {
                                if(pRecSink->mRecordMode & RECORDER_MODE_AUDIO)
                                {
                                    int videoStreamIndex = (int)CODEC_TYPE_VIDEO;
                                    int audioStreamIndex = (int)CODEC_TYPE_AUDIO;
                                    if(pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex] <
                                        pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])
                                    {

                                        alogw("avsync_ch_v:%lld-%lld-%d",pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex],
                                                   pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex],pRecSink->mpMediaInf->nWidth);
                                        pRecSink->mPrefetchFlag = TRUE; // to cache video pkt in order to push more audio packet
                                    }
                                }
                            }
                        }
                    }
                    BOOL bGrant = RecSinkGrantSwitchFile(pRecSink, pRSPacket);
                    if(bGrant)
                    {
                        if(pRecSink->nOutputFd >= 0 || pRecSink->mPath)
                        {
                            list_add_tail(&pRSPacket->mList, &pRecSink->mPrefetchRSPacketList);
                                alogw("avsync_rc_swfv:%d-%d-%d-%lld-%lld",pRecSink->mpMediaInf->nWidth,
                                    pRecSink->mDuration,pRecSink->mDurationAudio,pRecSink->mLoopDuration,pRecSink->mCurFileEndTm);
                            pRSPacket = NULL;
                            RecSinkSwitchFile(pRecSink, 0);
                            pRSPacket = RecSinkGetRSPacket(pRecSink);
                            if(NULL == pRSPacket)
                            {
                                aloge("fatal error! check muxerId[%d]!", pRecSink->mMuxerId);
                            }
                        }
                        else
                        {
                            if(FALSE == pRecSink->nCallbackOutFlag)
                            {
                                aloge("fatal error! this muxerId[%d] has not fd, should be callback mode[%d], so don't switch file!", 
                                    pRecSink->mMuxerId, pRecSink->nCallbackOutFlag);
                            }
                        }
                    }
                }
                else if(need_switch_audio)
                {                    

                    if(pRecSink->mPrefetchFlagAudio == FALSE) // cache started or not
                    {
                        if(pRecSink->mRecordMode & RECORDER_MODE_AUDIO)
                        { 
                            int videoStreamIndex = (int)CODEC_TYPE_VIDEO;
                            int audioStreamIndex = (int)CODEC_TYPE_AUDIO;
                            if(pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex] >
                                pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex])
                            {
                                alogw("avsync_ch_a:%lld-%lld-%d",pRecSink->mPrevPts[audioStreamIndex]+pRecSink->mBasePts[audioStreamIndex],
                                           pRecSink->mPrevPts[videoStreamIndex]+pRecSink->mBasePts[videoStreamIndex],pRecSink->mpMediaInf->nWidth);
                                pRecSink->mPrefetchFlagAudio = TRUE; // to cache audio pkt in order to push more video packet
                            } 
                        }
                    }


                    BOOL bGrant = RecSinkGrantSwitchFileAudio(pRecSink, pRSPacket);
                    if(TRUE == bGrant)  // grant to switch file
                    {
                        if(pRecSink->nOutputFd >= 0 || pRecSink->mPath)
                        {
                            list_add_tail(&pRSPacket->mList, &pRecSink->mPrefetchRSPacketList);

                            pRSPacket = NULL;
                            RecSinkSwitchFile(pRecSink, 0);
                            pRSPacket = RecSinkGetRSPacket(pRecSink);
                            if(NULL == pRSPacket)
                            {
                                aloge("fatal error! check muxerId[%d]!", pRecSink->mMuxerId);
                            }
                        }
                        else
                        {
                            if(FALSE == pRecSink->nCallbackOutFlag)
                            {
                                aloge("fatal error! this muxerId[%d] has not fd, should be callback mode[%d], so don't switch file!", 
                                    pRecSink->mMuxerId, pRecSink->nCallbackOutFlag);
                            }
                        }
                    }
                    else        // not grant to switch file now
                    {
                        if(TRUE == pRecSink->bTimeMeetAudio)    //time meet,but video frm is not key frame
                        {
                            if(pRSPacket->mStreamType==CODEC_TYPE_VIDEO)
                            {
                                pRecSink->mVideoFrmCntWriteMore++;
                                alogw("avsync_cal:%d-%lld-%lld-%d",pRecSink->mpMediaInf->nWidth,pRSPacket->mPts,pRecSink->mVideoPtsWriteMoreSt,pRecSink->mVideoFrmCntWriteMore);
                                if(-1 == pRecSink->mVideoPtsWriteMoreSt)    // to record the pts of video frame write more
                                {
                                    pRecSink->mVideoPtsWriteMoreSt = pRSPacket->mPts;
                                    pRecSink->mVideoPtsWriteMoreEnd = pRSPacket->mPts;
                                }
                                else
                                {
                                    pRecSink->mVideoPtsWriteMoreEnd = pRSPacket->mPts;
                                }
                            }
                        }
                    }
            


                    
                }

                
                if(pRecSink->mbMuxerInit == FALSE
                    && (pRecSink->nCallbackOutFlag==TRUE || (pRecSink->nOutputFd>=0 || pRecSink->mPath!=NULL) || pRecSink->reset_fd_flag==TRUE))
                {
                    BOOL bGrant = TRUE;
//                    if(pRecSink->mRecordMode & RECORDER_MODE_VIDEO)
//                    {
//                        if(pRSPacket->mStreamType==CODEC_TYPE_VIDEO && pRSPacket->mFlags&AVPACKET_FLAG_KEYFRAME)
//                        {
//                            bGrant = TRUE;
//                        }
//                        else
//                        {
//                            bGrant = FALSE;
//                        }
//                    }
                    //first packet will be video or not.
                    //first video frame may be not key frame. 
                    //we will make first packet to be video, although to do this will discard audio frames.
//                    alogd("muxerId[%d]: first packet grant[%d]: stream_type[%d], flags[0x%x], pts[%lld]ms, videoSize[%dx%d]", 
//                        pRecSink->mMuxerId, bGrant, pRSPacket->mStreamType, pRSPacket->mFlags, pRSPacket->mPts, pRecSink->mpMediaInf->nWidth, pRecSink->mpMediaInf->nHeight);
                    if(bGrant)
                    {
                        if(RecSinkMuxerInit(pRecSink) != SUCCESS)
                        {
                            aloge("fatal error! muxerId[%d][%p]ValidMuxerInit Error!", pRecSink->mMuxerId, pRecSink);
                            pRecSink->mStatus = COMP_StateInvalid;   //OMX_StateIdle;
                            pthread_mutex_unlock(&pRecSink->mutex_reset_writer_lock);
                            RecSinkReleaseRSPacket(pRecSink, pRSPacket);
                            goto PROCESS_MESSAGE;
                        }
                        pRecSink->mVideoPtsWriteMoreSt = -1;
                        if(pRecSink->nCallbackOutFlag == FALSE)
                        {
                            //pRecSink->reset_fd_flag = FALSE;
                            if (pRecSink->rec_file == FILE_IMPACT_RECDRDING)
                            {
                                alogd("Need set next fd immediately after switch to impactFile");
                                pRecSink->mpCallbacks->EventHandler(pRecSink, pRecSink->mpAppData, COMP_EventNeedNextFd, pRecSink->mMuxerId, 0, NULL);
                                pRecSink->need_set_next_fd = FALSE;
                            }
                            else
                            {
                                pRecSink->need_set_next_fd = TRUE;
                            }
                        }
                    }
                }
                pthread_mutex_unlock(&pRecSink->mutex_reset_writer_lock);
                BOOL bReleasePacket = TRUE;
                if(pRecSink->mbMuxerInit)
                {
                    if(pRecSink->mPrefetchFlag)
                    {
                        if(pRSPacket->mStreamType == CODEC_TYPE_VIDEO)
                        {
                            list_add_tail(&pRSPacket->mList, &pRecSink->mPrefetchRSPacketList);
                            bReleasePacket = FALSE;
                        }
                        else
                        {
                            RecSinkWriteRSPacket(pRecSink, pRSPacket);
                        }
                    } 
                    else if(pRecSink->mPrefetchFlagAudio)
                    { 
                        if(pRSPacket->mStreamType == CODEC_TYPE_AUDIO)
                        {
                            list_add_tail(&pRSPacket->mList, &pRecSink->mPrefetchRSPacketList);
                            bReleasePacket = FALSE;
                        }
                        else
                        {
                            RecSinkWriteRSPacket(pRecSink, pRSPacket);
                        }
                    }
                    else
                    {
                        RecSinkWriteRSPacket(pRecSink, pRSPacket);
                    }
                }
                //release RSPacket
                if(bReleasePacket)
                {
                    RecSinkReleaseRSPacket(pRecSink, pRSPacket);
                }
//                if(pRecSink->mCurMaxFileDuration > 0 
//                    && pRecSink->need_set_next_fd == TRUE
//                    && (pRecSink->mLoopDuration + NOTIFY_NEEDNEXTFD_IN_ADVANCE) >= pRecSink->mCurFileEndTm) 
                if(RecSinkIfNeedRequestNextFd(pRecSink))
                {
                    alogd("Need set next fd. SinkInfo[0x%x], videosize[%dx%d], LoopDuration[%lld]ms, curFileEndTm[%lld]ms", 
                        pRecSink, pRecSink->mpMediaInf->nWidth, pRecSink->mpMediaInf->nHeight, pRecSink->mLoopDuration, pRecSink->mCurFileEndTm);
                    pRecSink->need_set_next_fd = FALSE;
                    pRecSink->mpCallbacks->EventHandler(pRecSink, pRecSink->mpAppData, COMP_EventNeedNextFd, pRecSink->mMuxerId, 0, NULL);
                }
            }
            else
            {
                pthread_mutex_lock(&pRecSink->mRSPacketListMutex);
                pRecSink->mNoInputPacketFlag = TRUE;
                pthread_mutex_unlock(&pRecSink->mRSPacketListMutex);
                TMessage_WaitQueueNotEmpty(&pRecSink->mMsgQueue, 0);
            }
        }
        else
        {
            TMessage_WaitQueueNotEmpty(&pRecSink->mMsgQueue, 0);
        }
    }
EXIT:
    alogv("RecSinkThread stopped");
    return (void*) SUCCESS;
}

static ERRORTYPE RecSinkConfig(PARAM_IN COMP_HANDLETYPE hComponent, PARAM_IN CdxOutputSinkInfo *pCdxSink)
{
    RecSink *pRecRenderSink = (RecSink*)hComponent;
    if(pRecRenderSink->mStatus == COMP_StateExecuting)
    {
        alogw("status already executing, cannot config again!");
        return SUCCESS;
    }
    if(pRecRenderSink->mStatus != COMP_StateLoaded)
    {
        aloge("fatal error! call in wrong status[%d]", pRecRenderSink->mStatus);
        return SUCCESS;
    }
    pRecRenderSink->mMuxerId = pCdxSink->mMuxerId;
    pRecRenderSink->nMuxerMode = pCdxSink->nMuxerMode;
    if(pRecRenderSink->nOutputFd >= 0)
    {
        aloge("fatal error! nOutputFd[%d]>=0", pRecRenderSink->nOutputFd);
        close(pRecRenderSink->nOutputFd);
        pRecRenderSink->nOutputFd = -1;
    }
    if(pRecRenderSink->mPath)
    {
        aloge("fatal error! mPath[%s] is not null", pRecRenderSink->mPath);
        free(pRecRenderSink->mPath);
        pRecRenderSink->mPath = NULL;
    }
    pRecRenderSink->nCallbackOutFlag = pCdxSink->nCallbackOutFlag;
    pRecRenderSink->bBufFromCacheFlag = pCdxSink->bBufFromCacheFlag;
    //pRecRenderSink->pOutputFile = NULL;
    if(pCdxSink->nOutputFd >= 0)
    {
        pRecRenderSink->nOutputFd = dup(pCdxSink->nOutputFd);
        //pRecRenderSink->nOutputFd = dup2SeldomUsedFd(pCdxSink->nOutputFd);
        alogd("dup fd[%d]->[%d]", pCdxSink->nOutputFd, pRecRenderSink->nOutputFd);
        pRecRenderSink->nFallocateLen = pCdxSink->nFallocateLen;
        //pRecRenderSink->reset_fd_flag = TRUE;
    }
    else
    {
        alogd("fd or path both not set");
    }
    message_t   msg;
    msg.command = SetState;
    msg.para0 = COMP_StateExecuting;
    put_message(&pRecRenderSink->mMsgQueue, &msg);
    cdx_sem_down(&pRecRenderSink->mSemStateComplete);

    return SUCCESS;
}

static ERRORTYPE RecSinkSetCallbacks(
        PARAM_IN COMP_HANDLETYPE hComponent,
		PARAM_IN RecSinkCallbackType* pCallbacks, 
		PARAM_IN void* pAppData) 
{
    RecSink *pThiz = (RecSink*)hComponent;
	ERRORTYPE eError = SUCCESS;

	pThiz->mpCallbacks = pCallbacks;
	pThiz->mpAppData = pAppData;

	return eError;
}

static ERRORTYPE RecSinkEmptyThisBuffer(
            PARAM_IN COMP_HANDLETYPE hComponent,
            PARAM_IN RecSinkPacket* pRSPacket)
{
    RecSink *pThiz = (RecSink*)hComponent;
    ERRORTYPE eError = SUCCESS;
    RecSinkPacket   *pEntryPacket;
    pthread_mutex_lock(&pThiz->mRSPacketListMutex);
    if(list_empty(&pThiz->mIdleRSPacketList))
    {
        alogw("idleRSPacketList are all used, malloc more!");
        if(SUCCESS!=RecSinkIncreaseIdleRSPacketList(pThiz))
        {
            pthread_mutex_unlock(&pThiz->mRSPacketListMutex);
            return ERR_MUX_NOMEM;
        }
    }
    pEntryPacket = list_first_entry(&pThiz->mIdleRSPacketList, RecSinkPacket, mList);
    RSSetRecSinkPacket(pEntryPacket, pRSPacket);
    //debug input pts.
    int videoStreamIndex = (int)CODEC_TYPE_VIDEO;
    int audioStreamIndex = (int)CODEC_TYPE_AUDIO;
    if(CODEC_TYPE_VIDEO == pEntryPacket->mStreamType)
    {
        pThiz->mDebugInputPts[videoStreamIndex] = pEntryPacket->mPts;
    }
    else if(CODEC_TYPE_AUDIO == pEntryPacket->mStreamType)
    {
        pThiz->mDebugInputPts[audioStreamIndex] = pEntryPacket->mPts;
    }
    list_move_tail(&pEntryPacket->mList, &pThiz->mValidRSPacketList);
    if(TRUE == pThiz->mNoInputPacketFlag)
    {
        message_t   msg;
        msg.command = RecSink_InputPacketAvailable;
        put_message(&pThiz->mMsgQueue, &msg);
        pThiz->mNoInputPacketFlag = FALSE;
    }
    pthread_mutex_unlock(&pThiz->mRSPacketListMutex);
    return eError;
}

static ERRORTYPE RecSinkSetRecordMode(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN RECORDER_MODE nMode)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mRecordMode = nMode;
    return SUCCESS;
}

static ERRORTYPE RecSinkSetMediaInf(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN _media_file_inf_t* pMediaInf)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mpMediaInf = pMediaInf;
    return SUCCESS;
}

static ERRORTYPE RecSinkSetVencExtraData(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN VencHeaderData* pExtraData)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mpVencExtraData = pExtraData;
    return SUCCESS;
}

static ERRORTYPE RecSinkSetMaxFileDuration(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN int64_t nDuration)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mMaxFileDuration = nDuration;

    pthread_mutex_lock(&pThiz->mutex_reset_writer_lock);
    if(pThiz->rec_file == FILE_NORMAL && pThiz->mbMuxerInit)
    {
        if(pThiz->mMaxFileDuration > pThiz->mCurMaxFileDuration)
        {
            pThiz->mCurFileEndTm += pThiz->mMaxFileDuration - pThiz->mCurMaxFileDuration;
            alogd("RecSinkSetMaxFileDuration muxid[%d] type[%d] oldDur:%lldms newDur:%lldms newFileEndTm:%lldms",
                pThiz->mMuxerId, pThiz->rec_file, pThiz->mCurMaxFileDuration, pThiz->mMaxFileDuration, pThiz->mCurFileEndTm);
            pThiz->mCurMaxFileDuration = pThiz->mMaxFileDuration;
        }
        else
        {
            aloge("fatal error! new Duration[%lld]ms can't be apply to current file!", pThiz->mMaxFileDuration);
        }
    }
    pthread_mutex_unlock(&pThiz->mutex_reset_writer_lock);

    return SUCCESS;
}

/*static ERRORTYPE RecSinkSetImpactFileDuration(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN int64_t nDuration)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mImpactFileDuration = nDuration;
    return SUCCESS;
}*/

static ERRORTYPE RecSinkSetFileDurationPolicy(
    PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_IN RecordFileDurationPolicy nPolicy)
{
    RecordFileDurationPolicy mFileDurationPolicy;
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mFileDurationPolicy = nPolicy;
    return SUCCESS;
}

static ERRORTYPE RecSinkSetShutDownNow(
    PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_IN BOOL bShutDownNowFlag)
{
    RecordFileDurationPolicy mFileDurationPolicy;
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mbShutDownNowFlag = bShutDownNowFlag;
    return SUCCESS;
}
    
static ERRORTYPE RecSinkSetMaxFileSize(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN int64_t nSize)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mMaxFileSizeBytes = nSize;
    return SUCCESS;
}

static ERRORTYPE RecSinkSetFsWriteMode(
    PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_IN FSWRITEMODE nFsWriteMode)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mFsWriteMode = nFsWriteMode;
    return SUCCESS;
}

static ERRORTYPE RecSinkSetFsSimpleCacheSize(
    PARAM_IN COMP_HANDLETYPE hComponent,
    PARAM_IN int nSize)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mFsSimpleCacheSize = nSize;
    return SUCCESS;
}

static ERRORTYPE RecSinkSetCallbackWriter(
        PARAM_IN COMP_HANDLETYPE hComponent,
        PARAM_IN CdxRecorderWriterCallbackInfo *pCallbackWriter)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mpCallbackWriter = pCallbackWriter;
    return SUCCESS;
}

static ERRORTYPE RecSinkSwitchFd(PARAM_IN COMP_HANDLETYPE hComponent, PARAM_IN int nFd, PARAM_IN int nFallocateLen, PARAM_IN int nIsImpact)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pthread_mutex_lock(&pThiz->mutex_reset_writer_lock);
    if (nIsImpact == 1 && (pThiz->rec_file == FILE_IMPACT_RECDRDING || pThiz->rec_file == FILE_NEED_SWITCH_TO_IMPACT)) 
    {
        alogd("impact file is recording, don't accept new impact Fd[%d].", nFd);
        pthread_mutex_unlock(&pThiz->mutex_reset_writer_lock);
        return SUCCESS;
    }
    if(nFd < 0)
    {
        aloge("fatal error! wrong new fd[%d]", nFd);
        pthread_mutex_unlock(&pThiz->mutex_reset_writer_lock);
        return ERR_MUX_ILLEGAL_PARAM;
    }
    if(pThiz->nSwitchFd >= 0)
    {
        alogd("nSwithFd[%d] already exist, directly close it! maybe impact happen during new fd is setting.", pThiz->nSwitchFd);
        close(pThiz->nSwitchFd);
        pThiz->nSwitchFd = -1;
        pThiz->nSwitchFdFallocateSize = 0;
    }
    if(pThiz->mSwitchFilePath)
    {
        alogd("switchFilePath[%s] already exist, maybe impact happen during new fd is setting.", pThiz->mSwitchFilePath);
        free(pThiz->mSwitchFilePath);
        pThiz->mSwitchFilePath = NULL;
        pThiz->nSwitchFdFallocateSize = 0;
    }
    if(nFd >= 0)
    {
        pThiz->nSwitchFd = dup(nFd);
        if(pThiz->nSwitchFd < 0)
        {
            aloge("fatal error! dup fail:[%d]->[%d],(%s)", nFd, pThiz->nSwitchFd, strerror(errno));
            system("lsof");
        }
        //pThiz->nSwitchFd = dup2SeldomUsedFd(nFd);
    }
    
    pThiz->nSwitchFdFallocateSize = nFallocateLen;
    //pThiz->mSwitchFdImpactFlag = nIsImpact;
    alogd("dup setfd[%d] to nSwitchFd[%d]", nFd, pThiz->nSwitchFd);
    if(TRUE == pThiz->reset_fd_flag)
    {
        alogd("reset__fd_flag is already true, maybe impact happen during new fd is setting");
    }
    pThiz->reset_fd_flag = TRUE;
    pthread_mutex_unlock(&pThiz->mutex_reset_writer_lock);
    return SUCCESS;
}

static ERRORTYPE RecSinkSendCmdSwitchFile(PARAM_IN COMP_HANDLETYPE hComponent, BOOL nCacheFlag)
{
    ERRORTYPE eError;
    RecSink *pThiz = (RecSink*)hComponent;
    if(pThiz->rec_file == FILE_NORMAL)
    {
        eError = SUCCESS;
    }
    else
    {
        alogd("already impact recording, recFileState[%d]", pThiz->rec_file);
        eError = ERR_MUX_SAMESTATE;
    }
    message_t   msg;
    msg.command = SwitchFile;
    msg.para0 = nCacheFlag;
    put_message(&pThiz->mMsgQueue, &msg);
    return eError;
}

static ERRORTYPE RecSinkSendCmdSwitchFileNormal(PARAM_IN COMP_HANDLETYPE hComponent)
{
    ERRORTYPE eError;
    RecSink *pThiz = (RecSink*)hComponent;
    if(pThiz->rec_file == FILE_NORMAL)
    {
        eError = SUCCESS;
    }
    else
    {
        aloge("fatal error! can't switch file normal in impact recording, recFileState[%d]", pThiz->rec_file);
        eError = ERR_MUX_NOT_PERM;
        return eError;
    }
    message_t   msg;
    msg.command = SwitchFileNormal;
    put_message(&pThiz->mMsgQueue, &msg);
    return eError;
}

static ERRORTYPE RecSinkSetSdcardState(PARAM_IN COMP_HANDLETYPE hComponent, PARAM_IN BOOL bSdcardState)
{
    RecSink *pThiz = (RecSink*)hComponent;
    pThiz->mbSdCardState = bSdcardState;
    return SUCCESS;
}

static ERRORTYPE RecSinkReset(PARAM_IN COMP_HANDLETYPE hComponent)
{
    RecSink *pThiz = (RecSink*)hComponent;
    if(pThiz->mStatus == COMP_StateLoaded)
    {
        alogv("status already loaded.");
        return SUCCESS;
    }
    if(pThiz->mStatus != COMP_StateExecuting && pThiz->mStatus != COMP_StatePause)
    {
        aloge("fatal error! call in wrong status[%d]", pThiz->mStatus);
    }
    message_t   msg;
    msg.command = SetState;
    msg.para0 = COMP_StateLoaded;
    put_message(&pThiz->mMsgQueue, &msg);
    cdx_sem_down(&pThiz->mSemStateComplete);
    if(pThiz->nOutputFd >= 0)
    {
        if(pThiz->mPath)
        {
            aloge("fatal error! fd[%d] and path[%s] all exist! check code!", pThiz->nOutputFd, pThiz->mPath);
        }
        aloge("maybe not muxerInit? nOutputFd[%d]>=0", pThiz->nOutputFd);
        close(pThiz->nOutputFd);
        pThiz->nOutputFd = -1;
    }
    if(pThiz->mPath)
    {
        aloge("maybe not muxerInit? path[%s]>=0", pThiz->mPath);
        free(pThiz->mPath);
        pThiz->mPath = NULL;
    }
    pthread_mutex_lock(&pThiz->mutex_reset_writer_lock);
    if(pThiz->nSwitchFd >= 0)
    {
        if(pThiz->mSwitchFilePath)
        {
            aloge("fatal error! fd[%d] and path[%s] all exist! check code!", pThiz->nSwitchFd, pThiz->mSwitchFilePath);
        }
        close(pThiz->nSwitchFd);
        pThiz->nSwitchFd = -1;
        pThiz->reset_fd_flag = FALSE;
    }
    if(pThiz->mSwitchFilePath)
    {
        free(pThiz->mSwitchFilePath);
        pThiz->mSwitchFilePath = NULL;
        pThiz->reset_fd_flag = FALSE;
    }
    pthread_mutex_unlock(&pThiz->mutex_reset_writer_lock);
    RecSinkResetSomeMembers(pThiz);
    return SUCCESS;
}

ERRORTYPE RecSinkInit(PARAM_IN RecSink *pThiz)
{
    int err;
    ERRORTYPE   eError = SUCCESS;
    pThiz->nOutputFd = -1;
    pThiz->nSwitchFd = -1;
    pThiz->mPath = NULL;
    pThiz->mSwitchFilePath = NULL;
    pThiz->mStatus = COMP_StateLoaded;
    RecSinkResetSomeMembers(pThiz);
    if(0!=pthread_mutex_init(&pThiz->mutex_reset_writer_lock, NULL))
    {
        aloge("fatal error! pthread_mutex init fail");
        return ERR_MUX_NOMEM;
    }
    pThiz->ConfigByCdxSink = RecSinkConfig;
    pThiz->SetCallbacks = RecSinkSetCallbacks;
    pThiz->EmptyThisBuffer = RecSinkEmptyThisBuffer;
    pThiz->SetRecordMode = RecSinkSetRecordMode;
    pThiz->SetMediaInf = RecSinkSetMediaInf;
    pThiz->SetVencExtraData = RecSinkSetVencExtraData;
    pThiz->SetMaxFileDuration = RecSinkSetMaxFileDuration;
    //pThiz->SetImpactFileDuration = RecSinkSetImpactFileDuration;
    pThiz->SetFileDurationPolicy = RecSinkSetFileDurationPolicy;    
    pThiz->SetMaxFileSize = RecSinkSetMaxFileSize;
    pThiz->SetFsWriteMode = RecSinkSetFsWriteMode;
    pThiz->SetFsSimpleCacheSize = RecSinkSetFsSimpleCacheSize;
    pThiz->SetCallbackWriter = RecSinkSetCallbackWriter;
    pThiz->SwitchFd = RecSinkSwitchFd;
    pThiz->SendCmdSwitchFile = RecSinkSendCmdSwitchFile;
    pThiz->SendCmdSwitchFileNormal = RecSinkSendCmdSwitchFileNormal;
    pThiz->SetSdcardState = RecSinkSetSdcardState;
    pThiz->Reset = RecSinkReset;
    pThiz->SetShutDownNow = RecSinkSetShutDownNow;

    if(message_create(&pThiz->mMsgQueue)<0)
    {
        aloge("message create fail!");
        eError = ERR_MUX_NOMEM;
        goto _err0;
	}
	if(cdx_sem_init(&pThiz->mSemStateComplete, 0)<0)
	{
        aloge("cdx sem init fail!");
        eError = ERR_MUX_NOMEM;
        goto _err1;
	}
//    if(cdx_sem_init(&pThiz->mSemCmdComplete, 0)<0)
//	{
//        aloge("cdx sem init fail!");
//        eError = OMX_ErrorUndefined;
//        goto _err2;
//	}
    if(pthread_mutex_init(&pThiz->mRSPacketListMutex, NULL)!=0)
    {
        aloge("pthread mutex init fail!");
        eError = ERR_MUX_NOMEM;
        goto _err3;
    }
    //INIT_LIST_HEAD(&pThiz->mRSPacketBufList);
    INIT_LIST_HEAD(&pThiz->mPrefetchRSPacketList);
    INIT_LIST_HEAD(&pThiz->mValidRSPacketList);
    INIT_LIST_HEAD(&pThiz->mIdleRSPacketList);
    if(SUCCESS!=RecSinkIncreaseIdleRSPacketList(pThiz))
    {
        goto _err4;
    }
    
    err = pthread_create(&pThiz->mThreadId, NULL, RecSinkThread, (void*)pThiz);
	if (err || !pThiz->mThreadId) 
    {
        aloge("pthread create fail!");
		eError = ERR_MUX_NOMEM;
		goto _err5;
	}
    return SUCCESS;
_err5:
    if(!list_empty(&pThiz->mIdleRSPacketList))
    {
        RecSinkPacket   *pEntry, *pTmp;
        list_for_each_entry_safe(pEntry, pTmp, &pThiz->mIdleRSPacketList, mList)
        {
            list_del(&pEntry->mList);
            free(pEntry);
        }
    }
    //INIT_LIST_HEAD(&pThiz->mRSPacketBufList);
_err4:
    pthread_mutex_destroy(&pThiz->mRSPacketListMutex);
_err3:
    //cdx_sem_deinit(&pThiz->mSemCmdComplete);
//_err2:
    cdx_sem_deinit(&pThiz->mSemStateComplete);
_err1:
    message_destroy(&pThiz->mMsgQueue);
_err0:
    pthread_mutex_destroy(&pThiz->mutex_reset_writer_lock);
    return eError;
}

ERRORTYPE RecSinkDestroy(RecSink *pThiz)
{
    int err;
    message_t   msg;
    msg.command = Stop;
    put_message(&pThiz->mMsgQueue, &msg);
	// Wait for thread to exit so we can get the status into "error"
	pthread_join(pThiz->mThreadId, (void*) &err);
    alogv("RecSink thread exit[%d]!", err);

    pthread_mutex_lock(&pThiz->mRSPacketListMutex);
    if(!list_empty(&pThiz->mPrefetchRSPacketList))
    {
        aloge("fatal error! prefetch RSPacket list is not empty! check code!");
    }
    if(!list_empty(&pThiz->mValidRSPacketList))
    {
        aloge("fatal error! valid RSPacket list is not empty! check code!");
    }
    if(!list_empty(&pThiz->mIdleRSPacketList))
    {
        RecSinkPacket   *pEntry, *pTmp;
        list_for_each_entry_safe(pEntry, pTmp, &pThiz->mIdleRSPacketList, mList)
        {
            list_del(&pEntry->mList);
            free(pEntry);
        }
    }
    pthread_mutex_unlock(&pThiz->mRSPacketListMutex);
    //INIT_LIST_HEAD(&pThiz->mRSPacketBufList);
    INIT_LIST_HEAD(&pThiz->mPrefetchRSPacketList);
    INIT_LIST_HEAD(&pThiz->mValidRSPacketList);
    INIT_LIST_HEAD(&pThiz->mIdleRSPacketList);
    pthread_mutex_destroy(&pThiz->mutex_reset_writer_lock);
    message_destroy(&pThiz->mMsgQueue);
    cdx_sem_deinit(&pThiz->mSemStateComplete);
    //cdx_sem_deinit(&pThiz->mSemCmdComplete);
    pthread_mutex_destroy(&pThiz->mRSPacketListMutex);
    if(pThiz->mPath)
    {
        free(pThiz->mPath);
        pThiz->mPath = NULL;
    }
    if(pThiz->mSwitchFilePath)
    {
        free(pThiz->mSwitchFilePath);
        pThiz->mSwitchFilePath = NULL;
    }
    pThiz->mStatus = COMP_StateInvalid;
    return SUCCESS;
}

