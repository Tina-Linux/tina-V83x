#ifndef VIDEO_RENDER_H
#define VIDEO_RENDER_H

#include "player_i.h"
#include "videoDecComponent.h"
#include "framerateEstimater.h"
#include "layerControl.h"

typedef void* VideoRenderComp;

VideoRenderComp* VideoRenderCompCreate(void);

int VideoRenderCompDestroy(VideoRenderComp* v);

int VideoRenderCompStart(VideoRenderComp* v);

int VideoRenderCompStop(VideoRenderComp* v);

int VideoRenderCompPause(VideoRenderComp* v);

enum EPLAYERSTATUS VideoRenderCompGetStatus(VideoRenderComp* v);

int VideoRenderCompReset(VideoRenderComp* v);

int VideoRenderCompSetEOS(VideoRenderComp* v);

int VideoRenderCompSetCallback(VideoRenderComp* v, PlayerCallback callback, void* pUserData);

int VideoRenderCompSetTimer(VideoRenderComp* v, AvTimer* timer);

int VideoRenderCompSetWindow(VideoRenderComp* v, void* pNativeWindow);

int VideoRenderCompSetDecodeComp(VideoRenderComp* v, VideoDecComp* d);

int VideoRenderSet3DMode(VideoRenderComp* v,
                         enum EPICTURE3DMODE ePicture3DMode,
                         enum EDISPLAY3DMODE eDisplay3DMode);

int VideoRenderGet3DMode(VideoRenderComp* v,
                         enum EPICTURE3DMODE* ePicture3DMode,
                         enum EDISPLAY3DMODE* eDisplay3DMode);

int VideoRenderVideoHide(VideoRenderComp* v, int bHideVideo);

int VideoRenderSetHoldLastPicture(VideoRenderComp* v, int bHold);

void VideoRenderCompSetProtecedFlag(VideoRenderComp* v, int bProtectedFlag);

int VideoRenderCompSetSyncFirstPictureFlag(VideoRenderComp* v, int bSyncFirstPictureFlag);

int VideoRenderCompSetFrameRateEstimater(VideoRenderComp* v, FramerateEstimater* fe);

int VideoRenderCompSetVideoStreamInfo(VideoRenderComp* v, VideoStreamInfo* pStreamInfo);

int VideoRenderCompSetLayerCtlOps(VideoRenderComp* v, LayerControlOpsT* ops);

#endif
