
/*
 ******************************************************************************
 *
 * isp.c
 *
 * Hawkview ISP - isp.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/05/27	VIDEO INPUT
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
#include <time.h>
#include <unistd.h>

#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/prctl.h>

#include <linux/spi/spidev.h>
#include <pthread.h>

#include "device/isp_dev.h"
#include "isp_dev/tools.h"

#include "isp_events/events.h"
#include "isp_tuning/isp_tuning_priv.h"
#include "isp_tuning.h"

#include "isp_manage.h"
#include "isp_version.h"

#include "iniparser/src/iniparser.h"
#include "include/isp_cmd_intf.h"
#include "include/V4l2Camera/sunxi_camera_v2.h"

#define ISP_STAT_SAVE		0

#define MEDIA_DEVICE		"/dev/media0"

struct hw_isp_media_dev media_params;
struct isp_lib_context isp_ctx[HW_ISP_DEVICE_NUM];
struct isp_tuning *tuning[HW_ISP_DEVICE_NUM];
struct isp_param_config iq_params[HW_ISP_DEVICE_NUM];
struct event_list events_arr[HW_ISP_DEVICE_NUM];
isp_hal_params isp_hal_interaction_params;

/*
 * Image quality
 */

static void __ae_done(struct isp_lib_context *lib,
			ae_result_t *result __attribute__((__unused__)))
{
	if(media_params.isp_sync_mode && (lib->isp_id == 0)) {
		result->ae_avg_lum = isp_ctx[1].ae_entity_ctx.ae_result.ae_avg_lum;
		result->ae_gain = isp_ctx[1].ae_entity_ctx.ae_result.ae_gain;
		result->ae_wdr_ratio = isp_ctx[1].ae_entity_ctx.ae_result.ae_wdr_ratio;
		result->BrightPixellValue = isp_ctx[1].ae_entity_ctx.ae_result.BrightPixellValue;
		result->DarkPixelValue = isp_ctx[1].ae_entity_ctx.ae_result.DarkPixelValue;
		result->wdr_hi_th = isp_ctx[1].ae_entity_ctx.ae_result.wdr_hi_th;
		result->wdr_low_th = isp_ctx[1].ae_entity_ctx.ae_result.wdr_low_th;
		memcpy(&result->sensor_set, &isp_ctx[1].ae_entity_ctx.ae_result.sensor_set, sizeof(sensor_setting_t));
		memcpy(&result->sensor_set_short, &isp_ctx[1].ae_entity_ctx.ae_result.sensor_set_short, sizeof(sensor_setting_t));
	}
	FUNCTION_LOG;
}
static void __af_done(struct isp_lib_context *lib,
			af_result_t *result __attribute__((__unused__)))
{
	if(media_params.isp_sync_mode && (lib->isp_id == 0)) {
		result->real_code_output = isp_ctx[1].af_entity_ctx.af_result.real_code_output;
		result->af_sap_lim_output = isp_ctx[1].af_entity_ctx.af_result.af_sap_lim_output;
		result->af_sharp_output = isp_ctx[1].af_entity_ctx.af_result.af_sharp_output;
		result->std_code_output = isp_ctx[1].af_entity_ctx.af_result.std_code_output;
		memcpy(&result->af_status_output, &isp_ctx[1].af_entity_ctx.af_result.af_status_output, sizeof(result->af_status_output));
	}
	FUNCTION_LOG;
}
static void __awb_done(struct isp_lib_context *lib,
			awb_result_t *result __attribute__((__unused__)))
{
	if(media_params.isp_sync_mode && (lib->isp_id == 0)) {
		result->color_temp_output = isp_ctx[1].awb_entity_ctx.awb_result.color_temp_output;
		memcpy(&result->wb_gain_output, &isp_ctx[1].awb_entity_ctx.awb_result.wb_gain_output, sizeof(result->wb_gain_output));
	}
	FUNCTION_LOG;
}
static void __afs_done(struct isp_lib_context *lib,
			afs_result_t *result __attribute__((__unused__)))
{
	if(media_params.isp_sync_mode && (lib->isp_id == 0)) {
		memcpy(&result->flicker_type_output, &isp_ctx[1].afs_entity_ctx.afs_result.flicker_type_output, sizeof(result->flicker_type_output));
	}
	FUNCTION_LOG;
}

static void __md_done(struct isp_lib_context *lib,
			md_result_t *result __attribute__((__unused__)))
{
	FUNCTION_LOG;
}

static void __pltm_done(struct isp_lib_context *lib,
			pltm_result_t *result __attribute__((__unused__)))
{
	if(media_params.isp_sync_mode && (lib->isp_id == 0)) {
		result->pltm_ae_comp = isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_ae_comp;
		result->pltm_block_height = isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_block_height;
		result->pltm_block_width = isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_block_width;
		result->pltm_cal_en = isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_cal_en;
		result->pltm_frm_sm_en = isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_frm_sm_en;
		result->pltm_last_order_ratio = isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_last_order_ratio;
		result->pltm_oripic_ratio = isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_oripic_ratio;
		result->pltm_tr_order = isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_tr_order;
		result->pltm_statistic_div = isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_statistic_div;
		memcpy(result->pltm_tbl, isp_ctx[1].pltm_entity_ctx.pltm_result.pltm_tbl, 768*sizeof(HW_U16));
	}
	FUNCTION_LOG;
}

