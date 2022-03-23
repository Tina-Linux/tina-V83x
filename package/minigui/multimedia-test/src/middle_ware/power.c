#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/statfs.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <string.h>
#include <signal.h>

#include "power.h"

int power_get_battery_level(void) {
	int level = -1;
	char pwr_val[5];
	int ival = 0;
	int pwr_fd;
	int ret;

	pwr_fd = open("/sys/devices/platform/lradc_battery/capacity", O_RDONLY);
	if (pwr_fd == -1) {
		/* printf("%s:open disp handle is not exist!\r\n", __func__); */
		return -1;
	}
	memset(pwr_val, 0, sizeof(pwr_val));
	ret = read(pwr_fd, pwr_val, 5);
	if (ret < 0) {
		printf("read power val failed!\n");
		level = -1;
		goto error_out;
	}
	/* printf("%s\n", pwr_val); */

	ival = atoi(pwr_val);

	switch (ival) {
	case 0:
		level = POWER_LEVEL_0;
		break;
	case 20:
		level = POWER_LEVEL_1;
		break;
	case 40:
		level = POWER_LEVEL_2;
		break;
	case 60:
		level = POWER_LEVEL_3;
		break;
	case 80:
		level = POWER_LEVEL_4;
		break;
	case 100:
		level = POWER_LEVEL_5;
		break;
	default:
		printf("not know the lev.\n");
		level = -1;
		break;
	}

	error_out:

	close(pwr_fd);

	return level;
}

int power_is_charging(void) {
	int pwr_type = -1;
	char val[2];
	int ival = 0;
	int pwr_fd;
	int ret;

	pwr_fd = open("/sys/devices/platform/lradc_battery/charging", O_RDONLY);
	if (pwr_fd == -1) {
		/* printf("%s:open disp handle is not exist!\r\n", __func__); */
		return -1;
	}

	memset(val, 0, sizeof(val));
	ret = read(pwr_fd, val, 1);
	if (ret < 0) {
		printf("read power val failed!\n");
		pwr_type = -1;
		goto error_out;
	}
	ival = atoi(val);
	switch (ival) {
	case 0:
		pwr_type = POWER_BAT_ONLY;
		break;
	case 1:
		pwr_type = POWER_CHARGER_LINK;
		break;
	case 2:
		pwr_type = POWER_PC_LINK;
		break;
	default:
		printf("not know the type.\n");
		pwr_type = -1;
		break;
	}

	error_out:

	close(pwr_fd);

	return pwr_type;
}

/*1 Low battery 0 Normal battery*/
int power_is_low(void) {
	int level = 0;

	level = power_get_battery_level();
	if (level < POWER_LEVEL_2)
		return 1;
	else
		return 0;
}
