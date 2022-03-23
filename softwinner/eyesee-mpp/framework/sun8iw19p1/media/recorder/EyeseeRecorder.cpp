/******************************************************************************
  Copyright (C), 2001-2016, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     : EyeseeRecorder.cpp
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2016/06/07
  Last Modified :
  Description   : recorder use mpp modules to implement video recording.
  Function List :
  History       :
******************************************************************************/
//#define LOG_NDEBUG 0
#define LOG_TAG "EyeseeRecorder"
#include <utils/plat_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <limits.h>
#include <unistd.h>
#include <fcntl.h>
#include <vector>
#include <utility>

#include <RecAVSync.h>
#include <aenc_sw_lib.h>
#include <mm_common.h>
#include <mm_comm_aio.h>
#include <mpi_sys.h>
#include <mpi_ai.h>
#include <mpi_aenc.h>
#include <TextEncApi.h> 
#include <mpi_tenc.h>
#include <mpi_venc_private.h>
#include <mpi_venc.h>
#include <mpi_region.h>
#include <mpi_mux.h>
#include <record_writer.h>
#include <media_common.h>

#include <utils/Mutex.h>
#include <MediaStructConvert.h>
//#include <EyeseeCamera.h>
#include <EyeseeRecorder.h>
#include <MediaCallbackDispatcher.h>
#include "CameraFrameManager.h"
#include "DynamicBitRateControl.h"
#include <dup2SeldomUsedFd.h>
//#include <system/audio.h>
#include <SystemBase.h>

#define DEFAULT_SIMPLE_CACHE_SIZE_VFS       (64*1024)//(4*1024)
namespace EyeseeLinux {
/*
static ERRORTYPE setQpRangeToVENC_CHN_ATTR_S(VENC_CHN_ATTR_S *pChnAttr, int minqp, int maxqp)
{
    switch(pChnAttr->RcAttr.mRcMode)
    {
        case VENC_RC_MODE_H264CBR:
        {
            pChnAttr->RcAttr.mAttrH264Cbr.mMaxQp = maxqp;
            pChnAttr->RcAttr.mAttrH264Cbr.mMinQp = minqp;
            break;
        }
        case VENC_RC_MODE_H264VBR:
        {
            pChnAttr->RcAttr.mAttrH264Vbr.mMaxQp = maxqp;
            pChnAttr->RcAttr.mAttrH264Vbr.mMinQp = minqp;
            break;
        }
        case VENC_RC_MODE_H264FIXQP:
        {
            pChnAttr->RcAttr.mAttrH264FixQp.mIQp = minqp;
            pChnAttr->RcAttr.mAttrH264FixQp.mPQp = maxqp;
            break;
        }
        case VENC_RC_MODE_H264ABR:
        {
            pChnAttr->RcAttr.mAttrH264Abr.mMaxIQp = maxqp;
            pChnAttr->RcAttr.mAttrH264Abr.mMinIQp = minqp;
            break;
        }
        case VENC_RC_MODE_H265CBR:
        {
            pChnAttr->RcAttr.mAttrH265Cbr.mMaxQp = maxqp;
            pChnAttr->RcAttr.mAttrH265Cbr.mMinQp = minqp;
            break;
        }
        case VENC_RC_MODE_H265VBR:
        {
            pChnAttr->RcAttr.mAttrH265Vbr.mMaxQp = maxqp;
            pChnAttr->RcAttr.mAttrH265Vbr.mMinQp = minqp;
            break;
        }
        case VENC_RC_MODE_H265FIXQP:
        {
            pChnAttr->RcAttr.mAttrH265FixQp.mIQp = minqp;
            pChnAttr->RcAttr.mAttrH265FixQp.mPQp = maxqp;
            break;
        }
        case VENC_RC_MODE_H265ABR:
        {
            pChnAttr->RcAttr.mAttrH265Abr.mMaxIQp = maxqp;
            pChnAttr->RcAttr.mAttrH265Abr.mMinIQp = minqp;
            break;
        }
        case VENC_RC_MODE_MJPEGFIXQP:
        {
            pChnAttr->RcAttr.mAttrMjpegeFixQp.mQfactor = minqp;
            break;
        }
        default:
        {
            alogd("fatal error! other rc mode[0x%x] don't need set qp!", pChnAttr->RcAttr.mRcMode);
            break;
        }
    }
    return SUCCESS;
}
*/

ERRORTYPE setVideoEncodingBitRateToVENC_CHN_ATTR_S(VENC_CHN_ATTR_S *pChnAttr, int bitRate)
{
    switch(pChnAttr->RcAttr.mRcMode)
    {
        case VENC_RC_MODE_H264CBR:
        {
            pChnAttr->RcAttr.mAttrH264Cbr.mBitRate = bitRate;
            break;
        }
        case VENC_RC_MODE_H264ABR:
        {
            pChnAttr->RcAttr.mAttrH264Abr.mMaxBitRate = bitRate;
            break;
        }
        case VENC_RC_MODE_H265CBR:
        {
            pChnAttr->RcAttr.mAttrH265Cbr.mBitRate = bitRate;
            break;
        }
        case VENC_RC_MODE_H265ABR:
        {
            pChnAttr->RcAttr.mAttrH265Abr.mMaxBitRate = bitRate;
            break;
        }
        case VENC_RC_MODE_MJPEGCBR:
        {
            pChnAttr->RcAttr.mAttrMjpegeCbr.mBitRate = bitRate;
            break;
        }
        default:
        {
            aloge("fatal error! other rc mode[0x%x] don't need set bitRate!", pChnAttr->RcAttr.mRcMode);
            break;
        }
    }
    return SUCCESS;
}

extern "C" ERRORTYPE MPPCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
	((EyeseeRecorder*)cookie)->notify(pChn, event, pEventData);
    return SUCCESS;
}

unsigned int EyeseeRecorder::gRecorderIdCounter = RecorderIdPrefixMark | 0x00;

EyeseeRecorder::EventHandler::EventHandler(EyeseeRecorder *pC)
{
    mpMediaRecorder = pC;
}

EyeseeRecorder::EventHandler::~EventHandler()
{
}

void EyeseeRecorder::EventHandler::handleMessage(const CallbackMessage &msg)
{
    switch (msg.what) 
    {
        case MEDIA_RECORDER_EVENT_ERROR:
            if (mpMediaRecorder->mOnErrorListener != NULL) 
            {
                int extra;
                if(msg.arg1 == MEDIA_ERROR_VENC_TIMEOUT)
                {
                    void *pData = msg.mDataPtr->getPointer();
                    extra = (int)pData;
                }
                else
                {
                    extra = msg.arg2;
                }
                mpMediaRecorder->mOnErrorListener->onError(mpMediaRecorder, msg.arg1, extra);
            }
            return;
        case MEDIA_RECORDER_EVENT_INFO:
            if (mpMediaRecorder->mOnInfoListener != NULL) 
            {
                mpMediaRecorder->mOnInfoListener->onInfo(mpMediaRecorder, msg.arg1, msg.arg2);
            }
            return;
        case MEDIA_RECORDER_VENDOR_EVENT_BSFRAME_AVAILABLE:
            if (mpMediaRecorder->mOnDataListener != NULL) 
            {
                mpMediaRecorder->mOnDataListener->onData(mpMediaRecorder, msg.arg1, msg.arg2);
            }
            return;
        default:
            aloge("fatal error! Unknown message type 0x%x, 0x%x, 0x%x", msg.what, msg.arg1, msg.arg2);
            return;
    }
}

EyeseeRecorder::CameraProxyListener::CameraProxyListener(EyeseeRecorder *recorder)
{
    mRecorder = recorder;
}

void EyeseeRecorder::CameraProxyListener::dataCallbackTimestamp(const void *pdata)
{
    mRecorder->dataCallbackTimestamp((const VIDEO_FRAME_BUFFER_S*)pdata);
}

bool EyeseeRecorder::process_media_recorder_call(status_t opStatus, const char* message)
{
    alogv("process_media_recorder_call");
    if (opStatus == (status_t)INVALID_OPERATION) 
    {
        aloge("INVALID_OPERATION");
        return true;
    } 
    else if (opStatus != (status_t)OK) 
    {
        aloge("%s", message);
        return true;
    }
    return false;
}

EyeseeRecorder::EyeseeRecorder() :
    mRecorderId(gRecorderIdCounter++),
    mOnErrorListener(NULL),
    mOnInfoListener(NULL),
    mOnDataListener(NULL)
{
    alogv("Constructor");
    mpInputFrameManager = NULL;
    mpCameraProxy = NULL;
    mCameraSourceChannel = 0;
    mTimeLapseEnable = false;
    mTimeBetweenFrameCapture = 0;
    mMaxFileDuration = 0;
    mMuxCacheDuration = 0;
    mMaxFileSizeBytes = 0;
    mVideoEncoder = PT_MAX;
    mAudioEncoder = PT_MAX;
    last_frm_pts = -1;
    frm_cnt = 0;
    gps_state = 0;
    rec_start_timestamp = -1;
    mIgnoreAudioBlockNum = 0;
    mIgnoreAudioBytes = 0;
    mPauseAudioDuration = 0;

    
    mVideoMaxKeyItl = 30;
    mCallbackOutDataType = CALLBACK_OUT_DATA_VIDEO_ONLY;
    mIdleEncBufList.resize(ENC_BACKUP_BUFFER_NUM);
    for(std::list<VEncBuffer>::iterator it = mIdleEncBufList.begin(); it != mIdleEncBufList.end(); ++it)
    {
        memset(&*it, 0, sizeof(VEncBuffer));
    }

    //MPP components
    mVeChn = MM_INVALID_CHN;
    mAiDev = -1;
    mAiChn = MM_INVALID_CHN;
    mAeChn = MM_INVALID_CHN;
    mTeChn = MM_INVALID_CHN;
    mMuxGrp = MM_INVALID_CHN;
    mAVSync = NULL;
    mpDBRC = NULL;
    mEnableDBRC = false;
    mMuteMode = false;

    doCleanUp();
    mEventHandler = new EventHandler(this);

    mCurrentState = MEDIA_RECORDER_IDLE;
}

EyeseeRecorder::~EyeseeRecorder()
{
	alogv("destructor");
    Mutex::Autolock autoLock(mLock);
    if (!(mCurrentState & MEDIA_RECORDER_IDLE)) 
    {
        aloge("fatal error! can't destruct in an invalid state: 0x%x", mCurrentState);
        reset_l();
    }
    if(mEventHandler)
    {
        delete mEventHandler;
        mEventHandler = NULL;
    }

    if(mpInputFrameManager)
    {
        delete mpInputFrameManager;
        mpInputFrameManager = NULL;
    }
    if(mpCameraProxy)
    {
        delete mpCameraProxy;
    }
}
/*
status_t EyeseeRecorder::setCamera(EyeseeCamera *pC)
{
    if(mpCameraProxy)
    {
        alogw("Be careful! CameraProxy[%p] is already set!", mpCameraProxy);
        delete mpCameraProxy;
    }
    mpCameraProxy = pC->getRecordingProxy();
    return NO_ERROR;
}

status_t EyeseeRecorder::setCamera(EyeseeCamera *pC, int channelId)
{
    status_t ret;
    ret = setCamera(pC);
    ret = setSourceChannel(channelId);
    return ret;
}

status_t EyeseeRecorder::setISE(EyeseeISE *pISE)
{
    if(mpCameraProxy)
    {
        alogw("Be careful! CameraProxy[%p] is already set!", mpCameraProxy);
        delete mpCameraProxy;
    }
    mpCameraProxy = pISE->getRecordingProxy();
    return NO_ERROR;
}

status_t EyeseeRecorder::setISE(EyeseeISE *pISE, int channelId)
{
    status_t ret;
    ret = setISE(pISE);
    ret = setSourceChannel(channelId);
    return ret;
}

status_t EyeseeRecorder::setSourceChannel(int channelId)
{
    mCameraSourceChannel = channelId;
    return NO_ERROR;
}
*/
status_t EyeseeRecorder::setCameraProxy(CameraRecordingProxy *pCameraProxy, int channelId)
{
    if(mpCameraProxy)
    {
        alogw("Be careful! CameraProxy[%p] is already set!", mpCameraProxy);
        delete mpCameraProxy;
    }
    mpCameraProxy = pCameraProxy;
    mCameraSourceChannel = channelId;
    return NO_ERROR;
}

status_t EyeseeRecorder::setCaptureRate(double fps)
{
    if(fps <= 0)
    {
        mTimeLapseEnable = false;
        return NO_ERROR;
    }
    int64_t timeUs = (int64_t) (1000000.0 / fps + 0.5f);
    // Not allowing time more than a day
    if (timeUs <= 0 || timeUs > 86400*1E6) 
    {
        aloge("Time between frame capture (%lld) is out of range [0, 1 Day]", (long long)timeUs);
        return BAD_VALUE;
    }
    mTimeLapseEnable = true;
    mTimeBetweenFrameCapture = timeUs;
    return NO_ERROR;
}

