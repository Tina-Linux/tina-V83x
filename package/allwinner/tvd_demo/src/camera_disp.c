/*************************************************************************
	> File Name: camera_disp.c
	> Author:
	> Mail:
	> Created Time: 2017年09月21日 星期四 10时17分08秒
 ************************************************************************/

#include "display.h"

int camera_dispInit(void *arg)
{
	cameraplay *hdl = (cameraplay *)arg;

    VoutRect mPreviewRect;
	videoParam param;

    mPreviewRect.x = 0;
	mPreviewRect.y = 0;
	mPreviewRect.width = 800;//1280;
	mPreviewRect.height = 480;//800;

	hdl->display.DisPort = CreateVideoOutport(0);

	hdl->display.DisPort->init(hdl->display.DisPort,1,ROTATION_ANGLE_0,&mPreviewRect);
    mPreviewRect.width = hdl->display.DisPort->getScreenWidth(hdl->display.DisPort);
    mPreviewRect.height = hdl->display.DisPort->getScreenHeight(hdl->display.DisPort);
    printf("width:%d, height:%d\n", (int)mPreviewRect.width, (int)mPreviewRect.height);

    /* set Route */
	hdl->display.DisPort->setRoute(hdl->display.DisPort,VIDEO_SRC_FROM_CAM);

	/* set Rect */
	hdl->display.DisPort->setRect(hdl->display.DisPort,&mPreviewRect);

	param.srcInfo.w = hdl->capture.video_width;
	param.srcInfo.h = hdl->capture.video_height;
	hdl->display.DisPort->allocateVideoMem(hdl->display.DisPort,&param);

    hdl->display.DisPort->SetZorder(hdl->display.DisPort,VIDEO_ZORDER_MIDDLE);

    CAMPLAY_DBG_LOG(" disp init end!\n");

    return 0;
}

int dispClose(dispOutPort *disPort)
{
    disPort->enable = 0;
	disPort->setEnable(disPort,0);
    disPort->freeVideoMem(disPort);
    disPort->deinit(disPort);
    DestroyVideoOutport(disPort);

	return 0;
}

int display(void *param)
{
	cameraplay *hdl = (cameraplay *)param;
	videoParam paramDisp;
    static int flag = 0;

	/* set rotate angel */
	hdl->display.DisPort->setRotateAngel(hdl->display.DisPort,ROTATION_ANGLE_0);

	paramDisp.isPhy = 0;
	paramDisp.srcInfo.w = hdl->capture.video_width;
	paramDisp.srcInfo.h = hdl->capture.video_height;
	paramDisp.srcInfo.crop_x = 0;
	paramDisp.srcInfo.crop_y = 0;
	paramDisp.srcInfo.crop_w = hdl->capture.video_width;
	paramDisp.srcInfo.crop_h = hdl->capture.video_height;
	paramDisp.srcInfo.format = VIDEO_PIXEL_FORMAT_NV12;
	paramDisp.srcInfo.color_space = 0;
 /*
    if(hdl->capture.pixelformat == V4L2_PIX_FMT_YUYV){
        unsigned char NV12;
        NV12 = malloc(hdl->capture.video_width*hdl->capture.video_height*1.5);
        if(NV12 == NULL)
            return-1;
        YUYVToNV12(NV12, hdl->capture.data_info.start, hdl->capture.video_width, hdl->capture.video_height);
        hdl->display.DisPort->writeData(hdl->display.DisPort, NV12, hdl->capture.video_width*hdl->capture.video_height*1.5,&paramDisp);
        free(NV12);
    }else*/
	    hdl->display.DisPort->writeData(hdl->display.DisPort,hdl->capture.data_info.start,hdl->capture.data_info.length,&paramDisp);

    if(flag == 0)
    {
        flag++;

        hdl->display.DisPort->SetZorder(hdl->display.DisPort,VIDEO_ZORDER_MIDDLE);

        /* enable display */
		hdl->display.DisPort->setEnable(hdl->display.DisPort,1);
    }
}

void *DisplayThread(void *param)
{
    cameraplay *playhdl = (cameraplay *)param;

    while(!playhdl->quitFlag)
    {
        sem_wait(&playhdl->capture.dqfinSem);

        display(playhdl);

        sem_post(&playhdl->capture.qbufSem);
		//usleep(10000);
    }

	/* Prevent program deadlock, release multiple times */
	sem_post(&playhdl->capture.qbufSem);

    dispClose(playhdl->display.DisPort);

    CAMPLAY_DBG_LOG(" display thread end\n");

    return (void *)0;
}
