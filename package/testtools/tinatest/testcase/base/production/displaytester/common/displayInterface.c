#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>
#include "displayInterface.h"

/* ----disp global---- */
/* ----Set the background color---- */
int DispSetBackColor(int dispFb, unsigned int screenId, unsigned int color)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	disp_color ck;
	unsigned int r;
	unsigned int g;
	unsigned int b;
	r = (color >> 16) & 0xff;
	g = (color >> 8) & 0xff;
	b = (color >> 0) & 0xff;
	ck.alpha = 0xff;
	ck.red = r;
	ck.green = g;
	ck.blue = b;
	ioctlParam[1] = (unsigned long) &ck;
	return ioctl(dispFb, DISP_SET_BKCOLOR, ioctlParam);
#else
	disp_color_info ck;
	unsigned int r;
	unsigned int g;
	unsigned int b;
	r = (color >> 16) & 0xff;
	g = (color >> 8) & 0xff;
	b = (color >> 0) & 0xff;
	ck.alpha = 0xff;
	ck.red = r;
	ck.green = g;
	ck.blue = b;
	ioctlParam[1] = (unsigned long) &ck;
	return ioctl(dispFb, DISP_CMD_SET_BKCOLOR, ioctlParam);
#endif
}

/* ----Set filter color---- */
int DispSetColorKey(int dispFb, unsigned int screenId, unsigned int color)
{
#ifdef __SUNXI_DISPLAY2__
	/*DE2 does not support this cmd*/
	return DISP_NOT_SUPPORT;
#else
	disp_color_info ck;
	unsigned int r;
	unsigned int g;
	unsigned int b;
	r = (color >> 16) & 0xff;
	g = (color >> 8) & 0xff;
	b = (color >> 0) & 0xff;
	ck.alpha = 0xff;
	ck.red = r;
	ck.green = g;
	ck.blue = b;
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = screenId;
	ioctlParam[1] = (unsigned long) &ck;
	return ioctl(dispFb, DISP_CMD_SET_COLORKEY, ioctlParam);
#endif
}

/* ----Get the screen width---- */
int DispGetScrWidth(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_GET_SCN_WIDTH, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_GET_SCN_WIDTH, ioctlParam);
#endif
}

/* ----Get the screen height---- */
int DispGetScrHeight(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_GET_SCN_HEIGHT, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_GET_SCN_HEIGHT, ioctlParam);
#endif
}

/* ----Get the out put type---- */
int DispGetOutPutType(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_GET_OUTPUT_TYPE, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_GET_OUTPUT_TYPE, ioctlParam);
#endif
}

/* ----Set Vsync event enable---- */
int DispVsyncEventEnable(int dispFb, unsigned int screenId, bool enable)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) enable;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_VSYNC_EVENT_EN, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_VSYNC_EVENT_EN, ioctlParam);
#endif
}

/* ----Set blank enable---- */
int DispSetBlankEnable(int dispFb, unsigned int screenId, bool enable)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) enable;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_BLANK, ioctlParam);
#else
	/*DE1 does not support this cmd*/
	return DISP_NOT_SUPPORT;
#endif
}

/* ----Set shadow protect---- */
int DispShadowProtect(int dispFb, unsigned int screenId, bool protect)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) protect;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_SHADOW_PROTECT, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_SHADOW_PROTECT, ioctlParam);
#endif
}

/**
 * Device switch
 * You can set lcd/tv/hdmi/vga enable/disable, and set tv mode
 *
 * @outPutType lcd/tv/hdmi/vga, DE1 not support vga
 * @tvMode Screen support for the tv mode, DE1 lcd not support set tv mode
 * @enable 0 is disable, 1 is enable
 */