void __isp_flash_open(struct isp_lib_context *isp_gen, struct hw_isp_device *isp)
{
	ae_result_t *ae_result = &isp_gen->ae_entity_ctx.ae_result;

	isp_gen->ae_settings.take_pic_start_cnt = isp_gen->ae_frame_cnt;
	ISP_PRINT("%s: TORCH_ON, ev_set.ev_idx:%d, take_pic_start_cnt:%d, flash_delay:%d.\n",
		__FUNCTION__, ae_result->sensor_set.ev_set.ev_idx,
		isp_gen->ae_settings.take_pic_start_cnt,
		isp_gen->isp_ini_cfg.isp_tunning_settings.flash_delay_frame);

	ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_TORCH;
	isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_TORCH);
	isp_gen->ae_settings.flash_open = 1;
}

void isp_flash_update_status(struct isp_lib_context *isp_gen, struct hw_isp_device *isp)
{
	isp_ae_entity_context_t *isp_ae_cxt = &isp_gen->ae_entity_ctx;
	ae_result_t *ae_result = &isp_gen->ae_entity_ctx.ae_result;
	int flash_gain = isp_gen->isp_ini_cfg.isp_tunning_settings.flash_gain;
	int flash_delay = isp_gen->isp_ini_cfg.isp_tunning_settings.flash_delay_frame;
	int ev_idx_flash = 0;

	switch (isp_gen->ae_settings.flash_mode) {
	case FLASH_MODE_ON:
		if (isp_gen->ae_settings.take_picture_flag == V4L2_TAKE_PICTURE_FLASH) {
			if(isp_gen->ae_settings.flash_open == 0) {
				__isp_flash_open(isp_gen, isp);
			}
		}
		break;
	case FLASH_MODE_AUTO:
		if (isp_gen->ae_settings.take_picture_flag == V4L2_TAKE_PICTURE_FLASH) {
			if(isp_gen->ae_settings.flash_open == 0) {
				if(ae_result->sensor_set.ev_set.ev_idx > (ae_result->sensor_set.ev_idx_max - 40)) {
					__isp_flash_open(isp_gen, isp);
				}
			} else {
				if (isp_gen->ae_frame_cnt == (isp_gen->ae_settings.take_pic_start_cnt + flash_delay))
				{
					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);

					isp_gen->ae_settings.exposure_lock = true;

					if (ae_result->ae_flash_ev_cumul >= 100)
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect;
					}
					else if (ae_result->ae_flash_ev_cumul < 100 && ae_result->ae_flash_ev_cumul >= flash_gain*100/256)
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect;
					}
					else if (ae_result->ae_flash_ev_cumul >= -25 && ae_result->ae_flash_ev_cumul < flash_gain*100/256)
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect + ae_result->ae_flash_ev_cumul * flash_gain/256;
					}
					else
					{
						ev_idx_flash = ae_result->sensor_set.ev_idx_expect + ae_result->ae_flash_ev_cumul * flash_gain/256;
					}
					ev_idx_flash = clamp(ev_idx_flash, 1, ae_result->sensor_set.ev_idx_max);

					ISP_PRINT("%s: FLASH_OFF, ae_flash_ev_cumul:%d, ev_idx_expect:%d, ev_idx_flash:%d, flash_gain:%d.\n",
						__FUNCTION__,	ae_result->ae_flash_ev_cumul,
						ae_result->sensor_set.ev_idx_expect, ev_idx_flash,
						isp_gen->isp_ini_cfg.isp_tunning_settings.flash_gain);

					isp_ae_cxt->ae_param.u.ae_pline_index = ev_idx_flash;
					isp_ae_set_params_helper(isp_ae_cxt, ISP_AE_SET_EXP_IDX);
				}
				else if (isp_gen->ae_frame_cnt == (1 + isp_gen->ae_settings.take_pic_start_cnt + flash_delay))
				{
					ISP_PRINT("%s: FLASH_ON.\n", __FUNCTION__);

					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_FLASH;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_FLASH);
				}
				else if ((isp_gen->ae_frame_cnt >= (4 + isp_gen->ae_settings.take_pic_start_cnt + flash_delay)) &&
					(isp_gen->ae_frame_cnt <= (6 + isp_gen->ae_settings.take_pic_start_cnt + flash_delay)))
				{
					ae_result->ae_flash_ok = 1;
					isp_hal_interaction_params.ith_params.image_para.bits.flash_ok = ae_result->ae_flash_ok;
				}
				else if (isp_gen->ae_frame_cnt == (7 + isp_gen->ae_settings.take_pic_start_cnt + flash_delay))
				{
					ISP_PRINT("%s: FLASH_OFF.\n", __FUNCTION__);

					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);

					isp_gen->ae_settings.exposure_lock = false;
				}
			}
		}
		break;
	case FLASH_MODE_TORCH:
		if (isp_gen->ae_settings.take_picture_flag == V4L2_TAKE_PICTURE_FLASH) {
			if (isp_gen->ae_settings.flash_open == 0) {
				__isp_flash_open(isp_gen, isp);
 			} else {
				if ((isp_gen->ae_frame_cnt >= (1 + isp_gen->ae_settings.take_pic_start_cnt + flash_delay))
					&& (isp_gen->ae_frame_cnt <= (3 + isp_gen->ae_settings.take_pic_start_cnt + flash_delay)))
				{
					ae_result->ae_flash_ok = 1;
					isp_hal_interaction_params.ith_params.image_para.bits.flash_ok = ae_result->ae_flash_ok;
				}
				if (isp_gen->ae_frame_cnt == (4 + isp_gen->ae_settings.take_pic_start_cnt + flash_delay))
				{
					ISP_PRINT("%s: FLASH_OFF.\n", __FUNCTION__);
					ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_TORCH;
					isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
				}
			}
		} else {
			ISP_PRINT("%s: TORCH_ON, FLASH_MODE TORCH_ON.\n", __FUNCTION__);
			ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_TORCH;
			isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_TORCH);
		}
		break;
	case FLASH_MODE_RED_EYE:
		break;
	case FLASH_MODE_OFF:
		ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
		isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
		break;
	default:
		ae_result->ae_flash_led = V4L2_FLASH_LED_MODE_NONE;
		isp_flash_ctrl(isp, V4L2_FLASH_LED_MODE_NONE);
		break;
	}
}

