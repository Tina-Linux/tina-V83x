#ifndef __DISP_H__
#define __DISP_H__

/*********************************************************************************************************
*
*  Header File Include
**********************************************************************************************************/
//#include <common/platform_cfg.h>
#define MODULE_DISP_COMPILE_CFG 1
#if MODULE_DISP_COMPILE_CFG == 1
#include "drv_display_sun4i.h"

/*********************************************************************************************************
*
*  User Macro Define
**********************************************************************************************************/
#define DISP_DEV_NAME								("/dev/disp")
#define DISP_FB0_NAME								("/dev/fb0")

#define DISP_INPUT_WIDTH							640
#define DISP_INPUT_HEIGHT							480

#define DISP_INPUT_MODE								DISP_MOD_NON_MB_PLANAR
#define DISP_INPUT_FMT								DISP_FORMAT_YUV420
#define DISP_INPUT_SEQ								DISP_SEQ_UVUV

#define DISP_UI_MODE								DISP_MOD_INTERLEAVED
#define DISP_UI_FMT									DISP_FORMAT_RGB888
#define DISP_UI_SEQ									DISP_SEQ_ARGB
#define DISP_UI_ALPHA								0x7f
#define DISP_HV_SWITCH								MODULE_DISP_HV_SWITCH
#define GUI_FRM_BUF_WIDTH_PIXEL 320
#define GUI_FRM_BUF_HEIGHT_PIXEL 240
#define DISP_FRAME_RATE							20
typedef void																	VOID;
typedef unsigned char															INT8U;
typedef unsigned short															INT16U;
typedef unsigned int															INT32U;
typedef unsigned long long														INT64U;
typedef char																	INT8S;
typedef signed short															INT16S;
typedef signed int																INT32S;
typedef signed long long														INT64S;

typedef enum {
	DISP_YUV420_PLANNAR,		//use to show CSI video
	DISP_MB32_PLANNAR,			//use to show picture/video that decoded by cedarx
	DISP_ARGB8888_INTERLEAVED,	//used to play ARGB8888 format image or video
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

/*********************************************************************************************************
*
*  Structure Define
**********************************************************************************************************/
struct st_mb32_img_info {
	int outVideoWidth;
	int outVideoHeight;
	int outLineStride;
	int outTopOffset;
	int outLeftOffset;
	int outBottomOffset;
	int outRightOffset;
	int outRotateFlag;
};

struct win_info {
	int width;
	int height;
	struct st_mb32_img_info src_win;
	struct st_mb32_img_info scn_win;
};

/*********************************************************************************************************
*
*  Fuction Protype Declare
**********************************************************************************************************/
INT32S disp_layer_request(INT32S dispFd, INT32U *dispLayer, __disp_layer_work_mode_t workMode);
INT32S disp_layer_release(INT32S dispFd, INT32U dispLayer);
INT32S disp_layer_open(INT32S dispFd, INT32U dispLayer);
INT32S disp_layer_close(INT32S dispFd, INT32U dispLayer);
INT32S disp_layer_param_set(INT32S dispFd, INT32U dispLayer, __disp_layer_info_t *dispLayerInfo);
INT32S disp_layer_param_get(INT32S dispFd, INT32U dispLayer, __disp_layer_info_t *dispLayerInfo);
INT32S disp_layer_frm_buf_open(INT8S *path);
INT32S disp_layer_frm_buf_close(INT32S fbFd);
INT32S disp_layer_frm_buf_layer_get(INT32S fbFd, DISP_LAYER_ID fbLayerId, INT32U *fbLayer);
INT32S disp_layer_frm_buf_view_set(INT32S fbFd, INT32U fbLayer, DISP_VIEW_MODE mode);
INT32S disp_video_layer_param_set(INT32U phy_addr, DISP_INPUT_FORMAT fmt, struct win_info win, INT8U apha);
INT32S disp_video_layer_param_set_with_multiaddr(INT32U *phy_addr, DISP_INPUT_FORMAT fmt, struct win_info win, INT8U apha);
INT32S disp_layer_on(INT32S dispFd, INT32U dispLayer);
INT32S disp_layer_off(INT32S dispFd, INT32U dispLayer);
INT32S disp_normal_frm_buf_set(INT32U fbAddr);
INT32S disp_scaler_frm_buf_set(INT32U fbAddr, INT32U img_width, INT32U img_height, DISP_INPUT_FORMAT fbFmt);
INT32S disp_ctrl_video_alpha(INT8U alpha);
INT32S disp_rotate_set(INT32U angle);
INT32S disp_src_win_set(INT32U src_x, INT32U src_y, INT32U src_w, INT32U src_h);
INT32S disp_scn_win_set(INT32U scn_x, INT32U scn_y, INT32U scn_w, INT32U scn_h);
INT32S disp_scaler_init(void);
INT32S disp_scaler_uninit(void);
INT32S disp_lcd_on(INT32S dispFd);
INT32S disp_lcd_off(INT32S dispFd);
INT32S disp_video_chanel_open(void);
INT32S disp_video_chanel_close(void);
INT32S disp_open(INT8S *path);
INT32S disp_close(INT32S dispFd);
INT32S disp_open_backlight(void);
INT32S disp_close_backlight(void);
INT32S disp_init(void);
INT32S disp_uninit(void);
INT32S disp_screenwidth_get();
INT32S disp_screenheight_get();
INT32S disp_get_cur_fd();

#endif // #if MODULE_DISP_COMPILE_CFG == 1

#endif
