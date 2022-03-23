/**
 * Copyright (c) 2017-2020 Allwinner Technology Co. Ltd.
 * All rights reserved.
 *
 * File Name: displayInterface.h
 * Description : Display engine interface,compatible DE1 DE2
 *				 Returns -2 if DE1 or DE2 does not support a function
 * History :
 * Author  : allwinnertech
 * Date    : 2017/12/08
 * Comment : first version
 */
#ifndef __DISPLAYINTERFACE_H_
#define __DISPLAYINTERFACE_H_

#include <stdbool.h>

#ifdef __SUNXI_DISPLAY2__
#include "sunxi_display_v2.h"
#else
#include "sunxi_display_v1.h"
#endif

#define DISP_NOT_SUPPORT -2

typedef enum {
	ROTATION_DEGREE_0 = 0,
	ROTATION_DEGREE_90 = 1,
	ROTATION_DEGREE_180 = 2,
	ROTATION_DEGREE_270 = 3
} luaip_rotate_degree;

typedef enum {
	LUAPI_ZORDER_TOP = 0, LUAPI_ZORDER_MIDDLE = 1, LUAPI_ZORDER_BOTTOM = 2
} luapi_zorder;

typedef struct luapi_layer_config {
#ifdef __SUNXI_DISPLAY2__
	disp_layer_config layerConfig;
#else
	disp_layer_info layerConfig;
#endif
} luapi_layer_config;

typedef struct luapi_capture_info {
#ifdef __SUNXI_DISPLAY2__
	disp_capture_info captureInfo;
#else
	disp_capture_para captureInfo;
#endif
} luapi_capture_info;

typedef struct {
	int x;
	int y;
	unsigned int width;
	unsigned int height;
} luapi_disp_window;

/* ----disp global---- */
int DispSetBackColor(int dispFb, unsigned int screenId, unsigned int color);
int DispSetColorKey(int dispFb, unsigned int screenId, unsigned int color); /*DE2 not support*/
int DispGetScrWidth(int dispFb, unsigned int screenId);
int DispGetScrHeight(int dispFb, unsigned int screenId);
int DispGetOutPutType(int dispFb, unsigned int screenId);
int DispVsyncEventEnable(int dispFb, unsigned int screenId, bool enable);
int DispSetBlankEnable(int dispFb, unsigned int screenId, bool enable); /*DE1 not support*/
int DispShadowProtect(int dispFb, unsigned int screenId, bool protect);
int DispDeviceSwitch(int dispFb, unsigned int screenId,
		disp_output_type outPutType, disp_tv_mode tvMode, bool enable);
int DispSetColorRange(int dispFb, unsigned int screenId,
		unsigned int colorRange); /*DE1 not support*/
int DispGetColorRange(int dispFb, unsigned int screenId); /*DE1 not support*/

/* ----layer---- */
int DispSetLayerEnable(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int channelId, unsigned int layerNum, bool enable);
int DispSetLayerConfig(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int layerNum, luapi_layer_config *luapiConfig);
int DispGetLayerConfig(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int channelId, unsigned int layerNum,
		luapi_layer_config *luapiConfig);
int DispSetLayerZorder(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int channelId, unsigned int layerNum, luapi_zorder zorder);
int DispGetLayerFrameId(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int channelId, unsigned int layerNum);

/* ----hdmi---- */
int DispCheckHdmiSupportMode(int dispFb, unsigned int screenId,
		disp_tv_mode tvMode);
int DispGetHdmiEdid(int dispFb, unsigned int screenId, unsigned char *buf,
		unsigned int bytes);

/* ----lcd---- */
int DispGetBrightness(int dispFb, unsigned int screenId);
int DispSetBrightness(int dispFb, unsigned int screenId,
		unsigned int brightness);
int DispSetBackLightEnable(int dispFb, unsigned int screenId, bool enable);

/* ----capture---- */
int DispCaptureSatrt(int dispFb, unsigned int screenId,
		luapi_capture_info *luapiPapture);
int DispCaptureStop(int dispFb, unsigned int screenId);

/* ---enhance--- */
int DispSetEnhanceEnable(int dispFb, unsigned int screenId, bool enable);
int DispSetEnhanceDemoEnable(int dispFb, unsigned int screenId, bool enable);	/*DE1 not support*/
int DispGetEnhanceEnable(int dispFb, unsigned int screenId);
int DispSetEnhanceWindow(int dispFb, unsigned int screenId,
		luapi_disp_window dispWindow); /*DE2 not support*/
int DispGetEnhanceWindow(int dispFb, unsigned int screenId,
		luapi_disp_window *dispWindow); /*DE2 not support*/
int DispSetEnhanceMode(int dispFb, unsigned int screenId,
		unsigned int mode);
int DispGetEnhanceMode(int dispFb, unsigned int screenId);
int DispSetEnhanceBright(int dispFb, unsigned int screenId,
		unsigned int bright);	/*DE2 not support*/
int DispGetEnhanceBright(int dispFb, unsigned int screenId);	/*DE2 not support*/
int DispSetEnhanceContrast(int dispFb, unsigned int screenId,
		unsigned int contrast);	/*DE2 not support*/
int DispGetEnhanceContrast(int dispFb, unsigned int screenId);	/*DE2 not support*/
int DispSetEnhanceSatuation(int dispFb, unsigned int screenId,
		unsigned int satuation);	/*DE2 not support*/
int DispGetEnhanceSatuation(int dispFb, unsigned int screenId);	/*DE2 not support*/
int DispSetEnhanceHue(int dispFb, unsigned int screenId, unsigned int hue);	/*DE2 not support*/
int DispGetEnhanceHue(int dispFb, unsigned int screenId);	/*DE2 not support*/

/* ---smart backlight--- */
int DispSetSMBLEnable(int dispFb, unsigned int screenId, bool enable);
int DispGetSMBLEnable(int dispFb, unsigned int screenId); /*DE2 not support*/
int DispSetSMBLWindow(int dispFb, unsigned int screenId,
		luapi_disp_window dispWindow);
int DispGetSMBLWindow(int dispFb, unsigned int screenId,
		luapi_disp_window *dispWindow); /*DE2 not support*/

/* ---mem--- */
int DispMemRequest(int dispFb, unsigned int memId, unsigned int memSize);
int DispMemRelease(int dispFb, unsigned int memId);
unsigned long DispMemGetAdrress(int dispFb, unsigned int memId);

/* ---rotate--- */
int DispSetRotateDegree(int dispFb, unsigned int screenId,
		luaip_rotate_degree degree);	/*DE1 not support*/
int DispGetRotateDegree(int dispFb, unsigned int screenId);	/*DE1 not support*/

#endif /* __DISPLAYINTERFACE_H_ */
