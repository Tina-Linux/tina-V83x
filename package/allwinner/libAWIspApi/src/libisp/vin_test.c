
/*
 ******************************************************************************
 *
 * snapshot.c
 *
 * Hawkview ISP - snapshot.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/17	VIDEO INPUT
 *
 *****************************************************************************
 */

#include <endian.h>
#include <errno.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "vi_api/vi_api.h"
#include "vi_api/common_vi.h"

#define MAX_VIDEO_NUM 	4

char path_name[20];
int save_cnt = 20;
int test_cnt = 1;

typedef struct awVI_PrivCap_S
{
	AW_CHN Chn;
	AW_S32 s32MilliSec;
	VI_ATTR_S pstAttr;
	VI_FRAME_BUF_INFO_S pstFrameInfo;

} VI_PrivCap_S;

static void *loop_cap(void *pArg)
{
	VI_PrivCap_S *privCap = (VI_PrivCap_S *)pArg;
	AW_CHN ViCh;
	int ret = 0, i = 0;

	ViCh = privCap->Chn;
	printf("current vi channel is = %d\n", privCap->Chn);

	ret = AW_MPI_VI_InitCh(ViCh);
	if (SUCCESS != ret) {
		printf("AW_MPI_VI_InitCh failed\n");
		goto vi_exit;
	}

	ret = AW_MPI_VI_SetChnAttr(ViCh, &privCap->pstAttr);
	if (SUCCESS != ret) {
		printf("AW_MPI_VI_SetChnAttr failed\n");
		goto exit_vich;
	}

	ret = AW_MPI_VI_GetChnAttr(ViCh, &privCap->pstAttr);
	if (SUCCESS != ret) {
		printf("AW_MPI_VI_GetChnAttr failed\n");
		goto exit_vich;
	}

	ret = AW_MPI_VI_EnableChn(ViCh);
	if (SUCCESS != ret) {
		printf("AW_MPI_VI_EnableChn failed\n");
		goto exit_vich;
	}

	while (i < save_cnt) {
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_BRIGHTNESS, i%128);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_CONTRAST, i%128);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_SATURATION, i%4);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_HUE, i%128);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_AUTO_WHITE_BALANCE, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_EXPOSURE, i%128);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_AUTOGAIN, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_GAIN, i%128+16);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_POWER_LINE_FREQUENCY, i%4);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_HUE_AUTO, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_WHITE_BALANCE_TEMPERATURE, 2800+i%6500);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_SHARPNESS, i%64 - 32);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_CHROMA_AGC, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_COLORFX, i%16);

		AW_MPI_VI_SetControl(ViCh, V4L2_CID_AUTOBRIGHTNESS, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_BAND_STOP_FILTER, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_ILLUMINATORS_1, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_ILLUMINATORS_2, i%2);

		AW_MPI_VI_SetControl(ViCh, V4L2_CID_EXPOSURE_AUTO, i%4);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_EXPOSURE_ABSOLUTE, i%1000000);

		AW_MPI_VI_SetControl(ViCh, V4L2_CID_EXPOSURE_AUTO_PRIORITY, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_FOCUS_ABSOLUTE, i%128);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_FOCUS_RELATIVE, i%128);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_FOCUS_AUTO, i%2);

		AW_MPI_VI_SetControl(ViCh, V4L2_CID_AUTO_EXPOSURE_BIAS, i%4);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, i%10);

		AW_MPI_VI_SetControl(ViCh, V4L2_CID_WIDE_DYNAMIC_RANGE, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_IMAGE_STABILIZATION, i%2);

		AW_MPI_VI_SetControl(ViCh, V4L2_CID_ISO_SENSITIVITY, i%4);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_ISO_SENSITIVITY_AUTO, i%2);

		AW_MPI_VI_SetControl(ViCh, V4L2_CID_EXPOSURE_METERING, i%4);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_SCENE_MODE, i%2);

		AW_MPI_VI_SetControl(ViCh, V4L2_CID_3A_LOCK, i%8);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_AUTO_FOCUS_START, i%1);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_AUTO_FOCUS_STOP, i%1);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_AUTO_FOCUS_RANGE, i%4);

		AW_MPI_VI_SetControl(ViCh, V4L2_CID_HFLIP, i%2);
		AW_MPI_VI_SetControl(ViCh, V4L2_CID_VFLIP, i%2);

		if (AW_MPI_VI_GetFrame(ViCh, &privCap->pstFrameInfo, privCap->s32MilliSec) < 0) {
			//printf("VI Get Frame failed!\n");
			continue;
		}
		printf("ch%d process buffer %d\r\n", ViCh, i);
		AW_MPI_VI_ReleaseFrame(ViCh, &privCap->pstFrameInfo);
		i++;
	}

