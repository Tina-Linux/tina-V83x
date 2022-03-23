
#include "sunxi_vop.h"
#include "../src/sunxi_display2.h"

#define APP_DEBUG	printf
#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static char *format_str[] = {
[SUNXI_VIDEO_FORMAT_UNKNOWN] = "< unknown/not supported video format.",
 [SUNXI_VIDEO_FORMAT_NTSC] = "< NTSC",
 [SUNXI_VIDEO_FORMAT_PAL] = "< PAL",
 [SUNXI_VIDEO_FORMAT_480P] = "< NTSC Progressive (27Mhz).",
 [SUNXI_VIDEO_FORMAT_576P] = "< HD PAL Progressive 50hz for Australia.",
 [SUNXI_VIDEO_FORMAT_1080I] = "< HD 1080 Interlaced.",
 [SUNXI_VIDEO_FORMAT_1080I_50HZ] = "< European 50hz HD 1080.",
 [SUNXI_VIDEO_FORMAT_1080P_24HZ] = "< HD 1080 Progressive, 24hz.",
 [SUNXI_VIDEO_FORMAT_1080P_25HZ] = "< HD 1080 Progressive, 25hz.",
 [SUNXI_VIDEO_FORMAT_1080P_30HZ] = "< HD 1080 Progressive, 30hz.",
 [SUNXI_VIDEO_FORMAT_1080P_50HZ] = "< HD 1080 Progressive, 50hz.",
 [SUNXI_VIDEO_FORMAT_1080P_60HZ] = "< HD 1080 Progressive, 60hz.",
 [SUNXI_VIDEO_FORMAT_1080P] = "< HD 1080 Progressive, 60hz.",
 [SUNXI_VIDEO_FORMAT_720P] = "< HD 720 Progressive.",
 [SUNXI_VIDEO_FORMAT_720P_24HZ] = "< HD 720p 24hz.",
 [SUNXI_VIDEO_FORMAT_720P_25HZ] = "< HD 720p 25hz.",
 [SUNXI_VIDEO_FORMAT_720P_30HZ] = "< HD 720p 30hz.",
 [SUNXI_VIDEO_FORMAT_720P_50HZ] = "< HD 720p 50hz for Australia.",
 [SUNXI_VIDEO_FORMAT_3840x2160P_24HZ] = "< UHD 3840x2160 24Hz.",
 [SUNXI_VIDEO_FORMAT_3840x2160P_25HZ] = "< UHD 3840x2160 25Hz.",
 [SUNXI_VIDEO_FORMAT_3840x2160P_30HZ] = "< UHD 3840x2160 30Hz.",
 [SUNXI_VIDEO_FORMAT_3840x2160P_50HZ] = "< UHD 3840x2160 50Hz.",
 [SUNXI_VIDEO_FORMAT_3840x2160P_60HZ] = "< UHD 3840x2160 60Hz.",
 [SUNXI_VIDEO_FORMAT_4096x2160P_24HZ] = "< UHD 4096x2160 24Hz.",
 [SUNXI_VIDEO_FORMAT_4096x2160P_25HZ] = "< UHD 4096x2160 25Hz.",
 [SUNXI_VIDEO_FORMAT_4096x2160P_30HZ] = "< UHD 4096x2160 30Hz.",
 [SUNXI_VIDEO_FORMAT_4096x2160P_50HZ] = "< UHD 4096x2160 50Hz.",
 [SUNXI_VIDEO_FORMAT_4096x2160P_60HZ] = "< UHD 4096x2160 60Hz.",
 [SUNXI_VIDEO_FORMAT_3D_1080P_24HZ] = "< HD 3D 1080P 24Hz, 2750 sample per line, SMPTE 274M-1998."

};