static void __isp_stats_process(struct hw_isp_device *isp,	const void *buffer)
{
	struct isp_lib_context *ctx;
	struct isp_table_reg_map reg;
	struct isp_table_reg_map tbl;
	struct sensor_exp_gain exp_gain;
	ae_result_t *result = NULL;
	awb_result_t *awb_result = NULL;

	ctx = isp_dev_get_ctx(isp);
	if (ctx == NULL)
		return;
	if(media_params.isp_sync_mode) {
		if(isp_dev_get_dev_id(isp) == 1) {
			memcpy(isp_ctx[1].isp1_stat_buf, buffer, ISP_STAT_TOTAL_SIZE);
			memcpy(isp_ctx[0].isp1_stat_buf, buffer, ISP_STAT_TOTAL_SIZE);
		}
		if(isp_dev_get_dev_id(isp) == 0) {
			memcpy(isp_ctx[1].isp0_stat_buf, buffer, ISP_STAT_TOTAL_SIZE);
			memcpy(isp_ctx[0].isp0_stat_buf, buffer, ISP_STAT_TOTAL_SIZE);
		}
		isp_ctx_stats_prepare_sync(ctx, (void*)buffer);
	} else {
		isp_ctx_stats_prepare(ctx, (void*)buffer);
	}
	FUNCTION_LOG;
	isp_ctx_algo_run(ctx);
	FUNCTION_LOG;

	if (ctx->isp_ini_cfg.isp_test_settings.af_en) {
		if (ctx->af_entity_ctx.af_result.last_code_output != ctx->af_entity_ctx.af_result.real_code_output) {
			isp_act_set_pos(isp, ctx->af_entity_ctx.af_result.real_code_output);
			ctx->af_entity_ctx.af_result.last_code_output = ctx->af_entity_ctx.af_result.real_code_output;
		}
	}

	isp_flash_update_status(ctx, isp);

	result = &ctx->ae_entity_ctx.ae_result;
	awb_result = &ctx->awb_entity_ctx.awb_result;
	exp_gain.exp_val = result->sensor_set.ev_set_curr.ev_sensor_exp_line;
	exp_gain.gain_val = result->sensor_set.ev_set_curr.ev_analog_gain/16;
	exp_gain.r_gain = awb_result->wb_gain_output.r_gain;
	exp_gain.b_gain = awb_result->wb_gain_output.b_gain;
	isp_sensor_set_exp_gain(isp, &exp_gain);

	FUNCTION_LOG;

	reg.addr = ctx->load_reg_base;
	reg.size = 0x400;
	isp_set_load_reg(isp, &reg);

	tbl.addr = ctx->module_cfg.table_mapping1;
	tbl.size = ISP_TABLE_MAPPING1_SIZE;
	isp_set_table1_map(isp, &tbl);

	tbl.addr = ctx->module_cfg.table_mapping2;
	tbl.size = ISP_TABLE_MAPPING2_SIZE;
	isp_set_table2_map(isp, &tbl);
}
static void __isp_fsync_process(struct hw_isp_device *isp __attribute__((__unused__)))
{

}

