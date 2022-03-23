
/*
 ******************************************************************************
 *
 * iq.c
 *
 * Hawkview ISP - iq.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/22	VIDEO INPUT
 *
 *****************************************************************************
 */

#include <errno.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "device/isp_dev.h"
#include "../isp_dev/tools.h"
#include "isp_ini_parse.h"

#include "isp_tuning_priv.h"
#include "../include/isp_tuning.h"

enum isp_sense_config {
	ISP_SCENE_CONFIG_0 = 0,
	ISP_SCENE_CONFIG_1,
	ISP_SCENE_CONFIG_2,
	ISP_SCENE_CONFIG_3,
	ISP_SCENE_CONFIG_4,
	ISP_SCENE_CONFIG_5,
	ISP_SCENE_CONFIG_6,
	ISP_SCENE_CONFIG_7,
	ISP_SCENE_CONFIG_8,

	ISP_SCENE_CONFIG_MAX,
};

struct isp_tuning {
	struct hw_isp_device *isp;
	struct isp_lib_context *ctx;
	struct isp_param_config params;
	unsigned int frame_count;
	unsigned int pix_max;
	pthread_mutex_t mutex;
};

int isp_params_parse(struct hw_isp_device *isp, struct isp_param_config *params, int ir, int wdr, int sync_mode)
{
	struct isp_lib_context *ctx = isp_dev_get_ctx(isp);

	if (ctx == NULL)
		return -1;

	return parser_ini_info(params, ctx->sensor_info.name,
		ctx->sensor_info.sensor_width, ctx->sensor_info.sensor_height,
        ctx->sensor_info.fps_fixed, wdr, ir, sync_mode, ctx->isp_id);
}

int isp_sensor_otp_init(struct hw_isp_device *isp)
{
	struct isp_lib_context *ctx = isp_dev_get_ctx(isp);
	int ret = 0;
	int i = 0;

	if (ctx == NULL)
		return -1;

	// shading
	ctx->pmsc_table = malloc(ISP_MSC_TBL_LENGTH*sizeof(unsigned short));
	memset(ctx->pmsc_table, 0, ISP_MSC_TBL_LENGTH*sizeof(unsigned short));

	if (!ctx->pmsc_table) {
		ISP_ERR("msc_table alloc failed, no memory!\n");
		return -1;
	}

	ctx->otp_enable = 1;
	ret = ioctl(isp->sensor.fd, VIDIOC_VIN_GET_SENSOR_OTP_INFO, ctx->pmsc_table);
	if(ret < 0) {
		ISP_ERR("VIDIOC_VIN_GET_SENSOR_OTP_INFO return error:%s\n", strerror(errno));
		ctx->otp_enable = -1;
	}
	//ctx->otp_enable = -1;
	ctx->pwb_table = (void *)((HW_U64)(ctx->pmsc_table) + 16*16*3*sizeof(unsigned short));

	return 0;
}

int isp_sensor_otp_exit(struct hw_isp_device *isp)
{
	struct isp_lib_context *ctx = isp_dev_get_ctx(isp);

	if (ctx == NULL)
		return -1;

	// shading
	free(ctx->pmsc_table);

	return 0;
}

int isp_config_sensor_info(struct hw_isp_device *isp)
{
	struct sensor_config cfg;
	struct isp_lib_context *ctx = isp_dev_get_ctx(isp);
	int i = 0, j = 0;
	memset(&cfg, 0, sizeof(cfg));

	if (ctx == NULL)
		return -1;

	ctx->sensor_info.name = isp_dev_get_sensor_name(isp);
	if (!ctx->sensor_info.name)
		return -1;

	isp_sensor_get_configs(isp, &cfg);
	FUNCTION_LOG;

	ctx->sensor_info.sensor_width = cfg.width;
	ctx->sensor_info.sensor_height = cfg.height;
	ctx->sensor_info.fps_fixed = cfg.fps_fixed;
	ctx->sensor_info.wdr_mode = cfg.wdr_mode;
	// BT601_FULL_RANGE
	ctx->sensor_info.color_space = 0;

	switch (cfg.mbus_code) {
	case V4L2_MBUS_FMT_SBGGR8_1X8:
	case V4L2_MBUS_FMT_SBGGR10_1X10:
	case V4L2_MBUS_FMT_SBGGR12_1X12:
		ctx->sensor_info.input_seq = ISP_BGGR;
		break;
	case V4L2_MBUS_FMT_SGBRG8_1X8:
	case V4L2_MBUS_FMT_SGBRG10_1X10:
	case V4L2_MBUS_FMT_SGBRG12_1X12:
		ctx->sensor_info.input_seq = ISP_GBRG;
		break;
	case V4L2_MBUS_FMT_SGRBG8_1X8:
	case V4L2_MBUS_FMT_SGRBG10_1X10:
	case V4L2_MBUS_FMT_SGRBG12_1X12:
		ctx->sensor_info.input_seq = ISP_GRBG;
		break;
	case V4L2_MBUS_FMT_SRGGB8_1X8:
	case V4L2_MBUS_FMT_SRGGB10_1X10:
	case V4L2_MBUS_FMT_SRGGB12_1X12:
		ctx->sensor_info.input_seq = ISP_RGGB;
		break;
	default:
		ctx->sensor_info.input_seq = ISP_BGGR;
		break;
	}

	if (cfg.hts && cfg.vts && cfg.pclk) {
		ctx->sensor_info.hts = cfg.hts;
		ctx->sensor_info.vts = cfg.vts;
		ctx->sensor_info.pclk = cfg.pclk;
		ctx->sensor_info.bin_factor = cfg.bin_factor;
		ctx->sensor_info.gain_min = cfg.gain_min;
		ctx->sensor_info.gain_max = cfg.gain_max;
		ctx->sensor_info.hoffset = cfg.hoffset;
		ctx->sensor_info.voffset = cfg.voffset;

	} else {
		ctx->sensor_info.hts = cfg.width;
		ctx->sensor_info.vts = cfg.height;
		ctx->sensor_info.pclk = cfg.width * cfg.height * 30;
		ctx->sensor_info.bin_factor = 1;
		ctx->sensor_info.gain_min = 16;
		ctx->sensor_info.gain_max = 255;
		ctx->sensor_info.hoffset = 0;
		ctx->sensor_info.voffset = 0;
	}

	ctx->stat.pic_size.width = cfg.width;
	ctx->stat.pic_size.height = cfg.height;

	ctx->stats_ctx.pic_w = cfg.width;
	ctx->stats_ctx.pic_h = cfg.height;
#if 1
	// update otp infomation
	if(ctx->otp_enable == -1){
		ISP_PRINT("otp disabled, msc use 1024\n");
		for(i = 0; i < 16*16*3; i++) {
			((unsigned short*)(ctx->pmsc_table))[i] = 1024;
		}
	}
#endif
	return 0;
}

int isp_tuning_update(struct hw_isp_device *isp)
{
	struct isp_tuning *tuning;

	tuning = isp_dev_get_tuning(isp);
	if (tuning == NULL)
		return -1;

	if (tuning->ctx == NULL)
		return -1;
	pthread_mutex_lock(&tuning->mutex);
	tuning->ctx->isp_ini_cfg = tuning->params;
	pthread_mutex_unlock(&tuning->mutex);
	isp_ctx_config_update(tuning->ctx);
	return 0;
}

int isp_tuning_reset(struct hw_isp_device *isp, struct isp_param_config *param)
{
	struct isp_tuning *tuning;

	tuning = isp_dev_get_tuning(isp);
	if (tuning == NULL)
		return -1;

	if (tuning->ctx == NULL)
		return -1;
	pthread_mutex_lock(&tuning->mutex);
	tuning->params = *param;
	pthread_mutex_unlock(&tuning->mutex);
	isp_ctx_config_reset(tuning->ctx);
	return 0;
}

struct isp_tuning * isp_tuning_init(struct hw_isp_device *isp,
			const struct isp_param_config *params)
{
	struct isp_tuning *tuning;

	tuning = malloc(sizeof(struct isp_tuning));
	if (tuning == NULL)
		return NULL;
	memset(tuning, 0, sizeof(*tuning));

	tuning->isp = isp;
	tuning->frame_count = 0;

	tuning->ctx = isp_dev_get_ctx(isp);
	if (tuning->ctx == NULL) {
		ISP_ERR("ISP context is not init!\n");
		free(tuning);
		return NULL;
	}
	FUNCTION_LOG;
	tuning->params = *params;

	isp_ctx_config_init(tuning->ctx);

	isp_dev_banding_tuning(isp, tuning);

	FUNCTION_LOG;

	pthread_mutex_init(&tuning->mutex, NULL);

	return tuning;
}

void isp_tuning_exit(struct hw_isp_device *isp)
{
	struct isp_tuning *tuning;

	tuning = isp_dev_get_tuning(isp);
	if (tuning == NULL)
		return;
	pthread_mutex_destroy(&tuning->mutex);
	isp_ctx_config_exit(tuning->ctx);
	isp_dev_unbanding_tuning(isp);
	free(tuning);
}

