#ifndef __CAMERA_V4L2_H__
#define __CAMERA_V4L2_H__

#include <CameraInterface.h>
#include "CameraParseConfig.h"
#include "CameraLog.h"

int V4L2QueryCap(void *hdl);
int V4L2SetInput(void *hdl);
int V4L2EnumFmt(void *hdl);
int V4L2TryFmt(void *hdl);
int V4L2EnumFrameSizes(void *hdl);
int V4L2EnumFrameIntervals(void *hdl);
int V4L2GetParm(void *hdl);
int V4L2SetParm(void *hdl);
int V4L2GetFormat(void *hdl);
int V4L2SetFormat(void *hdl);
int V4L2RequestQueue(void *hdl);
int V4L2ReleaseQueue(void *hdl);
int V4L2StreamOn(void *hdl);
int V4L2StreamOff(void *hdl);
int V4L2DQBUF(void *hdl, void *camerabuffer);
int V4L2QBUF(void *hdl, void *camerabuffer);

int CameraGetConfig(void *hdl);
int CameraSetConfig(void *hdl);
int CameraUpdateSupportConfig(void *hdl);
/*
CameraPixelFormat FormatDriverToUsr(unsigned int fourcc);
unsigned int FormatUsrToDriver(CameraPixelFormat format);
unsigned char *FormatUsrToStr(CameraPixelFormat format);
CameraPixelFormat FormatStrToUsr(unsigned char *name);
unsigned int FormatStrToDriver(unsigned char *name);
unsigned char *FormatDriverToStr(unsigned int fourcc);
*/
#endif
