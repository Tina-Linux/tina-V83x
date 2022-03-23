#ifndef __CAMERAV4L2__H__
#define __CAMERAV4L2__H__

#ifdef __cplusplus
extern "C" {
#endif

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
#include <linux/videodev2.h>
#include <pthread.h>
#include <semaphore.h>
#include <ctype.h>
#include <errno.h>
#include <stdio.h>

#define V4L2_BUF_NUM 5

struct buffer {
	void *start[3];
	size_t length[3];
	unsigned int image_id;
	unsigned int lost_image_num;
	unsigned int exp_time;
	long long image_timestamp;
};

typedef struct camera_handle {
	int video_index;
	int videofd;
	int driver_type;
	int nplanes;
	struct buffer *buffers;
	int buf_count;
	int fps;
	int width;
	int height;
	unsigned int format;
	void *private_info;
} camera_hal;

int camerainit(camera_hal *camera);
int setformat(camera_hal *camera);
int requestbuf(camera_hal *camera);
int releasebuf(camera_hal *camera);
int streamon(camera_hal *camera);
int streamoff(camera_hal *camera);
int releasecamera(camera_hal *camera);
int resetsync(camera_hal *camera);
int waitingbuf(camera_hal *camera);
int dqbuf(camera_hal *camera);
int qbuf(camera_hal *camera, int index);

#ifdef __cplusplus
}
#endif

#endif
