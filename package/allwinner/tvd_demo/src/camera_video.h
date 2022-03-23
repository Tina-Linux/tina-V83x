/*************************************************************************
	> File Name: camera_video.h
	> Author:
	> Mail:
	> Created Time: 2017年09月20日 星期三 20时22分00秒
 ************************************************************************/

#ifndef _CAMERA_VIDEO_H
#define _CAMERA_VIDEO_H

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


struct buffer
{
    void *start;
    size_t length;
};

struct datainfo
{
    int index;
    void *start;
    unsigned long phaddr;
    size_t length;
    struct timeval timestamp;
};

typedef struct camerahdl
{
    int videofd;
    pthread_t tid;
    int output_enable;
    unsigned int video_width;
    unsigned int video_height;
    unsigned int pixelformat;
    unsigned int fps;
    unsigned int num_buffer;
    struct buffer *buffers;
    struct datainfo data_info;
    sem_t dqfinSem;
    sem_t qbufSem;

	int difd;
	int frist_frame;
	struct v4l2_buffer tvd_buf[2];
	struct SunxiMemOpsS *ionfd;
}camera_hdl;

int cameraInit(camera_hdl *hdl);
int cameraStartCapture(camera_hdl *hdl);
int cameraStopCapture(camera_hdl *hdl);
int cameraRelease(camera_hdl *hdl);
void *CameraThread(void *param);

#endif
