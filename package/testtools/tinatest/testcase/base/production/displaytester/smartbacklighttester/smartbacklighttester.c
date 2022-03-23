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
#include "sysinfo.h"
#include "../common/displayInterface.h"
#ifdef __SUNXI_DISPLAY2__
#include "../common/sunxi_display_v2.h"
#else
#include "../common/sunxi_display_v1.h"
#endif

struct test_smart_backlight_info {
	int screen_id;
	luapi_disp_window window;
};

luapi_disp_window window;

struct test_smart_backlight_info smbl_info;

void parse_configs(void)
{
	int sid;

	sid = mjson_fetch_int(
			"/base/production/displaytester/smartbacklighttester/screen_id");
	if (sid != -1) {
		smbl_info.screen_id = sid;
		printf("id of the screen is %d \n", sid);
	} else {
		smbl_info.screen_id = 0;
		printf("get the screen id failed, use the default id 0\n");
	}
}

int main(int argc, int *argv[])
{
	int disphd;
	int ret;
	int width;
	int height;
	int setwin;
	int enable;
	int disable;

	memset(&smbl_info, 0, sizeof(struct test_smart_backlight_info));
	disphd = open("/dev/disp", O_RDWR);
	if (disphd == -1) {
		printf("open display device faild ( %s )\n", strerror(errno));
		return -1;
	}

	parse_configs();

	smbl_info.window.x = 0;
	smbl_info.window.y = 0;
	width = DispGetScrWidth(disphd, smbl_info.screen_id);
	if (width < 0) {
		width = 300;
		printf("DispGetScrWidth FAILED\n");
	}
	height = DispGetScrHeight(disphd, smbl_info.screen_id);
	if (height < 0) {
		height = 300;
		printf("DispGetScrHeight FAILED\n");
	}
	smbl_info.window.width = width / 2;
	smbl_info.window.height = height;
	printf("the width of screen is %d\n", width);
	printf("the height of screen is %d\n", height);

	window.x = 0;
	window.y = 0;
	window.width = width;
	window.height = height;

	setwin = DispSetSMBLWindow(disphd, smbl_info.screen_id, window);

	if (setwin < 0) {
		printf("DispSetSMBLWindow FAILED\n");
		goto err;
	} else {
		printf("DispSetSMBLWindow SUCCESS\n");
	}

	enable = DispSetSMBLEnable(disphd, smbl_info.screen_id, 1);
	if (enable < 0) {
		printf("DispSetSMBLEnable enable FAILED\n");
		goto err;
	} else {
		printf("DispSetSMBLEnable enable SUCCESS\n");
	}

	sleep(3);

	disable = DispSetSMBLEnable(disphd, smbl_info.screen_id, 0);
	if (disable < 0) {
		printf("DispSetSMBLEnable disable FAILED\n");
		goto err;
	} else {
		printf("DispSetSMBLEnable disable SUCCESS\n");
	}

	close(disphd);
	printf("GOOD, smart back light test success.\n");
	return 0;

	err: close(disphd);
	printf("GOOD, smart back light test failed.\n");
	return -1;
}
