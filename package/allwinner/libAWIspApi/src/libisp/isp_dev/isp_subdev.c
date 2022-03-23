
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
 *   3.0		  Yang Feng   	2016/03/17	VIDEO INPUT
 *
 *****************************************************************************
 */

#include <errno.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "isp_v4l2_helper.h"
#include "../include/isp_debug.h"
#include "isp_subdev.h"
#include "../include/device/isp_dev.h"
#include "isp_dev_priv.h"

#define ENTITY_SUNXI_ISP	"sunxi_isp"

struct isp_cid {
	char name[64];
	int id;
};

struct isp_cid isp_cid_array[] = {
#if 1
	{ "V4L2_CID_BRIGHTNESS", V4L2_CID_BRIGHTNESS, },
	{ "V4L2_CID_CONTRAST", V4L2_CID_CONTRAST, },
	{ "V4L2_CID_SATURATION", V4L2_CID_SATURATION, },
	{ "V4L2_CID_HUE", V4L2_CID_HUE, },
	{ "V4L2_CID_AUTO_WHITE_BALANCE", V4L2_CID_AUTO_WHITE_BALANCE, },
	{ "V4L2_CID_EXPOSURE", V4L2_CID_EXPOSURE, },
	{ "V4L2_CID_AUTOGAIN", V4L2_CID_AUTOGAIN, },
	{ "V4L2_CID_GAIN", V4L2_CID_GAIN, },

	{ "V4L2_CID_POWER_LINE_FREQUENCY", V4L2_CID_POWER_LINE_FREQUENCY, },
	{ "V4L2_CID_HUE_AUTO", V4L2_CID_HUE_AUTO, },
	{ "V4L2_CID_WHITE_BALANCE_TEMPERATURE", V4L2_CID_WHITE_BALANCE_TEMPERATURE, },
	{ "V4L2_CID_SHARPNESS", V4L2_CID_SHARPNESS, },
	{ "V4L2_CID_CHROMA_AGC", V4L2_CID_CHROMA_AGC, },
	{ "V4L2_CID_COLORFX", V4L2_CID_COLORFX, },
	{ "V4L2_CID_AUTOBRIGHTNESS", V4L2_CID_AUTOBRIGHTNESS, },
	{ "V4L2_CID_BAND_STOP_FILTER", V4L2_CID_BAND_STOP_FILTER, },
	{ "V4L2_CID_ILLUMINATORS_1", V4L2_CID_ILLUMINATORS_1, },
	{ "V4L2_CID_ILLUMINATORS_2", V4L2_CID_ILLUMINATORS_2, },

	{ "V4L2_CID_EXPOSURE_AUTO", V4L2_CID_EXPOSURE_AUTO, },
	{ "V4L2_CID_EXPOSURE_ABSOLUTE", V4L2_CID_EXPOSURE_ABSOLUTE, },

	{ "V4L2_CID_EXPOSURE_AUTO_PRIORITY", V4L2_CID_EXPOSURE_AUTO_PRIORITY, },
	{ "V4L2_CID_FOCUS_ABSOLUTE", V4L2_CID_FOCUS_ABSOLUTE, },
	{ "V4L2_CID_FOCUS_RELATIVE", V4L2_CID_FOCUS_RELATIVE, },
	{ "V4L2_CID_FOCUS_AUTO", V4L2_CID_FOCUS_AUTO, },
	{ "V4L2_CID_AUTO_EXPOSURE_BIAS", V4L2_CID_AUTO_EXPOSURE_BIAS, },
	{ "V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE", V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE, },

	{ "V4L2_CID_WIDE_DYNAMIC_RANGE", V4L2_CID_WIDE_DYNAMIC_RANGE, },
	{ "V4L2_CID_IMAGE_STABILIZATION", V4L2_CID_IMAGE_STABILIZATION, },

	{ "V4L2_CID_ISO_SENSITIVITY", V4L2_CID_ISO_SENSITIVITY, },
	{ "V4L2_CID_ISO_SENSITIVITY_AUTO", V4L2_CID_ISO_SENSITIVITY_AUTO, },

	{ "V4L2_CID_EXPOSURE_METERING", V4L2_CID_EXPOSURE_METERING, },
	{ "V4L2_CID_SCENE_MODE", V4L2_CID_SCENE_MODE, },

