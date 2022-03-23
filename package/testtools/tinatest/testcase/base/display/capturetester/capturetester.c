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
#include <linux/kernel.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <sys/ioctl.h>

#include "mjson.h"
#include "sysinfo.h"
#include "interact.h"

#include "../../production/displaytester/common/displayInterface.h"
#ifdef __SUNXI_DISPLAY2__
#include "../../production/displaytester/common/sunxi_display_v2.h"
#else
#include "../../production/displaytester/common/sunxi_display_v1.h"
#endif

struct test_capture_info {
	luapi_capture_info capture_info;
	int screen_id;
	int layer_id;
	int channel_id;
	int layer_num;
	int mem_id;
	int clear;
	char filename[3][32];
	int dispfh;
};

#pragma pack(push, 1)
typedef struct BITMAPFILEHEADER {
	u_int16_t bfType;
	u_int32_t bfSize;
	u_int16_t bfReserved1;
	u_int16_t bfReserved2;
	u_int32_t bfOffBits;
} BITMAPFILEHEADER;

typedef struct BITMAPINFOHEADER {
	u_int32_t biSize;
	u_int32_t biWidth;
	u_int32_t biHeight;
	u_int16_t biPlanes;
	u_int16_t biBitCount;
	u_int32_t biCompression;
	u_int32_t biSizeImage;
	u_int32_t biXPelsPerMeter;
	u_int32_t biYPelsPerMeter;
	u_int32_t biClrUsed;
	u_int32_t biClrImportant;
} BITMAPINFODEADER;
#pragma pack(pop)

struct test_capture_info test_info;