void __isp_ctrl_process(struct hw_isp_device *isp, struct v4l2_event *event)
{
	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	HW_S32 iso_qmenu[] = { 100, 200, 400, 800, 1600, 3200, 6400};
	HW_S32 exp_bias_qmenu[] = { -4, -3, -2, -1, 0, 1, 2, 3, 4, };

	if (isp_gen == NULL)
		return;

	switch(event->id) {
	case V4L2_CID_BRIGHTNESS:
		bsp_isp_s_brightness(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_CONTRAST:
		bsp_isp_s_contrast(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_SATURATION:
		bsp_isp_s_saturation(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_HUE:
		bsp_isp_s_hue(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_AUTO_WHITE_BALANCE:
		bsp_isp_s_auto_white_balance(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_EXPOSURE:
		bsp_isp_s_exposure(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_AUTOGAIN:
		bsp_isp_s_auto_gain(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_GAIN:
		bsp_isp_s_gain(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_POWER_LINE_FREQUENCY:
		bsp_isp_s_power_line_frequency(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_WHITE_BALANCE_TEMPERATURE:
		bsp_isp_s_white_balance_temperature(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_SHARPNESS:
		bsp_isp_s_sharpness(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_AUTOBRIGHTNESS:
		bsp_isp_s_auto_brightness(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_BAND_STOP_FILTER:
		bsp_isp_s_band_stop_filter(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_ILLUMINATORS_1:
		bsp_isp_s_illuminators_1(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_ILLUMINATORS_2:
		bsp_isp_s_illuminators_2(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_EXPOSURE_AUTO:
		bsp_isp_s_exposure_auto(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_EXPOSURE_ABSOLUTE:
		bsp_isp_s_exposure_absolute(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_FOCUS_ABSOLUTE:
		bsp_isp_s_focus_absolute(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_FOCUS_RELATIVE:
		bsp_isp_s_focus_relative(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_FOCUS_AUTO:
		bsp_isp_s_focus_auto(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_AUTO_EXPOSURE_BIAS:
		bsp_isp_s_auto_exposure_bias(isp_gen, exp_bias_qmenu[event->u.ctrl.value]);
		break;
	case V4L2_CID_AUTO_N_PRESET_WHITE_BALANCE:
		bsp_isp_s_auto_n_preset_white_balance(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_ISO_SENSITIVITY:
		bsp_isp_s_iso_sensitivity(isp_gen, iso_qmenu[event->u.ctrl.value]);
		break;
	case V4L2_CID_ISO_SENSITIVITY_AUTO:
		bsp_isp_s_iso_sensitivity_auto(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_EXPOSURE_METERING:
		bsp_isp_s_ae_metering_mode(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_SCENE_MODE:
		bsp_isp_s_scene_mode(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_3A_LOCK:
		//bsp_isp_s_3a_lock(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_AUTO_FOCUS_START:
		bsp_isp_s_auto_focus_start(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_AUTO_FOCUS_STOP:
		bsp_isp_s_auto_focus_stop(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_AUTO_FOCUS_RANGE:
		bsp_isp_s_auto_focus_range(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_TAKE_PICTURE:
		ISP_ERR("V4L2_CID_TAKE_PICTURE.\n");
		bsp_isp_s_take_picture(isp_gen, event->u.ctrl.value);
		break;
	case V4L2_CID_FLASH_LED_MODE:
		ISP_ERR("V4L2_CID_FLASH_LED_MODE.\n");
		bsp_isp_s_flash_mode(isp_gen, event->u.ctrl.value);
		break;
	default:
		ISP_ERR("Unknown ctrl.\n");
		break;
	}
}

static void __isp_stream_off(struct hw_isp_device *isp __attribute__((__unused__)))
{
	int dev_id = 0;

	dev_id = isp_dev_get_dev_id(isp);
	if ((dev_id >= HW_ISP_DEVICE_NUM) || (dev_id == -1))
		ISP_ERR("ISP ID is invalid, __isp_stream_off failed!\n");

	events_stop(&events_arr[dev_id]);
}

static void __isp_monitor_fd(int id, int fd, enum hw_isp_event_type type,
			      void(*callback)(void *), void *priv)
{
	events_monitor_fd(&events_arr[id], fd, type, callback, priv);
}

static void __isp_unmonitor_fd(int id, int fd)
{
	events_unmonitor_fd(&events_arr[id], fd);
}

static struct isp_ctx_operations isp_ctx_ops = {
	.ae_done = __ae_done,
	.af_done = __af_done,
	.awb_done = __awb_done,
	.afs_done = __afs_done,
	.md_done = __md_done,
	.pltm_done = __pltm_done,
};

static struct isp_dev_operations isp_dev_ops = {
	.stats_ready = __isp_stats_process,
	.fsync = __isp_fsync_process,
	.stream_off = __isp_stream_off,
	.vsync = NULL,
	.ctrl_process = __isp_ctrl_process,
	.monitor_fd = __isp_monitor_fd,
	.unmonitor_fd = __isp_unmonitor_fd,
};

static void *__isp_thread(void *arg)
{
	int ret = 0;
	int dev_id = 0;
	struct hw_isp_device *isp = (struct hw_isp_device *)arg;

	dev_id = isp_dev_get_dev_id(isp);
	if ((dev_id >= HW_ISP_DEVICE_NUM) || (dev_id == -1))
		ISP_ERR("ISP ID is invalid, isp_run failed!\n");

	ret = isp_dev_start(isp);
	if (ret < 0)
		goto end;
	if (events_loop(&events_arr[dev_id]))
		goto end;
end:
	isp_dev_stop(isp);
	return NULL;
}

static int __isp_get_saved_ctx(int dev_id)
{
	FILE *file_fd = NULL;
	char fdstr[50];

	sprintf(fdstr, "/mnt/isp%d_ctx_saved.bin", dev_id);
	file_fd = fopen(fdstr, "rb");

	if (!file_fd) {
		ISP_WARN("open /mnt/isp%d_ctx_saved.bin failed!!!\n", dev_id);
		return -1;
	} else {
		ISP_WARN("open /mnt/isp%d_ctx_saved.bin success!!!\n", dev_id);
		fread(&isp_ctx[dev_id], sizeof(isp_ctx[dev_id]), 1, file_fd);
	}

	fclose(file_fd);

	return 0;
}

static int __isp_set_saved_ctx(int dev_id)
{
	FILE *file_fd = NULL;
	char fdstr[50];

	sprintf(fdstr, "/mnt/isp%d_ctx_saved.bin", dev_id);
	file_fd = fopen(fdstr, "wb");

	if (!file_fd) {
		ISP_WARN("open /mnt/isp%d_ctx_saved.bin failed!!!\n", dev_id);
		return -1;
	} else {
		ISP_WARN("open /mnt/isp%d_ctx_saved.bin success!!!\n", dev_id);
		fwrite(&isp_ctx[dev_id], sizeof(isp_ctx[dev_id]), 1, file_fd);
	}

	fclose(file_fd);

	return 0;
}
int __isp_set_sync(int mode)
{
	if(mode == 0 || mode == 1)
		media_params.isp_sync_mode = mode;
	else
		return -1;
	return 0;
}

int media_dev_init(void)
{
	/*must be called before all isp init*/
	//memset(&media_params, 0, sizeof(media_params));
	return 0;
}

void media_dev_exit(void)
{
	/*must be called after all isp exit*/
	if ((media_params.isp_use_cnt[0] == 0) && (media_params.isp_use_cnt[1] == 0)) {
		if (media_params.mdev) {
			media_close(media_params.mdev);
			media_params.mdev = NULL;
		}
	}
}

int isp_reset(int dev_id)
{
	int ret = 0;
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_params_parse(isp, &iq_params[dev_id], media_params.isp_sync_mode);

	ret = isp_tuning_reset(isp, &iq_params[dev_id]);
	if (ret) {
		ISP_ERR("error: unable to reset isp tuning\n");
	}
	return ret;
}

int isp_init(int dev_id)
{
	int ret = 0;
	struct hw_isp_device *isp = NULL;
	char fdstr[50];
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (media_params.isp_use_cnt[dev_id]++ > 0)
		return 0;

	/*update media entity and links*/
	if (media_params.mdev) {
		media_close(media_params.mdev);
		media_params.mdev = NULL;
	}

	memset(&isp_hal_interaction_params, 0, sizeof(isp_hal_params));

	media_params.mdev = media_open(MEDIA_DEVICE, 0);
	if (media_params.mdev == NULL) {
		ISP_ERR("isp%d update media entity and links failed!\n", dev_id);
		return -1;
	}

	isp_version_info();

	ret = isp_dev_open(&media_params, dev_id);
	if (ret < 0)
		return ret;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	isp_dev_register(isp, &isp_dev_ops);
	isp_dev_banding_ctx(isp, &isp_ctx[dev_id]);

	__isp_get_saved_ctx(dev_id);

#if ISP_STAT_SAVE
	sprintf(fdstr, "/mnt/csi_test/isp%d_stat_log.bin", dev_id);
	isp_ctx[dev_id].stat_log_fd = fopen(fdstr, "wb");
	if (isp_ctx[dev_id].stat_log_fd == NULL) {
		ISP_WARN("/mnt/csi_test/isp%d_stat_log.bin failed!!!\n", dev_id);
		return -1;
	} else {
		ISP_PRINT("/mnt/csi_test/isp%d_stat_log.bin success!!!\n", dev_id);
	}
#endif

	isp_ctx[dev_id].isp_id = dev_id;

	isp_config_sensor_info(isp);

	isp_params_parse(isp, &iq_params[dev_id], media_params.isp_sync_mode);

	FUNCTION_LOG;
	isp_ctx_algo_init(&isp_ctx[dev_id], &isp_ctx_ops);
	FUNCTION_LOG;

	tuning[dev_id] = isp_tuning_init(isp, &iq_params[dev_id]);
	if (tuning[dev_id] == NULL) {
		ISP_ERR("error: unable to initialize isp tuning\n");
		return -1;
	}
	FUNCTION_LOG;

	if (isp_ctx[dev_id].isp_ini_cfg.isp_test_settings.af_en || isp_ctx[dev_id].isp_ini_cfg.isp_test_settings.isp_test_focus)
		isp_act_init_range(isp, isp_ctx[dev_id].isp_ini_cfg.isp_3a_settings.vcm_min_code, isp_ctx[dev_id].isp_ini_cfg.isp_3a_settings.vcm_max_code);

	events_arr[dev_id].isp_id = dev_id;
	events_init(&events_arr[dev_id]);
	events_star(&events_arr[dev_id]);
	FUNCTION_LOG;

	ISP_DEV_LOG(ISP_LOG_ISP, "isp%d init end!!!\n", dev_id);

	return 0;
}

int isp_update(int dev_id)
{
	int ret = 0;
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ret = isp_tuning_update(isp);
	if (ret) {
		ISP_ERR("error: unable to update isp tuning\n");
	}
	return ret;
}

int isp_getparams(void * pParams) {
	memcpy(pParams, &isp_hal_interaction_params, sizeof(isp_hal_params));
	return 0;
}

int isp_stop(int dev_id)
{
	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (media_params.isp_use_cnt[dev_id] == 1)
		events_stop(&events_arr[dev_id]);

	return 0;
}

int isp_exit(int dev_id)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if ((media_params.isp_use_cnt[dev_id] == 0) || (--media_params.isp_use_cnt[dev_id] > 0))
		return 0;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	/*wait to exit until the thread is finished*/
	pthread_join(media_params.isp_tid[dev_id], NULL);

#if ISP_STAT_SAVE
	fclose(isp_ctx[dev_id].stat_log_fd);
	isp_ctx[dev_id].stat_log_fd = NULL;
#endif
	__isp_set_saved_ctx(dev_id);

	isp_tuning_exit(isp);
	isp_ctx_algo_exit(&isp_ctx[dev_id]);
	isp_dev_close(&media_params, dev_id);

	ISP_DEV_LOG(ISP_LOG_ISP, "isp%d exit end!!!\n", dev_id);

	return 0;
}

int isp_run(int dev_id)
{
	int ret = 0;
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (media_params.isp_use_cnt[dev_id] > 1)
		return 0;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ISP_PRINT("create isp%d server thread!\n", dev_id);

	ret = pthread_create(&media_params.isp_tid[dev_id], NULL, __isp_thread, isp);
	if(ret != 0)
		ISP_ERR("%s: %s\n",__func__, strerror(ret));

	return ret;
}

HW_S32 isp_pthread_join(int dev_id)
{
#if 0
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	if (media_params.isp_use_cnt[dev_id] == 1)
		pthread_join(media_params.isp_tid[dev_id], NULL);
#endif
	return 0;
}

HW_S32 isp_get_cfg(int dev_id, HW_U8 group_id, HW_U32 cfg_ids, void *cfg_data)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	return isp_tuning_get_cfg(isp, group_id, cfg_ids, cfg_data);
}
HW_S32 isp_set_cfg(int dev_id, HW_U8 group_id, HW_U32 cfg_ids, void *cfg_data)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	return isp_tuning_set_cfg(isp, group_id, cfg_ids, cfg_data);
}

HW_S32 isp_stats_req(int dev_id, struct isp_stats_context *stats_ctx)
{
	struct hw_isp_device *isp = NULL;
	struct isp_lib_context *ctx = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ctx = isp_dev_get_ctx(isp);
	if (ctx == NULL)
		return -1;

	return isp_ctx_stats_req(ctx, stats_ctx);
}

HW_S32 isp_set_saved_ctx(int dev_id)
{
	return __isp_set_saved_ctx(dev_id);
}

int isp_set_fps(int dev_id, int s_fps)
{
	struct hw_isp_device *isp = NULL;
	struct sensor_fps fps;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	fps.fps = s_fps;
	isp_sensor_set_fps(isp, &fps);
	if(s_fps > 1)
		isp_ctx_update_ae_tbl(&isp_ctx[dev_id], s_fps);

	return 0;
}

HW_S32 isp_get_sensor_info(int dev_id, struct sensor_config *cfg)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}
	memset(cfg, 0, sizeof(struct sensor_config));

	isp_sensor_get_configs(isp, cfg);

	return 0;
}

/*******************isp for video buffer*********************/
HW_S32 isp_get_lv(int dev_id)
{
	struct hw_isp_device *isp;
	struct isp_lib_context *ctx;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		//ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	ctx = isp_dev_get_ctx(isp);
	if (ctx == NULL) {
		//ISP_ERR("isp%d get isp ctx failed!\n", dev_id);
		return -1;
	}

	return ctx->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_lv;
}

HW_S32 isp_get_attr_cfg(int dev_id, HW_U32 ctrl_id, HW_S32 *value)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL)
		return -1;

	switch(ctrl_id) {
		case ISP_CTRL_DIGITAL_GAIN:
			*value = isp_gen->ae_entity_ctx.ae_result.sensor_set.ev_set_curr.ev_digital_gain;
			break;
		case ISP_CTRL_PLTMWDR_STR:
			*value = isp_gen->tune.pltmwdr_level;
			break;
		case ISP_CTRL_DN_STR:
			*value = isp_gen->tune.denoise_level;
			break;
		case ISP_CTRL_3DN_STR:
			*value = isp_gen->tune.tdf_level;
			break;
		case ISP_CTRL_HIGH_LIGHT:
			*value = isp_gen->tune.highlight_level;
			break;
		case ISP_CTRL_BACK_LIGHT:
			*value = isp_gen->tune.backlight_level;
			break;
		default:
			ISP_ERR("Unknown ctrl.\n");
			break;
	}
	return 0;
}

HW_S32 isp_set_attr_cfg(int dev_id, HW_U32 ctrl_id, HW_S32 *value)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;

	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL)
		return -1;

	switch(ctrl_id) {
		case ISP_CTRL_DIGITAL_GAIN:
			break;
		case ISP_CTRL_PLTMWDR_STR:
			if (isp_gen->tune.pltmwdr_level != *value)
				isp_gen->tune.pltmwdr_level = *value;
			break;
		case ISP_CTRL_DN_STR:
			if (isp_gen->tune.denoise_level != *value)
				isp_gen->tune.denoise_level = *value;
			break;
		case ISP_CTRL_3DN_STR:
			if (isp_gen->tune.tdf_level != *value)
				isp_gen->tune.tdf_level = *value;
			break;
		case ISP_CTRL_HIGH_LIGHT:
			if (isp_gen->tune.highlight_level != *value)
				isp_gen->tune.highlight_level = *value;
			break;
		case ISP_CTRL_BACK_LIGHT:
			if (isp_gen->tune.backlight_level != *value)
				isp_gen->tune.backlight_level = *value;
			break;
		default:
			ISP_ERR("Unknown ctrl.\n");
			break;
	}
	return 0;
}

HW_S32 isp_set_gain_cfg(int dev_id, struct gain_cfg *gains)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;
	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}
	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL) {
		ISP_ERR("isp_gen is NULL!\n");
		return -1;
	}

	if (isp_gen->tune.gains.ana_gain_min != gains->ana_gain_min)
	{
		isp_gen->tune.gains.ana_gain_min = gains->ana_gain_min;
		isp_gen->isp_3a_change_flags |= ISP_SET_GAIN_STR;
	}
	if (isp_gen->tune.gains.ana_gain_max != gains->ana_gain_max)
	{
		isp_gen->tune.gains.ana_gain_max = gains->ana_gain_max;
		isp_gen->isp_3a_change_flags |= ISP_SET_GAIN_STR;
	}
	if (isp_gen->tune.gains.dig_gain_min != gains->dig_gain_min)
	{
		isp_gen->tune.gains.dig_gain_min = gains->dig_gain_min;
		isp_gen->isp_3a_change_flags |= ISP_SET_GAIN_STR;
	}
	if (isp_gen->tune.gains.dig_gain_max != gains->dig_gain_max)
	{
		isp_gen->tune.gains.dig_gain_max = gains->dig_gain_max;
		isp_gen->isp_3a_change_flags |= ISP_SET_GAIN_STR;
	}
	if (isp_gen->tune.gains.gain_favor != gains->gain_favor)
	{
		isp_gen->tune.gains.gain_favor = gains->gain_favor;
		isp_gen->isp_3a_change_flags |= ISP_SET_GAIN_STR;
	}
	return 0;
}

HW_S32 isp_get_gain_cfg(int dev_id, struct gain_cfg *gains)
{
	struct hw_isp_device *isp = NULL;

	if (dev_id >= HW_ISP_DEVICE_NUM)
		return -1;
	isp = media_params.isp_dev[dev_id];
	if (!isp) {
		ISP_ERR("isp%d device is NULL!\n", dev_id);
		return -1;
	}

	struct isp_lib_context *isp_gen = isp_dev_get_ctx(isp);
	if (isp_gen == NULL)
		return -1;

	gains->ana_gain_min = isp_gen->tune.gains.ana_gain_min;
	gains->ana_gain_max = isp_gen->tune.gains.ana_gain_max;
	gains->dig_gain_min = isp_gen->tune.gains.dig_gain_min;
	gains->dig_gain_max = isp_gen->tune.gains.dig_gain_max;
	gains->gain_favor = isp_gen->tune.gains.gain_favor;

	return 0;
}

/********************isp cmd interface for effect set**********************/
void bsp_isp_s_brightness(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->tune.brightness_level != value)
	{
		isp_gen->tune.brightness_level = value;
		isp_gen->isp_3a_change_flags |= ISP_SET_BRIGHTNESS;
	}
}

void bsp_isp_s_contrast(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->tune.contrast_level != value)
	{
		isp_gen->tune.contrast_level = value;
		isp_gen->isp_3a_change_flags |= ISP_SET_CONTRAST;
	}
}

void bsp_isp_s_saturation(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->tune.saturation_level != value)
	{
		isp_gen->tune.saturation_level = value;
		isp_gen->isp_3a_change_flags |= ISP_SET_SATURATION;
	}
}

void bsp_isp_s_hue(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->tune.hue_level != value)
	{
		isp_gen->tune.hue_level = value;
		isp_gen->isp_3a_change_flags |= ISP_SET_HUE;
	}
}

void bsp_isp_s_auto_white_balance(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->awb_settings.wb_mode != value)
	{
		isp_gen->awb_settings.wb_mode = value;
		isp_gen->isp_3a_change_flags |= ISP_SET_AWB_MODE;
	}
}

void bsp_isp_s_exposure(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->sensor_info.pclk)
		isp_gen->ae_settings.exp_absolute = (((double)value / 16) * 1000000 * isp_gen->sensor_info.hts / isp_gen->sensor_info.pclk);
}

void bsp_isp_s_auto_gain(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.iso_mode != value)
		isp_gen->ae_settings.iso_mode = value;
}

void bsp_isp_s_gain(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.sensor_gain != value)
		isp_gen->ae_settings.sensor_gain = value;
}

void bsp_isp_s_power_line_frequency(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.flicker_mode != value)
	{
		isp_gen->ae_settings.flicker_mode = value;
		isp_gen->isp_3a_change_flags |= ISP_SET_FLICKER_MODE;
	}
}

void bsp_isp_s_white_balance_temperature(struct isp_lib_context *isp_gen, int value)
{
	isp_gen->awb_settings.wb_mode = WB_MANUAL;
	if (isp_gen->awb_settings.wb_temperature != value)
		isp_gen->awb_settings.wb_temperature = value;
}

void bsp_isp_s_sharpness(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->tune.sharpness_level != value)
	{
		isp_gen->tune.sharpness_level = value;
		isp_gen->isp_3a_change_flags |= ISP_SET_SHARPNESS;
	}
}

void bsp_isp_s_auto_brightness(struct isp_lib_context *isp_gen, int value)
{
	if(1 == value)
		isp_gen->tune.brightness_level = 0;
}

void bsp_isp_s_band_stop_filter(struct isp_lib_context *isp_gen, int value)
{
	if (value  == 1)
	{
		isp_gen->ae_settings.flicker_mode = FREQUENCY_AUTO;
		isp_gen->isp_3a_change_flags |= ISP_SET_FLICKER_MODE;
	}
	else
	{
		isp_gen->ae_settings.flicker_mode = FREQUENCY_DISABLED;
		isp_gen->isp_3a_change_flags |= ISP_SET_FLICKER_MODE;
	}
}

void bsp_isp_s_illuminators_1(struct isp_lib_context *isp_gen, int value)
{
	if (0 == value)
		isp_gen->ae_settings.flash_mode = FLASH_MODE_OFF;
	else
		isp_gen->ae_settings.flash_mode = FLASH_MODE_TORCH;
}

void bsp_isp_s_illuminators_2(struct isp_lib_context *isp_gen, int value)
{
	;
}

void bsp_isp_s_af_metering_mode(struct isp_lib_context *isp_gen, int value, struct isp_h3a_coor_win *coor)
{

	isp_gen->af_settings.af_metering_mode = value;
	if ((value == AUTO_FOCUS_METERING_SPOT) && (coor != NULL))
	{
		//isp_gen->af_settings.af_mode = AUTO_FOCUS_TOUCH;
		isp_gen->af_settings.af_coor.x1 = coor->x1;
		isp_gen->af_settings.af_coor.y1 = coor->y1;
		isp_gen->af_settings.af_coor.x2 = coor->x2;
		isp_gen->af_settings.af_coor.y2 = coor->y2;
	}
	else
	{
		//isp_gen->af_settings.af_mode = AUTO_FOCUS_CONTINUEOUS;
	}
	isp_gen->isp_3a_change_flags |= ISP_SET_AF_METERING_MODE;
}

void bsp_isp_s_flash_mode(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.flash_mode != value)
		isp_gen->ae_settings.flash_mode= value;
}
void bsp_isp_s_ae_metering_mode(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.exp_metering_mode != value) {
		isp_gen->ae_settings.exp_metering_mode = value;
		isp_gen->ae_settings.exposure_lock = false;
		isp_gen->isp_3a_change_flags |= ISP_SET_AE_METERING_MODE;
	}
}
/*
void bsp_isp_s_ae_metering_mode(struct isp_lib_context *isp_gen, int value, struct isp_h3a_coor_win *coor)
{
	if (isp_gen->exp_settings.exp_metering_mode  != value) {
		isp_gen->ae_settings.exp_metering_mode = value;
		if ((value == AE_METERING_MODE_SPOT) && (coor != NULL)) {
			isp_gen->ae_settings.ae_coor.x1 = coor->x1;
			isp_gen->ae_settings.ae_coor.y1 = coor->y1;
			isp_gen->ae_settings.ae_coor.x2 = coor->x2;
			isp_gen->ae_settings.ae_coor.y2 = coor->y2;
		}
		isp_gen->ae_settings.exposure_lock = false;
		isp_gen->isp_3a_change_flags |= ISP_SET_AE_METERING_MODE;
	}
}
*/

void bsp_isp_s_light_mode(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.light_mode != value)
		isp_gen->ae_settings.light_mode = value;
}

void bsp_isp_s_exposure_auto(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.exp_mode != value)
		isp_gen->ae_settings.exp_mode = value;
}

void bsp_isp_s_exposure_absolute(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.exp_absolute != value)
		isp_gen->ae_settings.exp_absolute = value;
}