int DispDeviceSwitch(int dispFb, unsigned int screenId,
		disp_output_type outPutType, disp_tv_mode tvMode, bool enable)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	if (!enable)
		outPutType = (unsigned long) DISP_OUTPUT_TYPE_NONE;
	ioctlParam[1] = (unsigned long) outPutType;
	ioctlParam[2] = (unsigned long) tvMode;
	return ioctl(dispFb, DISP_DEVICE_SWITCH, ioctlParam);
#else
	switch (outPutType) {
	case DISP_OUTPUT_TYPE_LCD:
		if (enable) {
			return ioctl(dispFb, DISP_CMD_LCD_ENABLE, ioctlParam);
		} else {
			return ioctl(dispFb, DISP_CMD_LCD_DISABLE, ioctlParam);
		}
		break;
	case DISP_OUTPUT_TYPE_TV:
		if (enable) {
			ioctlParam[1] = (unsigned long) tvMode;
			int ret = ioctl(dispFb, DISP_CMD_TV_ON, ioctlParam);
			if (ret < 0)
				return ret;
			else
				return ioctl(dispFb, DISP_CMD_TV_SET_MODE, ioctlParam);
		} else {
			return ioctl(dispFb, DISP_CMD_TV_OFF, ioctlParam);
		}
		break;
	case DISP_OUTPUT_TYPE_HDMI:
		if (enable) {
			ioctlParam[1] = (unsigned long) tvMode;
			int ret = ioctl(dispFb, DISP_CMD_HDMI_ENABLE, ioctlParam);
			if (ret < 0)
				return ret;
			else
				return ioctl(dispFb, DISP_CMD_HDMI_SET_MODE, ioctlParam);
		} else {
			return ioctl(dispFb, DISP_CMD_HDMI_DISABLE, ioctlParam);
		}
		break;
	case DISP_OUTPUT_TYPE_VGA:
		return DISP_NOT_SUPPORT;
		break;
	default:
		return DISP_NOT_SUPPORT;
		break;
	}
#endif
}

/* ----Set color range---- */
int DispSetColorRange(int dispFb, unsigned int screenId,
		unsigned int colorRange)
{
#ifdef __SUNXI_DISPLAY2__
	unsigned long ioctlParam[4] = {0};
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) colorRange;
	return ioctl(dispFb, DISP_SET_COLOR_RANGE, ioctlParam);
#else
	/*DE1 does not support this cmd*/
	return DISP_NOT_SUPPORT;
#endif
}

/* ----Get color range---- */
int DispGetColorRange(int dispFb, unsigned int screenId)
{
#ifdef __SUNXI_DISPLAY2__
	unsigned long ioctlParam[4] = {0};
	ioctlParam[0] = (unsigned long) screenId;
	return ioctl(dispFb, DISP_GET_COLOR_RANGE, ioctlParam);
#else
	/*DE1 does not support this cmd*/
	return DISP_NOT_SUPPORT;
#endif
}

/* ----layer---- */
/* ----Set layer enable---- */
int DispSetLayerEnable(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int channelId, unsigned int layerNum, bool enable)
{
#ifdef __SUNXI_DISPLAY2__
	luapi_layer_config luapiconfig;
	memset(&luapiconfig, 0, sizeof(luapi_layer_config));
	int ret = DispGetLayerConfig(dispFb, screenId, layerId, channelId, layerNum,
			&luapiconfig);
	if (ret < 0)
		return ret;

	luapiconfig.layerConfig.enable = enable;

	ret = DispSetLayerConfig(dispFb, screenId, layerId, layerNum, &luapiconfig);

	return ret;
#else
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) layerId;
	if (enable)
		return ioctl(dispFb, DISP_CMD_LAYER_ENABLE, ioctlParam);
	else
		return ioctl(dispFb, DISP_CMD_LAYER_DISABLE, ioctlParam);
#endif
}

/**
 * Set layer config
 * You need to call DispGetLayerConfig before calling DispSetLayerConfig
 *
 * @layerId DE1 is layerId, DE2 is layer num
 */
