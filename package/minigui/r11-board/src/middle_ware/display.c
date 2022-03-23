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
#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <stdbool.h>
#include "display.h"
#include "common.h"

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


#define DISP_CLAR(x)  memset(x, 0, sizeof(x))

#define DISP_DEV_NAME ("/dev/disp")

int display_get_lcd_rect(disp_rectsz *rect)
{
	int disp_fd = 0;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];
	int ret;
	disp_fd = open(DISP_DEV_NAME, O_RDWR);
	if(disp_fd == -1){
		printf("%s:open disp handle is not exist!\r\n", __func__);
		return -1;
	}
	DISP_CLAR(ioctlParam);
	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = 0;
	rect->width = ioctl(disp_fd, DISP_GET_SCN_WIDTH, ioctlParam);
	DISP_CLAR(ioctlParam);
	ioctlParam[DISP_IOCTL_ARG_OUT_SRC] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[DISP_IOCTL_ARG_OUT_LAYER] = 0;
	rect->height = ioctl(disp_fd, DISP_GET_SCN_HEIGHT, ioctlParam);
	close(disp_fd);
	return TINAL_OK;
}

int display_lcd_set_brightness(int val)
{
	int disp_fd = 0;
	int ret;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];
	if(val<0)val = 0;
	if(val>100)val = 100;
	disp_fd = open(DISP_DEV_NAME, O_RDWR);
	if(disp_fd == -1){
		printf("%s:open disp handle is not exist!\r\n", __func__);
		return -1;
	}
	DISP_CLAR(ioctlParam);
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = val;
	ret = ioctl(disp_fd, DISP_LCD_SET_BRIGHTNESS, (void*)ioctlParam);
	if(ret < 0){
		printf("%s: fail to open backlight!\r\n", __func__);
	}
	close(disp_fd);
	return TINAL_OK;
}

int display_lcd_backlight_onoff(bool onoff)
{
	int disp_fd = 0;
	int ret;
	unsigned int cmd;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];
	disp_fd = open(DISP_DEV_NAME, O_RDWR);
	if(disp_fd == -1){
		printf("%s:open disp handle is not exist!\r\n", __func__);
		return -1;
	}
	DISP_CLAR(ioctlParam);
	if(onoff == 1){
		cmd = DISP_LCD_BACKLIGHT_ENABLE;
	}else{
		cmd = DISP_LCD_BACKLIGHT_DISABLE;
	}
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = 0;
	ret = ioctl(disp_fd, cmd, (void*)ioctlParam);
	if(ret < 0){
		printf("%s: fail to open backlight!\r\n", __func__);
	}
	close(disp_fd);
	return TINAL_OK;
}

int display_lcd_onoff(bool onoff)
{
	int disp_fd = 0;
	int ret;
	unsigned int cmd;
	unsigned int ioctlParam[DISP_IOCTL_ARG_MAX];
	disp_fd = open(DISP_DEV_NAME, O_RDWR);
	if(disp_fd == -1){
		printf("%s:open disp handle is not exist!\r\n", __func__);
		return -1;
	}
	DISP_CLAR(ioctlParam);
	if(onoff == 1){
		cmd = DISP_LCD_ENABLE;
	}else{
		cmd = DISP_LCD_DISABLE;
	}
	ioctlParam[0] = DISP_OUT_SRC_SEL_LCD;
	ioctlParam[1] = 0;
	ret = ioctl(disp_fd, cmd, (void*)ioctlParam);
	if(ret < 0){
		printf("%s: fail to open backlight!\r\n", __func__);
	}
	close(disp_fd);
	return TINAL_OK;
}