void bsp_isp_s_aperture(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.iris_fno != value)
		isp_gen->ae_settings.iris_fno = value;
}

void bsp_isp_s_focus_absolute(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->af_settings.focus_absolute != value)
		isp_gen->af_settings.focus_absolute = value;
}

void bsp_isp_s_focus_relative(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->af_settings.focus_relative != value)
		isp_gen->af_settings.focus_relative = value;
}

void bsp_isp_s_focus_auto(struct isp_lib_context *isp_gen, int value)
{
	if (value != AUTO_FOCUS_MANUAL)
	{
		isp_gen->af_settings.af_mode = AUTO_FOCUS_CONTINUEOUS;
		isp_gen->af_settings.focus_lock = false;
		isp_gen->isp_3a_change_flags |= ISP_SET_AF_METERING_MODE;
		clear(isp_gen->af_entity_ctx.af_param);
		isp_gen->af_entity_ctx.af_param.u.auto_focus_trigger = 1;
		isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_TRIGGER);
	}
	else
	{
		isp_gen->af_settings.af_mode = AUTO_FOCUS_MANUAL;
	}
}

void bsp_isp_s_auto_exposure_bias(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.exp_compensation != value)
		isp_gen->ae_settings.exp_compensation = value;
}

void bsp_isp_s_auto_n_preset_white_balance(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->awb_settings.wb_mode != value)
		isp_gen->awb_settings.wb_mode = value;
}

