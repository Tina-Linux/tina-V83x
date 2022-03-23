#include "disp.h"

#if MODULE_DISP_COMPILE_CFG == 1
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>
#include <linux/fb.h>
#include <linux/videodev2.h>

/*********************************************************************************************************
*
*  Static Define
**********************************************************************************************************/
#define DISP_CLAR(x)			memset(&(x),  0, sizeof(x))

/*********************************************************************************************************
*
*  Static Variable
**********************************************************************************************************/
static INT8U DISP_LOG = 1;
#define DISP_DBG_LOG	 if(DISP_LOG)  printf



/*********************************************************************************************************
*
*  Extern And Global Variable
**********************************************************************************************************/
static INT32U disp_normal_layer = 100;
static INT32S disp_fd           = 0;
static INT32U disp_scaler_layer = 0;
static __disp_layer_info_t disp_scaler_layer_info;
static INT8U   disp_video_start_flag = 0;

#if 0
static INT32S disp_normal_fd = 0;
static INT32S disp_normal_fb_fd = 0;
static INT32U disp_normal_layer = 0;
static __disp_layer_info_t disp_normal_layer_info;
#endif
/*********************************************************************************************************
*
*  Function Define
**********************************************************************************************************/
INT32S disp_layer_request(INT32S dispFd, INT32U *dispLayer, __disp_layer_work_mode_t workMode)
{
	INT32U ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];
	INT32U tmpLayer = 0;

	if(workMode > DISP_LAYER_WORK_MODE_SCALER || dispLayer == NULL){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		goto errHdl;
	}

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = workMode;
	tmpLayer = ioctl(dispFd, DISP_CMD_LAYER_REQUEST,  (void*)ioctlParam);
	if(tmpLayer == 0){
		DISP_DBG_LOG("%s:request disp layer failed!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	*dispLayer = tmpLayer;
	DISP_DBG_LOG("request layer = %u success\r\n", *dispLayer);
errHdl:
	return ret;
}

INT32S disp_layer_release(INT32S dispFd, INT32U dispLayer)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = dispLayer;
	ret = ioctl(dispFd, DISP_CMD_LAYER_RELEASE,	(void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl release disp handle is not exist!\r\n", __func__);
		goto errHdl;
	}

	DISP_DBG_LOG("release layer = %u success\r\n", dispLayer);
errHdl:
	return ret;
}

INT32S disp_layer_open(INT32S dispFd, INT32U dispLayer)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = dispLayer;
	ret = ioctl(dispFd, DISP_CMD_LAYER_OPEN,  (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl close disp handle is not exist!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_layer_close(INT32S dispFd, INT32U dispLayer)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = dispLayer;
	ret = ioctl(dispFd, DISP_CMD_LAYER_CLOSE,  (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl close disp handle is not exist!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_layer_param_set(INT32S dispFd, INT32U dispLayer, __disp_layer_info_t *dispLayerInfo)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	if(dispLayerInfo == NULL){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
    ioctlParam[1] = dispLayer;
    ioctlParam[2] = (INT32U)dispLayerInfo;
    ioctl(dispFd, DISP_CMD_LAYER_SET_PARA, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl set disp layer param failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_layer_param_get(INT32S dispFd, INT32U dispLayer, __disp_layer_info_t *dispLayerInfo)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	if(dispLayerInfo == NULL){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
    ioctlParam[1] = dispLayer;
    ioctlParam[2] = (INT32U)dispLayerInfo;
    ioctl(dispFd, DISP_CMD_LAYER_GET_PARA, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl get disp layer param failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

static INT32S disp_layer_cmd_cache(INT32S dispFd)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ret = ioctl(dispFd, DISP_CMD_START_CMD_CACHE, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl set cmd cache failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

static INT32S disp_layer_cmd_submit(INT32S dispFd)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ret = ioctl(dispFd, DISP_CMD_EXECUTE_CMD_AND_STOP_CACHE, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl exec cmd and submit cache failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}




INT32S disp_layer_frm_buf_open(INT8S *path)
{
	INT32S fbFd = 0;

	if(path == NULL){ // /dev/fbxxx
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		fbFd = -1;
		goto errHdl;
	}

	fbFd = open(path, O_RDWR);
	if(fbFd == -1){
		DISP_DBG_LOG("%s:open disp layer frame buffer handle error!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return fbFd;
}

INT32S disp_layer_frm_buf_close(INT32S fbFd)
{
	INT32S ret = 0;

	ret = close(fbFd);
	if(ret != 0){
		DISP_DBG_LOG("%s:close disp layer frame buffer handle failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_layer_frm_buf_layer_get(INT32S fbFd, DISP_LAYER_ID fbLayerId, INT32U *fbLayer)
{
	INT32S ret = 0;
	INT32U tmpFbLayer;

	if(fbLayer == NULL){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		fbFd = -1;
		goto errHdl;
	}

	if(fbLayerId == 0){
		ret = ioctl(fbFd, FBIOGET_LAYER_HDL_0, (void*)&tmpFbLayer);
	}else if(fbLayerId == 1){
		ret = ioctl(fbFd, FBIOGET_LAYER_HDL_1, (void*)&tmpFbLayer);
	}

	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl get disp layer fb handle failed!\r\n", __func__);
		goto errHdl;
	}

	*fbLayer = tmpFbLayer;

	DISP_DBG_LOG("%s:fbLayer = %u\r\n", __func__, *fbLayer);
errHdl:
	return ret;
}

INT32S disp_layer_frm_buf_view_set(INT32S fbFd, INT32U fbLayer, DISP_VIEW_MODE mode)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];
	__disp_cmd_t viewMode = 0xff;

	if(mode >= DISP_VIEW_MODE_MAX){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	DISP_CLAR(ioctlParam);

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
    ioctlParam[1] = fbLayer;
	if(mode == DISP_VIEW_MODE_TOP){
	viewMode = DISP_CMD_LAYER_TOP;
	}else{
		viewMode = DISP_CMD_LAYER_BOTTOM;
	}

	ioctl(fbFd, viewMode, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl get disp layer param failed!\r\n", __func__);
		goto errHdl;
	}
errHdl:
	return ret;
}

INT32S disp_layer_on(INT32S dispFd, INT32U dispLayer)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = dispLayer;
    ret = ioctl(dispFd, DISP_CMD_VIDEO_START, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl disp disp on failed!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_layer_off(INT32S dispFd, INT32U dispLayer)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = dispLayer;
    ret = ioctl(dispFd, DISP_CMD_VIDEO_STOP, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl disp disp off failed!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_video_chanel_open(void)
{
	INT32S ret = -1;

	if (0 == disp_video_start_flag) {
		printf("%s: open video chanel\n", __func__);
		ret = disp_layer_open(disp_fd, disp_scaler_layer);
		if(ret != 0){
			DISP_DBG_LOG("%s:open disp layer failed!\r\n", __func__);
			ret = -1;
			goto errHdl;
		}

		ret = disp_layer_on(disp_fd, disp_scaler_layer);
		if(ret != 0){
			DISP_DBG_LOG("%s:disp layer disp on failed!\r\n", __func__);
			ret = -2;
			goto errHdl;
		}

		disp_video_start_flag = 1;
	} else {
		ret = 0;	//video chanel already open
	}

errHdl:
	return ret;
}

INT32S disp_video_chanel_close(void)
{
	INT32S ret = -1;

	if (1 == disp_video_start_flag) {
		ret = disp_layer_off(disp_fd, disp_scaler_layer);
		if(ret != 0){
			DISP_DBG_LOG("%s:disp layer disp off failed!\r\n", __func__);
			ret = -1;
			goto errHdl;
		}

		ret = disp_layer_close(disp_fd, disp_scaler_layer);
		if(ret != 0){
			DISP_DBG_LOG("%s:close disp layer failed!\r\n", __func__);
			ret = -2;
			goto errHdl;
		}

		disp_video_start_flag = 0;
	} else {
		ret = 0;	//video chanel already closed
	}

errHdl:
	return ret;
}

static __disp_pixel_fmt_t convert_pixel_format(DISP_INPUT_FORMAT fbFmt)
{
	__disp_pixel_fmt_t fmt = 0;
	switch (fbFmt) {
	case DISP_YUV420_PLANNAR:
		fmt = DISP_FORMAT_YUV420;
		break;
	case DISP_MB32_PLANNAR:
		fmt = DISP_FORMAT_YUV420;
		break;
	case DISP_ARGB8888_INTERLEAVED:
		fmt = DISP_FORMAT_RGB888;
		break;
	default:
		fmt = DISP_UNKOWN_FORMAT;
		printf("%s: unkown input format, fbFmt=%d\n", __func__, fbFmt);
		break;
	}

	return fmt;
}
static __disp_pixel_seq_t convert_pixel_seq(DISP_INPUT_FORMAT fbFmt)
{
	__disp_pixel_seq_t seq = 0;
	switch (fbFmt) {
	case DISP_YUV420_PLANNAR:
		seq = DISP_SEQ_UVUV;
		break;
	case DISP_MB32_PLANNAR:
		seq = DISP_SEQ_UVUV;
		break;
	case DISP_ARGB8888_INTERLEAVED:
		seq = DISP_SEQ_ARGB;
		break;
	default:
		seq = DISP_UNKOWN_SEQ;
		printf("%s: unkown input format, fbFmt=%d\n", __func__, fbFmt);
		break;
	}

	return seq;
}

static __disp_pixel_mod_t convert_pixel_mod(DISP_INPUT_FORMAT fbFmt)
{
	__disp_pixel_mod_t mod = 0;
	switch (fbFmt) {
	case DISP_YUV420_PLANNAR:
		mod = DISP_MOD_NON_MB_PLANAR;
		break;
	case DISP_MB32_PLANNAR:
		mod = DISP_MOD_MB_UV_COMBINED;
		break;
	case DISP_ARGB8888_INTERLEAVED:
		mod = DISP_MOD_INTERLEAVED;
		break;
	default:
		mod = DISP_UNKOWN_MOD;
		printf("%s: unkown input format, fbFmt=%d\n", __func__, fbFmt);
		break;
	}

	return mod;
}

INT32S disp_update_layer_param(INT32U img_width, INT32U img_height, DISP_INPUT_FORMAT fbFmt)
{
	__disp_layer_info_t dispLayerInfo;
	__disp_pixel_fmt_t      format;
	__disp_pixel_seq_t      seq;
	__disp_pixel_mod_t      mode;
	INT8U change_flag = 0;
	INT32S ret = -1;

	DISP_CLAR(dispLayerInfo);

	//get param of layer
	ret = disp_layer_param_get(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if (0 != ret) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_scaler_layer);
		return -1;
	}

	if ((img_width != dispLayerInfo.fb.size.width) || (img_width != dispLayerInfo.src_win.width)) {
		dispLayerInfo.fb.size.width = img_width;
		dispLayerInfo.src_win.width = img_width;
		change_flag = 1;
	}

	if ((img_height != dispLayerInfo.fb.size.height) || (img_height != dispLayerInfo.src_win.height)) {
		dispLayerInfo.fb.size.height = img_height;
		dispLayerInfo.src_win.height = img_height;
		change_flag = 1;
	}

	format = convert_pixel_format(fbFmt);
	if ((DISP_UNKOWN_FORMAT != format) && (dispLayerInfo.fb.format != format)) {
		dispLayerInfo.fb.format = format;
		change_flag = 1;
	}

	seq = convert_pixel_seq(fbFmt);
	if ((DISP_UNKOWN_SEQ != seq) && (dispLayerInfo.fb.seq != seq)) {
		dispLayerInfo.fb.seq = seq;
		change_flag = 1;
	}

	mode = convert_pixel_mod(fbFmt);
	if ((DISP_UNKOWN_MOD != mode) && (dispLayerInfo.fb.mode != mode)) {
		dispLayerInfo.fb.mode = mode;
		change_flag = 1;
	}

	//no need to update param of layer
	if (0 == change_flag) {
		return 0;
	}

	//set param to layer
	ret = disp_layer_param_set(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if(ret != 0){
		DISP_DBG_LOG("%s:fail to update param of layer(%u), ret=%d!\r\n", __func__, disp_scaler_layer, ret);
		return -2;
	}
	DISP_DBG_LOG("%s: update param of layer(%u) ok\n", __func__, disp_scaler_layer);

	return 0;
}

INT32S disp_normal_frm_buf_set(INT32U fbAddr)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];
	__disp_video_fb_t	dispFbAddr;
	__disp_layer_info_t dispLayerInfo;

	if(fbAddr == 0){
		DISP_DBG_LOG("%s:param error!gui layer addr=0x%x\r\n", __func__, fbAddr);
		ret = -1;
		goto errHdl;
	}

	DISP_CLAR(ioctlParam);
	DISP_CLAR(dispFbAddr);
	DISP_CLAR(dispLayerInfo);

	ret = disp_layer_param_get(disp_fd, disp_normal_layer, &dispLayerInfo);
	if(ret != 0){
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_normal_layer);
		return -2;
	}

	dispLayerInfo.fb.addr[0] =	fbAddr;
	ret = disp_layer_param_set(disp_fd, disp_normal_layer, &dispLayerInfo);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl disp disp off failed!\r\n", __func__);
		ret = -3;
		goto errHdl;
	}

errHdl:
	return ret;
}


INT32S disp_scaler_frm_buf_set(INT32U fbAddr, INT32U img_width, INT32U img_height, DISP_INPUT_FORMAT fbFmt)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];
	 __disp_video_fb_t  dispFbAddr;
	 __disp_pixel_fmt_t format = 0;
	 INT32U h32aglin = 0;
	 INT32U w32aglin = 0;
	 INT32U h64aglin = 0;

	DISP_DBG_LOG("%s: set addr=0x%x, w=%u, h=%u, fmt=%d\n", __func__, fbAddr, img_width, img_height, fbFmt);
	if(fbAddr == 0){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	if (0 == disp_video_start_flag) {
		DISP_DBG_LOG("%s: video is stop, cannot show video now\n", __func__);
		return -2;
	}

	DISP_CLAR(ioctlParam);
	DISP_CLAR(dispFbAddr);

	(void)disp_update_layer_param(img_width, img_height, fbFmt);

	// set disp frame buffer param
	dispFbAddr.id = 0;
	dispFbAddr.interlace       = 0;
	dispFbAddr.top_field_first = 0;
	dispFbAddr.frame_rate      = 25;

	dispFbAddr.addr[0] = fbAddr;

	format = convert_pixel_format(fbFmt);
	switch(format){
	case DISP_FORMAT_YUV422:
		dispFbAddr.addr[1]       = fbAddr + img_width * img_height;
		dispFbAddr.addr[2]       = fbAddr + img_width * img_height * 3 / 2;
		break;
	case DISP_FORMAT_YUV420:
		if (DISP_MB32_PLANNAR == fbFmt) {
			h32aglin = ((img_height + 31) & ~31);
			w32aglin = ((img_width + 31) & ~31);
			h64aglin = ((img_height + 63) & ~63);

			dispFbAddr.addr[1]       = fbAddr + w32aglin * h32aglin;
			dispFbAddr.addr[2]       = fbAddr + w32aglin * h32aglin + img_width * h64aglin /2;
		} else {
			dispFbAddr.addr[1]       = fbAddr + img_width * img_height;
			dispFbAddr.addr[2]       = fbAddr + img_width * img_height * 5 / 4;
		}
		break;
	case DISP_FORMAT_RGB888:
		dispFbAddr.addr[1] = 0;
		dispFbAddr.addr[2] = 0;
		break;
	default:
		break;
	}

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = disp_scaler_layer;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER_PARAM] = (INT32U)&dispFbAddr;
	ret = ioctl(disp_fd, DISP_CMD_VIDEO_SET_FB, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl disp disp off failed!\r\n", __func__);
		ret = -3;
		goto errHdl;
	}

errHdl:
	return ret;
}

static INT32S disp_video_addr_set(INT32U fbAddr, INT32U img_width, INT32U img_height, DISP_INPUT_FORMAT fbFmt)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];
	 __disp_video_fb_t  dispFbAddr;
	 __disp_pixel_fmt_t format = 0;
	 INT32U h32aglin = 0;
	 INT32U w32aglin = 0;
	 INT32U h64aglin = 0;

	//DISP_DBG_LOG("%s: set addr=0x%x, w=%d, h=%d, fmt=%d\n", __func__, fbAddr, img_width, img_height, fbFmt);
	if(fbAddr == 0){
		printf("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	if (0 == disp_video_start_flag) {
		printf("%s: video is stop, cannot show video now\n", __func__);
		return -2;
	}

	DISP_CLAR(ioctlParam);
	DISP_CLAR(dispFbAddr);

	// set disp frame buffer param
	dispFbAddr.id = 0;
	dispFbAddr.interlace       = 0;
	dispFbAddr.top_field_first = 0;
	dispFbAddr.frame_rate      = 25;

	dispFbAddr.addr[0] = fbAddr;

	format = convert_pixel_format(fbFmt);
	switch(format){
	case DISP_FORMAT_YUV422:
		dispFbAddr.addr[1]       = fbAddr + img_width * img_height;
		dispFbAddr.addr[2]       = fbAddr + img_width * img_height * 3 / 2;
		break;
	case DISP_FORMAT_YUV420:
		if (DISP_MB32_PLANNAR == fbFmt) {
			h32aglin = ((img_height + 31) & ~31);
			w32aglin = ((img_width + 31) & ~31);
			h64aglin = ((img_height + 63) & ~63);

			dispFbAddr.addr[1]       = fbAddr + w32aglin * h32aglin;
			dispFbAddr.addr[2]       = fbAddr + w32aglin * h32aglin + img_width * h64aglin /2;
		} else {
			dispFbAddr.addr[1]       = fbAddr + img_width * img_height;
			dispFbAddr.addr[2]       = fbAddr + img_width * img_height * 5 / 4;
		}
		break;
	case DISP_FORMAT_RGB888:
		dispFbAddr.addr[1] = 0;
		dispFbAddr.addr[2] = 0;
		break;
	default:
		break;
	}

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = disp_scaler_layer;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER_PARAM] = (INT32U)&dispFbAddr;
	ret = ioctl(disp_fd, DISP_CMD_VIDEO_SET_FB, (void*)ioctlParam);
	if(ret != 0){
		printf("%s:ioctl disp disp off failed!\r\n", __func__);
		ret = -3;
		goto errHdl;
	}

errHdl:
	return ret;
}


INT32S disp_ctrl_video_alpha(INT8U alpha)
{
	__disp_layer_info_t dispLayerInfo;
	INT32S ret = -1;
	INT8U alphaVal = 0;

	alphaVal = alpha;
	DISP_CLAR(dispLayerInfo);
	ret = disp_layer_param_get(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if (0 != ret) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_scaler_layer);
		return -1;
	}

	dispLayerInfo.alpha_en = 1;
	if (alphaVal != dispLayerInfo.alpha_val) {
		dispLayerInfo.alpha_val = alphaVal;
	}

	ret = disp_layer_param_set(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if(ret != 0){
		DISP_DBG_LOG("%s:set disp layer param failed, ret=%d!\r\n", __func__, ret);
		return -2;
	}

	return 0;
}

static INT32S disp_video_addr_set_multi(INT32U *fbAddr, INT32U img_width, INT32U img_height, DISP_INPUT_FORMAT fbFmt)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];
	 __disp_video_fb_t  dispFbAddr;
	 __disp_pixel_fmt_t format = 0;
	 INT32U h32aglin = 0;
	 INT32U w32aglin = 0;
	 INT32U h64aglin = 0;

	//DISP_DBG_LOG("%s: set addr=0x%x, w=%d, h=%d, fmt=%d\n", __func__, (unsigned int)fbAddr, img_width, img_height, fbFmt);
	if(fbAddr == NULL){
		printf("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	if (0 == disp_video_start_flag) {
		printf("%s: video is stop, cannot show video now\n", __func__);
		return -2;
	}

	DISP_CLAR(ioctlParam);
	DISP_CLAR(dispFbAddr);

	// set disp frame buffer param
	dispFbAddr.id = 0;
	dispFbAddr.interlace       = 0;
	dispFbAddr.top_field_first = 0;
	dispFbAddr.frame_rate      = 25;

	dispFbAddr.addr[0] = fbAddr[0];
	dispFbAddr.addr[1] = fbAddr[1];
	dispFbAddr.addr[2] = fbAddr[2];
	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = disp_scaler_layer;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER_PARAM] = (INT32U)&dispFbAddr;
	ret = ioctl(disp_fd, DISP_CMD_VIDEO_SET_FB, (void*)ioctlParam);
	if(ret != 0){
		printf("%s:ioctl disp disp off failed!\r\n", __func__);
		ret = -3;
		goto errHdl;
	}

errHdl:
	return ret;
}

static void print_display_layer(__disp_layer_info_t *info)
{
	printf("m:%d brs:%d pipe:%d alen:%d alval:%d srcx:%d srcy:%d srcw:%d srch:%d scnx:%d scny:%d scnw:%d scnh:%d a0:%x a1:%x a2:%x fbformat:%d fbw:%d fbh:%d\n",
		info->mode,info->b_from_screen,info->pipe,info->alpha_en,info->alpha_val,info->src_win.x,info->src_win.y,info->src_win.width,info->src_win.height,
		info->scn_win.x,info->scn_win.y,info->scn_win.width,info->scn_win.height,info->fb.addr[0],info->fb.addr[1],info->fb.addr[2],
		info->fb.format,info->fb.size.width,info->fb.size.height);

}

/*
update all param at once, to get more faster
*/
INT32S disp_video_layer_param_set_with_multiaddr(INT32U *phy_addr, DISP_INPUT_FORMAT fmt, struct win_info win, INT8U apha)
{
	__disp_layer_info_t dispLayerInfo;
	struct st_mb32_img_info *src_win;
	struct st_mb32_img_info *scn_win;
	__disp_pixel_fmt_t      format;
	__disp_pixel_seq_t      seq;
	__disp_pixel_mod_t      mode;
	INT8U update_flag = 0;
	INT32S ret = -1;

	if (NULL == phy_addr) {
		printf("%s: invalid phy address\n", __func__);
		return -1;
	}

	disp_layer_cmd_cache(disp_fd);	//protech param update

	DISP_CLAR(dispLayerInfo);
	ret = disp_layer_param_get(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if (0 != ret) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_scaler_layer);
		ret = -1;
		goto errHdl;
	}

	src_win = &(win.src_win);
	scn_win = &(win.scn_win);

	//check src window update
	if((dispLayerInfo.src_win.x == src_win->outLeftOffset)
	   &&(dispLayerInfo.src_win.y == src_win->outTopOffset)
	   &&(dispLayerInfo.src_win.width == src_win->outRightOffset)
	   &&(dispLayerInfo.src_win.height == src_win->outBottomOffset)){
		//DISP_DBG_LOG("%s: src win no need to update!\r\n", __func__);
	} else {
		dispLayerInfo.src_win.x = src_win->outLeftOffset;
		dispLayerInfo.src_win.y = src_win->outTopOffset;
		dispLayerInfo.src_win.width = src_win->outRightOffset;
		dispLayerInfo.src_win.height = src_win->outBottomOffset;
		update_flag = 1;
	}

	//check screen window update
	if((dispLayerInfo.scn_win.x == scn_win->outLeftOffset) &&
	   (dispLayerInfo.scn_win.y == scn_win->outTopOffset) &&
	   (dispLayerInfo.scn_win.width == scn_win->outRightOffset) &&
	   (dispLayerInfo.scn_win.height == scn_win->outBottomOffset)){
		//DISP_DBG_LOG("%s: scn win no need to update!\r\n", __func__);
	} else {
		dispLayerInfo.scn_win.x = scn_win->outLeftOffset;
		dispLayerInfo.scn_win.y = scn_win->outTopOffset;
		dispLayerInfo.scn_win.width = scn_win->outRightOffset;
		dispLayerInfo.scn_win.height = scn_win->outBottomOffset;
		update_flag = 1;
	}

	//check apha update
	dispLayerInfo.alpha_en = 1;
	if (apha == dispLayerInfo.alpha_val) {
		//DISP_DBG_LOG("%s: apha value no need to update!\r\n", __func__);
	}  else {
		dispLayerInfo.alpha_val = apha;
		update_flag = 1;
	}

	if (win.width != dispLayerInfo.fb.size.width) {
		dispLayerInfo.fb.size.width = win.width;
		update_flag = 1;
	}

	if ((win.height != dispLayerInfo.fb.size.height) ) {
		dispLayerInfo.fb.size.height = win.height;
		update_flag = 1;
	}

	format = convert_pixel_format(fmt);
	if ((DISP_UNKOWN_FORMAT != format) && (dispLayerInfo.fb.format != format)) {
		dispLayerInfo.fb.format = format;
		update_flag = 1;
	}

	seq = convert_pixel_seq(fmt);
	if ((DISP_UNKOWN_SEQ != seq) && (dispLayerInfo.fb.seq != seq)) {
		dispLayerInfo.fb.seq = seq;
		update_flag = 1;
	}

	mode = convert_pixel_mod(fmt);
	if ((DISP_UNKOWN_MOD != mode) && (dispLayerInfo.fb.mode != mode)) {
		dispLayerInfo.fb.mode = mode;
		update_flag = 1;
	}

	//dispLayerInfo.scn_win.width = 320;
	//dispLayerInfo.scn_win.height = 240;

	print_display_layer(&dispLayerInfo);
	if (update_flag) {
		ret = disp_layer_param_set(disp_fd, disp_scaler_layer, &dispLayerInfo);
		if (0 != ret) {
			DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_scaler_layer);
		}
	} else {
		ret = 0;
	}

	//sleep(1);

	//disp_video_addr_set_multi(phy_addr, win.width, win.height, fmt);
	disp_video_addr_set(phy_addr[0],720,480,fmt);
	//return ret;

errHdl:
	disp_layer_cmd_submit(disp_fd);
	return ret;
}


/*
update all param at once, to get more faster
*/
INT32S disp_video_layer_param_set(INT32U phy_addr, DISP_INPUT_FORMAT fmt, struct win_info win, INT8U apha)
{
	__disp_layer_info_t dispLayerInfo;
	struct st_mb32_img_info *src_win;
	struct st_mb32_img_info *scn_win;
	__disp_pixel_fmt_t      format;
	__disp_pixel_seq_t      seq;
	__disp_pixel_mod_t      mode;
	INT8U update_flag = 0;
	INT32S ret = -1;

	if (0 == phy_addr) {
		printf("%s: invalid phy address\n", __func__);
		return -1;
	}

	disp_layer_cmd_cache(disp_fd);	//protech param update

	DISP_CLAR(dispLayerInfo);
	ret = disp_layer_param_get(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if (0 != ret) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_scaler_layer);
		ret = -1;
		goto errHdl;
	}

	src_win = &(win.src_win);
	scn_win = &(win.scn_win);

	//check src window update
	if((dispLayerInfo.src_win.x == src_win->outLeftOffset)
	   &&(dispLayerInfo.src_win.y == src_win->outTopOffset)
	   &&(dispLayerInfo.src_win.width == src_win->outRightOffset)
	   &&(dispLayerInfo.src_win.height == src_win->outBottomOffset)){
		//DISP_DBG_LOG("%s: src win no need to update!\r\n", __func__);
	} else {
		dispLayerInfo.src_win.x = src_win->outLeftOffset;
		dispLayerInfo.src_win.y = src_win->outTopOffset;
		dispLayerInfo.src_win.width = src_win->outRightOffset;
		dispLayerInfo.src_win.height = src_win->outBottomOffset;
		update_flag = 1;
	}

	//check screen window update
	if((dispLayerInfo.scn_win.x == scn_win->outLeftOffset) &&
	   (dispLayerInfo.scn_win.y == scn_win->outTopOffset) &&
	   (dispLayerInfo.scn_win.width == scn_win->outRightOffset) &&
	   (dispLayerInfo.scn_win.height == scn_win->outBottomOffset)){
		//DISP_DBG_LOG("%s: scn win no need to update!\r\n", __func__);
	} else {
		dispLayerInfo.scn_win.x = scn_win->outLeftOffset;
		dispLayerInfo.scn_win.y = scn_win->outTopOffset;
		dispLayerInfo.scn_win.width = scn_win->outRightOffset;
		dispLayerInfo.scn_win.height = scn_win->outBottomOffset;
		update_flag = 1;
	}

	//check apha update
	dispLayerInfo.alpha_en = 1;
	if (apha == dispLayerInfo.alpha_val) {
		//DISP_DBG_LOG("%s: apha value no need to update!\r\n", __func__);
	}  else {
		dispLayerInfo.alpha_val = apha;
		update_flag = 1;
	}

	if (win.width != dispLayerInfo.fb.size.width) {
		dispLayerInfo.fb.size.width = win.width;
		update_flag = 1;
	}

	if ((win.height != dispLayerInfo.fb.size.height) ) {
		dispLayerInfo.fb.size.height = win.height;
		update_flag = 1;
	}

	format = convert_pixel_format(fmt);
	if ((DISP_UNKOWN_FORMAT != format) && (dispLayerInfo.fb.format != format)) {
		dispLayerInfo.fb.format = format;
		update_flag = 1;
	}

	seq = convert_pixel_seq(fmt);
	if ((DISP_UNKOWN_SEQ != seq) && (dispLayerInfo.fb.seq != seq)) {
		dispLayerInfo.fb.seq = seq;
		update_flag = 1;
	}

	mode = convert_pixel_mod(fmt);
	if ((DISP_UNKOWN_MOD != mode) && (dispLayerInfo.fb.mode != mode)) {
		dispLayerInfo.fb.mode = mode;
		update_flag = 1;
	}

	if (update_flag) {
		ret = disp_layer_param_set(disp_fd, disp_scaler_layer, &dispLayerInfo);
		if (0 != ret) {
			DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_scaler_layer);
		}
	} else {
		ret = 0;
	}

	//sleep(1);

	disp_video_addr_set(phy_addr, win.width, win.height, fmt);

	//return ret;

errHdl:
	disp_layer_cmd_submit(disp_fd);
	return ret;
}


INT32S disp_rotate_set(INT32U angle)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	if ((0 != angle) && (180 != angle)) {
		printf("%s: param error, angle=%u\n", __func__, angle);
		return -1;
	}

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = angle;
	ret = ioctl(disp_fd, DISP_CMD_SET_ROTATE, (void*)ioctlParam);
	if(ret != 0){
		printf("%s: set rotate failed, angle=%u!\r\n", __func__, angle);
		ret = -2;
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_src_win_set(INT32U src_x, INT32U src_y, INT32U src_w, INT32U src_h)
{
	__disp_layer_info_t dispLayerInfo;
	INT32S ret = -1;

	DISP_CLAR(dispLayerInfo);
	ret = disp_layer_param_get(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if(0 != ret) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_scaler_layer);
		return -2;
	}

	if((dispLayerInfo.scn_win.x == src_x) &&
	   (dispLayerInfo.scn_win.y == src_y) &&
	   (dispLayerInfo.scn_win.width == src_w) &&
	   (dispLayerInfo.scn_win.height == src_h)){
		DISP_DBG_LOG("%s: src win no need to update!\r\n", __func__);
		return 0;
	}

	dispLayerInfo.src_win.x = src_x;
	dispLayerInfo.src_win.y = src_y;
	dispLayerInfo.src_win.width = src_w;
	dispLayerInfo.src_win.height = src_h;

	DISP_DBG_LOG("%s: set source window: x=%u, y=%u, w=%u, h=%u\n", __func__, src_x, src_y, src_y, src_h);

	ret = disp_layer_param_set(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if(ret != 0){
		DISP_DBG_LOG("%s:set disp layer param failed, ret=%d!\r\n", __func__, ret);
		return -3;
	}

	return 0;
}

/*
display buffer regard as a horizon screen, if you use MCU panel , you need to rotate 90 degree and reset the video
window like below:
case 0:
case 180:
	scn_x = 0;
	scn_y = GUI_TOP_STATUS_BAR_HEIGHT_PIXEL;
	scn_w = GUI_FRM_BUF_WIDTH_PIXEL;
	scn_h = GUI_FRM_BUF_HEIGHT_PIXEL - GUI_TOP_STATUS_BAR_HEIGHT_PIXEL - GUI_BOTTOM_STATUS_BAR_HEIGHT_PIXEL;

case 90:
case 270:
	scn_x = GUI_TOP_STATUS_BAR_HEIGHT_PIXEL;
	scn_y = 0;
	scn_w = GUI_FRM_BUF_HEIGHT_PIXEL - GUI_TOP_STATUS_BAR_HEIGHT_PIXEL - GUI_BOTTOM_STATUS_BAR_HEIGHT_PIXEL;
	scn_h = GUI_FRM_BUF_WIDTH_PIXEL;
*/
INT32S disp_scn_win_set(INT32U scn_x, INT32U scn_y, INT32U scn_w, INT32U scn_h)
{
	__disp_layer_info_t dispLayerInfo;
	INT32S ret = -1;

	DISP_CLAR(dispLayerInfo);
	ret = disp_layer_param_get(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if(0 != ret) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_scaler_layer);
		return -2;
	}

	if((dispLayerInfo.scn_win.x == scn_x) &&
	   (dispLayerInfo.scn_win.y == scn_y) &&
	   (dispLayerInfo.scn_win.width == scn_w) &&
	   (dispLayerInfo.scn_win.height == scn_h)){
		DISP_DBG_LOG("%s: scn win no need to update!\r\n", __func__);
		return 0;
	}

	dispLayerInfo.scn_win.x = scn_x;
	dispLayerInfo.scn_win.y = scn_y;
	dispLayerInfo.scn_win.width = scn_w;
	dispLayerInfo.scn_win.height = scn_h;

	DISP_DBG_LOG("%s: set screen window: x=%u, y=%u, w=%u, h=%u\n", __func__, scn_x, scn_y, scn_y, scn_h);

	ret = disp_layer_param_set(disp_fd, disp_scaler_layer, &dispLayerInfo);
	if(ret != 0){
		DISP_DBG_LOG("%s:set disp layer param failed, ret=%d!\r\n", __func__, ret);
		return -3;
	}

	return 0;
}


INT32S disp_scaler_init(void)
{
	INT32S ret = 0;

	disp_fd = disp_open(DISP_DEV_NAME);
	if(disp_fd == -1){
		DISP_DBG_LOG("%s:open disp handle is not exist!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	ret = disp_layer_request(disp_fd, &disp_scaler_layer, DISP_LAYER_WORK_MODE_SCALER);
	if(ret != 0){
		DISP_DBG_LOG("%s:request layer failed!\r\n", __func__);
		ret = -3;
		goto errHdl;
	}

	DISP_DBG_LOG("%s:disp_fd = %d, disp_scaler_layer = %u\r\n", __func__, disp_fd, disp_scaler_layer);

	disp_scaler_layer_info.mode			= DISP_LAYER_WORK_MODE_SCALER;
	disp_scaler_layer_info.pipe			= 1;
	disp_scaler_layer_info.fb.addr[0]       = 0;
	disp_scaler_layer_info.fb.addr[1]       = 0;
	disp_scaler_layer_info.fb.addr[2]       = 0;
	disp_scaler_layer_info.fb.size.width    = DISP_INPUT_WIDTH;
	disp_scaler_layer_info.fb.size.height   = DISP_INPUT_HEIGHT;
	disp_scaler_layer_info.fb.mode          = DISP_INPUT_MODE;
	disp_scaler_layer_info.fb.format        = DISP_INPUT_FMT;
	disp_scaler_layer_info.fb.br_swap       = 0;
	disp_scaler_layer_info.fb.seq           = DISP_INPUT_SEQ;
	disp_scaler_layer_info.ck_enable        = 0;
	disp_scaler_layer_info.alpha_en         = 1;
	disp_scaler_layer_info.alpha_val        = DISP_UI_ALPHA;
	disp_scaler_layer_info.src_win.x        = 0;
	disp_scaler_layer_info.src_win.y        = 0;
	disp_scaler_layer_info.src_win.width    = DISP_INPUT_WIDTH;
	disp_scaler_layer_info.src_win.height   = DISP_INPUT_HEIGHT;

	disp_scaler_layer_info.scn_win.x        = 0;
	disp_scaler_layer_info.scn_win.y        = 0;
	disp_scaler_layer_info.scn_win.width    = GUI_FRM_BUF_WIDTH_PIXEL;
	disp_scaler_layer_info.scn_win.height   = GUI_FRM_BUF_HEIGHT_PIXEL;

	ret = disp_layer_param_set(disp_fd, disp_scaler_layer, &disp_scaler_layer_info);
	if(ret != 0){
		DISP_DBG_LOG("%s:set disp layer param failed!\r\n", __func__);
		ret = -4;
		goto errHdl;
	}

	DISP_DBG_LOG("%s:disp on ok\r\n", __func__);
errHdl:
	return ret;
}

INT32S disp_scaler_uninit(void)
{
	INT32S ret = 0;

	 /* close back light of panel before un-init disp module */
	 disp_close_backlight();

	ret = disp_layer_off(disp_fd, disp_scaler_layer);
	if(ret != 0){
		DISP_DBG_LOG("%s:disp layer off failed,layer not open!\r\n", __func__);
	}

	ret = disp_layer_close(disp_fd, disp_scaler_layer);
	if(ret != 0){
		DISP_DBG_LOG("%s:close disp layer failed!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	ret = disp_layer_release(disp_fd, disp_scaler_layer);
	if(ret != 0){
		DISP_DBG_LOG("%s:release disp layer failed!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}


	ret = disp_lcd_off(disp_fd);
	if(ret != 0){
		DISP_DBG_LOG("%s:disp lcd off failed!\r\n", __func__);
		ret = -4;
		goto errHdl;
	}

	ret = disp_close(disp_fd);
	if(ret != 0){
		DISP_DBG_LOG("%s:close disp failed!\r\n", __func__);
		ret = -5;
		goto errHdl;
	}

	DISP_DBG_LOG("%s:disp on ok\r\n", __func__);
errHdl:
	return ret;
}

#if 0
INT32S disp_normal_init(void)
{
	INT32S ret = 0;
	INT32U fbLayer;

	disp_normal_fd = disp_open(DISP_DEV_NAME);
	if(disp_normal_fd == -1){
		DISP_DBG_LOG("%s:open disp handle is not exist!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	ret = disp_lcd_on(disp_normal_fd);
	if(ret != 0){
		DISP_DBG_LOG("%s:disp lcd on failed!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}

	ret = disp_layer_request(disp_normal_fd, &disp_normal_layer, DISP_LAYER_WORK_MODE_NORMAL);
	if(ret != 0){
		DISP_DBG_LOG("%s:request layer failed!\r\n", __func__);
		ret = -3;
		goto errHdl;
	}

	DISP_DBG_LOG("%s:disp_normal_fd = %d, disp_normal_layer = %d\r\n", __func__, disp_normal_fd, disp_normal_layer);

	disp_normal_layer_info.mode			= DISP_LAYER_WORK_MODE_NORMAL;
	disp_normal_layer_info.pipe			= 1;
	disp_normal_layer_info.fb.addr[0]       = 0;
	disp_normal_layer_info.fb.addr[1]       = 0;
	disp_normal_layer_info.fb.addr[2]       = 0;
	disp_normal_layer_info.fb.size.width    = GUI_RES_PIXEL_WIDTH;
	disp_normal_layer_info.fb.size.height   = GUI_RES_PIXEL_HEIGHT;
	disp_normal_layer_info.fb.mode          = DISP_UI_MODE;
	disp_normal_layer_info.fb.format        = DISP_UI_FMT;
	disp_normal_layer_info.fb.br_swap       = 0;
	disp_normal_layer_info.fb.seq           = DISP_UI_SEQ;
	disp_normal_layer_info.ck_enable        = 0;
	disp_normal_layer_info.alpha_en         = 1;
	disp_normal_layer_info.alpha_val        = 0x7f;
	disp_normal_layer_info.src_win.x        = 0;
	disp_normal_layer_info.src_win.y        = 0;
	disp_normal_layer_info.src_win.width    = GUI_RES_PIXEL_WIDTH;
	disp_normal_layer_info.src_win.height   = GUI_RES_PIXEL_HEIGHT;
	disp_normal_layer_info.scn_win.x        = 0;
	disp_normal_layer_info.scn_win.y        = 0;
	disp_normal_layer_info.scn_win.width    = GUI_RES_PIXEL_WIDTH;
	disp_normal_layer_info.scn_win.height   = GUI_RES_PIXEL_HEIGHT;

	ret = disp_layer_param_set(disp_normal_fd, disp_normal_layer, &disp_scaler_layer_info);
	if(ret != 0){
		DISP_DBG_LOG("%s:set disp layer param failed!\r\n", __func__);
		ret = -4;
		goto errHdl;
	}

	ret = disp_layer_open(disp_normal_fd, disp_normal_layer);
	if(ret != 0){
		DISP_DBG_LOG("%s:open disp layer failed!\r\n", __func__);
		ret = -5;
		goto errHdl;
	}

	disp_scaler_fb_fd = disp_layer_frm_buf_open(GUI_MGR_FB_DEV_NAME);
	if(disp_scaler_fb_fd == -1){
		DISP_DBG_LOG("%s:open disp frame buffer handle is not exist!\r\n", __func__);
		ret = -6;
		goto errHdl;
	}

	ret = disp_layer_frm_buf_layer_get(disp_scaler_fb_fd, DISP_LAYER_ID_1, &fbLayer);
	if(ret != 0){
		DISP_DBG_LOG("%s:set disp layer view mode failed!\r\n", __func__);
		ret = -7;
		goto errHdl;
	}

	ret = disp_layer_frm_buf_view_set(disp_scaler_fb_fd, fbLayer, DISP_VIEW_MODE_TOP);
	if(ret != 0){
		DISP_DBG_LOG("%s:set disp layer view mode failed!\r\n", __func__);
		ret = -8;
		goto errHdl;
	}

	ret = disp_layer_on(disp_normal_fd, disp_normal_layer);
	if(ret != 0){
		DISP_DBG_LOG("%s:disp layer disp on failed!\r\n", __func__);
		ret = -7;
		goto errHdl;
	}

	ret = disp_lcd_on(disp_normal_fd);
	if(ret != 0){
		DISP_DBG_LOG("%s:disp lcd on failed!\r\n", __func__);
		ret = -8;
		goto errHdl;
	}

	DISP_DBG_LOG("%s:disp on ok\r\n", __func__);
errHdl:
	return ret;
}

INT32S disp_normal_uninit(void)
{
	INT32S ret = 0;

	ret = disp_close(disp_normal_fd);
	if(ret != 0){
		DISP_DBG_LOG("%s:close disp failed!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	ret = disp_layer_close(disp_normal_fd, disp_normal_layer);
	if(ret != 0){
		DISP_DBG_LOG("%s:close disp layer failed!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}

	ret = disp_layer_release(disp_normal_fd, disp_normal_layer);
	if(ret != 0){
		DISP_DBG_LOG("%s:release disp layer failed!\r\n", __func__);
		ret = -3;
		goto errHdl;
	}

	ret = disp_layer_off(disp_normal_fd, disp_normal_layer);
	if(ret != 0){
		DISP_DBG_LOG("%s:disp layer off failed!\r\n", __func__);
		ret = -6;
		goto errHdl;
	}

	ret = disp_lcd_off(disp_normal_fd);
	if(ret != 0){
		DISP_DBG_LOG("%s:disp lcd off failed!\r\n", __func__);
		ret = -7;
		goto errHdl;
	}
	DISP_DBG_LOG("%s:disp on ok\r\n", __func__);
errHdl:
	return ret;
}
#endif

INT32S disp_lcd_on(INT32S dispFd)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
    ret = ioctl(dispFd, DISP_CMD_LCD_ON, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl disp handle is not exist!\r\n", __func__);
		//dispFd = -1;
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_lcd_off(INT32S dispFd)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
    ret = ioctl(dispFd, DISP_CMD_LCD_OFF, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl close disp handle is not exist!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_open(INT8S *path)
{
	INT32S dispFd = 0;

	if(path == NULL){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		dispFd = -1;
		goto errHdl;
	}

	dispFd = open(path, O_RDWR);
	if(dispFd == -1){
		DISP_DBG_LOG("%s:open disp handle error!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return dispFd;
}


INT32S disp_open_backlight(void)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	if (disp_fd <= 0) {
		printf("%s: disp device has not open yet.\n", __func__);
		return -1;
	}

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = 100;
	ret = ioctl(disp_fd, DISP_CMD_LCD_SET_BRIGHTNESS, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s: fail to open backlight!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_close_backlight(void)
{
	INT32S ret = 0;
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	if (disp_fd <= 0) {
		printf("%s: disp device has not open yet.\n", __func__);
		return -1;
	}

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = 0;
	ret = ioctl(disp_fd, DISP_CMD_LCD_SET_BRIGHTNESS, (void*)ioctlParam);
	if(ret != 0){
		DISP_DBG_LOG("%s: fail to open backlight!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}

errHdl:
	return ret;
}


INT32S disp_close(INT32S dispFd)
{
	INT32S ret = 0;

	ret = close(dispFd);
	if(ret != 0){
		DISP_DBG_LOG("%s:close disp handle failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_init(void)
{
	INT32S ret = 0;

	ret = disp_scaler_init();
	if(ret != 0){
		DISP_DBG_LOG("%s:disp scaler init failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_uninit(void)
{
	INT32S ret = 0;

	ret = disp_scaler_uninit();
	if(ret != 0){
		DISP_DBG_LOG("%s:disp scaler uninit failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

INT32S disp_screenwidth_get()
{
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];
	INT32U width;

	DISP_CLAR(ioctlParam);
	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = 0;

	width = ioctl(disp_fd,DISP_CMD_SCN_GET_WIDTH,ioctlParam);
	return width;
}


INT32S disp_screenheight_get()
{
	INT32U ioctlParam[DISP_IOCTL_ARG_MAX];
	INT32U height;

	DISP_CLAR(ioctlParam);
	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = 0;

	height = ioctl(disp_fd,DISP_CMD_SCN_GET_HEIGHT,ioctlParam);
	return height;
}

INT32S disp_get_cur_fd()
{
	return disp_fd;
}

#endif // #if MODULE_DISP_COMPILE_CFG == 1