int DispSetLayerConfig(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int layerNum, luapi_layer_config *luapiconfig)
{
#ifdef __SUNXI_DISPLAY2__
	unsigned long ioctlParam[4] = {0};
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) &luapiconfig->layerConfig;
	ioctlParam[2] = (unsigned long) layerNum;
	return ioctl(dispFb, DISP_LAYER_SET_CONFIG, ioctlParam);
#else
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) layerId;
	ioctlParam[2] = (unsigned long) &luapiconfig->layerConfig;
	return ioctl(dispFb, DISP_CMD_LAYER_SET_INFO, ioctlParam);
#endif
}

/* ----Get layer config---- */
int DispGetLayerConfig(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int channelId, unsigned int layerNum,
		luapi_layer_config *luapiConfig)
{
#ifdef __SUNXI_DISPLAY2__
	luapiConfig->layerConfig.channel = channelId;
	luapiConfig->layerConfig.layer_id = layerId;

	unsigned long ioctlParam[4] = {0};
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) &luapiConfig->layerConfig;
	ioctlParam[2] = (unsigned long) layerNum;
	return ioctl(dispFb, DISP_LAYER_GET_CONFIG, ioctlParam);
#else
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) layerId;
	ioctlParam[2] = (unsigned long) &luapiConfig->layerConfig;
	return ioctl(dispFb, DISP_CMD_LAYER_GET_INFO, ioctlParam);
#endif
}

/* ----Get layer zorder---- */
int DispSetLayerZorder(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int channelId, unsigned int layerNum, luapi_zorder zorder)
{
	luapi_layer_config luapiconfig;
	memset(&luapiconfig, 0, sizeof(luapi_layer_config));
	int ret = DispGetLayerConfig(dispFb, screenId, layerId, channelId, layerNum,
			&luapiconfig);
	if (ret < 0)
		return ret;

#ifdef __SUNXI_DISPLAY2__
	switch (zorder) {
		case LUAPI_ZORDER_TOP:
			luapiconfig.layerConfig.info.zorder = 11;
		break;
		case LUAPI_ZORDER_MIDDLE:
			luapiconfig.layerConfig.info.zorder = 5;
		break;
		case LUAPI_ZORDER_BOTTOM:
			luapiconfig.layerConfig.info.zorder = 0;
		break;
			default:
		break;
	}
#else
	switch (zorder) {
	case LUAPI_ZORDER_TOP:
		luapiconfig.layerConfig.zorder = 3;
		break;
	case LUAPI_ZORDER_MIDDLE:
		luapiconfig.layerConfig.zorder = 1;
		break;
	case LUAPI_ZORDER_BOTTOM:
		luapiconfig.layerConfig.zorder = 0;
		break;
	default:
		break;
	}
#endif
	ret = DispSetLayerConfig(dispFb, screenId, layerId, layerNum, &luapiconfig);
	return ret;
}

/* ----Get layer frame id---- */
int DispGetLayerFrameId(int dispFb, unsigned int screenId, unsigned int layerId,
		unsigned int channelId, unsigned int layerNum)
{
#ifdef __SUNXI_DISPLAY2__
	luapi_layer_config luapiconfig;
	memset(&luapiconfig, 0, sizeof(luapi_layer_config));
	int ret = DispGetLayerConfig(dispFb, screenId, layerId, channelId, layerNum,
			&luapiconfig);
	if (ret < 0)
		return ret;
	else
		return luapiconfig.layerConfig.info.id;
#else
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) layerId;
	return ioctl(dispFb, DISP_CMD_LAYER_GET_FRAME_ID, ioctlParam);
#endif
}

/* ----hdmi---- */
/**
 * Checkout hdmi support mode
 *
 * @tvMode The mode to check
 */
int DispCheckHdmiSupportMode(int dispFb, unsigned int screenId,
		disp_tv_mode tvMode)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) tvMode;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_HDMI_SUPPORT_MODE, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_HDMI_SUPPORT_MODE, ioctlParam);
