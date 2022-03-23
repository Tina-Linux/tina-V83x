/*
 * Copyright (c) 2008-2016 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File : videoRenderComponent.h
 * Description : video render component
 * History :
 *
 */

#ifndef VIDEO_RENDER_H
#define VIDEO_RENDER_H

#include "player_i.h"
#include "videoDecComponent.h"
#include "framerateEstimater.h"
#include "layerControl.h"

typedef struct VideoRenderComp VideoRenderComp;

VideoRenderComp* VideoRenderCompCreate(void);

int VideoRenderCompDestroy(VideoRenderComp* v);

int VideoRenderCompStart(VideoRenderComp* v);

int VideoRenderCompStop(VideoRenderComp* v);

int VideoRenderCompPause(VideoRenderComp* v);

enum EPLAYERSTATUS VideoRenderCompGetStatus(VideoRenderComp* v);

int VideoRenderCompReset(VideoRenderComp* v);

int VideoRenderCompSetEOS(VideoRenderComp* v);

int VideoRenderCompSetSeekInfo(VideoRenderComp* v, int bSeekClosest, int nSeekTimeMs);

int VideoRenderCompSetCallback(VideoRenderComp* v, PlayerCallback callback, void* pUserData);

int VideoRenderCompSetTimer(VideoRenderComp* v, AvTimer* timer);

int VideoRenderCompSetWindow(VideoRenderComp* v, LayerCtrl* lc);

int VideoRenderCompSetDeinterlace(VideoRenderComp* v, Deinterlace* pDi);

int VideoRenderCompSetDecodeComp(VideoRenderComp* v, VideoDecComp* d);

int VideoRenderSet3DMode(VideoRenderComp* v,
                         enum EPICTURE3DMODE ePicture3DMode,
                         enum EDISPLAY3DMODE eDisplay3DMode);

int VideoRenderGet3DMode(VideoRenderComp* v,
                         enum EPICTURE3DMODE* ePicture3DMode,
                         enum EDISPLAY3DMODE* eDisplay3DMode);

int VideoRenderSetPlayRate(VideoRenderComp* p,float rate);

int VideoRenderVideoHide(VideoRenderComp* v, int bHideVideo);

int VideoRenderSetHoldLastPicture(VideoRenderComp* v, int bHold);

void VideoRenderCompSetProtecedFlag(VideoRenderComp* v, int bProtectedFlag);

int VideoRenderCompSetSyncFirstPictureFlag(VideoRenderComp* v, int bSyncFirstPictureFlag);

int VideoRenderCompSetFrameRateEstimater(VideoRenderComp* v, FramerateEstimater* fe);

int VideoRenderCompSetVideoStreamInfo(VideoRenderComp* v, VideoStreamInfo* pStreamInfo);

int VideoRenderCompGetDispFrameRate(VideoRenderComp* p,float* dispFrameRate);

#endif
