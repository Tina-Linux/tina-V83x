/*
* Copyright (c) 2017-2020 Allwinner Technology Co. Ltd.
* All rights reserved.
*
* File Name: disp.h
* Description : Display engine interface
* History :
*   Author  : allwinnertech
*   Date    : 2017/05/16
*   Comment : first version
*
*/

#ifndef __VIDEOOUTPORT_H__
#define __VIDEOOUTPORT__

typedef enum{
	DISP_VIEW_MODE_TOP,
	DISP_VIEW_MODE_BOTTOM,
	DISP_VIEW_MODE_MAX
}DISP_VIEW_MODE;

typedef enum{
	DISP_LAYER_ID_0,
	DISP_LAYER_ID_1,
	DISP_LAYER_ID_2,
	DISP_LAYER_ID_3,
	DISP_LAYER_ID_MAX
}DISP_LAYER_ID;

typedef enum {
	VIDEO_CMD_SET_BRIGHTNESS = 0,
	VIDEO_CMD_SET_CONTRAST,
	VIDEO_CMD_SET_HUE,
	VIDEO_CMD_SET_SATURATION,
	VIDEO_CMD_SET_VEDIO_ENHANCE_DEFAULT,
}DISP_CMD_TYPE;
typedef enum
{
    VIDEO_PIXEL_FORMAT_DEFAULT            = 0,

    VIDEO_PIXEL_FORMAT_YUV_PLANER_420     = 1,
    VIDEO_PIXEL_FORMAT_YUV_PLANER_422     = 2,
    VIDEO_PIXEL_FORMAT_YUV_PLANER_444     = 3,

    VIDEO_PIXEL_FORMAT_YV12               = 4,
    VIDEO_PIXEL_FORMAT_NV21               = 5,
    VIDEO_PIXEL_FORMAT_NV12               = 6,
    VIDEO_PIXEL_FORMAT_YUV_MB32_420       = 7,
    VIDEO_PIXEL_FORMAT_YUV_MB32_422       = 8,
    VIDEO_PIXEL_FORMAT_YUV_MB32_444       = 9,

    VIDEO_PIXEL_FORMAT_RGBA                = 10,
    VIDEO_PIXEL_FORMAT_ARGB                = 11,
    VIDEO_PIXEL_FORMAT_ABGR                = 12,
    VIDEO_PIXEL_FORMAT_BGRA                = 13,

    VIDEO_PIXEL_FORMAT_YUYV                = 14,
    VIDEO_PIXEL_FORMAT_YVYU                = 15,
    VIDEO_PIXEL_FORMAT_UYVY                = 16,
    VIDEO_PIXEL_FORMAT_VYUY                = 17,

    VIDEO_PIXEL_FORMAT_PLANARUV_422        = 18,
    VIDEO_PIXEL_FORMAT_PLANARVU_422        = 19,
    VIDEO_PIXEL_FORMAT_PLANARUV_444        = 20,
    VIDEO_PIXEL_FORMAT_PLANARVU_444        = 21,

    VIDEO_PIXEL_FORMAT_MIN = VIDEO_PIXEL_FORMAT_DEFAULT,
    VIDEO_PIXEL_FORMAT_MAX = VIDEO_PIXEL_FORMAT_YUV_PLANER_444,
}VideoPixelFormat;

typedef struct
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
}VoutRect;
typedef struct
{
	unsigned int x;
	unsigned int y;
	unsigned int width;
	unsigned int height;
}ViewRect;

typedef enum
{
	VIDEO_BT601  = 0,
	VIDEO_BT709  = 1,
	VIDEO_YCC    = 2,
}Vcolor_space;

typedef struct
{
	unsigned int w;
	unsigned int h;
	unsigned int crop_x;
	unsigned int crop_y;
    unsigned int crop_w;
	unsigned int crop_h;
	VideoPixelFormat format;
	Vcolor_space color_space;
}SrcInfo;

