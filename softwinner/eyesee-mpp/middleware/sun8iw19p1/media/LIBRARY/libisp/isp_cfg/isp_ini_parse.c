
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


#include "SENSOR_H/sc2232/sc2232_20fps_ini_ipc_default.h"
#include "SENSOR_H/imx307/imx307_30fps_ini_cdr.h"

#ifdef SENSOR_SC2232
#include "SENSOR_H/sc2232/sc2232_20fps_ini_ipc_day.h"
#include "SENSOR_H/sc2232/sc2232_20fps_ini_ipc_night.h"
//#include "SENSOR_H/sp2305/sp2305_20fps_ini_ipc_day.h"
//#include "SENSOR_H/sp2305/sp2305_20fps_ini_ipc_night.h"
#endif

#ifdef SENSOR_IMX335
#include "SENSOR_H/imx335/imx335_30fps_ini_cdr.h"
#endif

#ifdef SENSOR_GC2053
//#include "SENSOR_H/gc2053/gc2053_20fps_ini_cdr.h"
#include "SENSOR_H/gc2053/gc2053_30fps_ini_cdr.h"
#endif

#ifdef SENSOR_IMX317
#include "SENSOR_H/imx317/imx317_25fps_ini_cdr.h"
#include "SENSOR_H/imx317/imx317_full_ini_sdv.h"
#include "SENSOR_H/imx317/imx317_60fps_ini_sdv.h"
#include "SENSOR_H/imx317/imx317_120fps_ini_sdv.h"
#include "SENSOR_H/imx317/imx317_240fps_ini_sdv.h"
#include "SENSOR_H/imx317/imx317_30fps_ini_cdr.h"
#endif

#ifdef SENSOR_IMX278
#include "SENSOR_H/imx278/imx278_full_v5.h"
#include "SENSOR_H/imx278/imx278_4k_v5.h"
#include "SENSOR_H/imx278/imx278_60fps_v5.h"
#include "SENSOR_H/imx278/imx278_120fps_v5.h"
#endif

#ifdef SENSOR_IMX386
#include "SENSOR_H/imx386/imx386_1800p_v200.h"
#include "SENSOR_H/imx386/imx386_4k_v200.h"
#include "SENSOR_H/imx386/imx386_60fps_v200.h"
#include "SENSOR_H/imx386/imx386_120fps_v200.h"
#include "SENSOR_H/imx386/imx386_240fps_v200.h"
#include "SENSOR_H/imx386/imx386_full_v200.h"
#endif

#define ISP_TUNING_ENABLE 0


unsigned int isp_cfg_log_param = ISP_LOG_CFG;

#define SIZE_OF_LSC_TBL     (12*768*2)
#define SIZE_OF_GAMMA_TBL   (5*1024*3*2)

#if ISP_LIB_USE_INIPARSER

#define SET_ISP_SINGLE_VALUE(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg, void *value, int len) \
{\
	isp_ini_cfg->struct_name.key = *(int *)value;\
}

#define SET_ISP_ARRAY_VALUE(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg, void *value, int len) \
{\
	int i, *tmp = (int *)value;\
	for (i = 0; i < len; i++) \
		isp_ini_cfg->struct_name.key[i] = tmp[i];\
}

#define SET_ISP_ARRAY_INT(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg, void *value, int len)\
{\
	memcpy(&isp_ini_cfg->struct_name.key[0], value, 4*len);\
}

#define SET_ISP_STRUCT_INT(struct_name, key) \
void set_##key(struct isp_param_config *isp_ini_cfg, void *value, int len)\
{\
	memcpy(&isp_ini_cfg->struct_name.key, value, 4*len);\
}

#define SET_ISP_STRUCT_ARRAY_INT(struct_name, key, idx) \
void set_##key##_##idx(struct isp_param_config *isp_ini_cfg, void *value, int len)\
{\
	memcpy(&isp_ini_cfg->struct_name.key[idx], value, 4*len);\
}

