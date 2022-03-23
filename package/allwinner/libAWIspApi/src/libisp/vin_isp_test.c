
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

#include "isp.h"

#define MAX_VIDEO_NUM 	4
#define ISP_SERVER_STOP 0

char path_name[30];
int save_cnt;
int test_cnt = 10;

typedef struct awVI_PrivCap_S
{
	AW_CHN Chn;
	AW_S32 isp_id;
	AW_S32 s32MilliSec;
	VI_ATTR_S pstAttr;
	VI_FRAME_BUF_INFO_S pstFrameInfo;

} VI_PrivCap_S;

static void isp_server_start(int isp_id)
{
	/*
	 * media_open must after AW_MPI_VI_InitCh, otherwise, media_pipeline_get_head
	 * will not get sensor entity, and then lead to failure of calling isp_sensor_get_configs,
	 * because of sensor link will not enable until s_input is called.
	 */

	isp_init(isp_id);
	isp_run(isp_id);
}

static void isp_server_stop(int isp_id)
{
	isp_stop(isp_id);
	isp_pthread_join(isp_id);
	isp_exit(isp_id);
}

static void isp_server_wait_to_exit(int isp_id)
{
	isp_pthread_join(isp_id);
	isp_exit(isp_id);
}

static void *loop_cap(void *pArg)
{
	VI_PrivCap_S *privCap = (VI_PrivCap_S *)pArg;
	int ret = 0, i = 0;

	printf("current vi channel is = %d\n", privCap->Chn);

	//AW_MPI_OSD_SetFmt(privCap->Chn);
	//AW_MPI_OSD_Update(privCap->Chn, 1);

	ret = AW_MPI_VI_EnableChn(privCap->Chn);
	if (SUCCESS != ret) {
		printf("AW_MPI_VI_EnableChn failed\n");
		goto vi_exit;
	}

	while (i < save_cnt) {
		if (AW_MPI_VI_GetFrame(privCap->Chn, &privCap->pstFrameInfo, privCap->s32MilliSec) < 0) {
			//printf("VI Get Frame failed!\n");
			continue;
		}
		printf("process channel(%d) frame %d\r\n", privCap->Chn, i);
		AW_MPI_VI_SaveFrame(privCap->Chn, &privCap->pstFrameInfo, path_name);
		AW_MPI_VI_ReleaseFrame(privCap->Chn, &privCap->pstFrameInfo);
		i++;
#if ISP_SERVER_STOP

		if (i == save_cnt/4) {
			printf("isp%d server stop!!!\n", privCap->isp_id);
			isp_server_stop(privCap->isp_id);
		}

		if (i == save_cnt/2) {
			printf("isp%d server restart!!!\n", privCap->isp_id);
			isp_server_start(privCap->isp_id);
		}

		if (i == save_cnt*3/4) {
			printf("isp%d server stop!!!\n", privCap->isp_id);
			isp_server_stop(privCap->isp_id);
		}

		if (i == save_cnt*3/5) {
			printf("isp%d server update!!!\n", privCap->isp_id);
			isp_update(privCap->isp_id);
		}
#endif
		//AW_MPI_VI_GetEvent(privCap->Chn);
	}
disablech:

	//AW_MPI_OSD_Update(privCap->Chn, 0);
	AW_MPI_VI_DisableChn(privCap->Chn);
vi_exit:
	return NULL;
}

int main_test(int ch_num, int width, int height, int out_fmt)
{
	pthread_t thid[MAX_VIDEO_NUM];
	int ret, i, ch = -1;
	VI_PrivCap_S privCap[MAX_VIDEO_NUM];

	ret = AW_MPI_VI_Init();
	if (SUCCESS != ret) {
		printf("AW_MPI_VI_Init failed\n");
		return -1;
	}

	media_dev_init();

	if (ch_num > MAX_VIDEO_NUM)
		ch_num = MAX_VIDEO_NUM;

	if (ch_num == 0 || ch_num == 1) {
		ch = ch_num;
		ch_num = 2;
	}

	for(i = 0; i < ch_num; i++) {
		if (i != ch && ch != -1)
			continue;
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
		privCap[i].pstAttr.use_current_win = 0;

		privCap[i].pstAttr.nbufs = 8;
		privCap[i].pstAttr.nplanes = 3;
		privCap[i].pstAttr.fps = 30;
		privCap[i].pstAttr.capturemode = V4L2_MODE_VIDEO;

		ret = AW_MPI_VI_InitCh(i);
		if (SUCCESS != ret) {
			printf("AW_MPI_VI_InitCh failed\n");
			return -1;
		}

		ret = AW_MPI_VI_SetChnAttr(i, &privCap[i].pstAttr);
		if (SUCCESS != ret) {
			printf("AW_MPI_VI_SetChnAttr failed\n");
			return -1;
		}

		ret = AW_MPI_VI_GetChnAttr(i, &privCap[i].pstAttr);
		if (SUCCESS != ret) {
			printf("AW_MPI_VI_GetChnAttr failed\n");
			return -1;
		}

		AW_MPI_VI_GetIspId(i, &privCap[i].isp_id);
		if (privCap[i].isp_id == -1)
			continue;

		/*Call Video Thread*/
		ret = pthread_create(&thid[i], NULL, loop_cap, (void *)&privCap[i]);
		if (ret < 0) {
			printf("pthread_create loop_cap Chn[%d] failed.\n", i);
			continue;
		}

		/*Call isp server*/
		printf("isp%d server start!!!\n", privCap[i].isp_id);
		isp_server_start(privCap[i].isp_id);
	}

#if !ISP_SERVER_STOP
	for(i = 0; i < ch_num; i++) {
		if (i != ch && ch != -1)
			continue;
		printf("isp%d server wait to exit!!!\n", privCap[i].isp_id);
		isp_server_wait_to_exit(privCap[i].isp_id);
	}
#endif
	media_dev_exit();

	for(i = 0; i < ch_num; i++) {
		if (i != ch && ch != -1)
			continue;
		printf("video%d wait to exit!!!\n", i);
		pthread_join(thid[i], NULL);
	}

	for(i = 0; i < ch_num; i++) {
		if (i != ch && ch != -1)
			continue;
		AW_MPI_VI_ExitCh(i);
	}

	AW_MPI_VI_Exit();

	return 0;
}

int main(int argc __attribute__((__unused__)), char *argv[] __attribute__((__unused__)))
{
	int ret, n = 0;
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

	for (n = 0; n < test_cnt; n++) {
		ret = main_test(ch_num, width, height, out_fmt);
		printf("vin isp test %d, return %d\n", n + 1, ret);
	}

	return 0;
}

