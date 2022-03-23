#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <string.h>
#include <signal.h>
#include <time.h>
#include <linux/fb.h>
#include <sys/ioctl.h>
#include <linux/kernel.h>

#include "mjson.h"
#include "sysinfo.h"
#include "../common/displayInterface.h"
#ifdef __SUNXI_DISPLAY2__
#include "../common/sunxi_display_v2.h"
#else
#include "../common/sunxi_display_v1.h"
#endif

struct test_smart_color_info {
	int screen_id;
	int enable;
	int bright;
	int contrast;
	int saturation;
	int hue;
	int mode;
	int width, height;
	luapi_disp_window window;
	int dispfh;
};

struct test_smart_color_info test_info;

int parse_cmdline(struct test_smart_color_info *p)
{

	p->screen_id = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/screen_id");
	if (p->screen_id != -1) {
		printf("ID of the screen_id is %d \n", p->screen_id);
	} else {
		p->screen_id = 0;
		printf("get the screen id failed, use the default id 0\n");
	}

	p->enable = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/enhance_enable");
	if (p->enable != -1) {
		printf("enhance_enable is %d \n", p->enable);
	} else {
		p->enable = 1;
		printf("get the enhance_enable failed, use the default id 1\n");
	}

	p->mode = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/enhance_mode");
	if (p->mode != -1) {
		printf("enhance_mode is %d \n", p->mode);
	} else {
		p->mode = 8;
		printf("get the enhance_mode failed, use the default id 8\n");
	}

	p->bright = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/bright");
	if (p->bright != -1) {
		printf("bright is %d \n", p->bright);
	} else {
		p->bright = 50;
		printf("get the bright failed, use the default id 50\n");
	}

	p->saturation = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/saturation");
	if (p->saturation != -1) {
		printf("saturation is %d \n", p->saturation);
	} else {
		p->saturation = 50;
		printf("get the saturation failed, use the default id 50\n");
	}

	p->contrast = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/contrast");
	if (p->contrast != -1) {
		printf("contrast is %d \n", p->contrast);
	} else {
		p->contrast = 50;
		printf("get the contrast failed, use the default id 50\n");
	}

	p->hue = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/hue");
	if (p->hue != -1) {
		printf("hue is %d \n", p->hue);
	} else {
		p->hue = 50;
		printf("get the hue failed, use the default id 50\n");
	}

	p->window.x = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/window_x");
	if (p->window.x != -1) {
		printf("window_x is %d \n", p->window.x);
	} else {
		p->window.x = 0;
		printf("get the window_x failed, use the default id 0\n");
	}

	p->window.y = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/window_y");
	if (p->window.y != -1) {
		printf("window_y is %d \n", p->window.y);
	} else {
		p->window.y = 0;
		printf("get the window_y failed, use the default id 0\n");
	}

	p->window.width = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/window_width");
	if (p->window.width != -1) {
		printf("window_width is %u \n", p->window.width);
	} else {
		p->window.width = 800;
		printf("get the window_width failed, use the default id 800\n");
	}

	p->window.height = mjson_fetch_int(
			"/base/production/displaytester/smartcolortester/window_height");
	if (p->window.height != -1) {
		printf("window_height is %u \n", p->window.height);
	} else {
		p->window.height = 1280;
		printf("get the window_height failed, use the default id 1280\n");
	}

	/*printf(
	 "example : ./disp_smart_color -ch 0 -en 1 -mode 8 -enhance 50 50 50 50 -window 0 0 1280 800\n");
	 printf(
	 "example : mode: standard(7), vivi(8);      enhance:bright,saturation,contrast,hue\n");*/
	return 0;
}