#define SET_ISP_CM_VALUE(key, idx) \
void set_##key##idx(struct isp_param_config *isp_ini_cfg, void *value, int len)\
{\
	int *tmp = (int *)value;\
	struct isp_rgb2rgb_gain_offset *color_matrix = &isp_ini_cfg->isp_tunning_settings.color_matrix_ini[idx];\
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
	#main_key,  #sub_key, 1,  set_##sub_key,\
}

#define ISP_FILE_ARRAY_ATTR(main_key, sub_key , len)\
{\
	#main_key, #sub_key, len,  set_##sub_key,\
}

#define ISP_FILE_STRUCT_ARRAY_ATTR(main_key, sub_key , len)\
{\
	#main_key, #sub_key, len,  set_##main_key,\
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
SET_ISP_SINGLE_VALUE(isp_test_settings, msc_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, lsc_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, lca_en 		    );
SET_ISP_SINGLE_VALUE(isp_test_settings, gca_en 		    );


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
SET_ISP_SINGLE_VALUE(isp_tunning_settings, lsc_mode);


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
SET_ISP_ARRAY_VALUE(isp_3a_settings, rsc_center_cfg);
SET_ISP_ARRAY_VALUE(isp_3a_settings, af_tolerance_value_tbl);

SET_ISP_ARRAY_VALUE(isp_tunning_settings, bayer_gain);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, lsc_trig_cfg);
SET_ISP_ARRAY_VALUE(isp_tunning_settings, gamma_trig_cfg);
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

SET_ISP_STRUCT_INT(isp_iso_settings, triger);
SET_ISP_ARRAY_INT(isp_iso_settings, isp_lum_mapping_point);
SET_ISP_ARRAY_INT(isp_iso_settings, isp_gain_mapping_point);

SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 0);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 1);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 2);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 3);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 4);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 5);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 6);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 7);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 8);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 9);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 10);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 11);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 12);
SET_ISP_STRUCT_ARRAY_INT(isp_iso_settings, isp_dynamic_cfg, 13);

SET_ISP_CM_VALUE(isp_color_matrix, 0);
SET_ISP_CM_VALUE(isp_color_matrix, 1);
SET_ISP_CM_VALUE(isp_color_matrix, 2);

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
	ISP_FILE_ARRAY_ATTR(isp_dynamic_gbl, triger, 16),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_gbl, isp_lum_mapping_point, 14),
	ISP_FILE_ARRAY_ATTR(isp_dynamic_gbl, isp_gain_mapping_point, 14),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_0, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_1, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_2, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_3, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_4, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_5, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_6, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_7, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_8, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_9, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_10, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_11, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_12, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_dynamic_cfg_13, iso_param, sizeof(struct isp_dynamic_config) / sizeof(HW_S32)),
};