int main(int argc, char **argv)
{
	int rv;
	int fps = 0;
	unsigned int bpp = 32;
	unsigned int align_line;
	unsigned int mem_size;
	FILE *fp;
	unsigned long long phy_adr;
	unsigned char *vir_adr = NULL;
	int i;

	luapi_layer_config config;
	int screenX;
	int screenY;
	unsigned int screenW;
	unsigned int screenH;
	unsigned int format;
	unsigned int fb_num = 1;
	test_info.mem_id = 0;

	memset(&test_info, 0, sizeof(struct test_capture_info));
	memset(&config, 0, sizeof(struct luapi_layer_config));

	test_info.screen_id = mjson_fetch_int(
			"/base/display/capturetester/screen_id");
	if (test_info.screen_id != -1) {
		printf("ID of the screen is %d \n", test_info.screen_id);
	} else {
		test_info.screen_id = 0;
		printf("get the screen id failed, use the default id 0\n");
	}

	test_info.layer_id = mjson_fetch_int(
			"/base/display/capturetester/layer_id");
	if (test_info.layer_id != -1) {
		printf("ID of the layer id is %d \n", test_info.layer_id);
	} else {
		test_info.layer_id = 0;
		printf("get the layer id failed, use the default id 0\n");
	}

	test_info.channel_id = mjson_fetch_int(
			"/base/display/capturetester/channel_id");
	if (test_info.channel_id != -1) {
		printf("ID of the channel id is %d \n", test_info.channel_id);
	} else {
		test_info.channel_id = 1;
		printf("get the channel id failed, use the default id 1\n");
	}

	test_info.layer_num = mjson_fetch_int(
			"/base/display/capturetester/layer_num");
	if (test_info.layer_num != -1) {
		printf("ID of the layer num is %d \n", test_info.layer_num);
	} else {
		test_info.layer_num = 1;
		printf("get the layer num failed, use the default id 1\n");
	}

	/*	strcpy(test_info.filename[0], "capture0.bmp");
	 strcpy(test_info.filename[1], "capture1.bmp");
	 strcpy(test_info.filename[2], "capture2.bmp");*/

	if ((test_info.dispfh = open("/dev/disp", O_RDWR)) == -1) {
		printf("open display device fail!\n");
		return -1;
	}
	/*get the layer information*/
	if (DispGetLayerConfig(test_info.dispfh, test_info.screen_id,
			test_info.layer_id, test_info.channel_id, test_info.layer_num,
			&config) < 0) {
		printf("Get layer information failed,exit\n");
		close(test_info.dispfh);
		return -1;
	}
#ifdef __SUNXI_DISPLAY2__
	screenX = config.layerConfig.info.screen_win.x;
	screenY = config.layerConfig.info.screen_win.y;
	screenW = config.layerConfig.info.screen_win.width;
	screenH = config.layerConfig.info.screen_win.height;
	format = config.layerConfig.info.fb.format;
#else
	screenX = config.layerConfig.screen_win.x;
	screenY = config.layerConfig.screen_win.y;
	screenW = config.layerConfig.screen_win.width;
	screenH = config.layerConfig.screen_win.height;
	format = config.layerConfig.fb.format;
#endif
	printf("screen is [ %d,%d,%u,%u ]\n", screenX, screenY, screenW, screenH);

	/*set capture information*/
#ifdef __SUNXI_DISPLAY2__
	test_info.capture_info.captureInfo.window.x = screenX;
	test_info.capture_info.captureInfo.window.y = screenY;
	test_info.capture_info.captureInfo.window.height = screenH;
	test_info.capture_info.captureInfo.window.width = screenW;
	test_info.capture_info.captureInfo.out_frame.format = format;
	test_info.capture_info.captureInfo.out_frame.crop.x = screenX;
	test_info.capture_info.captureInfo.out_frame.crop.y = screenY;
	test_info.capture_info.captureInfo.out_frame.crop.width = screenW;
	test_info.capture_info.captureInfo.out_frame.crop.height = screenH;
#else
	test_info.capture_info.captureInfo.mode = 0;
	test_info.capture_info.captureInfo.fps = 0;
	test_info.capture_info.captureInfo.buffer_num = 1;
	test_info.capture_info.captureInfo.screen_size.width = screenW;
	test_info.capture_info.captureInfo.screen_size.height = screenH;
	test_info.capture_info.captureInfo.capture_window.x = screenX;
	test_info.capture_info.captureInfo.capture_window.y = screenY;
	test_info.capture_info.captureInfo.capture_window.height = screenH;
	test_info.capture_info.captureInfo.capture_window.width = screenW;
	test_info.capture_info.captureInfo.output_fb[0].format = format;
	test_info.capture_info.captureInfo.output_fb[0].src_win.x = screenX;
	test_info.capture_info.captureInfo.output_fb[0].src_win.y = screenY;
	test_info.capture_info.captureInfo.output_fb[0].src_win.width = screenW;
	test_info.capture_info.captureInfo.output_fb[0].src_win.height = screenH;
#endif

	align_line = ((bpp * screenW + 31) >> 5) << 2;
	mem_size = align_line * screenH * fb_num;
	printf("align_line is %u,mem_size is %u\n", align_line, mem_size);

	/*request memory*/
	if (DispMemRequest(test_info.dispfh, test_info.mem_id, mem_size) < 0) {
		printf("request memory failed!\n");
		close(test_info.dispfh);
		return -1;
	}

	/*get phy address*/
	phy_adr = DispMemGetAdrress(test_info.dispfh, test_info.mem_id);
	if (phy_adr < 0) {
		printf("get memory kernel address failed!\n");
		DispMemRelease(test_info.dispfh, test_info.mem_id);
		close(test_info.dispfh);
		return -1;
	}

	printf("phy address is %llx\n", phy_adr);

	/*mmap memery*/
	vir_adr = mmap(NULL, mem_size, PROT_READ, MAP_SHARED, test_info.dispfh,
			phy_adr);
	if (vir_adr == NULL) {
		printf("mmap memory failed!\n");
		DispMemRelease(test_info.dispfh, test_info.mem_id);
		close(test_info.dispfh);
		return -1;
	}

	for (i = 0; i < fb_num; i++) {
#ifdef __SUNXI_DISPLAY2__
		test_info.capture_info.captureInfo.out_frame.addr[i] =
				(unsigned int) phy_adr + i * mem_size / fb_num;
		test_info.capture_info.captureInfo.out_frame.size[i].width = screenW;
		test_info.capture_info.captureInfo.out_frame.size[i].height = screenH;
#else
		test_info.capture_info.captureInfo.output_fb[i].addr[0] =
				(unsigned int) phy_adr + i * mem_size / fb_num;
		test_info.capture_info.captureInfo.output_fb[i].size.width = screenW;
		test_info.capture_info.captureInfo.output_fb[i].size.height = screenH;
		test_info.capture_info.captureInfo.output_fb[i].format = format;
#endif
	}

	if (DispCaptureSatrt(test_info.dispfh, test_info.screen_id,
			&test_info.capture_info) < 0) {
		printf("capture start failed!\n");
		munmap(vir_adr, mem_size);
		DispMemRelease(test_info.dispfh, test_info.mem_id);
		close(test_info.dispfh);
		return -1;
	}

#ifdef __SUNXI_DISPLAY2__
	printf("format[ %d ]  ",
			test_info.capture_info.captureInfo.out_frame.format);
	printf("crop[ %d,%d,%u,%u ]\n",
			test_info.capture_info.captureInfo.out_frame.crop.x,
			test_info.capture_info.captureInfo.out_frame.crop.y,
			test_info.capture_info.captureInfo.out_frame.crop.width,
			test_info.capture_info.captureInfo.out_frame.crop.height);
	for (i = 0; i < 3; i++) {
		printf("size[%d] [ %u,%u ]  ", i,
				test_info.capture_info.captureInfo.out_frame.size[i].width,
				test_info.capture_info.captureInfo.out_frame.size[i].height);
		printf("addr[%d] ( %llx )\n", i,
				test_info.capture_info.captureInfo.out_frame.addr[i]);
	}
#else
	printf("format[ %d ]  ",
			test_info.capture_info.captureInfo.output_fb[0].format);
	printf("crop[ %d,%d,%u,%u ]\n",
			test_info.capture_info.captureInfo.output_fb[0].src_win.x,
			test_info.capture_info.captureInfo.output_fb[0].src_win.y,
			test_info.capture_info.captureInfo.output_fb[0].src_win.width,
			test_info.capture_info.captureInfo.output_fb[0].src_win.height);
	for (i = 0; i < 3; i++) {
		printf("size[%d] [ %u,%u ]  ", i,
				test_info.capture_info.captureInfo.output_fb[i].size.width,
				test_info.capture_info.captureInfo.output_fb[i].size.height);
		printf("addr[%d] ( %u )\n", i,
				test_info.capture_info.captureInfo.output_fb[i].addr[i]);
	}
#endif

	BITMAPFILEHEADER bmp_file;
	BITMAPINFODEADER bmp_info;

	bmp_file.bfType = 0x4d42;
	bmp_file.bfReserved1 = 0;
	bmp_file.bfReserved2 = 0;
	bmp_file.bfOffBits = sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFODEADER);
	bmp_file.bfSize = bmp_file.bfOffBits + mem_size / fb_num;

	bmp_info.biSize = sizeof(BITMAPINFODEADER);
	bmp_info.biWidth = screenW;
	bmp_info.biHeight = -screenH;
	bmp_info.biPlanes = 1;
	bmp_info.biBitCount = 32;
	bmp_info.biCompression = 0;
	bmp_info.biSizeImage = mem_size / fb_num;
	bmp_info.biXPelsPerMeter = 0;
	bmp_info.biYPelsPerMeter = 0;
	bmp_info.biClrImportant = 0;
	bmp_info.biClrUsed = 0;

	/*write framebuffer data to file*/
	for (i = 0; i < fb_num; i++) {
		fp = fopen("/tmp/capture01.bmp", "wb+");
		if (fp < 0) {
			printf("open file faild\n");
			munmap(vir_adr, mem_size);
			DispMemRelease(test_info.dispfh, test_info.mem_id);
			close(test_info.dispfh);
			return -1;
		}
		fwrite(&bmp_file, sizeof(BITMAPFILEHEADER), 1, fp);
		fwrite(&bmp_info, sizeof(bmp_info), 1, fp);
		fwrite(vir_adr + i * mem_size / fb_num, mem_size / fb_num, 1, fp);

		fclose(fp);
	}

	if (DispCaptureStop(test_info.dispfh, test_info.screen_id) < 0) {
		printf("capture stop failed!\n");
		munmap(vir_adr, mem_size);
		DispMemRelease(test_info.dispfh, test_info.mem_id);
		close(test_info.dispfh);
		return -1;
	}

	munmap(vir_adr, mem_size);

	if (DispMemRelease(test_info.dispfh, test_info.mem_id) < 0)
		printf("release memory failed!\n");

	close(test_info.dispfh);
	printf("GOOD, The image is saved in /tmp/capture01.bmp\n");

	int ret = tshowimg("/usr/share/capture01.bmp",
			"Could you see this picture clearly?");
	if (ret == 0) {
		printf("GOOD, picture show success.\n");
		return 0;
	} else {
		printf("ERROR, Failed to upload the image to the computer. You can use the adb pull command to transfer the image to your computer.\n");
		return -1;
	}
}
