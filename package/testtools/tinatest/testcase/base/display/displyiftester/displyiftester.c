#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>

#include "mjson.h"

#include "../../production/displaytester/common/displayInterface.h"
#include "../../production/displaytester/common/displaytest.h"
#ifdef __SUNXI_DISPLAY2__
#include "../../production/displaytester/common/sunxi_display_v2.h"
#else
#include "../../production/displaytester/common/sunxi_display_v1.h"
#endif

struct test_lcd_info {
	int screen_id;
};

struct test_lcd_info test_info;

void parse_configs(void)
{
	int sid;

	sid = mjson_fetch_int(
			"/base/display/displyiftester/screen_id");
	if (sid != -1) {
		test_info.screen_id = sid;
		DISP_TEST_INFO(("id of the screen is %d \n", sid));
	} else {
		test_info.screen_id = 0;
		DISP_TEST_INFO(("get the screen id failed, use the default id 0\n"));
	}
}

int main(int argc, int *argv[])
{
	int disphd, ret;

	memset(&test_info, 0, sizeof(struct test_lcd_info));
	disphd = open("/dev/disp", O_RDWR);
	if (disphd == -1) {
		DISP_TEST_ERROR(("open display device faild ( %s )", strerror(errno)));
		goto err;
	}

	parse_configs();

	if (DispSetBackColor(disphd, test_info.screen_id, 0xFFDC143C) < 0) {
		DISP_TEST_FAIL(("DispSetBackColor FAILED"));
	} else {
		DISP_TEST_OK(("DispSetBackColor SUCCESS color is 0xFFDC143C"));
	}

	ret = DispSetColorKey(disphd, test_info.screen_id, 0xFFDCFC3C);
	if (ret >= 0) {
		DISP_TEST_OK(("DispSetColorKey SUCCESS color is 0xFFDCFC3C"));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetColorKey NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispSetColorKey FAILED"));
	}

	int width = DispGetScrWidth(disphd, test_info.screen_id);
	if (width >= 0) {
		DISP_TEST_OK(("DispGetScrWidth SUCCESS width is %d", width));
	} else {
		DISP_TEST_FAIL(("DispGetScrWidth FAILED"));
	}

	int height = DispGetScrHeight(disphd, test_info.screen_id);
	if (height >= 0) {
		DISP_TEST_OK(("DispGetScrHeight SUCCESS height is %d", height));
	} else {
		DISP_TEST_FAIL(("DispGetScrHeight FAILED"));
	}

	int outPutType = DispGetOutPutType(disphd, test_info.screen_id);
	if (outPutType > 0) {
		DISP_TEST_OK(
				("DispGetOutPutType SUCCESS outPutType is %d", outPutType));
	} else {
		DISP_TEST_FAIL(("DispGetOutPutType FAILED"));
	}

	ret = DispDeviceSwitch(disphd, test_info.screen_id, DISP_OUTPUT_TYPE_LCD,
			DISP_TV_MOD_1080P_60HZ, 1);
	if (ret >= 0) {
		DISP_TEST_OK(("DispDeviceSwitch SUCCESS type is LCD"));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetBlankEnable NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispDeviceSwitch FAILED"));
	}

	if (DispVsyncEventEnable(disphd, test_info.screen_id, 1) < 0) {
		DISP_TEST_FAIL(("DispVsyncEventEnable FAILED"));
	} else {
		DISP_TEST_OK(("DispVsyncEventEnable SUCCESS enable is 1"));
	}

	ret = DispSetBlankEnable(disphd, test_info.screen_id, 0);
	if (ret >= 0) {
		DISP_TEST_OK(("DispSetBlankEnable SUCCESS enable is 0"));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetBlankEnable NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispSetBlankEnable FAILED"));
	}

	if (DispShadowProtect(disphd, test_info.screen_id, 1) < 0) {
		DISP_TEST_FAIL(("DispShadowProtect FAILED"));
	} else {
		DISP_TEST_OK(("DispShadowProtect SUCCESS"));
	}

	if (DispSetColorRange(disphd, test_info.screen_id, 80) < 0) {
		DISP_TEST_FAIL(("DispSetColorRange FAILED"));
	} else {
		DISP_TEST_OK(("DispSetColorRange SUCCESS color range is 80"));
	}

	int colorRange = DispGetColorRange(disphd, test_info.screen_id);
	if (colorRange < 0) {
		DISP_TEST_FAIL(("DispGetColorRange FAILED"));
	} else {
		DISP_TEST_OK(
				("DispGetColorRange SUCCESS color range is %d",colorRange));
	}

	if (DispSetLayerEnable(disphd, test_info.screen_id, 0, 1, 1, 1) < 0) {
		DISP_TEST_FAIL(("DispSetLayerEnable FAILED"));
	} else {
		DISP_TEST_OK(("DispSetLayerEnable SUCCESS enable is 1"));
	}

	luapi_layer_config luapiconfig;
	memset(&luapiconfig, 0, sizeof(luapi_layer_config));
	if (DispGetLayerConfig(disphd, test_info.screen_id, 0, 1, 1, &luapiconfig)
			< 0) {
		DISP_TEST_FAIL(("DispGetLayerConfig FAILED"));
	} else {
		DISP_TEST_OK(("DispGetLayerConfig SUCCESS"));
	}

	if (DispSetLayerConfig(disphd, test_info.screen_id, 0, 1, &luapiconfig)
			< 0) {
		DISP_TEST_FAIL(("DispSetLayerConfig FAILED"));
	} else {
		DISP_TEST_OK(("DispSetLayerConfig SUCCESS"));
	}

	if (DispSetLayerZorder(disphd, test_info.screen_id, 0, 1, 1,
			LUAPI_ZORDER_TOP) < 0) {
		DISP_TEST_FAIL(("DispSetLayerZorder FAILED"));
	} else {
		DISP_TEST_OK(("DispSetLayerZorder SUCCESS"));
	}

	int frameId = DispGetLayerFrameId(disphd, test_info.screen_id, 0, 1, 1);
	if (frameId < 0) {
		DISP_TEST_FAIL(("DispGetLayerFrameId FAILED"));
	} else {
		DISP_TEST_OK(("DispGetLayerFrameId SUCCESS frameId is %d",frameId));
	}

	if (DispCheckHdmiSupportMode(disphd, test_info.screen_id,
			DISP_TV_MOD_1080P_60HZ) < 0) {
		DISP_TEST_FAIL(("DispCheckHdmiSupportMode FAILED"));
	} else {
		DISP_TEST_OK(
				("DispCheckHdmiSupportMode SUCCESS checkout mode is 1080P 60HZ"));
	}

	char hdmiEdid[32];
	if (DispGetHdmiEdid(disphd, test_info.screen_id, hdmiEdid, 32) < 0) {
		DISP_TEST_FAIL(("DispGetHdmiEdid FAILED"));
	} else {
		DISP_TEST_OK(("DispGetHdmiEdid SUCCESS hdmi edid is %s",hdmiEdid));
	}

	int brightness = DispGetBrightness(disphd, test_info.screen_id);
	if (brightness < 0) {
		DISP_TEST_FAIL(("DispGetBrightness FAILED"));
	} else {
		DISP_TEST_OK(("DispGetBrightness SUCCESS brightness is %d",brightness));
	}

	if (DispSetBrightness(disphd, test_info.screen_id, brightness) < 0) {
		DISP_TEST_FAIL(("DispSetBrightness FAILED"));
	} else {
		DISP_TEST_OK(("DispSetBrightness SUCCESS brightness is %d",brightness));
	}

	if (DispSetBackLightEnable(disphd, test_info.screen_id, 1) < 0) {
		DISP_TEST_FAIL(("DispSetBackLightEnable FAILED"));
	} else {
		DISP_TEST_OK(("DispSetBackLightEnable SUCCESS enable is 1"));
	}

	if (DispSetEnhanceEnable(disphd, test_info.screen_id, 1) < 0) {
		DISP_TEST_FAIL(("DispSetEnhanceEnable FAILED"));
	} else {
		DISP_TEST_OK(("DispSetEnhanceEnable SUCCESS enable is 1"));
	}

	ret = DispSetEnhanceDemoEnable(disphd, test_info.screen_id, 1);
	if (ret >= 0) {
		DISP_TEST_OK(("DispSetEnhanceDemoEnable SUCCESS enable is 1"));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetEnhanceDemoEnable NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispSetEnhanceDemoEnable FAILED"));
	}

	ret = DispGetEnhanceEnable(disphd, test_info.screen_id);
	if (ret >= 0) {
		DISP_TEST_OK(("DispGetEnhanceEnable SUCCESS enhanceEnable is %d",ret));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispGetEnhanceEnable NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispGetEnhanceEnable FAILED"));
	}

	luapi_disp_window dispWindow;
	dispWindow.x = 0;
	dispWindow.y = 0;
	dispWindow.width = width;
	dispWindow.height = height;
	ret = DispGetEnhanceWindow(disphd, test_info.screen_id, &dispWindow);
	if (ret >= 0) {
		DISP_TEST_OK(
				("DispGetEnhanceWindow SUCCESS x = %d y = %d w = %u h = %u",
						dispWindow.x,dispWindow.y,dispWindow.width,dispWindow.height));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispGetEnhanceWindow NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispGetEnhanceWindow FAILED"));
	}

	ret = DispSetEnhanceWindow(disphd, test_info.screen_id, dispWindow);
	if (ret >= 0) {
		DISP_TEST_OK(
				("DispSetEnhanceWindow SUCCESS x = %d y = %d w = %u h = %u",
						dispWindow.x,dispWindow.y,dispWindow.width,dispWindow.height));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetEnhanceWindow NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispSetEnhanceWindow FAILED"));
	}

	int enhanceMode = DispGetEnhanceMode(disphd, test_info.screen_id);
	if (enhanceMode < 0) {
		DISP_TEST_FAIL(("DispGetEnhanceMode FAILED"));
		enhanceMode = 8;
	} else {
		DISP_TEST_OK(
				("DispGetEnhanceMode SUCCESS enhanceMode is %d",enhanceMode));
	}

	if (DispSetEnhanceMode(disphd, test_info.screen_id, enhanceMode) < 0) {
		DISP_TEST_FAIL(("DispSetEnhanceMode FAILED"));
	} else {
		DISP_TEST_OK(
				("DispSetEnhanceMode SUCCESS enhanceMode is %d",enhanceMode));
	}

	int enhanceBright = DispGetEnhanceBright(disphd, test_info.screen_id);
	if (enhanceBright >= 0) {
		DISP_TEST_OK(
				("DispGetEnhanceBright SUCCESS enhanceBright is %d", enhanceBright));
	} else if (enhanceBright == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispGetEnhanceBright NOT SUPPORT"));
		enhanceBright = 50;
	} else {
		DISP_TEST_FAIL(("DispGetEnhanceBright FAILED"));
		enhanceBright = 50;
	}

	ret = DispSetEnhanceBright(disphd, test_info.screen_id, enhanceBright);
	if (ret >= 0) {
		DISP_TEST_OK(
				("DispSetEnhanceBright SUCCESS enhanceBright is %d",enhanceBright));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetEnhanceBright NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispSetEnhanceBright FAILED"));
	}

	int enhanceContrast = DispGetEnhanceContrast(disphd, test_info.screen_id);
	if (enhanceContrast >= 0) {
		DISP_TEST_OK(
				("DispGetEnhanceContrast SUCCESS enhanceContrast is %d", enhanceContrast));
	} else if (enhanceContrast == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispGetEnhanceContrast NOT SUPPORT"));
		enhanceContrast = 50;
	} else {
		DISP_TEST_FAIL(("DispGetEnhanceContrast FAILED"));
		enhanceContrast = 50;
	}

	ret = DispSetEnhanceContrast(disphd, test_info.screen_id, enhanceContrast);
	if (ret >= 0) {
		DISP_TEST_OK(
				("DispSetEnhanceContrast SUCCESS enhanceContrast is %d",enhanceContrast));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetEnhanceContrast NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispSetEnhanceContrast FAILED"));
	}

	int enhanceSatuation = DispGetEnhanceSatuation(disphd, test_info.screen_id);
	if (enhanceSatuation >= 0) {
		DISP_TEST_OK(
				("DispGetEnhanceSatuation SUCCESS enhanceSatuation is %d", enhanceSatuation));
	} else if (enhanceSatuation == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispGetEnhanceSatuation NOT SUPPORT"));
		enhanceSatuation = 50;
	} else {
		DISP_TEST_FAIL(("DispGetEnhanceSatuation FAILED"));
		enhanceSatuation = 50;
	}

	ret = DispSetEnhanceSatuation(disphd, test_info.screen_id,
			enhanceSatuation);
	if (ret >= 0) {
		DISP_TEST_OK(
				("DispSetEnhanceSatuation SUCCESS enhanceSatuation is %d",enhanceSatuation));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetEnhanceSatuation NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispSetEnhanceSatuation FAILED"));
	}

	int enhanceHue = DispGetEnhanceHue(disphd, test_info.screen_id);
	if (enhanceHue >= 0) {
		DISP_TEST_OK(("DispGetEnhanceHue SUCCESS enhanceHue is %d", enhanceHue));
	} else if (enhanceHue == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispGetEnhanceHue NOT SUPPORT"));
		enhanceHue = 50;
	} else {
		DISP_TEST_FAIL(("DispGetEnhanceHue FAILED"));
		enhanceHue = 50;
	}

	ret = DispSetEnhanceHue(disphd, test_info.screen_id, enhanceHue);
	if (ret >= 0) {
		DISP_TEST_OK(("DispSetEnhanceHue SUCCESS enhanceHue is %d",enhanceHue));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetEnhanceHue NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispSetEnhanceHue FAILED"));
	}

	int smblEnable = DispGetSMBLEnable(disphd, test_info.screen_id);
	if (smblEnable >= 0) {
		DISP_TEST_OK(("DispGetSMBLEnable SUCCESS smblEnable is %d", smblEnable));
	} else if (smblEnable == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispGetSMBLEnable NOT SUPPORT"));
		smblEnable = 0;
	} else {
		DISP_TEST_FAIL(("DispGetSMBLEnable FAILED"));
		smblEnable = 0;
	}

	if (DispSetSMBLEnable(disphd, test_info.screen_id, smblEnable) < 0) {
		DISP_TEST_FAIL(("DispSetSMBLEnable FAILED"));
	} else {
		DISP_TEST_OK(("DispSetSMBLEnable SUCCESS smblEnable is %d",smblEnable));
	}

	luapi_disp_window smblWindow;
	smblWindow.x = 0;
	smblWindow.y = 0;
	smblWindow.width = width;
	smblWindow.height = height;
	ret = DispGetSMBLWindow(disphd, test_info.screen_id, &smblWindow);
	if (ret >= 0) {
		DISP_TEST_OK(
				("DispGetSMBLWindow SUCCESS x = %d y = %d w = %u h = %u",
						smblWindow.x,smblWindow.y,smblWindow.width,smblWindow.height));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispGetSMBLWindow NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispGetSMBLWindow FAILED"));
	}

	if (DispSetSMBLWindow(disphd, test_info.screen_id, smblWindow) < 0){
		DISP_TEST_FAIL(("DispSetSMBLWindow FAILED"));
	} else {
		DISP_TEST_OK(
				("DispSetSMBLWindow SUCCESS x = %d y = %d w = %u h = %u",
						smblWindow.x,smblWindow.y,smblWindow.width,smblWindow.height));
	}

	int rotateDegree = DispGetRotateDegree(disphd, test_info.screen_id);
	if (rotateDegree >= 0) {
		DISP_TEST_OK(
				("DispGetRotateDegree SUCCESS rotateDegree is %d", rotateDegree));
	} else if (rotateDegree == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispGetRotateDegree NOT SUPPORT"));
		rotateDegree = ROTATION_DEGREE_0;
	} else {
		DISP_TEST_FAIL(("DispGetRotateDegree FAILED"));
		rotateDegree = ROTATION_DEGREE_0;
	}

	ret = DispSetRotateDegree(disphd, test_info.screen_id, rotateDegree);
	if (ret >= 0) {
		DISP_TEST_OK(("DispSetRotateDegree SUCCESS degree is %d", rotateDegree));
	} else if (ret == DISP_NOT_SUPPORT) {
		DISP_TEST_WARN(("DispSetRotateDegree NOT SUPPORT"));
	} else {
		DISP_TEST_FAIL(("DispSetRotateDegree FAILED"));
	}

	close(disphd);
	DISP_TEST_INFO(("GOOD, display interface test end.\n"));
	return 0;

	err: close(disphd);
	DISP_TEST_ERROR(("ERROR, display interface test failed."));
	return -1;
}
