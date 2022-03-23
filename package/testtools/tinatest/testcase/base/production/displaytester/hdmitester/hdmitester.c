#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/mman.h>
#include <pthread.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <linux/fb.h>
#include <stdlib.h>

#include <alsa/asoundlib.h>
#include "mjson.h"
#include "sysinfo.h"
#include "interact.h"
#include "../common/displayInterface.h"
#ifdef __SUNXI_DISPLAY2__
#include "../common/sunxi_display_v2.h"
#else
#include "../common/sunxi_display_v1.h"
#endif

static int disphd;
int screen_id;
struct fb_var_screeninfo vinfo;
struct fb_fix_screeninfo finfo;
char *frameBuffer = 0;

void parse_configs(void)
{
	screen_id = mjson_fetch_int(
			"/base/production/displaytester/hdmitester/screen_id");
	if (screen_id != -1) {
		printf("ID of the layer is %d \n", screen_id);
	} else {
		screen_id = 0;
		printf("get the screen id failed, use the default id 0\n");
	}
}

/*8alpha+8reds+8greens+8blues*/
static void drawRect_rgb32(int x0, int y0, int width, int height, int color)
{
	const int bytesPerPixel = 4;
	const int stride = finfo.line_length / bytesPerPixel;

	int *dest = (int *) (frameBuffer) + (y0 + vinfo.yoffset) * stride
			+ (x0 + vinfo.xoffset);

	int x, y;
	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			dest[x] = color;
		}
		dest += stride;
	}
}

/*5reds+6greens+5blues*/
static void drawRect_rgb16(int x0, int y0, int width, int height, int color)
{
	const int bytesPerPixel = 2;
	const int stride = finfo.line_length / bytesPerPixel;
	const int red = (color & 0xff0000) >> (16 + 3);
	const int green = (color & 0xff00) >> (8 + 2);
	const int blue = (color & 0xff) >> 3;
	const short color16 = blue | (green << 5) | (red << (5 + 6));

	short *dest = (short *) (frameBuffer) + (y0 + vinfo.yoffset) * stride
			+ (x0 + vinfo.xoffset);

	int x, y;
	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			dest[x] = color16;
		}
		dest += stride;
	}
}

/*5reds+5greens+5blues*/
static void drawRect_rgb15(int x0, int y0, int width, int height, int color)
{
	const int bytesPerPixel = 2;
	const int stride = finfo.line_length / bytesPerPixel;
	const int red = (color & 0xff0000) >> (16 + 3);
	const int green = (color & 0xff00) >> (8 + 3);
	const int blue = (color & 0xff) >> 3;
	const short color15 = blue | (green << 5) | (red << (5 + 5)) | 0x8000;

	short *dest = (short *) (frameBuffer) + (y0 + vinfo.yoffset) * stride
			+ (x0 + vinfo.xoffset);

	int x, y;
	for (y = 0; y < height; ++y) {
		for (x = 0; x < width; ++x) {
			dest[x] = color15;
		}
		dest += stride;
	}
}

static void drawRect(int x0, int y0, int width, int height, int color)
{
	switch (vinfo.bits_per_pixel) {
	case 32:
		drawRect_rgb32(x0, y0, width, height, color);
		break;
	case 16:
		drawRect_rgb16(x0, y0, width, height, color);
		break;
	case 15:
		drawRect_rgb15(x0, y0, width, height, color);
		break;
	default:
		printf("Warning: drawRect() not implemented for color depth%i\n",
				vinfo.bits_per_pixel);
		break;
	}
}

