
/*
 ******************************************************************************
 *
 * isp_subdev.h
 *
 * Hawkview ISP - isp_subdev.h module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/17	VIDEO INPUT
 *
 *****************************************************************************
 */

#ifndef _ISP_DEV_H_
#define _ISP_DEV_H_
#include <linux/videodev2.h>
#include "isp_dev_priv.h"

int isp_subdev_start(struct hw_isp_device *isp);
void isp_subdev_stop(struct hw_isp_device *isp);
int isp_subdev_init(struct hw_isp_device *isp);
void isp_subdev_exit(struct hw_isp_device *isp);

#endif /*_ISP_DEV_H_*/


