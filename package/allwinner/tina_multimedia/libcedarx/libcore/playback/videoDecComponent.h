/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : videoDecComponent.h
 * Description : video decoder component
 * History :
 *
 */

#ifndef VIDEO_DECODE_COMPONENT
#define VIDEO_DECODE_COMPONENT

#include "player_i.h"
#include "vdecoder.h"
#include "avtimer.h"

enum VIDEORENDERCALLBACKMG
{
    VIDEO_RENDER_VIDEO_INFO = 0,
    VIDEO_RENDER_REQUEST_BUFFER,
    VIDEO_RENDER_DISPLAYER_BUFFER,
    VIDEO_RENDER_RETURN_BUFFER,
    VIDEO_RENDER_RESOLUTION_CHANGE,
};

typedef struct VideoDecComp VideoDecComp;

typedef int (*VideoRenderCallback)(void* pUserData, int eMessageId, void* param);

VideoDecComp* VideoDecCompCreate(void);

int VideoDecCompDestroy(VideoDecComp* v);

int VideoDecCompStart(VideoDecComp* v);

int VideoDecCompStop(VideoDecComp* v);

int VideoDecCompPause(VideoDecComp* v);

enum EPLAYERSTATUS VideoDecCompGetStatus(VideoDecComp* v);

int VideoDecCompReset(VideoDecComp* v);

int VideoDecCompSetEOS(VideoDecComp* v);

int VideoDecCompSetCallback(VideoDecComp* v, PlayerCallback callback, void* pUserData);

int VideoDecCompSetDecodeKeyFrameOnly(VideoDecComp* v, int bDecodeKeyFrameOnly);

int VideoDecCompSetDropDelayFrames(VideoDecComp* v, int bDropDelayFrames);

int VideoDecCompSetVideoStreamInfo(VideoDecComp* v, VideoStreamInfo* pStreamInfo,
        VConfig* pVconfig);

int VideoDecCompGetVideoStreamInfo(VideoDecComp* v, VideoStreamInfo* pStreamInfo);

int VideoDecCompSetTimer(VideoDecComp* v, AvTimer* timer);

int VideoDecCompRequestStreamBuffer(VideoDecComp* v,
                                    int           nRequireSize,
                                    char**        ppBuf,
                                    int*          pBufSize,
                                    char**        ppRingBuf,
                                    int*          pRingBufSize,
                                    int           nStreamIndex);

int VideoDecCompSubmitStreamData(VideoDecComp*        v,
                                 VideoStreamDataInfo* pDataInfo,
                                 int                  nStreamIndex);

int VideoDecCompStreamBufferSize(VideoDecComp* v, int nStreamIndex);

int VideoDecCompStreamDataSize(VideoDecComp* v, int nStreamIndex);

int VideoDecCompStreamFrameNum(VideoDecComp* v, int nStreamIndex);

VideoPicture* VideoDecCompRequestPicture(VideoDecComp* v, int nStreamIndex,
        int* bResolutionChanged);

int VideoDecCompReturnPicture(VideoDecComp* v, VideoPicture* pPicture);

VideoPicture* VideoDecCompNextPictureInfo(VideoDecComp* v, int nStreamIndex,
        int* bResolutionChanged);

int VideoDecCompTotalPictureBufferNum(VideoDecComp* v, int nStreamIndex);

int VideoDecCompEmptyPictureBufferNum(VideoDecComp* v, int nStreamIndex);

int VideoDecCompValidPictureNum(VideoDecComp* v, int nStreamIndex);

int VideoDecCompReopenVideoEngine(VideoDecComp* v);

int VideoDecCompRotatePicture(VideoDecComp* v,
                              VideoPicture* pPictureIn,
                              VideoPicture* pPictureOut,
                              int           nRotateDegree,
                              int           nGpuYAlign,
                              int           nGpuCAlign);

//VideoPicture* VideoDecCompAllocatePictureBuffer(int nWidth, int nHeight,
//            int nLineStride, int ePixelFormat);

//int VideoDecCompFreePictureBuffer(VideoPicture* pPicture);

#if 0
//*for new display
int VideoDecCompSetVideoRenderCallback(VideoDecComp* v, VideoRenderCallback callback,
        void* pUserData);
#endif

//*******************************  START  **********************************//
//** for new display structure interface.
//**
FbmBufInfo* VideoDecCompGetVideoFbmBufInfo(VideoDecComp* v);

VideoPicture* VideoDecCompSetVideoFbmBufAddress(VideoDecComp* v, VideoPicture* pVideoPicture,
       int bForbidUseFlag);

int VideoDecCompSetVideoFbmBufRelease(VideoDecComp* v);

VideoPicture* VideoDecCompRequestReleasePicture(VideoDecComp* v);

VideoPicture* VideoDecCompReturnRelasePicture(VideoDecComp* v, VideoPicture* pVpicture,
        int bForbidUseFlag);

int VideoDecCompSetExtraScaleInfo(VideoDecComp* v, int nWidthTh, int nHeightTh,
                                  int nHorizontalScaleRatio, int nVerticalScaleRatio);
//********************************  END  ***********************************//

void VideoDecCompFreePictureBuffer(VideoDecComp* v, VideoPicture* pPicture);

#endif