#endif
}

/**
 * Get hdmi edid
 * DE1 Only supported in versions using the linux3.4/linux3.10 kernel
 * DE2 Only supported in versions using the linux3.4 kernel
 *
 * @buf Data
 * @bytes Data length, Maximum value 1024
 */
int DispGetHdmiEdid(int dispFb, unsigned int screenId, unsigned char *buf,
		unsigned int bytes)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) buf;
	ioctlParam[2] = (unsigned long) bytes;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_HDMI_GET_EDID, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_HDMI_GET_EDID, ioctlParam);
#endif
}

/* ----lcd---- */
/* ----Get the brightness---- */
int DispGetBrightness(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_LCD_GET_BRIGHTNESS, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_LCD_GET_BRIGHTNESS, ioctlParam);
#endif
}

/* ----Set the brightness---- */
int DispSetBrightness(int dispFb, unsigned int screenId,
		unsigned int brightness)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) brightness;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_LCD_SET_BRIGHTNESS, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_LCD_SET_BRIGHTNESS, ioctlParam);
#endif
}

/**
 * Set back light enable
 * DE1 Only supported in versions using the linux3.4/linux3.10 kernel
 * DE2 Only supported in versions using the linux3.4 kernel
 */
int DispSetBackLightEnable(int dispFb, unsigned int screenId, bool enable)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	if (enable)
		return ioctl(dispFb, DISP_LCD_BACKLIGHT_ENABLE, ioctlParam);
	else
		return ioctl(dispFb, DISP_LCD_BACKLIGHT_DISABLE, ioctlParam);
#else
	if (enable)
		return ioctl(dispFb, DISP_CMD_LCD_BACKLIGHT_ENABLE, ioctlParam);
	else
		return ioctl(dispFb, DISP_CMD_LCD_BACKLIGHT_DISABLE, ioctlParam);
#endif
}

/* ----capture---- */
/**
 * Start screen capture
 * DE2 linux3.4 kernel not supported and return -1
 */
int DispCaptureSatrt(int dispFb, unsigned int screenId,
		luapi_capture_info *luapiPapture)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	int ret = ioctl(dispFb, DISP_CAPTURE_START, ioctlParam);
	if(ret < 0) {
		return ret;
	} else {
		ioctlParam[1] = (unsigned long) &luapiPapture->captureInfo;
		ret = ioctl(dispFb, DISP_CAPTURE_COMMIT, ioctlParam);
		if(ret < 0)
			ioctl(dispFb, DISP_CAPTURE_STOP, ioctlParam);
		return ret;
	}
#else
	ioctlParam[1] = (unsigned long) &luapiPapture->captureInfo;
	return ioctl(dispFb, DISP_CMD_CAPTURE_SCREEN, ioctlParam);
#endif
}

/* ----Stop screen capture---- */
int DispCaptureStop(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_CAPTURE_STOP, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_CAPTURE_SCREEN_STOP, ioctlParam);
#endif
}

/* ---enhance --- */
/* ----Set enhance enable---- */
int DispSetEnhanceEnable(int dispFb, unsigned int screenId, bool enable)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	if (enable)
		return ioctl(dispFb, DISP_ENHANCE_ENABLE, ioctlParam);
	else
		return ioctl(dispFb, DISP_ENHANCE_DISABLE, ioctlParam);
#else
	if (enable)
		return ioctl(dispFb, DISP_CMD_ENHANCE_ENABLE, ioctlParam);
	else
		return ioctl(dispFb, DISP_CMD_ENHANCE_DISABLE, ioctlParam);
#endif
}

/* ----Set enhance demo enable---- */
int DispSetEnhanceDemoEnable(int dispFb, unsigned int screenId, bool enable)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	if (enable)
		return ioctl(dispFb, DISP_ENHANCE_DEMO_ENABLE, ioctlParam);
	else
		return ioctl(dispFb, DISP_ENHANCE_DEMO_DISABLE, ioctlParam);
