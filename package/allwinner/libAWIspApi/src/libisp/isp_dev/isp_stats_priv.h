
/*
 ******************************************************************************
 *
 * stats_priv.h
 *
 * Hawkview ISP - stats_priv.h module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/21	VIDEO INPUT
 *
 *****************************************************************************
 */
#ifndef _ISP_STATS_PRIV_H_
#define _ISP_STATS_PRIV_H_


#include <stdint.h>

#include <linux/v4l2-mediabus.h>
#include <linux/videodev2.h>
#include "../../include/isp_manage.h"

#include "isp_stats.h"

struct isp_stats {
	struct media_entity entity;
	unsigned int size;
	void *buffer;

	unsigned int pic_w;
	unsigned int pic_h;

	unsigned int win_x;
	unsigned int win_y;
	unsigned int win_n_x;
	unsigned int win_n_y;
	unsigned int win_w;
	unsigned int win_h;
	unsigned int win_inc_x;
	unsigned int win_inc_y;

	unsigned int saturation;
	bool enabled;
};

int isp_stats_start(struct hw_isp_device *isp);
void isp_stats_stop(struct hw_isp_device *isp);
int isp_stats_init(struct hw_isp_device *isp);
void isp_stats_exit(struct hw_isp_device *isp);
#endif /*_ISP_STATS_PRIV_H_*/

