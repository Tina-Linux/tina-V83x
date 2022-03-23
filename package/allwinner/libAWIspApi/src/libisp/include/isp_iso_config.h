
/*
 ******************************************************************************
 *
 * isp_init_iso_config.h
 *
 * Hawkview ISP - isp_init_iso_config.h module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/31	VIDEO INPUT
 *
 *****************************************************************************
 */

#ifndef _ISP_ISO_CONFIG_H_
#define _ISP_ISO_CONFIG_H_

typedef enum isp_iso_param_type {
	ISP_ISO_PLATFORM_ID,
	ISP_ISO_FRAME_ID,
	ISP_ISO_UPDATE_PARAMS,
	ISP_ISO_REG_LIB_CTX,

	ISP_ISO_ADJUST_CNR,
	ISP_ISO_ADJUST_SHARP,
	ISP_ISO_ADJUST_SATUR,
	ISP_ISO_ADJUST_CONTR,
	ISP_ISO_ADJUST_BRIGH,
	ISP_ISO_ADJUST_CEM_RATIO,
	ISP_ISO_ADJUST_DENOISE,
	ISP_ISO_ADJUST_SENSOR_OFFSET,
	ISP_ISO_ADJUST_BLACK_LEVEL,
	ISP_ISO_ADJUST_DPC,
	ISP_ISO_ADJUST_DEFOG_VALUE,
	ISP_ISO_ADJUST_PLTM_DYNAMIC_CFG,
	ISP_ISO_ADJUST_TDNR,
	ISP_ISO_ADJUST_AE_CFG,
	ISP_ISO_ADJUST_GTM_CFG,

	ISP_ISO_TEST_CONFIG,
	ISP_ISO_PARAM_TYPE_MAX,
} iso_param_type_t;

typedef struct iso_test_config {
	HW_S32 isp_test_mode;
} iso_test_config_t;

typedef struct isp_lib_iso_param {
	iso_param_type_t type;
	union{
		HW_S32 isp_platform_id;
		HW_S32 iso_frame_id;

		HW_U32 cnr_adjust;
		HW_U32 sharpness_adjust;
		HW_U32 saturation_adjust;
		HW_U32 contrast_adjust;
		HW_U32 brightness_adjust;
		HW_U32 cem_ratio_adjust;
		HW_U32 denoise_adjust;
		HW_U32 sensor_offset_adjust;
		HW_U32 black_level_adjust;
		HW_U32 dpc_adjust;
		HW_U32 defog_value_adjust;
		HW_U32 pltm_dynamic_cfg_adjust;
		HW_U32 tdnr_adjust;
		HW_U32 ae_cfg_adjust;
		HW_U32 gtm_cfg_adjust;

		struct isp_lib_context *isp_gen;
		struct isp_dynamic_param *param;
		iso_test_config_t test_cfg;
	} u;
} iso_param_t;

typedef struct isp_iso_result {

} iso_result_t;

typedef struct isp_iso_cfg_core_ops {
	HW_S32 (*isp_iso_set_params)(void * iso_core_obj, iso_param_t *param,
						iso_result_t *result);
	HW_S32 (*isp_iso_get_params)(void *iso_core_obj, iso_param_t *param);
	HW_S32 (*isp_iso_run)(void *iso_core_obj, iso_result_t *result);

} isp_iso_core_ops_t;

void* iso_init(isp_iso_core_ops_t **iso_core_ops);
void  iso_exit(void *iso_core_obj);

#endif /*_ISP_ISO_CONFIG_H_*/

