#ifndef __CSI_H__
#define __CSI_H__

/*********************************************************************************************************
*
*  Header File Include
**********************************************************************************************************/
#define MODULE_CSI_COMPILE_CFG 1
#if MODULE_CSI_COMPILE_CFG == 1
#include "sunxi_camera.h"
//#include <middleware/disp_mgr/drv_display_sun4i.h>

/*********************************************************************************************************
*
*  User Macro Define
**********************************************************************************************************/
#define CSI_DEV_NAME							  ("/dev/video0")

#define CSI_INPUT_FMT							  V4L2_PIX_FMT_NV21 //V4L2_PIX_FMT_YUV422P//V4L2_PIX_FMT_NV12//V4L2_PIX_FMT_YUYV
#define CSI_WIDTH_MAX							  640 //1920
#define CSI_HEIGHT_MAX							  480 //1080
#define CSI_INPUT_FRM_BUF_MAX					  5	//app handle time must less than  (MAX -2) * 33ms
#define CSI_INPUT_1080P_FRM_RATE				  30
#define CSI_INPUT_720P_FRM_RATE					  30
#define CSI_SELECT_FRM_TIMEOUT					  2
#define CSI_INPUT_FRM_TIMEOUT					  (60 * 1000)
#define CSI_MOTION_DETECT_PIXEL_THRESHOLD         10

#define CSI_DOWN_SMPL_PRECISION_LEVEL			  4

int usrcmd;
typedef int INT32S;
typedef unsigned int INT32U;
typedef short INT16S;
typedef unsigned short INT16U;
typedef char INT8S;
typedef unsigned char INT8U;
typedef long long INT64S;
typedef unsigned long long INT64U;

enum v4l2_api_colorfx {
	V4l2_API_COLORFX_NONE			= 0,
	V4l2_API_COLORFX_BW				= 1,
	V4l2_API_COLORFX_SEPIA			= 2,
	V4l2_API_COLORFX_NEGATIVE		= 3,
	V4l2_API_COLORFX_EMBOSS			= 4,
	V4l2_API_COLORFX_SKETCH			= 5,
	V4l2_API_COLORFX_SKY_BLUE		= 6,
	V4l2_API_COLORFX_GRASS_GREEN	= 7,
	V4l2_API_COLORFX_SKIN_WHITEN	= 8,
};

enum v4l2_api_resolution{
	V4l2_API_RESOLUTION_UXGA		=0,
	V4l2_API_RESOLUTION_720P		=1,
	V4l2_API_RESOLUTION_XGA			=2,
	V4l2_API_RESOLUTION_SVGA		=3,
	V4l2_API_RESOLUTION_VGA			=4,
	V4l2_API_RESOLUTION_QVGA		=5,
	V4l2_API_RESOLUTION_1080P		=6,
};

enum v4l2_api_auto_n_preset_white_balance {
	V4l2_API_WHITE_BALANCE_MANUAL			= 0,
	V4l2_API_WHITE_BALANCE_AUTO				= 1,
	V4l2_API_WHITE_BALANCE_INCANDESCENT		= 2,
	V4l2_API_WHITE_BALANCE_FLUORESCENT		= 3,
	V4l2_API_WHITE_BALANCE_TUNGSTEN			= 4,
	V4l2_API_WHITE_BALANCE_HORIZON			= 5,
	V4l2_API_WHITE_BALANCE_DAYLIGHT			= 6,
	V4l2_API_WHITE_BALANCE_FLASH			= 7,
	V4l2_API_WHITE_BALANCE_CLOUDY			= 8,
	V4l2_API_WHITE_BALANCE_SHADE			= 9,
};

enum v4l2_api_light_freq{
	V4l2_API_LIGHT_FREQ_AUTO		=0,
	V4l2_API_LIGHT_FREQ_50Hz		=1,
	V4l2_API_LIGHT_FREQ_60Hz		=2,
	V4l2_API_LIGHT_FREQ_MAX			=3,
};

typedef enum{
	CSI_INPUT_ID_OV5640,
	CSI_INPUT_ID_MAX
}CSI_INPUT_ID;

typedef enum{
	CSI_CTRL_ID_SET_RESOLUTION,
	CSI_CTRL_ID_SET_HUE,
	CSI_CTRL_ID_SET_LIGHT_FREQ,
	CSI_CTRL_ID_SET_AUTO_AWB,
	CSI_CTRL_ID_SET_AE,
	CSI_CTRL_ID_SET_FLIP,
	CSI_CTRL_ID_SET_BRIGHTNESS,
	CSI_CTRL_ID_SET_SATURATION,
	CSI_CTRL_ID_SET_CONTRAST,
	CSI_CTRL_ID_SET_AG,
	CSI_CTRL_ID_SET_ISO,
	CSI_CTRL_ID_SET_SHAPNESS,
	CSI_CTRL_ID_SET_AUTO_FOCUS,
	CSI_CTRL_ID_SET_FRM_RATE,
	CSI_CTRL_ID_MAX
}CSI_CTRL_ID;

/*********************************************************************************************************
*
*  Structure Define
**********************************************************************************************************/
typedef struct {
	INT32U m_addr;
	INT8U m_ref;
}V4l2_BUF;

typedef struct {
  V4l2_BUF m_v4l2Buf[CSI_INPUT_FRM_BUF_MAX];
  INT32U m_v4l2BufIdx;
  INT32U m_v4l2BufLen;
  INT32U m_buf_start;
}CSI_BUF;

typedef struct {
	INT32U m_csiSensorIdx;
	INT16U m_csiFrmRate;
	INT16U m_csiWidth;
	INT16U m_csiHeight;
	INT32U m_csiFmt;
}CSI_INFO;

/*********************************************************************************************************
*
*  Fuction Protype Declare
**********************************************************************************************************/
INT32S csi_ctrl_set(INT32S fd, CSI_CTRL_ID ctrlId, INT32S ctrlVal);
INT32S csi_input_set(INT32S fd, INT32U idx);
INT32S csi_steam_param_set(INT32S fd, INT16U frmRate);
INT32S csi_fmt_set(INT32S fd, INT16U width, INT16U height, INT32U fmt);
INT32S csi_querycap_get(INT32S fd);
INT32S csi_querystd_get(INT32S fd);
INT32S csi_down_smpl_buf_create(INT16U width, INT16U height);
INT32S csi_down_smpl_buf_destroy(void);
INT32S csi_down_sampling(INT8U *src, INT8U *dst, INT16U width, INT16U height);
INT8U csi_is_move(INT8U *img1, INT8U *img2, INT16U width, INT16U height);
INT32S csi_buf_enqueue(CSI_BUF *csiBufAddr);
INT32S csi_buf_dequeue(CSI_BUF *csiBufAddr, INT64U *curV4l2BufMsec, INT64U *diffV4l2BufMsec);
INT32S csi_buf_request(INT32S fd, INT8U frmNum);
INT32S csi_buf_release(void);
INT32S csi_stream_on(INT32S fd);
INT32S csi_stream_off(INT32S fd);
INT32S csi_default_param_set(void);
INT32S csi_system_param_set(void);
INT32S csi_open(INT8S *path);
INT32S csi_close(INT32S csiFd);
INT32S csi_get_fd();
INT32S csi_init(void);
INT32S csi_uninit(void);
#endif // #if MODULE_CSI_COMPILE_CFG == 1

#endif // __CSI_H__
