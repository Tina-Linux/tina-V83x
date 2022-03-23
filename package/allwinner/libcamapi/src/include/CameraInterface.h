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

#ifndef __CAMERA_INTERFACE_H__
#define __CAMERA_INTERFACE_H__
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
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <errno.h>

#include <linux/videodev2.h>


/*
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
*/

#define MAX_SELECT_TIME 2
#define MAX_NUM 20
#define MAX_PLANES 1
#define NUM_SUPPORT_FMT 30

typedef enum {
	PIXEL_BASE,
	/* 4:2:0 */
	PIXEL_YUV420SP,
	PIXEL_YVU420SP,
	PIXEL_YUV420P,
	PIXEL_YVU420P,
	/* 4:2:2 */
	PIXEL_YUV422SP,
	PIXEL_YVU422SP,
	PIXEL_YUV422P,
	/*videodev2.h not define
	PIXEL_YVU422P,*/
	/* Y U Y V */
	PIXEL_YUYV,
	PIXEL_UYVY,
	PIXEL_YVYU,
	PIXEL_VYUY,
	/* codec */
	PIXEL_MJPEG,
	PIXEL_H264,
} CameraPixelFormat;

struct CameraFrameSize
{
    unsigned int width;
    unsigned int height;
    unsigned int framerate[MAX_NUM];
};

struct CameraFrameInfo
{
    unsigned char name[32];
    unsigned int fourcc;
	CameraPixelFormat format;
    struct CameraFrameSize size[MAX_NUM];
};

typedef enum {
	CAMERA_STATE_NONE,
	CAMERA_STATE_CREATED,
	CAMERA_STATE_PREPARED,
	CAMERA_STATE_CAPTURING,
} CameraState;

typedef struct {
	int index;
	void *phyaddr[MAX_PLANES];
	void *viraddr[MAX_PLANES];
	unsigned int length[MAX_PLANES];
} CameraBuffer;

typedef struct {
	/* base */
	int fd;
	int index;
	int enable;
	CameraPixelFormat format;
	unsigned int width,height;
	unsigned int framerate;
	unsigned int reqbufcount;
	CameraBuffer *bufqueue;
	CameraState state;
	/* func */
	int (*open) (void *hdl, int index);
	int (*close) (void *hdl);
	int (*init) (void *hdl);
	int (*deinit) (void *hdl);
	int (*streamon) (void *hdl);
	int (*streamoff) (void *hdl);
	int (*dequeue) (void *hdl, CameraBuffer *buf);
	int (*enqueue) (void *hdl, CameraBuffer *buf);
} Camera;


/* try and rewrite camera.cfg */
int CameraOpen(void *hdl, int index);
int CameraClose(void *hdl);

/* init with camera.cfg */
static int CameraInit(void *hdl);
static int CameraDeInit(void *hdl);

int CameraStreamOn(void *hdl);
int CameraStreamOff(void *hdl);

int CameraDequeue(void *hdl, CameraBuffer *buf);
int CameraEnqueue(void *hdl, CameraBuffer *buf);

/*
   only set when not streamon and part of init
*/

/*
   can set when streamon
*/
static int CameraSetCtl();
static int CameraGetCtl();
static int CameraQueryCtl();

Camera *CreateCameraDevice();
#endif
