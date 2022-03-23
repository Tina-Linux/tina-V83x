
/*
 ******************************************************************************
 *
 * vi_api.c
 *
 * Hawkview ISP - vi_api.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/04/01	VIDEO INPUT
 *
 *****************************************************************************
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "common_vi.h"
#include "video.h"
#include "isp_dev.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif  /* __cplusplus */

#define MEDIA_DEVICE "/dev/media0"

struct hw_isp_media_dev *media = NULL;

AW_S32 AW_MPI_VI_Init(void)
{
	if(media!=NULL)
	{
		ISP_PRINT("mpi_vi already init\n");
		return SUCCESS;
	}
	media = isp_md_open(MEDIA_DEVICE);

	if (media == NULL) {
		ISP_PRINT("unable to open media device %s\n", MEDIA_DEVICE);
		return FAILURE;
	}
	return SUCCESS;
}

AW_S32 AW_MPI_VI_Exit(void)
{
	/* Cleanup the ISP resources. */
	if (media) {
		isp_md_close(media);
	}
	media = NULL;
	return SUCCESS;
}

AW_S32 AW_MPI_VI_InitCh(AW_CHN ViCh)
{
	if (isp_video_open(media, ViCh) < 0)
		return FAILURE;
	return SUCCESS;
}

AW_S32 AW_MPI_VI_ExitCh(AW_CHN ViCh)
{
	isp_video_close(media, ViCh);
	return SUCCESS;
}

AW_S32 AW_MPI_VI_SetChnAttr(AW_CHN ViCh, VI_ATTR_S *pstAttr)
{
	struct isp_video_device *video = NULL;
	struct video_fmt vfmt;
	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}
	memset(&vfmt, 0, sizeof(vfmt));

	vfmt.type = pstAttr->type;
	vfmt.memtype = pstAttr->memtype;
	vfmt.format = pstAttr->format;
	vfmt.nbufs = pstAttr->nbufs;
	vfmt.nplanes = pstAttr->nplanes;
	vfmt.fps = pstAttr->fps;
	vfmt.capturemode = pstAttr->capturemode;
	vfmt.use_current_win = pstAttr->use_current_win;

	if (video_set_fmt(video, &vfmt) < 0)
		return FAILURE;

	return SUCCESS;
}

AW_S32 AW_MPI_VI_GetChnAttr(AW_CHN ViCh, VI_ATTR_S *pstAttr)
{
	struct isp_video_device *video = NULL;
	struct video_fmt vfmt;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}
	memset(&vfmt, 0, sizeof(vfmt));
	video_get_fmt(video, &vfmt);

	pstAttr->type = vfmt.type;
	pstAttr->format = vfmt.format;
	pstAttr->nbufs = vfmt.nbufs;
	pstAttr->nplanes = vfmt.nplanes;
	return SUCCESS;
}

AW_S32 AW_MPI_VI_EnableChn(AW_CHN ViCh)
{
	struct isp_video_device *video = NULL;
	struct buffers_pool *pool = NULL;
	struct video_fmt vfmt;
	int i;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}
	pool = buffers_pool_new(video);
	if (NULL == pool)
		return FAILURE;

	if (video_req_buffers(video, pool) < 0)
		return FAILURE;
	memset(&vfmt, 0, sizeof(vfmt));
	video_get_fmt(video, &vfmt);
	for (i = 0; i < vfmt.nbufs; i++)
		video_queue_buffer(video, i);

	//video_event_subscribe(video, V4L2_EVENT_VSYNC);

	if (video_stream_on(video) < 0)
		return FAILURE;

	return SUCCESS;
}

AW_S32 AW_MPI_VI_DisableChn(AW_CHN ViCh)
{
	struct isp_video_device *video = NULL;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}
	//video_event_unsubscribe(video, V4L2_EVENT_VSYNC);

	if (video_stream_off(video) < 0)
		return FAILURE;
	if (video_free_buffers(video) < 0)
		return FAILURE;
	buffers_pool_delete(video);

	return SUCCESS;
}