static struct IspParamAttribute IspTuningParam[] = {
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flash_gain),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flash_delay_frame),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, cfa_dir_th),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_th_max),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_th_min),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_th_slope),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_dir_wt),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ctc_dir_th),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flicker_type),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, flicker_ratio),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, hor_visual_angle),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ver_visual_angle),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, focus_length),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, lsc_center_x),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, lsc_center_y),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, bayer_gain, 4),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gtm_type),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, auto_alpha_en),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, gamma_type),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, lsc_mode),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, ff_mod),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, lsc_trig_cfg, 6),

	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, mff_mod),
	ISP_FILE_SINGLE_ATTR(isp_tuning_cfg, msc_mode),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, msc_blw_lut, 12),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, msc_blh_lut, 12),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, gamma_trig_cfg, 5),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, cm_trig_cfg, 3),

	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, gca_cfg, ISP_GCA_MAX),
	ISP_FILE_ARRAY_ATTR(isp_tuning_cfg, lca_cfg, ISP_LCA_MAX),

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

	ISP_FILE_STRUCT_ARRAY_ATTR(isp_color_matrix0, matrix, 12),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_color_matrix1, matrix, 12),
	ISP_FILE_STRUCT_ARRAY_ATTR(isp_color_matrix2, matrix, 12),
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

	if (stat(file_path, &s)) {
		ISP_ERR("%s stat error!\n", file_path);
		return -1;
	}

	fp = fopen(file_path, "r");
	if (fp == NULL) {
		ISP_ERR("open %s failed!\n", file_path);
		return -1;
	}

	buf_len = fread(buf, 1, s.st_size, fp);
	ISP_CFG_LOG(ISP_LOG_CFG, "%s len = %Zu, expect len = %ld\n", file_path, buf_len, s.st_size);
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
				ISP_WARN("fetch %s->%s failed!\n", param->main, param->sub);
			} else {
				if (param->set_param) {
					param->set_param(isp_ini_cfg, (void *)&val, param->len);
					ISP_CFG_LOG(ISP_LOG_CFG, "fetch %s->%s value = %d\n", param->main, param->sub, val);
				}
			}
		} else if (param->len > 1) {
			array_value = (int *)malloc(param->len * sizeof(int));
			memset(array_value, 0, param->len * sizeof(int));

			param->len = cfg_get_int_arr(ini, param->main, param->sub, array_value, param->len);
			if (-1 == param->len) {
				ISP_WARN("fetch %s->%s failed!\n", param->main, param->sub);
			} else {
				if (param->set_param) {
					param->set_param(isp_ini_cfg, (void *)array_value, param->len);
					ISP_CFG_LOG(ISP_LOG_CFG, "fetch %s->%s length = %d\n", param->main, param->sub, param->len);
				}
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
	char isp_gamma_tbl_path[128] = "\0";
	char isp_lsc_tbl_path[128] = "\0";
	char isp_cem_tbl_path[128] = "\0";
	char isp_pltm_tbl_path[128] = "\0";
	char isp_wdr_tbl_path[128] = "\0";

	strcpy(isp_gamma_tbl_path, tbl_patch);
	strcpy(isp_lsc_tbl_path, tbl_patch);
	strcpy(isp_cem_tbl_path, tbl_patch);
	strcpy(isp_pltm_tbl_path, tbl_patch);
	strcpy(isp_wdr_tbl_path, tbl_patch);

	strcat(isp_gamma_tbl_path, "gamma_tbl.bin");
	strcat(isp_lsc_tbl_path, "lsc_tbl.bin");
	strcat(isp_cem_tbl_path, "cem_tbl.bin");
	strcat(isp_pltm_tbl_path, "pltm_tbl.bin");
	strcat(isp_wdr_tbl_path, "wdr_tbl.bin");

	ISP_PRINT("Fetch table from \"%s\"\n", tbl_patch);

	/* fetch gamma_tbl table! */
	len = isp_read_file(isp_gamma_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.gamma_tbl_ini, SIZE_OF_GAMMA_TBL);
	if (len < 0 || len != SIZE_OF_GAMMA_TBL) {
		ISP_WARN("read gamma_tbl from failed! len = %d, but %d is required\n", len, SIZE_OF_GAMMA_TBL);
		isp_ini_cfg->isp_test_settings.gamma_en = 1;
	}

	/* fetch lsc table! */
	len = isp_read_file(isp_lsc_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.lsc_tbl, SIZE_OF_LSC_TBL);
	if (len < 0 || len != SIZE_OF_LSC_TBL) {
		ISP_WARN("read lsc_tbl from failed! len = %d, but %d is required\n", len, SIZE_OF_LSC_TBL);
		isp_ini_cfg->isp_test_settings.lsc_en = 0;
	}

	/* fetch cem table! */
	len = isp_read_file(isp_cem_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.isp_cem_table, ISP_CEM_MEM_SIZE);
	if (len < 0 || len != ISP_CEM_MEM_SIZE) {
		ISP_WARN("read cem_tbl from failed! len = %d, but %d is required\n", len, ISP_CEM_MEM_SIZE);
		isp_ini_cfg->isp_test_settings.cem_en = 0;
	}

	/* fetch pltm table! */
	len = isp_read_file(isp_pltm_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.isp_pltm_table, ISP_PLTM_MEM_SIZE);
	if (len < 0 || len != ISP_PLTM_MEM_SIZE) {
		ISP_WARN("read pltm_table from failed! len = %d, but %d is required\n", len, ISP_PLTM_MEM_SIZE);
		isp_ini_cfg->isp_test_settings.pltm_en = 0;
	}

	/* fetch wdr table! */
	len = isp_read_file(isp_wdr_tbl_path, (char *)isp_ini_cfg->isp_tunning_settings.isp_wdr_table, ISP_WDR_MEM_SIZE);
	if (len < 0 || len != ISP_WDR_MEM_SIZE) {
		ISP_WARN("read wdr_table from wdr_table.bin len = %d, but %d is required\n", len, ISP_WDR_MEM_SIZE);
		isp_ini_cfg->isp_test_settings.wdr_en = 0;
	}

	return ret;
}
#else

struct isp_cfg_array cfg_arr[] = {

#ifdef SENSOR_IMX335
	{"imx335_mipi",  "imx335_30fps_ini_cdr", 2592, 1944, 30, 0, 0, &imx335_30fps_ini_cdr},
#endif

#ifdef SENSOR_GC2053
//	{"gc2053_mipi",  "gc2053_20fps_ini_cdr", 1920, 1080, 20, 0, 0, &gc2053_20fps_ini_cdr},
	{"gc2053_mipi",  "gc2053_30fps_ini_cdr", 1920, 1080, 30, 0, 0, &gc2053_30fps_ini_cdr},
#endif

#ifdef SENSOR_SC2232
	{"sc2232_mipi",  "sc2232_20fps_ini_ipc_day", 1920, 1080, 20, 0, 0, &sc2232_20fps_ini_ipc_day},
	{"sc2232_mipi",  "sc2232_20fps_ini_ipc_night", 1920, 1080, 20, 0, 1, &sc2232_20fps_ini_ipc_night},
//	{"sp2305_mipi",  "sp2305_20fps_ini_ipc_day", 1920, 1080, 20, 0, 0, &sp2305_mipi_day_isp_cfg},
//	{"sp2305_mipi",  "sp2305_20fps_ini_ipc_night", 1920, 1080, 20, 0, 1, &sp2305_mipi_night_isp_cfg},
#endif

#ifdef SENSOR_IMX386
	{"imx386_mipi",  "imx386_60fps_v200", 1920, 1080, 30, 0, 0, &imx386_60fps_mipi_isp_cfg},
#endif
    /*---------------------------------------- v459 default isp_config ----------------------------------------*/
	{"sc2232_mipi",  "sc2232_20fps_ini_ipc_default", 1920, 1080, 20, 0, 0, &sc2232_20fps_ini_ipc_default},
	{"imx307_mipi",  "imx307_30fps_ini_cdr", 1920, 1080, 30, 0, 0, &imx307_30fps_ini_cdr},

};

int find_nearest_index(int mod, int temp)
{
	int i = 0, index = 0;
	int min_dist = 1 << 30, tmp_dist;
	int temp_array1[4] = {2800, 4000, 5000, 6500};
	int temp_array2[6] = {2200, 2800, 4000, 5000, 5500, 6500};

	if(mod == 1) {
		for(i = 0; i < 4; i++) {
			tmp_dist = temp_array1[i] - temp;
			tmp_dist = (tmp_dist < 0) ? -tmp_dist : tmp_dist;
			if(tmp_dist < min_dist) {
				min_dist = tmp_dist;
				index = i;
			}
			ISP_CFG_LOG(ISP_LOG_CFG, "mode: %d, tmp_dist: %d, min_dist: %d, index: %d.\n", mod, tmp_dist, min_dist, index);
		}
	} else if (mod == 2) {
		for(i = 0; i < 6; i++) {
			tmp_dist = temp_array2[i] - temp;
			tmp_dist = (tmp_dist < 0) ? -tmp_dist : tmp_dist;
			if(tmp_dist < min_dist) {
				min_dist = tmp_dist;
				index = i;
			}
			ISP_CFG_LOG(ISP_LOG_CFG, "mode: %d, tmp_dist: %d, min_dist: %d, index: %d.\n", mod, tmp_dist, min_dist, index);
		}
	} else {
		ISP_ERR("mod error.\n");
	}
	ISP_CFG_LOG(ISP_LOG_CFG, "nearest temp index: %d.\n", index);
	return index;
}

int parser_sync_info(struct isp_param_config *param, char *isp_cfg_name, int isp_id)
{
	FILE *file_fd = NULL;
	char fdstr[50];
	int sync_info[775];
	int lsc_ind = 0, lsc_cnt = 0, lsc_tab_cnt = 0;
	int version_num = 0;
	int lsc_temp = 0;
	int enable = 0;

	sprintf(fdstr, "/mnt/extsd/%s_%d.bin", isp_cfg_name, isp_id);
	file_fd = fopen(fdstr, "rb");
	if (file_fd == NULL) {
		ISP_ERR("open bin failed.\n");
		return -1;
	} else {
		fread(&sync_info[0], sizeof(int)*775, 1, file_fd);
	}
	fclose(file_fd);

	version_num = sync_info[0];
	enable = sync_info[1];

	ISP_CFG_LOG(ISP_LOG_CFG, "%s enable mode = %d.\n", __func__, enable);

	if (0 == enable) {
		return 0;
	} else if (1 == enable) {
		memcpy(param->isp_tunning_settings.bayer_gain, &sync_info[2], sizeof(int)*ISP_RAW_CH_MAX);
		return 0;
	} else if (2 == enable) {
		goto lsc_tbl;
	}
	memcpy(param->isp_tunning_settings.bayer_gain, &sync_info[2], sizeof(int)*ISP_RAW_CH_MAX);

	ISP_CFG_LOG(ISP_LOG_CFG, "%s bayer_gain: %d, %d, %d, %d.\n", __func__,
		param->isp_tunning_settings.bayer_gain[0], param->isp_tunning_settings.bayer_gain[1],
		param->isp_tunning_settings.bayer_gain[2], param->isp_tunning_settings.bayer_gain[3]);
lsc_tbl:
	lsc_temp = sync_info[6];
	lsc_ind = find_nearest_index(param->isp_tunning_settings.ff_mod, lsc_temp);
	ISP_CFG_LOG(ISP_LOG_CFG, "%s lsc_ind: %d.\n", __func__, lsc_ind);

	if(param->isp_tunning_settings.ff_mod == 1) {
		for(lsc_tab_cnt = 0; lsc_tab_cnt < 4; lsc_tab_cnt++) {
			if(lsc_tab_cnt == lsc_ind)
				continue;
			for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++) {
				param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]
					= sync_info[7+lsc_cnt]*param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]/param->isp_tunning_settings.lsc_tbl[lsc_ind][lsc_cnt];
			}
		}

		for(lsc_tab_cnt = 4; lsc_tab_cnt < 8; lsc_tab_cnt++) {
			if(lsc_tab_cnt == (lsc_ind+4))
				continue;
			for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++) {
				param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]
					= sync_info[7+lsc_cnt]*param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]/param->isp_tunning_settings.lsc_tbl[lsc_ind+4][lsc_cnt];
			}
		}
		for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++)
			param->isp_tunning_settings.lsc_tbl[lsc_ind][lsc_cnt] = param->isp_tunning_settings.lsc_tbl[lsc_ind+4][lsc_cnt]
				= sync_info[7+lsc_cnt];

		ISP_CFG_LOG(ISP_LOG_CFG, "%s lsc_tbl_1 0: %d, 1: %d, 766: %d, 767: %d.\n", __func__,
			param->isp_tunning_settings.lsc_tbl[1][0], param->isp_tunning_settings.lsc_tbl[1][1],
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
						= sync_info[7+lsc_cnt]*param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]/param->isp_tunning_settings.lsc_tbl[lsc_ind][lsc_cnt];
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
						= sync_info[7+lsc_cnt]*param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt]/param->isp_tunning_settings.lsc_tbl[lsc_ind+6][lsc_cnt];
				if(param->isp_tunning_settings.lsc_tbl[lsc_tab_cnt][lsc_cnt] == 0) {
					ISP_ERR("result------>lsc_ind: %d, lsc_cnt: %d is zero.\n", lsc_tab_cnt, lsc_cnt);
				}
			}
		}
		for(lsc_cnt = 0; lsc_cnt < 768; lsc_cnt++)
			param->isp_tunning_settings.lsc_tbl[lsc_ind][lsc_cnt] = param->isp_tunning_settings.lsc_tbl[lsc_ind+6][lsc_cnt]
				= sync_info[7+lsc_cnt];

		ISP_CFG_LOG(ISP_LOG_CFG, "%s lsc_tbl_1 0: %d, 1: %d, 766: %d, 767: %d.\n", __func__,
			param->isp_tunning_settings.lsc_tbl[1][0], param->isp_tunning_settings.lsc_tbl[1][1],
			param->isp_tunning_settings.lsc_tbl[1][766], param->isp_tunning_settings.lsc_tbl[1][767]);
	} else {
		ISP_ERR("isp ff_mod error.\n");
	}
	return 0;
}
#endif