#else
	if (enable)
		return DISP_NOT_SUPPORT;
	else
		return DISP_NOT_SUPPORT;
#endif
}

/* ----Get enhance enable---- */
int DispGetEnhanceEnable(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_GET_ENHANCE_EN, ioctlParam);
#endif
}

/* ----Set enhance window---- */
int DispSetEnhanceWindow(int dispFb, unsigned int screenId,
		luapi_disp_window dispWindow)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	disp_window window;
	window.x = dispWindow.x;
	window.y = dispWindow.y;
	window.width = dispWindow.width;
	window.height = dispWindow.height;
	ioctlParam[1] = (unsigned long) &window;
	return ioctl(dispFb, DISP_CMD_SET_ENHANCE_WINDOW, ioctlParam);
#endif
}

/* ----Get enhance window---- */
int DispGetEnhanceWindow(int dispFb, unsigned int screenId,
		luapi_disp_window *dispWindow)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	disp_window window;
	ioctlParam[1] = (unsigned long) &window;
	int ret = ioctl(dispFb, DISP_CMD_GET_ENHANCE_WINDOW, ioctlParam);
	if (ret < 0)
		return ret;
	dispWindow->x = window.x;
	dispWindow->y = window.y;
	dispWindow->width = window.width;
	dispWindow->height = window.height;
	return 0;
#endif
}

/* ----Set enhance mode---- */
int DispSetEnhanceMode(int dispFb, unsigned int screenId, unsigned int mode)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) mode;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_ENHANCE_SET_MODE, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_SET_ENHANCE_MODE, ioctlParam);
#endif
}

/* ----Get enhance mode---- */
int DispGetEnhanceMode(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_ENHANCE_GET_MODE, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_GET_ENHANCE_MODE, ioctlParam);
#endif
}

/**
 * Set enhance bright
 * Only supported in versions using the linux3.10 kernel
 */
int DispSetEnhanceBright(int dispFb, unsigned int screenId, unsigned int bright)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) bright;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_SET_BRIGHT, ioctlParam);
#endif
}

/**
 * Get enhance bright
 * Only supported in versions using the linux3.10 kernel
 */
int DispGetEnhanceBright(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_GET_BRIGHT, ioctlParam);
#endif
}

/**
 * Set enhance contrast
 * Only supported in versions using the linux3.10 kernel
 */
int DispSetEnhanceContrast(int dispFb, unsigned int screenId,
		unsigned int contrast)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) contrast;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_SET_CONTRAST, ioctlParam);
#endif
}

/**
 * Get enhance contrast
 * Only supported in versions using the linux3.10 kernel
 */
int DispGetEnhanceContrast(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_GET_CONTRAST, ioctlParam);
#endif
}

/**
 * Set enhance satuation
 * Only supported in versions using the linux3.10 kernel
 */
int DispSetEnhanceSatuation(int dispFb, unsigned int screenId,
		unsigned int satuation)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) satuation;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_SET_SATURATION, ioctlParam);
#endif
}

/**
 * Get enhance satuation
 * Only supported in versions using the linux3.10 kernel
 */
int DispGetEnhanceSatuation(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_GET_SATURATION, ioctlParam);
#endif
}

/**
 * Set enhance hue
 * Only supported in versions using the linux3.10 kernel
 */
int DispSetEnhanceHue(int dispFb, unsigned int screenId, unsigned int hue)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) hue;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_SET_HUE, ioctlParam);
#endif
}

/**
 * Get enhance hue
 * Only supported in versions using the linux3.10 kernel
 */
int DispGetEnhanceHue(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_GET_HUE, ioctlParam);
#endif
}

/* ----smart backlight---- */
/* ----Set smart backlight enable---- */
int DispSetSMBLEnable(int dispFb, unsigned int screenId, bool enable)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	if(enable)
		return ioctl(dispFb, DISP_SMBL_ENABLE, ioctlParam);
	else
		return ioctl(dispFb, DISP_SMBL_DISABLE, ioctlParam);
