
/*
 ******************************************************************************
 *
 * stats.c
 *
 * Hawkview ISP - stats.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/17	VIDEO INPUT
 *
 *****************************************************************************
 */

#include <sys/ioctl.h>
#include <sys/time.h>

#include <linux/videodev2.h>

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/device/isp_dev.h"
#include "isp_dev_priv.h"
#include "isp_stats.h"
#include "isp_v4l2_helper.h"
#include "tools.h"

#define ENTITY_SUNXI_H3A		"sunxi_h3a"

static void isp_stats_process(struct hw_isp_device *isp,
			void *buffer, size_t size __attribute__((__unused__)))
{

	if(isp->ops->stats_ready)
		isp->ops->stats_ready(isp, buffer);
	else
		ISP_DEV_LOG(ISP_LOG_STAT, "Stats: stats_ready is NULL.\n");
}

static void isp_stats_event(void *priv)
{
	struct hw_isp_device *isp = priv;
	struct isp_stats *stats_sd = &isp->stats_sd;
	struct vin_isp_stat_data data;
	struct v4l2_event event;
	struct vin_isp_stat_event_status *status = (struct vin_isp_stat_event_status *)event.u.data;
	int ret;

	memset(&event, 0, sizeof event);
	ret = ioctl(stats_sd->entity.fd, VIDIOC_DQEVENT, &event);
	if (ret < 0) {
		ISP_ERR("unable to retrieve AEWB event: %s (%d).\n",
			strerror(errno), errno);
		return;
	}

	if (status->buf_err) {
		ISP_ERR("AEWB: stats error, skipping buffer.\n");
		return;
	}

	memset(&data, 0, sizeof data);
	data.buf = stats_sd->buffer;
	data.buf_size = stats_sd->size;

	ret = ioctl(stats_sd->entity.fd, VIDIOC_VIN_ISP_STAT_REQ, &data);
	if (ret < 0) {
		ISP_ERR("unable to retrieve AEWB data: %s (%d).\n",
			strerror(errno), errno);
		return;
	}
	//ISP_PRINT("isp%d process %d stat buffer!\n", isp->id, data.frame_number);
	isp_stats_process(isp, data.buf, data.buf_size);
}

static int isp_stats_setup(struct hw_isp_device *isp)
{
	struct isp_stats *stats_sd = &isp->stats_sd;
	struct vin_isp_h3a_config config;
	int ret;

	config.buf_size = ISP_STAT_TOTAL_SIZE;
	ret = ioctl(stats_sd->entity.fd, VIDIOC_VIN_ISP_H3A_CFG, &config);
	if (ret < 0)
		return -errno;

	stats_sd->size = config.buf_size;
	stats_sd->buffer = malloc(config.buf_size);
	if (stats_sd->buffer == NULL)
		return -ENOMEM;

	return 0;
}

/* -----------------------------------------------------------------------------
 * Start/stop, init/cleanup
 */
void isp_stats_enable(struct hw_isp_device *isp, bool enable)
{
	struct isp_stats *stats_sd = &isp->stats_sd;

	stats_sd->enabled = enable;
}

int isp_stats_start(struct hw_isp_device *isp)
{
	struct isp_stats *stats_sd = &isp->stats_sd;
	struct v4l2_event_subscription esub;
	unsigned int enable = 1;
	int ret;

	if (!stats_sd->enabled)
		return 0;

	ret = isp_stats_setup(isp);
	if (ret < 0) {
		ISP_ERR("unable to configure AEWB engine: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	memset(&esub, 0, sizeof(struct v4l2_event_subscription));
	esub.id = 0;
	esub.type = V4L2_EVENT_VIN_H3A;
	ret = ioctl(stats_sd->entity.fd, VIDIOC_SUBSCRIBE_EVENT, &esub);
	if (ret < 0) {
		ISP_ERR("unable to subscribe to AEWB event: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}
	ret = ioctl(stats_sd->entity.fd, VIDIOC_VIN_ISP_STAT_EN, &enable);
	if (ret < 0) {
		ISP_ERR("unable to start AEWB engine: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	isp->ops->monitor_fd(isp->id, stats_sd->entity.fd, HW_ISP_EVENT_EXCEPTION,
			   isp_stats_event, isp);

	return 0;
}

void isp_stats_stop(struct hw_isp_device *isp)
{
	struct isp_stats *stats_sd = &isp->stats_sd;
	struct v4l2_event_subscription esub;
	unsigned int enable = 0;

	if (!stats_sd->enabled)
		return;

	isp->ops->unmonitor_fd(isp->id, stats_sd->entity.fd);
	ioctl(stats_sd->entity.fd, VIDIOC_VIN_ISP_STAT_EN, &enable);

	memset(&esub, 0, sizeof esub);
	esub.type = V4L2_EVENT_VIN_H3A;
	ioctl(stats_sd->entity.fd, VIDIOC_UNSUBSCRIBE_EVENT, &esub);

	if (stats_sd->buffer != NULL) {
		free(stats_sd->buffer);
		stats_sd->buffer = NULL;
	}
}

int isp_stats_init(struct hw_isp_device *isp)
{
	struct media_entity *entity = NULL;
	struct hw_isp_media_dev *media = hw_isp_get_priv_data(isp);
	char name[32];

	snprintf(name, sizeof(name), ENTITY_SUNXI_H3A".%d", isp->id);
	ISP_DEV_LOG(ISP_LOG_STAT, "stats device name is %s\n", name);
	entity = media_get_entity_by_name(media->mdev, name);
	if (entity == NULL)
		return -ENOENT;

	isp->stats_sd.entity = *entity;

	return v4l2_subdev_open(&isp->stats_sd.entity);
}

void isp_stats_exit(struct hw_isp_device *isp)
{
	v4l2_subdev_close(&isp->stats_sd.entity);
}

