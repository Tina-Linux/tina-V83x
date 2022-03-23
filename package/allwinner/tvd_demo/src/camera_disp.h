/*************************************************************************
	> File Name: camera_disp.h
	> Author:
	> Mail:
	> Created Time: 2017年09月21日 星期四 10时16分40秒
 ************************************************************************/

#ifndef _CAMERA_DISP_H
#define _CAMERA_DISP_H

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
#include "videoOutPort.h"


typedef struct _disphdl
{
	dispOutPort *DisPort;
    pthread_t tid;

}displayhdl;

int camera_dispInit(void *arg);
int dispClose(dispOutPort *disPort);
int display(void *param);
void *DisplayThread(void *param);

#endif