disablech:
	AW_MPI_VI_DisableChn(ViCh);
exit_vich:
	AW_MPI_VI_ExitCh(ViCh);
vi_exit:
	return NULL;
}

int main(int argc __attribute__((__unused__)), char *argv[] __attribute__((__unused__)))
{

	pthread_t thid[MAX_VIDEO_NUM];
	int ret, i, cnt;
	VI_PrivCap_S privCap[MAX_VIDEO_NUM];
	int ch_num = 0;
	int width = 640;
	int height = 480;
	int out_fmt = 1;

	memset(path_name, 0, sizeof (path_name));

	if (argc == 1) {
		sprintf(path_name, "/mnt/sdcard");
	} else if (argc == 2) {
		sprintf(path_name, "/mnt/sdcard");
		ch_num = atoi(argv[1]);
	} else if (argc == 4) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "/mnt/sdcard");
	} else if (argc == 5) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
	} else if (argc == 6) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
		out_fmt = atoi(argv[5]);
	} else if (argc == 7) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
		out_fmt = atoi(argv[5]);
		save_cnt = atoi(argv[6]);
	} else if (argc == 8) {
		ch_num = atoi(argv[1]);
		width = atoi(argv[2]);
		height = atoi(argv[3]);
		sprintf(path_name, "%s", argv[4]);
		out_fmt = atoi(argv[5]);
		save_cnt = atoi(argv[6]);
		test_cnt = atoi(argv[7]);
	} else {
		printf("please select the ch_num: 1~4 ......\n");
		scanf("%d", &ch_num);

		printf("please input the resolution: width height......\n");
		scanf("%d %d", &width, &height);

		printf("please input the frame saving path......\n");
		scanf("%15s", path_name);

		printf("please input the test out_fmt: 0~3......\n");
		scanf("%d", &out_fmt);

		printf("please input the save_cnt: >=1......\n");
		scanf("%d", &save_cnt);
	}

	if (ch_num > MAX_VIDEO_NUM)
		ch_num = MAX_VIDEO_NUM;

	for (cnt = 0; cnt < test_cnt; cnt++) {
		printf("vin test count is %d\n", cnt);

		ret = AW_MPI_VI_Init();
		if (SUCCESS != ret) {
			printf("AW_MPI_VI_Init failed\n");
			return -1;
		}

		for(i = 0; i < ch_num; i++) {
			memset(&privCap[i], 0, sizeof(VI_PrivCap_S));
			/*Set Dev ID and Chn ID*/
			privCap[i].Chn = i;
			privCap[i].s32MilliSec = 2000;

			/*Set VI Channel Attribute*/
			privCap[i].pstAttr.type = V4L2_BUF_TYPE_VIDEO_CAPTURE_MPLANE;
			privCap[i].pstAttr.memtype = V4L2_MEMORY_MMAP;
			switch (out_fmt) {
			case 0: privCap[i].pstAttr.format.pixelformat = V4L2_PIX_FMT_SBGGR8; break;
			case 1: privCap[i].pstAttr.format.pixelformat = V4L2_PIX_FMT_YUV420M; break;
			case 2: privCap[i].pstAttr.format.pixelformat = V4L2_PIX_FMT_YUV420; break;
			case 3: privCap[i].pstAttr.format.pixelformat = V4L2_PIX_FMT_NV12M; break;
			default: privCap[i].pstAttr.format.pixelformat = V4L2_PIX_FMT_YUV420M; break;
			}
			privCap[i].pstAttr.format.field = V4L2_FIELD_NONE;
			privCap[i].pstAttr.format.width = width;
			privCap[i].pstAttr.format.height = height;
			privCap[i].pstAttr.nbufs = 8;
			privCap[i].pstAttr.nplanes = 3;
			privCap[i].pstAttr.fps = 30;
			privCap[i].pstAttr.capturemode = V4L2_MODE_VIDEO;

			/*Call Video Thread*/
			ret = pthread_create(&thid[i], NULL, loop_cap, (void *)&privCap[i]);

			if (ret < 0) {
				printf("pthread_create failed, Chn[%d].\n",
					privCap[i].Chn);
				continue;
			}
		}
		for(i = 0; i < ch_num; i++)
			pthread_join(thid[i], NULL);

		AW_MPI_VI_Exit();
	}

	return 0;
}

