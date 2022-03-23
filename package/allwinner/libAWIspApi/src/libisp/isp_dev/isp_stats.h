
/*
 ******************************************************************************
 *
 * isp_stats.h
 *
 * Hawkview ISP - isp_stats.h module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/17	VIDEO INPUT
 *
 *****************************************************************************
 */

#ifndef _ISP_STATS_H_
#define _ISP_STATS_H_

#include <stdint.h>

struct hw_isp_device;

/*
 * isp_stats_enable - Enable or disable the statistics engine
 * @isp: The ISP device
 * @enable: Whether to enable to disable the statistics engine
 *
 * The statistics engine must be enabled prior to starting the video stream.
 * When enabled, it statistics will be computed for every frame and delivered
 * through the ISP aewb_ready() callback.
 */
void isp_stats_enable(struct hw_isp_device *isp, bool enable);

#endif /*_ISP_STATS_H_*/
