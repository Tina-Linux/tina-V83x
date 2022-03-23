
/*
 ******************************************************************************
 *
 * hwisp_priv.h
 *
 * Hawkview ISP - hwisp_priv.h module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/18	VIDEO INPUT
 *
 *****************************************************************************
 */

#ifndef _HWISP_PRIV_H_
#define _HWISP_PRIV_H_

#include <linux/v4l2-mediabus.h>
#include <pthread.h>

#include "../include/device/isp_dev.h"
#include "isp_stats_priv.h"

#include "tools.h"

struct isp_sensor {
	struct media_entity entity;
};

struct isp_subdev {
	struct media_entity entity;
};

struct hw_isp_device {
	unsigned int id;
	struct isp_sensor sensor;
	struct isp_subdev isp_sd;
	struct isp_stats stats_sd;
	const struct isp_dev_operations *ops;
	void *priv;
	void *ctx;
	void *tuning;
};

static inline void hw_isp_set_priv_data(struct hw_isp_device *isp, void *p)
{
	isp->priv = p;
}

static inline void *hw_isp_get_priv_data(const struct hw_isp_device *isp)
{
	return isp->priv;
}


#endif /*_HWISP_PRIV_H_*/