	{ "V4L2_CID_3A_LOCK", V4L2_CID_3A_LOCK, },
	{ "V4L2_CID_AUTO_FOCUS_START", V4L2_CID_AUTO_FOCUS_START, },
	{ "V4L2_CID_AUTO_FOCUS_STOP", V4L2_CID_AUTO_FOCUS_STOP, },
	{ "V4L2_CID_AUTO_FOCUS_RANGE", V4L2_CID_AUTO_FOCUS_RANGE, },
#if 0
	{ "V4L2_CID_TAKE_PICTURE", V4L2_CID_TAKE_PICTURE, },
	{ "V4L2_CID_FLASH_LED_MODE", V4L2_CID_FLASH_LED_MODE, },
#endif
#endif
	{ "AE windows x1", V4L2_CID_AE_WIN_X1, },
	{ "AE windows y1", V4L2_CID_AE_WIN_Y1, },
	{ "AE windows x2", V4L2_CID_AE_WIN_X2, },
	{ "AE windows Y2", V4L2_CID_AE_WIN_Y2, },


	{ "AF windows x1", V4L2_CID_AF_WIN_X1, },
	{ "AF windows y1", V4L2_CID_AF_WIN_Y1, },
	{ "AF windows x2", V4L2_CID_AF_WIN_X2, },
	{ "AF windows y2", V4L2_CID_AF_WIN_Y2, },
};

/* -----------------------------------------------------------------------------
 * isp subdev vsync
 */

static void isp_vsync_event(struct hw_isp_device *isp,
						struct v4l2_event* event)
{
	ISP_DEV_LOG(ISP_LOG_EVENTS, "enter vsync event.\n");

	if(isp->ops->vsync)
		isp->ops->vsync(isp);

	ISP_DEV_LOG(ISP_LOG_EVENTS, "ISP%d Vsync done.\n", isp->id);
}

/* -----------------------------------------------------------------------------
 * isp subdev s_ctrl
 */

static void isp_ctrl_event(struct hw_isp_device *isp,
						struct v4l2_event* event)
{
	int i;

	ISP_DEV_LOG(ISP_LOG_EVENTS, "enter s_ctrl event.\n");

	for(i = 0; i < array_size(isp_cid_array); i++) {
		if (isp_cid_array[i].id == event->id)
			ISP_PRINT(" name is %s, value is 0x%x\n",
				isp_cid_array[i].name, event->u.ctrl.value);
	}

	if(isp->ops->ctrl_process)
		isp->ops->ctrl_process(isp, event);
}



/* -----------------------------------------------------------------------------
 * isp subdev frame done
 */

static void isp_fsync_event(struct hw_isp_device *isp,
					struct v4l2_event* event)
{
	ISP_DEV_LOG(ISP_LOG_EVENTS, "enter frame sync event.\n");

	if(isp->ops->fsync)
		isp->ops->fsync(isp);

	ISP_DEV_LOG(ISP_LOG_EVENTS, "ISP%d frame done.\n", isp->id);
}


/* -----------------------------------------------------------------------------
 * isp subdev stream off
 */

static void isp_streamoff_event(struct hw_isp_device *isp,
					struct v4l2_event* event)
{
	ISP_PRINT("enter isp%d stream off event.\n", isp->id);

	if(isp->ops->stream_off)
		isp->ops->stream_off(isp);

	ISP_DEV_LOG(ISP_LOG_EVENTS, "ISP%d stream off.\n", isp->id);
}

/* -----------------------------------------------------------------------------
 * isp subdev frame done
 */

static void isp_handle_event(void *priv)
{
	struct hw_isp_device *isp = priv;
	struct v4l2_event event;
	int ret;
	memset(&event, 0, sizeof event);

	ret = ioctl(isp->isp_sd.entity.fd, VIDIOC_DQEVENT, &event);
	if (ret < 0) {
		ISP_ERR("unable to retrieve isp subdev event: %s (%d)\n",
			strerror(errno), errno);
		return;
	}

	switch(event.type) {
	case V4L2_EVENT_VSYNC:
		isp_vsync_event(isp, &event);
		break;
	case V4L2_EVENT_CTRL:
		isp_ctrl_event(isp, &event);
		break;
	case V4L2_EVENT_FRAME_SYNC:
		isp_fsync_event(isp, &event);
		break;
	case V4L2_EVENT_VIN_ISP_OFF:
		isp_streamoff_event(isp, &event);
		break;
	default:
		ISP_ERR("Unknown enent.\n");
	}
}

int isp_set_load_reg(struct hw_isp_device *isp, struct isp_table_reg_map *reg)
{

	struct isp_subdev *isp_sd = &isp->isp_sd;
	int ret = 0;
	ret = ioctl(isp_sd->entity.fd, VIDIOC_VIN_ISP_LOAD_REG, reg);
	if (ret)
		ISP_ERR("VIDIOC_VIN_ISP_LOAD_REG error!\n");
	return ret;
}

int isp_set_table1_map(struct hw_isp_device *isp, struct isp_table_reg_map *tbl)
{

	struct isp_subdev *isp_sd = &isp->isp_sd;
	int ret = 0;
	ret = ioctl(isp_sd->entity.fd, VIDIOC_VIN_ISP_TABLE1_MAP, tbl);
	if (ret)
		ISP_ERR("VIDIOC_VIN_ISP_TABLE_MAPPING error!\n");
	return ret;
}