int isp_save_tbl(struct isp_param_config *param, char *tbl_patch)
{
	FILE *file_fd = NULL;
	char fdstr[50];

	sprintf(fdstr, "%s/gamma_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (file_fd == NULL) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.gamma_tbl_ini, SIZE_OF_GAMMA_TBL, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	sprintf(fdstr, "%s/lsc_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (file_fd == NULL) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.lsc_tbl, SIZE_OF_LSC_TBL, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	sprintf(fdstr, "%s/cem_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (file_fd == NULL) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.isp_cem_table, ISP_CEM_MEM_SIZE, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	sprintf(fdstr, "%s/cem_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (file_fd == NULL) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.isp_cem_table1, ISP_CEM_MEM_SIZE, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	sprintf(fdstr, "%s/pltm_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (file_fd == NULL) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.isp_pltm_table, ISP_PLTM_MEM_SIZE, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	sprintf(fdstr, "%s/wdr_tbl.bin", tbl_patch);
	file_fd = fopen(fdstr, "wb");
	if (file_fd == NULL) {
		ISP_WARN("open %s failed!!!\n", fdstr);
		return -1;
	} else {
		fwrite(param->isp_tunning_settings.isp_wdr_table, ISP_WDR_MEM_SIZE, 1, file_fd);
		ISP_PRINT("save isp_ctx to %s success!!!\n", fdstr);
	}
	fclose(file_fd);

	return 0;
}

int parser_ini_info(struct isp_param_config *param, char *sensor_name,
			int w, int h, int fps, int wdr, int ir, int sync_mode, int isp_id)
{
	int i, ret = 0;

#if !ISP_LIB_USE_INIPARSER



#define ISP_TEST_PATH    "/mnt/extsd/isp/isp_test_settings.bin"
#define ISP_3A_PATH      "/mnt/extsd/isp/isp_3a_settings.bin"
#define ISP_ISO_PATH     "/mnt/extsd/isp/isp_iso_settings.bin"
#define ISP_TUNNING_PATH "/mnt/extsd/isp/isp_tuning_settings.bin"

    int fd;
    struct stat buf;
    FILE *file_fd = NULL;
	struct load_isp_param_t {
		char path[128];
		char *isp_param_settings;
		int size;
	};

	struct load_isp_param_t load_isp_param[4] = {
		{ISP_TEST_PATH,(char *)&param->isp_test_settings, sizeof(struct isp_test_param)},
		{ISP_3A_PATH, (char *)&param->isp_3a_settings, sizeof(struct isp_3a_param)},
		{ISP_ISO_PATH, (char *)&param->isp_iso_settings, sizeof(struct isp_dynamic_param )},
		{ISP_TUNNING_PATH, (char *)&param->isp_tunning_settings, sizeof(struct isp_tunning_param)},
	};

	if ( ISP_TUNING_ENABLE &&
        !access(ISP_TEST_PATH, 0) &&
	    !access(ISP_3A_PATH, 0) &&
	    !access(ISP_ISO_PATH, 0) &&
	    !access(ISP_TUNNING_PATH, 0)) {

		ISP_PRINT("================ ISP will use external isp cfg ============ \n");

		for(i=0; i<4; i++) {
			file_fd = fopen(load_isp_param[i].path, "r");
			fread(load_isp_param[i].isp_param_settings,
			      load_isp_param[i].size, 1, file_fd);

            fd = fileno(file_fd);
            fstat(fd, &buf);
            ISP_PRINT("isp_cfg version : %s\n", ctime(&buf.st_mtime));

			fclose(file_fd);
			file_fd = NULL;
		}

		if(sync_mode)
			parser_sync_info(param, "isp external effect", isp_id);

	} else {

		struct isp_cfg_pt *cfg = NULL;

		for (i = 0; i < array_size(cfg_arr); i++) {
			if (!strncmp(sensor_name, cfg_arr[i].sensor_name, 6) &&
			    (w == cfg_arr[i].width) && (h == cfg_arr[i].height) &&
			    (fps == cfg_arr[i].fps) && (wdr == cfg_arr[i].wdr)) {

	           if(cfg_arr[i].ir == ir)
			    {
	                cfg = cfg_arr[i].cfg;
	                ISP_PRINT("find %s_%d_%d_%d_%d [%s] isp config\n", cfg_arr[i].sensor_name,
		                cfg_arr[i].width, cfg_arr[i].height, cfg_arr[i].fps, cfg_arr[i].wdr, cfg_arr[i].isp_cfg_name);
	                break;
	           }
			}
		}

		if (i == array_size(cfg_arr)) {
			for (i = 0; i < array_size(cfg_arr); i++) {
				if (!strncmp(sensor_name, cfg_arr[i].sensor_name, 6) && (0 == cfg_arr[i].wdr)) {
					cfg = cfg_arr[i].cfg;
					ISP_WARN("cannot find %s_%d_%d_%d_%d_%d isp config, use %s_%d_%d_%d_%d_%d -> [%s]\n", sensor_name, w, h, fps, wdr, ir,
						cfg_arr[i].sensor_name,	cfg_arr[i].width, cfg_arr[i].height, cfg_arr[i].fps, cfg_arr[i].wdr,
						cfg_arr[i].ir, cfg_arr[i].isp_cfg_name);
					break;
				}
			}

			if (i == array_size(cfg_arr)) {
				ISP_WARN("cannot find %s_%d_%d_%d_%d_%d isp config, use default config [%s]\n", sensor_name, w, h, fps, wdr, ir,
	                    cfg_arr[i-1].isp_cfg_name);
	            cfg = cfg_arr[i-1].cfg;     // use sc2232_20fps_ini_ipc_default
				/*return -1;*/
			}
		}
		param->isp_test_settings = *cfg->isp_test_settings;
		param->isp_3a_settings = *cfg->isp_3a_settings;
		param->isp_iso_settings = *cfg->isp_iso_settings;
		param->isp_tunning_settings = *cfg->isp_tunning_settings;

		//isp_save_tbl(param, "/mnt/extsd");

		if(sync_mode)
			parser_sync_info(param, cfg_arr[i].isp_cfg_name, isp_id);
	}

	return 0;

#else
	char path[20] = "/mnt/extsd/";
	char isp_cfg_path[128], isp_tbl_path[128], file_name[128];
	dictionary *ini;

	if (strcmp(sensor_name, "")) {
		sprintf(isp_cfg_path, "%s%s/", path, sensor_name);
		sprintf(isp_tbl_path, "%s%s/bin/", path, sensor_name);
	} else {
		ISP_ERR("sensor cfg name is invalid\n");
		return -1;
	}

	ISP_PRINT("read ini start!!!\n");

	for (i = 0; i < array_size(FileAttr); i++) {
		sprintf(file_name, "%s%s", isp_cfg_path, FileAttr[i].file_name);
		ISP_PRINT("Fetch ini file form \"%s\"\n", file_name);

		ini = iniparser_load(file_name);
		if (ini == NULL) {
			ISP_ERR("read ini error!!!\n");
			return -1;
		}
		isp_parser_cfg(param, ini, &FileAttr[i]);
		iniparser_freedict(ini);
	}
	isp_parser_tbl(param, isp_tbl_path);

	ISP_PRINT("read ini end!!!\n");

	return ret;
#endif
}


