
/*
 ******************************************************************************
 *
 * isp_ini_parse.c
 *
 * Hawkview ISP - isp_ini_parse.c module
 *
 * Copyright (c) 2015 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2015/11/23	ISP Tuning Tools Support
 *
 ******************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "../iniparser/src/iniparser.h"
#include "../include/isp_ini_parse.h"
#include "../include/isp_manage.h"
#include "../include/isp_debug.h"
#include "../isp_dev/tools.h"

#include "SENSOR_H/imx290_default_ini_4v5.h"
#include "SENSOR_H/imx291_default_v5.h"
#include "SENSOR_H/isp_default_ini_4v5.h"
#include "SENSOR_H/ar0238_default_ini_4v5.h"
#include "SENSOR_H/imx317_default_ini_4v5.h"
#include "SENSOR_H/ov12895_default_ini_4v5.h"
#include "SENSOR_H/ov2718_wdr_ini_4v5.h"
#include "SENSOR_H/imx317_wdr_ini_4v5.h"
#include "SENSOR_H/imx274_wdr_ini_4v5.h"
#include "SENSOR_H/imx274_default_ini_4v5.h"
#include "SENSOR_H/ov2775_mipi.h"
#include "SENSOR_H/gc2355_mipi.h"
#include "SENSOR_H/gc5024_mipi.h"
#include "SENSOR_H/imx179_t7_2288.h"
#include "SENSOR_H/gc1034_720p_r311.h"
#include "SENSOR_H/s5k5e9_720p_15fps_r311.h"
#include "SENSOR_H/ov7251_mipi_gray.h"
#include "SENSOR_H/ov7251_mipi_10fps_gray.h"
#include "SENSOR_H/ov5658.h"
#include "SENSOR_H/gc5025_mipi.h"
#include "SENSOR_H/sc031gs_mipi_gray.h"
#include "SENSOR_H/gc0403_mipi.h"
#include "SENSOR_H/ov7750_mipi.h"
#include "SENSOR_H/ov2718_mipi.h"
#include "SENSOR_H/jxh62_mipi.h"
#include "SENSOR_H/gc5035_mipi.h"

unsigned int isp_cfg_log_param = ISP_LOG_CFG;

#define ISP_LIB_USE_INIPARSER 1

#if ISP_LIB_USE_INIPARSER

#define SIZE_OF_LSC_TBL     (12*768*2)
#define SIZE_OF_HDR_TBL     (4*256*2)
#define SIZE_OF_GAMMA_TBL   (256*2)

#define SET_ISP_SINGLE_VALUE(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg,\
			       void *value,\
			       int len)\
{\
	isp_ini_cfg->struct_name.key = *(int *)value;\
}

#define SET_ISP_ARRAY_VALUE(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg,\
			       void *value,\
			       int len)\
{\
	int i, *tmp;\
	tmp = (int *)value;\
	for (i = 0; i < len; i++) { \
		isp_ini_cfg->struct_name.key[i] = tmp[i];\
	} \
}

#define SET_ISP_DYNAMIC_VALUE(struct_name, idx) \
void set_##struct_name##_##idx(struct isp_param_config *isp_ini_cfg,\
			       void *value,\
			       int len)\
{\
	int i, *tmp, *cfg_pt;\
	tmp = (int *)value;\
	cfg_pt = &isp_ini_cfg->isp_iso_settings.struct_name[idx].sharp_cfg[0];\
	for (i = 0; i < len; i++) { \
		cfg_pt[i] = tmp[i];\
	} \
}

#define SET_ISP_CM_VALUE(num) \
void set_color_matrix##num(struct isp_param_config *isp_ini_cfg,\
			       void *value,\
			       int len)\
{\
	int *tmp = (int *)value;\
	struct isp_rgb2rgb_gain_offset *color_matrix = \
	&isp_ini_cfg->isp_tunning_settings.color_matrix_ini[num];\
	color_matrix->matrix[0][0] = tmp[0];\
	color_matrix->matrix[0][1] = tmp[1];\
	color_matrix->matrix[0][2] = tmp[2];\
	color_matrix->matrix[1][0] = tmp[3];\
	color_matrix->matrix[1][1] = tmp[4];\
	color_matrix->matrix[1][2] = tmp[5];\
	color_matrix->matrix[2][0] = tmp[6];\
	color_matrix->matrix[2][1] = tmp[7];\
	color_matrix->matrix[2][2] = tmp[8];\
	color_matrix->offset[0] = tmp[9];\
	color_matrix->offset[1] = tmp[10];\
	color_matrix->offset[2] = tmp[11];\
}

#define ISP_FILE_SINGLE_ATTR(main_key, sub_key)\
{\
	#main_key,  #sub_key, 1 ,  set_##sub_key,\
}

#define ISP_FILE_ARRAY_ATTR(main_key, sub_key , len)\
{\
	#main_key, #sub_key, len ,  set_##sub_key,\
}

#define ISP_FILE_DYNAMIC_ATTR(main_key, sub_key , len)\
{\
	#main_key, #sub_key, len ,  set_##main_key,\
}

#define ISP_FILE_CM_ATTR(prefix, main_key, sub_key)\
{\
	#prefix#main_key, #sub_key, 12 ,  set_##main_key,\
}

SET_ISP_SINGLE_VALUE(isp_test_settings, isp_test_mode);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_test_exptime);
SET_ISP_SINGLE_VALUE(isp_test_settings, exp_line_start);
SET_ISP_SINGLE_VALUE(isp_test_settings, exp_line_step);
SET_ISP_SINGLE_VALUE(isp_test_settings, exp_line_end);
SET_ISP_SINGLE_VALUE(isp_test_settings, exp_change_interval);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_test_gain);
SET_ISP_SINGLE_VALUE(isp_test_settings, gain_start);
SET_ISP_SINGLE_VALUE(isp_test_settings, gain_step);
SET_ISP_SINGLE_VALUE(isp_test_settings, gain_end);
SET_ISP_SINGLE_VALUE(isp_test_settings, gain_change_interval);

SET_ISP_SINGLE_VALUE(isp_test_settings, isp_test_focus);
SET_ISP_SINGLE_VALUE(isp_test_settings, focus_start);
SET_ISP_SINGLE_VALUE(isp_test_settings, focus_step);
SET_ISP_SINGLE_VALUE(isp_test_settings, focus_end);
SET_ISP_SINGLE_VALUE(isp_test_settings, focus_change_interval);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_log_param);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_gain);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_exp_line);
SET_ISP_SINGLE_VALUE(isp_test_settings, isp_color_temp);
SET_ISP_SINGLE_VALUE(isp_test_settings, ae_forced);
SET_ISP_SINGLE_VALUE(isp_test_settings, lum_forced);


SET_ISP_SINGLE_VALUE(isp_test_settings, manual_en		);
SET_ISP_SINGLE_VALUE(isp_test_settings, afs_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, sharp_en 	    );
SET_ISP_SINGLE_VALUE(isp_test_settings, contrast_en     );
SET_ISP_SINGLE_VALUE(isp_test_settings, denoise_en 	    );
SET_ISP_SINGLE_VALUE(isp_test_settings, drc_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, cem_en	    );
SET_ISP_SINGLE_VALUE(isp_test_settings, lsc_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, gamma_en	    );
SET_ISP_SINGLE_VALUE(isp_test_settings, cm_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, ae_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, af_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, awb_en		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, hist_en		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, blc_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, so_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, wb_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, otf_dpc_en 	    );
SET_ISP_SINGLE_VALUE(isp_test_settings, cfa_en		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, tdf_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, cnr_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, satur_en 	    );
SET_ISP_SINGLE_VALUE(isp_test_settings, defog_en 	    );
SET_ISP_SINGLE_VALUE(isp_test_settings, linear_en 	    );
SET_ISP_SINGLE_VALUE(isp_test_settings, gtm_en		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, dig_gain_en     );
SET_ISP_SINGLE_VALUE(isp_test_settings, pltm_en   );
SET_ISP_SINGLE_VALUE(isp_test_settings, wdr_en);
SET_ISP_SINGLE_VALUE(isp_test_settings, ctc_en);


SET_ISP_SINGLE_VALUE(isp_3a_settings, define_ae_table);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_table_preview_length);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_table_capture_length);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_table_video_length);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_max_lv);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_gain_favor);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_hist_mod_en);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_hist_sel);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_stat_sel);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_ki);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_ConvDataIndex);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_blowout_pre_en);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_blowout_attr);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_delay_frame);
SET_ISP_SINGLE_VALUE(isp_3a_settings, exp_delay_frame);
SET_ISP_SINGLE_VALUE(isp_3a_settings, gain_delay_frame);
SET_ISP_SINGLE_VALUE(isp_3a_settings, exp_comp_step);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_touch_dist_ind);
SET_ISP_SINGLE_VALUE(isp_3a_settings, ae_iso2gain_ratio);

SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_interval);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_speed);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_stat_sel);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_color_temper_low);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_color_temper_high);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_base_temper);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_green_zone_dist);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_blue_sky_dist);

SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_rgain_favor);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_bgain_favor);

SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_light_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_ext_light_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_skin_color_num);
SET_ISP_SINGLE_VALUE(isp_3a_settings, awb_special_color_num);

SET_ISP_SINGLE_VALUE(isp_3a_settings, af_use_otp);
SET_ISP_SINGLE_VALUE(isp_3a_settings, vcm_min_code);
SET_ISP_SINGLE_VALUE(isp_3a_settings, vcm_max_code);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_interval_time);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_speed_ind);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_auto_fine_en);

SET_ISP_SINGLE_VALUE(isp_3a_settings, af_single_fine_en);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_fine_step);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_move_cnt);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_still_cnt);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_move_monitor_cnt);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_still_monitor_cnt);

SET_ISP_SINGLE_VALUE(isp_3a_settings, af_stable_min);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_stable_max);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_low_light_lv);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_near_tolerance);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_far_tolerance);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_tolerance_off);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_peak_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_dir_th);

SET_ISP_SINGLE_VALUE(isp_3a_settings, af_change_ratio);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_move_minus);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_still_minus);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_scene_motion_th);
SET_ISP_SINGLE_VALUE(isp_3a_settings, af_tolerance_tbl_len);

SET_ISP_SINGLE_VALUE(isp_tunning_settings, flash_gain);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, flash_delay_frame);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, flicker_type);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, flicker_ratio);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, cfa_dir_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_th_max);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_th_min);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_th_slope);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_dir_wt);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ctc_dir_th);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, hor_visual_angle);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ver_visual_angle);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, focus_length);

SET_ISP_SINGLE_VALUE(isp_tunning_settings, lsc_center_x);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, lsc_center_y);


SET_ISP_SINGLE_VALUE(isp_tunning_settings, gtm_type);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, auto_alpha_en);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, gamma_type);
SET_ISP_SINGLE_VALUE(isp_tunning_settings, ff_mod);


SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_light_info);
SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_ext_light_info);
SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_skin_color_info);
SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_special_color_info);
SET_ISP_ARRAY_VALUE(isp_3a_settings, awb_preset_gain);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_fno_step);
SET_ISP_ARRAY_VALUE(isp_3a_settings, wdr_cfg);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_table_preview);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_table_capture);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_table_video);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_win_weight);
SET_ISP_ARRAY_VALUE(isp_3a_settings, ae_gain_range);
SET_ISP_ARRAY_VALUE(isp_3a_settings, af_std_code_tbl);
SET_ISP_ARRAY_VALUE(isp_3a_settings, af_tolerance_value_tbl);

SET_ISP_ARRAY_VALUE(isp_tunning_settings, bayer_gain);

SET_ISP_ARRAY_VALUE(isp_tunning_settings, cm_trig_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_bdnf_th);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_th);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_ref_noise);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_k);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_contrast_val);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_contrast_lum);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_sharp_val);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_sharp_lum);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_tdnf_diff);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, isp_contrat_pe);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, pltm_cfg);

SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 0);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 1);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 2);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 3);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 4);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 5);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 6);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 7);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 8);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 9);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 10);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 11);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 12);
SET_ISP_DYNAMIC_VALUE(isp_dynamic_cfg, 13);

SET_ISP_CM_VALUE(0);
SET_ISP_CM_VALUE(1);
SET_ISP_CM_VALUE(2);

struct IspParamAttribute {
	char *main;
	char *sub;
	int len;
	void (*set_param) (struct isp_param_config *, void *, int len);
};

struct FileAttribute {
	char *file_name;
	int param_len;
	struct IspParamAttribute *pIspParam;
};

static struct IspParamAttribute IspTestParam[] = {

	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_test_mode),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_test_exptime),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, exp_line_start),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, exp_line_step),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, exp_line_end),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, exp_change_interval),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_test_gain),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, gain_start),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, gain_step),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, gain_end),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, gain_change_interval),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_test_focus),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, focus_start),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, focus_step),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, focus_end),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, focus_change_interval),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_log_param),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_gain),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_exp_line),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, isp_color_temp),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, ae_forced),
	ISP_FILE_SINGLE_ATTR(isp_test_cfg, lum_forced),

	ISP_FILE_SINGLE_ATTR(isp_en_cfg, manual_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, afs_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, sharp_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, contrast_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, denoise_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, drc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, cem_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, lsc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, gamma_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, cm_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, ae_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, af_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, awb_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, hist_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, blc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, so_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, wb_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, otf_dpc_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, cfa_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, tdf_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, cnr_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, satur_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, defog_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, linear_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, gtm_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, dig_gain_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, pltm_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, wdr_en),
	ISP_FILE_SINGLE_ATTR(isp_en_cfg, ctc_en),
};

static struct IspParamAttribute Isp3aParam[] = {
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, define_ae_table),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_max_lv),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_table_preview_length),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_table_capture_length),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_table_video_length),

	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_fno_step, 16),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_fno_step, 3),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_table_preview, 42),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_table_capture, 42),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_table_video, 42),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_win_weight, 64),
	ISP_FILE_ARRAY_ATTR(isp_ae_cfg, ae_gain_range, 4),

	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_hist_mod_en),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_hist_sel),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_stat_sel),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_ki),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_ConvDataIndex),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_blowout_pre_en),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_blowout_attr),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_delay_frame),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, exp_delay_frame),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, gain_delay_frame),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, exp_comp_step),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_touch_dist_ind),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_iso2gain_ratio),
	ISP_FILE_SINGLE_ATTR(isp_ae_cfg, ae_gain_favor),

	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_interval),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_speed),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_stat_sel),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_color_temper_low),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_color_temper_high),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_base_temper),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_green_zone_dist),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_blue_sky_dist),

	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_light_num),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_ext_light_num),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_skin_color_num),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_special_color_num),

	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_rgain_favor),
	ISP_FILE_SINGLE_ATTR(isp_awb_cfg, awb_bgain_favor),
	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_light_info, 320),
	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_ext_light_info, 320),
	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_skin_color_info, 160),
	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_special_color_info, 320),
	ISP_FILE_ARRAY_ATTR(isp_awb_cfg, awb_preset_gain, 22),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_use_otp),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, vcm_min_code),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, vcm_max_code),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_interval_time),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_speed_ind),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_auto_fine_en),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_single_fine_en),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_fine_step),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_move_cnt),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_still_cnt),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_move_monitor_cnt),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_still_monitor_cnt),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_stable_min),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_stable_max),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_low_light_lv),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_near_tolerance),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_far_tolerance),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_tolerance_off),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_peak_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_dir_th),

	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_change_ratio),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_move_minus),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_still_minus),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_scene_motion_th),
	ISP_FILE_SINGLE_ATTR(isp_af_cfg, af_tolerance_tbl_len),

	ISP_FILE_ARRAY_ATTR(isp_af_cfg, af_std_code_tbl, 20),
	ISP_FILE_ARRAY_ATTR(isp_af_cfg, af_tolerance_value_tbl, 20),

};
static struct IspParamAttribute IspDynamicParam[] = {
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_0, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_1, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_2, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_3, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_4, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_5, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_6, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_7, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_8, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_9, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_10, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_11, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_12, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_DYNAMIC_ATTR(isp_dynamic_cfg_13, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
};


static struct IspParamAttribute IspTuningParam[] = {
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flash_gain),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flash_delay_frame),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, cfa_dir_th),

	ISP_FILE_SINGLE_ATTR(isp_ctc_cfg, ctc_th_max),
	ISP_FILE_SINGLE_ATTR(isp_ctc_cfg, ctc_th_min),
	ISP_FILE_SINGLE_ATTR(isp_ctc_cfg, ctc_th_slope),
	ISP_FILE_SINGLE_ATTR(isp_ctc_cfg, ctc_dir_wt),
	ISP_FILE_SINGLE_ATTR(isp_ctc_cfg, ctc_dir_th),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flicker_type),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flicker_ratio),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, hor_visual_angle),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ver_visual_angle),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, focus_length),
	ISP_FILE_SINGLE_ATTR(isp_lsc, lsc_center_x),
	ISP_FILE_SINGLE_ATTR(isp_lsc, lsc_center_y),

	ISP_FILE_ARRAY_ATTR(isp_gain_offset, bayer_gain, 4),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_type),//
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, auto_alpha_en),//
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gamma_type),//
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ff_mod),//

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, cm_trig_cfg, 3),//
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, pltm_cfg, ISP_PLTM_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_bdnf_th, ISP_REG_TBL_LENGTH),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_th, ISP_REG_TBL_LENGTH),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_ref_noise, ISP_REG_TBL_LENGTH),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_k, ISP_REG_TBL_LENGTH - 1),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_contrast_val, ISP_REG_TBL_LENGTH),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_contrast_lum, ISP_REG_TBL_LENGTH),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_sharp_val, ISP_REG_TBL_LENGTH),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_sharp_lum, ISP_REG_TBL_LENGTH),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_tdnf_diff, 256),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, isp_contrat_pe, 128),

	ISP_FILE_CM_ATTR(isp_, color_matrix0, matrix),
	ISP_FILE_CM_ATTR(isp_, color_matrix1, matrix),
	ISP_FILE_CM_ATTR(isp_, color_matrix2, matrix),

};

static struct FileAttribute FileAttr[] = {
	{"isp_test_param.ini", array_size(IspTestParam), &IspTestParam[0],},
	{"isp_3a_param.ini", array_size(Isp3aParam), &Isp3aParam[0],},
	{"isp_dynamic_param.ini", array_size(IspDynamicParam), &IspDynamicParam[0],},
	{"isp_tuning_param.ini", array_size(IspTuningParam), &IspTuningParam[0],},

};

#define MAGIC_NUM 0x64387483

int cfg_get_int(dictionary *ini, char *main, char *sub, int *val)
{
	char key[128] = { 0 };
	int ret;

	strcpy(key, main);
	strcat(key,":");
	strcat(key,sub);
	ret = iniparser_getint(ini, key, MAGIC_NUM);
	if(ret == MAGIC_NUM) {
		*val = 0;
		return -1;
	}
	*val = ret;
	return 0;
}

int cfg_get_double(dictionary *ini, char *main, char *sub, double *val)
{
	char key[128] = { 0 };
	double ret;

	strcpy(key, main);
	strcat(key, ":");
	strcat(key, sub);
	ret = iniparser_getdouble(ini, key, MAGIC_NUM/1.0);
	if((ret <= MAGIC_NUM/1.0 + 1.0) || (ret >= MAGIC_NUM/1.0 - 1.0)) {
		*val = 0.0;
		return -1;
	}
	*val = ret;
	//printf("MAGIC_NUM/1.0 = %f, MAGIC_NUM = %d\n", MAGIC_NUM/1.0, MAGIC_NUM);
	return 0;
}

int cfg_get_int_arr(dictionary *ini, char *main, char *sub, int *arr, int len)
{
	char key[128] = { 0 };
	int ret;

	strcpy(key, main);
	strcat(key, ":");
	strcat(key, sub);
	ret = iniparser_get_int_array(ini, key, arr, len, MAGIC_NUM);
	if(ret == MAGIC_NUM) {
		return -1;
	}
	return ret;
}

int cfg_get_double_arr(dictionary *ini, char *main, char *sub, double *arr, int len)
{
	char key[128] = { 0 };
	int ret;

	strcpy(key, main);
	strcat(key, ":");
	strcat(key, sub);
	ret = iniparser_get_double_array(ini, key, arr, len, MAGIC_NUM);
	if(ret == MAGIC_NUM) {
		return -1;
	}
	return ret;
}

int isp_read_file(char *file_path, char *buf, size_t len)
{
	FILE *fp;
	size_t buf_len;
	struct stat s;
	if (stat(file_path,&s)) {
		ISP_ERR("stat error!\n");
		return -1;
	}

	fp = fopen(file_path, "r");
	if (!fp) {
		ISP_ERR("open file failed!\n");
		return -1;
	}

	buf_len = fread(buf, 1, s.st_size, fp);
	ISP_CFG_LOG(ISP_LOG_CFG, "buf_len = %Zu, expect len = %lld\n", buf_len, s.st_size);
	fclose(fp);

	if (buf_len <= 0)
		return -1;

	return buf_len;
}

int isp_parser_cfg(struct isp_param_config *isp_ini_cfg,
		  dictionary *ini, struct FileAttribute *file_attr)
{
	int i, j, *array_value, val = 0;
	struct IspParamAttribute *param;
	char sub_name[128] = { 0 };

	/* fetch ISP config! */
	for (i = 0; i < file_attr->param_len; i++) {
		param = file_attr->pIspParam + i;
		if (param->main == NULL || param->sub == NULL) {
			ISP_WARN("param->main or param->sub is NULL!\n");
			continue;
		}
		if (param->len == 1) {
			if (-1 == cfg_get_int(ini, param->main, param->sub, &val)) {
				ISP_WARN("%s->%s,apply default value!\n", param->main, param->sub);
			} else {
				if (param->set_param) {
					param->set_param(isp_ini_cfg, (void *)&val, param->len);
					ISP_CFG_LOG(ISP_LOG_CFG, "fetch_isp_cfg_single: %s->%s = %d\n",
						param->main, param->sub, val);
				}
			}
		} else if (param->len > 1) {
			array_value = (int *)malloc(param->len * sizeof(int));
			memset(array_value, 0, param->len * sizeof(int));

			param->len = cfg_get_int_arr(ini, param->main, param->sub,
						array_value, param->len);
			if (-1 == param->len) {
				ISP_WARN("fetch %s from %s failed, set %s = 0!\n",
					     param->sub, param->main, param->sub);
			} else {
				ISP_PRINT("the real length of %s->%s is %d\n",
					param->main, param->sub, param->len);

				if (param->set_param)
					param->set_param(isp_ini_cfg, (void *)array_value, param->len);
			}

			if (array_value)
				free(array_value);
		}
	}
	ISP_PRINT("fetch isp_cfg done!\n");
	return 0;
}