static SetWindowsSize(SUNXIVOPDevice_e device, SUNXIVOPVideoFormat_e format)
{
	signed int fd;
	unsigned int screen_id;
	unsigned int args[4];
	struct disp_layer_config2 config;

	memset(&config, 0, sizeof(struct disp_layer_config2));
	fd = open("/dev/disp", O_RDWR);
	config.layer_id = 0;
	if (device == SUNXI_OUTPUT_DEVICE_HDMI) {
		screen_id = 0;
		config.channel = 1;
	} else {
		screen_id = 1;
		config.channel = 1;
	}
	args[0] = screen_id;
	args[1] = (unsigned long)&config;
	args[2] = (unsigned long)1;
	ioctl(fd, DISP_LAYER_GET_CONFIG2, (void *)args);

	config.info.screen_win.x = 0;
	config.info.screen_win.y = 0;
	if (format >= SUNXI_VIDEO_FORMAT_720P && format <= SUNXI_VIDEO_FORMAT_720P_50HZ) {
		config.info.screen_win.width = 1280;
		config.info.screen_win.height = 720;
	} else if (format >= SUNXI_VIDEO_FORMAT_1080I && format <= SUNXI_VIDEO_FORMAT_1080P) {
		config.info.screen_win.width = 1920;
		config.info.screen_win.height = 1080;
	} else if (format >= SUNXI_VIDEO_FORMAT_3840x2160P_24HZ && format <= SUNXI_VIDEO_FORMAT_3840x2160P_60HZ) {
		config.info.screen_win.width = 3840;
		config.info.screen_win.height = 2160;
	} else if (format >= SUNXI_VIDEO_FORMAT_4096x2160P_24HZ && format <= SUNXI_VIDEO_FORMAT_4096x2160P_60HZ) {
		config.info.screen_win.width = 4096;
		config.info.screen_win.height = 2160;
	} else if (format == SUNXI_VIDEO_FORMAT_480P || format == SUNXI_VIDEO_FORMAT_NTSC) {
		config.info.screen_win.width = 720;
		config.info.screen_win.height = 480;
	} else if (format == SUNXI_VIDEO_FORMAT_576P || format == SUNXI_VIDEO_FORMAT_PAL) {
		config.info.screen_win.width = 720;
		config.info.screen_win.height = 576;
	}
	args[0] = screen_id;
	args[1] = (unsigned long)&config;
	args[2] = (unsigned long)1;
	ioctl(fd, DISP_LAYER_SET_CONFIG2, (void *)args);
	close(fd);
}

void *pthread_handle_func(void *pv)
{
	SUNXIVOPDevice_e device = SUNXI_OUTPUT_DEVICE_CVBS;

	while(1) {/* nead ac200 support */
	#if 0
		SUNXIVOPEnable(device);

		SUNXIVOPSetVideoFormat(device, SUNXI_VIDEO_FORMAT_NTSC);
		SetWindowsSize(device, SUNXI_VIDEO_FORMAT_NTSC);
		sleep(5);
		SUNXIVOPSetVideoFormat(device, SUNXI_VIDEO_FORMAT_PAL);
		SetWindowsSize(device, SUNXI_VIDEO_FORMAT_PAL);
		sleep(5);

		SUNXIVOPDisable(device);
	#endif
		sleep(1);
	}
}

static void print_help() {
    
    printf("voptest [device] [video format]\n");
    printf("formats:\n");
    int i, len;
    len = ARRAY_SIZE(format_str);
    for (i = 0; i < len; i++)
        printf("%d\t:\t%s\n", i, format_str[i]);

}

int main(int argc, char **argv)
{
	int i, j;
	SUNXIVOPHDMIMonitoStatus status;
	SUNXIVOPDevice_e device = SUNXI_OUTPUT_DEVICE_HDMI;
    int format;
	char ret = 0;
	pthread_t TreadID;


    if (argc == 2 && !strcmp(argv[1], "-h")) {
        print_help();
        return 0;
    }
    else if (argc == 3) {
		SUNXIVOPSetColorSpace(device, SUNXI_COLOR_SPACE_YCBCR_444);
        device = atoi(argv[1]);
        format = atoi(argv[2]);
		SUNXIVOPEnable(device);
        SUNXIVOPSetVideoFormat(device, format);
        return 0;
    }

	/* loop switch cvbs disp-mode */
	pthread_create(&TreadID, NULL, pthread_handle_func, NULL);

	/* loop switch hdmi disp-mode */
	while(1) {
		SUNXIVOPSetColorSpace(device, SUNXI_COLOR_SPACE_YCBCR_444);
		SUNXIVOPEnable(device);
		for (i = SUNXI_VIDEO_FORMAT_480P; i < SUNXI_VIDEO_FORMAT_4096x2160P_60HZ; i++) {
			APP_DEBUG("[App]Demo is start!\n");
			if (SUNXIVOPSetVideoFormat(device, i) == -1) {
				APP_DEBUG("[App]not support video format %d\n", i);
				APP_DEBUG("[App]Demo is end\n\n");
				continue;
			}
			APP_DEBUG("[App]VideoFormat=%d\n", i);
			APP_DEBUG("[App]SUNXIVOPEnable\n");

			SetWindowsSize(device, i);
			SUNXIVOPGetHDMIMonitorStatus(&status);
			sleep(3);
			APP_DEBUG("[App]SUNXIVOPDisable\n");
			APP_DEBUG("[App]Demo is end!\n\n");
		}
		SUNXIVOPDisable(device);
		sleep(3);
	}

	return 0;
}