status_t EyeseeRecorder::setSlowRecordMode(bool bEnable)
{
    mTimeLapseEnable = bEnable;
    if(mTimeLapseEnable)
    {
        mTimeBetweenFrameCapture = 0;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setOrientationHint(int degrees)
{
    mRotationDegrees = degrees;
    return NO_ERROR;
}

status_t EyeseeRecorder::setLocation(float latitude, float longitude)
{
    mGeoAvailable = true;
    mLatitude = latitude;
    mLongitude = longitude;
    return NO_ERROR;
}

status_t EyeseeRecorder::setVideoSize(int width, int height)
{
    mVideoWidth = width;
    mVideoHeight = height;
    return NO_ERROR;
}

status_t EyeseeRecorder::setVideoFrameRate(int rate)
{
    mFrameRate = rate;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        CameraParameters cameraParam;
        mpCameraProxy->getParameters(mCameraSourceChannel, cameraParam);
        VENC_FRAME_RATE_S stFrameRate;
        stFrameRate.SrcFrmRate = cameraParam.getPreviewFrameRate();
        stFrameRate.DstFrmRate = mFrameRate;
        AW_MPI_VENC_SetFrameRate(mVeChn, &stFrameRate);
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setVideoEncodingRateControlMode(VideoEncodeRateControlMode rcMode)
{
    mVideoRCMode = rcMode;
    return NO_ERROR;
}

status_t EyeseeRecorder::setVEncBitRateControlAttr(VEncBitRateControlAttr& RcAttr)
{
    if(RcAttr.mVEncType == mVideoEncoder && RcAttr.mRcMode == mVideoRCMode)
    {
        //Todo: we will implement dynamic param setting in future.
        Mutex::Autolock autoLock(mLock);
        if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
        {
            if(RcAttr.mVEncType == mVEncRcAttr.mVEncType && RcAttr.mRcMode == mVEncRcAttr.mRcMode)
            {
                //update bitRate when cbr.
                if(VideoRCMode_CBR == RcAttr.mRcMode)
                {
                    if(PT_H264 == RcAttr.mVEncType)
                    {
                        if(mVEncRcAttr.mAttrH264Cbr.mBitRate != RcAttr.mAttrH264Cbr.mBitRate
                            || mVEncRcAttr.mAttrH264Cbr.mMaxQp != RcAttr.mAttrH264Cbr.mMaxQp
                            || mVEncRcAttr.mAttrH264Cbr.mMinQp != RcAttr.mAttrH264Cbr.mMinQp)
                        {
                            alogd("need update h264 cbr bitRate[%d]->[%d], maxQp[%d]->[%d], minQp[%d]->[%d]", 
                                mVEncRcAttr.mAttrH264Cbr.mBitRate, RcAttr.mAttrH264Cbr.mBitRate,
                                mVEncRcAttr.mAttrH264Cbr.mMaxQp, RcAttr.mAttrH264Cbr.mMaxQp,
                                mVEncRcAttr.mAttrH264Cbr.mMinQp, RcAttr.mAttrH264Cbr.mMinQp);
                            mVEncRcAttr.mAttrH264Cbr.mBitRate = RcAttr.mAttrH264Cbr.mBitRate;
                            mVEncRcAttr.mAttrH264Cbr.mMaxQp = RcAttr.mAttrH264Cbr.mMaxQp;
                            mVEncRcAttr.mAttrH264Cbr.mMinQp = RcAttr.mAttrH264Cbr.mMinQp;
                            VENC_CHN_ATTR_S attr;
                            AW_MPI_VENC_GetChnAttr(mVeChn, &attr);
                            if(attr.RcAttr.mRcMode!=VENC_RC_MODE_H264CBR)
                            {
                                aloge("fatal error! check mpp_rcMode[0x%x]", attr.RcAttr.mRcMode);
                            }
                            attr.RcAttr.mAttrH264Cbr.mBitRate = RcAttr.mAttrH264Cbr.mBitRate;
                            attr.RcAttr.mAttrH264Cbr.mMaxQp = RcAttr.mAttrH264Cbr.mMaxQp;
                            attr.RcAttr.mAttrH264Cbr.mMinQp = RcAttr.mAttrH264Cbr.mMinQp;
                            AW_MPI_VENC_SetChnAttr(mVeChn, &attr);
                        }
                    }
                    else if(PT_H265 == RcAttr.mVEncType)
                    {
                        if(mVEncRcAttr.mAttrH265Cbr.mBitRate != RcAttr.mAttrH265Cbr.mBitRate)
                        {
                            alogd("need update h265 cbr bitRate[%d]->[%d]", mVEncRcAttr.mAttrH265Cbr.mBitRate, RcAttr.mAttrH265Cbr.mBitRate);
                            mVEncRcAttr.mAttrH265Cbr.mBitRate = RcAttr.mAttrH265Cbr.mBitRate;
                            VENC_CHN_ATTR_S attr;
                            AW_MPI_VENC_GetChnAttr(mVeChn, &attr);
                            if(attr.RcAttr.mRcMode!=VENC_RC_MODE_H265CBR)
                            {
                                aloge("fatal error! check mpp_rcMode[0x%x]", attr.RcAttr.mRcMode);
                            }
                            attr.RcAttr.mAttrH265Cbr.mBitRate = RcAttr.mAttrH265Cbr.mBitRate;
                            AW_MPI_VENC_SetChnAttr(mVeChn, &attr);
                        }
                    }
                    else if(PT_MJPEG == RcAttr.mVEncType)
                    {
                        if(mVEncRcAttr.mAttrMjpegCbr.mBitRate != RcAttr.mAttrMjpegCbr.mBitRate)
                        {
                            alogd("need update mjpeg cbr bitRate[%d]->[%d]", mVEncRcAttr.mAttrMjpegCbr.mBitRate, RcAttr.mAttrMjpegCbr.mBitRate);
                            mVEncRcAttr.mAttrMjpegCbr.mBitRate = RcAttr.mAttrMjpegCbr.mBitRate;
                            VENC_CHN_ATTR_S attr;
                            AW_MPI_VENC_GetChnAttr(mVeChn, &attr);
                            if(attr.RcAttr.mRcMode!=VENC_RC_MODE_MJPEGCBR)
                            {
                                aloge("fatal error! check mpp_rcMode[0x%x]", attr.RcAttr.mRcMode);
                            }
                            attr.RcAttr.mAttrMjpegeCbr.mBitRate = RcAttr.mAttrMjpegCbr.mBitRate;
                            AW_MPI_VENC_SetChnAttr(mVeChn, &attr);
                        }
                    }
                    else
                    {
                        aloge("fatal error! unsupport vencType[0x%x]", RcAttr.mVEncType);
                    }
                }
            }
            else
            {
                aloge("fatal error! check VEncType[0x%x]->[0x%x], RcMode[0x%x]->[0x%x]", mVEncRcAttr.mVEncType, RcAttr.mVEncType, mVEncRcAttr.mRcMode, RcAttr.mRcMode);
            }
        }
        mVEncRcAttr = RcAttr;
        return NO_ERROR;
    }
    else
    {
        aloge("fatal error! VEncType[0x%x][0x%x] or RcMode[0x%x][0x%x] is not match!", RcAttr.mVEncType, mVideoEncoder, RcAttr.mRcMode, mVideoRCMode);
        return BAD_VALUE;
    }
}

status_t EyeseeRecorder::getVEncBitRateControlAttr(VEncBitRateControlAttr& RcAttr)
{
    RcAttr = mVEncRcAttr;
    return NO_ERROR;
}

/*
status_t EyeseeRecorder::setVideoEncodingBitRate(int bitRate)
{
    mVideoBitRate = bitRate;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        VENC_CHN_ATTR_S attr;
        AW_MPI_VENC_GetChnAttr(mVeChn, &attr);
        setVideoEncodingBitRateToVENC_CHN_ATTR_S(&attr, bitRate);
        return AW_MPI_VENC_SetChnAttr(mVeChn, &attr);
    }
    return NO_ERROR;
}
*/

/*status_t EyeseeRecorder::setVideoEncodingBufferTime(int nBufferTime)
{
    mVideoEncodingBufferTime = nBufferTime;
    return NO_ERROR;
}*/

/*
status_t EyeseeRecorder::SetVideoEncodingQpRange(int minqp, int maxqp)
{
    mMaxQp = maxqp;
    mMinQp = minqp;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        VENC_CHN_ATTR_S attr;
        AW_MPI_VENC_GetChnAttr(mVeChn, &attr);
        setQpRangeToVENC_CHN_ATTR_S(&attr, minqp, maxqp);
        return AW_MPI_VENC_SetChnAttr(mVeChn, &attr);
    }
    return OK;
}
*/

status_t EyeseeRecorder::setVideoEncodingIFramesNumberInterval(int nMaxKeyItl)
{
    mVideoMaxKeyItl = nMaxKeyItl;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        VENC_CHN_ATTR_S attr;
        AW_MPI_VENC_GetChnAttr(mVeChn, &attr);
        attr.VeAttr.MaxKeyInterval = nMaxKeyItl;
        return AW_MPI_VENC_SetChnAttr(mVeChn, &attr);
    }
    return OK;
}
#if 0
status_t EyeseeRecorder::setVirtualIFrameInterval(int nVirtualIFrameInterval)
{
    mVirtualIFrameInterval = nVirtualIFrameInterval;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        VENC_CHN_ATTR_S attr;
        AW_MPI_VENC_GetChnAttr(mVeChn, &attr);
        if(PT_H264 == attr.VeAttr.Type)
        {
            attr.VeAttr.AttrH264e.mVirtualIFrameInterval = nVirtualIFrameInterval;
        }
        else if(PT_H265 == attr.VeAttr.Type)
        {
            attr.VeAttr.AttrH265e.mVirtualIFrameInterval = nVirtualIFrameInterval;
        }
        else
        {
            aloge("fatal error! vencType[0x%x] don't support virtual frame!", attr.VeAttr.Type);
        }
        return AW_MPI_VENC_SetChnAttr(mVeChn, &attr);
    }
    return NO_ERROR;
}
#endif

status_t EyeseeRecorder::reencodeIFrame()
{
    if (!(mCurrentState & MEDIA_RECORDER_RECORDING))
    {
        aloge("reencodeIFrame called in an invalid state: 0x%x", mCurrentState);
        return INVALID_OPERATION;
    }

    if(mVeChn >= 0)
    {
        AW_MPI_VENC_RequestIDR(mVeChn, TRUE);
        return NO_ERROR;
    }
    else
    {
        aloge("fatal error! no venc channel!");
        return UNKNOWN_ERROR;
    }
}

status_t EyeseeRecorder::SetVideoEncodingIntraRefresh(VENC_PARAM_INTRA_REFRESH_S *pIntraRefresh)
{
    mIntraRefreshParam = *pIntraRefresh;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        if(PT_H264 == mVideoEncoder || PT_H265 == mVideoEncoder)
        {
            AW_MPI_VENC_SetIntraRefresh(mVeChn, pIntraRefresh);
        }
        else
        {
            aloge("fatal error! encoder[0x%x] don't support IntraRefresh!", mVideoEncoder);
        }
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setVideoEncodingSmartP(VencSmartFun *pParam)
{
    mSmartPParam = *pParam;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        if(PT_H264 == mVideoEncoder || PT_H265 == mVideoEncoder)
        {
            AW_MPI_VENC_SetSmartP(mVeChn, pParam);
        }
        else
        {
            aloge("fatal error! encoder[0x%x] don't support smartP!", mVideoEncoder);
        }
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setGopAttr(const VENC_GOP_ATTR_S *pParam)
{
    if(pParam)
    {
        mVEncChnAttr.GopAttr = *pParam;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::getGopAttr(VENC_GOP_ATTR_S *pParam)
{
    if(pParam)
    {    
        *pParam = mVEncChnAttr.GopAttr;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setRefParam(const VENC_PARAM_REF_S * pstRefParam)
{
    if(pstRefParam)
    {
        mVEncRefParam = *pstRefParam;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::getRefParam(VENC_PARAM_REF_S * pstRefParam)
{
    if(pstRefParam)
    {
        *pstRefParam = mVEncRefParam;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setMuteMode(bool mute)
{
    mMuteMode = mute;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        if(mAiDev >=0 && mAiChn >= 0)
        {
            AW_MPI_AI_SetChnMute(mAiDev, mAiChn, mMuteMode?TRUE:FALSE);
        }        
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setMaxDuration(int max_duration_ms)
{
    mMaxFileDuration = max_duration_ms;
    /*
    int i;
    for(i=0; i<(int)mMuxChnAttrs.size(); i++)
    {
        mMuxChnAttrs[i].mMaxFileDuration = mMaxFileDuration;
    }
    */
    for(MUX_CHN_ATTR_S& i : mMuxChnAttrs)
    {
        i.mMaxFileDuration = mMaxFileDuration;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setMaxDuration(int muxer_id, int max_duration_ms)
{
    BOOL nSuccessFlag = FALSE;
    mMaxFileDuration  = max_duration_ms;
    int idx = 0;
    for(MUX_CHN_ATTR_S& i : mMuxChnAttrs)
    {
        if (i.mMuxerId == muxer_id)
        {
            i.mMaxFileDuration = mMaxFileDuration;
            nSuccessFlag = TRUE;
            break;
        }
        idx++;
    }
    if (nSuccessFlag && (mMuxChns[idx] < MUX_MAX_CHN_NUM && mMuxChns[idx] >= 0))
    {
        MUX_CHN_ATTR_S pChnAttr;
        int ret = AW_MPI_MUX_GetChnAttr(mMuxGrp, mMuxChns[idx], &pChnAttr);
        if(SUCCESS == ret)
        {
            pChnAttr.mMaxFileDuration = mMaxFileDuration;
            AW_MPI_MUX_SetChnAttr(mMuxGrp, mMuxChns[idx], &pChnAttr);
        }
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setMaxFileSize(int64_t max_filesize_bytes)
{
    mMaxFileSizeBytes = max_filesize_bytes;
    if (mMaxFileSizeBytes > (int64_t)MAX_FILE_SIZE)
    {
        aloge("fatal error! maxFileSizeBytes[%lld] bigger than max[%lld]", mMaxFileSizeBytes, MAX_FILE_SIZE);
    }
    for(MUX_CHN_ATTR_S& i : mMuxChnAttrs)
    {
        i.mMaxFileSizeBytes = mMaxFileSizeBytes;
    }
    return OK;
}

status_t EyeseeRecorder::setAudioEncoder(PAYLOAD_TYPE_E audio_encoder)
{
    mAudioEncoder = audio_encoder;
    return NO_ERROR;
}

status_t EyeseeRecorder::setVideoEncoder(PAYLOAD_TYPE_E video_encoder)
{
    mVideoEncoder = video_encoder;
    return NO_ERROR;
}

status_t EyeseeRecorder::setAudioSamplingRate(int samplingRate)
{
    if ((samplingRate!=AUDIO_SAMPLE_RATE_8000 ) &&
        (samplingRate!=AUDIO_SAMPLE_RATE_12000) &&
        (samplingRate!=AUDIO_SAMPLE_RATE_11025) &&
        (samplingRate!=AUDIO_SAMPLE_RATE_16000) &&
        (samplingRate!=AUDIO_SAMPLE_RATE_22050) &&
        (samplingRate!=AUDIO_SAMPLE_RATE_24000) &&
        (samplingRate!=AUDIO_SAMPLE_RATE_32000) &&
        (samplingRate!=AUDIO_SAMPLE_RATE_44100) &&
        (samplingRate!=AUDIO_SAMPLE_RATE_48000) )
    {
        alogw("wrong audio SampleRate(%d) setting, change to default(8000)!", samplingRate);
        samplingRate = 8000;
    }
    mSampleRate = samplingRate;

    return NO_ERROR;
}

status_t EyeseeRecorder::setAudioChannels(int numChannels)
{
    if ((numChannels!=1) &&
        (numChannels!=2) )
    {
        alogw("wrong audio TrackCnt(%d) setting, change to default(1)!", numChannels);
        numChannels = 1;
    }
    mAudioChannels = numChannels;
    return NO_ERROR;
}

status_t EyeseeRecorder::setAudioEncodingBitRate(int bitRate)
{
    mAudioBitRate = bitRate;
    return NO_ERROR;
}

status_t EyeseeRecorder::setAudioSource(int audio_source)
{
    if (audio_source != AudioSource::MIC && audio_source != AudioSource::DEFAULT)
    {
		aloge("not support audio source[0x%x] now", audio_source);
		return BAD_VALUE;
	}
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_IDLE) 
    {
        alogv("Call init() since the media recorder is not initialized yet");
        status_t ret = init();
        if (OK != ret) 
        {
            return ret;
        }
    }
    if (mIsAudioSourceSet) 
    {
        aloge("audio source has already been set");
        return INVALID_OPERATION;
    }
    if (!(mCurrentState & MEDIA_RECORDER_INITIALIZED)) 
    {
        aloge("setAudioSource called in an invalid state(%d)", mCurrentState);
        return INVALID_OPERATION;
    }

    mAudioSource = audio_source;

    mIsAudioSourceSet = true;
    return NO_ERROR;
}

status_t EyeseeRecorder::setVideoSource(int video_source)
{
    if (video_source != VideoSource::CAMERA && video_source != VideoSource::DEFAULT)
    {
		aloge("not support video source[0x%x] now", video_source);
		return BAD_VALUE;
	}
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_IDLE) 
    {
        alogv("Call init() since the media recorder is not initialized yet");
        status_t ret = init();
        if (OK != ret) 
        {
            return ret;
        }
    }
    if (mIsVideoSourceSet) 
    {
        aloge("video source has already been set");
        return INVALID_OPERATION;
    }
    if (!(mCurrentState & MEDIA_RECORDER_INITIALIZED)) 
    {
        aloge("setVideoSource called in an invalid state(%d)", mCurrentState);
        return INVALID_OPERATION;
    }

    mVideoSource = video_source;

    mIsVideoSourceSet = true;
    return NO_ERROR;
}

int EyeseeRecorder::addOutputFormatAndOutputSink(MEDIA_FILE_FORMAT_E output_format, int fd, int FallocateLen, bool callback_out_flag)
{
    int retMuxerId = -1;
    alogv("addOutputFormatAndOutputSink(%d)", output_format);
    Mutex::Autolock autoLock(mLock);
    if (!(mCurrentState & MEDIA_RECORDER_INITIALIZED) && !(mCurrentState & MEDIA_RECORDER_DATASOURCE_CONFIGURED) && !(mCurrentState & MEDIA_RECORDER_RECORDING)) 
    {
        aloge("addOutputFormatAndOutputSink called in an invalid state: %d", mCurrentState);
        return -1;
    }
    if (mIsVideoSourceSet && judgeAudioFileFormat(output_format)) 
    { //first non-video output format
        aloge("output format (%d) is meant for audio recording only and incompatible with video recording", output_format);
        return -1;
    }

    alogd("(of:0x%x, fd:%d, FallocateLen:%d, callback_out_flag:%d)", output_format, fd, FallocateLen, callback_out_flag);
    if(fd >= 0 && true == callback_out_flag)
    {
        aloge("fatal error! one muxer cannot support two sink methods!");
        return -1;
    }
    //find if the same output_format sinkInfo exist or callback out stream is exist.
    for(std::vector<OutputSinkInfo>::iterator it = mSinkInfos.begin(); it != mSinkInfos.end(); ++it)
    {
        if(it->mOutputFormat == output_format)
        {
            alogd("Be careful! same outputForamt[0x%x] exist in array", output_format);
        }
        if(callback_out_flag && it->mCallbackOutFlag == callback_out_flag)
        {
            aloge("fatal error! only support one callback out stream.");
        }
    }
    OutputSinkInfo sinkInfo;
    sinkInfo.mMuxerId = mMuxerIdCounter;
    sinkInfo.mOutputFormat = output_format;
    if(fd >= 0)
    {
        sinkInfo.mOutputFd = dup(fd);
        //sinkInfo.mOutputFd = dup2SeldomUsedFd(fd);
    }
    else
    {
        sinkInfo.mOutputFd = -1;
    }
    sinkInfo.mFallocateLen = FallocateLen;
    sinkInfo.mCallbackOutFlag = callback_out_flag;
    mIsOutputFileSet = true;

    //config mMuxChnAttrs.
    MUX_CHN_ATTR_S muxChnAttr;
    muxChnAttr.mMuxerId = sinkInfo.mMuxerId;
    muxChnAttr.mMediaFileFormat = sinkInfo.mOutputFormat;
    muxChnAttr.mMaxFileDuration = mMaxFileDuration;
    muxChnAttr.mMaxFileSizeBytes = mMaxFileSizeBytes;
    muxChnAttr.mFallocateLen = sinkInfo.mFallocateLen;
    muxChnAttr.mCallbackOutFlag = sinkInfo.mCallbackOutFlag;
    muxChnAttr.mFsWriteMode = FSWRITEMODE_SIMPLECACHE;
    muxChnAttr.mSimpleCacheSize = DEFAULT_SIMPLE_CACHE_SIZE_VFS;
    muxChnAttr.bBufFromCacheFlag = false;

    if(mCurrentState==MEDIA_RECORDER_PREPARED || mCurrentState==MEDIA_RECORDER_RECORDING)
    {
        ERRORTYPE ret;
        BOOL nSuccessFlag = FALSE;
        MUX_CHN nMuxChn = 0;
        while(nMuxChn < MUX_MAX_CHN_NUM)
        {
            ret = AW_MPI_MUX_CreateChn(mMuxGrp, nMuxChn, &muxChnAttr, sinkInfo.mOutputFd);
            if(SUCCESS == ret)
            {
                nSuccessFlag = TRUE;
                alogd("create mux group[%d] channel[%d] success, muxerId[%d]!", mMuxGrp, nMuxChn, muxChnAttr.mMuxerId);
                break;
            }
            else if(ERR_MUX_EXIST == ret)
            {
                alogv("mux group[%d] channel[%d] is exist, find next!", mMuxGrp, nMuxChn);
                nMuxChn++;
            }
            else
            {
                aloge("fatal error! create mux group[%d] channel[%d] fail ret[0x%x], find next!", mMuxGrp, nMuxChn, ret);
                nMuxChn++;
            }
        }
        if(nSuccessFlag)
        {
            mMuxChns.push_back(nMuxChn);
            mSinkInfos.push_back(sinkInfo);
            mMuxChnAttrs.push_back(muxChnAttr);
            mPolicy.insert(std::make_pair(muxChnAttr.mMuxerId, RecordFileDurationPolicy_AverageDuration));
            retMuxerId = sinkInfo.mMuxerId;
            mMuxerIdCounter++;
        }
        else
        {
            aloge("fatal error! create mux group[%d] channel fail!", mMuxGrp);
            if(sinkInfo.mOutputFd>=0)
            {
                ::close(sinkInfo.mOutputFd);
                sinkInfo.mOutputFd = -1;
            }
            retMuxerId = -1;
        }
    }
    else
    {
        mMuxChns.push_back(MM_INVALID_CHN);
        mSinkInfos.push_back(sinkInfo);
        mMuxChnAttrs.push_back(muxChnAttr);
        retMuxerId = sinkInfo.mMuxerId;
        mPolicy.insert(std::make_pair(muxChnAttr.mMuxerId, RecordFileDurationPolicy_AverageDuration));
        mMuxerIdCounter++;
    }

    if ((mCurrentState & MEDIA_RECORDER_INITIALIZED))
    {
        mCurrentState = MEDIA_RECORDER_DATASOURCE_CONFIGURED;
    }
    return retMuxerId;
}

int EyeseeRecorder::addOutputFormatAndOutputSink(MEDIA_FILE_FORMAT_E output_format, char* path, int FallocateLen, bool callback_out_flag)
{
    int muxerId = -1;
    if(path!=NULL)
    {
        int fd = open(path, O_RDWR | O_CREAT, 0666);
    	if (fd < 0) 
        {
    		aloge("Failed to open %s", path);
    		return -1;
    	}
        muxerId = addOutputFormatAndOutputSink(output_format, fd, FallocateLen, callback_out_flag);
        ::close(fd);
    }
    return muxerId;
}

int EyeseeRecorder::addOutputSink(SinkParam *pSinkParam)
{
    int retMuxerId = -1;
    Mutex::Autolock autoLock(mLock);
    alogd("addOutputSink Format:0x%x, fd:%d, path:%s, FallocateLen:%d, callback_out_flag:%d, cache_flag:%d",
           pSinkParam->mOutputFormat, pSinkParam->mOutputFd, pSinkParam->mOutputPath, pSinkParam->mFallocateLen ,
           pSinkParam->bCallbackOutFlag, pSinkParam->bBufFromCacheFlag);
    if (!(mCurrentState & MEDIA_RECORDER_INITIALIZED) && !(mCurrentState & MEDIA_RECORDER_DATASOURCE_CONFIGURED) && !(mCurrentState & MEDIA_RECORDER_RECORDING)) 
    {
        aloge("fatal error! addOutputSink called in an invalid state: %d", mCurrentState);
        return -1;
    }

    for(MUX_CHN_ATTR_S& i : mMuxChnAttrs)
    {
        if (TRUE == i.bBufFromCacheFlag)
        {
            alogw("Be careful! muxer %d GetBufFromCache has already exists!",i.mMuxerId);
            return ALREADY_EXISTS;
        }
    }

    OutputSinkInfo sinkInfo;
    sinkInfo.mMuxerId = mMuxerIdCounter;
    if (pSinkParam->mOutputFormat >= MEDIA_FILE_FORMAT_DEFAULT && pSinkParam->mOutputFormat < MEDIA_FILE_FORMAT_UNKNOWN)
    {
        sinkInfo.mOutputFormat = pSinkParam->mOutputFormat;
    }
    else
    {
        aloge("fatal error! called in an invalid OutputFormat: %d", pSinkParam->mOutputFormat);
        return -1;
    }

    if(pSinkParam->mOutputFd >= 0)
    {
        sinkInfo.mOutputFd = dup(pSinkParam->mOutputFd);
    }
    else
    {
        sinkInfo.mOutputFd = -1;
        if(pSinkParam->mOutputPath != NULL)
        {
            int fd = open(pSinkParam->mOutputPath, O_RDWR | O_CREAT, 0666);
            if (fd < 0)
            {
                aloge("Failed to open %s", pSinkParam->mOutputPath);
                return -1;
            }
            sinkInfo.mOutputFd = dup(fd);
            ::close(fd);
        }
        else
        {
            aloge("fatal error! mOutputPath is NULL, mOutputFd < 0 !");
            return -1;
        }
    }

    if((pSinkParam->mOutputFd >= 0 || pSinkParam->mOutputPath != NULL) && true == pSinkParam->bCallbackOutFlag)
    {
        aloge("fatal error! one muxer cannot support two sink methods!");
        return -1;
    }

    //if (pSinkParam->mMaxDurationMs <= 0)
    //{
    //    // set default value 30 seconds.
    //    pSinkParam->mMaxDurationMs = 30*1000;
    //}
    sinkInfo.mFallocateLen = pSinkParam->mFallocateLen;
    sinkInfo.mCallbackOutFlag = pSinkParam->bCallbackOutFlag;

    //config mMuxChnAttrs.
    MUX_CHN_ATTR_S muxChnAttr;
    muxChnAttr.mMuxerId = sinkInfo.mMuxerId;
    muxChnAttr.mMediaFileFormat = sinkInfo.mOutputFormat;
    muxChnAttr.mMaxFileDuration = pSinkParam->mMaxDurationMs;
    muxChnAttr.mMaxFileSizeBytes = mMaxFileSizeBytes;
    muxChnAttr.mFallocateLen = sinkInfo.mFallocateLen;
    muxChnAttr.mCallbackOutFlag = sinkInfo.mCallbackOutFlag;
    muxChnAttr.mFsWriteMode = FSWRITEMODE_SIMPLECACHE;
    muxChnAttr.mSimpleCacheSize = DEFAULT_SIMPLE_CACHE_SIZE_VFS;
    muxChnAttr.bBufFromCacheFlag = pSinkParam->bBufFromCacheFlag;
    if(mCurrentState==MEDIA_RECORDER_PREPARED || mCurrentState==MEDIA_RECORDER_RECORDING)
    {
        ERRORTYPE ret;
        BOOL nSuccessFlag = FALSE;
        MUX_CHN nMuxChn = 0;
        while(nMuxChn < MUX_MAX_CHN_NUM)
        {
            ret = AW_MPI_MUX_CreateChn(mMuxGrp, nMuxChn, &muxChnAttr, sinkInfo.mOutputFd);
            if(SUCCESS == ret)
            {
                nSuccessFlag = TRUE;
                alogd("create mux group[%d] channel[%d] success, muxerId[%d]!", mMuxGrp, nMuxChn, muxChnAttr.mMuxerId);
                break;
            }
            else if(ERR_MUX_EXIST == ret)
            {
                alogv("mux group[%d] channel[%d] is exist, find next!", mMuxGrp, nMuxChn);
                nMuxChn++;
            }
            else
            {
                aloge("fatal error! create mux group[%d] channel[%d] fail ret[0x%x], find next!", mMuxGrp, nMuxChn, ret);
                nMuxChn++;
            }
        }
        if(nSuccessFlag)
        {
            mMuxChns.push_back(nMuxChn);
            mSinkInfos.push_back(sinkInfo);
            mMuxChnAttrs.push_back(muxChnAttr);
            retMuxerId = sinkInfo.mMuxerId;
            mMuxerIdCounter++;
        }
        else
        {
            aloge("fatal error! create mux group[%d] channel fail!", mMuxGrp);
            if(sinkInfo.mOutputFd>=0)
            {
                ::close(sinkInfo.mOutputFd);
                sinkInfo.mOutputFd = -1;
            }
            retMuxerId = -1;
        }
    }
    else
    {
        mMuxChns.push_back(MM_INVALID_CHN);
        mSinkInfos.push_back(sinkInfo);
        mMuxChnAttrs.push_back(muxChnAttr);
        retMuxerId = sinkInfo.mMuxerId;
        mMuxerIdCounter++;
    }

    return retMuxerId;
}

status_t EyeseeRecorder::prepare()
{
    status_t result = UNKNOWN_ERROR;
    Mutex::Autolock autoLock(mLock);
    if (!(mCurrentState & MEDIA_RECORDER_DATASOURCE_CONFIGURED))
    {
        aloge("prepare called in an invalid state: 0x%x", mCurrentState);
        return INVALID_OPERATION;
    }
    if (mVideoEncoder != PT_MAX) 
    {
        //create CameraFrameManager
        if(mpInputFrameManager!=NULL)
        {
            aloge("fatal error! inputFrameManager must null!");
            delete mpInputFrameManager;
            mpInputFrameManager = NULL;
        }
        mpInputFrameManager = new CameraFrameManager(mpCameraProxy, mCameraSourceChannel);
        //create venc channel.
        bool nSuccessFlag = false;
        ERRORTYPE ret;
        config_VENC_CHN_ATTR_S();
        mVeChn = 0;
        while(mVeChn < VENC_MAX_CHN_NUM)
        {
            ret = AW_MPI_VENC_CreateChn(mVeChn, &mVEncChnAttr);
            if(SUCCESS == ret)
            {
                nSuccessFlag = true;
                alogd("create venc channel[%d] success!", mVeChn);
                break;
            }
            else if(ERR_VENC_EXIST == ret)
            {
                alogv("venc channel[%d] is exist, find next!", mVeChn);
                mVeChn++;
            }
            else
            {
                alogd("create venc channel[%d] ret[0x%x], find next!", mVeChn, ret);
                mVeChn++;
            }
        }
        if(false == nSuccessFlag)
        {
            mVeChn = MM_INVALID_CHN;
            aloge("fatal error! create venc channel fail!");
            result = UNKNOWN_ERROR;
            goto _err0;
        }

        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)this;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_VENC_RegisterCallback(mVeChn, &cbInfo);

        CameraParameters cameraParam;
        mpCameraProxy->getParameters(mCameraSourceChannel, cameraParam);
        VENC_FRAME_RATE_S stFrameRate;
        stFrameRate.SrcFrmRate = cameraParam.getPreviewFrameRate();
        stFrameRate.DstFrmRate = mFrameRate;
        AW_MPI_VENC_SetFrameRate(mVeChn, &stFrameRate);
        if(mTimeLapseEnable)
        {
            AW_MPI_VENC_SetTimeLapse(mVeChn, mTimeBetweenFrameCapture);
        }
        //IntraRefresh
        AW_MPI_VENC_SetIntraRefresh(mVeChn, &mIntraRefreshParam);
        //smartP
        if(PT_H264 == mVideoEncoder || PT_H265 == mVideoEncoder)
        {
            AW_MPI_VENC_SetSmartP(mVeChn, &mSmartPParam);
        }

        //AW_MPI_VENC_SetVEFreq(mVeChn, 534);

        AW_MPI_VENC_SetRefParam(mVeChn, &mVEncRefParam);

        AW_MPI_VENC_SetHorizonFlip(mVeChn, mbHorizonfilp);
        AW_MPI_VENC_SetAdaptiveIntraInP(mVeChn, mbAdaptiveintrainp);
        AW_MPI_VENC_Set3DNR(mVeChn, mb3DNR);
        AW_MPI_VENC_SetSuperFrameCfg(mVeChn, &mVencSuperFrameCfg);
        AW_MPI_VENC_SaveBsFile(mVeChn, &mSaveBSFileParam);
        AW_MPI_VENC_SetProcSet(mVeChn, &mVeProcSet);

        VENC_COLOR2GREY_S  tColor2Grey;
        tColor2Grey.bColor2Grey = mbColor2Grey;
        AW_MPI_VENC_SetColor2Grey(mVeChn, &tColor2Grey);

        if(mNullSkipEnable)
        {
            AW_MPI_VENC_EnableNullSkip(mVeChn, (BOOL)mNullSkipEnable);
        }
        if(mPSkipEnable)
        {
            AW_MPI_VENC_EnablePSkip(mVeChn, (BOOL)mPSkipEnable);
        }

        rec_start_timestamp = -1;
    }

    if (!mTimeLapseEnable && mAudioEncoder != PT_MAX)
    {
        mAiDev = 0;
        config_AIO_ATTR_S();
        AW_MPI_AI_SetPubAttr(mAiDev, &mAioAttr);

        //enable audio_hw_ai
        AW_MPI_AI_Enable(mAiDev);

        //create ai channel.
        ERRORTYPE ret;
        BOOL nSuccessFlag = FALSE;
        mAiChn = 0;
        while (mAiChn < AIO_MAX_CHN_NUM) {
            ret = AW_MPI_AI_CreateChn(mAiDev, mAiChn);
            if (SUCCESS == ret)
            {
                nSuccessFlag = TRUE;
                alogd("create ai channel[%d] success!", mAiChn);
                break;
            }
            else if (ERR_AI_EXIST == ret)
            {
                alogv("ai channel[%d] exist, find next!", mAiChn);
                mAiChn++;
            }
            else if (ERR_AI_NOT_ENABLED == ret)
            {
                aloge("audio_hw_ai not started!");
                break;
            }
            else
            {
                aloge("create ai channel[%d] fail! ret[0x%x]!", mAiChn, ret);
                break;
            }
        }
        if(FALSE == nSuccessFlag)
        {
            mAiChn = MM_INVALID_CHN;
            aloge("fatal error! create ai channel fail!");
            result = UNKNOWN_ERROR;
            goto _err0;
        }
        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)this;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_AI_RegisterCallback(mAiDev, mAiChn, &cbInfo);
        AW_MPI_AI_SetChnMute(mAiDev, mAiChn, mMuteMode?TRUE:FALSE);

        //create aenc channel.
        nSuccessFlag = FALSE;
        config_AENC_CHN_ATTR_S();
        mAeChn = 0;
        while(mAeChn < AENC_MAX_CHN_NUM)
        {
            ret = AW_MPI_AENC_CreateChn(mAeChn, &mAEncChnAttr);
            if(SUCCESS == ret)
            {
                nSuccessFlag = TRUE;
                alogd("create aenc channel[%d] success!", mAeChn);
                break;
            }
            else if(ERR_AENC_EXIST == ret)
            {
                alogv("aenc channel[%d] exist, find next!", mAeChn);
                mAeChn++;
            }
            else
            {
                alogd("create aenc channel[%d] ret[0x%x], find next!", mAeChn, ret);
                mAeChn++;
            }
        }
        if(FALSE == nSuccessFlag)
        {
            mAeChn = MM_INVALID_CHN;
            aloge("fatal error! create aenc channel fail!");
            result = UNKNOWN_ERROR;
            goto _err0;
        }
        AW_MPI_AENC_RegisterCallback(mAeChn, &cbInfo);
    }

    if(mVeChn >= 0 || mAeChn>=0)
    {
        //create mux group.
        bool nSuccessFlag = false;
        ERRORTYPE ret;
        MPPCallbackInfo cbInfo;

        if(gps_state)
        {
            config_TENC_CHN_ATTR_S();
            mTeChn = 0;
            while(mTeChn < TENC_MAX_CHN_NUM)
            {
                ret = AW_MPI_TENC_CreateChn(mTeChn, &mTEncChnAttr);
                if(SUCCESS == ret)
                {
                    nSuccessFlag = TRUE;
                    alogd("create tenc channel[%d] success!", mTeChn);
                    break;
                }
                else if(ERR_TENC_EXIST == ret)
                {
                    alogv("tenc channel[%d] exist, find next!", mTeChn);
                    mTeChn++;
                }
                else
                {
                    alogd("create tenc channel[%d] ret[0x%x], find next!", mTeChn, ret);
                    mTeChn++;
                }
            }
            if(FALSE == nSuccessFlag)
            {
                mTeChn = MM_INVALID_CHN;
                aloge("fatal error! create aenc channel fail!");
                result = UNKNOWN_ERROR;
                goto _err0;
            }
            
            cbInfo.cookie = (void*)this;
            cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
            AW_MPI_TENC_RegisterCallback(mTeChn, &cbInfo);
        }
        
        nSuccessFlag = false;
        config_MUX_GRP_ATTR_S();
        mMuxGrp = 0;
        while(mMuxGrp < MUX_MAX_GRP_NUM)
        {
            ret = AW_MPI_MUX_CreateGrp(mMuxGrp, &mMuxGrpAttr);
            if(SUCCESS == ret)
            {
                nSuccessFlag = true;
                alogd("create mux group[%d] success!", mMuxGrp);
                break;
            }
            else if(ERR_MUX_EXIST == ret)
            {
                alogv("mux group[%d] is exist, find next!", mMuxGrp);
                mMuxGrp++;
            }
            else
            {
                alogd("create mux group[%d] ret[0x%x], find next!", mMuxGrp, ret);
                mMuxGrp++;
            }
        }
        if(false == nSuccessFlag)
        {
            mMuxGrp = MM_INVALID_CHN;
            aloge("fatal error! create mux group fail!");
            result = UNKNOWN_ERROR;
            goto _err0;
        }
        //set cache duration to muxGroup.
        if(mMuxCacheDuration >= 0)
        {
            AW_MPI_MUX_SetMuxCacheDuration(mMuxGrp, mMuxCacheDuration);
        }
        //set callback
//        MPPCallbackInfo cbInfo;
        cbInfo.cookie = (void*)this;
        cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
        AW_MPI_MUX_RegisterCallback(mMuxGrp, &cbInfo);

        //set spspps
        if(mVeChn >= 0)
        {
            if(mVideoEncoder == PT_H264)
            {
                int venc_ret = 0;   
                VencHeaderData H264SpsPpsInfo;
                memset(&H264SpsPpsInfo,0,sizeof(H264SpsPpsInfo));
                venc_ret = AW_MPI_VENC_GetH264SpsPpsInfo(mVeChn, &H264SpsPpsInfo);
                if(SUCCESS == venc_ret) // failure process to avoid using of null pointer
                {
                    AW_MPI_MUX_SetH264SpsPpsInfo(mMuxGrp, &H264SpsPpsInfo);
                }
                else
                {
                    result = UNKNOWN_ERROR;
                    goto _err0;
                }
            }
            else if(mVideoEncoder == PT_H265)
            {
                int venc_ret = 0;   
                VencHeaderData H265SpsPpsInfo;
                memset(&H265SpsPpsInfo,0,sizeof(H265SpsPpsInfo));
                venc_ret = AW_MPI_VENC_GetH265SpsPpsInfo(mVeChn, &H265SpsPpsInfo);
                if(SUCCESS == venc_ret)
                {
                    AW_MPI_MUX_SetH265SpsPpsInfo(mMuxGrp, &H265SpsPpsInfo);
                }
                else
                {
                    result = UNKNOWN_ERROR;
                    goto _err0;
                }
            }
        }
        //create mux channel in group.
        alogd("there is [%d]=[%d] mux channel need to create", mMuxChnAttrs.size(), mMuxChns.size());
        unsigned int i;
        for(i=0; i<mMuxChnAttrs.size(); i++)
        {
            mMuxChns[i] = 0;;
            BOOL nSuccessFlag = FALSE;
            while(mMuxChns[i] < MUX_MAX_CHN_NUM)
            {
                ret = AW_MPI_MUX_CreateChn(mMuxGrp, mMuxChns[i], &mMuxChnAttrs[i], mSinkInfos[i].mOutputFd);
                if(SUCCESS == ret)
                {
                    nSuccessFlag = TRUE;
                    alogd("create mux group[%d] channel[%d] success, muxerId[%d]!", mMuxGrp, mMuxChns[i], mMuxChnAttrs[i].mMuxerId);
                    break;
                }
                else if(ERR_MUX_EXIST == ret)
                {
                    alogv("mux group[%d] channel[%d] is exist, find next!", mMuxGrp, mMuxChns[i]);
                    ++mMuxChns[i];
                }
                else
                {
                    alogd("create mux group[%d] channel[%d] fail ret[0x%x], find next!", mMuxGrp, mMuxChns[i], ret);
                    ++mMuxChns[i];
                }
            }
            if(FALSE == nSuccessFlag)
            {
                mMuxChns[i] = MM_INVALID_CHN;
                aloge("fatal error! create mux group[%d] channel fail!", mMuxGrp);
                result = UNKNOWN_ERROR;
                goto _err0;
            }
        }
        
        //bind previous component
        if (mVeChn >= 0) 
        {
            MPP_CHN_S VeChn{MOD_ID_VENC, 0, mVeChn};
            MPP_CHN_S MuxGrp{MOD_ID_MUX, 0, mMuxGrp};
            AW_MPI_SYS_Bind(&VeChn, &MuxGrp);
        }
        if (mAeChn >= 0)
        {
            MPP_CHN_S AiChn{MOD_ID_AI, mAiDev, mAiChn};
            MPP_CHN_S AeChn{MOD_ID_AENC, 0, mAeChn};
            MPP_CHN_S MuxGrp{MOD_ID_MUX, 0, mMuxGrp};
            AW_MPI_SYS_Bind(&AiChn, &AeChn);
            AW_MPI_SYS_Bind(&AeChn, &MuxGrp);
        }
        if (mTeChn >= 0)
        {
            MPP_CHN_S TeChn{MOD_ID_TENC, 0, mTeChn};
            MPP_CHN_S MuxGrp{MOD_ID_MUX, 0, mMuxGrp};
            AW_MPI_SYS_Bind(&TeChn, &MuxGrp);
        }

        
    }

    if (mVeChn>=0 && mAeChn>=0) 
    {
        mAVSync = RecAVSyncConstruct();
        if (mAVSync == NULL) 
        {
            aloge("fatal error! RecAVSync Construct failed!");
        }
        mAVSync->SetAudioInfo(mAVSync, mAEncChnAttr.AeAttr.sampleRate, mAEncChnAttr.AeAttr.channels, mAEncChnAttr.AeAttr.bitsPerSample);
    }
    mCurrentState = MEDIA_RECORDER_PREPARED;
    return NO_ERROR;
_err0:
    return result;
}

status_t EyeseeRecorder::start()
{
    Mutex::Autolock autoLock(mLock);
    if (!(mCurrentState & MEDIA_RECORDER_PREPARED)) 
    {
        aloge("start called in an invalid state: %d", mCurrentState);
        return INVALID_OPERATION;
    }
    //alogd("EyeseeRecorder[%p] start, videoSize[%dx%d]!", this, mVideoWidth, mVideoHeight);
    status_t result = NO_ERROR;
    Mutex::Autolock autoLock2(mSendFrameLock);
    if(mpCameraProxy)
    {
        mpCameraProxy->startRecording(mCameraSourceChannel, new CameraProxyListener(this), mRecorderId);
    }

    if(mEnableDBRC)
    {
        if (mVideoRCMode==VideoRCMode_CBR)
        {
            mpDBRC = new DynamicBitRateControl(this);
            if (mpDBRC == NULL)
            {
                aloge("fatal error! DynamicBitRateControl construct fail!");
            }
            else
            {
                alogd("Start DBRC to control VEnc bitrate when write_card speed is low!");
            }
        }
        else
        {
            alogw("Dynamic BitRate Control only run when VideoRCMode_CBR, but yours: %d!", mVideoRCMode);
        }
    }

    if(mVeChn >= 0)
    {
        int venc_ret = 0;
        venc_ret = AW_MPI_VENC_StartRecvPic(mVeChn);
        if(SUCCESS != venc_ret)
        {
            aloge("fatal error:%x rec AW_MPI_VENC_StartRecvPic",venc_ret);
            if(ERR_VENC_NOMEM == venc_ret)
            {
                result = NO_MEMORY;
            }
            else
            {
                result = UNKNOWN_ERROR;
            }
        }
    }

    if(mAiChn >= 0)
    {
        AW_MPI_AI_EnableChn(mAiDev, mAiChn);
    }
    if(mAeChn >= 0)
    {
        AW_MPI_AENC_StartRecvPcm(mAeChn);
    }
    if(mTeChn >= 0)
    {
        AW_MPI_TENC_StartRecvText(mTeChn);
    }
    if(mMuxGrp >= 0)
    {
        AW_MPI_MUX_StartGrp(mMuxGrp);
    }
    mCurrentState = MEDIA_RECORDER_RECORDING;
    return result;
}

status_t EyeseeRecorder::stop(bool bShutDownNowFlag)
{
    status_t ret = NO_ERROR;
    alogv("stop");
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState != MEDIA_RECORDER_RECORDING && mCurrentState != MEDIA_RECORDER_PAUSE)
    {
        aloge("stop called in an invalid state: %d", mCurrentState);
        return INVALID_OPERATION;
    }
    stop_l(bShutDownNowFlag);
    doCleanUp();
    mCurrentState = MEDIA_RECORDER_IDLE;
    return ret;
}

status_t EyeseeRecorder::pause(bool bPause)
{
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState != MEDIA_RECORDER_RECORDING && mCurrentState != MEDIA_RECORDER_PAUSE)
    {
        aloge("stop called in an invalid state: %d", mCurrentState);
        return INVALID_OPERATION;
    }
    if((bPause && mCurrentState==MEDIA_RECORDER_PAUSE) || (bPause==false && mCurrentState==MEDIA_RECORDER_RECORDING))
    {
        alogd("already in state[0x%x]", mCurrentState);
        return NO_ERROR;
    }
    Mutex::Autolock autoLock2(mSendFrameLock);
    if(mAiDev >=0 && mAiChn >= 0)
    {
        AW_MPI_AI_IgnoreData(mAiDev, mAiChn, bPause?TRUE:FALSE);
    }
    if (bPause == true)
        mCurrentState = MEDIA_RECORDER_PAUSE;
    else
        mCurrentState = MEDIA_RECORDER_RECORDING;
    return NO_ERROR;
}

status_t EyeseeRecorder::reset()
{
    alogv("reset");
    Mutex::Autolock autoLock(mLock);
    //doCleanUp();
    status_t ret = UNKNOWN_ERROR;
    switch (mCurrentState) 
    {
        case MEDIA_RECORDER_IDLE:
            ret = OK;
            break;
        case MEDIA_RECORDER_PREPARED:
        case MEDIA_RECORDER_RECORDING:
        case MEDIA_RECORDER_DATASOURCE_CONFIGURED:
        case MEDIA_RECORDER_PAUSE:
        case MEDIA_RECORDER_ERROR: 
        {
            ret = doReset();
            if (OK != ret) 
            {
                return ret;  // No need to continue
            }
        }  // Intentional fall through
        case MEDIA_RECORDER_INITIALIZED:
            ret = close();
            break;

        default: 
        {
            aloge("Unexpected non-existing state: %d", mCurrentState);
            break;
        }
    }
    doCleanUp();
    return ret;
}

status_t EyeseeRecorder::init()
{
    alogv("init");
    if (!(mCurrentState & MEDIA_RECORDER_IDLE)) 
    {
        aloge("init called in an invalid state(%d)", mCurrentState);
        return INVALID_OPERATION;
    }

    mCurrentState = MEDIA_RECORDER_INITIALIZED;
    return NO_ERROR;
}

status_t EyeseeRecorder::close()
{
    alogv("close");
    if (!(mCurrentState & MEDIA_RECORDER_INITIALIZED))
    {
        aloge("close called in an invalid state: %d", mCurrentState);
        return INVALID_OPERATION;
    }
    mCurrentState = MEDIA_RECORDER_IDLE;
    return NO_ERROR;
}

status_t EyeseeRecorder::doReset()
{
    alogv("doReset");
    if (!(mCurrentState & MEDIA_RECORDER_PREPARED) 
        && !(mCurrentState & MEDIA_RECORDER_RECORDING) 
        && !(mCurrentState & MEDIA_RECORDER_DATASOURCE_CONFIGURED) 
        && !(mCurrentState & MEDIA_RECORDER_ERROR))
    {
        aloge("doReset called in an invalid state: %d", mCurrentState);
        return INVALID_OPERATION;
    }
    status_t ret = stop_l();
    if (OK != ret) 
    {
        aloge("doReset failed: %d", ret);
        mCurrentState = MEDIA_RECORDER_ERROR;
        return ret;
    } 
    else 
    {
        mCurrentState = MEDIA_RECORDER_INITIALIZED;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::stop_l(bool bShutDownNowFlag)
{
    if(mpCameraProxy)
    {
        mpCameraProxy->stopRecording(mCameraSourceChannel, mRecorderId);
    }
    
    if (mpDBRC != NULL)
    {
        delete mpDBRC;
        mpDBRC = NULL;
    }

    if(mVeChn >= 0)
    {
        AW_MPI_VENC_StopRecvPic(mVeChn);
    }
    
    if(mTeChn >= 0)
    {
        AW_MPI_TENC_StopRecvText(mTeChn);
    } 
    
    if(mAiChn >= 0)
    {
        AW_MPI_AI_DisableChn(mAiDev, mAiChn);
    }
    if(mAeChn >= 0)
    {
        AW_MPI_AENC_StopRecvPcm(mAeChn);
    }
    if(mMuxGrp >= 0)
    {
        AW_MPI_MUX_StopGrp(mMuxGrp);
    }

    if(mMuxGrp >= 0)
    {
        //AW_MPI_MUX_DestroyGrp(mMuxGrp);
        AW_MPI_MUX_DestroyGrpEx(mMuxGrp, (BOOL)bShutDownNowFlag);
        mMuxChns.clear();
        mMuxChnAttrs.clear();
        mMuxGrp = MM_INVALID_CHN;
    }
    if(mVeChn >= 0)
    {
        AW_MPI_VENC_ResetChn(mVeChn);
        AW_MPI_VENC_DestroyChn(mVeChn);
        mVeChn = MM_INVALID_CHN;
    }
    if(mTeChn >= 0)
    {
        AW_MPI_TENC_ResetChn(mTeChn); 
        
        AW_MPI_TENC_DestroyChn(mTeChn); 
        mTeChn = MM_INVALID_CHN;
    }
    if(mAiChn >= 0)
    {
        AW_MPI_AI_ResetChn(mAiDev, mAiChn);
        AW_MPI_AI_DestroyChn(mAiDev, mAiChn);
        mAiChn = MM_INVALID_CHN;
    }
    if(mAeChn >= 0)
    {
        AW_MPI_AENC_ResetChn(mAeChn);
        AW_MPI_AENC_DestroyChn(mAeChn);
        mAeChn = MM_INVALID_CHN;
    }
    for(std::vector<OutputSinkInfo>::iterator it = mSinkInfos.begin(); it != mSinkInfos.end(); ++it)
    {
        if(it->mOutputFd >= 0)
        {
            ::close(it->mOutputFd);
            it->mOutputFd = -1;
        }
    }
    mSinkInfos.clear();

    if(mpInputFrameManager!=NULL)
    {
        delete mpInputFrameManager;
        mpInputFrameManager = NULL;
    }
    if(mpCameraProxy)
    {
        delete mpCameraProxy;
        mpCameraProxy = NULL;
    }
    if (mAVSync) 
    {
        RecAVSyncDestruct(mAVSync);
        mAVSync = NULL;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::reset_l()
{
    alogv("reset_l");
    //doCleanUp();
    status_t ret = UNKNOWN_ERROR;
    switch (mCurrentState) 
    {
        case MEDIA_RECORDER_IDLE:
            ret = OK;
            break;
        case MEDIA_RECORDER_PREPARED:
        case MEDIA_RECORDER_RECORDING:
        case MEDIA_RECORDER_DATASOURCE_CONFIGURED:
        case MEDIA_RECORDER_ERROR: 
        {
            ret = doReset();
            if (OK != ret) 
            {
                return ret;  // No need to continue
            }
        }  // Intentional fall through
        case MEDIA_RECORDER_INITIALIZED:
            ret = close();
            break;

        default: 
        {
            aloge("Unexpected non-existing state: %d", mCurrentState);
            break;
        }
    }
    doCleanUp();
    return ret;
}

void EyeseeRecorder::setOnErrorListener(OnErrorListener *pl)
{
    mOnErrorListener = pl;
}

void EyeseeRecorder::setOnInfoListener(OnInfoListener *pListener)
{
	mOnInfoListener = pListener;
}

void EyeseeRecorder::setOnDataListener(OnDataListener *pListener)
{
	mOnDataListener = pListener;
}

status_t EyeseeRecorder::setBsFrameRawDataType(callback_out_data_type type)
{
    mCallbackOutDataType = type;
    return NO_ERROR;
}

void EyeseeRecorder::dataCallbackTimestamp(const VIDEO_FRAME_BUFFER_S *pCameraFrameInfo)
{
    //Mutex::Autolock autoLock(mLock);
    Mutex::Autolock autoLock2(mSendFrameLock);
    media_recorder_states eCurState = mCurrentState;
    // if encoder is stopped,release this frame.if encoder is paused ,because need to avsync so continue.
    if(eCurState != MEDIA_RECORDER_RECORDING && eCurState != MEDIA_RECORDER_PAUSE)
    {
        aloge("fatal error! call dataCallbackTimestamp when recorder state is [0x%x]", eCurState);
        mpCameraProxy->releaseRecordingFrame(mCameraSourceChannel, pCameraFrameInfo->mFrameBuf.mId);
        return;
    }
    VIDEO_FRAME_INFO_S *pFrameInfo = (VIDEO_FRAME_INFO_S*)&pCameraFrameInfo->mFrameBuf;
    if(!mTimeLapseEnable && mAudioEncoder != PT_MAX && MM_INVALID_CHN != mAeChn)
    {
        if(-1 == rec_start_timestamp)
        {
            AW_MPI_AENC_GetFirstFrmPts(mAeChn,&rec_start_timestamp); 
        } 
    }
    

    if(!mTimeLapseEnable && mAudioEncoder != PT_MAX && 
		(pFrameInfo->VFrame.mpts < rec_start_timestamp || -1==rec_start_timestamp))
    {
        alogw("avsync_drp:%lld-%lld-%d-%d-%d",rec_start_timestamp,pFrameInfo->VFrame.mpts,mVeChn,
            mVEncChnAttr.VeAttr.SrcPicWidth,mVEncChnAttr.VeAttr.SrcPicHeight);
        mpCameraProxy->releaseRecordingFrame(mCameraSourceChannel, pCameraFrameInfo->mFrameBuf.mId);
    }
    else
    {
        //modify video frame pts to suit to audio pts.
        if(mAVSync)
        {
            int64_t diff_time = 0;  //unit:us
            mAVSync->GetTimeDiff(mAVSync, &diff_time);
            VIDEO_FRAME_INFO_S *pFrameInfo = (VIDEO_FRAME_INFO_S*)&pCameraFrameInfo->mFrameBuf;
            int64_t framePts = pFrameInfo->VFrame.mpts;
            framePts += diff_time;
            framePts = (framePts/1000 - mPauseAudioDuration)*1000;
            pFrameInfo->VFrame.mpts = framePts;
            mAVSync->SetVideoPts(mAVSync, pFrameInfo->VFrame.mpts/1000);
        }
        if(eCurState == MEDIA_RECORDER_PAUSE)
        {
            mpCameraProxy->releaseRecordingFrame(mCameraSourceChannel, pCameraFrameInfo->mFrameBuf.mId);
        }
        else
        {
            ERRORTYPE ret = AW_MPI_VENC_SendFrame(mVeChn, (VIDEO_FRAME_INFO_S*)&pCameraFrameInfo->mFrameBuf, -1);
            if(ret!=SUCCESS)
            {
                if(ERR_VENC_EXIST == ret)
                {
                    //alogd("copy success ,return frame now!");
                }
                else
                {
                    aloge("fatal error! send frame to venc fail[0x%x]", ret);
                }
                mpCameraProxy->releaseRecordingFrame(mCameraSourceChannel, pCameraFrameInfo->mFrameBuf.mId);
            }
        }

    } 
}

status_t EyeseeRecorder::removeOutputFormatAndOutputSink(int muxerId)
{
    alogv("removeOutputFormatAndOutputSink, muxerId[%d]", muxerId);
    //status_t result = UNKNOWN_ERROR;
    if (muxerId < 0) 
    {
        aloge("Invalid muxerId");
        return BAD_TYPE;
    }

    Mutex::Autolock autoLock(mLock);
    if (!(mCurrentState & MEDIA_RECORDER_DATASOURCE_CONFIGURED) && !(mCurrentState & MEDIA_RECORDER_RECORDING))
    {
        aloge("removeOutputFormatAndOutputSink called in an invalid state: %d", mCurrentState);
        return INVALID_OPERATION;
    }
    int idx = 0;
    bool bFindFlag = false;
    //test if normal.
    int findNum = 0;
    for(std::vector<OutputSinkInfo>::iterator it = mSinkInfos.begin(); it != mSinkInfos.end(); ++it)
    {
        if(it->mMuxerId == muxerId)
        {
            findNum++;
        }
    }
    if(findNum > 1)
    {
        aloge("fatal error! find more [%d] muxerId[%d], check code!", findNum, muxerId);
    }
    //remote OutputSinkInfo
    for(std::vector<OutputSinkInfo>::iterator it = mSinkInfos.begin(); it != mSinkInfos.end(); ++it, ++idx)
    {
        if(it->mMuxerId == muxerId)
        {
            bFindFlag = true;
            break;
        }
    }
    if(bFindFlag)
    {
        if(mSinkInfos[idx].mOutputFd >= 0)
        {
            //alogd("close fd[%d]", mSinkInfos[idx].mOutputFd);
            ::close(mSinkInfos[idx].mOutputFd);
            mSinkInfos[idx].mOutputFd = -1;
        }
        mSinkInfos.erase(mSinkInfos.begin()+idx);
    }

    idx = 0;
    bFindFlag = false;
    for(std::vector<MUX_CHN_ATTR_S>::iterator it = mMuxChnAttrs.begin(); it != mMuxChnAttrs.end(); ++it, ++idx)
    {
        if(it->mMuxerId == muxerId)
        {
            bFindFlag = true;
            break;
        }
    }
    if(bFindFlag)
    {
        if(mCurrentState == MEDIA_RECORDER_RECORDING)
        {
            if(mMuxGrp >= 0)
            {
                if(AW_MPI_MUX_DestroyChn(mMuxGrp, mMuxChns[idx]) != SUCCESS)
                {
                    aloge("fatal error! mux destroy channel fail!");
                    return UNKNOWN_ERROR;
                }
            }
        }
        mMuxChns.erase(mMuxChns.begin()+idx);
        mMuxChnAttrs.erase(mMuxChnAttrs.begin()+idx);
        mPolicy.erase(muxerId);
        return NO_ERROR;
    }
    else
    {
        aloge("fatal error! we can't find muxerId[%d]!", muxerId);
        return BAD_VALUE;
    }
}

status_t EyeseeRecorder::setOutputFileSync(int fd, int64_t fallocateLength, int muxerId)
{
    alogv("setOutputFileSync fd=%d", fd);
	if (fd < 0) 
    {
		aloge("Invalid parameter");
		return BAD_VALUE;
	}
    if (!(mCurrentState & MEDIA_RECORDER_RECORDING))
    {
        aloge("removeOutputFormatAndOutputSink called in an invalid state: 0x%x", mCurrentState);
        return INVALID_OPERATION;
    }
    if(mMuxChns.size() != mMuxChnAttrs.size())
    {
        aloge("fatal error! mux channels are not same, check code!");
    }
    //find mux channel by muxerId
    MUX_CHN muxChn = MM_INVALID_CHN;
    std::vector<MUX_CHN>::iterator itChn = mMuxChns.begin();
    for(std::vector<MUX_CHN_ATTR_S>::iterator it = mMuxChnAttrs.begin(); it != mMuxChnAttrs.end(); ++it)
    {
        if(it->mMuxerId == muxerId)
        {
            muxChn = *itChn;
            break;
        }
        ++itChn;
    }
    if(muxChn != MM_INVALID_CHN)
    {
        int nFindCnt = 0;
        for(OutputSinkInfo& i : mSinkInfos)
        {
            if(i.mMuxerId == muxerId)
            {
                if(0 == nFindCnt)
                {
                    if(i.mOutputFd >= 0)
                    {
                        ::close(i.mOutputFd);
                        i.mOutputFd = dup(fd);
                    }
                    else
                    {
                        aloge("fatal error! check code!");
                    }
                }
                else
                {
                    aloge("fatal error! nFindCnt[%d]", nFindCnt);
                }
                nFindCnt++;
            }
        }
        if(nFindCnt <= 0)
        {
            aloge("fatal error! why muxerId[%d] is not found!", muxerId);
        }
        AW_MPI_MUX_SwitchFd(mMuxGrp, muxChn, fd, fallocateLength);
        return NO_ERROR;
    }
    else
    {
        aloge("fatal error! can't find muxChn which muxerId[%d]", muxerId);
        return BAD_VALUE;
    }
}

status_t EyeseeRecorder::setOutputFileSync(char* path, int64_t fallocateLength, int muxerId)
{
    status_t ret;
    if(path!=NULL)
    {
        int fd = open(path, O_RDWR | O_CREAT, 0666);
    	if (fd < 0) 
        {
    		aloge("Failed to open %s", path);
    		return BAD_VALUE;
    	}
        ret = setOutputFileSync(fd, fallocateLength, muxerId);
        ::close(fd);
        return ret;
    }
    else
    {
        return BAD_VALUE;
    }
}

status_t EyeseeRecorder::setSdcardState(bool bExist)
{
    alogd("need implement");
    return UNKNOWN_ERROR;
}

#if 0
status_t EyeseeRecorder::setImpactFileDuration(int bfTimeMs, int afTimeMs)
{
    if (!(mCurrentState & MEDIA_RECORDER_IDLE) && !(mCurrentState & MEDIA_RECORDER_INITIALIZED) && !(mCurrentState & MEDIA_RECORDER_DATASOURCE_CONFIGURED))
    {
        aloge("setImpactFileDuration called in an invalid state: 0x%x", mCurrentState);
        return INVALID_OPERATION;
    }
    mImpactFileDuration[0] = bfTimeMs;
    mImpactFileDuration[1] = afTimeMs;
    return UNKNOWN_ERROR;
}

status_t EyeseeRecorder::setImpactOutputFile(int fd, int64_t fallocateLength, int muxerId)
{
    if (!(mCurrentState & MEDIA_RECORDER_RECORDING))
    {
        aloge("setImpactOutputFile called in an invalid state: 0x%x", mCurrentState);
        return INVALID_OPERATION;
    }
    alogd("need implement");
    return UNKNOWN_ERROR;
}

status_t EyeseeRecorder::setImpactOutputFile(char* path, int64_t fallocateLength, int muxerId)
{
    status_t ret;
    if(path!=NULL)
    {
        int fd = open(path, O_RDWR | O_CREAT, 0666);
    	if (fd < 0) 
        {
    		aloge("Failed to open %s", path);
    		return BAD_VALUE;
    	}
        ret = setImpactOutputFile(fd, fallocateLength, muxerId);
        ::close(fd);
        return ret;
    }
    else
    {
        return BAD_VALUE;
    }
}
#endif

status_t EyeseeRecorder::setMuxCacheDuration(int nCacheMs)
{
    if (!(mCurrentState & MEDIA_RECORDER_IDLE) && !(mCurrentState & MEDIA_RECORDER_INITIALIZED) && !(mCurrentState & 
MEDIA_RECORDER_DATASOURCE_CONFIGURED))
    {
        aloge("called in an invalid state: 0x%x", mCurrentState);
        return INVALID_OPERATION;
    }
    if (nCacheMs < 0)
    {
        aloge("fatal error! called in an invalid bfTimeMs: %lld  or afTimeMs: %lld", nCacheMs);
        return BAD_VALUE;
    }
    mMuxCacheDuration = nCacheMs;
    return UNKNOWN_ERROR;
}

status_t EyeseeRecorder::switchFileNormal(int fd, int64_t fallocateLength, int muxerId)
{
    if (fd < 0) 
    {
		aloge("Invalid parameter");
		return BAD_VALUE;
	}
    if (!(mCurrentState & MEDIA_RECORDER_RECORDING))
    {
        aloge("switch file smooth called in an invalid state: 0x%x", mCurrentState);
        return INVALID_OPERATION;
    }
    if(mMuxChns.size() != mMuxChnAttrs.size())
    {
        aloge("fatal error! mux channels are not same, check code!");
    }
    //find mux channel by muxerId
    MUX_CHN muxChn = MM_INVALID_CHN;
    std::vector<MUX_CHN>::iterator itChn = mMuxChns.begin();
    for(std::vector<MUX_CHN_ATTR_S>::iterator it = mMuxChnAttrs.begin(); it != mMuxChnAttrs.end(); ++it)
    {
        if(it->mMuxerId == muxerId)
        {
            muxChn = *itChn;
            break;
        }
        ++itChn;
    }
    if(muxChn != MM_INVALID_CHN)
    {
        for(auto i : mSinkInfos)
        {
            if(i.mMuxerId == muxerId)
            {
                if(i.mOutputFd >= 0)
                {
                    ::close(i.mOutputFd);
                    i.mOutputFd = dup(fd);
                }
            }
        }
        AW_MPI_MUX_SwitchFd(mMuxGrp, muxChn, fd, fallocateLength);
        AW_MPI_MUX_SwitchFileNormal(mMuxGrp, muxChn);
        return NO_ERROR;
    }
    else
    {
        aloge("fatal error! can't find muxChn match muxerId[%d]", muxerId);
        return BAD_VALUE;
    }
}

status_t EyeseeRecorder::setSwitchFileDurationPolicy(int muxerId,const RecordFileDurationPolicy ePolicy)
{
    Mutex::Autolock autoLock(mLock);
    bool bFindFlag = false;
    MUX_CHN muxchn = MM_INVALID_CHN;
    for(std::vector<OutputSinkInfo>::iterator it = mSinkInfos.begin(); it != mSinkInfos.end(); ++it)
    {
        if(it->mMuxerId == muxerId)
        {
            muxchn = mMuxChns[it - mSinkInfos.begin()];
            bFindFlag = true;
            break;
        }
    }
    if(!bFindFlag)
    {
        aloge("fatal error, the muxerId[%d] is error!", muxerId);
        return UNKNOWN_ERROR;
    }

    if(mCurrentState & MEDIA_RECORDER_PREPARED)
    {
        mPolicy.find(muxerId)->second = ePolicy;
        return AW_MPI_MUX_SetSwitchFileDurationPolicy(mMuxGrp, muxchn, ePolicy);
    }
    else
    {
        aloge("the error state[%d] set", mCurrentState);
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::getSwitchFileDurationPolicy(int muxerId, RecordFileDurationPolicy *pPolicy) const
{
    if(pPolicy)
    {
        if(mPolicy.find(muxerId) == mPolicy.end())
        {
            aloge("fatal error, the muxerId[%d] is error!", muxerId);
            return UNKNOWN_ERROR;
        }
        
        *pPolicy = mPolicy.find(muxerId)->second;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::switchFileNormal(char* path, int64_t fallocateLength, int muxerId)
{
    status_t ret;
    if(path!=NULL)
    {
        int fd = open(path, O_RDWR | O_CREAT, 0666);
    	if (fd < 0) 
        {
    		aloge("Failed to open %s", path);
    		return BAD_VALUE;
    	}
        ret = switchFileNormal(fd, fallocateLength, muxerId);
        ::close(fd);
        return ret;
    }
    else
    {
        return BAD_VALUE;
    }
}

status_t EyeseeRecorder::enableDynamicBitRateControl(bool bEnable)
{
    alogd("need implement");
    return UNKNOWN_ERROR;
}

status_t EyeseeRecorder::enableHorizonFlip(bool enable)
{
    mbHorizonfilp = enable;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        BOOL bHorizonFlipFlag;
        if(enable)
        {
            bHorizonFlipFlag = TRUE;
        }
        else
        {
            bHorizonFlipFlag = FALSE;
        }
        
        AW_MPI_VENC_SetHorizonFlip(mVeChn, bHorizonFlipFlag);
    }
    return NO_ERROR;
    
}

 status_t EyeseeRecorder::enable3DNR(int nLevel)
 {
    mb3DNR = nLevel;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        AW_MPI_VENC_Set3DNR(mVeChn, nLevel);
    }
    return NO_ERROR;
 }

status_t EyeseeRecorder::get3DNRLevel(int *p3DNRLevel)
 {
    status_t result = UNKNOWN_ERROR;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        ERRORTYPE ret = AW_MPI_VENC_Get3DNR(mVeChn, p3DNRLevel);
        if(SUCCESS == ret)
        {
            result = NO_ERROR;
        }
    }
    return result;
 }

status_t EyeseeRecorder::enableSaveBsFile(VencSaveBSFile *pSaveParam)
{
    mSaveBSFileParam = *pSaveParam;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        AW_MPI_VENC_SaveBsFile(mVeChn, pSaveParam);
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::setProcSet(VeProcSet *pVeProcSet)
{
    mVeProcSet = *pVeProcSet;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        AW_MPI_VENC_SetProcSet(mVeChn, pVeProcSet);
    }
    return NO_ERROR;
}

 status_t EyeseeRecorder::enableColor2Grey(bool enable)
 {
    mbColor2Grey = enable;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        VENC_COLOR2GREY_S bColor2GreyFlag;
        if(enable)
        {
            bColor2GreyFlag.bColor2Grey = TRUE;
        }
        else
        {
            bColor2GreyFlag.bColor2Grey = FALSE;
        }
        AW_MPI_VENC_SetColor2Grey(mVeChn, &bColor2GreyFlag);
    }
    return NO_ERROR;
 }

 status_t EyeseeRecorder::enableAdaptiveIntraInp(bool enable)
 {
    mbAdaptiveintrainp = enable;
    Mutex::Autolock autoLock(mLock);
    if (mCurrentState & MEDIA_RECORDER_PREPARED)
    {
        BOOL bAdaptiveIntraInpFlag;
        if(enable)
        {
            bAdaptiveIntraInpFlag = TRUE;
        }
        else
        {
            bAdaptiveIntraInpFlag = FALSE;
        }
        AW_MPI_VENC_SetAdaptiveIntraInP(mVeChn, bAdaptiveIntraInpFlag);
    }
    return NO_ERROR;   
 }

status_t EyeseeRecorder::enableIframeFilter(bool enable)
{
    alogd("need implement");
    return UNKNOWN_ERROR;
}

status_t EyeseeRecorder::enableNullSkip(bool enable)
{
    mNullSkipEnable = enable;
    return NO_ERROR;
}

status_t EyeseeRecorder::enablePSkip(bool enable)
{
    mPSkipEnable = enable;
    return NO_ERROR;
}

#if 0
status_t EyeseeRecorder::enableVideoEncodingLongTermRef(bool enable)
{
    mbLongTermRef = enable;
    return NO_ERROR;
}
#endif

status_t EyeseeRecorder::enableFastEncode(bool enable)
{
    mFastEncFlag = enable;
    return NO_ERROR;
}

status_t EyeseeRecorder::enableVideoEncodingPIntra(bool enable)
{
    mbPIntraEnable = enable;
    return NO_ERROR;
}
status_t EyeseeRecorder::setVideoEncodingMode(int Mode)
{
    alogd("need implement");
    return UNKNOWN_ERROR;
}

status_t EyeseeRecorder::setVideoSliceHeight(int sliceHeight)
{
    alogd("need implement");
    return UNKNOWN_ERROR;
}

/**
 * nIQpOffset:
 * for h264, [0,10). default 0, if want to decrease I frame size, increase IQpOffset to 6 in common.
 * for h265, [-12, 12]. decrease bs_size to 50% for every increase 6.
 */
status_t EyeseeRecorder::setIQpOffset(int nIQpOffset)
{
    if (PT_H264 == mVideoEncoder)
    {
        if (!(nIQpOffset>=0 && nIQpOffset<10))
        {
            aloge("IQpOffset value must be in [0, 10) for 264!");
            return BAD_VALUE;
        }
    }
    else if (PT_H265 == mVideoEncoder)
    {
        if (!(nIQpOffset>=-12 && nIQpOffset<=12))
        {
            aloge("IQpOffset value must be in [-12, 12] for 265!");
            return BAD_VALUE;
        }
    }
    else
    {
        aloge("IQpOffset can not be set for other vencoder(%d)!", mVideoEncoder);
        return BAD_VALUE;
    }

    mIQpOffset = nIQpOffset;
    return NO_ERROR;
}

/*
status_t EyeseeRecorder::setIQpRange(int maxIQp, int minIQp)
{
    if ((VideoRCMode_ABR==mVideoRCMode) && (PT_H265==mVideoEncoder || PT_H264==mVideoEncoder))
    {
        mMaxIQp = maxIQp;
        mMinIQp = minIQp;
        return NO_ERROR;
    }
    else
    {
        aloge("maxIQp and minIQp only used in ABR mode! current_vtype:%d, rc_mode:%d", mVideoEncoder, mVideoRCMode);
        return BAD_VALUE;
    }
}
*/

/*
status_t EyeseeRecorder::setIQpAndPQp(int nIQp, int nPQp)
{
    if ((VideoRCMode_FIXQP==mVideoRCMode) && (PT_H265==mVideoEncoder || PT_H264==mVideoEncoder))
    {
        mIQp = nIQp;
        mPQp = nPQp;
        return NO_ERROR;
    }
    else
    {
        aloge("mIQp and mPQp only used in FixQp mode! current_vtype:%d, rc_mode:%d", mVideoEncoder, mVideoRCMode);
        return BAD_VALUE;
    }
}
*/

/*
status_t EyeseeRecorder::setMaxVideoBitRate(int maxBitRate)
{
    mMaxVideoBitRate = maxBitRate;
    if (VideoRCMode_VBR==mVideoRCMode || VideoRCMode_ABR==mVideoRCMode)
    {
        return NO_ERROR;
    }
    else
    {
        alogw("maxBitRate only used in VBR or ABR mode! current_vtype:%d, rc_mode:%d", mVideoEncoder, mVideoRCMode);
        return NO_ERROR;
    }
}
*/

/*
status_t EyeseeRecorder::setAbrRatioChangeQp(int nAbrRatioQp)
{
    mAbrRatioChangeQp = nAbrRatioQp;
    if ((PT_H264==mVideoEncoder || PT_H265==mVideoEncoder) && VideoRCMode_ABR==mVideoRCMode)
    {
        return NO_ERROR;
    }
    else
    {
        alogw("mAbrRatioChangeQp only used in H264ABR or H265ABR mode! current_vtype:%d, rc_mode:%d", mVideoEncoder, mVideoRCMode);
        return NO_ERROR;
    }
}
*/

/*
status_t EyeseeRecorder::setAbrQuality(int nAbrQuality)
{
    mAbrQuality = nAbrQuality;
    if ((PT_H264==mVideoEncoder || PT_H265==mVideoEncoder) && VideoRCMode_ABR==mVideoRCMode)
    {
        return NO_ERROR;
    }
    else
    {
        alogw("mAbrQuality only used in H264ABR or H265ABR mode! current_vtype:%d, rc_mode:%d", mVideoEncoder, mVideoRCMode);
        return NO_ERROR;
    }
}
*/

/*
status_t EyeseeRecorder::setVEncProfile(VEncProfile nProfile)
{
    mVEncAttr.mType = mVideoEncoder;
    if(PT_H264 == mVEncAttr.mType)
    {
        switch(nProfile)
        {
            case VEncProfile_BaseLine:
                mVEncAttr.mAttrH264.mProfile = 0;
                break;
            case VEncProfile_MP:
                mVEncAttr.mAttrH264.mProfile = 1;
                break;
            case VEncProfile_HP:
                mVEncAttr.mAttrH264.mProfile = 2;
                break;
            default:
                aloge("fatal error! unsupport h264 profile[0x%x]", nProfile);
                mVEncAttr.mAttrH264.mProfile = 1;
                break;
        }
        mVEncAttr.mAttrH264.mLevel = H264_LEVEL_51;
    }
    else if(PT_H265 == mVEncAttr.mType)
    {
        switch(nProfile)
        {
            case VEncProfile_MP:
                mVEncAttr.mAttrH265.mProfile = 0;
                break;
            default:
                aloge("fatal error! unsupport h265 profile[0x%x]", nProfile);
                mVEncAttr.mAttrH265.mProfile = 0;
                break;
        }
        mVEncAttr.mAttrH265.mLevel = H265_LEVEL_62;
    }
    return NO_ERROR;
}
*/

status_t EyeseeRecorder::setVEncAttr(VEncAttr *pVEncAttr)
{
    mVEncAttr = *pVEncAttr;
    return NO_ERROR;
}

status_t EyeseeRecorder::setRoiCfg(VENC_ROI_CFG_S *pVencRoiCfg)
{
    status_t ret = NO_ERROR;
    if (!(mCurrentState & MEDIA_RECORDER_PREPARED) && !(mCurrentState & MEDIA_RECORDER_RECORDING))
    {
        aloge("called in an invalid state: 0x%x", mCurrentState);
        return INVALID_OPERATION;
    }
    if(SUCCESS != AW_MPI_VENC_SetRoiCfg(mVeChn, pVencRoiCfg))
    {
        ret = UNKNOWN_ERROR;
    }
    return ret;
}

status_t EyeseeRecorder::getRoiCfg(unsigned int nIndex, VENC_ROI_CFG_S *pVencRoiCfg)
{
    status_t ret = NO_ERROR;
    if (!(mCurrentState & MEDIA_RECORDER_PREPARED) && !(mCurrentState & MEDIA_RECORDER_RECORDING))
    {
        aloge("called in an invalid state: 0x%x", mCurrentState);
        return INVALID_OPERATION;
    }
    if(SUCCESS != AW_MPI_VENC_GetRoiCfg(mVeChn, nIndex, pVencRoiCfg))
    {
        ret = UNKNOWN_ERROR;
    }
    return ret;
}

status_t EyeseeRecorder::setVencSuperFrameConfig(VENC_SUPERFRAME_CFG_S *pSuperFrameConfig)
{
    status_t ret = NO_ERROR;
    Mutex::Autolock autoLock(mLock);
    mVencSuperFrameCfg = *pSuperFrameConfig;
    if (mCurrentState & MEDIA_RECORDER_PREPARED || mCurrentState & MEDIA_RECORDER_RECORDING)
    {
        if(SUCCESS != AW_MPI_VENC_SetSuperFrameCfg(mVeChn, &mVencSuperFrameCfg))
        {
            ret = UNKNOWN_ERROR;
        }
    }
    return ret;
}

status_t EyeseeRecorder::enableAttachAACHeader(bool enable)
{
    mAttachAacHeaderFlag = enable;
    return NO_ERROR;
}

status_t EyeseeRecorder::enableDBRC(bool enable)
{
    mEnableDBRC = enable;
    return NO_ERROR;
}

status_t EyeseeRecorder::GetBufferState(BufferState &state)
{
    if(mEnableDBRC)
    {
        return mpDBRC->GetBufferState(state);
    }
    else
    {
        return NO_INIT;
    }
}

void EyeseeRecorder::notify(MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    if(MOD_ID_AI == pChn->mModId)
    {
        switch(event)
        {
            case MPP_EVENT_CAPTURE_AUDIO_DATA:
            {
                if(mAVSync)
                {
                    AISendDataInfo * pUserData = (AISendDataInfo *)pEventData;
                    unsigned int nSize = pUserData->mLen;
                    unsigned int nPause = pUserData->mbIgnore;
                    if(nPause == 1)
                    {
                        mIgnoreAudioBlockNum++;
                        mIgnoreAudioBytes += nSize;
                        int nBitsPerSample = 16;
                        switch(mAioAttr.enBitwidth)
                        {
                            case AUDIO_BIT_WIDTH_8:
                                nBitsPerSample = 8;
                                break;
                            case AUDIO_BIT_WIDTH_16:
                                nBitsPerSample = 16;
                                break;
                            case AUDIO_BIT_WIDTH_24:
                                nBitsPerSample = 24;
                                break;
                            case AUDIO_BIT_WIDTH_32:
                                nBitsPerSample = 32;
                                break;
                            default:
                                nBitsPerSample = 16;
                                break;
                        }
                        int nChannels = mAioAttr.u32ChnCnt;
                        int nSampleRate = mAioAttr.enSamplerate;
                        mPauseAudioDuration = mIgnoreAudioBytes*1000/((nBitsPerSample/8) * nChannels * nSampleRate);
                    }
                    else
                    {
                        mAVSync->AddAudioDataLen(mAVSync, nSize);
                    }
                }
                break;
            }
            default:
            {
                //postEventFromNative(this, event, 0, 0, pEventData);
                aloge("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!", event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                break;
            }
        }
    }
    else if(MOD_ID_VENC == pChn->mModId)
    {
        switch(event)
        {
            case MPP_EVENT_RELEASE_VIDEO_BUFFER:
            {
                mpInputFrameManager->addReleaseFrame((VIDEO_FRAME_INFO_S*)pEventData);
                break;
            }
            case MPP_EVENT_VENC_TIMEOUT:
            {
                uint64_t framePts = *(uint64_t*)pEventData;
                std::shared_ptr<CMediaMemory> spMem = std::make_shared<CMediaMemory>(sizeof(framePts));
                memcpy(spMem->getPointer(), &framePts, sizeof(framePts));
                postEventFromNative(this, MEDIA_RECORDER_EVENT_ERROR, MEDIA_ERROR_VENC_TIMEOUT, 0, &spMem);
                break;
            }
            case MPP_EVENT_VENC_BUFFER_FULL:
            {                
                postEventFromNative(this, MEDIA_RECORDER_EVENT_ERROR, MEDIA_ERROR_VENC_BUFFER_FULL, 0, NULL);
                break;                
            }
            default:
            {
                //postEventFromNative(this, event, 0, 0, pEventData);
                aloge("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!", event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                break;
            }
        }
    }
    else if(MOD_ID_AENC == pChn->mModId) {
        alogw("not support notify recorder by AEnc with event(%d)", event);
    }
    else if(MOD_ID_MUX == pChn->mModId)
    {
        switch(event)
        {
            case MPP_EVENT_NEED_NEXT_FD:
            {
                int muxerId = *(int*)pEventData;
                postEventFromNative(this, MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_NEED_SET_NEXT_FD, muxerId, NULL);
                break;
            }
            case MPP_EVENT_RECORD_DONE:
            {
                int muxerId = *(int*)pEventData;
                postEventFromNative(this, MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_RECORD_FILE_DONE, muxerId, NULL);
                break;
            }
            case MPP_EVENT_WRITE_DISK_ERROR:
            {
                int muxerId = *(int*)pEventData;
                postEventFromNative(this, MEDIA_RECORDER_EVENT_ERROR, MEDIA_ERROR_WRITE_DISK_ERROR, muxerId, NULL);
                break;
            }
            case MPP_EVENT_BSFRAME_AVAILABLE:
            {
                status_t ret = pushOneBsFrame((CDXRecorderBsInfo*)pEventData);
                if (ret == NO_ERROR)
                {
                    //judge if need send message.
                    Mutex::Autolock autoLock(mEncBufLock);
                    if(1 == mReadyEncBufList.size())
                    {
                        postEventFromNative(this, MEDIA_RECORDER_VENDOR_EVENT_BSFRAME_AVAILABLE, 0, 0, NULL);
                    }
                }
                else if(ret == NO_MEMORY)
                {
                    //postEventFromNative(this, MEDIA_RECORDER_EVENT_ERROR, 
                    //    MPP_EVENT_ERROR_ENCBUFFER_OVERFLOW, 0, NULL);
                    alogv("bsFrame buf queue is full");
                } 
                else
                    aloge("UNKNOWN ERROR");
                break;
            }
            default:
            {
                //postEventFromNative(this, event, 0, 0, pEventData);
                aloge("fatal error! unknown event[0x%x] from channel[0x%x][0x%x][0x%x]!", event, pChn->mModId, pChn->mDevId, pChn->mChnId);
                break;
            }
        }
    }
    else
    {
        aloge("fatal error! need implement!");
    }
}

void EyeseeRecorder::doCleanUp()
{
    mIsAudioSourceSet  = false;
    mIsVideoSourceSet  = false;
    mIsAudioEncoderSet = false;
    mIsVideoEncoderSet = false;
    mIsOutputFileSet   = false;
    mMuxerIdCounter = 0;

    mIgnoreAudioBlockNum = 0;
    mIgnoreAudioBytes = 0;
    mPauseAudioDuration = 0;

    mVideoEncoder = PT_MAX;
    mAudioEncoder = PT_MAX;
    mTimeLapseEnable = false;
    mAttachAacHeaderFlag = false;

    mVideoRCMode = VideoRCMode_CBR;
    
    mVEncRcAttr = {
        .mVEncType = PT_H264, 
        .mRcMode = VideoRCMode_CBR,
        {
            .mAttrH264Cbr = {
                .mBitRate = 1*1024*1024,
                .mMaxQp = 51,
                .mMinQp = 1,
            },
        },
    };
    //mVideoBitRate = 0;
    //mVideoEncodingBufferTime = 0;
    mIQpOffset = 0;
    mVEncAttr = {
        .mType = PT_H264,
        .mBufSize = 0,
        .mThreshSize = 0,
        {
            .mAttrH264 = {
                .mProfile = 1,
                .mLevel = H264_LEVEL_51,
            },
        },
    };
    //mbLongTermRef = true;
    mNullSkipEnable = false;
    mPSkipEnable = false;
    mFastEncFlag = false;
    mbPIntraEnable = true;
    mb3DNR = 0;
    mbAdaptiveintrainp = false;
    mVencSuperFrameCfg = {
        .enSuperFrmMode = SUPERFRM_NONE,
        .SuperIFrmBitsThr = 0,
        .SuperPFrmBitsThr = 0,
        .SuperBFrmBitsThr = 0,
    };
    mbColor2Grey = false;
    mbHorizonfilp = false;
    mMuteMode = false;

    mAudioBitRate = 0;

    memset(&mVEncChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
    memset(&mVEncRefParam, 0, sizeof(VENC_PARAM_REF_S));
    memset(&mSmartPParam, 0, sizeof(VencSmartFun));
    memset(&mIntraRefreshParam, 0, sizeof(VENC_PARAM_INTRA_REFRESH_S));
    memset(&mSaveBSFileParam, 0, sizeof(mSaveBSFileParam));
    memset(&mVeProcSet, 0, sizeof(mVeProcSet));

    Mutex::Autolock lock(mRgnLock);
    ERRORTYPE ret;
    size_t num = mRgnHandleList.size();
    if(num > 0)
    {
        alogd("Be careful! There are [%d]regions need to destroy!", num);
    }
    for(std::list<RGN_HANDLE>::iterator it = mRgnHandleList.begin(); it != mRgnHandleList.end();)
    {
        ret = AW_MPI_RGN_Destroy(*it);
        if(SUCCESS == ret)
        {
            it = mRgnHandleList.erase(it);
        }
        else
        {
            aloge("fatal error! destroy region[%d] fail!", *it);
            ++it;
        }
    }
}

void EyeseeRecorder::postEventFromNative(EyeseeRecorder *pRecorder, int what, int arg1, int arg2, const std::shared_ptr<CMediaMemory>* pDataPtr)
{
    if (pRecorder == NULL)
    {
        aloge("fatal error! pRecorder == NULL");
        return;
    }

    if (pRecorder->mEventHandler != NULL) 
    {
        CallbackMessage msg;
        msg.what = what;
        msg.arg1 = arg1;
        msg.arg2 = arg2;
        if(pDataPtr)
        {
            msg.mDataPtr = std::const_pointer_cast<const CMediaMemory>(*pDataPtr);
        }
        pRecorder->mEventHandler->post(msg);
    }
}

status_t EyeseeRecorder::config_AIO_ATTR_S()
{
    memset(&mAioAttr, 0, sizeof(AIO_ATTR_S));
    mAioAttr.enBitwidth = AUDIO_BIT_WIDTH_16;

    if ((mSampleRate!=AUDIO_SAMPLE_RATE_8000 ) &&
        (mSampleRate!=AUDIO_SAMPLE_RATE_12000) &&
        (mSampleRate!=AUDIO_SAMPLE_RATE_11025) &&
        (mSampleRate!=AUDIO_SAMPLE_RATE_16000) &&
        (mSampleRate!=AUDIO_SAMPLE_RATE_22050) &&
        (mSampleRate!=AUDIO_SAMPLE_RATE_24000) &&
        (mSampleRate!=AUDIO_SAMPLE_RATE_32000) &&
        (mSampleRate!=AUDIO_SAMPLE_RATE_44100) &&
        (mSampleRate!=AUDIO_SAMPLE_RATE_48000) )
    {
        alogw("wrong audio SampleRate(%d) setting, change to default(8000)!", mSampleRate);
        mSampleRate = 8000;
    }
    mAioAttr.enSamplerate = (AUDIO_SAMPLE_RATE_E)mSampleRate;

    if ((mAudioChannels!=1) && (mAudioChannels!=2))
    {
        alogw("wrong audio TrackCnt(%d) setting, change to default(1)!", mAudioChannels);
        mAudioChannels = 1;
    }
    mAioAttr.u32ChnCnt = mAudioChannels;
    mAioAttr.enSoundmode = (mAudioChannels==1)?AUDIO_SOUND_MODE_MONO:AUDIO_SOUND_MODE_STEREO;
    alogd("AIO_Attr ==>> SampleRate:%d, TrackCnt:%d", mAioAttr.enSamplerate, mAioAttr.u32ChnCnt);

    return NO_ERROR;
}

status_t EyeseeRecorder::config_AENC_CHN_ATTR_S()
{
    memset(&mAEncChnAttr, 0, sizeof(AENC_CHN_ATTR_S));
    if((PT_AAC==mAudioEncoder) ||
       (PT_PCM_AUDIO==mAudioEncoder) ||
       (PT_LPCM==mAudioEncoder) ||
       (PT_ADPCMA==mAudioEncoder) ||
       (PT_MP3==mAudioEncoder) ||
       (PT_G726==mAudioEncoder) ||
       (PT_G711A==mAudioEncoder) ||
       (PT_G711U==mAudioEncoder))
    {
        mAEncChnAttr.AeAttr.Type = mAudioEncoder;
        mAEncChnAttr.AeAttr.sampleRate = mSampleRate;
        mAEncChnAttr.AeAttr.channels = mAudioChannels;
        mAEncChnAttr.AeAttr.bitRate = mAudioBitRate;
        mAEncChnAttr.AeAttr.bitsPerSample = 16;
        mAEncChnAttr.AeAttr.attachAACHeader = (int)mAttachAacHeaderFlag;
    }
    else
    {
        aloge("unsupported audio encoder formate(%d) temporaryly", mAudioEncoder);
    }
    alogd("AEnc_Attr ==>> Type:%d, SampleRate:%d, TrackCnt:%d, AttachAacHeader:%d",
        mAudioEncoder, mSampleRate, mAudioChannels, mAttachAacHeaderFlag);

    return NO_ERROR;
}

status_t EyeseeRecorder::config_TENC_CHN_ATTR_S()
{
    memset(&mTEncChnAttr, 0, sizeof(TENC_CHN_ATTR_S));
    mTEncChnAttr.tInfo.enc_enable_type |= 1<<0; // just enable gps enc 

    return NO_ERROR;
}

status_t EyeseeRecorder::gpsInfoEn(int gps_en)
{
    gps_state = gps_en; 
    
    memset(&mTEncChnAttr, 0, sizeof(TENC_CHN_ATTR_S));
    if(gps_state)
    {
        mTEncChnAttr.tInfo.enc_enable_type |= 1<<0; // just enable gps enc 
    }

    return NO_ERROR;
}

status_t EyeseeRecorder::gpsInfoSend(void *gps_info)
{
    TEXT_FRAME_S text_frm; 
    
    if(NULL!=gps_info && gps_state)
    { 
        memset(&text_frm,0,sizeof(TEXT_FRAME_S));

        memcpy((void *)text_frm.mpAddr,gps_info,sizeof(RMCINFO));
        text_frm.mLen = sizeof(RMCINFO);
        text_frm.mTimeStamp = -1;
        text_frm.mId = 0; 

        ERRORTYPE ret = AW_MPI_TENC_SendFrame(mTeChn, &text_frm);
        if(SUCCESS != ret)
        {
            aloge("send_text_frame_fail");
        } 
    } 

    return NO_ERROR;
} 

status_t EyeseeRecorder::config_VENC_CHN_ATTR_S()
{
    //memset(&mVEncChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
    mVEncChnAttr.VeAttr.Type = mVideoEncoder;
    if(NULL == mpCameraProxy)
    {
        aloge("fatal error! camera is null!");
    }
    CameraParameters param;
    mpCameraProxy->getParameters(mCameraSourceChannel, param);

    mVEncChnAttr.VeAttr.MaxKeyInterval = mVideoMaxKeyItl;
    SIZE_S frameBufSize;
    param.getVideoBufSizeOut(frameBufSize);
    mVEncChnAttr.VeAttr.SrcPicWidth = frameBufSize.Width;
    mVEncChnAttr.VeAttr.SrcPicHeight = frameBufSize.Height;
    mVEncChnAttr.VeAttr.Field = VIDEO_FIELD_FRAME;
    mVEncChnAttr.VeAttr.PixelFormat = param.getPreviewFormat();
    mVEncChnAttr.VeAttr.mColorSpace = param.getColorSpace();
    if(mVideoEncoder != mVEncAttr.mType)
    {
        aloge("fatal error! vencType is not match: [0x%x]!=[0x%x]!", mVideoEncoder, mVEncAttr.mType);
    }
    if(PT_H264 == mVEncChnAttr.VeAttr.Type)
    {
        mVEncChnAttr.VeAttr.AttrH264e.BufSize = mVEncAttr.mBufSize;
        mVEncChnAttr.VeAttr.AttrH264e.mThreshSize = mVEncAttr.mThreshSize;
        mVEncChnAttr.VeAttr.AttrH264e.Profile = mVEncAttr.mAttrH264.mProfile;
        mVEncChnAttr.VeAttr.AttrH264e.mLevel = mVEncAttr.mAttrH264.mLevel;
        mVEncChnAttr.VeAttr.AttrH264e.bByFrame = TRUE;
        mVEncChnAttr.VeAttr.AttrH264e.PicWidth = mVideoWidth;
        mVEncChnAttr.VeAttr.AttrH264e.PicHeight = mVideoHeight;
        mVEncChnAttr.VeAttr.AttrH264e.IQpOffset = mIQpOffset;
       // mVEncChnAttr.VeAttr.AttrH264e.mbLongTermRef = mbLongTermRef;
        mVEncChnAttr.VeAttr.AttrH264e.FastEncFlag = mFastEncFlag;
       // mVEncChnAttr.VeAttr.AttrH264e.mVirtualIFrameInterval = mVirtualIFrameInterval;
        mVEncChnAttr.VeAttr.AttrH264e.mbPIntraEnable = mbPIntraEnable;
       alogd("config venc bufSize[%d], threshSize[%d]", mVEncChnAttr.VeAttr.AttrH264e.BufSize, mVEncChnAttr.VeAttr.AttrH264e.mThreshSize);
    }
    else if(PT_H265 == mVEncChnAttr.VeAttr.Type)
    {
        mVEncChnAttr.VeAttr.AttrH265e.mBufSize = mVEncAttr.mBufSize;
        mVEncChnAttr.VeAttr.AttrH265e.mThreshSize = mVEncAttr.mThreshSize;
        mVEncChnAttr.VeAttr.AttrH265e.mProfile = mVEncAttr.mAttrH265.mProfile;
        mVEncChnAttr.VeAttr.AttrH265e.mLevel = mVEncAttr.mAttrH265.mLevel;
        mVEncChnAttr.VeAttr.AttrH265e.mbByFrame = TRUE;
        mVEncChnAttr.VeAttr.AttrH265e.mPicWidth = mVideoWidth;
        mVEncChnAttr.VeAttr.AttrH265e.mPicHeight = mVideoHeight;
        mVEncChnAttr.VeAttr.AttrH265e.IQpOffset = mIQpOffset;
        //mVEncChnAttr.VeAttr.AttrH265e.mbLongTermRef = mbLongTermRef;
        mVEncChnAttr.VeAttr.AttrH265e.mFastEncFlag = mFastEncFlag;
        //mVEncChnAttr.VeAttr.AttrH265e.mVirtualIFrameInterval = mVirtualIFrameInterval;
        mVEncChnAttr.VeAttr.AttrH265e.mbPIntraEnable = mbPIntraEnable;
    }
    else if(PT_MJPEG == mVEncChnAttr.VeAttr.Type)
    {
        mVEncChnAttr.VeAttr.AttrMjpeg.mBufSize = mVEncAttr.mBufSize;
        mVEncChnAttr.VeAttr.AttrMjpeg.mbByFrame = TRUE;
        mVEncChnAttr.VeAttr.AttrMjpeg.mPicWidth = mVideoWidth;
        mVEncChnAttr.VeAttr.AttrMjpeg.mPicHeight = mVideoHeight;
    }
    else
    {
        aloge("fatal error! unsupported temporary");
    }
    if(mVideoRCMode!=mVEncRcAttr.mRcMode)
    {
        aloge("fatal error! why rcMode[0x%x]!=[0x%x], check code!", mVideoRCMode, mVEncRcAttr.mRcMode);
    }
    if(PT_H264 == mVEncChnAttr.VeAttr.Type)
    {
        switch(mVideoRCMode)
        {
            case VideoRCMode_CBR:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
                break;
            case VideoRCMode_VBR:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264VBR;
                break;
            case VideoRCMode_FIXQP:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264FIXQP;
                break;
            case VideoRCMode_ABR:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264ABR;
                break;
            case VideoRCMode_QPMAP:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264QPMAP;
                break;
            default:
                aloge("fatal error! unknown rcMode[%d]", mVideoRCMode);
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
                break;
        }
        if(VENC_RC_MODE_H264CBR == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrH264Cbr.mBitRate = mVEncRcAttr.mAttrH264Cbr.mBitRate;
            mVEncChnAttr.RcAttr.mAttrH264Cbr.mMaxQp = mVEncRcAttr.mAttrH264Cbr.mMaxQp;
            mVEncChnAttr.RcAttr.mAttrH264Cbr.mMinQp = mVEncRcAttr.mAttrH264Cbr.mMinQp;
        }
        else if(VENC_RC_MODE_H264VBR == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrH264Vbr.mMaxBitRate = mVEncRcAttr.mAttrH264Vbr.mMaxBitRate;
            mVEncChnAttr.RcAttr.mAttrH264Vbr.mMaxQp = mVEncRcAttr.mAttrH264Vbr.mMaxQp;
            mVEncChnAttr.RcAttr.mAttrH264Vbr.mMinQp = mVEncRcAttr.mAttrH264Vbr.mMinQp;
            mVEncChnAttr.RcAttr.mAttrH264Vbr.mMovingTh = mVEncRcAttr.mAttrH264Vbr.mMovingTh;
            mVEncChnAttr.RcAttr.mAttrH264Vbr.mQuality = mVEncRcAttr.mAttrH264Vbr.mQuality;
        }
        else if(VENC_RC_MODE_H264FIXQP == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrH264FixQp.mIQp = mVEncRcAttr.mAttrH264FixQp.mIQp;
            mVEncChnAttr.RcAttr.mAttrH264FixQp.mPQp = mVEncRcAttr.mAttrH264FixQp.mPQp;
        }
        else if(VENC_RC_MODE_H264ABR == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrH264Abr.mMaxBitRate = mVEncRcAttr.mAttrH264Abr.mMaxBitRate;
            mVEncChnAttr.RcAttr.mAttrH264Abr.mRatioChangeQp = mVEncRcAttr.mAttrH264Abr.mRatioChangeQp;
            mVEncChnAttr.RcAttr.mAttrH264Abr.mQuality = mVEncRcAttr.mAttrH264Abr.mQuality;
            mVEncChnAttr.RcAttr.mAttrH264Abr.mMinIQp = mVEncRcAttr.mAttrH264Abr.mMinIQp;
            mVEncChnAttr.RcAttr.mAttrH264Abr.mMaxIQp = mVEncRcAttr.mAttrH264Abr.mMaxIQp;
            mVEncChnAttr.RcAttr.mAttrH264Abr.mMaxQp = mVEncRcAttr.mAttrH264Abr.mMaxQp;
            mVEncChnAttr.RcAttr.mAttrH264Abr.mMinQp = mVEncRcAttr.mAttrH264Abr.mMinQp;
        }
        else if(VENC_RC_MODE_H264QPMAP == mVEncChnAttr.RcAttr.mRcMode)
        {
            aloge("QPMap not support now!");
        }
        else
        {
            aloge("fatal error! unknown rcMode[%d]", mVEncChnAttr.RcAttr.mRcMode);
        }
    }
    else if(PT_H265 == mVEncChnAttr.VeAttr.Type)
    {
        switch(mVideoRCMode)
        {
            case VideoRCMode_CBR:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
                break;
            case VideoRCMode_VBR:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265VBR;
                break;
            case VideoRCMode_FIXQP:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265FIXQP;
                break;
            case VideoRCMode_ABR:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265ABR;
                break;
            case VideoRCMode_QPMAP:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265QPMAP;
                break;
            default:
                aloge("fatal error! unknown rcMode[%d]", mVideoRCMode);
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
                break;
        }
        if(VENC_RC_MODE_H265CBR == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrH265Cbr.mBitRate = mVEncRcAttr.mAttrH265Cbr.mBitRate;
            mVEncChnAttr.RcAttr.mAttrH265Cbr.mMaxQp = mVEncRcAttr.mAttrH265Cbr.mMaxQp;
            mVEncChnAttr.RcAttr.mAttrH265Cbr.mMinQp = mVEncRcAttr.mAttrH265Cbr.mMinQp;
        }
        else if(VENC_RC_MODE_H265VBR == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrH265Vbr.mMaxBitRate = mVEncRcAttr.mAttrH265Vbr.mMaxBitRate;
            mVEncChnAttr.RcAttr.mAttrH265Vbr.mMaxQp = mVEncRcAttr.mAttrH265Vbr.mMaxQp;
            mVEncChnAttr.RcAttr.mAttrH265Vbr.mMinQp = mVEncRcAttr.mAttrH265Vbr.mMinQp;
            mVEncChnAttr.RcAttr.mAttrH265Vbr.mMovingTh = mVEncRcAttr.mAttrH265Vbr.mMovingTh;
            mVEncChnAttr.RcAttr.mAttrH265Vbr.mQuality = mVEncRcAttr.mAttrH265Vbr.mQuality;
        }
        else if(VENC_RC_MODE_H265FIXQP == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrH265FixQp.mIQp = mVEncRcAttr.mAttrH265FixQp.mIQp;
            mVEncChnAttr.RcAttr.mAttrH265FixQp.mPQp = mVEncRcAttr.mAttrH265FixQp.mPQp;
        }
        else if(VENC_RC_MODE_H265ABR == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrH265Abr.mMaxBitRate = mVEncRcAttr.mAttrH265Abr.mMaxBitRate;
            mVEncChnAttr.RcAttr.mAttrH265Abr.mRatioChangeQp = mVEncRcAttr.mAttrH265Abr.mRatioChangeQp;
            mVEncChnAttr.RcAttr.mAttrH265Abr.mQuality = mVEncRcAttr.mAttrH265Abr.mQuality;
            mVEncChnAttr.RcAttr.mAttrH265Abr.mMinIQp = mVEncRcAttr.mAttrH265Abr.mMinIQp;
            mVEncChnAttr.RcAttr.mAttrH265Abr.mMaxIQp = mVEncRcAttr.mAttrH265Abr.mMaxIQp;
            mVEncChnAttr.RcAttr.mAttrH265Abr.mMaxQp = mVEncRcAttr.mAttrH265Abr.mMaxQp;
            mVEncChnAttr.RcAttr.mAttrH265Abr.mMinQp = mVEncRcAttr.mAttrH265Abr.mMinQp;
        }
        else if(VENC_RC_MODE_H265QPMAP == mVEncChnAttr.RcAttr.mRcMode)
        {
            aloge("QPMap not support now!");
        }
        else
        {
            aloge("fatal error! unknown rcMode[%d]", mVEncChnAttr.RcAttr.mRcMode);
        }
    }
    else if(PT_MJPEG == mVEncChnAttr.VeAttr.Type)
    {
        switch(mVideoRCMode)
        {
            case VideoRCMode_CBR:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
                break;
            case VideoRCMode_FIXQP:
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGFIXQP;
                break;
            default:
                aloge("fatal error! unknown rcMode[%d]", mVideoRCMode);
                mVEncChnAttr.RcAttr.mRcMode = VENC_RC_MODE_MJPEGCBR;
                break;
        }
        if(VENC_RC_MODE_MJPEGCBR == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrMjpegeCbr.mBitRate = mVEncRcAttr.mAttrMjpegCbr.mBitRate;
        }
        else if(VENC_RC_MODE_MJPEGFIXQP == mVEncChnAttr.RcAttr.mRcMode)
        {
            mVEncChnAttr.RcAttr.mAttrMjpegeFixQp.mQfactor = mVEncRcAttr.mAttrMjpegFixQp.mQfactor;
        }
        else
        {
            aloge("fatal error! unknown rcMode[%d]", mVEncChnAttr.RcAttr.mRcMode);
        }
    }
    else
    {
        aloge("fatal error! unsupported temporary");
    }

    //set BufSize
//    unsigned int nBufSize = 0;
//    unsigned int nThreshSize = mVideoWidth*mVideoHeight;
//    int nBitRate = GetBitRateFromVENC_CHN_ATTR_S(&mVEncChnAttr);
//    if(mVideoEncodingBufferTime > 0)
//    {
//        nBufSize = (unsigned int)((uint64_t)nBitRate*mVideoEncodingBufferTime/(8*1000))+ nThreshSize;
//    }
//    if(PT_H264 == mVEncChnAttr.VeAttr.Type)
//    {
//        mVEncChnAttr.VeAttr.AttrH264e.BufSize = nBufSize;
//    }
//    else if(PT_H265 == mVEncChnAttr.VeAttr.Type)
//    {
//        mVEncChnAttr.VeAttr.AttrH265e.mBufSize = nBufSize;
//    }
//    else if(PT_MJPEG == mVEncChnAttr.VeAttr.Type)
//    {
//        mVEncChnAttr.VeAttr.AttrMjpeg.mBufSize = nBufSize;
//    }
//    else
//    {
//        aloge("fatal error! unsupported temporary");
//    }
    return NO_ERROR;
}

status_t EyeseeRecorder::config_MUX_GRP_ATTR_S()
{
    memset(&mMuxGrpAttr, 0, sizeof(MUX_GRP_ATTR_S));
    if(mVeChn >= 0)
    {
        mMuxGrpAttr.mVideoEncodeType = mVideoEncoder;
        if(PT_MAX!=mMuxGrpAttr.mVideoEncodeType)
        {
            mMuxGrpAttr.mWidth = mVideoWidth;
            mMuxGrpAttr.mHeight = mVideoHeight;
            CameraParameters param;
            mpCameraProxy->getParameters(mCameraSourceChannel, param);
            mMuxGrpAttr.mVideoFrmRate = mFrameRate*1000; //param.getPreviewFrameRate()*1000;
            mMuxGrpAttr.mMaxKeyInterval = mFrameRate; //param.getPreviewFrameRate();
        }
        else
        {
            aloge("fatal error! why veChn create when vencType is not known?");
        }
    }
    else
    {
        mMuxGrpAttr.mVideoEncodeType = PT_MAX;
    }

    if(mAeChn >= 0)
    {
        mMuxGrpAttr.mAudioEncodeType = mAudioEncoder;
        if(PT_MAX!=mMuxGrpAttr.mAudioEncodeType)
        {
            mMuxGrpAttr.mChannels = mAudioChannels;
            mMuxGrpAttr.mBitsPerSample = 16;
            mMuxGrpAttr.mSamplesPerFrame = MAXDECODESAMPLE;
            mMuxGrpAttr.mSampleRate = mSampleRate;
        }
        else
        {
            aloge("fatal error! why aeChn create when aencType is not known?");
        }
    }
    else
    {
        mMuxGrpAttr.mAudioEncodeType = PT_MAX;
    }
    
    if(mTeChn >= 0)
    {
        mMuxGrpAttr.mTextEncodeType = PT_TEXT;
    }
    else
    {
        mMuxGrpAttr.mTextEncodeType = PT_MAX;
    }

    
    return NO_ERROR;
}

status_t EyeseeRecorder::getEncDataHeader(VencHeaderData *pEncDataHeader)
{
    if(mCurrentState != MEDIA_RECORDER_PREPARED && mCurrentState != MEDIA_RECORDER_RECORDING)
    {
        alogw("can't get EncDataHeader before prepare()");
        return INVALID_OPERATION;
    }
    if(SUCCESS != AW_MPI_VENC_GetH264SpsPpsInfo(mVeChn, pEncDataHeader))
    {
        return UNKNOWN_ERROR;
    }
    return NO_ERROR;
}

status_t EyeseeRecorder::pushOneBsFrame(CDXRecorderBsInfo *frame)
{
    int total_bs_size=0;
    for (int i=0; i<frame->bs_count; i++) 
    {
        total_bs_size+=frame->bs_size[i];
    }
    if(total_bs_size != frame->total_size)
    {
        aloge("fatal error! BsFrameSize[%d]!=[%d], check code!", total_bs_size, frame->total_size);
    }

    if (frame->mode == 0)
    {
        RawPacketHeader *pRawPacketHeader = (RawPacketHeader*)frame->bs_data[0];
        if(pRawPacketHeader->size != frame->total_size - frame->bs_size[0])
        {
            aloge("fatal error! BsFrameDataSize[%d]!=[%d], check code!", pRawPacketHeader->size, frame->total_size - frame->bs_size[0]);
        }
        int streamType = pRawPacketHeader->stream_type;
        if ((mCallbackOutDataType==CALLBACK_OUT_DATA_VIDEO_ONLY && streamType!=RawPacketTypeVideo && streamType!=RawPacketTypeVideoExtra) 
            || (mCallbackOutDataType==CALLBACK_OUT_DATA_AUDIO_ONLY && streamType!=RawPacketTypeAudio && streamType!=RawPacketTypeAudioExtra))
        {
            return UNKNOWN_ERROR;
        }
    }
    else if(frame->mode == 1)
    {
        TSPacketHeader *pTsPacketHeader = (TSPacketHeader*)frame->bs_data[0];
        if(pTsPacketHeader->size != frame->total_size - frame->bs_size[0])
        {
            aloge("fatal error! BsFrameDataSize[%d]!=[%d], check code!", pTsPacketHeader->size, frame->total_size - frame->bs_size[0]);
        }
    }
    Mutex::Autolock autoLock(mEncBufLock);
    if(mIdleEncBufList.empty())
    {
        alogw("EncBufList is full! ReadyNum[%d], GetOutNum[%d]", mReadyEncBufList.size(), mGetOutEncBufList.size());
        return NO_MEMORY;
    }
    std::list<VEncBuffer>::iterator it = mIdleEncBufList.begin();
    //config VEncBuffer
    if(0 == frame->mode)
    {
        RawPacketHeader *pRawPacketHeader = (RawPacketHeader*)frame->bs_data[0];
        it->total_size = frame->total_size;
        it->stream_type = pRawPacketHeader->stream_type;
        it->data_size = pRawPacketHeader->size;
        it->pts = pRawPacketHeader->pts;
        it->CurrQp = pRawPacketHeader->CurrQp;
        it->avQp = pRawPacketHeader->avQp;
        it->nGopIndex = pRawPacketHeader->nGopIndex;
        it->nFrameIndex = pRawPacketHeader->nFrameIndex;
        it->nTotalIndex = pRawPacketHeader->nTotalIndex;
        if(it->data_size > 0)
        {
            it->data = (char*)malloc(it->data_size);
            if(NULL == it->data)
            {
                aloge("fatal error! malloc fail!");
            }
            int nCopyLen = 0;
            for(int i=1; i<frame->bs_count; i++)
            {
                memcpy(it->data+nCopyLen, frame->bs_data[i], frame->bs_size[i]);
                nCopyLen += frame->bs_size[i];
            }
        }
        else
        {
            aloge("fatal error! BsFrameDataSize == 0!");
            it->data = NULL;
        }
    }
    else if(1 == frame->mode)
    {
        TSPacketHeader *pTsPacketHeader = (TSPacketHeader*)frame->bs_data[0];
        it->total_size = frame->total_size;
        it->stream_type = pTsPacketHeader->stream_type;
        it->data_size = pTsPacketHeader->size;
        it->pts = pTsPacketHeader->pts;
        it->CurrQp = 0;
        it->avQp = 0;
        it->nGopIndex = 0;
        it->nFrameIndex = 0;
        it->nTotalIndex = 0;
        if(it->data_size > 0)
        {
            it->data = (char*)malloc(it->data_size);
            if(NULL == it->data)
            {
                aloge("fatal error! malloc fail!");
            }
            int nCopyLen = 0;
            for(int i=1; i<frame->bs_count; i++)
            {
                memcpy(it->data+nCopyLen, frame->bs_data[i], frame->bs_size[i]);
                nCopyLen += frame->bs_size[i];
            }
        }
        else
        {
            aloge("fatal error! BsFrameDataSize == 0!");
            it->data = NULL;
        }
    }
    else
    {
        aloge("fatal error! unsupport callback mode[%d]!", frame->mode);
    }
    //move to ready list
    mReadyEncBufList.splice(mReadyEncBufList.end(), mIdleEncBufList, it);
    return NO_ERROR;
}

VEncBuffer* EyeseeRecorder::getOneBsFrame()
{
    Mutex::Autolock autoLock(mEncBufLock);
    if(mReadyEncBufList.empty())
    {
        alogv("ReadyBufList is empty! GetOutBufList[%d]", mGetOutEncBufList.size());
        return NULL;
    }
    mGetOutEncBufList.splice(mGetOutEncBufList.end(), mReadyEncBufList, mReadyEncBufList.begin());
    return &mGetOutEncBufList.back();
}

void EyeseeRecorder::freeOneBsFrame(VEncBuffer *pEncData)
{
    Mutex::Autolock autoLock(mEncBufLock);
    if(NULL == pEncData)
    {
        return;
    }
    if(mGetOutEncBufList.empty())
    {
        aloge("fatal error! GetOutBufList is empty");
        return;
    }
    int nFindFlag = 0;
    std::list<VEncBuffer>::iterator DstIt;
    for(std::list<VEncBuffer>::iterator it = mGetOutEncBufList.begin(); it != mGetOutEncBufList.end(); ++it)
    {
        if(&*it == pEncData)
        {
            alogv("find EncBuf[%p]", pEncData);
            if(0 == nFindFlag)
            {
                DstIt = it;
            }
            nFindFlag++;
            //break;
        }
    }
    if(nFindFlag <= 0)
    {
        aloge("fatal error! not find in GetOutEncBufList, pEncData[%p]", pEncData);
    }
    else if(nFindFlag > 1)
    {
        aloge("fatal error! find [%d]nodes in GetOutEncBufList, pEncData[%p]", nFindFlag, pEncData);
    }
    else
    {
        if(DstIt->data)
        {
            free(DstIt->data);
            DstIt->data = NULL;
        }
        memset(&*DstIt, 0, sizeof(VEncBuffer));
        mIdleEncBufList.splice(mIdleEncBufList.end(), mGetOutEncBufList, DstIt);
    }
}
/*
bool EyeseeRecorder::compareOSDRectPriority(const VEncOSDRectInfo& first, const VEncOSDRectInfo& second)
{
    if (first.nPriority < second.nPriority)
    {
        return true;
    }
    return false;
}

status_t EyeseeRecorder::setOSDRects(std::list<VEncOSDRectInfo> &rects)
{
    mOSDRects = rects;
    mOSDRects.sort(compareOSDRectPriority);

    VENC_OVERLAY_INFO stOsdRegion;
    memset(&stOsdRegion, 0, sizeof(VENC_OVERLAY_INFO));
    stOsdRegion.regionNum = mOSDRects.size();
    if (stOsdRegion.regionNum > 0)
    {
        int pixelSize = 0;
        if (OSD_BITMAP_ARGB8888 == mOSDRects.begin()->mFormat)
        {
            stOsdRegion.nBitMapColorType = BITMAP_COLOR_ARGB8888;
            pixelSize = 4;
        }
        else if(OSD_BITMAP_ARGB4444 == mOSDRects.begin()->mFormat)
        {
            stOsdRegion.nBitMapColorType = BITMAP_COLOR_ARGB4444;
            pixelSize = 2;
        }
        else
        {
            stOsdRegion.nBitMapColorType = BITMAP_COLOR_ARGB1555;
            pixelSize = 2;
        }

        std::list<VEncOSDRectInfo>::iterator it;
        int i = 0;
        int bufSize = 0;
        for(it=mOSDRects.begin(); it!=mOSDRects.end(); ++it)
        {
            stOsdRegion.region[i].rect.X = it->mRect.X;
            stOsdRegion.region[i].rect.Y = it->mRect.Y;
            stOsdRegion.region[i].rect.Width = it->mRect.Width;
            stOsdRegion.region[i].rect.Height = it->mRect.Height;
            if (it->mOsdType == OSD_TYPE_OVERLAY)
            {
                stOsdRegion.region[i].bOverlayType = OVERLAY_STYLE_NORMAL;
            }
            else if (it->mOsdType == OSD_TYPE_COVER)
            {
                stOsdRegion.region[i].bOverlayType = OVERLAY_STYLE_COVER;
            }
            else
            {
                stOsdRegion.region[i].bOverlayType = OVERLAY_STYLE_LUMA_REVERSE;
            }
            stOsdRegion.region[i].bRegionID = i;
            stOsdRegion.region[i].nPriority = it->nPriority;
            stOsdRegion.region[i].coverYUV.bCoverY = (it->coverYUVColor >> 16) & 0xFF;
            stOsdRegion.region[i].coverYUV.bCoverU = (it->coverYUVColor >> 8) & 0xFF;
            stOsdRegion.region[i].coverYUV.bCoverV = (it->coverYUVColor & 0xFF);
            stOsdRegion.region[i].extraAlphaFlag = 0;
            stOsdRegion.region[i].extraAlphaVal = 0;
            stOsdRegion.region[i].pBitMapAddr = it->mpBuf;
            stOsdRegion.region[i].nBitMapSize = pixelSize * it->mRect.Width * it->mRect.Height;
            bufSize += stOsdRegion.region[i].nBitMapSize;
            i++;
        }
    }
    AW_MPI_VENC_setOsdMaskRegions(mVeChn, &stOsdRegion);

    return NO_ERROR;
}
*/
RGN_HANDLE EyeseeRecorder::createRegion(const RGN_ATTR_S *pstRegion)
{
    Mutex::Autolock lock(mRgnLock);
    ERRORTYPE ret;
    bool bSuccess = false;
    RGN_HANDLE handle = 0;
    while(handle < RGN_HANDLE_MAX)
    {
        ret = AW_MPI_RGN_Create(handle, pstRegion);
        if(SUCCESS == ret)
        {
            bSuccess = true;
            alogv("create region[%d] success!", handle);
            break;
        }
        else if(ERR_RGN_EXIST == ret)
        {
            alogv("region[%d] is exist, find next!", handle);
            handle++;
        }
        else
        {
            aloge("fatal error! create region[%d] ret[0x%x]!", handle, ret);
            break;
        }
    }
    if(bSuccess)
    {
        mRgnHandleList.push_back(handle);
        return handle;
    }
    else
    {
        alogd("create region fail");
        return MM_INVALID_HANDLE;
    }
}

status_t EyeseeRecorder::setRegionBitmap(RGN_HANDLE Handle, const BITMAP_S *pBitmap)
{
    Mutex::Autolock lock(mRgnLock);
    status_t result = NO_ERROR;
    bool bExist = false;
    for(RGN_HANDLE& i : mRgnHandleList)
    {
        if(i == Handle)
        {
            bExist = true;
            break;
        }
    }
    if(bExist)
    {
        ERRORTYPE ret = AW_MPI_RGN_SetBitMap(Handle, pBitmap);
        if(SUCCESS == ret)
        {
            result = NO_ERROR;
        }
        else
        {
            result = UNKNOWN_ERROR;
        }
    }
    else
    {
        result = UNKNOWN_ERROR;
    }
    return result;
}

status_t EyeseeRecorder::attachRegionToVenc(RGN_HANDLE Handle, const RGN_CHN_ATTR_S *pstChnAttr)
{
    status_t result = NO_ERROR;
    Mutex::Autolock autoLock(mLock);
    if (!(mCurrentState&MEDIA_RECORDER_PREPARED || mCurrentState&MEDIA_RECORDER_RECORDING))
    {
        alogw("Be careful! current state[0x%x], must set region in state prepared or recording!", mCurrentState);
        return INVALID_OPERATION;
    }
    if(mVeChn < 0)
    {
        aloge("fatal error! venc is not created");
        return UNKNOWN_ERROR;
    }
    Mutex::Autolock lock(mRgnLock);
    bool bExist = false;
    for(RGN_HANDLE& i : mRgnHandleList)
    {
        if(i == Handle)
        {
            bExist = true;
            break;
        }
    }
    if(bExist)
    {
        MPP_CHN_S stChn = {MOD_ID_VENC, 0, mVeChn};
        ERRORTYPE ret = AW_MPI_RGN_AttachToChn(Handle, &stChn, pstChnAttr);
        if(SUCCESS == ret)
        {
            result = NO_ERROR;
        }
        else
        {
            result = UNKNOWN_ERROR;
        }
    }
    else
    {
        aloge("fatal error! region[%d] is unexist!", Handle);
        result = UNKNOWN_ERROR;
    }
    return result;
}

status_t EyeseeRecorder::detachRegionFromVenc(RGN_HANDLE Handle)
{
    status_t result = NO_ERROR;
    Mutex::Autolock autoLock(mLock);
    if(mVeChn < 0)
    {
        aloge("fatal error! venc is not created");
        return UNKNOWN_ERROR;
    }
    Mutex::Autolock lock(mRgnLock);
    bool bExist = false;
    for(RGN_HANDLE& i : mRgnHandleList)
    {
        if(i == Handle)
        {
            bExist = true;
            break;
        }
    }
    if(bExist)
    {
        MPP_CHN_S stChn = {MOD_ID_VENC, 0, mVeChn};
        ERRORTYPE ret = AW_MPI_RGN_DetachFromChn(Handle, &stChn);
        if(SUCCESS == ret)
        {
            result = NO_ERROR;
        }
        else
        {
            result = UNKNOWN_ERROR;
        }
    }
    else
    {
        aloge("fatal error! region[%d] is unexist!", Handle);
        result = UNKNOWN_ERROR;
    }
    return result;
}

status_t EyeseeRecorder::setRegionDisplayAttrOfVenc(RGN_HANDLE Handle, const RGN_CHN_ATTR_S *pstChnAttr)
{
    status_t result = NO_ERROR;
    Mutex::Autolock autoLock(mLock);
    if(mVeChn < 0)
    {
        aloge("fatal error! venc is not created");
        return UNKNOWN_ERROR;
    }
    Mutex::Autolock lock(mRgnLock);
    bool bExist = false;
    for(RGN_HANDLE& i : mRgnHandleList)
    {
        if(i == Handle)
        {
            bExist = true;
            break;
        }
    }
    if(bExist)
    {
        MPP_CHN_S stChn = {MOD_ID_VENC, 0, mVeChn};
        ERRORTYPE ret = AW_MPI_RGN_SetDisplayAttr(Handle, &stChn, pstChnAttr);
        if(SUCCESS == ret)
        {
            result = NO_ERROR;
        }
        else
        {
            result = UNKNOWN_ERROR;
        }
    }
    else
    {
        aloge("fatal error! region[%d] is unexist!", Handle);
        result = UNKNOWN_ERROR;
    }
    return result;
}
status_t EyeseeRecorder::getRegionDisplayAttrOfVenc(RGN_HANDLE Handle, RGN_CHN_ATTR_S *pstChnAttr)
{
    status_t result = NO_ERROR;
    Mutex::Autolock autoLock(mLock);
    if(mVeChn < 0)
    {
        aloge("fatal error! venc is not created");
        return UNKNOWN_ERROR;
    }
    Mutex::Autolock lock(mRgnLock);
    bool bExist = false;
    for(RGN_HANDLE& i : mRgnHandleList)
    {
        if(i == Handle)
        {
            bExist = true;
            break;
        }
    }
    if(bExist)
    {
        MPP_CHN_S stChn = {MOD_ID_VENC, 0, mVeChn};
        ERRORTYPE ret = AW_MPI_RGN_GetDisplayAttr(Handle, &stChn, pstChnAttr);
        if(SUCCESS == ret)
        {
            result = NO_ERROR;
        }
        else
        {
            result = UNKNOWN_ERROR;
        }
    }
    else
    {
        aloge("fatal error! region[%d] is unexist!", Handle);
        result = UNKNOWN_ERROR;
    }
    return result;
}
status_t EyeseeRecorder::destroyRegion(RGN_HANDLE Handle)
{
    status_t result = NO_ERROR;
    Mutex::Autolock lock(mRgnLock);
    bool bExist = false;
    std::list<RGN_HANDLE>::iterator it;
    for(it = mRgnHandleList.begin(); it != mRgnHandleList.end(); ++it)
    {
        if(*it == Handle)
        {
            bExist = true;
            break;
        }
    }
    if(bExist)
    {
        ERRORTYPE ret = AW_MPI_RGN_Destroy(Handle);
        if(SUCCESS == ret)
        {
            mRgnHandleList.erase(it);
            result = NO_ERROR;
        }
        else
        {
            result = UNKNOWN_ERROR;
        }
    }
    else
    {
        aloge("fatal error! region[%d] is unexist!", Handle);
        result = UNKNOWN_ERROR;
    }
    return result;
}

};

