
/*
 ******************************************************************************
 *
 * isp_dev.c
 *
 * Hawkview ISP - isp_dev.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/05/11	VIDEO INPUT
 *
 *****************************************************************************
 */

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "video/video_priv.h"
#include "isp_dev_priv.h"
#include "../include/device/isp_dev.h"
#include "isp_v4l2_helper.h"
#include "isp_subdev.h"
#include "../include/isp_ini_parse.h"

#define MEDIA_DEVICE "/dev/media0"
#define ENTITY_SUNXI_VIDEO "vin_video"

unsigned int isp_dev_log_param = 0;

int isp_dev_open(struct hw_isp_media_dev *isp_md, int id)
{
	int ret = -1;
	struct hw_isp_device *isp;
	struct media_entity *entity = NULL;

	if (id >= HW_ISP_DEVICE_NUM)
		return ret;
	if (NULL != isp_md->isp_dev[id])
		return 0;

	isp = malloc(sizeof(struct hw_isp_device));
	if (isp == NULL)
		goto error;
	memset(isp, 0, sizeof(*isp));

	isp->id = id;
	hw_isp_set_priv_data(isp, isp_md);

	ret = isp_stats_init(isp);
	if (ret < 0) {
		ISP_ERR("unable to initialize statistics engine.\n");
		goto free_isp;
	}

	ret = isp_subdev_init(isp);
	if (ret < 0) {
		ISP_ERR("unable to initialize isp subdev.\n");
		goto free_isp;
	}

	entity = media_pipeline_get_head(&isp->isp_sd.entity);
	if(entity) {
		ISP_PRINT("register sensor entity is %s\n", entity->info.name);
		isp->sensor.entity = *entity;
		ret = v4l2_subdev_open(&isp->sensor.entity);
		if (ret < 0) {
			ISP_ERR("unable to open sensor subdev.\n");
			goto free_isp;
		}
	} else {
		ISP_ERR("unable to get sensor subdev.\n");
		goto free_isp;
	}

	ISP_PRINT("open isp device[%d] success!\n", id);
	isp_md->isp_dev[id] = isp;
	return 0;
free_isp:
	free(isp);
error:
	isp_md->isp_dev[id] = NULL;
	ISP_ERR("unable to open isp device[%d]\n", id);
	return -1;
}

void isp_dev_close(struct hw_isp_media_dev *isp_md, int id)
{
	if (isp_md == NULL)
		return;

	if (isp_md->isp_dev[id] == NULL)
		return;

	isp_stats_exit(isp_md->isp_dev[id]);
	isp_subdev_exit(isp_md->isp_dev[id]);
	v4l2_subdev_close(&isp_md->isp_dev[id]->sensor.entity);
	free(isp_md->isp_dev[id]);
	isp_md->isp_dev[id] = NULL;
}

/*
 * make sure that, it is only one media device
 */
struct hw_isp_media_dev *isp_md_open(const char *devname)
{
	struct hw_isp_media_dev *isp_md;

	isp_md = malloc(sizeof *isp_md);
	if (isp_md == NULL)
		return NULL;
	if((access(devname, F_OK) != 0)) {
		printf("warning: mknod media device %s c 253 0\n", devname);
		system("mknod /dev/media0 c 253 0");
	}

	memset(isp_md, 0, sizeof *isp_md);
	isp_md->mdev = media_open(devname, 0);
	if (isp_md->mdev == NULL) {
		ISP_ERR("error: unable to open isp_md device %s\n", devname);
		free(isp_md);
		return NULL;
	}

	//ISP_PRINT("media open at the first time!!!\n");

	return isp_md;
}

void isp_md_close(struct hw_isp_media_dev *isp_md)
{
	if (isp_md == NULL)
		return;

	media_close(isp_md->mdev);
	free(isp_md);
}