typedef enum
{
	VIDEO_USE_INTERN_ALLOC_BUF  = 0,
	VIDEO_USE_EXTERN_ION_BUF  = 1,
}renderBufSrc;

typedef struct
{
	renderBufSrc isExtPhy;
	unsigned long phy_addr;
	unsigned long vir_addr;
	unsigned long y_phaddr; // the address of fb, contains top field luminance
	unsigned long u_phaddr; // the address of fb, contains top field chrominance
	unsigned long v_phaddr;
}renderBuf;

typedef struct
{
	int isPhy;
	SrcInfo srcInfo;
}videoParam;

typedef enum
{
	ROTATION_ANGLE_0 = 0,
	ROTATION_ANGLE_90 = 1,
	ROTATION_ANGLE_180 = 2,
	ROTATION_ANGLE_270 = 3
}rotateDegree;

typedef enum {
	VIDEO_SRC_FROM_ISP = 0,
	VIDEO_SRC_FROM_VE = 1,
	VIDEO_SRC_FROM_FILE = 2,
	VIDEO_SRC_FROM_CAM = 3
}videoSource;

typedef enum {
	VIDEO_ZORDER_TOP = 0,
	VIDEO_ZORDER_MIDDLE = 1,
	VIDEO_ZORDER_BOTTOM = 2
}videoZorder;
typedef enum
{
	LAYER_STATUS_REQUESTED = 1,
	LAYER_STATUS_NOTUSED = 2,
    LAYER_STATUS_OPENED = 4
}layerStatus;

typedef struct
{
	int (*init)(void *hdl, int enable, int rotate, VoutRect *rect);
	int (*deinit)(void *hdl);
	int (*writeData)(void *hdl, void *data, int size, videoParam *param);
	int (*dequeue)(void *hdl, renderBuf *rBuf);
	int (*queueToDisplay)(void *hdl, int size, videoParam *param, renderBuf *rBuf);
	int (*setEnable)(void *hdl, int enable);
	int (*setRect)(void *hdl, VoutRect *rect);
	int (*setRotateAngel)(void *hdl, int degree);
	int (*setRoute)(void *hdl, int route);
	int (*SetZorder)(void *hdl, int zorder);
	int (*getScreenWidth)(void *hdl);
	int (*getScreenHeight)(void *hdl);
	int (*allocateVideoMem)(void *hdl, videoParam *param);
	int (*freeVideoMem)(void *hdl);
	int (*setSrcRect)(void *hdl,VoutRect *rect);
	int (*setIoctl)(void *hdl, unsigned int cmd, unsigned long para);
	int disp_fd;
	int enable;
	int interBufSet[2];
	int bufindex[2];
	unsigned int hlayer;
	rotateDegree rotate;
	videoSource route;
	VoutRect rect;
	struct SunxiMemOpsS *pMemops;
	renderBuf renderbuf[2][2];
}dispOutPort;

int DispInit(void *hdl, int enable, int rotate, VoutRect *rect);
int DispDeinit(void *hdl);
int DispWriteData(void *hdl, void *data, int size, videoParam *param);

int DispDequeue(void *hdl, renderBuf *rBuf);
int DispQueueToDisplay(void *hdl, int size, videoParam *param, renderBuf * rBuf);

int DispSetEnable(void *hdl, int enable);
int DispSetRect(void *hdl, VoutRect *rect);
int DispSetRotateAngel(void *hdl, int degree);
int DispSetRoute(void *hdl, int route);
int DispSetZorder(void *hdl, int zorder);
int DispSetIoctl(void *hdl, unsigned int cmd, unsigned long para);

int DispGetScreenWidth(void *hdl);
int DispGetScreenHeight(void *hdl);

int DispAllocateVideoMem(void *hdl, videoParam *param);
int DispFreeVideoMem(void *hdl);
dispOutPort *CreateVideoOutport(int index);
int DestroyVideoOutport(dispOutPort *hdl);
#endif