void bsp_isp_s_iso_sensitivity(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.iso_sensitivity != value)
		isp_gen->ae_settings.iso_sensitivity = value;
}

void bsp_isp_s_iso_sensitivity_auto(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.iso_mode != value)
		isp_gen->ae_settings.iso_mode = value;
}
void bsp_isp_s_scene_mode(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->ae_settings.scene_mode != value)
	{
		isp_gen->ae_settings.scene_mode= value;
		isp_gen->isp_3a_change_flags |= ISP_SET_SCENE_MODE;
	}

}
void bsp_isp_s_auto_focus_start(struct isp_lib_context *isp_gen, int value)
{
	isp_gen->af_settings.focus_lock = false;
	isp_gen->af_settings.af_mode = AUTO_FOCUS_TOUCH;

	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.auto_focus_trigger = 1;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_TRIGGER);
}

void bsp_isp_s_auto_focus_stop(struct isp_lib_context *isp_gen, int value)
{
	isp_gen->af_settings.focus_lock = false;
	isp_gen->af_settings.af_mode  = AUTO_FOCUS_CONTINUEOUS;
}

void bsp_isp_s_auto_focus_status(struct isp_lib_context *isp_gen, int value)
{

}

void bsp_isp_s_auto_focus_range(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->af_settings.af_range != value)
		isp_gen->af_settings.af_range = value;
}

