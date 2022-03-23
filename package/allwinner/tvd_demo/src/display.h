/*************************************************************************
	> File Name: display.h
	> Author:
	> Mail:
	> Created Time: 2017年09月21日 星期四 13时52分52秒
 ************************************************************************/

#ifndef _DISPLAY_H
#define _DISPLAY_H

#include "convert.h"
#include "common.h"
#include "camera_video.h"
#include "camera_disp.h"

static unsigned char CAMPLAY_LOG = 1;
#define CAMPLAY_DBG_LOG if(CAMPLAY_LOG) \
                            printf

#define ALIGN_16B(x) (((x) + (15)) & ~(15))


#define MAX_FRAME 1000

typedef struct _cameraplay
{
    unsigned int quitFlag;

	unsigned int saveframe_Flag;

    camera_hdl capture;

    displayhdl display;

}cameraplay;

#endif
