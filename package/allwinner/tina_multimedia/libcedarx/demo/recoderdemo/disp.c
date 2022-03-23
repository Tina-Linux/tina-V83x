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
#include "disp.h"

#define DISP_CLAR(x)	memset(&(x),  0, sizeof(x))
static unsigned char DISP_LOG = 1;
#define DISP_DBG_LOG	 if(DISP_LOG)  printf

static int disp_fd = 0;
static unsigned int disp_layer_id = 1;
static unsigned char disp_video_start_flag = 0;

int disp_layer_param_set(int dispFd, unsigned int dispLayer, void *dispLayerInfo)
{
	int ret = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];
#ifdef __SUNXI_DISPLAY2__
		disp_layer_config *config;
#else
		disp_layer_info *config;
#endif
#ifdef __SUNXI_DISPLAY2__

	config = (disp_layer_config *)dispLayerInfo;
#else
	config = (disp_layer_info *)dispLayerInfo;

#endif
	if(dispLayerInfo == NULL){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	DISP_CLAR(ioctlParam);

#ifdef __SUNXI_DISPLAY2__
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
    ioctlParam[1] = (unsigned int)config;
    ioctlParam[2] = 1;
	ret = ioctl(dispFd, DISP_LAYER_SET_CONFIG, (void*)ioctlParam);
#else
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
    ioctlParam[1] = dispLayer;
    ioctlParam[2] = (unsigned int)config;
    ret = ioctl(dispFd, DISP_CMD_LAYER_SET_INFO, (void*)ioctlParam);
#endif
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl set disp layer param failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_layer_param_get(int dispFd, unsigned int dispLayer, void *dispLayerInfo)
{
	int ret = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];

	if(dispLayerInfo == NULL){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	DISP_CLAR(ioctlParam);

#ifdef __SUNXI_DISPLAY2__
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
    ioctlParam[1] = (unsigned int)dispLayerInfo;
    ioctlParam[2] = 1;
	ret = ioctl(dispFd, DISP_LAYER_GET_CONFIG, (void*)ioctlParam);
#else
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
    ioctlParam[1] = dispLayer;
    ioctlParam[2] = (unsigned int)dispLayerInfo;
    ret = ioctl(dispFd, DISP_CMD_LAYER_GET_INFO, (void*)ioctlParam);
#endif
	if(ret != 0){
		DISP_DBG_LOG("%s:ioctl get disp layer param failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_layer_frm_buf_open(char *path)
{
	int fbFd = 0;

	if(path == NULL){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		fbFd = -1;
		goto errHdl;
	}

	fbFd = open(path, O_RDWR);
	if(fbFd < 0){
		DISP_DBG_LOG("%s:open disp layer frame buffer handle error!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return fbFd;
}

int disp_layer_frm_buf_close(int fbFd)
{
	int ret = 0;

	ret = close(fbFd);
	if(ret != 0){
		DISP_DBG_LOG("%s:close disp layer frame buffer handle failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_layer_frm_buf_layer_get(int fbFd, DISP_LAYER_ID fbLayerId, unsigned int *fbLayer)
{
	int ret = 0;
	unsigned int tmpFbLayer = 0;

	if(fbLayer == NULL){
		DISP_DBG_LOG("%s:param error!\r\n", __func__);
		goto errHdl;
	}
#ifndef __SUNXI_DISPLAY2__
	if(fbLayerId == 0){
		ret = ioctl(fbFd, FBIOGET_LAYER_HDL_0, (void*)&tmpFbLayer);
	}else if(fbLayerId == 1){
		ret = ioctl(fbFd, FBIOGET_LAYER_HDL_1, (void*)&tmpFbLayer);
	}

	if(ret< 0){
		DISP_DBG_LOG("%s:ioctl get disp layer fb handle failed!\r\n", __func__);
		goto errHdl;
	}
#endif
	*fbLayer = tmpFbLayer;

	DISP_DBG_LOG("%s:fbLayer = %u\r\n", __func__, *fbLayer);
errHdl:
	return ret;
}

int disp_layer_on(int dispFd, unsigned int dispLayer)
{
	int ret = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = dispLayer;
#ifdef __SUNXI_DISPLAY2__
	ret = ioctl(dispFd, DISP_LAYER_ENABLE, (void*)ioctlParam);
#else
    ret = ioctl(dispFd, DISP_CMD_LAYER_ENABLE, (void*)ioctlParam);
#endif
	if(ret < 0){
		DISP_DBG_LOG("%s:ioctl disp disp on failed!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_layer_off(int dispFd, unsigned int dispLayer)
{
	int ret = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = dispLayer;
#ifdef __SUNXI_DISPLAY2__
		ret = ioctl(dispFd, DISP_LAYER_DISABLE, (void*)ioctlParam);
#else
		ret = ioctl(dispFd, DISP_CMD_LAYER_DISABLE, (void*)ioctlParam);
#endif
	if(ret < 0) {
		DISP_DBG_LOG("%s:ioctl disp disp off failed!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_video_chanel_open(void)
{
	int ret = -1;

	if (0 == disp_video_start_flag) {
		DISP_DBG_LOG("%s: open video chanel\n", __func__);
		ret = disp_layer_on(disp_fd, disp_layer_id);
		if(ret < 0){
			DISP_DBG_LOG("%s:disp layer disp on failed!\r\n", __func__);
			ret = -2;
			goto errHdl;
		}

		disp_video_start_flag = 1;
	} else {
		ret = 0;
	}

errHdl:
	return ret;
}

int disp_video_chanel_close(void)
{
	int ret = -1;

	if (1 == disp_video_start_flag) {
		ret = disp_layer_off(disp_fd, disp_layer_id);
		if(ret < 0) {
			DISP_DBG_LOG("%s:disp layer disp off failed!\r\n", __func__);
			ret = -1;
			goto errHdl;
		}
		disp_video_start_flag = 0;
	} else {
		ret = 0;
	}

errHdl:
	return ret;
}

static disp_pixel_format convert_pixel_format(DISP_INPUT_FORMAT fbFmt)
{
	disp_pixel_format fmt = 0;
	switch (fbFmt) {
	case DISP_YUV420_PLANNAR:
		fmt = DISP_FORMAT_YUV420_P;
		break;
	case DISP_MB32_PLANNAR:
		fmt = DISP_FORMAT_YUV420_SP_UVUV;
		break;
	case DISP_ARGB8888_INTERLEAVED:
		fmt = DISP_FORMAT_ARGB_8888;
		break;
	default:
		fmt = DISP_UNKOWN_FORMAT;
		DISP_DBG_LOG("%s: unkown input fbFmt=%d\n", __func__, fbFmt);
		break;
	}

	return fmt;
}

int disp_update_layer_param(unsigned int img_width, unsigned int img_height, DISP_INPUT_FORMAT fbFmt)
{
#ifdef __SUNXI_DISPLAY2__
	disp_layer_config dispLayerInfo;
#else
	disp_layer_info dispLayerInfo;
#endif
	disp_pixel_format format;
	unsigned char change_flag = 0;
	int ret = -1;

	DISP_CLAR(dispLayerInfo);
	ret = disp_layer_param_get(disp_fd, disp_layer_id, (void *)&dispLayerInfo);
	if (ret < 0) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n",
			__func__, disp_fd, disp_layer_id);
		return -1;
	}

#ifdef __SUNXI_DISPLAY2__
	if ((img_width != dispLayerInfo.info.screen_win.width) ||
		(img_width != dispLayerInfo.info.fb.crop.width)) {
		dispLayerInfo.info.screen_win.width = img_width;
		dispLayerInfo.info.fb.crop.width = img_width;
		change_flag = 1;
	}

	if ((img_height != dispLayerInfo.info.screen_win.height) ||
		(img_height != dispLayerInfo.info.fb.crop.height)) {
		dispLayerInfo.info.screen_win.height = img_height;
		dispLayerInfo.info.fb.crop.height = img_height;
		change_flag = 1;
	}

	format = convert_pixel_format(fbFmt);
	if ((DISP_UNKOWN_FORMAT != format) && (dispLayerInfo.info.fb.format != format)) {
		dispLayerInfo.info.fb.format = format;
		change_flag = 1;
	}
#else
	if ((img_width != dispLayerInfo.fb.src_win.width) ||
		(img_width != dispLayerInfo.screen_win.width)) {
		dispLayerInfo.fb.src_win.width = img_width;
		dispLayerInfo.screen_win.width = img_width;
		change_flag = 1;
	}

	if ((img_height != dispLayerInfo.fb.src_win.height) ||
		(img_height != dispLayerInfo.screen_win.height)) {
		dispLayerInfo.fb.src_win.height = img_height;
		dispLayerInfo.screen_win.height = img_height;
		change_flag = 1;
	}

	format = convert_pixel_format(fbFmt);
	if ((DISP_UNKOWN_FORMAT != format) && (dispLayerInfo.fb.format != format)) {
		dispLayerInfo.fb.format = format;
		change_flag = 1;
	}
#endif
	//no need to update param of layer
	if (0 == change_flag) {
		return 0;
	}

	//set param to layer
	ret = disp_layer_param_set(disp_fd, disp_layer_id, (void *)&dispLayerInfo);
	if(ret < 0){
		DISP_DBG_LOG("%s:fail to update param of layer(%u), ret=%d!\r\n",
			__func__, disp_layer_id, ret);
		return -2;
	}
	DISP_DBG_LOG("%s: update param of layer(%u) ok\n",
		__func__, disp_layer_id);

	return 0;
}

int disp_ctrl_video_alpha(unsigned char alpha)
{
#ifdef __SUNXI_DISPLAY2__
	disp_layer_config dispLayerInfo;
#else
	disp_layer_info dispLayerInfo;
#endif
	int ret = -1;
	unsigned char alphaVal = 0;

	alphaVal = alpha;
	DISP_CLAR(dispLayerInfo);
	ret = disp_layer_param_get(disp_fd, disp_layer_id, (void *)&dispLayerInfo);
	if (ret < 0) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n", __func__, disp_fd, disp_layer_id);
		return -1;
	}
#ifdef __SUNXI_DISPLAY2__
	dispLayerInfo.info.alpha_mode = 1; /*0:pixel alpha 1:global alpha*/
	if (alphaVal != dispLayerInfo.info.alpha_value) {
		dispLayerInfo.info.alpha_value = alphaVal;
	}
#else
	dispLayerInfo.alpha_mode = 1;	/*0:pixel alpha 1:global alpha*/
	if (alphaVal != dispLayerInfo.alpha_value) {
		dispLayerInfo.alpha_value = alphaVal;
	}
#endif
	ret = disp_layer_param_set(disp_fd, disp_layer_id, (void *)&dispLayerInfo);
	if(ret < 0){
		DISP_DBG_LOG("%s:set disp layer param failed, ret=%d!\r\n", __func__, ret);
		return -2;
	}

	return 0;
}

static void print_display_layer(void *info)
{
#ifdef __SUNXI_DISPLAY2__
	disp_layer_config *config;
	config = (disp_layer_config *)info;
#else
	disp_layer_info *config;
	config = (disp_layer_info *)info;
#endif

#ifdef __SUNXI_DISPLAY2__
	printf("m:%d alm:%u alval:%u src:(%lld %lld, %lldx%lld) scn(%d %d, %ux%u)\n",
		config->info.mode, config->info.alpha_mode, config->info.alpha_value,
		config->info.fb.crop.x,
		config->info.fb.crop.y,
		config->info.fb.crop.width,
		config->info.fb.crop.height,
		config->info.screen_win.x,
		config->info.screen_win.y,
		config->info.screen_win.width,
		config->info.screen_win.height);

	printf("(a0:%llu a1:%llu a2:%llu) fbformat:%d fb(%ux%u)\n",
		config->info.fb.addr[0],
		config->info.fb.addr[1],
		config->info.fb.addr[2],
		config->info.fb.format,
		config->info.fb.size[0].width,
		config->info.fb.size[0].height);
#else


	printf("m:%d pipe:%d alen:%d alval:%d src:(%d %d,%u x %u) scn(%d %d, %u x %u)\n",
		config->mode ,config->pipe,config->alpha_mode,config->alpha_value,
		config->fb.src_win.x,
		config->fb.src_win.y,
		config->fb.src_win.width,
		config->fb.src_win.height,
		config->screen_win.x,
		config->screen_win.y,
		config->screen_win.width,
		config->screen_win.height);
	printf("(a0:%x a1:%x a2:%x) fbformat:%d fbw:%d fbh:%d\n",
		config->fb.addr[0],
		config->fb.addr[1],
		config->fb.addr[2],
		config->fb.format,
		config->fb.size.width,
		config->fb.size.height);
#endif
}

/*
update all param at once, to get more faster
*/
int disp_video_layer_param_set(unsigned int phy_addr[],
		enum VIDEO_PIXEL_FORMAT fmt, struct win_info *win)
{
#ifdef __SUNXI_DISPLAY2__
	disp_layer_config config;
#else
	disp_layer_info config;
#endif
	disp_pixel_format format;
	unsigned char update_flag = 0;
	int ret = -1;

	if (0 == phy_addr) {
		DISP_DBG_LOG("%s: invalid phy address\n", __func__);
		return -1;
	}

	DISP_CLAR(config);
	ret = disp_layer_param_get(disp_fd, disp_layer_id, (void *)&config);
	if (ret < 0) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n",
			__func__, disp_fd, disp_layer_id);
		ret = -1;
		goto errHdl;
	}

#ifdef __SUNXI_DISPLAY2__
    //* transform pixel format.
    switch(fmt)
    {
        case VIDEO_PIXEL_FORMAT_YUV_PLANER_420:
        config.info.fb.format = DISP_FORMAT_YUV420_P;
        config.info.fb.addr[0] = (unsigned long )phy_addr[0];
        config.info.fb.addr[1] = (unsigned long )phy_addr[1];
        config.info.fb.addr[2] = (unsigned long )phy_addr[2];
        config.info.fb.size[0].width = win->src_win.nWidth;
        config.info.fb.size[0].height	= win->scn_win.nHeight;
        config.info.fb.size[1].width = win->src_win.nWidth/2;
        config.info.fb.size[1].height = win->scn_win.nHeight/2;
        config.info.fb.size[2].width = win->src_win.nWidth/2;
        config.info.fb.size[2].height = win->scn_win.nHeight/2;
        break;

        case VIDEO_PIXEL_FORMAT_YV12:
        config.info.fb.format = DISP_FORMAT_YUV420_P;
        config.info.fb.addr[0] = (unsigned long )phy_addr[0];
        config.info.fb.addr[1] = (unsigned long )phy_addr[2];
        config.info.fb.addr[2] = (unsigned long )phy_addr[1];
        config.info.fb.size[0].width = win->src_win.nWidth;
        config.info.fb.size[0].height = win->scn_win.nHeight;
        config.info.fb.size[1].width = win->src_win.nWidth/2;
        config.info.fb.size[1].height = win->scn_win.nHeight/2;
        config.info.fb.size[2].width = win->src_win.nWidth/2;
        config.info.fb.size[2].height = win->scn_win.nHeight/2;
        break;

        case VIDEO_PIXEL_FORMAT_NV12:
        config.info.fb.format = DISP_FORMAT_YUV420_SP_UVUV;
        config.info.fb.addr[0] = (unsigned long )phy_addr[0];
        config.info.fb.addr[1] = (unsigned long )phy_addr[1];
        config.info.fb.size[0].width = win->src_win.nWidth;
        config.info.fb.size[1].width = win->src_win.nWidth/2;
        config.info.fb.size[1].height = win->scn_win.nHeight/2;
        config.info.fb.size[2].height = win->scn_win.nHeight/2;
        break;

        case VIDEO_PIXEL_FORMAT_NV21:
        config.info.fb.format = DISP_FORMAT_YUV420_SP_VUVU;
        config.info.fb.addr[0] = (unsigned long )phy_addr[0];
        config.info.fb.addr[1] = (unsigned long )phy_addr[1];
        config.info.fb.size[0].width = win->src_win.nWidth;
        config.info.fb.size[1].width = win->src_win.nWidth/2;
        config.info.fb.size[2].width = win->src_win.nWidth/2;
        config.info.fb.size[0].height = win->scn_win.nHeight;
        config.info.fb.size[1].height = win->scn_win.nHeight/2;
        config.info.fb.size[2].height = win->scn_win.nHeight/2;
        break;

        default:
        {
            DISP_DBG_LOG("unsupported pixel format.");
            return -1;
        }
    }

    //* initialize the layerInfo.
    config.info.mode		= LAYER_MODE_BUFFER;
    config.info.zorder		= 1;
    config.info.alpha_mode	= 1;
    config.info.alpha_value	= 0xff;

    //* image size.
    config.info.fb.crop.x = 0;
    config.info.fb.crop.y = 0;
    config.info.fb.crop.width   = (unsigned long long)win->src_win.nWidth << 32;
    config.info.fb.crop.height  = (unsigned long long)win->src_win.nHeight << 32;
    config.info.fb.color_space  = (win->src_win.nHeight < 720) ? DISP_BT601 : DISP_BT709;

    //* source window.
    config.info.screen_win.x        = win->scn_win.nLeftOff;
    config.info.screen_win.y        = win->scn_win.nTopOff;
    config.info.screen_win.width    = win->nScreenWidth;
    config.info.screen_win.height   = win->nScreenHeight;
    config.channel = 0;
    config.enable = 1;
    config.layer_id = 0;
#else
    switch(fmt)
    {
        case VIDEO_PIXEL_FORMAT_YUV_PLANER_420:
        config.fb.format = DISP_FORMAT_YUV420_P;
        config.fb.addr[0] = (unsigned long )phy_addr[0];
        config.fb.addr[1] = (unsigned long )phy_addr[1];
        config.fb.addr[2] = (unsigned long )phy_addr[2];
        break;

        case VIDEO_PIXEL_FORMAT_YV12:
        config.fb.format = DISP_FORMAT_YUV420_P;
        config.fb.addr[0] = (unsigned long )phy_addr[0];
        config.fb.addr[1] = (unsigned long )phy_addr[2];
        config.fb.addr[2] = (unsigned long )phy_addr[1];
        break;

        case VIDEO_PIXEL_FORMAT_NV12:
        config.fb.format = DISP_FORMAT_YUV420_SP_UVUV;
        config.fb.addr[0] = (unsigned long )phy_addr[0];
        config.fb.addr[1] = (unsigned long )phy_addr[1];
        break;

        case VIDEO_PIXEL_FORMAT_NV21:
        config.fb.format = DISP_FORMAT_YUV420_SP_VUVU;
        config.fb.addr[0] = (unsigned long )phy_addr[0];
        config.fb.addr[1] = (unsigned long )phy_addr[1];
        break;

        default:
        {
            DISP_DBG_LOG("unsupported pixel format.");
            return -1;
        }
    }

	config.mode		= DISP_LAYER_WORK_MODE_NORMAL;
	config.pipe		= 1;
	config.zorder		= 1;
	config.alpha_mode	= 1;
	config.alpha_value	= 0xff;

	config.fb.size.width	 = win->nScreenWidth;
	config.fb.size.height	 = win->nScreenHeight;
	config.fb.cs_mode		 = DISP_BT601;

	config.fb.src_win.x	 = 0;
	config.fb.src_win.y	 = 0;
	config.fb.src_win.width  = win->src_win.nWidth;
	config.fb.src_win.height	 = win->src_win.nWidth;

	config.screen_win.x	 =  win->scn_win.nLeftOff;
	config.screen_win.y	 =  win->scn_win.nTopOff;
	config.screen_win.width  = win->scn_win.nWidth;
	config.screen_win.height	 =  win->scn_win.nHeight;

#endif
	ret = disp_layer_param_set(disp_fd, disp_layer_id, (void *)&config);
	if (ret < 0) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n",
			__func__, disp_fd, disp_layer_id);
	}

errHdl:
	return ret;
}

int disp_src_win_set(unsigned int src_x, unsigned int src_y, unsigned int src_w, unsigned int src_h)
{
#ifdef __SUNXI_DISPLAY2__
	disp_layer_config dispLayerInfo;
#else
	disp_layer_info dispLayerInfo;
#endif
	int ret = -1;

	DISP_CLAR(dispLayerInfo);
	ret = disp_layer_param_get(disp_fd, disp_layer_id, (void *)&dispLayerInfo);
	if(ret < 0) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n",
				__func__, disp_fd, disp_layer_id);
		return -2;
	}
#ifdef __SUNXI_DISPLAY2__
	if((dispLayerInfo.info.fb.crop.x == src_x) &&
	   (dispLayerInfo.info.fb.crop.y == src_y) &&
	   (dispLayerInfo.info.fb.crop.width == src_w) &&
	   (dispLayerInfo.info.fb.crop.height == src_h)) {
		DISP_DBG_LOG("%s: src win no need to update!\r\n", __func__);
		return 0;
	}

	dispLayerInfo.info.fb.crop.x = src_x;
	dispLayerInfo.info.fb.crop.y = src_y;
	dispLayerInfo.info.fb.crop.width = src_w;
	dispLayerInfo.info.fb.crop.height = src_h;
#else
	if((dispLayerInfo.fb.src_win.x == src_x) &&
		(dispLayerInfo.fb.src_win.y == src_y) &&
		(dispLayerInfo.fb.src_win.width == src_w) &&
		(dispLayerInfo.fb.src_win.height == src_h)){
		DISP_DBG_LOG("%s: src win no need to update!\r\n", __func__);
		return 0;
	}

	dispLayerInfo.fb.src_win.x = src_x;
	dispLayerInfo.fb.src_win.y = src_y;
	dispLayerInfo.fb.src_win.width = src_w;
	dispLayerInfo.fb.src_win.height = src_h;
#endif
	DISP_DBG_LOG("%s: set source window: x=%u, y=%u, w=%u, h=%u\n",
			__func__, src_x, src_y, src_y, src_h);

	ret = disp_layer_param_set(disp_fd, disp_layer_id, (void *)&dispLayerInfo);
	if(ret < 0){
		DISP_DBG_LOG("%s:set disp layer param failed, ret=%d!\r\n", __func__, ret);
		return -3;
	}

	return 0;
}

/*
display buffer regard as a horizon screen, if you use MCU panel ,
you need to rotate 90 degree and reset the video window like below:
case 0:
case 180:
	scn_x = 0;
	scn_y = GUI_TOP_STATUS_BAR_HEIGHT_PIXEL;
	scn_w = GUI_FRM_BUF_WIDTH_PIXEL;
	scn_h = GUI_FRM_BUF_HEIGHT_PIXEL - GUI_TOP_STATUS_BAR_HEIGHT_PIXEL
	- GUI_BOTTOM_STATUS_BAR_HEIGHT_PIXEL;

case 90:
case 270:
	scn_x = GUI_TOP_STATUS_BAR_HEIGHT_PIXEL;
	scn_y = 0;
	scn_w = GUI_FRM_BUF_HEIGHT_PIXEL - GUI_TOP_STATUS_BAR_HEIGHT_PIXEL
	- GUI_BOTTOM_STATUS_BAR_HEIGHT_PIXEL;
	scn_h = GUI_FRM_BUF_WIDTH_PIXEL;
*/
int disp_scn_win_set(unsigned int scn_x, unsigned int scn_y,
		unsigned int scn_w, unsigned int scn_h)
{
#ifdef __SUNXI_DISPLAY2__
	disp_layer_config dispLayerInfo;
#else
	disp_layer_info dispLayerInfo;
#endif
	int ret = -1;

	DISP_CLAR(dispLayerInfo);
	ret = disp_layer_param_get(disp_fd, disp_layer_id,
			(void *)&dispLayerInfo);
	if(ret < 0) {
		DISP_DBG_LOG("%s: fail to get layer param, fd=%d, layer=%u\n",
				__func__, disp_fd, disp_layer_id);
		return -2;
	}

	#ifdef __SUNXI_DISPLAY2__
	if((dispLayerInfo.info.screen_win.x == scn_x) &&
	   (dispLayerInfo.info.screen_win.y == scn_y) &&
	   (dispLayerInfo.info.screen_win.width == scn_w) &&
	   (dispLayerInfo.info.screen_win.height == scn_h)){
		DISP_DBG_LOG("%s: screen window no need to update!\r\n", __func__);
		return 0;
	}

	dispLayerInfo.info.screen_win.x = scn_x;
	dispLayerInfo.info.screen_win.y = scn_y;
	dispLayerInfo.info.screen_win.width = scn_w;
	dispLayerInfo.info.screen_win.height = scn_h;
#else
	if((dispLayerInfo.screen_win.x == scn_x) &&
		(dispLayerInfo.screen_win.y == scn_y) &&
		(dispLayerInfo.screen_win.width == scn_w) &&
		(dispLayerInfo.screen_win.height == scn_h)){
		DISP_DBG_LOG("%s: screen window no need to update!\r\n", __func__);
		return 0;
	}

	dispLayerInfo.screen_win.x = scn_x;
	dispLayerInfo.screen_win.y = scn_y;
	dispLayerInfo.screen_win.width = scn_w;
	dispLayerInfo.screen_win.height = scn_h;
#endif

	DISP_DBG_LOG("%s: set screen window: x=%u, y=%u, w=%u, h=%u\n",
			__func__, scn_x, scn_y, scn_y, scn_h);
	ret = disp_layer_param_set(disp_fd, disp_layer_id, (void *)&dispLayerInfo);
	if(ret < 0){
		DISP_DBG_LOG("%s:set disp layer param failed, ret=%d!\r\n", __func__, ret);
		return -3;
	}

	return 0;
}

int disp_scaler_init(void)
{
	int ret = 0;

	disp_fd = open(DISP_DEV_NAME, O_RDWR);
	if(disp_fd == -1){
		DISP_DBG_LOG("%s:open disp handle is not exist!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

	/*ret = disp_layer_param_set(disp_fd, disp_layer_id, (void *)&layer_info);
	if(ret < 0){
		DISP_DBG_LOG("%s:set disp layer param failed!\r\n", __func__);
		ret = -4;
		goto errHdl;
	}

	ret = disp_layer_on(disp_fd, disp_layer_id);
	if(ret < 0){
		DISP_DBG_LOG("%s:disp layer disp on failed!\r\n", __func__);
		ret = -7;
		goto errHdl;
	}*/

	ret = disp_lcd_on(disp_fd);
	if(ret < 0){
		DISP_DBG_LOG("%s:disp lcd on failed!\r\n", __func__);
		ret = -8;
		goto errHdl;
	}

	DISP_DBG_LOG("%s:disp on ok\r\n", __func__);
errHdl:
	return ret;
}

int disp_scaler_uninit(void)
{
	int ret = 0;

	 /* close back light of panel before un-init disp module */
	 disp_close_backlight();

	ret = disp_layer_off(disp_fd, disp_layer_id);
	if(ret < 0){
		DISP_DBG_LOG("%s:disp layer off failed,layer not open!\r\n", __func__);
	}

	ret = disp_lcd_off(disp_fd);
	if(ret < 0){
		DISP_DBG_LOG("%s:disp lcd off failed!\r\n", __func__);
		ret = -4;
		goto errHdl;
	}

	ret = disp_close(disp_fd);
	if(ret < 0){
		DISP_DBG_LOG("%s:close disp failed!\r\n", __func__);
		ret = -5;
		goto errHdl;
	}

	DISP_DBG_LOG("%s:disp on ok\r\n", __func__);
errHdl:
	return ret;
}

int disp_lcd_on(int dispFd)
{
	int ret = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
#ifdef __SUNXI_DISPLAY2__
	return ret;
#else
    ret = ioctl(dispFd, DISP_CMD_LCD_ENABLE, (void*)ioctlParam);
#endif
	if(ret < 0){
		DISP_DBG_LOG("%s:ioctl disp handle is not exist!\r\n", __func__);
		//dispFd = -1;
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_lcd_off(int dispFd)
{
	int ret = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
#ifdef __SUNXI_DISPLAY2__
	return ret;
#else
    ret = ioctl(dispFd, DISP_CMD_LCD_DISABLE, (void*)ioctlParam);
#endif
	if(ret < 0){
		DISP_DBG_LOG("%s:ioctl close disp handle is not exist!\r\n", __func__);
		ret = -1;
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_open(char *path)
{
	int dispFd = 0;

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

int disp_set_ioctl(int dispFd, unsigned int cmd,  unsigned long para)
{
	int ret = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];
	unsigned int disp_cmd;
	DISP_CLAR(ioctlParam);
	if (dispFd <= 0) {
		DISP_DBG_LOG("%s: disp device has not open yet.\n", __func__);
		return -1;
	}
#ifdef __SUNXI_DISPLAY2__
	switch(cmd) {
		case DISP_CMD_SET_BRIGHTNESS:
			disp_cmd = DISP_LCD_SET_BRIGHTNESS;
			break;
		case DISP_CMD_SET_CONTRAST:
		case DISP_CMD_SET_HUE:
		case DISP_CMD_SET_SATURATION:
			break;
		case DISP_CMD_SET_VEDIO_ENHANCE_DEFAULT:
			disp_cmd = DISP_ENHANCE_SET_MODE;
			break;
	}
#else
	switch(cmd) {
		case DISP_CMD_SET_BRIGHTNESS:
			disp_cmd = DISP_LCD_SET_BRIGHTNESS;
			break;
		case DISP_CMD_SET_CONTRAST:
			disp_cmd = DISP_CMD_LAYER_SET_CONTRAST;
			break;
		case DISP_CMD_SET_HUE:
			disp_cmd = DISP_CMD_LAYER_SET_HUE;
			break;
		case DISP_CMD_SET_SATURATION:
			disp_cmd = DISP_CMD_LAYER_SET_SATURATION;
			break;
		case DISP_CMD_SET_VEDIO_ENHANCE_DEFAULT:
			disp_cmd = DISP_CMD_SET_ENHANCE_MODE;
			break;
	}
#endif
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = para;
	ret = ioctl(dispFd, cmd, (void*)ioctlParam);
	if(ret < 0){
		DISP_DBG_LOG("%s: fail to open backlight!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}
errHdl:
	return ret;
}

int disp_open_backlight(void)
{
	int ret = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	if (disp_fd <= 0) {
		DISP_DBG_LOG("%s: disp device has not open yet.\n", __func__);
		return -1;
	}

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = 100;
#ifdef __SUNXI_DISPLAY2__
	ret = ioctl(disp_fd, DISP_LCD_SET_BRIGHTNESS, (void*)ioctlParam);

#else
	ret = ioctl(disp_fd, DISP_CMD_LCD_SET_BRIGHTNESS, (void*)ioctlParam);
#endif
	if(ret < 0){
		DISP_DBG_LOG("%s: fail to open backlight!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_close_backlight(void)
{
	int ret = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];

	DISP_CLAR(ioctlParam);

	if (disp_fd <= 0) {
		DISP_DBG_LOG("%s: disp device has not open yet.\n", __func__);
		return -1;
	}

	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = 0;
#ifdef __SUNXI_DISPLAY2__
	ret = ioctl(disp_fd, DISP_LCD_SET_BRIGHTNESS, (void*)ioctlParam);
#else
	ret = ioctl(disp_fd, DISP_CMD_LCD_SET_BRIGHTNESS, (void*)ioctlParam);
#endif
	if(ret < 0){
		DISP_DBG_LOG("%s: fail to open backlight!\r\n", __func__);
		ret = -2;
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_close(int dispFd)
{
	int ret = 0;

	ret = close(dispFd);
	if(ret != 0){
		DISP_DBG_LOG("%s:close disp handle failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_init(void)
{
	int ret = 0;

	ret = disp_scaler_init();
	if(ret < 0){
		DISP_DBG_LOG("%s:disp scaler init failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_uninit(void)
{
	int ret = 0;

	ret = disp_scaler_uninit();
	if(ret < 0){
		DISP_DBG_LOG("%s:disp scaler uninit failed!\r\n", __func__);
		goto errHdl;
	}

errHdl:
	return ret;
}

int disp_screenwidth_get()
{
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];
	unsigned int width;

	DISP_CLAR(ioctlParam);
	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = 0;
#ifdef __SUNXI_DISPLAY2__
	width = ioctl(disp_fd, DISP_GET_SCN_WIDTH, ioctlParam);
#else
	width = ioctl(disp_fd, DISP_CMD_GET_SCN_WIDTH, ioctlParam);
#endif

	return width;
}

int disp_screenheight_get()
{
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];
	unsigned int height;

	DISP_CLAR(ioctlParam);
	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = 0;
#ifdef __SUNXI_DISPLAY2__
	height = ioctl(disp_fd, DISP_GET_SCN_HEIGHT, ioctlParam);
#else
	height = ioctl(disp_fd, DISP_CMD_GET_SCN_HEIGHT, ioctlParam);
#endif

	return height;
}

int disp_get_cur_fd()
{
	return disp_fd;
}