int main(int argc, char *argv[])
{
	int i, n;
	int fb_width, fb_height;
	int sw, sh;
	int ret;

	memset(&test_info, 0, sizeof(struct test_smart_color_info));
	parse_cmdline(&test_info);

	if ((test_info.dispfh = open("/dev/disp", O_RDWR)) == -1) {
		printf("open display device fail!\n");
		return -1;
	}

	test_info.width = DispGetScrWidth(test_info.dispfh, test_info.screen_id);
	test_info.height = DispGetScrHeight(test_info.dispfh, test_info.screen_id);
	if ((0 == test_info.window.width) || (0 == test_info.window.height)) {
		test_info.window.width = test_info.width;
		test_info.window.height = test_info.height;
	}

	if (!test_info.enable) {
		if (DispSetEnhanceEnable(test_info.dispfh, test_info.screen_id, 0)
				< 0) {
			printf("DispSetEnhanceEnable FAILED\n");
		} else {
			printf("DispSetEnhanceEnable SUCCESS\n");
		}
		return 0;
	}

	if (DispSetEnhanceMode(test_info.dispfh, test_info.screen_id,
			test_info.mode) < 0) {
		printf("DispSetEnhanceMode FAILED\n");
	} else {
		printf("DispSetEnhanceMode SUCCESS\n");
	}

	ret = DispSetEnhanceBright(test_info.dispfh, test_info.screen_id,
			test_info.bright);
	if (ret > 0) {
		printf("DispSetEnhanceBright SUCCESS\n");
	} else if (ret == DISP_NOT_SUPPORT) {
		printf("DispSetEnhanceBright NOT SUPPORT\n");
	} else {
		printf("DispSetEnhanceBright FAILED\n");
	}

	ret = DispSetEnhanceSatuation(test_info.dispfh, test_info.screen_id,
			test_info.saturation);
	if (ret > 0) {
		printf("DispSetEnhanceSatuation SUCCESS\n");
	} else if (ret == DISP_NOT_SUPPORT) {
		printf("DispSetEnhanceSatuation NOT SUPPORT\n");
	} else {
		printf("DispSetEnhanceSatuation FAILED\n");
	}

	ret = DispSetEnhanceContrast(test_info.dispfh, test_info.screen_id,
			test_info.contrast);
	if (ret > 0) {
		printf("DispSetEnhanceContrast SUCCESS\n");
	} else if (ret == DISP_NOT_SUPPORT) {
		printf("DispSetEnhanceContrast NOT SUPPORT\n");
	} else {
		printf("DispSetEnhanceContrast FAILED\n");
	}

	ret = DispSetEnhanceHue(test_info.dispfh, test_info.screen_id,
			test_info.hue);
	if (ret > 0) {
		printf("DispSetEnhanceHue SUCCESS\n");
	} else if (ret == DISP_NOT_SUPPORT) {
		printf("DispSetEnhanceHue NOT SUPPORT\n");
	} else {
		printf("DispSetEnhanceHue FAILED\n");
	}

	ret = DispSetEnhanceWindow(test_info.dispfh, test_info.screen_id,
			test_info.window);
	if (ret > 0) {
		printf("DispSetEnhanceWindow SUCCESS\n");
	} else if (ret == DISP_NOT_SUPPORT) {
		printf("DispSetEnhanceWindow NOT SUPPORT\n");
	} else {
		printf("DispSetEnhanceWindow FAILED\n");
	}

	ret = DispGetEnhanceEnable(test_info.dispfh, test_info.screen_id);
	if (ret > 0) {
		printf("DispGetEnhanceEnable SUCCESS ret = %d\n", ret);
	} else if (ret == DISP_NOT_SUPPORT) {
		printf("DispGetEnhanceEnable NOT SUPPORT\n");
	} else {
		printf("DispGetEnhanceEnable FAILED\n");
	}

	if (DispSetEnhanceEnable(test_info.dispfh, test_info.screen_id, 1) < 0) {
		printf("DispSetEnhanceEnable FAILED\n");
		close(test_info.dispfh);
		return -1;
	} else {
		printf("DispSetEnhanceEnable SUCCESS\n");
	}

	printf("GOOD, smart color test success.\n");

	close(test_info.dispfh);
	return 0;
}
