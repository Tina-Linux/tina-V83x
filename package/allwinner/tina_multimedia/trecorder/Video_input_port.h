/*
 * opyright (C) 2017 Allwinnertech
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef __VIDEO_INPUT_PORT_H__
#define __VIDEO_INPUT_PORT_H__

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include "videodev.h"
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <errno.h>
#include <ion_mem_alloc.h>
#include "water_mark.h"

#ifdef __USE_VIN_ISP__
#include "AWIspApi.h"
#endif

#define csi_camera 0
#define raw_camera 1
#define usb_camera 2

#define other_drive 0
#define vin_drive   1

#define CSI_SELECT_FRM_TIMEOUT 2
#define DISPLAY_S_WIDTH 1280
#define DISPLAY_S_HEIGHT 720
#define ALIGN_4K(x) (((x) + (4095)) & ~(4095))
#define ALIGN_32B(x) (((x) + (31)) & ~(31))
#define ALIGN_16B(x) (((x) + (15)) & ~(15))

typedef struct {
	void *start[3];
	unsigned int length[3];
} buffer;

typedef struct {
	int isPhy;
	unsigned int bufferId;
	long long pts;
	//int sourceIndex;
	void *MainYVirAddr;
	void *MainYPhyAddr;
	void *MainCVirAddr;
	void *MainCPhyAddr;
	void *SubYVirAddr;
	void *SubYPhyAddr;
	void *SubCVirAddr;
	void *SubCPhyAddr;
	void *RotYVirAddr;
	void *RotYPhyAddr;
	void *RotCVirAddr;
	void *RotCPhyAddr;
	int MainSize;
	int SubSize;
	int RotSize;
	int BytesUsed;
} dataParam;

typedef enum {
	TR_PIXEL_YUV420SP,
	TR_PIXEL_YVU420SP,
	TR_PIXEL_YUV420P,
	TR_PIXEL_YVU420P,
	TR_PIXEL_YUV422SP,
	TR_PIXEL_YVU422SP,
	TR_PIXEL_YUV422P,
	TR_PIXEL_YVU422P,
	TR_PIXEL_YUYV422,
	TR_PIXEL_UYVY422,
	TR_PIXEL_YVYU422,
	TR_PIXEL_VYUY422,
	TR_PIXEL_ARGB,
	TR_PIXEL_RGBA,
	TR_PIXEL_ABGR,
	TR_PIXEL_BGRA,
	TR_PIXEL_MJPEG,
	TR_PIXEL_H264,
	TR_PIXEL_TILE_32X32,
	TR_PIXEL_TILE_128X32,
} pixelFormat;

typedef enum {
	CAMERA_STATE_NONE,	/* Before create */
	CAMERA_STATE_CREATED,	/* Created, but not initialized yet */
	CAMERA_STATE_PREPARED,	/* Initialized and prepare for capturing */
	CAMERA_STATE_CAPTURING,	/* While capturing */
} camera_state_e;

typedef struct {
	int (*open) (void *hdl, int index);
	int (*close) (void *hdl);
	int (*init) (void *hdl, int enable, int format, int framerate,int width,
			int height,int subwidth, int subheight,int ispEnable, int rot_angle, int use_wm);
	int (*requestbuf) (void *hdl, int count);
	int (*releasebuf) (void *hdl, int count);
	int (*dequeue) (void *hdl, dataParam *param);
	int (*enqueue) (void *hdl, unsigned int index);
	int (*startcapture) (void *hdl);
	int (*stopcapture) (void *hdl);
	int (*getEnable) (void *hdl);
	int (*getFrameRate) (void *hdl);
	int (*getFormat) (void *hdl);
	int (*getSize) (void *hdl, int *width, int *height);
	int (*setEnable) (void *hdl, int enable);
	int (*setFrameRate) (void *hdl, int framerate);
	int (*setFormat) (void *hdl, int format);
	int (*setSize) (void *hdl, int width, int height);
	int (*setparameter) (void *hdl, int cmd, int param);
	int (*getCameraIndex)(void* hdl,int* index);
	int (*queryContrast) (void *hdl);
	int (*getContrast) (void *hdl);
	int (*setContrast) (void *hdl, int value);
	int (*getWhiteBalance) (void *hdl);
	int (*setWhiteBalance) (void *hdl, enum v4l2_auto_n_preset_white_balance value);
	int (*queryExposureCompensation) (void *hdl);
	int (*getExposureCompensation) (void *hdl);
	int (*setExposureCompensation) (void *hdl, int value);
	int (*WMInit) (WaterMarkInfo *WM_info, char WMPath[30]);
	int (*WMRelease) (void *hdl);
	int (*addWM) (void *hdl, unsigned int bg_width, unsigned int bg_height, void *bg_y_vir,
		void *bg_c_vir, unsigned int wm_pos_x, unsigned int wm_pos_y, struct tm *time_data);
	int fd;
	int enable;
    int cameraType;
    int driveType;
#ifdef __USE_VIN_ISP__
    int sensor_type;
    int ispId;
    AWIspApi *ispPort;
#endif
	int framerate;
	int MainWidth;
	int MainHeight;
	int SubWidth;
	int SubHeight;
	int use_isp;
	int scale_down_enable;
	int use_wm;
	int wm_pos_x;
	int wm_pos_y;
	int rot_angle;
	int buf_num;
    int nplanes;
    int mCameraIndex;
    long long lastFarmeTime;
    long long intervalTime;
	struct SunxiMemOpsS *pMemops;
	WaterMarkInfo WM_info;
	camera_state_e state;
	pixelFormat format;
	buffer *buffers;
} vInPort;

vInPort *CreateTinaVinport();
int DestroyTinaVinport(vInPort *hdl);

#endif