int isp_parser_tbl(struct isp_param_config *isp_ini_cfg, char *tbl_patch)
{
	int len, ret = 0;
	char isp_gamma_tbl_path[128] = "\0", isp_hdr_tbl_path[128] = "\0";
	char isp_lsc_tbl_path[128] = "\0", isp_cem_tbl_path[128] = "\0";
	char isp_pltm_tbl_path[128] = "\0", isp_wdr_tbl_path[128] = "\0";
	char *buf;

	strcpy(isp_gamma_tbl_path, tbl_patch);
	strcpy(isp_hdr_tbl_path, tbl_patch);
	strcpy(isp_lsc_tbl_path, tbl_patch);
	strcpy(isp_cem_tbl_path, tbl_patch);
	strcpy(isp_pltm_tbl_path, tbl_patch);
	strcpy(isp_wdr_tbl_path, tbl_patch);

	strcat(isp_gamma_tbl_path, "gamma_tbl.bin");
	strcat(isp_hdr_tbl_path, "hdr_tbl.bin");
	strcat(isp_lsc_tbl_path, "lsc_tbl.bin");
	strcat(isp_cem_tbl_path, "cem_tbl.bin");
	strcat(isp_pltm_tbl_path, "pltm_tbl.bin");
	strcat(isp_wdr_tbl_path, "wdr_tbl.bin");

	ISP_PRINT("Fetch table form \"%s\"\n", isp_gamma_tbl_path);
	buf = (char *)malloc(SIZE_OF_LSC_TBL);
	memset(buf, 0, SIZE_OF_LSC_TBL);

	/* fetch gamma_tbl table!
	len = isp_read_file(isp_gamma_tbl_path, buf, ISP_GAMMA_MEM_SIZE * 5);
	if (len < 0) {
		ISP_WARN("read gamma_tbl from gamma_tbl.bin failed!\n");
		ret = -1;
	} else {
		memcpy(isp_ini_cfg->isp_tunning_settings.gamma_tbl_ini, buf, len);
	}
	*/

	/* fetch lsc table! */
	len = isp_read_file(isp_lsc_tbl_path, buf, SIZE_OF_LSC_TBL);
	if (len != SIZE_OF_LSC_TBL)
		ISP_WARN("read lsc_tbl from lsc_tbl.bin len = %d,"
				" but %d is required\n", len, SIZE_OF_LSC_TBL);
	if (len < 0) {
		ISP_WARN("read lsc_tbl from lsc_tbl.bin failed!\n");
		ret = -1;
	} else {
		memcpy(isp_ini_cfg->isp_tunning_settings.lsc_tbl, buf, len);
	}

	/* fetch cem table! */
	len = isp_read_file(isp_cem_tbl_path, buf, ISP_CEM_MEM_SIZE);
	if (len != ISP_CEM_MEM_SIZE)
		ISP_WARN("read cem_tbl from cem_tbl.bin len = %d,"
				" but %d is required\n", len, ISP_CEM_MEM_SIZE);
	if (len < 0) {
		ISP_WARN("read cem_tbl from cem_tbl.bin failed!\n");
		//ret = -1;
	} else {
		memcpy(isp_ini_cfg->isp_tunning_settings.isp_cem_table, buf, len);
	}

	/* fetch pltm table! */
	len = isp_read_file(isp_pltm_tbl_path, buf, ISP_PLTM_MEM_SIZE);
	if (len != ISP_PLTM_MEM_SIZE)
		ISP_WARN("read pltm_table from pltm_table.bin len = %d,"
				" but %d is required\n", len, ISP_PLTM_MEM_SIZE);
	if (len < 0) {
		ISP_WARN("read pltm_table from pltm_table.bin failed!\n");
		//ret = -1;
	} else {
		memcpy(isp_ini_cfg->isp_tunning_settings.isp_pltm_table, buf, len);
	}

	/* fetch wdr table! */
	len = isp_read_file(isp_wdr_tbl_path, buf, ISP_WDR_MEM_SIZE);
	if (len != ISP_WDR_MEM_SIZE)
		ISP_WARN("read wdr_table from wdr_table.bin len = %d,"
				" but %d is required\n", len, ISP_WDR_MEM_SIZE);
	if (len < 0) {
		ISP_WARN("read wdr_table from wdr_table.bin failed!\n");
		//ret = -1;
	} else {
		memcpy(isp_ini_cfg->isp_tunning_settings.isp_wdr_table, buf, len);
	}

	if (buf)
		free(buf);
	return ret;
}
#endif