#else
	if (enable)
		return ioctl(dispFb, DISP_CMD_DRC_ENABLE, ioctlParam);
	else
		return ioctl(dispFb, DISP_CMD_DRC_DISABLE, ioctlParam);
#endif
}

/* ----Get smart backlight enable---- */
int DispGetSMBLEnable(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	return ioctl(dispFb, DISP_CMD_GET_DRC_EN, ioctlParam);
#endif
}

/* ----Set smart backlight window---- */
int DispSetSMBLWindow(int dispFb, unsigned int screenId,
		luapi_disp_window dispWindow)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	disp_rect window;
	window.x = dispWindow.x;
	window.y = dispWindow.y;
	window.width = dispWindow.width;
	window.height = dispWindow.height;
	ioctlParam[1] = (unsigned long) &window;
	return ioctl(dispFb, DISP_SMBL_SET_WINDOW, ioctlParam);
#else
	disp_window window;
	window.x = dispWindow.x;
	window.y = dispWindow.y;
	window.width = dispWindow.width;
	window.height = dispWindow.height;
	ioctlParam[1] = (unsigned long) &window;
	return ioctl(dispFb, DISP_CMD_DRC_SET_WINDOW, ioctlParam);
#endif
}

/* ----Get smart backlight window---- */
int DispGetSMBLWindow(int dispFb, unsigned int screenId,
		luapi_disp_window *dispWindow)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return DISP_NOT_SUPPORT;
#else
	disp_window window;
	ioctlParam[1] = (unsigned long) &window;
	int ret = ioctl(dispFb, DISP_CMD_DRC_GET_WINDOW, ioctlParam);
	if (ret < 0)
		return ret;
	dispWindow->x = window.x;
	dispWindow->y = window.y;
	dispWindow->width = window.width;
	dispWindow->height = window.height;
	return 0;
#endif
}

/* ---mem--- */
/* ----Mem Request---- */
int DispMemRequest(int dispFb, unsigned int memId, unsigned int memSize)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) memId;
	ioctlParam[1] = (unsigned long) memSize;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_MEM_REQUEST, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_MEM_REQUEST, ioctlParam);
#endif
}

/* ----Mem Release---- */
int DispMemRelease(int dispFb, unsigned int memId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) memId;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_MEM_RELEASE, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_MEM_RELEASE, ioctlParam);
#endif
}

/* ----Mem Get Adrress---- */
unsigned long DispMemGetAdrress(int dispFb, unsigned int memId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) memId;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_MEM_GETADR, ioctlParam);
#else
	return ioctl(dispFb, DISP_CMD_MEM_GETADR, ioctlParam);
#endif
}

/* ---rotate--- */
/**
 * Set Rotate Degree
 * Only supported in versions using the linux3.4 kernel
 * You have to modify sys_config.fex, enable rotate_sw, this function to work
 */
int DispSetRotateDegree(int dispFb, unsigned int screenId,
		luaip_rotate_degree degree)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
	ioctlParam[1] = (unsigned long) degree;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_ROTATION_SW_SET_ROT, ioctlParam);
#else
	return DISP_NOT_SUPPORT;
#endif
}

/**
 * Get Rotate Degree
 * Only supported in versions using the linux3.4 kernel
 * You have to modify sys_config.fex, enable rotate_sw, this function to work
 */
int DispGetRotateDegree(int dispFb, unsigned int screenId)
{
	unsigned long ioctlParam[4] = { 0 };
	ioctlParam[0] = (unsigned long) screenId;
#ifdef __SUNXI_DISPLAY2__
	return ioctl(dispFb, DISP_ROTATION_SW_GET_ROT, ioctlParam);
#else
	return DISP_NOT_SUPPORT;
#endif
}