static int fb_show(void)
{
	const char *devfile = "/dev/fb0";
	long int screensize = 0;
	int fbFd = 0;

	/* Open the file for reading and writing */
	fbFd = open(devfile, O_RDWR);
	if (fbFd == -1) {
		perror("Error: cannot open framebuffer device");
		return -1;
	}

	/*Get finfo information*/
	if (ioctl(fbFd, FBIOGET_FSCREENINFO, &finfo) == -1) {
		perror("Error reading fixed information");
		return -1;
	}

	/*Get vinfo information*/
	if (ioctl(fbFd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
		perror("Error reading variable information");
		return -1;
	}

	/* Figure out the size of the screen in bytes */
	screensize = finfo.smem_len;

	/* Map the device to memory */
	frameBuffer = (char *) mmap(0, screensize, PROT_READ | PROT_WRITE,
	MAP_SHARED, fbFd, 0);
	if (frameBuffer == MAP_FAILED) {
		perror("Error: Failed to map framebuffer device to memory");
		return -1;
	}

	printf("Will draw 7 rectangles on the screen\n");
	drawRect(0, 0, vinfo.xres, vinfo.yres / 2, 0xffff0000);
	usleep(500000);
	drawRect(0, vinfo.yres / 2, vinfo.xres, vinfo.yres / 2, 0xff00ff00);
	usleep(500000);
	drawRect(0, 0, vinfo.xres, vinfo.yres / 2, 0xff0000ff);
	usleep(500000);
	drawRect(0, vinfo.yres / 2, vinfo.xres, vinfo.yres / 2, 0xffff00ff);
	usleep(500000);
	drawRect(0, 0, vinfo.xres, vinfo.yres / 2, 0xff223366);
	usleep(500000);
	drawRect(0, vinfo.yres / 2, vinfo.xres, vinfo.yres / 2, 0xffff6600);
	usleep(500000);
	drawRect(0, 0, vinfo.xres, vinfo.yres / 2, 0xff35ff69);
	usleep(500000);

	memset(frameBuffer, 0, screensize);
	printf("Done.\n");

	munmap(frameBuffer, screensize);

	close(fbFd);
	return 0;
}

int main(int argc, char *argv[])
{
	unsigned int fd_hdmi;
	int ret;
	unsigned int output_type;
	char val;

	parse_configs();
	disphd = open("/dev/disp", O_RDWR);
	if (disphd == -1) {
		printf("hdmitester: open /dev/disp failed(%s)\n", strerror(errno));
		goto err;
	}

	/*screen0 output type*/
	output_type = DispGetOutPutType(disphd, screen_id);
	if (output_type == DISP_OUTPUT_TYPE_LCD)
		screen_id = 1;
	else
		screen_id = 0;
	printf("output_type = %u, screen id= %d\n", output_type, screen_id);

	/*hotplug detect*/
	fd_hdmi = open("/sys/class/hdmi/hdmi/attr/state", O_RDONLY);
	if (fd_hdmi < 0) {
		printf("hdmitester: open /sys/class/hdmi/hdmi/attr/state failed\n");
	}
	ret = read(fd_hdmi, &val, 1);
	close(fd_hdmi);

	if (val == 'n') {
		printf("-------hotplug in---------\n");

		/*device switch to HDMI*/
		if (DispDeviceSwitch(disphd, screen_id, DISP_OUTPUT_TYPE_HDMI,
				DISP_TV_MOD_1080P_60HZ, 1) == 0) {
			close(disphd);
			printf("DispDeviceSwitch SUCCESS\n");
		} else {
			printf("DispDeviceSwitch FAILED\n");
			goto err;
		}

		/*ret = init_layer();*/
		ret = fb_show();
		if (ret < 0) {
			printf("framebuffer show failed!\n");
			goto err;
		}
	} else {
		printf("-------hotplug out--------\n");
		goto err;
	}

	ret = ttrue("The color squares appear on the screen?");
	if (ret == 1) {
	    printf("GOOD, hdmi test success.\n");
		return 0;
	} else {
	    printf("ERROR, hdmi test failed.\n");
	    return -1;
	}

	err:
	/* error */
	close(fd_hdmi);
	close(disphd);
	printf("ERROR, hdmi test failed.\n");
	return -1;
}