struct isp_cfg_array cfg_arr[] = {
	{"imx317_mipi", "imx317_default_ini_4v5", 3840, 2160, 30, 0, 0, &imx317_default_ini_4v5},
	{"imx317_mipi", "imx317_default_ini_4v5", 1920, 1080, 30, 0, 0, &imx317_default_ini_4v5},
	{"imx317_mipi", "imx317_wdr_ini_4v5", 1920, 1080, 30, 1, 0, &imx317_wdr_ini_4v5},
	{"imx274_slvds", "imx274_default_ini_4v5", 1920, 1080, 60, 0, 0, &imx274_default_ini_4v5},
	{"imx274_slvds", "imx274_wdr_ini_4v5", 1920, 1080, 60, 1, 0, &imx274_wdr_ini_4v5},
	{"ov12895_mipi", "ov12895_default_ini_4v5", 1920, 1080, 30, 0, 0, &ov12895_default_ini_4v5},
	{"imx290_mipi", "imx290_default_ini_4v5", 1920, 1080, 30, 0, 0, &imx290_default_ini_4v5},
	{"imx291_mipi", "imx291_mipi_isp_cfg", 1920, 1080, 30, 0, 0, &imx291_mipi_isp_cfg},
	{"ar0238", "ar0238_default_ini_4v5", 1920, 1080, 30, 0, 0, &ar0238_default_ini_4v5},
	{"ov2718_mipi", "ov2718_wdr_ini_4v5", 1920, 1080, 30, 1, 0, &ov2718_wdr_ini_4v5},
	{"ov2775_mipi", "ov2775_mipi_isp_cfg", 1920, 1080, 30, 1, 0, &ov2775_mipi_isp_cfg},
	{"gc2355_mipi", "gc2355_mipi_isp_cfg", 1600, 1200, 30, 0, 0, &gc2355_mipi_isp_cfg},
	{"gc5024_mipi", "gc5024_mipi_isp_cfg", 2592, 1936, 30, 0, 0, &gc5024_mipi_isp_cfg},
	{"imx179_mipi", "imx179_t7_2288", 2288, 2288, 30, 0, 0, &imx179_mipi_isp_cfg},
	{"imx179_mipi", "imx179_t7_2288", 1920, 1080, 30, 0, 0, &imx179_mipi_isp_cfg},
	{"imx179_mipi", "imx179_t7_2288", 1280, 1080, 30, 0, 0, &imx179_mipi_isp_cfg},
	{"gc1034_mipi", "gc1034_720p_r311", 1280, 720, 30, 0, 0, &gc1034_mipi_isp_cfg},
	{"s5k5e9", "s5k5e9_720p_r311", 1280, 720, 15, 0, 0, &s5k5e9_isp_cfg},
	{"ov7251_mipi", "ov7251_mipi_gray_mr133", 640, 480, 30, 0, 0, &ov7251_mipi_isp_cfg},
	{"ov7251_mipi", "ov7251_mipi_10fps_gray_mr133", 640, 480, 10, 0, 0, &ov7251_mipi_10fps_isp_cfg},
	{"ov5658", "ov5658_mr133", 1920, 1600, 30, 0, 0, &ov5658_isp_cfg},
	{"gc5025_mipi", "gc5025_mipi_isp_cfg_r311", 2592, 1936, 30, 0, 0, &gc5025_mipi_isp_cfg},
	{"sc031gs_mipi", "sc031gs_mipi_isp_cfg", 640, 480, 30, 0, 0, &sc031gs_mipi_isp_cfg},
	{"gc0403_mipi", "gc0403_mipi_isp_cfg", 768, 576, 30, 0, 0, &gc0403_mipi_isp_cfg},
	{"ov7750_mipi", "ov7750_mipi_isp_cfg", 640, 480, 30, 0, 0, &ov7750_mipi_isp_cfg},
	{"ov2718_mipi", "ov2718_mipi_isp_cfg", 1920, 1080, 30, 0, 0, &ov2718_mipi_isp_cfg},
	{"jxh62_mipi", "jxh62_mipi_isp_cfg", 1280, 720, 25, 0, 0, &jxh62_mipi_isp_cfg},
	{"gc5035_mipi", "gc5035_mipi_isp_cfg", 2592, 1936, 10, 0, 0, &gc5035_mipi_isp_cfg},
};
int temp_array_1[4] = {2800, 4000, 5000, 6500};
int temp_array_2[6] = {2200, 2800, 4000, 5000, 5500, 6500};