HW_S32 isp_tuning_get_cfg(struct hw_isp_device *isp, HW_U8 group_id, HW_U32 cfg_ids, void *cfg_data)
{
	HW_S32 ret = AW_ERR_VI_INVALID_PARA;
	unsigned char *data_ptr = NULL;
	struct isp_tuning *tuning = NULL;

	if (!isp || !cfg_data)
		return AW_ERR_VI_INVALID_PARA;

	/* call isp api */
	tuning = isp_dev_get_tuning(isp);
	if (tuning == NULL)
		 return AW_ERR_VI_INVALID_NULL_PTR;

	/* fill cfg data */
	ret = 0;
	data_ptr = (unsigned char *)cfg_data;

	switch (group_id)
	{
	case HW_ISP_CFG_TEST: /* isp_test_param */
		if (cfg_ids & HW_ISP_CFG_TEST_PUB) /* isp_test_pub */
		{
			struct isp_test_pub_cfg *isp_test_pub = (struct isp_test_pub_cfg *)data_ptr;
			isp_test_pub->test_mode = tuning->params.isp_test_settings.isp_test_mode;
			isp_test_pub->gain = tuning->params.isp_test_settings.isp_gain;
			isp_test_pub->exp_line = tuning->params.isp_test_settings.isp_exp_line;
			isp_test_pub->color_temp = tuning->params.isp_test_settings.isp_color_temp;
			isp_test_pub->log_param = tuning->params.isp_test_settings.isp_log_param;

			/* offset */
			data_ptr += sizeof(struct isp_test_pub_cfg);
			ret += sizeof(struct isp_test_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_EXPTIME) /* isp_test_exptime */
		{
			struct isp_test_item_cfg *isp_test_exptime = (struct isp_test_item_cfg *)data_ptr;
			isp_test_exptime->enable = tuning->params.isp_test_settings.isp_test_exptime;
			isp_test_exptime->start = tuning->params.isp_test_settings.exp_line_start;
			isp_test_exptime->step = tuning->params.isp_test_settings.exp_line_step;
			isp_test_exptime->end = tuning->params.isp_test_settings.exp_line_end;
			isp_test_exptime->change_interval = tuning->params.isp_test_settings.exp_change_interval;

			/* offset */
			data_ptr += sizeof(struct isp_test_item_cfg);
			ret += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_GAIN) /* isp_test_gain */
		{
			struct isp_test_item_cfg *isp_test_gain = (struct isp_test_item_cfg *)data_ptr;
			isp_test_gain->enable = tuning->params.isp_test_settings.isp_test_gain;
			isp_test_gain->start = tuning->params.isp_test_settings.gain_start;
			isp_test_gain->step = tuning->params.isp_test_settings.gain_step;
			isp_test_gain->end = tuning->params.isp_test_settings.gain_end;
			isp_test_gain->change_interval = tuning->params.isp_test_settings.gain_change_interval;

			/* offset */
			data_ptr += sizeof(struct isp_test_item_cfg);
			ret += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_FOCUS) /* isp_test_focus */
		{
			struct isp_test_item_cfg *isp_test_focus = (struct isp_test_item_cfg *)data_ptr;
			isp_test_focus->enable = tuning->params.isp_test_settings.isp_test_focus;
			isp_test_focus->start = tuning->params.isp_test_settings.focus_start;
			isp_test_focus->step = tuning->params.isp_test_settings.focus_step;
			isp_test_focus->end = tuning->params.isp_test_settings.focus_end;
			isp_test_focus->change_interval = tuning->params.isp_test_settings.focus_change_interval;

			/* offset */
			data_ptr += sizeof(struct isp_test_item_cfg);
			ret += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_FORCED) /* isp_test_forced */
		{
			struct isp_test_forced_cfg *isp_test_forced = (struct isp_test_forced_cfg *)data_ptr;
			isp_test_forced->ae_enable = tuning->params.isp_test_settings.ae_forced;
			isp_test_forced->lum = tuning->params.isp_test_settings.lum_forced;

			/* offset */
			data_ptr += sizeof(struct isp_test_forced_cfg);
			ret += sizeof(struct isp_test_forced_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_ENABLE) /* isp_test_enable */
		{
			memcpy(data_ptr, &(tuning->params.isp_test_settings.manual_en), sizeof(struct isp_test_enable_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_test_enable_cfg);
			ret += sizeof(struct isp_test_enable_cfg);
		}
		break;
	case HW_ISP_CFG_3A: /* isp_3a_param */
		if (cfg_ids & HW_ISP_CFG_AE_PUB) /* isp_ae_pub */
		{
			struct isp_ae_pub_cfg *isp_ae_pub = (struct isp_ae_pub_cfg *)data_ptr;
			isp_ae_pub->define_table = tuning->params.isp_3a_settings.define_ae_table;
			isp_ae_pub->max_lv = tuning->params.isp_3a_settings.ae_max_lv;
			isp_ae_pub->hist_mode_en = tuning->params.isp_3a_settings.ae_hist_mod_en;
			isp_ae_pub->hist_sel = tuning->params.isp_3a_settings.ae_hist_sel;
			isp_ae_pub->stat_sel = tuning->params.isp_3a_settings.ae_stat_sel;
			isp_ae_pub->compensation_step = tuning->params.isp_3a_settings.exp_comp_step;
			isp_ae_pub->touch_dist_index = tuning->params.isp_3a_settings.ae_touch_dist_ind;
			isp_ae_pub->iso2gain_ratio = tuning->params.isp_3a_settings.ae_iso2gain_ratio;
			memcpy(&isp_ae_pub->fno_table[0], &tuning->params.isp_3a_settings.ae_fno_step[0],
				sizeof(isp_ae_pub->fno_table));
			isp_ae_pub->ki = tuning->params.isp_3a_settings.ae_ki;
			isp_ae_pub->conv_data_index = tuning->params.isp_3a_settings.ae_ConvDataIndex;
			isp_ae_pub->blowout_pre_en = tuning->params.isp_3a_settings.ae_blowout_pre_en;
			isp_ae_pub->blowout_attr = tuning->params.isp_3a_settings.ae_blowout_attr;
			memcpy(&isp_ae_pub->wdr_cfg[0], &tuning->params.isp_3a_settings.wdr_cfg[0],
				sizeof(isp_ae_pub->wdr_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_ae_pub_cfg);
			ret += sizeof(struct isp_ae_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_PREVIEW_TBL) /* isp_ae_preview_tbl */
		{
			struct isp_ae_table_cfg *isp_ae_preview_tbl = (struct isp_ae_table_cfg *)data_ptr;
			isp_ae_preview_tbl->length = tuning->params.isp_3a_settings.ae_table_preview_length;
			memcpy(&(isp_ae_preview_tbl->value[0]), tuning->params.isp_3a_settings.ae_table_preview,
				sizeof(isp_ae_preview_tbl->value));

			/* offset */
			data_ptr += sizeof(struct isp_ae_table_cfg);
			ret += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_CAPTURE_TBL) /* isp_ae_capture_tbl */
		{
			struct isp_ae_table_cfg *isp_ae_capture_tbl = (struct isp_ae_table_cfg *)data_ptr;
			isp_ae_capture_tbl->length = tuning->params.isp_3a_settings.ae_table_capture_length;
			memcpy(&(isp_ae_capture_tbl->value[0]), tuning->params.isp_3a_settings.ae_table_capture,
				sizeof(isp_ae_capture_tbl->value));

			/* offset */
			data_ptr += sizeof(struct isp_ae_table_cfg);
			ret += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_VIDEO_TBL) /* isp_ae_video_tbl */
		{
			struct isp_ae_table_cfg *isp_ae_video_tbl = (struct isp_ae_table_cfg *)data_ptr;
			isp_ae_video_tbl->length = tuning->params.isp_3a_settings.ae_table_video_length;
			memcpy(&(isp_ae_video_tbl->value[0]), tuning->params.isp_3a_settings.ae_table_video,
				sizeof(isp_ae_video_tbl->value));

			/* offset */
			data_ptr += sizeof(struct isp_ae_table_cfg);
			ret += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_WIN_WEIGHT) /* isp_ae_win_weight */
		{
			memcpy(data_ptr, tuning->params.isp_3a_settings.ae_win_weight, sizeof(struct isp_ae_weight_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_ae_weight_cfg);
			ret += sizeof(struct isp_ae_weight_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_DELAY) /* isp_ae_delay */
		{
			struct isp_ae_delay_cfg *isp_ae_delay = (struct isp_ae_delay_cfg *)data_ptr;
			isp_ae_delay->ae_frame = tuning->params.isp_3a_settings.ae_delay_frame;
			isp_ae_delay->exp_frame = tuning->params.isp_3a_settings.exp_delay_frame;
			isp_ae_delay->gain_frame = tuning->params.isp_3a_settings.gain_delay_frame;

			/* offset */
			data_ptr += sizeof(struct isp_ae_delay_cfg);
			ret += sizeof(struct isp_ae_delay_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_PUB) /* isp_awb_pub */
		{
			struct isp_awb_pub_cfg *isp_awb_pub = (struct isp_awb_pub_cfg *)data_ptr;
			isp_awb_pub->interval = tuning->params.isp_3a_settings.awb_interval;
			isp_awb_pub->speed = tuning->params.isp_3a_settings.awb_speed;
			isp_awb_pub->stat_sel = tuning->params.isp_3a_settings.awb_stat_sel;

			/* offset */
			data_ptr += sizeof(struct isp_awb_pub_cfg);
			ret += sizeof(struct isp_awb_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_TEMP_RANGE) /* isp_awb_temp_range */
		{
			struct isp_awb_temp_range_cfg *isp_awb_temp_range = (struct isp_awb_temp_range_cfg *)data_ptr;
			isp_awb_temp_range->low = tuning->params.isp_3a_settings.awb_color_temper_low;
			isp_awb_temp_range->high = tuning->params.isp_3a_settings.awb_color_temper_high;
			isp_awb_temp_range->base = tuning->params.isp_3a_settings.awb_base_temper;

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_range_cfg);
			ret += sizeof(struct isp_awb_temp_range_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_DIST) /* isp_awb_dist */
		{
			struct isp_awb_dist_cfg *isp_awb_dist = (struct isp_awb_dist_cfg *)data_ptr;
			isp_awb_dist->green_zone = tuning->params.isp_3a_settings.awb_green_zone_dist;
			isp_awb_dist->blue_sky = tuning->params.isp_3a_settings.awb_blue_sky_dist;

			/* offset */
			data_ptr += sizeof(struct isp_awb_dist_cfg);
			ret += sizeof(struct isp_awb_dist_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_LIGHT_INFO) /* isp_awb_light_info */
		{
			struct isp_awb_temp_info_cfg *isp_awb_light_info = (struct isp_awb_temp_info_cfg *)data_ptr;
			isp_awb_light_info->number = tuning->params.isp_3a_settings.awb_light_num;
			memcpy(isp_awb_light_info->value, tuning->params.isp_3a_settings.awb_light_info,
				sizeof(tuning->params.isp_3a_settings.awb_light_info));

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_info_cfg);
			ret += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_EXT_LIGHT_INFO) /* isp_awb_ext_light_info */
		{
			struct isp_awb_temp_info_cfg *isp_awb_ext_light_info = (struct isp_awb_temp_info_cfg *)data_ptr;
			isp_awb_ext_light_info->number = tuning->params.isp_3a_settings.awb_ext_light_num;
			memcpy(isp_awb_ext_light_info->value, tuning->params.isp_3a_settings.awb_ext_light_info,
				sizeof(tuning->params.isp_3a_settings.awb_ext_light_info));

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_info_cfg);
			ret += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_SKIN_INFO) /* isp_awb_skin_info */
		{
			struct isp_awb_temp_info_cfg *isp_awb_skin_info = (struct isp_awb_temp_info_cfg *)data_ptr;
			isp_awb_skin_info->number = tuning->params.isp_3a_settings.awb_skin_color_num;
			memcpy(isp_awb_skin_info->value, tuning->params.isp_3a_settings.awb_skin_color_info,
				sizeof(tuning->params.isp_3a_settings.awb_skin_color_info));  // !!!be careful, 160 -> 320

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_info_cfg);
			ret += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_SPECIAL_INFO) /* isp_awb_special_info */
		{
			struct isp_awb_temp_info_cfg *isp_awb_special_info = (struct isp_awb_temp_info_cfg *)data_ptr;
			isp_awb_special_info->number = tuning->params.isp_3a_settings.awb_special_color_num;
			memcpy(isp_awb_special_info->value, tuning->params.isp_3a_settings.awb_special_color_info,
				sizeof(tuning->params.isp_3a_settings.awb_special_color_info));

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_info_cfg);
			ret += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_PRESET_GAIN) /* isp_awb_preset_gain */
		{
			memcpy(data_ptr, tuning->params.isp_3a_settings.awb_preset_gain, sizeof(struct isp_awb_preset_gain_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_awb_preset_gain_cfg);
			ret += sizeof(struct isp_awb_preset_gain_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_FAVOR) /* isp_awb_favor */
		{
			struct isp_awb_favor_cfg *isp_awb_favor = (struct isp_awb_favor_cfg *)data_ptr;
			isp_awb_favor->rgain = tuning->params.isp_3a_settings.awb_rgain_favor;
			isp_awb_favor->bgain = tuning->params.isp_3a_settings.awb_bgain_favor;

			/* offset */
			data_ptr += sizeof(struct isp_awb_favor_cfg);
			ret += sizeof(struct isp_awb_favor_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_VCM_CODE) /* isp_af_vcm_code */
		{
			struct isp_af_vcm_code_cfg *isp_af_vcm_code = (struct isp_af_vcm_code_cfg *)data_ptr;
			isp_af_vcm_code->min = tuning->params.isp_3a_settings.vcm_min_code;
			isp_af_vcm_code->max = tuning->params.isp_3a_settings.vcm_max_code;

			/* offset */
			data_ptr += sizeof(struct isp_af_vcm_code_cfg);
			ret += sizeof(struct isp_af_vcm_code_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_OTP) /* isp_af_otp */
		{
			struct isp_af_otp_cfg *isp_af_otp = (struct isp_af_otp_cfg *)data_ptr;
			isp_af_otp->use_otp = tuning->params.isp_3a_settings.af_use_otp;

			/* offset */
			data_ptr += sizeof(struct isp_af_otp_cfg);
			ret += sizeof(struct isp_af_otp_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_SPEED) /* isp_af_speed */
		{
			struct isp_af_speed_cfg *isp_af_speed = (struct isp_af_speed_cfg *)data_ptr;
			isp_af_speed->interval_time = tuning->params.isp_3a_settings.af_interval_time;
			isp_af_speed->index = tuning->params.isp_3a_settings.af_speed_ind;

			/* offset */
			data_ptr += sizeof(struct isp_af_speed_cfg);
			ret += sizeof(struct isp_af_speed_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_FINE_SEARCH) /* isp_af_fine_search */
		{
			struct isp_af_fine_search_cfg *isp_af_fine_search = (struct isp_af_fine_search_cfg *)data_ptr;
			isp_af_fine_search->auto_en = tuning->params.isp_3a_settings.af_auto_fine_en;
			isp_af_fine_search->single_en = tuning->params.isp_3a_settings.af_single_fine_en;
			isp_af_fine_search->step = tuning->params.isp_3a_settings.af_fine_step;

			/* offset */
			data_ptr += sizeof(struct isp_af_fine_search_cfg);
			ret += sizeof(struct isp_af_fine_search_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_REFOCUS) /* isp_af_refocus */
		{
			struct isp_af_refocus_cfg *isp_af_refocus = (struct isp_af_refocus_cfg *)data_ptr;
			isp_af_refocus->move_cnt = tuning->params.isp_3a_settings.af_move_cnt;
			isp_af_refocus->still_cnt = tuning->params.isp_3a_settings.af_still_cnt;
			isp_af_refocus->move_monitor_cnt = tuning->params.isp_3a_settings.af_move_monitor_cnt;
			isp_af_refocus->still_monitor_cnt = tuning->params.isp_3a_settings.af_still_monitor_cnt;

			/* offset */
			data_ptr += sizeof(struct isp_af_refocus_cfg);
			ret += sizeof(struct isp_af_refocus_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_TOLERANCE) /* isp_af_tolerance */
		{
			struct isp_af_tolerance_cfg *isp_af_tolerance = (struct isp_af_tolerance_cfg *)data_ptr;
			isp_af_tolerance->near_distance = tuning->params.isp_3a_settings.af_near_tolerance;
			isp_af_tolerance->far_distance = tuning->params.isp_3a_settings.af_far_tolerance;
			isp_af_tolerance->offset = tuning->params.isp_3a_settings.af_tolerance_off;
			isp_af_tolerance->table_length = tuning->params.isp_3a_settings.af_tolerance_tbl_len;
			memcpy(isp_af_tolerance->std_code_table, tuning->params.isp_3a_settings.af_std_code_tbl,
				sizeof(isp_af_tolerance->std_code_table));
			memcpy(isp_af_tolerance->value, tuning->params.isp_3a_settings.af_tolerance_value_tbl,
				sizeof(isp_af_tolerance->value));

			/* offset */
			data_ptr += sizeof(struct isp_af_tolerance_cfg);
			ret += sizeof(struct isp_af_tolerance_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_SCENE) /* isp_af_scene */
		{
			struct isp_af_scene_cfg *isp_af_scene = (struct isp_af_scene_cfg *)data_ptr;
			isp_af_scene->stable_min = tuning->params.isp_3a_settings.af_stable_min;
			isp_af_scene->stable_max = tuning->params.isp_3a_settings.af_stable_max;
			isp_af_scene->low_light_lv = tuning->params.isp_3a_settings.af_low_light_lv;
			isp_af_scene->peak_thres = tuning->params.isp_3a_settings.af_peak_th;
			isp_af_scene->direction_thres = tuning->params.isp_3a_settings.af_dir_th;
			isp_af_scene->change_ratio = tuning->params.isp_3a_settings.af_change_ratio;
			isp_af_scene->move_minus = tuning->params.isp_3a_settings.af_move_minus;
			isp_af_scene->still_minus = tuning->params.isp_3a_settings.af_still_minus;
			isp_af_scene->scene_motion_thres = tuning->params.isp_3a_settings.af_scene_motion_th;

			/* offset */
			data_ptr += sizeof(struct isp_af_scene_cfg);
			ret += sizeof(struct isp_af_scene_cfg);
		}
		break;
	case HW_ISP_CFG_TUNING: /* isp_tunning_param */
		if (cfg_ids & HW_ISP_CFG_TUNING_FLASH) /* isp_tuning_flash */
		{
			struct isp_tuning_flash_cfg *isp_tuning_flash = (struct isp_tuning_flash_cfg *)data_ptr;
			isp_tuning_flash->gain = tuning->params.isp_tunning_settings.flash_gain;
			isp_tuning_flash->delay_frame = tuning->params.isp_tunning_settings.flash_delay_frame;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_flash_cfg);
			ret += sizeof(struct isp_tuning_flash_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_FLICKER) /* isp_tuning_flicker */
		{
			struct isp_tuning_flicker_cfg *isp_tuning_flicker = (struct isp_tuning_flicker_cfg *)data_ptr;
			isp_tuning_flicker->type = tuning->params.isp_tunning_settings.flicker_type;
			isp_tuning_flicker->ratio = tuning->params.isp_tunning_settings.flicker_ratio;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_flicker_cfg);
			ret += sizeof(struct isp_tuning_flicker_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_VISUAL_ANGLE) /* isp_visual_angle */
		{
			struct isp_tuning_visual_angle_cfg *isp_visual_angle = (struct isp_tuning_visual_angle_cfg *)data_ptr;
			isp_visual_angle->horizontal = tuning->params.isp_tunning_settings.hor_visual_angle;
			isp_visual_angle->vertical = tuning->params.isp_tunning_settings.ver_visual_angle;
			isp_visual_angle->focus_length = tuning->params.isp_tunning_settings.focus_length;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_visual_angle_cfg);
			ret += sizeof(struct isp_tuning_visual_angle_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_GTM) /* isp_tuning_gtm */
		{
			struct isp_tuning_gtm_cfg *isp_tuning_gtm = (struct isp_tuning_gtm_cfg *)data_ptr;
			isp_tuning_gtm->type = tuning->params.isp_tunning_settings.gtm_type;
			isp_tuning_gtm->gamma_type = tuning->params.isp_tunning_settings.gamma_type;
			isp_tuning_gtm->auto_alpha_en = tuning->params.isp_tunning_settings.auto_alpha_en;
			isp_tuning_gtm->hist_pix_cnt= tuning->params.isp_tunning_settings.hist_pix_cnt;
			isp_tuning_gtm->dark_minval= tuning->params.isp_tunning_settings.dark_minval;
			isp_tuning_gtm->bright_minval= tuning->params.isp_tunning_settings.bright_minval;

			memcpy(isp_tuning_gtm->plum_var, tuning->params.isp_tunning_settings.plum_var,
				sizeof(isp_tuning_gtm->plum_var));
			/* offset */
			data_ptr += sizeof(struct isp_tuning_gtm_cfg);
			ret += sizeof(struct isp_tuning_gtm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CFA) /* isp_tuning_cfa */
		{
			struct isp_tuning_cfa_cfg *isp_tuning_cfa = (struct isp_tuning_cfa_cfg *)data_ptr;
			isp_tuning_cfa->dir_thres = tuning->params.isp_tunning_settings.cfa_dir_th;
			isp_tuning_cfa->interp_mode = tuning->params.isp_tunning_settings.cfa_interp_mode;
			isp_tuning_cfa->zig_zag = tuning->params.isp_tunning_settings.cfa_zig_zag;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_cfa_cfg);
			ret += sizeof(struct isp_tuning_cfa_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CTC) /* isp_tuning_ctc */
		{
			struct isp_tuning_ctc_cfg *isp_tuning_ctc = (struct isp_tuning_ctc_cfg *)data_ptr;
			isp_tuning_ctc->min_thres = tuning->params.isp_tunning_settings.ctc_th_min;
			isp_tuning_ctc->max_thres = tuning->params.isp_tunning_settings.ctc_th_max;
			isp_tuning_ctc->slope_thres = tuning->params.isp_tunning_settings.ctc_th_slope;
			isp_tuning_ctc->dir_wt = tuning->params.isp_tunning_settings.ctc_dir_wt;
			isp_tuning_ctc->dir_thres = tuning->params.isp_tunning_settings.ctc_dir_th;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_ctc_cfg);
			ret += sizeof(struct isp_tuning_ctc_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_DIGITAL_GAIN) /* isp_digital_gain */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.bayer_gain, sizeof(struct isp_tuning_blc_gain_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_blc_gain_cfg);
			ret += sizeof(struct isp_tuning_blc_gain_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_LOW) /* isp_ccm_low */
		{
			struct isp_tuning_ccm_cfg *isp_ccm_low = (struct isp_tuning_ccm_cfg *)data_ptr;
			isp_ccm_low->temperature = tuning->params.isp_tunning_settings.cm_trig_cfg[0];
			memcpy(&isp_ccm_low->value, &(tuning->params.isp_tunning_settings.color_matrix_ini[0]),
				sizeof(struct isp_rgb2rgb_gain_offset));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_ccm_cfg);
			ret += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_MID) /* isp_ccm_mid */
		{
			struct isp_tuning_ccm_cfg *isp_ccm_mid = (struct isp_tuning_ccm_cfg *)data_ptr;
			isp_ccm_mid->temperature = tuning->params.isp_tunning_settings.cm_trig_cfg[1];
			memcpy(&isp_ccm_mid->value, &(tuning->params.isp_tunning_settings.color_matrix_ini[1]),
				sizeof(struct isp_rgb2rgb_gain_offset));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_ccm_cfg);
			ret += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_HIGH) /* isp_ccm_high */
		{
			struct isp_tuning_ccm_cfg *isp_ccm_high = (struct isp_tuning_ccm_cfg *)data_ptr;
			isp_ccm_high->temperature = tuning->params.isp_tunning_settings.cm_trig_cfg[2];
			memcpy(&isp_ccm_high->value, &(tuning->params.isp_tunning_settings.color_matrix_ini[2]),
				sizeof(struct isp_rgb2rgb_gain_offset));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_ccm_cfg);
			ret += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_PLTM) /* isp_tuning_pltm */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.pltm_cfg, sizeof(struct isp_tuning_pltm_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_pltm_cfg);
			ret += sizeof(struct isp_tuning_pltm_cfg);
		}
#if (ISP_VERSION >= 521)
		if (cfg_ids & HW_ISP_CFG_TUNING_GCA) /* isp_tuning_gca  */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.gca_cfg, sizeof(struct isp_tuning_gca_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_gca_cfg);
			ret += sizeof(struct isp_tuning_gca_cfg);
		}
#endif
		break;
	case HW_ISP_CFG_TUNING_TABLES: /* isp tuning tables */
		if (cfg_ids & HW_ISP_CFG_TUNING_LSC) /* isp_tuning_lsc */
		{
			struct isp_tuning_lsc_table_cfg *isp_tuning_lsc = (struct isp_tuning_lsc_table_cfg *)data_ptr;
#if (ISP_VERSION >= 521)
			isp_tuning_lsc->lsc_mode = tuning->params.isp_tunning_settings.lsc_mode;
#endif
			isp_tuning_lsc->ff_mod = tuning->params.isp_tunning_settings.ff_mod;
			isp_tuning_lsc->center_x = tuning->params.isp_tunning_settings.lsc_center_x;
			isp_tuning_lsc->center_y = tuning->params.isp_tunning_settings.lsc_center_y;
			isp_tuning_lsc->rolloff_ratio = tuning->params.isp_tunning_settings.rolloff_ratio;
			memcpy(&(isp_tuning_lsc->value[0][0]), &(tuning->params.isp_tunning_settings.lsc_tbl[0][0]),
				sizeof(isp_tuning_lsc->value));
			memcpy(isp_tuning_lsc->color_temp_triggers, tuning->params.isp_tunning_settings.lsc_trig_cfg,
				sizeof(isp_tuning_lsc->color_temp_triggers));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_lsc_table_cfg);
			ret += sizeof(struct isp_tuning_lsc_table_cfg);
		}

		if (cfg_ids & HW_ISP_CFG_TUNING_GAMMA) /* isp_tuning_gamma */
		{
			struct isp_tuning_gamma_table_cfg *isp_tuning_gamma = (struct isp_tuning_gamma_table_cfg *)data_ptr;
			isp_tuning_gamma->number = tuning->params.isp_tunning_settings.gamma_num;
			memcpy(&(isp_tuning_gamma->value[0][0]), &(tuning->params.isp_tunning_settings.gamma_tbl_ini[0][0]),
				sizeof(isp_tuning_gamma->value));
			memcpy(isp_tuning_gamma->lv_triggers, tuning->params.isp_tunning_settings.gamma_trig_cfg,
				sizeof(isp_tuning_gamma->lv_triggers));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_gamma_table_cfg);
			ret += sizeof(struct isp_tuning_gamma_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_LINEARITY) /* isp_tuning_linearity */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.linear_tbl, sizeof(struct isp_tuning_linearity_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_linearity_table_cfg);
			ret += sizeof(struct isp_tuning_linearity_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_DISTORTION) /* isp_tuning_distortion */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.disc_tbl, sizeof(struct isp_tuning_distortion_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_distortion_table_cfg);
			ret += sizeof(struct isp_tuning_distortion_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_BDNF) /* isp_tuning_bdnf */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.isp_bdnf_th, sizeof(struct isp_tuning_bdnf_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_bdnf_table_cfg);
			ret += sizeof(struct isp_tuning_bdnf_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_TDNF) /* isp_tuning_tdnf */
		{
			struct isp_tuning_tdnf_table_cfg *isp_tuning_tdnf = (struct isp_tuning_tdnf_table_cfg *)data_ptr;
			memcpy(isp_tuning_tdnf->thres, tuning->params.isp_tunning_settings.isp_tdnf_th, sizeof(isp_tuning_tdnf->thres));
			memcpy(isp_tuning_tdnf->ref_noise, tuning->params.isp_tunning_settings.isp_tdnf_ref_noise, sizeof(isp_tuning_tdnf->ref_noise));
			memcpy(isp_tuning_tdnf->k_val, tuning->params.isp_tunning_settings.isp_tdnf_k, sizeof(isp_tuning_tdnf->k_val));
			memcpy(isp_tuning_tdnf->diff, tuning->params.isp_tunning_settings.isp_tdnf_diff, sizeof(isp_tuning_tdnf->diff));
#if (ISP_VERSION >= 520)
			memcpy(isp_tuning_tdnf->d3d_k3d, tuning->params.isp_tunning_settings.isp_d3d_k3d_incre_curve, sizeof(isp_tuning_tdnf->d3d_k3d));
#endif

			/* offset */
			data_ptr += sizeof(struct isp_tuning_tdnf_table_cfg);
			ret += sizeof(struct isp_tuning_tdnf_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CONTRAST) /* isp_tuning_contrast */
		{
			struct isp_tuning_contrast_table_cfg *isp_tuning_contrast = (struct isp_tuning_contrast_table_cfg *)data_ptr;
			memcpy(isp_tuning_contrast->val, tuning->params.isp_tunning_settings.isp_contrast_val, sizeof(isp_tuning_contrast->val));
			memcpy(isp_tuning_contrast->lum, tuning->params.isp_tunning_settings.isp_contrast_lum, sizeof(isp_tuning_contrast->lum));
			memcpy(isp_tuning_contrast->pe, tuning->params.isp_tunning_settings.isp_contrat_pe, sizeof(isp_tuning_contrast->pe));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_contrast_table_cfg);
			ret += sizeof(struct isp_tuning_contrast_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_SHARP) /* isp_tuning_sharp */
		{
			struct isp_tuning_sharp_table_cfg *isp_tuning_sharp = (struct isp_tuning_sharp_table_cfg *)data_ptr;
			memcpy(isp_tuning_sharp->value, tuning->params.isp_tunning_settings.isp_sharp_val, sizeof(isp_tuning_sharp->value));
			memcpy(isp_tuning_sharp->lum, tuning->params.isp_tunning_settings.isp_sharp_lum, sizeof(isp_tuning_sharp->lum));
#if (ISP_VERSION >= 520)
			memcpy(isp_tuning_sharp->edge_lum, tuning->params.isp_tunning_settings.isp_sharp_edge_lum, sizeof(isp_tuning_sharp->edge_lum));
			memcpy(isp_tuning_sharp->hfrq_lum, tuning->params.isp_tunning_settings.isp_sharp_hfrq_lum, sizeof(isp_tuning_sharp->hfrq_lum));
			memcpy(isp_tuning_sharp->hsv, tuning->params.isp_tunning_settings.isp_sharp_hsv, sizeof(isp_tuning_sharp->hsv));
			memcpy(isp_tuning_sharp->smap, tuning->params.isp_tunning_settings.isp_sharp_s_map, sizeof(isp_tuning_sharp->smap));
#endif
			/* offset */
			data_ptr += sizeof(struct isp_tuning_sharp_table_cfg);
			ret += sizeof(struct isp_tuning_sharp_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CEM) /* isp_tuning_cem */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.isp_cem_table, sizeof(struct isp_tuning_cem_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_cem_table_cfg);
			ret += sizeof(struct isp_tuning_cem_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CEM_1) /* isp_tuning_cem_1 */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.isp_cem_table1, sizeof(struct isp_tuning_cem_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_cem_table_cfg);
			ret += sizeof(struct isp_tuning_cem_table_cfg);
		}
#if (ISP_VERSION != 522)
		if (cfg_ids & HW_ISP_CFG_TUNING_PLTM_TBL) /* isp_tuning_pltm_table */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.isp_pltm_table, sizeof(struct isp_tuning_pltm_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_pltm_table_cfg);
			ret += sizeof(struct isp_tuning_pltm_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_WDR) /* isp_tuning_wdr */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.isp_wdr_table, sizeof(struct isp_tuning_wdr_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_wdr_table_cfg);
			ret += sizeof(struct isp_tuning_wdr_table_cfg);
		}
#endif
#if (ISP_VERSION >= 521)
		if (cfg_ids & HW_ISP_CFG_TUNING_LCA_TBL) /* isp_tuning_lca  */
		{
			memcpy(data_ptr, tuning->params.isp_tunning_settings.lca_pf_satu_lut, sizeof(struct isp_tuning_lca_pf_satu_lut));
			/* lca_pf_satu_lut offset */
			data_ptr += sizeof(struct isp_tuning_lca_pf_satu_lut);
			ret += sizeof(struct isp_tuning_lca_pf_satu_lut);

            memcpy(data_ptr, tuning->params.isp_tunning_settings.lca_gf_satu_lut, sizeof(struct isp_tuning_lca_gf_satu_lut));
			/* lca_gf_satu_lut offset */
			data_ptr += sizeof(struct isp_tuning_lca_gf_satu_lut);
			ret += sizeof(struct isp_tuning_lca_gf_satu_lut);

		}
		if (cfg_ids & HW_ISP_CFG_TUNING_MSC) /* isp_tuning_msc */
		{
			struct isp_tuning_msc_table_cfg *isp_tuning_msc = (struct isp_tuning_msc_table_cfg *)data_ptr;
			isp_tuning_msc->mff_mod = tuning->params.isp_tunning_settings.mff_mod;
			isp_tuning_msc->msc_mode = tuning->params.isp_tunning_settings.msc_mode;
			memcpy(isp_tuning_msc->msc_blw_lut, tuning->params.isp_tunning_settings.msc_blw_lut,
				sizeof(isp_tuning_msc->msc_blw_lut));
			memcpy(isp_tuning_msc->msc_blh_lut, tuning->params.isp_tunning_settings.msc_blh_lut,
				sizeof(isp_tuning_msc->msc_blh_lut));
			memcpy(isp_tuning_msc->msc_blw_dlt_lut, tuning->params.isp_tunning_settings.msc_blw_dlt_lut,
				sizeof(isp_tuning_msc->msc_blw_dlt_lut));
			memcpy(isp_tuning_msc->msc_blh_dlt_lut, tuning->params.isp_tunning_settings.msc_blh_dlt_lut,
				sizeof(isp_tuning_msc->msc_blh_dlt_lut));
			memcpy(&(isp_tuning_msc->value[0][0]), &(tuning->params.isp_tunning_settings.msc_tbl[0][0]),
				sizeof(isp_tuning_msc->value));
			memcpy(isp_tuning_msc->color_temp_triggers, tuning->params.isp_tunning_settings.msc_trig_cfg,
				sizeof(isp_tuning_msc->color_temp_triggers));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_msc_table_cfg);
			ret += sizeof(struct isp_tuning_msc_table_cfg);
		}
#endif
		break;
	case HW_ISP_CFG_DYNAMIC: /* isp_dynamic_param */
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_LUM_POINT) /* isp_dynamic_lum_mapping_point */
		{
			memcpy(data_ptr, tuning->params.isp_iso_settings.isp_lum_mapping_point, sizeof(struct isp_dynamic_single_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_single_cfg);
			ret += sizeof(struct isp_dynamic_single_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_GAIN_POINT) /* isp_dynamic_gain_mapping_point */
		{
			memcpy(data_ptr, tuning->params.isp_iso_settings.isp_gain_mapping_point, sizeof(struct isp_dynamic_single_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_single_cfg);
			ret += sizeof(struct isp_dynamic_single_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SHARP) /* isp_dynamic_sharp */
		{
			struct isp_dynamic_sharp_cfg *isp_dynamic_sharp = (struct isp_dynamic_sharp_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_sharp->trigger = tuning->params.isp_iso_settings.triger.sharp_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				memcpy(isp_dynamic_sharp->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].sharp_cfg,
				        sizeof(isp_dynamic_sharp->tuning_cfg[i].value));
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_sharp_cfg);
			ret += sizeof(struct isp_dynamic_sharp_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_CONTRAST) /* isp_dynamic_contrast */
		{
			struct isp_dynamic_contrast_cfg *isp_dynamic_contrast = (struct isp_dynamic_contrast_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_contrast->trigger = tuning->params.isp_iso_settings.triger.contrast_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				memcpy(isp_dynamic_contrast->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].contrast_cfg,
					sizeof(isp_dynamic_contrast->tuning_cfg[i].value));
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_contrast_cfg);
			ret += sizeof(struct isp_dynamic_contrast_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DENOISE) /* isp_dynamic_denoise */
		{
			struct isp_dynamic_denoise_cfg *isp_dynamic_denoise = (struct isp_dynamic_denoise_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_denoise->trigger = tuning->params.isp_iso_settings.triger.denoise_triger;
			isp_dynamic_denoise->color_trigger = tuning->params.isp_iso_settings.triger.color_denoise_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				memcpy(isp_dynamic_denoise->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].denoise_cfg,
					sizeof(isp_dynamic_denoise->tuning_cfg[i].value));
				isp_dynamic_denoise->tuning_cfg[i].color_denoise = tuning->params.isp_iso_settings.isp_dynamic_cfg[i].color_denoise;
				isp_dynamic_denoise->tuning_cfg[i].ratio = 1;
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_denoise_cfg);
			ret += sizeof(struct isp_dynamic_denoise_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SENSOR_OFFSET) /* isp_dynamic_sensor_offset */
		{
			struct isp_dynamic_sensor_offset_cfg *isp_dynamic_sensor_offset = (struct isp_dynamic_sensor_offset_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_sensor_offset->trigger = tuning->params.isp_iso_settings.triger.sensor_offset_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(isp_dynamic_sensor_offset->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].sensor_offset,
					sizeof(isp_dynamic_sensor_offset->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_sensor_offset_cfg);
			ret += sizeof(struct isp_dynamic_sensor_offset_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_BLACK_LV) /* isp_dynamic_black_level */
		{
			struct isp_dynamic_black_level_cfg *isp_dynamic_black_level = (struct isp_dynamic_black_level_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_black_level->trigger = tuning->params.isp_iso_settings.triger.black_level_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(isp_dynamic_black_level->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].black_level,
					sizeof(isp_dynamic_black_level->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_black_level_cfg);
			ret += sizeof(struct isp_dynamic_black_level_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DPC) /* isp_dynamic_dpcl */
		{
			struct isp_dynamic_dpc_cfg *isp_dynamic_dpc = (struct isp_dynamic_dpc_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_dpc->trigger = tuning->params.isp_iso_settings.triger.dpc_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(isp_dynamic_dpc->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].dpc_cfg,
					sizeof(isp_dynamic_dpc->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_dpc_cfg);
			ret += sizeof(struct isp_dynamic_dpc_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_PLTM) /* isp_dynamic_pltm */
		{
			struct isp_dynamic_pltm_cfg *isp_dynamic_pltm = (struct isp_dynamic_pltm_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_pltm->trigger = tuning->params.isp_iso_settings.triger.pltm_dynamic_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(isp_dynamic_pltm->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].pltm_dynamic_cfg,
					sizeof(isp_dynamic_pltm->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_pltm_cfg);
			ret += sizeof(struct isp_dynamic_pltm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DEFOG) /* isp_dynamic_defog */
		{
			struct isp_dynamic_defog_cfg *isp_dynamic_defog = (struct isp_dynamic_defog_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_defog->trigger = tuning->params.isp_iso_settings.triger.defog_value_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				isp_dynamic_defog->tuning_cfg[i].value = tuning->params.isp_iso_settings.isp_dynamic_cfg[i].defog_value;

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_defog_cfg);
			ret += sizeof(struct isp_dynamic_defog_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_HISTOGRAM) /* isp_dynamic_histogram */
		{
			struct isp_dynamic_histogram_cfg *isp_dynamic_histogram = (struct isp_dynamic_histogram_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_histogram->brightness_trigger = tuning->params.isp_iso_settings.triger.brightness_triger;
			isp_dynamic_histogram->contrast_trigger = tuning->params.isp_iso_settings.triger.gcontrast_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				isp_dynamic_histogram->tuning_cfg[i].brightness = tuning->params.isp_iso_settings.isp_dynamic_cfg[i].brightness;
				isp_dynamic_histogram->tuning_cfg[i].contrast = tuning->params.isp_iso_settings.isp_dynamic_cfg[i].contrast;
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_histogram_cfg);
			ret += sizeof(struct isp_dynamic_histogram_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SATURATION) /* isp_dynamic_saturation */
		{
			struct isp_dynamic_saturation_cfg *isp_dynamic_saturation = (struct isp_dynamic_saturation_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_saturation->trigger = tuning->params.isp_iso_settings.triger.saturation_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				isp_dynamic_saturation->tuning_cfg[i].cb = tuning->params.isp_iso_settings.isp_dynamic_cfg[i].saturation_cb;
				isp_dynamic_saturation->tuning_cfg[i].cr = tuning->params.isp_iso_settings.isp_dynamic_cfg[i].saturation_cr;
				memcpy(isp_dynamic_saturation->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].saturation_cfg,
					sizeof(isp_dynamic_saturation->tuning_cfg[i].value));
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_saturation_cfg);
			ret += sizeof(struct isp_dynamic_saturation_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_CEM) /* isp_dynamic_cem */
		{
			struct isp_dynamic_cem_cfg *isp_dynamic_cem = (struct isp_dynamic_cem_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_cem->trigger = tuning->params.isp_iso_settings.triger.cem_ratio_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				isp_dynamic_cem->tuning_cfg[i].value = tuning->params.isp_iso_settings.isp_dynamic_cfg[i].cem_ratio;

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_cem_cfg);
			ret += sizeof(struct isp_dynamic_cem_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_TDF) /* isp_dynamic_tdf */
		{
			struct isp_dynamic_tdf_cfg *isp_dynamic_tdf = (struct isp_dynamic_tdf_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_tdf->trigger = tuning->params.isp_iso_settings.triger.tdf_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(isp_dynamic_tdf->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].tdf_cfg,
					sizeof(isp_dynamic_tdf->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_tdf_cfg);
			ret += sizeof(struct isp_dynamic_tdf_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_AE) /* isp_dynamic_ae */
		{
			struct isp_dynamic_ae_cfg *isp_dynamic_ae = (struct isp_dynamic_ae_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_ae->trigger = tuning->params.isp_iso_settings.triger.ae_cfg_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(isp_dynamic_ae->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].ae_cfg,
					sizeof(isp_dynamic_ae->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_ae_cfg);
			ret += sizeof(struct isp_dynamic_ae_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_GTM) /* isp_dynamic_gtm */
		{
			struct isp_dynamic_gtm_cfg *isp_dynamic_gtm = (struct isp_dynamic_gtm_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_gtm->trigger = tuning->params.isp_iso_settings.triger.gtm_cfg_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(isp_dynamic_gtm->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].gtm_cfg,
					sizeof(isp_dynamic_gtm->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_gtm_cfg);
			ret += sizeof(struct isp_dynamic_gtm_cfg);
		}
#if (ISP_VERSION >= 521)
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_LCA) /* isp_dynamic_lca */
		{
			struct isp_dynamic_lca_cfg *isp_dynamic_lca = (struct isp_dynamic_lca_cfg *)data_ptr;
			int i = 0;
			isp_dynamic_lca->trigger = tuning->params.isp_iso_settings.triger.lca_cfg_triger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(isp_dynamic_lca->tuning_cfg[i].value, tuning->params.isp_iso_settings.isp_dynamic_cfg[i].lca_cfg,
					sizeof(isp_dynamic_lca->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_lca_cfg);
			ret += sizeof(struct isp_dynamic_lca_cfg);
		}
#endif
		break;
	default:
		ret = AW_ERR_VI_INVALID_PARA;
		break;
	}

	data_ptr = NULL;
	return ret;
}

HW_S32 isp_tuning_set_cfg(struct hw_isp_device *isp, HW_U8 group_id, HW_U32 cfg_ids, void *cfg_data)
{
	int ret = AW_ERR_VI_INVALID_PARA;
	unsigned char *data_ptr = NULL;
	struct isp_tuning *tuning = NULL;

	if (!isp || !cfg_data)
		return AW_ERR_VI_INVALID_PARA;

	/* call isp api */
	tuning = isp_dev_get_tuning(isp);
	if (!tuning)
		 return AW_ERR_VI_INVALID_NULL_PTR;

	if (!tuning->ctx)
		 return AW_ERR_VI_INVALID_NULL_PTR;

	/* fill cfg data */
	ret = 0;
	data_ptr = (unsigned char *)cfg_data;

	switch (group_id)
	{
	case HW_ISP_CFG_TEST: /* isp_test_param */
		if (cfg_ids & HW_ISP_CFG_TEST_PUB) /* isp_test_pub */
		{
			struct isp_test_pub_cfg *isp_test_pub = (struct isp_test_pub_cfg *)data_ptr;
			tuning->params.isp_test_settings.isp_test_mode = isp_test_pub->test_mode;
			tuning->params.isp_test_settings.isp_gain = isp_test_pub->gain;
			tuning->params.isp_test_settings.isp_exp_line = isp_test_pub->exp_line;
			tuning->params.isp_test_settings.isp_color_temp = isp_test_pub->color_temp;
			tuning->params.isp_test_settings.isp_log_param = isp_test_pub->log_param;

			/* offset */
			data_ptr += sizeof(struct isp_test_pub_cfg);
			ret += sizeof(struct isp_test_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_EXPTIME) /* isp_test_exptime */
		{
			struct isp_test_item_cfg *isp_test_exptime = (struct isp_test_item_cfg *)data_ptr;
			tuning->params.isp_test_settings.isp_test_exptime = isp_test_exptime->enable;
			tuning->params.isp_test_settings.exp_line_start = isp_test_exptime->start;
			tuning->params.isp_test_settings.exp_line_step = isp_test_exptime->step;
			tuning->params.isp_test_settings.exp_line_end = isp_test_exptime->end;
			tuning->params.isp_test_settings.exp_change_interval = isp_test_exptime->change_interval;

			/* offset */
			data_ptr += sizeof(struct isp_test_item_cfg);
			ret += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_GAIN) /* isp_test_gain */
		{
			struct isp_test_item_cfg *isp_test_gain = (struct isp_test_item_cfg *)data_ptr;
			tuning->params.isp_test_settings.isp_test_gain = isp_test_gain->enable;
			tuning->params.isp_test_settings.gain_start = isp_test_gain->start;
			tuning->params.isp_test_settings.gain_step = isp_test_gain->step;
			tuning->params.isp_test_settings.gain_end = isp_test_gain->end;
			tuning->params.isp_test_settings.gain_change_interval = isp_test_gain->change_interval;

			/* offset */
			data_ptr += sizeof(struct isp_test_item_cfg);
			ret += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_FOCUS) /* isp_test_focus */
		{
			struct isp_test_item_cfg *isp_test_focus = (struct isp_test_item_cfg *)data_ptr;
			tuning->params.isp_test_settings.isp_test_focus = isp_test_focus->enable;
			tuning->params.isp_test_settings.focus_start = isp_test_focus->start;
			tuning->params.isp_test_settings.focus_step = isp_test_focus->step;
			tuning->params.isp_test_settings.focus_end = isp_test_focus->end;
			tuning->params.isp_test_settings.focus_change_interval = isp_test_focus->change_interval;

			/* offset */
			data_ptr += sizeof(struct isp_test_item_cfg);
			ret += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_FORCED) /* isp_test_forced */
		{
			struct isp_test_forced_cfg *isp_test_forced = (struct isp_test_forced_cfg *)data_ptr;
			tuning->params.isp_test_settings.ae_forced = isp_test_forced->ae_enable;
			tuning->params.isp_test_settings.lum_forced = isp_test_forced->lum;

			/* offset */
			data_ptr += sizeof(struct isp_test_forced_cfg);
			ret += sizeof(struct isp_test_forced_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_ENABLE) /* isp_test_enable */
		{
			memcpy(&(tuning->params.isp_test_settings.manual_en), data_ptr, sizeof(struct isp_test_enable_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_test_enable_cfg);
			ret += sizeof(struct isp_test_enable_cfg);
		}
		break;
	case HW_ISP_CFG_3A: /* isp_3a_param */
		if (cfg_ids & HW_ISP_CFG_AE_PUB) /* isp_ae_pub */
		{
			struct isp_ae_pub_cfg *isp_ae_pub = (struct isp_ae_pub_cfg *)data_ptr;
			tuning->params.isp_3a_settings.define_ae_table = isp_ae_pub->define_table;
			tuning->params.isp_3a_settings.ae_max_lv = isp_ae_pub->max_lv;
			tuning->params.isp_3a_settings.ae_hist_mod_en = isp_ae_pub->hist_mode_en;
			tuning->params.isp_3a_settings.ae_hist_sel = isp_ae_pub->hist_sel;
			tuning->params.isp_3a_settings.ae_stat_sel = isp_ae_pub->stat_sel;
			tuning->params.isp_3a_settings.exp_comp_step = isp_ae_pub->compensation_step;
			tuning->params.isp_3a_settings.ae_touch_dist_ind = isp_ae_pub->touch_dist_index;
			tuning->params.isp_3a_settings.ae_iso2gain_ratio = isp_ae_pub->iso2gain_ratio;
			memcpy(&tuning->params.isp_3a_settings.ae_fno_step[0], &isp_ae_pub->fno_table[0],
				sizeof(isp_ae_pub->fno_table));
			tuning->params.isp_3a_settings.ae_ki = isp_ae_pub->ki;
			tuning->params.isp_3a_settings.ae_ConvDataIndex = isp_ae_pub->conv_data_index;
			tuning->params.isp_3a_settings.ae_blowout_pre_en = isp_ae_pub->blowout_pre_en;
			tuning->params.isp_3a_settings.ae_blowout_attr = isp_ae_pub->blowout_attr;
			memcpy(&tuning->params.isp_3a_settings.wdr_cfg[0], &isp_ae_pub->wdr_cfg[0],
				sizeof(isp_ae_pub->wdr_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_ae_pub_cfg);
			ret += sizeof(struct isp_ae_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_PREVIEW_TBL) /* isp_ae_preview_tbl */
		{
			struct isp_ae_table_cfg *isp_ae_preview_tbl = (struct isp_ae_table_cfg *)data_ptr;
			tuning->params.isp_3a_settings.ae_table_preview_length = isp_ae_preview_tbl->length;
			memcpy(tuning->params.isp_3a_settings.ae_table_preview, &(isp_ae_preview_tbl->value[0]),
				sizeof(isp_ae_preview_tbl->value));

			/* offset */
			data_ptr += sizeof(struct isp_ae_table_cfg);
			ret += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_CAPTURE_TBL) /* isp_ae_capture_tbl */
		{
			struct isp_ae_table_cfg *isp_ae_capture_tbl = (struct isp_ae_table_cfg *)data_ptr;
			tuning->params.isp_3a_settings.ae_table_capture_length = isp_ae_capture_tbl->length;
			memcpy(tuning->params.isp_3a_settings.ae_table_capture, &(isp_ae_capture_tbl->value[0]),
				sizeof(isp_ae_capture_tbl->value));

			/* offset */
			data_ptr += sizeof(struct isp_ae_table_cfg);
			ret += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_VIDEO_TBL) /* isp_ae_video_tbl */
		{
			struct isp_ae_table_cfg *isp_ae_video_tbl = (struct isp_ae_table_cfg *)data_ptr;
			tuning->params.isp_3a_settings.ae_table_video_length = isp_ae_video_tbl->length;
			memcpy(tuning->params.isp_3a_settings.ae_table_video, &(isp_ae_video_tbl->value[0]),
				sizeof(isp_ae_video_tbl->value));

			/* offset */
			data_ptr += sizeof(struct isp_ae_table_cfg);
			ret += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_WIN_WEIGHT) /* isp_ae_win_weight */
		{
			memcpy(tuning->params.isp_3a_settings.ae_win_weight, data_ptr, sizeof(struct isp_ae_weight_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_ae_weight_cfg);
			ret += sizeof(struct isp_ae_weight_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_DELAY) /* isp_ae_delay */
		{
			struct isp_ae_delay_cfg *isp_ae_delay = (struct isp_ae_delay_cfg *)data_ptr;
			tuning->params.isp_3a_settings.ae_delay_frame = isp_ae_delay->ae_frame;
			tuning->params.isp_3a_settings.exp_delay_frame = isp_ae_delay->exp_frame;
			tuning->params.isp_3a_settings.gain_delay_frame = isp_ae_delay->gain_frame;

			/* offset */
			data_ptr += sizeof(struct isp_ae_delay_cfg);
			ret += sizeof(struct isp_ae_delay_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_PUB) /* isp_awb_pub */
		{
			struct isp_awb_pub_cfg *isp_awb_pub = (struct isp_awb_pub_cfg *)data_ptr;
			tuning->params.isp_3a_settings.awb_interval = isp_awb_pub->interval;
			tuning->params.isp_3a_settings.awb_speed = isp_awb_pub->speed;
			tuning->params.isp_3a_settings.awb_stat_sel = isp_awb_pub->stat_sel;

			/* offset */
			data_ptr += sizeof(struct isp_awb_pub_cfg);
			ret += sizeof(struct isp_awb_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_TEMP_RANGE) /* isp_awb_temp_range */
		{
			struct isp_awb_temp_range_cfg *isp_awb_temp_range = (struct isp_awb_temp_range_cfg *)data_ptr;
			tuning->params.isp_3a_settings.awb_color_temper_low = isp_awb_temp_range->low;
			tuning->params.isp_3a_settings.awb_color_temper_high = isp_awb_temp_range->high;
			tuning->params.isp_3a_settings.awb_base_temper = isp_awb_temp_range->base;

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_range_cfg);
			ret += sizeof(struct isp_awb_temp_range_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_DIST) /* isp_awb_dist */
		{
			struct isp_awb_dist_cfg *isp_awb_dist = (struct isp_awb_dist_cfg *)data_ptr;
			tuning->params.isp_3a_settings.awb_green_zone_dist = isp_awb_dist->green_zone;
			tuning->params.isp_3a_settings.awb_blue_sky_dist = isp_awb_dist->blue_sky;

			/* offset */
			data_ptr += sizeof(struct isp_awb_dist_cfg);
			ret += sizeof(struct isp_awb_dist_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_LIGHT_INFO) /* isp_awb_light_info */
		{
			struct isp_awb_temp_info_cfg *isp_awb_light_info = (struct isp_awb_temp_info_cfg *)data_ptr;
			tuning->params.isp_3a_settings.awb_light_num = isp_awb_light_info->number;
			memcpy(tuning->params.isp_3a_settings.awb_light_info, isp_awb_light_info->value,
				sizeof(tuning->params.isp_3a_settings.awb_light_info));

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_info_cfg);
			ret += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_EXT_LIGHT_INFO) /* isp_awb_ext_light_info */
		{
			struct isp_awb_temp_info_cfg *isp_awb_ext_light_info = (struct isp_awb_temp_info_cfg *)data_ptr;
			tuning->params.isp_3a_settings.awb_ext_light_num = isp_awb_ext_light_info->number;
			memcpy(tuning->params.isp_3a_settings.awb_ext_light_info, isp_awb_ext_light_info->value,
				sizeof(tuning->params.isp_3a_settings.awb_ext_light_info));

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_info_cfg);
			ret += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_SKIN_INFO) /* isp_awb_skin_info */
		{
			struct isp_awb_temp_info_cfg *isp_awb_skin_info = (struct isp_awb_temp_info_cfg *)data_ptr;
			tuning->params.isp_3a_settings.awb_skin_color_num = isp_awb_skin_info->number;
			memcpy(tuning->params.isp_3a_settings.awb_skin_color_info, isp_awb_skin_info->value,
				sizeof(tuning->params.isp_3a_settings.awb_skin_color_info));

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_info_cfg);
			ret += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_SPECIAL_INFO) /* isp_awb_special_info */
		{
			struct isp_awb_temp_info_cfg *isp_awb_special_info = (struct isp_awb_temp_info_cfg *)data_ptr;
			tuning->params.isp_3a_settings.awb_special_color_num = isp_awb_special_info->number;
			memcpy(tuning->params.isp_3a_settings.awb_special_color_info, isp_awb_special_info->value,
				sizeof(tuning->params.isp_3a_settings.awb_special_color_info));

			/* offset */
			data_ptr += sizeof(struct isp_awb_temp_info_cfg);
			ret += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_PRESET_GAIN) /* isp_awb_preset_gain */
		{
			memcpy(tuning->params.isp_3a_settings.awb_preset_gain, data_ptr, sizeof(struct isp_awb_preset_gain_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_awb_preset_gain_cfg);
			ret += sizeof(struct isp_awb_preset_gain_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_FAVOR) /* isp_awb_favor */
		{
			struct isp_awb_favor_cfg *isp_awb_favor = (struct isp_awb_favor_cfg *)data_ptr;
			tuning->params.isp_3a_settings.awb_rgain_favor = isp_awb_favor->rgain;
			tuning->params.isp_3a_settings.awb_bgain_favor = isp_awb_favor->bgain;

			/* offset */
			data_ptr += sizeof(struct isp_awb_favor_cfg);
			ret += sizeof(struct isp_awb_favor_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_VCM_CODE) /* isp_af_vcm_code */
		{
			struct isp_af_vcm_code_cfg *isp_af_vcm_code = (struct isp_af_vcm_code_cfg *)data_ptr;
			tuning->params.isp_3a_settings.vcm_min_code = isp_af_vcm_code->min;
			tuning->params.isp_3a_settings.vcm_max_code = isp_af_vcm_code->max;

			/* offset */
			data_ptr += sizeof(struct isp_af_vcm_code_cfg);
			ret += sizeof(struct isp_af_vcm_code_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_OTP) /* isp_af_otp */
		{
			struct isp_af_otp_cfg *isp_af_otp = (struct isp_af_otp_cfg *)data_ptr;
			tuning->params.isp_3a_settings.af_use_otp = isp_af_otp->use_otp;

			/* offset */
			data_ptr += sizeof(struct isp_af_otp_cfg);
			ret += sizeof(struct isp_af_otp_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_SPEED) /* isp_af_speed */
		{
			struct isp_af_speed_cfg *isp_af_speed = (struct isp_af_speed_cfg *)data_ptr;
			tuning->params.isp_3a_settings.af_interval_time = isp_af_speed->interval_time;
			tuning->params.isp_3a_settings.af_speed_ind = isp_af_speed->index;

			/* offset */
			data_ptr += sizeof(struct isp_af_speed_cfg);
			ret += sizeof(struct isp_af_speed_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_FINE_SEARCH) /* isp_af_fine_search */
		{
			struct isp_af_fine_search_cfg *isp_af_fine_search = (struct isp_af_fine_search_cfg *)data_ptr;
			tuning->params.isp_3a_settings.af_auto_fine_en = isp_af_fine_search->auto_en;
			tuning->params.isp_3a_settings.af_single_fine_en = isp_af_fine_search->single_en;
			tuning->params.isp_3a_settings.af_fine_step = isp_af_fine_search->step;

			/* offset */
			data_ptr += sizeof(struct isp_af_fine_search_cfg);
			ret += sizeof(struct isp_af_fine_search_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_REFOCUS) /* isp_af_refocus */
		{
			struct isp_af_refocus_cfg *isp_af_refocus = (struct isp_af_refocus_cfg *)data_ptr;
			tuning->params.isp_3a_settings.af_move_cnt = isp_af_refocus->move_cnt;
			tuning->params.isp_3a_settings.af_still_cnt = isp_af_refocus->still_cnt;
			tuning->params.isp_3a_settings.af_move_monitor_cnt = isp_af_refocus->move_monitor_cnt;
			tuning->params.isp_3a_settings.af_still_monitor_cnt = isp_af_refocus->still_monitor_cnt;

			/* offset */
			data_ptr += sizeof(struct isp_af_refocus_cfg);
			ret += sizeof(struct isp_af_refocus_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_TOLERANCE) /* isp_af_tolerance */
		{
			struct isp_af_tolerance_cfg *isp_af_tolerance = (struct isp_af_tolerance_cfg *)data_ptr;
			tuning->params.isp_3a_settings.af_near_tolerance = isp_af_tolerance->near_distance;
			tuning->params.isp_3a_settings.af_far_tolerance = isp_af_tolerance->far_distance;
			tuning->params.isp_3a_settings.af_tolerance_off = isp_af_tolerance->offset;
			tuning->params.isp_3a_settings.af_tolerance_tbl_len = isp_af_tolerance->table_length;
			memcpy(tuning->params.isp_3a_settings.af_std_code_tbl, isp_af_tolerance->std_code_table,
				sizeof(isp_af_tolerance->std_code_table));
			memcpy(tuning->params.isp_3a_settings.af_tolerance_value_tbl, isp_af_tolerance->value,
				sizeof(isp_af_tolerance->value));

			/* offset */
			data_ptr += sizeof(struct isp_af_tolerance_cfg);
			ret += sizeof(struct isp_af_tolerance_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_SCENE) /* isp_af_scene */
		{
			struct isp_af_scene_cfg *isp_af_scene = (struct isp_af_scene_cfg *)data_ptr;
			tuning->params.isp_3a_settings.af_stable_min = isp_af_scene->stable_min;
			tuning->params.isp_3a_settings.af_stable_max = isp_af_scene->stable_max;
			tuning->params.isp_3a_settings.af_low_light_lv = isp_af_scene->low_light_lv;
			tuning->params.isp_3a_settings.af_peak_th = isp_af_scene->peak_thres;
			tuning->params.isp_3a_settings.af_dir_th = isp_af_scene->direction_thres;
			tuning->params.isp_3a_settings.af_change_ratio = isp_af_scene->change_ratio;
			tuning->params.isp_3a_settings.af_move_minus = isp_af_scene->move_minus;
			tuning->params.isp_3a_settings.af_still_minus = isp_af_scene->still_minus;
			tuning->params.isp_3a_settings.af_scene_motion_th = isp_af_scene->scene_motion_thres;

			/* offset */
			data_ptr += sizeof(struct isp_af_scene_cfg);
			ret += sizeof(struct isp_af_scene_cfg);
		}
		break;
	case HW_ISP_CFG_TUNING: /* isp_tunning_param */
		if (cfg_ids & HW_ISP_CFG_TUNING_FLASH) /* isp_flash */
		{
			struct isp_tuning_flash_cfg *isp_tuning_flash = (struct isp_tuning_flash_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.flash_gain = isp_tuning_flash->gain;
			tuning->params.isp_tunning_settings.flash_delay_frame = isp_tuning_flash->delay_frame;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_flash_cfg);
			ret += sizeof(struct isp_tuning_flash_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_FLICKER) /* isp_flicker */
		{
			struct isp_tuning_flicker_cfg *isp_tuning_flicker = (struct isp_tuning_flicker_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.flicker_type = isp_tuning_flicker->type;
			tuning->params.isp_tunning_settings.flicker_ratio = isp_tuning_flicker->ratio;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_flicker_cfg);
			ret += sizeof(struct isp_tuning_flicker_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_VISUAL_ANGLE) /* isp_visual_angle */
		{
			struct isp_tuning_visual_angle_cfg *isp_visual_angle = (struct isp_tuning_visual_angle_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.hor_visual_angle = isp_visual_angle->horizontal;
			tuning->params.isp_tunning_settings.ver_visual_angle = isp_visual_angle->vertical;
			tuning->params.isp_tunning_settings.focus_length = isp_visual_angle->focus_length;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_visual_angle_cfg);
			ret += sizeof(struct isp_tuning_visual_angle_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_GTM) /* isp_gtm */
		{
			struct isp_tuning_gtm_cfg *isp_tuning_gtm = (struct isp_tuning_gtm_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.gtm_type = isp_tuning_gtm->type;
			tuning->params.isp_tunning_settings.gamma_type = isp_tuning_gtm->gamma_type;
			tuning->params.isp_tunning_settings.auto_alpha_en = isp_tuning_gtm->auto_alpha_en;
			tuning->params.isp_tunning_settings.hist_pix_cnt= isp_tuning_gtm->hist_pix_cnt;
			tuning->params.isp_tunning_settings.dark_minval= isp_tuning_gtm->dark_minval;
			tuning->params.isp_tunning_settings.bright_minval= isp_tuning_gtm->bright_minval;

			memcpy(tuning->params.isp_tunning_settings.plum_var, isp_tuning_gtm->plum_var,
				sizeof(isp_tuning_gtm->plum_var));
			/* offset */
			data_ptr += sizeof(struct isp_tuning_gtm_cfg);
			ret += sizeof(struct isp_tuning_gtm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CFA) /* isp_tuning_cfa */
		{
			struct isp_tuning_cfa_cfg *isp_tuning_cfa = (struct isp_tuning_cfa_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.cfa_dir_th = isp_tuning_cfa->dir_thres;
			tuning->params.isp_tunning_settings.cfa_interp_mode = isp_tuning_cfa->interp_mode;
			tuning->params.isp_tunning_settings.cfa_zig_zag = isp_tuning_cfa->zig_zag;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_cfa_cfg);
			ret += sizeof(struct isp_tuning_cfa_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CTC) /* isp_tuning_ctc */
		{
			struct isp_tuning_ctc_cfg *isp_tuning_ctc = (struct isp_tuning_ctc_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.ctc_th_min = isp_tuning_ctc->min_thres;
			tuning->params.isp_tunning_settings.ctc_th_max = isp_tuning_ctc->max_thres;
			tuning->params.isp_tunning_settings.ctc_th_slope = isp_tuning_ctc->slope_thres;
			tuning->params.isp_tunning_settings.ctc_dir_wt = isp_tuning_ctc->dir_wt;
			tuning->params.isp_tunning_settings.ctc_dir_th = isp_tuning_ctc->dir_thres;

			/* offset */
			data_ptr += sizeof(struct isp_tuning_ctc_cfg);
			ret += sizeof(struct isp_tuning_ctc_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_DIGITAL_GAIN) /* isp_tuning_digital_gain */
		{
			memcpy(tuning->params.isp_tunning_settings.bayer_gain, data_ptr, sizeof(struct isp_tuning_blc_gain_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_blc_gain_cfg);
			ret += sizeof(struct isp_tuning_blc_gain_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_LOW) /* isp_ccm_low */
		{
			struct isp_tuning_ccm_cfg *isp_ccm_low = (struct isp_tuning_ccm_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.cm_trig_cfg[0] = isp_ccm_low->temperature;
			memcpy(&(tuning->params.isp_tunning_settings.color_matrix_ini[0]), &isp_ccm_low->value,
				sizeof(struct isp_rgb2rgb_gain_offset));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_ccm_cfg);
			ret += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_MID) /* isp_ccm_mid */
		{
			struct isp_tuning_ccm_cfg *isp_ccm_mid = (struct isp_tuning_ccm_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.cm_trig_cfg[1] = isp_ccm_mid->temperature;
			memcpy(&(tuning->params.isp_tunning_settings.color_matrix_ini[1]), &isp_ccm_mid->value,
				sizeof(struct isp_rgb2rgb_gain_offset));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_ccm_cfg);
			ret += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_HIGH) /* isp_ccm_high */
		{
			struct isp_tuning_ccm_cfg *isp_ccm_high = (struct isp_tuning_ccm_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.cm_trig_cfg[2] = isp_ccm_high->temperature;
			memcpy(&(tuning->params.isp_tunning_settings.color_matrix_ini[2]), &isp_ccm_high->value,
				sizeof(struct isp_rgb2rgb_gain_offset));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_ccm_cfg);
			ret += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_PLTM) /* isp_tuning_pltm  */
		{
			memcpy(tuning->params.isp_tunning_settings.pltm_cfg, data_ptr, sizeof(struct isp_tuning_pltm_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_pltm_cfg);
			ret += sizeof(struct isp_tuning_pltm_cfg);
		}
#if (ISP_VERSION >= 521)
		if (cfg_ids & HW_ISP_CFG_TUNING_GCA) /* isp_tuning_gca  */
		{
			memcpy(tuning->params.isp_tunning_settings.gca_cfg, data_ptr, sizeof(struct isp_tuning_gca_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_gca_cfg);
			ret += sizeof(struct isp_tuning_gca_cfg);
		}
#endif
		break;
	case HW_ISP_CFG_TUNING_TABLES: /* isp tuning tables*/
		if (cfg_ids & HW_ISP_CFG_TUNING_LSC) /* isp_lsc */
		{
			struct isp_tuning_lsc_table_cfg *isp_tuning_lsc = (struct isp_tuning_lsc_table_cfg *)data_ptr;
#if (ISP_VERSION >= 521)
			tuning->params.isp_tunning_settings.lsc_mode = isp_tuning_lsc->lsc_mode;
#endif
			tuning->params.isp_tunning_settings.ff_mod = isp_tuning_lsc->ff_mod;
			tuning->params.isp_tunning_settings.lsc_center_x = isp_tuning_lsc->center_x;
			tuning->params.isp_tunning_settings.lsc_center_y = isp_tuning_lsc->center_y;
			tuning->params.isp_tunning_settings.rolloff_ratio = isp_tuning_lsc->rolloff_ratio;
			memcpy(&(tuning->params.isp_tunning_settings.lsc_tbl[0][0]), &(isp_tuning_lsc->value[0][0]),
				sizeof(isp_tuning_lsc->value));
			memcpy(tuning->params.isp_tunning_settings.lsc_trig_cfg, isp_tuning_lsc->color_temp_triggers,
				sizeof(isp_tuning_lsc->color_temp_triggers));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_lsc_table_cfg);
			ret += sizeof(struct isp_tuning_lsc_table_cfg);
		}

		if (cfg_ids & HW_ISP_CFG_TUNING_GAMMA) /* isp_gamma */
		{
			struct isp_tuning_gamma_table_cfg *isp_tuning_gamma = (struct isp_tuning_gamma_table_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.gamma_num = isp_tuning_gamma->number;
			memcpy(&(tuning->params.isp_tunning_settings.gamma_tbl_ini[0][0]), &(isp_tuning_gamma->value[0][0]),
				sizeof(isp_tuning_gamma->value));
			memcpy(tuning->params.isp_tunning_settings.gamma_trig_cfg, isp_tuning_gamma->lv_triggers,
				sizeof(isp_tuning_gamma->lv_triggers));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_gamma_table_cfg);
			ret += sizeof(struct isp_tuning_gamma_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_LINEARITY) /* isp_linearity */
		{
			memcpy(tuning->params.isp_tunning_settings.linear_tbl, data_ptr, sizeof(struct isp_tuning_linearity_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_linearity_table_cfg);
			ret += sizeof(struct isp_tuning_linearity_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_DISTORTION) /* isp_distortion */
		{
			memcpy(tuning->params.isp_tunning_settings.disc_tbl, data_ptr, sizeof(struct isp_tuning_distortion_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_distortion_table_cfg);
			ret += sizeof(struct isp_tuning_distortion_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_BDNF) /* isp_tuning_bdnf */
		{
			memcpy(tuning->params.isp_tunning_settings.isp_bdnf_th, data_ptr, sizeof(struct isp_tuning_bdnf_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_bdnf_table_cfg);
			ret += sizeof(struct isp_tuning_bdnf_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_TDNF) /* isp_tuning_tdnf */
		{
			struct isp_tuning_tdnf_table_cfg *isp_tuning_tdnf = (struct isp_tuning_tdnf_table_cfg *)data_ptr;
			memcpy(tuning->params.isp_tunning_settings.isp_tdnf_th, isp_tuning_tdnf->thres, sizeof(isp_tuning_tdnf->thres));
			memcpy(tuning->params.isp_tunning_settings.isp_tdnf_ref_noise, isp_tuning_tdnf->ref_noise, sizeof(isp_tuning_tdnf->ref_noise));
			memcpy(tuning->params.isp_tunning_settings.isp_tdnf_k, isp_tuning_tdnf->k_val, sizeof(isp_tuning_tdnf->k_val));
			memcpy(tuning->params.isp_tunning_settings.isp_tdnf_diff, isp_tuning_tdnf->diff, sizeof(isp_tuning_tdnf->diff));
#if (ISP_VERSION >= 520)
			memcpy(tuning->params.isp_tunning_settings.isp_d3d_k3d_incre_curve, isp_tuning_tdnf->d3d_k3d, sizeof(isp_tuning_tdnf->d3d_k3d));
#endif
			/* offset */
			data_ptr += sizeof(struct isp_tuning_tdnf_table_cfg);
			ret += sizeof(struct isp_tuning_tdnf_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CONTRAST) /* isp_tuning_contrast */
		{
			struct isp_tuning_contrast_table_cfg *isp_tuning_contrast = (struct isp_tuning_contrast_table_cfg *)data_ptr;
			memcpy(tuning->params.isp_tunning_settings.isp_contrast_val, isp_tuning_contrast->val, sizeof(isp_tuning_contrast->val));
			memcpy(tuning->params.isp_tunning_settings.isp_contrast_lum, isp_tuning_contrast->lum, sizeof(isp_tuning_contrast->lum));
			memcpy(tuning->params.isp_tunning_settings.isp_contrat_pe, isp_tuning_contrast->pe, sizeof(isp_tuning_contrast->pe));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_contrast_table_cfg);
			ret += sizeof(struct isp_tuning_contrast_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_SHARP) /* isp_tuning_sharp */
		{
			struct isp_tuning_sharp_table_cfg *isp_tuning_sharp = (struct isp_tuning_sharp_table_cfg *)data_ptr;
			memcpy(tuning->params.isp_tunning_settings.isp_sharp_val, isp_tuning_sharp->value, sizeof(isp_tuning_sharp->value));
			memcpy(tuning->params.isp_tunning_settings.isp_sharp_lum, isp_tuning_sharp->lum, sizeof(isp_tuning_sharp->lum));
#if (ISP_VERSION >= 520)
			memcpy(tuning->params.isp_tunning_settings.isp_sharp_edge_lum, isp_tuning_sharp->edge_lum, sizeof(isp_tuning_sharp->edge_lum));
			memcpy(tuning->params.isp_tunning_settings.isp_sharp_hfrq_lum, isp_tuning_sharp->hfrq_lum, sizeof(isp_tuning_sharp->hfrq_lum));
			memcpy(tuning->params.isp_tunning_settings.isp_sharp_hsv, isp_tuning_sharp->hsv, sizeof(isp_tuning_sharp->hsv));
			memcpy(tuning->params.isp_tunning_settings.isp_sharp_s_map, isp_tuning_sharp->smap, sizeof(isp_tuning_sharp->smap));
#endif
			/* offset */
			data_ptr += sizeof(struct isp_tuning_sharp_table_cfg);
			ret += sizeof(struct isp_tuning_sharp_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CEM) /* isp_tuning_cem */
		{
			memcpy(tuning->params.isp_tunning_settings.isp_cem_table, data_ptr, sizeof(struct isp_tuning_cem_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_cem_table_cfg);
			ret += sizeof(struct isp_tuning_cem_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CEM_1) /* isp_tuning_cem_1 */
		{
			memcpy(tuning->params.isp_tunning_settings.isp_cem_table1, data_ptr, sizeof(struct isp_tuning_cem_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_cem_table_cfg);
			ret += sizeof(struct isp_tuning_cem_table_cfg);
		}
#if (ISP_VERSION != 522)
		if (cfg_ids & HW_ISP_CFG_TUNING_PLTM_TBL) /* isp_tuning_pltm_table */
		{
			memcpy(tuning->params.isp_tunning_settings.isp_pltm_table, data_ptr, sizeof(struct isp_tuning_pltm_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_pltm_table_cfg);
			ret += sizeof(struct isp_tuning_pltm_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_WDR) /* isp_tuning_wdr */
		{
			memcpy(tuning->params.isp_tunning_settings.isp_wdr_table, data_ptr, sizeof(struct isp_tuning_wdr_table_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_wdr_table_cfg);
			ret += sizeof(struct isp_tuning_wdr_table_cfg);
		}
#endif
#if (ISP_VERSION >= 521)
		if (cfg_ids & HW_ISP_CFG_TUNING_LCA_TBL)
		{
			memcpy(tuning->params.isp_tunning_settings.lca_pf_satu_lut, data_ptr, sizeof(struct isp_tuning_lca_pf_satu_lut));
			// lca_pf_satu_lut offset
			data_ptr += sizeof(struct isp_tuning_lca_pf_satu_lut);
			ret += sizeof(struct isp_tuning_lca_pf_satu_lut);
			memcpy(tuning->params.isp_tunning_settings.lca_gf_satu_lut, data_ptr, sizeof(struct isp_tuning_lca_gf_satu_lut));
		    // lca_gf_satu_lut offset
			data_ptr += sizeof(struct isp_tuning_lca_gf_satu_lut);
			ret += sizeof(struct isp_tuning_lca_gf_satu_lut);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_MSC) /* isp_tuning_msc */
		{
			struct isp_tuning_msc_table_cfg *isp_tuning_msc = (struct isp_tuning_msc_table_cfg *)data_ptr;
			tuning->params.isp_tunning_settings.mff_mod = isp_tuning_msc->mff_mod;
			tuning->params.isp_tunning_settings.msc_mode = isp_tuning_msc->msc_mode;
			memcpy(tuning->params.isp_tunning_settings.msc_blw_lut, isp_tuning_msc->msc_blw_lut,
				sizeof(isp_tuning_msc->msc_blw_lut));
			memcpy(tuning->params.isp_tunning_settings.msc_blh_lut, isp_tuning_msc->msc_blh_lut,
				sizeof(isp_tuning_msc->msc_blh_lut));
			memcpy(tuning->params.isp_tunning_settings.msc_blw_dlt_lut, isp_tuning_msc->msc_blw_dlt_lut,
				sizeof(isp_tuning_msc->msc_blw_dlt_lut));
			memcpy(tuning->params.isp_tunning_settings.msc_blh_dlt_lut, isp_tuning_msc->msc_blh_dlt_lut,
				sizeof(isp_tuning_msc->msc_blh_dlt_lut));
			memcpy(&(tuning->params.isp_tunning_settings.msc_tbl[0][0]), &(isp_tuning_msc->value[0][0]),
				sizeof(isp_tuning_msc->value));
			memcpy(tuning->params.isp_tunning_settings.msc_trig_cfg, isp_tuning_msc->color_temp_triggers,
				sizeof(isp_tuning_msc->color_temp_triggers));

			/* offset */
			data_ptr += sizeof(struct isp_tuning_msc_table_cfg);
			ret += sizeof(struct isp_tuning_msc_table_cfg);
		}
#endif
		break;
	case HW_ISP_CFG_DYNAMIC: /* isp_dynamic_param */
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_LUM_POINT) /* isp_dynamic_lum_mapping_point */
		{
			memcpy(tuning->params.isp_iso_settings.isp_lum_mapping_point, data_ptr, sizeof(struct isp_dynamic_single_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_single_cfg);
			ret += sizeof(struct isp_dynamic_single_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_GAIN_POINT) /* isp_dynamic_gain_mapping_point */
		{
			memcpy(tuning->params.isp_iso_settings.isp_gain_mapping_point, data_ptr, sizeof(struct isp_dynamic_single_cfg));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_single_cfg);
			ret += sizeof(struct isp_dynamic_single_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SHARP) /* isp_dynamic_sharp */
		{
			struct isp_dynamic_sharp_cfg *isp_dynamic_sharp = (struct isp_dynamic_sharp_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.sharp_triger = (enum isp_triger_type)isp_dynamic_sharp->trigger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].sharp_cfg, isp_dynamic_sharp->tuning_cfg[i].value,
				        sizeof(isp_dynamic_sharp->tuning_cfg[i].value));
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_sharp_cfg);
			ret += sizeof(struct isp_dynamic_sharp_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_CONTRAST) /* isp_dynamic_contrast */
		{
			struct isp_dynamic_contrast_cfg *isp_dynamic_contrast = (struct isp_dynamic_contrast_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.contrast_triger = (enum isp_triger_type)isp_dynamic_contrast->trigger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].contrast_cfg, isp_dynamic_contrast->tuning_cfg[i].value,
					sizeof(isp_dynamic_contrast->tuning_cfg[i].value));
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_contrast_cfg);
			ret += sizeof(struct isp_dynamic_contrast_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DENOISE) /* isp_dynamic_denoise */
		{
			struct isp_dynamic_denoise_cfg *isp_dynamic_denoise = (struct isp_dynamic_denoise_cfg *)data_ptr;
			int i = 0;
			int j = 0;
			tuning->params.isp_iso_settings.triger.denoise_triger = (enum isp_triger_type)isp_dynamic_denoise->trigger;
			tuning->params.isp_iso_settings.triger.color_denoise_triger = (enum isp_triger_type)isp_dynamic_denoise->color_trigger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				if(isp_dynamic_denoise->tuning_cfg[i].ratio == 0)
					isp_dynamic_denoise->tuning_cfg[i].ratio = 1;
				for(j= 0; j < ISP_DENOISE_MAX; j++)
				{
					isp_dynamic_denoise->tuning_cfg[i].value[j] = isp_dynamic_denoise->tuning_cfg[i].value[j] * isp_dynamic_denoise->tuning_cfg[i].ratio;
				}
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].denoise_cfg, isp_dynamic_denoise->tuning_cfg[i].value,
					sizeof(isp_dynamic_denoise->tuning_cfg[i].value));
				tuning->params.isp_iso_settings.isp_dynamic_cfg[i].color_denoise = isp_dynamic_denoise->tuning_cfg[i].color_denoise;
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_denoise_cfg);
			ret += sizeof(struct isp_dynamic_denoise_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SENSOR_OFFSET) /* isp_dynamic_sensor_offset  */
		{
			struct isp_dynamic_sensor_offset_cfg *isp_dynamic_sensor_offset = (struct isp_dynamic_sensor_offset_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.sensor_offset_triger = (enum isp_triger_type)isp_dynamic_sensor_offset->trigger ;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].sensor_offset, isp_dynamic_sensor_offset->tuning_cfg[i].value,
					sizeof(isp_dynamic_sensor_offset->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_sensor_offset_cfg);
			ret += sizeof(struct isp_dynamic_sensor_offset_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_BLACK_LV) /* isp_dynamic_black_level  */
		{	struct isp_dynamic_black_level_cfg *isp_dynamic_black_level = (struct isp_dynamic_black_level_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.black_level_triger = (enum isp_triger_type)isp_dynamic_black_level->trigger ;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].black_level, isp_dynamic_black_level->tuning_cfg[i].value,
					sizeof(isp_dynamic_black_level->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_black_level_cfg);
			ret += sizeof(struct isp_dynamic_black_level_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DPC) /* isp_dynamic_dpc  */
		{	struct isp_dynamic_dpc_cfg *isp_dynamic_dpc = (struct isp_dynamic_dpc_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.dpc_triger = (enum isp_triger_type)isp_dynamic_dpc->trigger ;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].dpc_cfg, isp_dynamic_dpc->tuning_cfg[i].value,
					sizeof(isp_dynamic_dpc->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_dpc_cfg);
			ret += sizeof(struct isp_dynamic_dpc_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_PLTM) /* isp_dynamic_pltm  */
		{	struct isp_dynamic_pltm_cfg *isp_dynamic_pltm = (struct isp_dynamic_pltm_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.pltm_dynamic_triger = (enum isp_triger_type)isp_dynamic_pltm->trigger ;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].pltm_dynamic_cfg, isp_dynamic_pltm->tuning_cfg[i].value,
					sizeof(isp_dynamic_pltm->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_pltm_cfg);
			ret += sizeof(struct isp_dynamic_pltm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DEFOG) /* isp_dynamic_defog  */
		{	struct isp_dynamic_defog_cfg *isp_dynamic_defog = (struct isp_dynamic_defog_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.defog_value_triger = (enum isp_triger_type)isp_dynamic_defog->trigger ;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				tuning->params.isp_iso_settings.isp_dynamic_cfg[i].defog_value = isp_dynamic_defog->tuning_cfg[i].value;

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_defog_cfg);
			ret += sizeof(struct isp_dynamic_defog_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_HISTOGRAM) /* isp_dynamic_histogram */
		{
			struct isp_dynamic_histogram_cfg *isp_dynamic_histogram = (struct isp_dynamic_histogram_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.brightness_triger = (enum isp_triger_type)isp_dynamic_histogram->brightness_trigger;
			tuning->params.isp_iso_settings.triger.gcontrast_triger = (enum isp_triger_type)isp_dynamic_histogram->contrast_trigger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				tuning->params.isp_iso_settings.isp_dynamic_cfg[i].brightness = isp_dynamic_histogram->tuning_cfg[i].brightness;
				tuning->params.isp_iso_settings.isp_dynamic_cfg[i].contrast = isp_dynamic_histogram->tuning_cfg[i].contrast;
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_histogram_cfg);
			ret += sizeof(struct isp_dynamic_histogram_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SATURATION) /* isp_dynamic_saturation */
		{
			struct isp_dynamic_saturation_cfg *isp_dynamic_saturation = (struct isp_dynamic_saturation_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.saturation_triger = (enum isp_triger_type)isp_dynamic_saturation->trigger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
			{
				tuning->params.isp_iso_settings.isp_dynamic_cfg[i].saturation_cb = isp_dynamic_saturation->tuning_cfg[i].cb;
				tuning->params.isp_iso_settings.isp_dynamic_cfg[i].saturation_cr = isp_dynamic_saturation->tuning_cfg[i].cr;
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].saturation_cfg, isp_dynamic_saturation->tuning_cfg[i].value,
					sizeof(isp_dynamic_saturation->tuning_cfg[i].value));
			}

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_saturation_cfg);
			ret += sizeof(struct isp_dynamic_saturation_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_CEM) /* isp_dynamic_cem  */
		{	struct isp_dynamic_cem_cfg *isp_dynamic_cem = (struct isp_dynamic_cem_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.cem_ratio_triger = (enum isp_triger_type)isp_dynamic_cem->trigger ;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				tuning->params.isp_iso_settings.isp_dynamic_cfg[i].cem_ratio = isp_dynamic_cem->tuning_cfg[i].value;

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_cem_cfg);
			ret += sizeof(struct isp_dynamic_cem_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_TDF) /* isp_dynamic_tdf */
		{
			struct isp_dynamic_tdf_cfg *isp_dynamic_tdf = (struct isp_dynamic_tdf_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.tdf_triger = (enum isp_triger_type)isp_dynamic_tdf->trigger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].tdf_cfg, isp_dynamic_tdf->tuning_cfg[i].value,
					sizeof(isp_dynamic_tdf->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_tdf_cfg);
			ret += sizeof(struct isp_dynamic_tdf_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_AE) /* isp_dynamic_ae */
		{
			struct isp_dynamic_ae_cfg *isp_dynamic_ae = (struct isp_dynamic_ae_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.ae_cfg_triger = (enum isp_triger_type)isp_dynamic_ae->trigger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].ae_cfg, isp_dynamic_ae->tuning_cfg[i].value,
					sizeof(isp_dynamic_ae->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_ae_cfg);
			ret += sizeof(struct isp_dynamic_ae_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_GTM) /* isp_dynamic_gtm */
		{
			struct isp_dynamic_gtm_cfg *isp_dynamic_gtm = (struct isp_dynamic_gtm_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.gtm_cfg_triger = (enum isp_triger_type)isp_dynamic_gtm->trigger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].gtm_cfg, isp_dynamic_gtm->tuning_cfg[i].value,
					sizeof(isp_dynamic_gtm->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_gtm_cfg);
			ret += sizeof(struct isp_dynamic_gtm_cfg);
		}
#if (ISP_VERSION >= 521)
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_LCA) /* isp_dynamic_lca */
		{
			struct isp_dynamic_lca_cfg *isp_dynamic_lca = (struct isp_dynamic_lca_cfg *)data_ptr;
			int i = 0;
			tuning->params.isp_iso_settings.triger.lca_cfg_triger = (enum isp_triger_type)isp_dynamic_lca->trigger;
			for (i = 0; i < ISP_DYNAMIC_GROUP_COUNT; i++)
				memcpy(tuning->params.isp_iso_settings.isp_dynamic_cfg[i].lca_cfg, isp_dynamic_lca->tuning_cfg[i].value,
					sizeof(isp_dynamic_lca->tuning_cfg[i].value));

			/* offset */
			data_ptr += sizeof(struct isp_dynamic_lca_cfg);
			ret += sizeof(struct isp_dynamic_lca_cfg);
		}
#endif
		break;
	default:
		ret = AW_ERR_VI_INVALID_PARA;
		break;
	}
	ISP_PRINT("%s: set done(%d)\n", __FUNCTION__, ret);
	return ret;
}