AW_S32 AW_MPI_VI_GetFrame(AW_CHN ViCh, VI_FRAME_BUF_INFO_S *pstFrameInfo, AW_S32 s32MilliSec)
{
	struct isp_video_device *video = NULL;
	struct video_buffer buffer;
	struct video_fmt vfmt;
	int i;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}
	if (video_wait_buffer(video, s32MilliSec) < 0)
		return FAILURE;

	if (video_dequeue_buffer(video, &buffer) < 0)
		return FAILURE;

	memset(&vfmt, 0, sizeof(vfmt));
	video_get_fmt(video, &vfmt);
	for (i = 0; i < vfmt.nplanes; i++) {
		pstFrameInfo->pVirAddr[i] = buffer.planes[i].mem;
		pstFrameInfo->u32Stride[i] = buffer.planes[i].size;
		pstFrameInfo->u32PhyAddr[i] = buffer.planes[i].mem_phy;
	}
	pstFrameInfo->u32Width = vfmt.format.width;
	pstFrameInfo->u32Height = vfmt.format.height;
	pstFrameInfo->u32field = vfmt.format.field;
	pstFrameInfo->u32PixelFormat = vfmt.format.pixelformat;
	pstFrameInfo->stTimeStamp = buffer.timestamp;
	pstFrameInfo->u32PoolId = buffer.index;

	return SUCCESS;
}

AW_S32 AW_MPI_VI_ReleaseFrame(AW_CHN ViCh, VI_FRAME_BUF_INFO_S *pstFrameInfo)
{
	struct isp_video_device *video = NULL;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}

	if (video_queue_buffer(video, pstFrameInfo->u32PoolId) < 0)
		return FAILURE;
	return SUCCESS;
}

AW_S32 AW_MPI_VI_SaveFrame(AW_CHN ViCh, VI_FRAME_BUF_INFO_S *pstFrameInfo, char *path)
{
	struct isp_video_device *video = NULL;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}

	if (video_save_frames(video, pstFrameInfo->u32PoolId, path) < 0)
		return FAILURE;
	return SUCCESS;
}

AW_S32 AW_MPI_VI_SetControl(AW_CHN ViCh, int cmd, int value)
{
	struct isp_video_device *video = NULL;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}

	if (video_set_control(video, cmd, value) < 0)
		return FAILURE;
	return SUCCESS;
}

AW_S32 AW_MPI_VI_GetControl(AW_CHN ViCh, int cmd, int *value)
{
	struct isp_video_device *video = NULL;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}

	if (video_get_control(video, cmd, value) < 0)
		return FAILURE;
	return SUCCESS;
}

AW_S32 AW_MPI_VI_GetIspId(AW_CHN ViCh, AW_S32 *id)
{
	struct isp_video_device *video = NULL;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}

	*id = video_to_isp_id(video);
	return SUCCESS;
}

AW_S32 AW_MPI_VI_GetEvent(AW_CHN ViCh)
{
	struct isp_video_device *video = NULL;
	struct video_event vi_event;
	struct video_fmt vfmt;
	int i;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}
	if (video_wait_event(video) < 0)
		return FAILURE;

	video_dequeue_event(video, &vi_event);

	return SUCCESS;
}

AW_S32 AW_MPI_OSD_SetFmt(AW_CHN ViCh)
{
	struct isp_video_device *video = NULL;
	struct osd_fmt ofmt;
	void *bitmap = NULL;
	int i, j, bitmap_size = 0, *databuf = NULL;

	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}

	ofmt.clipcount = 2;
	ofmt.chromakey = V4L2_PIX_FMT_RGB32;
	ofmt.global_alpha = 255;

	for (i = 0; i < 2; i++) {
		ofmt.region[i].height = 32;
		ofmt.region[i].width = 464;
		ofmt.region[i].left = 0;
		ofmt.region[i].top = 200*i;

		bitmap_size = ofmt.region[i].width * ofmt.region[i].height;

		ofmt.bitmap[i] = calloc(bitmap_size, 4);
		if (ofmt.bitmap[i] == NULL) {
			printf("calloc of bitmap buf failed\n");
			return -ENOMEM;
		}
		databuf = ofmt.bitmap[i];

		for (j = 0; j < bitmap_size; j++) {
			databuf[j] = 0xff00ff00;
		}

		ofmt.reverse_close[i] = 0;
		ofmt.glb_alpha[i] = 16;
	}

	overlay_set_fmt(video, &ofmt);

	for (i = 0; i < 2; i++)
		free(ofmt.bitmap[i]);
	return 0;
}

AW_S32 AW_MPI_OSD_Update(AW_CHN ViCh, int on_off)
{
	struct isp_video_device *video = NULL;
	if (ViCh >= HW_VIDEO_DEVICE_NUM || NULL == media->video_dev[ViCh]) {
		ISP_ERR("VIN CH[%d] number is invalid!\n", ViCh);
		return AW_ERR_VI_INVALID_CHNID;
	} else {
		video = media->video_dev[ViCh];
	}

	return overlay_update(video, on_off);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif  /* __cplusplus */

