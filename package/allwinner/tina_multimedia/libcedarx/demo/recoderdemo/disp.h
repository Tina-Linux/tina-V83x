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

#ifndef __DISP_H__
#define __DISP_H__

#define __SUNXI_DISPLAY2__ 1

#if defined (__SUNXI_DISPLAY__)
#include "sunxi_display1.h"
#elif defined (__SUNXI_DISPLAY2__)
#include "sunxi_display2.h"
#else
#error "no display header file include"
#endif

#define DISP_DEV_NAME								("/dev/disp")
#define DISP_FB0_NAME								("/dev/fb0")

#define DISP_INPUT_WIDTH							720
#define DISP_INPUT_HEIGHT							480

#define DISP_INPUT_FMT								DISP_FORMAT_YUV420_P

#define DISP_UI_ALPHA								0x7f
#define GUI_FRM_BUF_WIDTH_PIXEL						800
#define GUI_FRM_BUF_HEIGHT_PIXEL					480
#define DISP_FRAME_RATE								30

typedef unsigned char								INT8U;
typedef unsigned short								INT16U;
typedef unsigned int								INT32U;
typedef unsigned long long							INT64U;
typedef char										INT8S;
typedef signed short								INT16S;
typedef signed int									INT32S;
typedef signed long long							INT64S;

typedef enum {
	DISP_YUV420_PLANNAR,		//use to show CSI video
	DISP_MB32_PLANNAR,			//use to show picture/video that decoded by cedarx
	DISP_ARGB8888_INTERLEAVED,	//used to play ARGB8888 format image or video
	DISP_UNKOWN_FORMAT,
}DISP_INPUT_FORMAT;

typedef enum{
	DISP_IOCTL_ARG_OUT_SRC,
	DISP_IOCTL_ARG_OUT_LAYER,
	DISP_IOCTL_ARG_OUT_LAYER_PARAM,
	DISP_IOCTL_ARG_DUMMY,
	DISP_IOCTL_ARG_MAX
}DISP_IOCTL_ARG;

typedef enum{
	DISP_OUT_SRC_SEL_LCD = 0x00,
	DISP_OUT_SRC_SEL_TV,
	DISP_OUT_SRC_SEL_LVDS,
	DISP_OUT_SRC_SEL_HDMI,
	DISP_OUT_SRC_SEL_MAX
}DISP_OUT_SRC_SEL;

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
	DISP_CMD_SET_BRIGHTNESS = 0,
	DISP_CMD_SET_CONTRAST,
	DISP_CMD_SET_HUE,
	DISP_CMD_SET_SATURATION,
	DISP_CMD_SET_VEDIO_ENHANCE_DEFAULT,
}DISP_CMD_TYPE;

enum VIDEO_PIXEL_FORMAT {
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
};

struct video_win_info {
	int nWidth;
	int nHeight;
	int nLineStride;
	int nTopOffset;
	int nLeftOffset;
	int nBottomOffset;
	int nRightOffset;
};

struct scn_win_info {
    int nWidth;
    int nHeight;
    int nLeftOff;
    int nTopOff;
    int nDisplayWidth;
    int nDisplayHeight;
};

struct win_info {
	int nScreenWidth;
	int nScreenHeight;
	struct video_win_info src_win;
	struct scn_win_info scn_win;
};

int disp_layer_param_set(int dispFd, unsigned int dispLayer, void *dispLayerInfo);
int disp_layer_param_get(int dispFd, unsigned int dispLayer, void *dispLayerInfo);
int disp_layer_frm_buf_open(char *path);
int disp_layer_frm_buf_close(int fbFd);
int disp_layer_frm_buf_layer_get(int fbFd, DISP_LAYER_ID fbLayerId,
		unsigned int *fbLayer);
int disp_video_layer_param_set(unsigned int phy_addr[],
		enum VIDEO_PIXEL_FORMAT fmt, struct win_info *win);
int disp_layer_on(int dispFd, unsigned int dispLayer);
int disp_layer_off(int dispFd, unsigned int dispLayer);
int disp_ctrl_video_alpha(unsigned char alpha);
int disp_src_win_set(unsigned int src_x, unsigned int src_y, unsigned int src_w, unsigned int src_h);
int disp_scn_win_set(unsigned int scn_x, unsigned int scn_y, unsigned int scn_w, unsigned int scn_h);
int disp_scaler_init(void);
int disp_scaler_uninit(void);
int disp_lcd_on(int dispFd);
int disp_lcd_off(int dispFd);
int disp_video_chanel_open(void);
int disp_video_chanel_close(void);
int disp_open(char *path);
int disp_close(int dispFd);
int disp_open_backlight(void);
int disp_close_backlight(void);
int disp_init(void);
int disp_uninit(void);
int disp_screenwidth_get();
int disp_screenheight_get();
int disp_get_cur_fd();
#endif