int find_nearest_index(int mod, int temp)
{
	int index = 0;
	int i= 0;
	int min_dist = 1 << 30, tmp_dist;
	if(mod == 1) {
		for(i = 0; i < 4; i++) {
			tmp_dist = temp_array_1[i] - temp;
			tmp_dist = (tmp_dist < 0) ? -tmp_dist : tmp_dist;
			if(tmp_dist < min_dist) {
				min_dist = tmp_dist;
				index = i;
			}
			ISP_CFG_LOG(ISP_LOG_CFG, "mode: %d, tmp_dist: %d, min_dist: %d, index: %d.\n", mod, tmp_dist, min_dist, index);
		}
	} else if (mod == 2) {
		for(i = 0; i < 6; i++) {
			tmp_dist = temp_array_2[i] - temp;
			tmp_dist = (tmp_dist < 0) ? -tmp_dist : tmp_dist;
			if(tmp_dist < min_dist) {
				min_dist = tmp_dist;
				index = i;
			}
			ISP_CFG_LOG(ISP_LOG_CFG, "mode: %d, tmp_dist: %d, min_dist: %d, index: %d.\n", mod, tmp_dist, min_dist, index);
		}
	} else
		ISP_ERR("find_nearest_index mod error.\n");
	ISP_CFG_LOG(ISP_LOG_CFG, "nearest temp index: %d.\n", index);
	return index;
}
int parser_sync_info(struct isp_param_config *param, char *isp_cfg_name, int isp_id)
{
	FILE *file_fd = NULL;
	char fdstr[50];
	int bayer_gain_result[775];
	int lsc_ind = 0, lsc_cnt = 0, lsc_tab_cnt = 0;
	int version_num = 0;
	int lsc_temp = 0;
	int enable = 0;
	sprintf(fdstr, "/mnt/extsd/%s_%d.bin", isp_cfg_name, isp_id);
	file_fd = fopen(fdstr, "rb");

	if (!file_fd) {
		ISP_ERR("parser_sync_info: open bin failed.\n");
		return -1;
	} else {
		fread(&bayer_gain_result[0], sizeof(int)*775, 1, file_fd);
	}
	fclose(file_fd);

	version_num = bayer_gain_result[0];
	enable = bayer_gain_result[1];

	if (0 == enable) {
		ISP_CFG_LOG(ISP_LOG_CFG, "parser_sync_info enable mode = 0.\n");
		return 0;
	} else if (1 == enable) {
		ISP_CFG_LOG(ISP_LOG_CFG, "parser_sync_info enable mode = 1.\n");
		memcpy(param->isp_tunning_settings.bayer_gain, &bayer_gain_result[2], sizeof(int)*ISP_RAW_CH_MAX);
		return 0;
	} else if (2 == enable) {
		ISP_CFG_LOG(ISP_LOG_CFG, "parser_sync_info enable mode = 2.\n");
		goto lsc_tbl;
	} else
		ISP_CFG_LOG(ISP_LOG_CFG, "parser_sync_info enable mode = 3.\n");
	memcpy(param->isp_tunning_settings.bayer_gain, &bayer_gain_result[2], sizeof(int)*ISP_RAW_CH_MAX);

	ISP_CFG_LOG(ISP_LOG_CFG, "bayer_gain: %d, %d, %d, %d.\n", param->isp_tunning_settings.bayer_gain[0], param->isp_tunning_settings.bayer_gain[1],
	param->isp_tunning_settings.bayer_gain[2], param->isp_tunning_settings.bayer_gain[3]);
lsc_tbl:
	lsc_temp = bayer_gain_result[6];
	lsc_ind = find_nearest_index(param->isp_tunning_settings.ff_mod, lsc_temp);
	ISP_CFG_LOG(ISP_LOG_CFG, "lsc_ind: %d.\n", lsc_ind);
	if(param->isp_tunning_settings.ff_mod == 1) {
		for(lsc_tab_cnt = 0; lsc_tab_cnt < 4; lsc_tab_cnt++) {
			if(lsc_tab_cnt == lsc_ind)
				continue;
			for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++) {
				param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]
					= bayer_gain_result[7+lsc_cnt]*param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]/param->isp_tunning_settings.lsc_tbl[lsc_ind][lsc_cnt];
			}
		}

		for(lsc_tab_cnt = 4; lsc_tab_cnt < 8; lsc_tab_cnt++) {
			if(lsc_tab_cnt == (lsc_ind+4))
				continue;
			for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++) {
				param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]
					= bayer_gain_result[7+lsc_cnt]*param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]/param->isp_tunning_settings.lsc_tbl[lsc_ind+4][lsc_cnt];
			}
		}
		for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++)
			param->isp_tunning_settings.lsc_tbl[lsc_ind][lsc_cnt] = param->isp_tunning_settings.lsc_tbl[lsc_ind+4][lsc_cnt]
			= bayer_gain_result[7+lsc_cnt];

		ISP_CFG_LOG(ISP_LOG_CFG, "lsc_tbl_1 0: %d, 1: %d, 766: %d, 767: %d.\n", param->isp_tunning_settings.lsc_tbl[1][0], param->isp_tunning_settings.lsc_tbl[1][1],
			param->isp_tunning_settings.lsc_tbl[1][766], param->isp_tunning_settings.lsc_tbl[1][767]);
	} else if(param->isp_tunning_settings.ff_mod == 2) {
		for(lsc_tab_cnt = 0; lsc_tab_cnt < 6; lsc_tab_cnt++) {
			if(lsc_tab_cnt == lsc_ind)
				continue;
			for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++) {
				if(param->isp_tunning_settings.lsc_tbl[lsc_ind][lsc_cnt] == 0) {
					ISP_ERR("lsc_ind: %d, lsc_cnt: %d is zero.\n", lsc_ind, lsc_cnt);
					continue;
				} else
					param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]
						= bayer_gain_result[7+lsc_cnt]*param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]/param->isp_tunning_settings.lsc_tbl[lsc_ind][lsc_cnt];
				if(param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt] == 0) {
					ISP_ERR("result------>lsc_ind: %d, lsc_cnt: %d is zero.\n", lsc_tab_cnt, lsc_cnt);
				}
			}
		}

		for(lsc_tab_cnt = 6; lsc_tab_cnt < 12; lsc_tab_cnt++) {
			if(lsc_tab_cnt == (lsc_ind+6))
				continue;
			for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++) {
				if(param->isp_tunning_settings.lsc_tbl[lsc_ind + 7][lsc_cnt] == 0) {
					ISP_ERR("lsc_ind: %d, lsc_cnt: %d is zero.\n", lsc_ind+6, lsc_cnt);
					continue;
				} else
					param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]
						= bayer_gain_result[7+lsc_cnt]*param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]/param->isp_tunning_settings.lsc_tbl[lsc_ind+6][lsc_cnt];
				if(param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt] == 0) {
					ISP_ERR("result------>lsc_ind: %d, lsc_cnt: %d is zero.\n", lsc_tab_cnt, lsc_cnt);
				}
			}
		}
		for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++)
			param->isp_tunning_settings.lsc_tbl[lsc_ind][lsc_cnt] = param->isp_tunning_settings.lsc_tbl[lsc_ind+6][lsc_cnt]
			= bayer_gain_result[7+lsc_cnt];

		ISP_CFG_LOG(ISP_LOG_CFG, "lsc_tbl_1 0: %d, 1: %d, 766: %d, 767: %d.\n", param->isp_tunning_settings.lsc_tbl[1][0], param->isp_tunning_settings.lsc_tbl[1][1],
			param->isp_tunning_settings.lsc_tbl[1][766], param->isp_tunning_settings.lsc_tbl[1][767]);
	} else {
		ISP_ERR("isp ff_mod error.\n");
	}
	return 0;
}
int parser_ini_info(struct isp_param_config *param, char *sensor_name,
			int w, int h, int fps, int wdr, int ir, int sync_mode, int isp_id)
{
	int i, ret = 0;
	char path[20] = "/mnt/extsd/";
	char isp_cfg_path[128], isp_tbl_path[128], file_name[128];
	dictionary * ini;

	struct isp_cfg_pt *cfg = &isp_default_ini_4v5;

	for (i = 0; i < array_size(cfg_arr); i++) {
		if (!strncmp(sensor_name, cfg_arr[i].sensor_name, 6) &&
		    (w == cfg_arr[i].width) && (h == cfg_arr[i].height) &&
		    (fps == cfg_arr[i].fps) && (wdr == cfg_arr[i].wdr)) {
			cfg = cfg_arr[i].cfg;
			ISP_PRINT("find %s_%d_%d_%d_%d isp config\n", cfg_arr[i].sensor_name,
				cfg_arr[i].width, cfg_arr[i].height, cfg_arr[i].fps, cfg_arr[i].wdr);
			break;
		}
	}

	if (i == array_size(cfg_arr)) {
		ISP_PRINT("cannot find %s_%d_%d_%d_%d isp config!!!\n",	sensor_name, w, h, fps, wdr);

		if (!strncmp(sensor_name, "imx317", 6)) {
			ISP_PRINT("use imx317 isp config.\n");
			cfg = &imx317_default_ini_4v5;
		} else if (!strncmp(sensor_name, "imx274", 6)) {
			ISP_PRINT("use imx274 isp config.\n");
			cfg = &imx274_default_ini_4v5;
		} else if (!strncmp(sensor_name, "ov12895", 7)) {
			ISP_PRINT("use ov12895 isp config.\n");
			cfg = &ov12895_default_ini_4v5;
		} else if (!strncmp(sensor_name, "imx290", 6)) {
			ISP_PRINT("use imx290 isp config.\n");
			cfg = &imx290_default_ini_4v5;
		} else if (!strncmp(sensor_name, "imx291", 6)) {
			ISP_PRINT("use imx291 isp config.\n");
			cfg = &imx291_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "ar0238", 6)) {
			ISP_PRINT("use ar0238 isp config.\n");
			cfg = &ar0238_default_ini_4v5;
		} else if (!strncmp(sensor_name, "ov2718", 6)) {
			ISP_PRINT("use ov2718 isp config.\n");
			cfg = &ov2718_wdr_ini_4v5;
		} else if (!strncmp(sensor_name, "ov2775", 6)) {
			ISP_PRINT("use ov2775 isp config.\n");
			cfg = &ov2775_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "gc2355", 6)) {
			ISP_PRINT("use gc2355 isp config.\n");
			cfg = &gc2355_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "gc5024_mipi", 11)) {
			ISP_PRINT("use gc5024_mipi isp config.\n");
			cfg = &gc5024_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "ov7251_mipi", 11)) {
			ISP_PRINT("use ov7251_mipi isp config.\n");
			cfg = &ov7251_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "sc031gs_mipi", 12)) {
			ISP_PRINT("use sc031gs_mipi isp config.\n");
			cfg = &sc031gs_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "gc0403_mipi", 11)) {
			ISP_PRINT("use gc0403_mipi isp config.\n");
			cfg = &gc0403_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "ov7750_mipi", 11)) {
			ISP_PRINT("use ov7750_mipi isp config.\n");
			cfg = &ov7750_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "ov2718_mipi", 11)) {
			ISP_PRINT("use ov2718_mipi isp config.\n");
			cfg = &ov2718_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "jxh62_mipi", 10)) {
			ISP_PRINT("use jxh62_mipi isp config.\n");
			cfg = &jxh62_mipi_isp_cfg;
		} else if (!strncmp(sensor_name, "gc5035_mipi", 11)) {
			ISP_PRINT("use gc5035_mipi isp config.\n");
			cfg = &gc5035_mipi_isp_cfg;
		} else {
			ISP_PRINT("use default isp config.\n");
		}
	}

	param->isp_test_settings = *cfg->isp_test_settings;
	param->isp_3a_settings = *cfg->isp_3a_settings;
	param->isp_iso_settings = *cfg->isp_iso_settings;
	param->isp_tunning_settings = *cfg->isp_tunning_settings;

	/******for a50 statistic select******/
	param->isp_3a_settings.ae_hist_sel = HIST_AFTER_CNR;
	param->isp_3a_settings.ae_stat_sel = AE_AFTER_CNR;
	param->isp_3a_settings.awb_stat_sel = 0;

	if(sync_mode && i < array_size(cfg_arr)) {
		parser_sync_info(param, cfg_arr[i].isp_cfg_name, isp_id);
	}
	return 0;