int isp_set_table2_map(struct hw_isp_device *isp, struct isp_table_reg_map *tbl)
{

	struct isp_subdev *isp_sd = &isp->isp_sd;
	int ret = 0;
	ret = ioctl(isp_sd->entity.fd, VIDIOC_VIN_ISP_TABLE2_MAP, tbl);
	if (ret)
		ISP_ERR("VIDIOC_VIN_ISP_TABLE_MAPPING error!\n");
	return ret;
}

int isp_subdev_start(struct hw_isp_device *isp)
{
	struct v4l2_event_subscription esub;
	int i, ret;

	memset(&esub, 0, sizeof(struct v4l2_event_subscription));
	esub.id = 0;
	esub.type = V4L2_EVENT_VSYNC;
	ret = ioctl(isp->isp_sd.entity.fd, VIDIOC_SUBSCRIBE_EVENT, &esub);
	if (ret < 0) {
		ISP_ERR("unable to subscribe to vsync event: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	memset(&esub, 0, sizeof(struct v4l2_event_subscription));
	esub.id = 0;
	esub.type = V4L2_EVENT_FRAME_SYNC;
	ret = ioctl(isp->isp_sd.entity.fd, VIDIOC_SUBSCRIBE_EVENT, &esub);
	if (ret < 0) {
		ISP_ERR("unable to subscribe to frame sync event: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	memset(&esub, 0, sizeof(struct v4l2_event_subscription));
	esub.id = 0;
	esub.type = V4L2_EVENT_VIN_ISP_OFF;
	ret = ioctl(isp->isp_sd.entity.fd, VIDIOC_SUBSCRIBE_EVENT, &esub);
	if (ret < 0) {
		ISP_ERR("unable to subscribe to stream off event: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	for(i = 0; i < array_size(isp_cid_array); i++) {
		memset(&esub, 0, sizeof(struct v4l2_event_subscription));
		esub.id = isp_cid_array[i].id;
		esub.type = V4L2_EVENT_CTRL;
		//esub.flags = V4L2_EVENT_SUB_FL_SEND_INITIAL | V4L2_EVENT_SUB_FL_ALLOW_FEEDBACK;
		ret = ioctl(isp->isp_sd.entity.fd, VIDIOC_SUBSCRIBE_EVENT, &esub);
		if (ret < 0) {
			ISP_ERR("unable to subscribe to ctrl event: %s (%d).\n",
				strerror(errno), errno);
			//return ret;
		}
	}

	isp->ops->monitor_fd(isp->id, isp->isp_sd.entity.fd, HW_ISP_EVENT_EXCEPTION,
			   isp_handle_event, isp);
	return 0;
}

void isp_subdev_stop(struct hw_isp_device *isp)
{
	struct isp_subdev *isp_sd = &isp->isp_sd;
	struct v4l2_event_subscription esub;

	isp->ops->unmonitor_fd(isp->id, isp_sd->entity.fd);

	memset(&esub, 0, sizeof esub);
	esub.type = V4L2_EVENT_ALL;
	ioctl(isp_sd->entity.fd, VIDIOC_UNSUBSCRIBE_EVENT, &esub);
}

int isp_subdev_init(struct hw_isp_device *isp)
{
	struct media_entity *entity = NULL;
	struct hw_isp_media_dev *media = hw_isp_get_priv_data(isp);
	char name[32];

	snprintf(name, sizeof(name), ENTITY_SUNXI_ISP".%d", isp->id);
	entity = media_get_entity_by_name(media->mdev, name);
	if (entity == NULL)
		return -ENOENT;

	isp->isp_sd.entity = *entity;

	return v4l2_subdev_open(&isp->isp_sd.entity);
}

void isp_subdev_exit(struct hw_isp_device *isp)
{
	v4l2_subdev_close(&isp->isp_sd.entity);
}

/* -----------------------------------------------------------------------------
 * isp sensor api helper
 */
int isp_sensor_get_exposure(struct hw_isp_device *isp,
				  unsigned int *exposure)
{
	struct v4l2_ext_control ctrls[1];
	int ret;

	ctrls[0].id = V4L2_CID_EXPOSURE;

	ret = v4l2_get_controls(&isp->sensor.entity, array_size(ctrls), ctrls);
	if (ret < 0)
		return ret;

	*exposure = ctrls[0].value;
	return 0;
}

int isp_sensor_set_exposure(struct hw_isp_device *isp,
				  unsigned int exposure)
{
	struct v4l2_ext_control ctrls[1];

	ctrls[0].id = V4L2_CID_EXPOSURE;
	ctrls[0].value = exposure;

	return v4l2_set_controls(&isp->sensor.entity, array_size(ctrls), ctrls);
}

int isp_sensor_set_gain(struct hw_isp_device *isp, unsigned int gain)
{
	struct v4l2_ext_control ctrls[1];

	ctrls[0].id = V4L2_CID_GAIN;
	ctrls[0].value = gain;

	return v4l2_set_controls(&isp->sensor.entity, array_size(ctrls), ctrls);
}

int isp_sensor_get_configs(struct hw_isp_device *isp,
						  struct sensor_config *cfg)
{
	int ret;
	ret = ioctl(isp->sensor.entity.fd, VIDIOC_VIN_SENSOR_CFG_REQ, cfg);
	if (ret < 0)
		ISP_ERR("%s get config failed: %s (%d).\n",
			isp->sensor.entity.info.name, strerror(errno), errno);

	ISP_DEV_LOG(ISP_LOG_SUBDEV, "get sensor config,\n"
		" width: %d, height: %d,\n hoffset: %d, voffset: %d\n"
		" hts: %d, vts: %d,\n pclk: %d, bin_factor: %d\n"
		" intg_min: %d, intg_max: %d,\n gain_min: %d, gain_max: %d\n"
		" mbus code: 0x%08x, wdr mode: 0x%08x\n",
		cfg->width, cfg->height, cfg->hoffset, cfg->voffset,
		cfg->hts, cfg->vts, cfg->pclk, cfg->bin_factor,
		cfg->intg_min, cfg->intg_max, cfg->gain_min, cfg->gain_max,
		cfg->mbus_code, cfg->wdr_mode);
	return ret;
}

int isp_sensor_set_exp_gain(struct hw_isp_device *isp,
						  struct sensor_exp_gain *exp_gain)
{
	int ret;
	ret = ioctl(isp->sensor.entity.fd, VIDIOC_VIN_SENSOR_EXP_GAIN, exp_gain);
	if (ret < 0)
		ISP_ERR("%s set exp and gain failed: %s (%d).\n",
			isp->sensor.entity.info.name, strerror(errno), errno);

	ISP_DEV_LOG(ISP_LOG_SUBDEV, "set sensor exp gain, exp_val: %d, gain_val: %d, r_gain: %d, b_gain: %d.\n",
		exp_gain->exp_val, exp_gain->gain_val, exp_gain->r_gain, exp_gain->b_gain);
	return ret;
}

int isp_sensor_set_fps(struct hw_isp_device *isp, struct sensor_fps *fps)
{
	int ret;
	ret = ioctl(isp->sensor.entity.fd, VIDIOC_VIN_SENSOR_SET_FPS, fps);
	if (ret < 0)
		ISP_ERR("%s set fps failed: %s (%d).\n",
			isp->sensor.entity.info.name, strerror(errno), errno);
	return ret;
}

int isp_sensor_get_temp(struct hw_isp_device *isp, struct sensor_temp *temp)
{

	int ret = 0;
	ret = ioctl(isp->sensor.entity.fd, VIDIOC_VIN_SENSOR_GET_TEMP, temp);
	if (ret < 0)
		ISP_ERR("%s get sensor_temp failed: %s (%d).\n",
			isp->sensor.entity.info.name, strerror(errno), errno);
	return ret;
}

int isp_act_init_range(struct hw_isp_device *isp, unsigned int min, unsigned int max)
{
	struct actuator_para vcm_range;

	vcm_range.code_min = min;
	vcm_range.code_max = max;

	if (-1 == ioctl(isp->sensor.entity.fd, VIDIOC_VIN_ACT_INIT, &vcm_range)) {
		//ISP_ERR("%s init act range failed: %s (%d).\n",
		//	isp->sensor.entity.info.name, strerror(errno), errno);
		return -1;
	}
	return 0;
}

int isp_act_set_pos(struct hw_isp_device *isp, unsigned int pos)
{
	struct actuator_ctrl vcm_pos;

	vcm_pos.code = pos;

	if (-1 == ioctl(isp->sensor.entity.fd, VIDIOC_VIN_ACT_SET_CODE, &vcm_pos)) {
		//ISP_ERR("%s set act position failed: %s (%d).\n",
		//	isp->sensor.entity.info.name, strerror(errno), errno);
		return -1;
	}
	return 0;
}

int isp_flash_init(struct hw_isp_device *isp, int mode)
{
	int ret = 0;
	return ret;
}

int isp_flash_ctrl(struct hw_isp_device *isp, int mode)
{
	struct flash_para sflash_para;

	sflash_para.mode = mode;

	if (-1 == ioctl(isp->sensor.entity.fd, VIDIOC_VIN_FLASH_EN, &sflash_para)) {
		//ISP_ERR("%s set act position failed: %s (%d).\n",
		//	isp->sensor.entity.info.name, strerror(errno), errno);
		return -1;
	}
	return 0;
}