int isp_video_open(struct hw_isp_media_dev *isp_md, unsigned int id)
{
	int ret = -1;
	struct isp_video_device *video;

	if (id >= HW_VIDEO_DEVICE_NUM)
		return ret;
	if (NULL != isp_md->video_dev[id])
		return 0;

	video = malloc(sizeof(struct isp_video_device));
	if (video == NULL)
		goto error;
	memset(video, 0, sizeof(*video));

	video->id = id;

	video_set_priv_data(video, isp_md);

	ret = video_init(video);
	if (ret < 0) {
		ISP_ERR("error: unable to initialize video device.\n");
		free(video);
		goto error;
	}

	video->isp_id = media_video_to_isp_id(video->entity);
	if (video->isp_id == -1) {
		ISP_ERR("error: unable to initialize video device.\n");
		video_cleanup(video);
		free(video);
		goto error;
	}

	ISP_PRINT("open video device[%d] success!\n", id);
	isp_md->video_dev[id] = video;
	return 0;
error:
	isp_md->video_dev[id] = NULL;
	ISP_ERR("unable to open video device[%d]!\n", id);
	return ret;
}

int isp_get_isp_id(int video_id)
{
	struct isp_video_device *video;
	struct hw_isp_media_dev *media_dev_isp = NULL;
	char name[32];
	int ret = -1;
	int isp_id = 0;

	if(media_dev_isp!=NULL)
	{
		ISP_PRINT("mpi_vi already init\n");
		return 0;
	}
	media_dev_isp = isp_md_open(MEDIA_DEVICE);

	if (media_dev_isp == NULL) {
		ISP_PRINT("unable to open media device %s\n", MEDIA_DEVICE);
		return -1;
	}

	if (video_id >= HW_VIDEO_DEVICE_NUM)
		return ret;
	if (NULL != media_dev_isp->video_dev[video_id])
		return 0;

	video = malloc(sizeof(struct isp_video_device));
	if (!video)
		return -1;
	memset(video, 0, sizeof(*video));

	video->id = video_id;
	video_set_priv_data(video, media_dev_isp);

	snprintf(name, sizeof(name), ENTITY_SUNXI_VIDEO"%d", video->id);
	ISP_PRINT("video device name is %s\n", name);
	video->entity = media_get_entity_by_name(media_dev_isp->mdev, name);
	if (!video->entity) {
		ISP_ERR("can not get entity by name %s\n", name);
		free(video);
		video = NULL;
		return -ENOENT;
	}
	isp_id = media_video_to_isp_id(video->entity);

	isp_md_close(media_dev_isp);
	free(video);

	return isp_id;
}
void isp_video_close(struct hw_isp_media_dev *isp_md, unsigned int id)
{
	if (isp_md == NULL)
		return;

	if (isp_md->video_dev[id]) {
		video_cleanup(isp_md->video_dev[id]);
		free(isp_md->video_dev[id]);
	}
	isp_md->video_dev[id] = NULL;
}

int isp_dev_start(struct hw_isp_device *isp)
{
	/* Start the isp lib. */
	isp_subdev_start(isp);

	/* Start the statistics engine. */
	isp_stats_start(isp);

	return 0;
}

int isp_dev_stop(struct hw_isp_device *isp)
{
	isp_subdev_stop(isp);
	isp_stats_stop(isp);

	return 0;
}

void isp_dev_register(struct hw_isp_device *isp,
				const struct isp_dev_operations *ops)
{
	isp->ops = ops;
}

void isp_dev_banding_ctx(struct hw_isp_device *isp, void *ctx)
{
	isp->ctx = ctx;
}

void isp_dev_unbanding_ctx(struct hw_isp_device *isp)
{
	isp->ctx = NULL;
}

void *isp_dev_get_ctx(struct hw_isp_device *isp)
{
	if (!isp->ctx) {
		ISP_ERR("ISP[%d] CTX is NULL.\n", isp->id);
		return NULL;
	}
	return isp->ctx;
}

void isp_dev_banding_tuning(struct hw_isp_device *isp, void *tuning)
{
	isp->tuning = tuning;
}

void isp_dev_unbanding_tuning(struct hw_isp_device *isp)
{
	isp->tuning = NULL;
}

void *isp_dev_get_tuning(struct hw_isp_device *isp)
{
	if (NULL == isp->tuning) {
		ISP_ERR("ISP[%d] tuning is NULL.\n", isp->id);
		return NULL;
	}

	return isp->tuning;
}

char *isp_dev_get_sensor_name(struct hw_isp_device *isp)
{
	if (NULL == isp) {
		ISP_ERR("ISP device is NULL.\n");
		return NULL;
	}

	return isp->sensor.entity.info.name;
}

int isp_dev_get_dev_id(struct hw_isp_device *isp)
{
	if (NULL == isp) {
		ISP_ERR("ISP device is NULL.\n");
		return -1;
	}

	return isp->id;
}