void bsp_isp_s_take_picture(struct isp_lib_context *isp_gen, int value)
{
	ISP_PRINT("%s: %d.", __FUNCTION__, value);
	if (isp_gen->ae_settings.take_picture_flag != value) {
		isp_gen->ae_settings.take_picture_flag = value;
	}

	// Actruly we turn off in isp_flash_update_status, but set flag there for avoid torch ligth turn on again.
	if(isp_gen->ae_settings.take_picture_flag == V4L2_TAKE_PICTURE_STOP) {
		isp_gen->ae_settings.flash_open = 0;
		isp_hal_interaction_params.ith_params.image_para.bits.flash_ok = 0;
	}

}

void bsp_isp_s_r_gain(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->awb_settings.wb_gain_manual.r_gain != value)
		isp_gen->awb_settings.wb_gain_manual.r_gain = value;
}
void bsp_isp_s_gr_gain(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->awb_settings.wb_gain_manual.gr_gain != value)
		isp_gen->awb_settings.wb_gain_manual.gr_gain = value;
}

void bsp_isp_s_gb_gain(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->awb_settings.wb_gain_manual.gb_gain != value)
		isp_gen->awb_settings.wb_gain_manual.gb_gain = value;
}

void bsp_isp_s_b_gain(struct isp_lib_context *isp_gen, int value)
{
	if (isp_gen->awb_settings.wb_gain_manual.b_gain != value)
		isp_gen->awb_settings.wb_gain_manual.b_gain = value;
}

/*******************get   isp version*********************/
HW_S32 isp_get_version(char* version)
{
	sprintf(version, "%s", ISP_VERSION);

	ISP_PRINT("ISP Version: %s", ISP_VERSION);

	return 0;
}