#if ISP_LIB_USE_INIPARSER
	if (strcmp(sensor_name, "")) {
		sprintf(isp_cfg_path, "%s%s/", path, sensor_name);
		sprintf(isp_tbl_path, "%s%s/bin/", path, sensor_name);
	} else {
		ISP_ERR("sensor cfg name is invalid\n");
		return -1;
	}

	sprintf(file_name, "%s%s", isp_cfg_path, FileAttr[0].file_name);

	ISP_PRINT("read ini start\n");

	for (i = 0; i < array_size(FileAttr); i++) {
		sprintf(file_name, "%s%s", isp_cfg_path, FileAttr[i].file_name);
		ISP_PRINT("Fetch ini file form \"%s\"\n", file_name);

		ini = iniparser_load(file_name);
		isp_parser_cfg(param, ini, &FileAttr[i]);
		iniparser_freedict(ini);
	}
	ret = isp_parser_tbl(param, &isp_tbl_path[0]);
	if (ret == -1) {

		ISP_ERR("ISP read ini error!\n");
		param->isp_test_settings = *cfg->isp_test_settings;
		param->isp_3a_settings = *cfg->isp_3a_settings;
		param->isp_iso_settings = *cfg->isp_iso_settings;
		param->isp_tunning_settings = *cfg->isp_tunning_settings;
	}

read_ini_info_end:
	ISP_PRINT("read ini end\n");
	return ret;
#endif
}


