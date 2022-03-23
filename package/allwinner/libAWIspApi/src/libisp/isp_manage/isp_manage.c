/*
 ******************************************************************************
 *
 * isp_manage.c
 *
 * Hawkview ISP - isp_manage.c module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/06/06	VIDEO INPUT
 *
 *****************************************************************************
 */

#include <linux/string.h>
#include "../include/isp_comm.h"
#include "../include/isp_module_cfg.h"
#include "../include/isp_manage.h"
#include "../include/isp_debug.h"
#include "../include/isp_base.h"
#include <math.h>

#define ISP_LIB_USE_AF		1
#define ISP_LIB_USE_MD		1
#define ISP_LIB_USE_ROLLOFF	0

unsigned int isp_lib_log_param = 0;//0xffffffff;

#define clear(x)	memset (&(x), 0, sizeof (x))

#define clamp(val, min, max) ({			\
	typeof(val) __val = (val);		\
	typeof(min) __min = (min);		\
	typeof(max) __max = (max);		\
	__val = __val < __min ? __min : __val;	\
	__val > __max ? __max : __val;		\
})

void isp_get_saved_regs(struct isp_lib_context *isp_gen)
{
	FUNCTION_LOG;
	//isp_gen->stat.min_rgb_saved = bsp_isp_get_saved_cfa_min_rgb();
	//isp_gen->stat.c_noise_saved = bsp_isp_get_saved_cnr_noise();
	FUNCTION_LOG;
}

void isp_rolloff_set_params_helper(isp_rolloff_entity_context_t *isp_rolloff_cxt, rolloff_param_type_t cmd_type)
{
#if ISP_LIB_USE_ROLLOFF
	isp_rolloff_cxt->rolloff_param.type = cmd_type;
	if(isp_rolloff_cxt->ops)
		isp_rolloff_cxt->ops->isp_rolloff_set_params(isp_rolloff_cxt->rolloff_entity,
			&isp_rolloff_cxt->rolloff_param, &isp_rolloff_cxt->rolloff_result);
	else
		printf("isp_rolloff_cxt->ops is NULL, rolloff_param.type = %d!\n", isp_rolloff_cxt->rolloff_param.type);
#endif
}

void isp_afs_set_params_helper(isp_afs_entity_context_t *isp_afs_cxt, afs_param_type_t cmd_type)
{
	isp_afs_cxt->afs_param.type = cmd_type;
	if(isp_afs_cxt->ops)
		isp_afs_cxt->ops->isp_afs_set_params(isp_afs_cxt->afs_entity, &isp_afs_cxt->afs_param, &isp_afs_cxt->afs_result);
	else
		printf("isp_afs_cxt->ops is NULL, afs_param.type = %d!\n", isp_afs_cxt->afs_param.type);
}

void isp_iso_set_params_helper(isp_iso_entity_context_t *isp_iso_cxt, iso_param_type_t cmd_type)
{
	isp_iso_cxt->iso_param.type = cmd_type;
	if(isp_iso_cxt->ops)
		isp_iso_cxt->ops->isp_iso_set_params(isp_iso_cxt->iso_entity, &isp_iso_cxt->iso_param, &isp_iso_cxt->iso_result);
	else
		printf("isp_iso_cxt->ops is NULL, iso_param.type = %d!\n", isp_iso_cxt->iso_param.type);
}

void isp_md_set_params_helper(isp_md_entity_context_t *isp_md_cxt, md_param_type_t cmd_type)
{
#if ISP_LIB_USE_MD
	isp_md_cxt->md_param.type = cmd_type;
	if(isp_md_cxt->ops)
		isp_md_cxt->ops->isp_md_set_params(isp_md_cxt->md_entity, &isp_md_cxt->md_param, &isp_md_cxt->md_result);
	else
		printf("isp_md_cxt->ops is NULL, md_param.type = %d!\n", isp_md_cxt->md_param.type);
#endif
}

void isp_af_set_params_helper(isp_af_entity_context_t *isp_af_cxt, af_param_type_t cmd_type)
{
#if ISP_LIB_USE_AF
	isp_af_cxt->af_param.type = cmd_type;
	if(isp_af_cxt->ops)
		isp_af_cxt->ops->isp_af_set_params(isp_af_cxt->af_entity, &isp_af_cxt->af_param, &isp_af_cxt->af_result);
	else
		printf("isp_af_cxt->ops is NULL, af_param.type = %d!\n", isp_af_cxt->af_param.type);
#endif
}

void isp_awb_set_params_helper(isp_awb_entity_context_t *isp_awb_cxt, awb_param_type_t cmd_type)
{
	isp_awb_cxt->awb_param.type = cmd_type;
	if(isp_awb_cxt->ops)
		isp_awb_cxt->ops->isp_awb_set_params(isp_awb_cxt->awb_entity, &isp_awb_cxt->awb_param, &isp_awb_cxt->awb_result);
	else
		printf("isp_awb_cxt->ops is NULL, awb_param.type = %d!\n", isp_awb_cxt->awb_param.type);
}

void isp_ae_set_params_helper(isp_ae_entity_context_t *isp_ae_cxt, ae_param_type_t cmd_type)
{
	isp_ae_cxt->ae_param.type = cmd_type;
	if(isp_ae_cxt->ops)
		isp_ae_cxt->ops->isp_ae_set_params(isp_ae_cxt->ae_entity, &isp_ae_cxt->ae_param, &isp_ae_cxt->ae_result);
	else
		printf("isp_ae_cxt->ops is NULL, ae_param.type = %d!\n", isp_ae_cxt->ae_param.type);
}

void isp_ae_get_params_helper(isp_ae_entity_context_t *isp_ae_cxt, ae_param_type_t cmd_type)
{
	isp_ae_cxt->ae_param.type = cmd_type;
	if(isp_ae_cxt->ops)
		isp_ae_cxt->ops->isp_ae_get_params(isp_ae_cxt->ae_entity, &isp_ae_cxt->ae_param);
	else
		printf("isp_ae_cxt->ops is NULL, ae_param.type = %d!\n", isp_ae_cxt->ae_param.type);
}

void isp_gtm_set_params_helper(isp_gtm_entity_context_t *isp_gtm_cxt, gtm_param_type_t cmd_type)
{
	isp_gtm_cxt->gtm_param.type = cmd_type;
	if(isp_gtm_cxt->ops)
		isp_gtm_cxt->ops->isp_gtm_set_params(isp_gtm_cxt->gtm_entity, &isp_gtm_cxt->gtm_param, &isp_gtm_cxt->gtm_result);
	else
		printf("isp_gtm_cxt->ops is NULL, af_param.type = %d!\n", isp_gtm_cxt->gtm_param.type);
}

void isp_pltm_set_params_helper(isp_pltm_entity_context_t *isp_pltm_cxt, pltm_param_type_t cmd_type)
{
	isp_pltm_cxt->pltm_param.type = cmd_type;
	if(isp_pltm_cxt->ops)
		isp_pltm_cxt->ops->isp_pltm_set_params(isp_pltm_cxt->pltm_entity, &isp_pltm_cxt->pltm_param, &isp_pltm_cxt->pltm_result);
	else
		printf("isp_pltm_cxt->ops is NULL, pltm_param.type = %d!\n", isp_pltm_cxt->pltm_param.type);
}

void __isp_iso_set_params(struct isp_lib_context *isp_gen)
{
	iso_param_t *iso_param = &isp_gen->iso_entity_ctx.iso_param;
	clear(*iso_param);
	iso_param->u.isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_PLATFORM_ID);

	clear(*iso_param);
	iso_param->u.iso_frame_id = isp_gen->iso_frame_cnt;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_FRAME_ID);

	clear(*iso_param);
	iso_param->u.test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_TEST_CONFIG);
}

void __isp_iso_run(struct isp_lib_context *isp_gen)
{
	isp_iso_entity_context_t *isp_iso_cxt = &isp_gen->iso_entity_ctx;

	isp_iso_cxt->ops->isp_iso_run(isp_iso_cxt->iso_entity, &isp_iso_cxt->iso_result);
}

void __isp_rolloff_set_params(struct isp_lib_context *isp_gen)
{
	rolloff_param_t *rolloff_param = &isp_gen->rolloff_entity_ctx.rolloff_param;
	clear(*rolloff_param);
	rolloff_param->u.isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	isp_rolloff_set_params_helper(&isp_gen->rolloff_entity_ctx, ISP_ROLLOFF_PLATFORM_ID);

	clear(*rolloff_param);
	rolloff_param->u.rolloff_frame_id = isp_gen->af_frame_cnt;
	isp_rolloff_set_params_helper(&isp_gen->rolloff_entity_ctx, ISP_ROLLOFF_FRAME_ID);
}

void __isp_rolloff_run(struct isp_lib_context *isp_gen)
{
	unsigned short *lens_tbl = (unsigned short *)isp_gen->module_cfg.lens_table;
	int i;

	isp_rolloff_entity_context_t *isp_rolloff_cxt = &isp_gen->rolloff_entity_ctx;

	isp_rolloff_cxt->ops->isp_rolloff_run(isp_rolloff_cxt->rolloff_entity,
		&isp_rolloff_cxt->rolloff_stats, &isp_rolloff_cxt->rolloff_result);

	if (!isp_gen->isp_ini_cfg.isp_test_settings.wdr_en) {
		for (i = 0; i < 256; i++)
		{
			lens_tbl[4*i + 0] = isp_rolloff_cxt->rolloff_result.lens_table_output[i + 0];
			lens_tbl[4*i + 1] = isp_rolloff_cxt->rolloff_result.lens_table_output[i + 256];
			lens_tbl[4*i + 2] = isp_rolloff_cxt->rolloff_result.lens_table_output[i + 512];
		}
	} else {
		for (i = 0; i < 256; i++)
		{
			lens_tbl[4*i + 0] = (unsigned short)sqrt((double)isp_rolloff_cxt->rolloff_result.lens_table_output[i + 0] * 1024);
			lens_tbl[4*i + 1] = (unsigned short)sqrt((double)isp_rolloff_cxt->rolloff_result.lens_table_output[i + 256] * 1024);
			lens_tbl[4*i + 2] = (unsigned short)sqrt((double)isp_rolloff_cxt->rolloff_result.lens_table_output[i + 512] * 1024);
		}
	}
}

void __isp_afs_set_params(struct isp_lib_context *isp_gen)
{
	afs_param_t *afs_param = &isp_gen->afs_entity_ctx.afs_param;
	clear(*afs_param);
	afs_param->u.isp_platform_id = isp_gen->module_cfg.isp_platform_id;
	isp_afs_set_params_helper(&isp_gen->afs_entity_ctx, ISP_AFS_PLATFORM_ID);

	clear(*afs_param);
	afs_param->u.afs_frame_id = isp_gen->af_frame_cnt;
	isp_afs_set_params_helper(&isp_gen->afs_entity_ctx, ISP_AFS_FRAME_ID);

	clear(*afs_param);
	afs_param->u.flicker_ratio = isp_gen->isp_ini_cfg.isp_tunning_settings.flicker_ratio;
	isp_afs_set_params_helper(&isp_gen->afs_entity_ctx, ISP_AFS_FLICKER_RATIO);

	clear(*afs_param);
	afs_param->u.flicker_type_ini = isp_gen->isp_ini_cfg.isp_tunning_settings.flicker_type;
	isp_afs_set_params_helper(&isp_gen->afs_entity_ctx, ISP_AFS_FLICKER_TPYE_CONFIG);

	clear(*afs_param);
	afs_param->u.afs_sensor_info = isp_gen->sensor_info;
	isp_afs_set_params_helper(&isp_gen->afs_entity_ctx, ISP_AFS_SENSOR_INFO);


	clear(*afs_param);
	afs_param->u.test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	afs_param->u.test_cfg.afs_en = isp_gen->isp_ini_cfg.isp_test_settings.afs_en;
	isp_afs_set_params_helper(&isp_gen->afs_entity_ctx, ISP_AFS_TEST_CONFIG);
}

void __isp_afs_run(struct isp_lib_context *isp_gen)
{
	isp_afs_entity_context_t *isp_afs_cxt = &isp_gen->afs_entity_ctx;

	isp_afs_cxt->ops->isp_afs_run(isp_afs_cxt->afs_entity, &isp_afs_cxt->afs_stats, &isp_afs_cxt->afs_result);

	if (isp_gen->ops->afs_done) {
		isp_gen->ops->afs_done(isp_gen, &isp_afs_cxt->afs_result);
	}

	isp_gen->ae_settings.flicker_type = isp_afs_cxt->afs_result.flicker_type_output;
}

void __isp_md_set_params(struct isp_lib_context *isp_gen)
{
	clear(isp_gen->md_entity_ctx.md_param);
	isp_gen->md_entity_ctx.md_param.u.isp_platform_id =
					isp_gen->module_cfg.isp_platform_id;
	isp_md_set_params_helper(&isp_gen->md_entity_ctx, ISP_MD_PLATFORM_ID);

	clear(isp_gen->md_entity_ctx.md_param);
	isp_gen->md_entity_ctx.md_param.u.md_frame_id = isp_gen->md_frame_cnt;
	isp_md_set_params_helper(&isp_gen->md_entity_ctx, ISP_MD_FRAME_ID);

	clear(isp_gen->md_entity_ctx.md_param);
	isp_gen->md_entity_ctx.md_param.u.af_scene_motion_th =
			isp_gen->isp_ini_cfg.isp_3a_settings.af_scene_motion_th;
	isp_md_set_params_helper(&isp_gen->md_entity_ctx, ISP_MD_SCENE_MOTION_TH);

	clear(isp_gen->md_entity_ctx.md_param);
	isp_gen->sensor_info.is_af_busy = (isp_gen->af_entity_ctx.af_result.af_status_output ==	AUTO_FOCUS_STATUS_BUSY) ? 1 : 0;
	isp_gen->md_entity_ctx.md_param.u.md_sensor_info = isp_gen->sensor_info;
	isp_md_set_params_helper(&isp_gen->md_entity_ctx, ISP_MD_SENSOR_INFO);

	clear(isp_gen->md_entity_ctx.md_param);
	isp_gen->md_entity_ctx.md_param.u.test_cfg.isp_test_mode =
			isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_md_set_params_helper(&isp_gen->md_entity_ctx, ISP_MD_TEST_CONFIG);
}

void __isp_md_run(struct isp_lib_context *isp_gen)
{
	isp_md_entity_context_t *isp_md_cxt = &isp_gen->md_entity_ctx;

	isp_md_cxt->ops->isp_md_run(isp_md_cxt->md_entity,
				&isp_md_cxt->md_stats, &isp_md_cxt->md_result);
	if (isp_gen->ops->md_done) {
		isp_gen->ops->md_done(isp_gen, &isp_md_cxt->md_result);
	}
	isp_gen->sensor_info.motion_flag = isp_md_cxt->md_result.motion_flag;
}

void __isp_af_set_params(struct isp_lib_context *isp_gen)
{
	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.af_frame_id = isp_gen->af_frame_cnt;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_FRAME_ID);

	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.focus_absolute =
					isp_gen->af_settings.focus_absolute;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_FOCUS_ABSOLUTE);


	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.focus_relative =
					isp_gen->af_settings.focus_relative;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_FOCUS_RELATIVE);

	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.af_run_mode =
						isp_gen->af_settings.af_mode;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_RUN_MODE);

	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.af_metering_mode =
					isp_gen->af_settings.af_metering_mode;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_METERING_MODE);

	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.af_range =
						isp_gen->af_settings.af_range;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_RANGE);

	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.focus_lock =
						isp_gen->af_settings.focus_lock;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_LOCK);

	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.sensor_info =	isp_gen->sensor_info;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_SENSOR_INFO);
}

void __isp_af_run(struct isp_lib_context *isp_gen)
{
	isp_af_entity_context_t *isp_af_cxt = &isp_gen->af_entity_ctx;

	isp_af_cxt->ops->isp_af_run(isp_af_cxt->af_entity,
				&isp_af_cxt->af_stats, &isp_af_cxt->af_result);

	if (isp_gen->ops->af_done) {
		isp_gen->ops->af_done(isp_gen, &isp_af_cxt->af_result);
	}

	isp_gen->module_cfg.af_cfg.af_sap_lim = isp_af_cxt->af_result.af_sap_lim_output;
}

void __isp_awb_set_params(struct isp_lib_context *isp_gen)
{

	clear(isp_gen->awb_entity_ctx.awb_param);
	isp_gen->awb_entity_ctx.awb_param.u.awb_frame_id = isp_gen->awb_frame_cnt;
	isp_awb_set_params_helper(&isp_gen->awb_entity_ctx, ISP_AWB_FRAME_ID);

	clear(isp_gen->awb_entity_ctx.awb_param);
	isp_gen->awb_entity_ctx.awb_param.u.awb_ctrl = isp_gen->awb_settings;
	isp_awb_set_params_helper(&isp_gen->awb_entity_ctx, ISP_AWB_CTRL_CFG);

	//sensor info.
	clear(isp_gen->awb_entity_ctx.awb_param);
	isp_gen->awb_entity_ctx.awb_param.u.awb_sensor_info = isp_gen->sensor_info;
	//printf("awb_entity_ctx r_gain= %d\n", isp_gen->awb_entity_ctx.awb_param.u.awb_sensor_info.wb_gain_saved.r_gain);
	isp_awb_set_params_helper(&isp_gen->awb_entity_ctx, ISP_AWB_SENSOR_INFO);

	//test cfg.
	clear(isp_gen->awb_entity_ctx.awb_param);
	isp_gen->awb_entity_ctx.awb_param.u.test_cfg.awb_en =
			isp_gen->isp_ini_cfg.isp_test_settings.awb_en;
	isp_gen->awb_entity_ctx.awb_param.u.test_cfg.isp_color_temp =
			isp_gen->isp_ini_cfg.isp_test_settings.isp_color_temp;
	isp_gen->awb_entity_ctx.awb_param.u.test_cfg.isp_test_mode =
			isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_awb_set_params_helper(&isp_gen->awb_entity_ctx, ISP_AWB_TEST_CONFIG);
}

void __isp_awb_run(struct isp_lib_context *isp_gen)
{
	isp_awb_entity_context_t *isp_awb_cxt = &isp_gen->awb_entity_ctx;

	isp_awb_cxt->ops->isp_awb_run(isp_awb_cxt->awb_entity, &isp_awb_cxt->awb_stats, &isp_awb_cxt->awb_result);

	if (isp_gen->ops->awb_done) {
		isp_gen->ops->awb_done(isp_gen, &isp_awb_cxt->awb_result);
	}

	if (!isp_gen->isp_ini_cfg.isp_test_settings.wdr_en) {
		isp_gen->module_cfg.wb_gain_cfg.wb_gain = isp_awb_cxt->awb_result.wb_gain_output;
	} else {
		isp_gen->module_cfg.wb_gain_cfg.wb_gain.r_gain = (HW_U16)sqrt((double)isp_awb_cxt->awb_result.wb_gain_output.r_gain*256);
		isp_gen->module_cfg.wb_gain_cfg.wb_gain.gr_gain = (HW_U16)sqrt((double)isp_awb_cxt->awb_result.wb_gain_output.gr_gain*256);
		isp_gen->module_cfg.wb_gain_cfg.wb_gain.gb_gain = (HW_U16)sqrt((double)isp_awb_cxt->awb_result.wb_gain_output.gb_gain*256);
		isp_gen->module_cfg.wb_gain_cfg.wb_gain.b_gain = (HW_U16)sqrt((double)isp_awb_cxt->awb_result.wb_gain_output.b_gain*256);
	}

	isp_gen->stats_ctx.wb_gain_saved = isp_awb_cxt->awb_result.wb_gain_output;
}

void __isp_ae_set_params(struct isp_lib_context *isp_gen)
{
	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_frame_id = isp_gen->ae_frame_cnt;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_FRAME_ID);

	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_isp_id = isp_gen->isp_id;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_ISP_ID);

	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_setting = isp_gen->ae_settings;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_SETTINGS);

	//ae_sensor_info.
	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_sensor_info = isp_gen->sensor_info;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_SENSOR_INFO);

	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_target_comp = isp_gen->pltm_entity_ctx.pltm_result.pltm_ae_comp;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_PLTM_COMP);
	//test cfg.
	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_gain = isp_gen->isp_ini_cfg.isp_test_settings.isp_gain;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_exp_line = isp_gen->isp_ini_cfg.isp_test_settings.isp_exp_line;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.ae_forced = isp_gen->isp_ini_cfg.isp_test_settings.ae_forced;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.lum_forced = isp_gen->isp_ini_cfg.isp_test_settings.lum_forced;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_test_exptime = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_exptime;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.exp_line_start = isp_gen->isp_ini_cfg.isp_test_settings.exp_line_start;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.exp_line_step = isp_gen->isp_ini_cfg.isp_test_settings.exp_line_step;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.exp_line_end = isp_gen->isp_ini_cfg.isp_test_settings.exp_line_end;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.exp_change_interval = isp_gen->isp_ini_cfg.isp_test_settings.exp_change_interval;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_test_gain = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_gain;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.gain_start = isp_gen->isp_ini_cfg.isp_test_settings.gain_start;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.gain_step = isp_gen->isp_ini_cfg.isp_test_settings.gain_step;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.gain_end = isp_gen->isp_ini_cfg.isp_test_settings.gain_end;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.gain_change_interval = isp_gen->isp_ini_cfg.isp_test_settings.gain_change_interval;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.ae_en = isp_gen->isp_ini_cfg.isp_test_settings.ae_en;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_TEST_CONFIG);
}

void __isp_ae_update_sensor_info(struct isp_lib_context *isp_gen)
{
	isp_sensor_info_t *sensor_info = &isp_gen->sensor_info;
	ae_result_t *result = &isp_gen->ae_entity_ctx.ae_result;
	HW_U32 total_vts = isp_gen->sensor_info.exp_line / 16;

	sensor_info->exp_line = result->sensor_set.ev_set_curr.ev_sensor_exp_line;
	sensor_info->ang_gain = result->sensor_set.ev_set_curr.ev_analog_gain;
	sensor_info->dig_gain = result->sensor_set.ev_set_curr.ev_digital_gain;

	sensor_info->ae_tbl_idx = result->sensor_set.ev_set_curr.ev_idx;
	sensor_info->ae_tbl_idx_max = result->sensor_set.ev_idx_max;

	sensor_info->ae_gain = result->ae_gain;
	sensor_info->ae_lv = result->sensor_set.ev_set_curr.ev_lv;

	if (sensor_info->vts && sensor_info->hts && sensor_info->pclk) {
		total_vts = sensor_info->vts > total_vts ? sensor_info->vts : total_vts;
		sensor_info->fps = sensor_info->pclk / (total_vts * sensor_info->hts);
	} else {
		sensor_info->fps = 30;
	}
	sensor_info->fps = clamp(sensor_info->fps, 1, 1000);
	sensor_info->frame_time = 1000 / sensor_info->fps; /*ms*/
}

void __isp_ae_run(struct isp_lib_context *isp_gen)
{
	isp_ae_entity_context_t *isp_ae_cxt = &isp_gen->ae_entity_ctx;
	isp_ae_cxt->ops->isp_ae_run(isp_ae_cxt->ae_entity,
				&isp_ae_cxt->ae_stats, &isp_ae_cxt->ae_result);

	if (isp_gen->ops->ae_done) {
		isp_gen->ops->ae_done(isp_gen, &isp_ae_cxt->ae_result);
	}

	__isp_ae_update_sensor_info(isp_gen);
	config_gamma(isp_gen);
	config_dig_gain(isp_gen, isp_ae_cxt->ae_result.sensor_set.ev_set_curr.ev_digital_gain);
	config_wdr(isp_gen, 0);
}

void __isp_gtm_set_params(struct isp_lib_context *isp_gen)
{
	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.gtm_frame_id = isp_gen->gtm_frame_cnt;
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_FRAME_ID);

	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.contrast = isp_gen->adjust.contrast;
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_CONTRAST);

	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.brightness = isp_gen->adjust.brightness;
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_BRIGHTNESS);

	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.gtm_enable = isp_gen->isp_ini_cfg.isp_test_settings.gtm_en;
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_ENABLE);

	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.BrightPixellValue =	isp_gen->ae_entity_ctx.ae_result.BrightPixellValue;
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_BRIGHT_PIXEL_VALUE);

	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.DarkPixelValue = isp_gen->ae_entity_ctx.ae_result.DarkPixelValue;
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_DARK_PIXEL_VALUE);

	//test cfg.
	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_TEST_CONFIG);

	//gtm_ini_cfg.
	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.gtm_ini.gtm_type = isp_gen->isp_ini_cfg.isp_tunning_settings.gtm_type;
	isp_gen->gtm_entity_ctx.gtm_param.u.gtm_ini.gamma_type = isp_gen->isp_ini_cfg.isp_tunning_settings.gamma_type;
	isp_gen->gtm_entity_ctx.gtm_param.u.gtm_ini.AutoAlphaEn	= isp_gen->isp_ini_cfg.isp_tunning_settings.auto_alpha_en;
	memcpy(&isp_gen->gtm_entity_ctx.gtm_param.u.gtm_ini.gtm_cfg[0], &isp_gen->ae_settings.ae_hist_eq_cfg[0], GTM_HEQ_MAX*sizeof(int));
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_INI_DATA);
}

void __isp_gtm_run(struct isp_lib_context *isp_gen)
{
	isp_gtm_entity_context_t *isp_gtm_cxt = &isp_gen->gtm_entity_ctx;

	isp_gtm_cxt->ops->isp_gtm_run(isp_gtm_cxt->gtm_entity,
				&isp_gtm_cxt->gtm_stats,
				&isp_gtm_cxt->gtm_result);
}

void __isp_pltm_set_params(struct isp_lib_context *isp_gen)
{
	clear(isp_gen->pltm_entity_ctx.pltm_param);
	isp_gen->pltm_entity_ctx.pltm_param.u.pltm_frame_id = isp_gen->ae_frame_cnt;
	isp_pltm_set_params_helper(&isp_gen->pltm_entity_ctx, ISP_PLTM_FRAME_ID);

	clear(isp_gen->pltm_entity_ctx.pltm_param);
	isp_gen->pltm_entity_ctx.pltm_param.u.pltm_enable =
				isp_gen->isp_ini_cfg.isp_test_settings.pltm_en;
	isp_pltm_set_params_helper(&isp_gen->pltm_entity_ctx, ISP_PLTM_ENABLE);

	//pltm sensor_info.
	clear(isp_gen->pltm_entity_ctx.pltm_param);
	isp_gen->pltm_entity_ctx.pltm_param.u.sensor_info = isp_gen->sensor_info;
	isp_pltm_set_params_helper(&isp_gen->pltm_entity_ctx, ISP_PLTM_SENSOR_INFO);

	//pltm_ini_cfg.
	clear(isp_gen->pltm_entity_ctx.pltm_param);
	memcpy(&isp_gen->pltm_entity_ctx.pltm_param.u.pltm_ini.pltm_cfg[0],
		&isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[0], ISP_PLTM_MAX*sizeof(HW_S32));
	memcpy(&isp_gen->pltm_entity_ctx.pltm_param.u.pltm_ini.pltm_dynamic_cfg[0],
		&isp_gen->ae_settings.pltm_dynamic_cfg[0], ISP_PLTM_DYNAMIC_MAX*sizeof(HW_S32));
	isp_pltm_set_params_helper(&isp_gen->pltm_entity_ctx, ISP_PLTM_INI_DATA);
}

void __isp_pltm_run(struct isp_lib_context *isp_gen)
{
	pltm_result_t *result = &isp_gen->pltm_entity_ctx.pltm_result;
	struct isp_pltm_config *pltm_cfg = &isp_gen->module_cfg.pltm_cfg;

	isp_pltm_entity_context_t *isp_pltm_cxt = &isp_gen->pltm_entity_ctx;

	isp_pltm_cxt->ops->isp_pltm_run(isp_pltm_cxt->pltm_entity,
			&isp_pltm_cxt->pltm_stats, &isp_pltm_cxt->pltm_result);
	if (isp_gen->ops->pltm_done) {
		isp_gen->ops->pltm_done(isp_gen, &isp_pltm_cxt->pltm_result);
	}
	memcpy(pltm_cfg->pltm_table, result->pltm_tbl, ISP_PLTM_MEM_SIZE);
	pltm_cfg->oripic_ratio = result->pltm_oripic_ratio;
	pltm_cfg->tr_order = result->pltm_tr_order;
	pltm_cfg->last_order_ratio = result->pltm_last_order_ratio;
	pltm_cfg->cal_en = result->pltm_cal_en;
	pltm_cfg->frm_sm_en = result->pltm_frm_sm_en;
	pltm_cfg->block_height = result->pltm_block_height;
	pltm_cfg->block_width = result->pltm_block_width;
	pltm_cfg->statistic_div = result->pltm_statistic_div;

	pltm_cfg->lss_switch = isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_LSS_SWITCH];
	pltm_cfg->lum_ratio = isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_LUM_RATIO];
	pltm_cfg->lp_halo_res = isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_LP_HALO_RES];
	pltm_cfg->white_level = isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_WHITE_LEVEL];
	pltm_cfg->intens_asym = isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_INTENS_ASYM];
	pltm_cfg->spatial_asm = isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_SPATIAL_ASM];
	pltm_cfg->block_h_num = isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_BLOCK_H_NUM];
	pltm_cfg->block_v_num = isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[ISP_PLTM_BLOCK_V_NUM];
	ISP_LIB_LOG(ISP_LOG_PLTM, "oripic_ratio = %d, tr_order = %d. last_order_ratio = %d, cal_en = %d, frm_sm_en = %d.\n",
		pltm_cfg->oripic_ratio, pltm_cfg->tr_order, pltm_cfg->last_order_ratio, pltm_cfg->cal_en, pltm_cfg->frm_sm_en);
	ISP_LIB_LOG(ISP_LOG_PLTM, " block_height = %d, block_width = %d, statistic_div = %d.\n",
		pltm_cfg->block_height, pltm_cfg->block_width, pltm_cfg->statistic_div);
}

void __isp_ctx_cfg_lib(struct isp_lib_context *isp_gen)
{
	struct isp_param_config *param = &isp_gen->isp_ini_cfg;

	// isp_lib_context def settings
	isp_gen->adjust.contrast   = 0;
	isp_gen->adjust.brightness = 0;
	isp_gen->adjust.sharpness  = 0;
	isp_gen->adjust.saturation_cb = 100;
	isp_gen->adjust.saturation_cr = 100;
	isp_gen->adjust.cem_ratio = 256;
	isp_gen->adjust.defog_value = 0;

	isp_gen->tune.contrast_level = 0;
	isp_gen->tune.brightness_level = 0;
	isp_gen->tune.sharpness_level = 0;
	isp_gen->tune.saturation_level = 100;
	isp_gen->tune.tdf_level = 50;
	isp_gen->tune.denoise_level = 50;
	isp_gen->tune.pltmwdr_level = 0;

	isp_gen->awb_settings.wb_gain_manual.r_gain = 256;
	isp_gen->awb_settings.wb_gain_manual.gr_gain = 256;
	isp_gen->awb_settings.wb_gain_manual.gb_gain = 256;
	isp_gen->awb_settings.wb_gain_manual.b_gain = 256;

	// exp settings
	isp_gen->ae_settings.exp_mode = EXP_AUTO;
	isp_gen->ae_settings.exp_metering_mode = AE_METERING_MODE_MATRIX;
	isp_gen->ae_settings.flash_mode = FLASH_MODE_OFF;
	isp_gen->ae_settings.flash_open = 0;
	isp_gen->ae_settings.flicker_mode = FREQUENCY_AUTO;
	isp_gen->ae_settings.iso_mode = ISO_AUTO;
	isp_gen->ae_settings.exposure_lock = false;
	isp_gen->ae_settings.exp_compensation = 0;

	// af settings
	isp_gen->af_settings.af_mode = AUTO_FOCUS_CONTINUEOUS;
	isp_gen->af_settings.af_metering_mode = AUTO_FOCUS_METERING_CENTER_WEIGHTED;
	isp_gen->af_settings.focus_lock = false;
	isp_gen->af_settings.af_range = AUTO_FOCUS_RANGE_AUTO;

	// awb settings
	isp_gen->awb_settings.wb_mode = WB_AUTO;
	isp_gen->awb_settings.white_balance_lock = false;

	isp_gen->awb_settings.awb_coor.x1 = -1000;
	isp_gen->awb_settings.awb_coor.y1 = -1000;
	isp_gen->awb_settings.awb_coor.x2 = 1000;
	isp_gen->awb_settings.awb_coor.y2 = 1000;
	isp_gen->ae_settings.ae_coor.x1 = -1000;
	isp_gen->ae_settings.ae_coor.y1 = -1000;
	isp_gen->ae_settings.ae_coor.x2 = 1000;
	isp_gen->ae_settings.ae_coor.y2 = 1000;

	isp_gen->af_settings.af_coor.x1 = -1000;
	isp_gen->af_settings.af_coor.y1 = -1000;
	isp_gen->af_settings.af_coor.x2 = 1000;
	isp_gen->af_settings.af_coor.y2 = 1000;

	isp_gen->stat.min_rgb_saved = 1023;
	isp_gen->stat.c_noise_saved = 20;

	isp_gen->stats_ctx.wb_gain_saved.r_gain = 256;
	isp_gen->stats_ctx.wb_gain_saved.gr_gain = 256;
	isp_gen->stats_ctx.wb_gain_saved.gb_gain = 256;
	isp_gen->stats_ctx.wb_gain_saved.b_gain = 256;
}

#define ISP_CTX_MODULE_EN(en_bit, ISP_FEATURES) \
{ \
	if (en_bit) {\
		mod_cfg->module_enable_flag |= ISP_FEATURES;\
	} else {\
		mod_cfg->module_enable_flag &= ~ISP_FEATURES;\
	}\
}

int __isp_ctx_apply_enable(struct isp_lib_context *isp_gen)
{
	struct isp_module_config *mod_cfg = &isp_gen->module_cfg;
	struct isp_param_config *param = &isp_gen->isp_ini_cfg;

	ISP_CTX_MODULE_EN(param->isp_test_settings.sharp_en       , ISP_FEATURES_SAP);
	ISP_CTX_MODULE_EN(param->isp_test_settings.contrast_en	  , ISP_FEATURES_CONTRAST);
	ISP_CTX_MODULE_EN(param->isp_test_settings.denoise_en     , ISP_FEATURES_BDNF);
	ISP_CTX_MODULE_EN(param->isp_test_settings.drc_en         , ISP_FEATURES_RGB_DRC);
	ISP_CTX_MODULE_EN(param->isp_test_settings.lsc_en         , ISP_FEATURES_LSC);
	ISP_CTX_MODULE_EN(param->isp_test_settings.gamma_en       , ISP_FEATURES_GAMMA);
	ISP_CTX_MODULE_EN(param->isp_test_settings.cm_en          , ISP_FEATURES_RGB2RGB);
	ISP_CTX_MODULE_EN(param->isp_test_settings.blc_en 	  , ISP_FEATURES_BLC);
	ISP_CTX_MODULE_EN(param->isp_test_settings.wb_en          , ISP_FEATURES_WB);
	ISP_CTX_MODULE_EN(param->isp_test_settings.otf_dpc_en     , ISP_FEATURES_OTF_DPC);
	ISP_CTX_MODULE_EN(param->isp_test_settings.cfa_en         , ISP_FEATURES_CFA);
	ISP_CTX_MODULE_EN(param->isp_test_settings.tdf_en         , ISP_FEATURES_3D_DENOISE);
	ISP_CTX_MODULE_EN(param->isp_test_settings.cnr_en         , ISP_FEATURES_CNR);
	ISP_CTX_MODULE_EN(param->isp_test_settings.satur_en       , ISP_FEATURES_SATU);
	ISP_CTX_MODULE_EN(param->isp_test_settings.linear_en      , ISP_FEATURES_LINEAR);
	ISP_CTX_MODULE_EN(param->isp_test_settings.dig_gain_en    , ISP_FEATURES_DG);
	ISP_CTX_MODULE_EN(param->isp_test_settings.cem_en	  , ISP_FEATURES_CEM);
	ISP_CTX_MODULE_EN(param->isp_test_settings.pltm_en	  , ISP_FEATURES_PLTM);
	ISP_CTX_MODULE_EN(param->isp_test_settings.wdr_en	  , ISP_FEATURES_WDR);
	ISP_CTX_MODULE_EN(param->isp_test_settings.so_en	  , ISP_FEATURES_SO);
	ISP_CTX_MODULE_EN(param->isp_test_settings.ctc_en	  , ISP_FEATURES_CTC);

	ISP_CTX_MODULE_EN(param->isp_test_settings.afs_en	  , ISP_FEATURES_AFS);
	ISP_CTX_MODULE_EN(param->isp_test_settings.ae_en	  , ISP_FEATURES_AE);
	ISP_CTX_MODULE_EN(param->isp_test_settings.awb_en	  , ISP_FEATURES_AWB);
	ISP_CTX_MODULE_EN(param->isp_test_settings.af_en	  , ISP_FEATURES_AF);
	ISP_CTX_MODULE_EN(param->isp_test_settings.hist_en	  , ISP_FEATURES_HIST);

	mod_cfg->module_enable_flag |= ISP_FEATURES_MODE;

	isp_lib_log_param = param->isp_test_settings.isp_log_param;

	return 0;
}

static void __isp_ctx_cfg_mod(struct isp_lib_context *isp_gen)
{
	//after __isp_ctx_cfg_lib
	struct isp_module_config *mod_cfg = &isp_gen->module_cfg;
	struct isp_param_config *param = &isp_gen->isp_ini_cfg;

	if (param->isp_test_settings.wb_en == 0 ||
	    mod_cfg->wb_gain_cfg.wb_gain.r_gain == 0 ||
	    mod_cfg->wb_gain_cfg.wb_gain.gr_gain == 0 ||
	    mod_cfg->wb_gain_cfg.wb_gain.gb_gain == 0 ||
	    mod_cfg->wb_gain_cfg.wb_gain.b_gain == 0) {
		mod_cfg->wb_gain_cfg.wb_gain.r_gain  = 256;
		mod_cfg->wb_gain_cfg.wb_gain.gr_gain = 256;
		mod_cfg->wb_gain_cfg.wb_gain.gb_gain = 256;
		mod_cfg->wb_gain_cfg.wb_gain.b_gain  = 256;
	}
	mod_cfg->wb_gain_cfg.clip_val = 4092;

	mod_cfg->cfa_cfg.dir_th = ISP_CFA_DIR_TH;
	mod_cfg->af_cfg.af_sap_lim = ISP_AF_DIR_TH;

	mod_cfg->sharp_cfg.sharp_max_val = 0;
	mod_cfg->sharp_cfg.sharp_min_val = 0;
	mod_cfg->sharp_cfg.black_level = 0;
	mod_cfg->sharp_cfg.white_level = 0;
	mod_cfg->sharp_cfg.black_clip = 0;
	mod_cfg->sharp_cfg.white_clip = 0;

	mod_cfg->hist_cfg.hist_mode = MAX_MODE;

	mod_cfg->mode_cfg.input_fmt = isp_gen->sensor_info.input_seq;

	if (isp_gen->sensor_info.wdr_mode == 2) {
		mod_cfg->mode_cfg.wdr_mode = COMANDING_WDR;
		isp_gen->ae_settings.ae_mode = AE_NORM;
	} else {
		mod_cfg->mode_cfg.wdr_mode = DOL_WDR;
		isp_gen->ae_settings.ae_mode = AE_WDR_FIXED_RATIO;
	}
	mod_cfg->mode_cfg.wdr_cmp_mode = 0;

	config_wdr(isp_gen, 1);

	//mod_cfg->mode_cfg.otf_dpc_mode = OTF_DPC_STRONG;
	mod_cfg->mode_cfg.saturation_mode = SATU_NORM_MODE;
	mod_cfg->mode_cfg.hist_sel = param->isp_3a_settings.ae_hist_sel;//HIST_AFTER_CNR;
	mod_cfg->mode_cfg.cfa_mode = CFA_NORM_MODE;
	mod_cfg->mode_cfg.ae_mode = param->isp_3a_settings.ae_stat_sel;//AE_AFTER_CNR;
	mod_cfg->mode_cfg.awb_mode = param->isp_3a_settings.awb_stat_sel;//AWB_AFTER_WDR;
	mod_cfg->mode_cfg.dg_mode = DG_AFTER_SO;

	mod_cfg->output_speed = 1;

	//config_blc(isp_gen);
	config_dig_gain(isp_gen, 256);

	/*  todo
	mod_cfg->awb_cfg.awb_r_sat_lim = clamp(((AWB_SAT_DEF_LIM *
		mod_cfg->gain_offset_cfg.gain.r_gain >> 10) +
		mod_cfg->gain_offset_cfg.offset.r_offset),
		1024, AWB_SAT_DEF_LIM)>>4;
	mod_cfg->awb_cfg.awb_g_sat_lim = clamp(((AWB_SAT_DEF_LIM *
		mod_cfg->gain_offset_cfg.gain.gr_gain >> 10) +
		mod_cfg->gain_offset_cfg.offset.gr_offset),
		1024, AWB_SAT_DEF_LIM)>>4;
	mod_cfg->awb_cfg.awb_b_sat_lim = clamp(((AWB_SAT_DEF_LIM *
		mod_cfg->gain_offset_cfg.gain.b_gain >> 10) +
		mod_cfg->gain_offset_cfg.offset.b_offset),
		1024, AWB_SAT_DEF_LIM)>>4;
	*/

	mod_cfg->awb_cfg.awb_r_sat_lim = AWB_SAT_DEF_LIM;
	mod_cfg->awb_cfg.awb_g_sat_lim = AWB_SAT_DEF_LIM;
	mod_cfg->awb_cfg.awb_b_sat_lim = AWB_SAT_DEF_LIM;

	if (isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist_mod_en == 1) {
		mod_cfg->hist_cfg.hist_mode = MAX_MODE;
	} else if (isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist_mod_en == 2) {
		mod_cfg->hist_cfg.hist_mode = MIN_MODE;
	} else if (isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist_mod_en == 3) {
		mod_cfg->hist_cfg.hist_mode = AVG_MODE;
	}

	//CFA
	if (isp_gen->isp_ini_cfg.isp_tunning_settings.cfa_dir_th == 0) {
		mod_cfg->cfa_cfg.dir_th = ISP_CFA_DIR_TH;
	} else {
		mod_cfg->cfa_cfg.dir_th = isp_gen->isp_ini_cfg.isp_tunning_settings.cfa_dir_th;
	}
	//2d-filter

	mod_cfg->rgb2rgb_cfg.color_matrix_default = param->isp_tunning_settings.color_matrix_ini[0];
	mod_cfg->rgb2rgb_cfg.color_matrix = param->isp_tunning_settings.color_matrix_ini[0];

	if (param->isp_test_settings.defog_en==1) {
		mod_cfg->cfa_cfg.min_rgb    = 1023;
	} else {
		mod_cfg->cfa_cfg.min_rgb    = 0;
	}

	//ctc todo
	mod_cfg->ctc_cfg.ctc_th_max = param->isp_tunning_settings.ctc_th_max;//316;
	mod_cfg->ctc_cfg.ctc_th_min = param->isp_tunning_settings.ctc_th_min;//60;
	mod_cfg->ctc_cfg.ctc_th_slope = param->isp_tunning_settings.ctc_th_slope;//262;
	mod_cfg->ctc_cfg.ctc_dir_wt = param->isp_tunning_settings.ctc_dir_wt;//64;
	mod_cfg->ctc_cfg.ctc_dir_th = param->isp_tunning_settings.ctc_dir_th;//80;

	//pltm
	mod_cfg->pltm_cfg.lss_switch = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_LSS_SWITCH];
	mod_cfg->pltm_cfg.last_order_ratio = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_LAST_ORDER_RATIO];
	mod_cfg->pltm_cfg.tr_order = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_TR_ORDER];
	mod_cfg->pltm_cfg.oripic_ratio = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_ORIPIC_RATIO];
	mod_cfg->pltm_cfg.intens_asym = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_INTENS_ASYM];
	mod_cfg->pltm_cfg.spatial_asm = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_SPATIAL_ASM];
	mod_cfg->pltm_cfg.white_level = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_WHITE_LEVEL];
	mod_cfg->pltm_cfg.lp_halo_res = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_LP_HALO_RES];
	mod_cfg->pltm_cfg.lum_ratio = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_LUM_RATIO];
	mod_cfg->pltm_cfg.block_v_num = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_BLOCK_V_NUM];
	mod_cfg->pltm_cfg.block_h_num = param->isp_tunning_settings.pltm_cfg[ISP_PLTM_BLOCK_H_NUM];

	config_band_step(isp_gen);

	//gamma table
	if (param->isp_test_settings.gamma_en)
		config_gamma(isp_gen);

	if (param->isp_test_settings.linear_en);
		memcpy(mod_cfg->linear_table, param->isp_tunning_settings.linear_tbl,
			ISP_ARRAY_SIZE(param->isp_tunning_settings.linear_tbl) * (sizeof(unsigned short)));

	if (param->isp_test_settings.lsc_en) {
		config_lens_table(isp_gen, 512);
		config_lens_center(isp_gen);
	}

	memcpy(&mod_cfg->cem_cfg.cem_table[0], &param->isp_tunning_settings.isp_cem_table[0], ISP_CEM_MEM_SIZE);

	//memcpy(&mod_cfg->pltm_cfg.pltm_table[0], &param->isp_tunning_settings.isp_pltm_table[0], ISP_PLTM_MEM_SIZE);

	//memcpy(&mod_cfg->wdr_cfg.wdr_table[0], &param->isp_tunning_settings.isp_wdr_table[0], ISP_WDR_MEM_SIZE);

	FUNCTION_LOG;
}

int __isp_alloc_reg(struct isp_lib_context *isp_gen)
{
	unsigned int isp_default_reg[0x100] = {
		0x00000101, 0x00000001, 0x00004111, 0x00000087,
		0x03c00010, 0x00000000, 0x28000000, 0x04000000,
		0x0dc11000, 0x0dc11400, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x80000200, 0x00000004, 0x00000000, 0x0136003c,
		0x00000106, 0x00005040, 0x00000000, 0x00000000,
		0x00000000, 0x000f0013, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x04380780, 0x04380780,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x0f000200, 0x01390010,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x04000804, 0x00000000, 0x00000000, 0x00000000,
		0x00021010, 0x00000000, 0x00000000, 0x00000000,
		0x00400010, 0x01000100, 0x00200020, 0x00000100,
		0x00200020, 0x00200020, 0x04000400, 0x04000400,
		0x00200020, 0x00200020, 0x00ff00ff, 0x000000ff,
		0x000f0013, 0x00000000, 0x00000000, 0x00000000,
		0x00080008, 0x00000000, 0x00000000, 0x00000000,
		0x40070f01, 0xfcff0080, 0x1f173c2d, 0x001845c8,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x01000100, 0x01000100, 0x00000fff, 0x00000010,
		0x30000000, 0x00000080, 0x0003875c, 0x00400010,
		0x02000200, 0x04000400, 0x00000000, 0x00000484,
		0x00000808, 0x00420077, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x02040107, 0x07680064, 0x01c206d6, 0x068701c2,
		0x000007b7, 0x02010010, 0x00000008, 0x00000000,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00000000, 0x00000000, 0x00000000,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x02108421, 0x02108421,
		0x02108421, 0x02108421, 0x02108421, 0x02108421,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00000000, 0x00000000,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00080008, 0x00080008,
		0x00080008, 0x00080008, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
		0x00000000, 0x00000000, 0x00000000, 0x00000000,
	};

	/* REG LOAD*/
	isp_gen->load_reg_base = malloc(ISP_LOAD_REG_SIZE);
	if(isp_gen->load_reg_base == NULL) {
		ISP_ERR("load_reg_base alloc failed, no memory!");
		return -1;
	}
	memset(isp_gen->load_reg_base, 0, ISP_LOAD_REG_SIZE);
	memcpy(isp_gen->load_reg_base, &isp_default_reg[0], 0x400);

	isp_gen->module_cfg.isp_dev_id = isp_gen->isp_id;
	isp_gen->module_cfg.isp_platform_id = ISP_PLATFORM_SUN8IW12P1;

	isp_map_addr(&isp_gen->module_cfg, (unsigned long)isp_gen->load_reg_base);

	return 0;
}

int __isp_alloc_tbl(struct isp_lib_context *isp_gen)
{
	/* LINEAR TABLE */
	isp_gen->module_cfg.table_mapping1 = malloc(ISP_TABLE_MAPPING1_SIZE);
	if(isp_gen->module_cfg.table_mapping1 == NULL) {
		ISP_ERR("linear lens gamma ... table alloc failed, no memory!");
		return -1;
	}
	memset(isp_gen->module_cfg.table_mapping1, 0, ISP_TABLE_MAPPING1_SIZE);

	isp_gen->module_cfg.lens_table = isp_gen->module_cfg.table_mapping1 + ISP_LSC_MEM_OFS;
	isp_gen->module_cfg.gamma_table = isp_gen->module_cfg.table_mapping1 + ISP_GAMMA_MEM_OFS;
	isp_gen->module_cfg.linear_table = isp_gen->module_cfg.table_mapping1 + ISP_LINEAR_MEM_OFS;
	isp_gen->module_cfg.wdr_table = isp_gen->module_cfg.table_mapping1 + ISP_WDR_GAMMA_FE_MEM_OFS;
	isp_gen->module_cfg.tdnf_diff_table = isp_gen->module_cfg.table_mapping1 + ISP_TDNF_DIFF_MEM_OFS;
	isp_gen->module_cfg.pltm_table = isp_gen->module_cfg.table_mapping1 + ISP_PLTM_H_MEM_OFS;
	isp_gen->module_cfg.contrast_table = isp_gen->module_cfg.table_mapping1 + ISP_CONTRAST_PE_MEM_OFS;

	/* DRC TABLE */
	isp_gen->module_cfg.table_mapping2 = malloc(ISP_TABLE_MAPPING2_SIZE);
	if(isp_gen->module_cfg.table_mapping2 == NULL) {
		ISP_ERR("drc saturation cem ... table alloc failed, no memory!");
		return -1;
	}
	memset(isp_gen->module_cfg.table_mapping2, 0, ISP_TABLE_MAPPING2_SIZE);

	isp_gen->module_cfg.drc_table = isp_gen->module_cfg.table_mapping2 + ISP_DRC_MEM_OFS;
	isp_gen->module_cfg.saturation_table = isp_gen->module_cfg.table_mapping2 + ISP_SATURATION_MEM_OFS;
	isp_gen->module_cfg.cem_table = isp_gen->module_cfg.table_mapping2 + ISP_CEM_MEM_OFS;

	return 0;
}
int __isp_dump_reg(struct isp_lib_context *isp_gen)
{
	int i, *reg;
	printf("dump ISP%d regs :\n", isp_gen->isp_id);
	for(i = 0; i < 0x40; i = i + 4)
	{
		reg = (int*)(isp_gen->load_reg_base + i);
		if(i % 0x10 == 0)
			printf("0x%08x:  ", i);
		printf("0x%08x, ", reg[0]);
		if(i % 0x10 == 0xc)
			printf("\n");
	}
	for(i = 0x40; i < 0x400; i = i + 4)
	{
		reg = (int*)(isp_gen->load_reg_base + i);

		if(i % 0x10 == 0)
			printf("0x%08x:  ", i);
		printf("0x%08x, ", reg[0]);
		if(i % 0x10 == 0xc)
			printf("\n");
	}

	return 0;
}

HW_S32 isp_ctx_stats_prepare(struct isp_lib_context *isp_gen, void *buffer)
{	FUNCTION_LOG;

	pthread_mutex_lock(&(isp_gen->ctx_lock));
	isp_handle_stats(isp_gen, buffer);
	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}
HW_S32 isp_ctx_stats_prepare_sync(struct isp_lib_context *isp_gen, void *buffer)
{	FUNCTION_LOG;

	pthread_mutex_lock(&(isp_gen->ctx_lock));
	isp_handle_stats_sync(isp_gen, buffer);
	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}


HW_S32 isp_ctx_stats_req(struct isp_lib_context *isp_gen,
					struct isp_stats_context *stats_ctx)
{	FUNCTION_LOG;

	pthread_mutex_lock(&(isp_gen->ctx_lock));
	*stats_ctx = isp_gen->stats_ctx;
	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}

HW_S32 __isp_ctx_update_iso_cfg(struct isp_lib_context *isp_gen)
{
	isp_iso_entity_context_t *isp_iso_cxt = &isp_gen->iso_entity_ctx;

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.isp_gen = isp_gen;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_REG_LIB_CTX);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.param =
				&isp_gen->isp_ini_cfg.isp_iso_settings;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_UPDATE_PARAMS);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.cnr_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_CNR);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.sharpness_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_SHARP);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.saturation_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_SATUR);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.contrast_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_CONTR);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.brightness_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_BRIGH);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.cem_ratio_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_CEM_RATIO);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.denoise_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_DENOISE);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.sensor_offset_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_SENSOR_OFFSET);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.black_level_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_BLACK_LEVEL);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.dpc_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_DPC);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.defog_value_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_DEFOG_VALUE);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.pltm_dynamic_cfg_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_PLTM_DYNAMIC_CFG);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.tdnr_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_TDNR);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.ae_cfg_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_AE_CFG);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.gtm_cfg_adjust = true;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_ADJUST_GTM_CFG);

	clear(isp_gen->iso_entity_ctx.iso_param);
	isp_gen->iso_entity_ctx.iso_param.u.test_cfg.isp_test_mode =
		isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_TEST_CONFIG);

	isp_iso_cxt->ops->isp_iso_run(isp_iso_cxt->iso_entity,
					&isp_iso_cxt->iso_result);

	return 0;
}

HW_S32 __isp_ctx_update_af_cfg(struct isp_lib_context *isp_gen)
{
	//af_ini_cfg.
	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_use_otp = isp_gen->isp_ini_cfg.isp_3a_settings.af_use_otp;
	isp_gen->af_entity_ctx.af_param.u.af_ini.vcm_min_code = isp_gen->isp_ini_cfg.isp_3a_settings.vcm_min_code;
	isp_gen->af_entity_ctx.af_param.u.af_ini.vcm_max_code = isp_gen->isp_ini_cfg.isp_3a_settings.vcm_max_code;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_interval_time = isp_gen->isp_ini_cfg.isp_3a_settings.af_interval_time;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_speed_ind = isp_gen->isp_ini_cfg.isp_3a_settings.af_speed_ind;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_auto_fine_en = isp_gen->isp_ini_cfg.isp_3a_settings.af_auto_fine_en;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_single_fine_en = isp_gen->isp_ini_cfg.isp_3a_settings.af_single_fine_en;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_fine_step = isp_gen->isp_ini_cfg.isp_3a_settings.af_fine_step;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_move_cnt = isp_gen->isp_ini_cfg.isp_3a_settings.af_move_cnt;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_still_cnt = isp_gen->isp_ini_cfg.isp_3a_settings.af_still_cnt;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_move_monitor_cnt = isp_gen->isp_ini_cfg.isp_3a_settings.af_move_monitor_cnt;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_still_monitor_cnt = isp_gen->isp_ini_cfg.isp_3a_settings.af_still_monitor_cnt;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_stable_min = isp_gen->isp_ini_cfg.isp_3a_settings.af_stable_min;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_stable_max = isp_gen->isp_ini_cfg.isp_3a_settings.af_stable_max;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_low_light_lv = isp_gen->isp_ini_cfg.isp_3a_settings.af_low_light_lv;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_near_tolerance = isp_gen->isp_ini_cfg.isp_3a_settings.af_near_tolerance;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_far_tolerance = isp_gen->isp_ini_cfg.isp_3a_settings.af_far_tolerance;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_tolerance_off = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_off;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_peak_th = isp_gen->isp_ini_cfg.isp_3a_settings.af_peak_th;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_dir_th = isp_gen->isp_ini_cfg.isp_3a_settings.af_dir_th;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_change_ratio = isp_gen->isp_ini_cfg.isp_3a_settings.af_change_ratio;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_move_minus = isp_gen->isp_ini_cfg.isp_3a_settings.af_move_minus;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_still_minus = isp_gen->isp_ini_cfg.isp_3a_settings.af_still_minus;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_scene_motion_th = isp_gen->isp_ini_cfg.isp_3a_settings.af_scene_motion_th;
	isp_gen->af_entity_ctx.af_param.u.af_ini.af_tolerance_tbl_len = isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_tbl_len;
	memcpy(&isp_gen->af_entity_ctx.af_param.u.af_ini.af_std_code_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.af_std_code_tbl[0], 20*sizeof(int));
	memcpy(&isp_gen->af_entity_ctx.af_param.u.af_ini.af_tolerance_value_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.af_tolerance_value_tbl[0], 20*sizeof(int));
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_INI_DATA);

	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.vcm.vcm_max_code = isp_gen->isp_ini_cfg.isp_3a_settings.vcm_max_code;
	isp_gen->af_entity_ctx.af_param.u.vcm.vcm_min_code = isp_gen->isp_ini_cfg.isp_3a_settings.vcm_min_code;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_VCM_PARAM);

	clear(isp_gen->af_entity_ctx.af_param);
	isp_gen->af_entity_ctx.af_param.u.test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_gen->af_entity_ctx.af_param.u.test_cfg.isp_test_focus = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_focus;
	isp_gen->af_entity_ctx.af_param.u.test_cfg.focus_start = isp_gen->isp_ini_cfg.isp_test_settings.focus_start;
	isp_gen->af_entity_ctx.af_param.u.test_cfg.focus_step = isp_gen->isp_ini_cfg.isp_test_settings.focus_step;
	isp_gen->af_entity_ctx.af_param.u.test_cfg.focus_end = isp_gen->isp_ini_cfg.isp_test_settings.focus_end;
	isp_gen->af_entity_ctx.af_param.u.test_cfg.focus_change_interval = isp_gen->isp_ini_cfg.isp_test_settings.focus_change_interval;
	isp_gen->af_entity_ctx.af_param.u.test_cfg.af_en = isp_gen->isp_ini_cfg.isp_test_settings.af_en;
	isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_TEST_CONFIG);

	return 0;
}
HW_S32 __isp_ctx_update_awb_cfg(struct isp_lib_context *isp_gen)
{
	//awb_ini_cfg.
	clear(isp_gen->awb_entity_ctx.awb_param);
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_interval = isp_gen->isp_ini_cfg.isp_3a_settings.awb_interval;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_speed = isp_gen->isp_ini_cfg.isp_3a_settings.awb_speed;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_color_temper_low = isp_gen->isp_ini_cfg.isp_3a_settings.awb_color_temper_low;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_color_temper_high = isp_gen->isp_ini_cfg.isp_3a_settings.awb_color_temper_high;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_base_temper = isp_gen->isp_ini_cfg.isp_3a_settings.awb_base_temper;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_green_zone_dist = isp_gen->isp_ini_cfg.isp_3a_settings.awb_green_zone_dist;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_blue_sky_dist = isp_gen->isp_ini_cfg.isp_3a_settings.awb_blue_sky_dist;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_light_num = isp_gen->isp_ini_cfg.isp_3a_settings.awb_light_num;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_ext_light_num = isp_gen->isp_ini_cfg.isp_3a_settings.awb_ext_light_num;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_skin_color_num = isp_gen->isp_ini_cfg.isp_3a_settings.awb_skin_color_num;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_special_color_num = isp_gen->isp_ini_cfg.isp_3a_settings.awb_special_color_num;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_rgain_favor = isp_gen->isp_ini_cfg.isp_3a_settings.awb_rgain_favor;
	isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_bgain_favor = isp_gen->isp_ini_cfg.isp_3a_settings.awb_bgain_favor;
	memcpy(&isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_light_info[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_light_info[0], 320*sizeof(int));
	memcpy(&isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_ext_light_info[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_ext_light_info[0], 320*sizeof(int));
	memcpy(&isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_skin_color_info[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_skin_color_info[0], 160*sizeof(int));
	memcpy(&isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_special_color_info[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_special_color_info[0], 320*sizeof(int));
	memcpy(&isp_gen->awb_entity_ctx.awb_param.u.awb_ini.awb_preset_gain[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.awb_preset_gain[0], 22*sizeof(int));

	isp_awb_set_params_helper(&isp_gen->awb_entity_ctx, ISP_AWB_INI_DATA);

	return 0;
}
HW_S32 __isp_ctx_update_ae_cfg(struct isp_lib_context *isp_gen)
{
	//ae_sensor_info.
	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_sensor_info = isp_gen->sensor_info;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_SENSOR_INFO);
	//ae_ini_cfg.
	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.define_ae_table = isp_gen->isp_ini_cfg.isp_3a_settings.define_ae_table;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_max_lv = isp_gen->isp_ini_cfg.isp_3a_settings.ae_max_lv;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].length = isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_preview_length;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].length = isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_capture_length;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].length = isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_video_length;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_hist_mod_en = isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist_mod_en;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_ki = isp_gen->isp_ini_cfg.isp_3a_settings.ae_ki;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_ConvDataIndex = isp_gen->isp_ini_cfg.isp_3a_settings.ae_ConvDataIndex;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_blowout_pre_en = isp_gen->isp_ini_cfg.isp_3a_settings.ae_blowout_pre_en;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_blowout_attr = isp_gen->isp_ini_cfg.isp_3a_settings.ae_blowout_attr;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.ae_delay_frame;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.exp_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.exp_delay_frame;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.gain_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.gain_delay_frame;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.exp_comp_step = isp_gen->isp_ini_cfg.isp_3a_settings.exp_comp_step;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_touch_dist_ind = isp_gen->isp_ini_cfg.isp_3a_settings.ae_touch_dist_ind;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_iso2gain_ratio = isp_gen->isp_ini_cfg.isp_3a_settings.ae_iso2gain_ratio;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_gain_favor = isp_gen->isp_ini_cfg.isp_3a_settings.ae_gain_favor;

	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_fno_step[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_fno_step[0], 16*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.wdr_cfg[0], &isp_gen->isp_ini_cfg.isp_3a_settings.wdr_cfg[0], 3*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_preview[0], 42*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].ae_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_capture[0], 42*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].ae_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_video[0], 42*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_win_weight[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_win_weight[0], 64*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_gain_range[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_gain_range[0], 4*sizeof(int));

	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_INI_DATA);

	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_setting = isp_gen->ae_settings;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_SETTINGS);
	//test cfg.

	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_test_mode = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_mode;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_gain = isp_gen->isp_ini_cfg.isp_test_settings.isp_gain;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_exp_line = isp_gen->isp_ini_cfg.isp_test_settings.isp_exp_line;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.ae_forced = isp_gen->isp_ini_cfg.isp_test_settings.ae_forced;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.lum_forced = isp_gen->isp_ini_cfg.isp_test_settings.lum_forced;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_test_exptime = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_exptime;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.exp_line_start = isp_gen->isp_ini_cfg.isp_test_settings.exp_line_start;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.exp_line_step = isp_gen->isp_ini_cfg.isp_test_settings.exp_line_step;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.exp_line_end = isp_gen->isp_ini_cfg.isp_test_settings.exp_line_end;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.exp_change_interval = isp_gen->isp_ini_cfg.isp_test_settings.exp_change_interval;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.isp_test_gain = isp_gen->isp_ini_cfg.isp_test_settings.isp_test_gain;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.gain_start = isp_gen->isp_ini_cfg.isp_test_settings.gain_start;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.gain_step = isp_gen->isp_ini_cfg.isp_test_settings.gain_step;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.gain_end = isp_gen->isp_ini_cfg.isp_test_settings.gain_end;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.gain_change_interval = isp_gen->isp_ini_cfg.isp_test_settings.gain_change_interval;
	isp_gen->ae_entity_ctx.ae_param.u.test_cfg.ae_en = isp_gen->isp_ini_cfg.isp_test_settings.ae_en;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_TEST_CONFIG);

	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_UPDATE_AE_TABLE);

	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_target_comp = isp_gen->pltm_entity_ctx.pltm_result.pltm_ae_comp;
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_PLTM_COMP);

	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_ae_get_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_INI_DATA);
	isp_gen->tune.gains.gain_favor = isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_gain_favor;
	isp_gen->tune.gains.ana_gain_min = isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_gain_range[0];
	isp_gen->tune.gains.ana_gain_max = isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_gain_range[1];
	isp_gen->tune.gains.dig_gain_min = isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_gain_range[2];
	isp_gen->tune.gains.dig_gain_max = isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_gain_range[3];

	return 0;
}

HW_S32 __isp_ctx_update_afs_cfg(struct isp_lib_context *isp_gen)
{
	return 0;
}

HW_S32 __isp_ctx_update_md_cfg(struct isp_lib_context *isp_gen)
{
	return 0;
}

HW_S32 __isp_ctx_update_rolloff_cfg(struct isp_lib_context *isp_gen)
{
	//rolloff_sensor_info.
	clear(isp_gen->rolloff_entity_ctx.rolloff_param);
	isp_gen->rolloff_entity_ctx.rolloff_param.u.rolloff_sensor_info = isp_gen->sensor_info;
	isp_rolloff_set_params_helper(&isp_gen->rolloff_entity_ctx, ISP_ROLLOFF_SENSOR_INFO);

	//rolloff_ini_cfg.
	clear(isp_gen->rolloff_entity_ctx.rolloff_param);
	memcpy(&isp_gen->rolloff_entity_ctx.rolloff_param.u.rolloff_ini.lens_table_ini[0],
		&isp_gen->isp_ini_cfg.isp_tunning_settings.lsc_tbl[0][3], 768*sizeof(unsigned short));
	isp_gen->rolloff_entity_ctx.rolloff_param.u.rolloff_ini.rolloff_ratio =
			isp_gen->isp_ini_cfg.isp_tunning_settings.rolloff_ratio;
	isp_rolloff_set_params_helper(&isp_gen->rolloff_entity_ctx, ISP_ROLLOFF_INI_DATA);

	//rolloff lsc_cfg.
	clear(isp_gen->rolloff_entity_ctx.rolloff_param);
	isp_gen->rolloff_entity_ctx.rolloff_param.u.lsc_cfg = isp_gen->module_cfg.lens_cfg.lsc_cfg;
	isp_rolloff_set_params_helper(&isp_gen->rolloff_entity_ctx, ISP_ROLLOFF_LSC_CONFIG);

	return 0;
}
HW_S32 __isp_ctx_update_gtm_cfg(struct isp_lib_context *isp_gen)
{
	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.gamma_tbl =	&isp_gen->module_cfg.gamma_cfg.gamma_tbl[0];
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_GAMMA_TBL);

	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.drc_table =	&isp_gen->module_cfg.drc_cfg.drc_table[0];
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_DRC_TBL);

	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.drc_table_last = &isp_gen->module_cfg.drc_cfg.drc_table_last[0];
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_DRC_TBL_LAST);

	//gtm_ini_cfg.
	clear(isp_gen->gtm_entity_ctx.gtm_param);
	isp_gen->gtm_entity_ctx.gtm_param.u.gtm_ini.gtm_type = isp_gen->isp_ini_cfg.isp_tunning_settings.gtm_type;
	isp_gen->gtm_entity_ctx.gtm_param.u.gtm_ini.gamma_type = isp_gen->isp_ini_cfg.isp_tunning_settings.gamma_type;
	isp_gen->gtm_entity_ctx.gtm_param.u.gtm_ini.AutoAlphaEn	= isp_gen->isp_ini_cfg.isp_tunning_settings.auto_alpha_en;
	memcpy(&isp_gen->gtm_entity_ctx.gtm_param.u.gtm_ini.gtm_cfg[0], &isp_gen->ae_settings.ae_hist_eq_cfg[0], GTM_HEQ_MAX*sizeof(int));
	isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_INI_DATA);

	return 0;
}

HW_S32 __isp_ctx_update_pltm_cfg(struct isp_lib_context *isp_gen)
{
	//pltm sensor_info.
	clear(isp_gen->pltm_entity_ctx.pltm_param);
	isp_gen->pltm_entity_ctx.pltm_param.u.sensor_info = isp_gen->sensor_info;
	isp_pltm_set_params_helper(&isp_gen->pltm_entity_ctx, ISP_PLTM_SENSOR_INFO);

	//pltm table
	clear(isp_gen->pltm_entity_ctx.pltm_param);
	isp_gen->pltm_entity_ctx.pltm_param.u.pltm_tbl = (HW_U16 *)isp_gen->isp_ini_cfg.isp_tunning_settings.isp_pltm_table;
	isp_pltm_set_params_helper(&isp_gen->pltm_entity_ctx, ISP_PLTM_TBL);

	//pltm_ini_cfg.
	clear(isp_gen->pltm_entity_ctx.pltm_param);
	memcpy(&isp_gen->pltm_entity_ctx.pltm_param.u.pltm_ini.pltm_cfg[0],
		&isp_gen->isp_ini_cfg.isp_tunning_settings.pltm_cfg[0], ISP_PLTM_MAX*sizeof(HW_S32));
	memcpy(&isp_gen->pltm_entity_ctx.pltm_param.u.pltm_ini.pltm_dynamic_cfg[0],
		&isp_gen->ae_settings.pltm_dynamic_cfg[0], ISP_PLTM_DYNAMIC_MAX*sizeof(HW_S32));
	isp_pltm_set_params_helper(&isp_gen->pltm_entity_ctx, ISP_PLTM_INI_DATA);
	return 0;
}

void __isp_ctx_config(struct isp_lib_context *isp_gen)
{
	__isp_ctx_apply_enable(isp_gen);

	__isp_ctx_cfg_lib(isp_gen);

	__isp_ctx_cfg_mod(isp_gen);

	__isp_ctx_update_iso_cfg(isp_gen);

	__isp_ctx_update_af_cfg(isp_gen);

	__isp_ctx_update_ae_cfg(isp_gen);

	__isp_ctx_update_awb_cfg(isp_gen);

	__isp_ctx_update_gtm_cfg(isp_gen);

	__isp_ctx_update_pltm_cfg(isp_gen);

	__isp_ctx_update_afs_cfg(isp_gen);

	__isp_ctx_update_md_cfg(isp_gen);

	__isp_ctx_update_rolloff_cfg(isp_gen);
}

void __isp_ctx_update(struct isp_lib_context *isp_gen)
{
	__isp_ctx_apply_enable(isp_gen);

	__isp_ctx_cfg_mod(isp_gen);

	__isp_ctx_update_iso_cfg(isp_gen);

	__isp_ctx_update_af_cfg(isp_gen);

	__isp_ctx_update_ae_cfg(isp_gen);

	__isp_ctx_update_awb_cfg(isp_gen);

	__isp_ctx_update_gtm_cfg(isp_gen);

	__isp_ctx_update_pltm_cfg(isp_gen);

	__isp_ctx_update_afs_cfg(isp_gen);

	__isp_ctx_update_md_cfg(isp_gen);

	__isp_ctx_update_rolloff_cfg(isp_gen);
}

int isp_ctx_algo_init(struct isp_lib_context *isp_gen,
					const struct isp_ctx_operations *ops)
{
	//ISO init
	isp_gen->iso_entity_ctx.iso_entity = iso_init(&isp_gen->iso_entity_ctx.ops);

	if (isp_gen->iso_entity_ctx.iso_entity == NULL ||
					NULL == isp_gen->iso_entity_ctx.ops) {
		ISP_ERR("ISO Entity is BUSY or NULL!\n");
		return -1;
	} else {
		clear(isp_gen->iso_entity_ctx.iso_param);
		isp_gen->iso_entity_ctx.iso_param.u.isp_platform_id =
					isp_gen->module_cfg.isp_platform_id;
		isp_iso_set_params_helper(&isp_gen->iso_entity_ctx, ISP_ISO_PLATFORM_ID);
		//iso_ini_cfg.
	}
#if ISP_LIB_USE_AF
	//AF init
	isp_gen->af_entity_ctx.af_entity = af_init(&isp_gen->af_entity_ctx.ops);
	if (isp_gen->af_entity_ctx.af_entity == NULL ||
					NULL == isp_gen->af_entity_ctx.ops) {
		ISP_ERR("AF Entity is BUSY or NULL!\n");
		return -1;
	} else {
		clear(isp_gen->af_entity_ctx.af_param);
		isp_gen->af_entity_ctx.af_param.u.isp_platform_id =
					isp_gen->module_cfg.isp_platform_id;
		isp_af_set_params_helper(&isp_gen->af_entity_ctx, ISP_AF_PLATFORM_ID);
	}
#endif
	//AFS init
	isp_gen->afs_entity_ctx.afs_entity = afs_init(&isp_gen->afs_entity_ctx.ops);
	if (isp_gen->afs_entity_ctx.afs_entity == NULL ||
					NULL == isp_gen->afs_entity_ctx.ops) {
		ISP_ERR("AFS Entity is BUSY or NULL!\n");
		return -1;
	}

#if ISP_LIB_USE_MD
	//MD init
	isp_gen->md_entity_ctx.md_entity = md_init(&isp_gen->md_entity_ctx.ops);
	if (isp_gen->md_entity_ctx.md_entity == NULL ||
					NULL == isp_gen->md_entity_ctx.ops) {
		ISP_ERR("MD Entity is BUSY or NULL!\n");
		return -1;
	}
#endif

	//AWB init
	isp_gen->awb_entity_ctx.awb_entity = awb_init(&isp_gen->awb_entity_ctx.ops);
	if (isp_gen->awb_entity_ctx.awb_entity == NULL ||
					NULL == isp_gen->awb_entity_ctx.ops) {
		ISP_ERR("AWB Entity is BUSY or NULL!\n");
		return -1;
	} else {
		clear(isp_gen->awb_entity_ctx.awb_param);
		isp_gen->awb_entity_ctx.awb_param.u.isp_platform_id = isp_gen->module_cfg.isp_platform_id;
		isp_awb_set_params_helper(&isp_gen->awb_entity_ctx, ISP_AWB_PLATFORM_ID);
	}

	//AE init
	isp_gen->ae_entity_ctx.ae_entity = ae_init(&isp_gen->ae_entity_ctx.ops);
	if (isp_gen->ae_entity_ctx.ae_entity == NULL ||
					NULL == isp_gen->ae_entity_ctx.ops) {
		ISP_ERR("AE Entity is BUSY or NULL!\n");
		return -1;
	} else {
		clear(isp_gen->ae_entity_ctx.ae_param);
		isp_gen->ae_entity_ctx.ae_param.u.isp_platform_id =
						isp_gen->module_cfg.isp_platform_id;
		isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_PLATFORM_ID);
	}

	//GTM init
	isp_gen->gtm_entity_ctx.gtm_entity =
					gtm_init(&isp_gen->gtm_entity_ctx.ops);
	if (isp_gen->gtm_entity_ctx.gtm_entity == NULL
				|| NULL == isp_gen->gtm_entity_ctx.ops) {
		ISP_ERR("GTM Entity is BUSY or NULL!\n");
		return -1;
	} else {
		clear(isp_gen->gtm_entity_ctx.gtm_param);
		isp_gen->gtm_entity_ctx.gtm_param.u.isp_platform_id =
					isp_gen->module_cfg.isp_platform_id;
		isp_gtm_set_params_helper(&isp_gen->gtm_entity_ctx, ISP_GTM_PLATFORM_ID);
	}
	//PLTM init
	isp_gen->pltm_entity_ctx.pltm_entity =
					pltm_init(&isp_gen->pltm_entity_ctx.ops);
	if (isp_gen->pltm_entity_ctx.pltm_entity == NULL
				|| NULL == isp_gen->pltm_entity_ctx.ops) {
		ISP_ERR("PLTM Entity is BUSY or NULL!\n");
		return -1;
	} else {
		clear(isp_gen->pltm_entity_ctx.pltm_param);
		isp_gen->pltm_entity_ctx.pltm_param.u.isp_platform_id =
					isp_gen->module_cfg.isp_platform_id;
		isp_pltm_set_params_helper(&isp_gen->pltm_entity_ctx, ISP_PLTM_PLATFORM_ID);
	}

#if ISP_LIB_USE_ROLLOFF
	//ROLLOFF init
	isp_gen->rolloff_entity_ctx.rolloff_entity = rolloff_init(&isp_gen->rolloff_entity_ctx.ops);
	if (isp_gen->rolloff_entity_ctx.rolloff_entity == NULL ||
					NULL == isp_gen->rolloff_entity_ctx.ops) {
		ISP_ERR("ROLLOFF Entity is BUSY or NULL!\n");
		return -1;
	} else {
		clear(isp_gen->rolloff_entity_ctx.rolloff_param);
		isp_gen->rolloff_entity_ctx.rolloff_param.u.isp_platform_id =
						isp_gen->module_cfg.isp_platform_id;
		isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_ROLLOFF_PLATFORM_ID);
	}
#endif
	isp_gen->ops = ops;

	isp_gen->af_frame_cnt  = 0;
	isp_gen->ae_frame_cnt  = 0;
	isp_gen->awb_frame_cnt = 0;
	isp_gen->gtm_frame_cnt = 0;

	isp_gen->md_frame_cnt  = 0;
	isp_gen->afs_frame_cnt  = 0;
	isp_gen->iso_frame_cnt = 0;
	isp_gen->rolloff_frame_cnt = 0;
	isp_gen->alg_frame_cnt = 0;

	if (__isp_alloc_reg(isp_gen))
		return -1;
	if (__isp_alloc_tbl(isp_gen))
		return -1;
	pthread_mutex_init(&(isp_gen->ctx_lock), NULL);
	return 0;
}

HW_S32 isp_ctx_algo_run(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));

	isp_get_saved_regs(isp_gen);

	isp_apply_settings(isp_gen);

	__isp_ae_set_params(isp_gen);
	__isp_ae_run(isp_gen);

	__isp_iso_set_params(isp_gen);
	__isp_iso_run(isp_gen);

	__isp_gtm_set_params(isp_gen);
	__isp_gtm_run(isp_gen);

	__isp_pltm_set_params(isp_gen);
	__isp_pltm_run(isp_gen);

	__isp_afs_set_params(isp_gen);
	__isp_afs_run(isp_gen);

#if ISP_LIB_USE_MD
	__isp_md_set_params(isp_gen);
	__isp_md_run(isp_gen);
#endif

#if ISP_LIB_USE_AF
	__isp_af_set_params(isp_gen);
	__isp_af_run(isp_gen);
#endif
	//__isp_flash_isr(isp_gen, isp_result);

	__isp_awb_set_params(isp_gen);
	__isp_awb_run(isp_gen);

	if (isp_gen->isp_ini_cfg.isp_test_settings.lsc_en) {
#if ISP_LIB_USE_ROLLOFF
		__isp_rolloff_set_params(isp_gen);
		__isp_rolloff_run(isp_gen);
#else
		config_lens_table(isp_gen, isp_gen->af_entity_ctx.af_result.std_code_output);
#endif
	}

	isp_apply_colormatrix(isp_gen);

	if (isp_gen->isp_ini_cfg.isp_test_settings.defog_en==1) {
		isp_gen->module_cfg.cfa_cfg.min_rgb    = 1023;
	} else {
		isp_gen->module_cfg.cfa_cfg.min_rgb    = 0;
	}
	FUNCTION_LOG;

	isp_hardware_update(&isp_gen->module_cfg);

	isp_gen->awb_frame_cnt++;
	isp_gen->ae_frame_cnt++;
	isp_gen->af_frame_cnt++;
	isp_gen->alg_frame_cnt ++;
	isp_gen->gtm_frame_cnt++;
	isp_gen->md_frame_cnt++;
	isp_gen->afs_frame_cnt ++;
	isp_gen->iso_frame_cnt++;
	isp_gen->rolloff_frame_cnt++;
	FUNCTION_LOG;
	//if (isp_gen->awb_frame_cnt % 10 == 0)
	//	__isp_dump_reg(isp_gen);
	pthread_mutex_unlock(&(isp_gen->ctx_lock));
	FUNCTION_LOG;
	return 0;
}

int isp_ctx_algo_exit(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));
#if ISP_LIB_USE_AF
	af_exit(isp_gen->af_entity_ctx.af_entity);
#endif
	afs_exit(isp_gen->afs_entity_ctx.afs_entity);
#if ISP_LIB_USE_MD
	md_exit(isp_gen->md_entity_ctx.md_entity);
#endif
	awb_exit(isp_gen->awb_entity_ctx.awb_entity);
	ae_exit(isp_gen->ae_entity_ctx.ae_entity);
	gtm_exit(isp_gen->gtm_entity_ctx.gtm_entity);
	pltm_exit(isp_gen->pltm_entity_ctx.pltm_entity);
	iso_exit(isp_gen->iso_entity_ctx.iso_entity);
#if ISP_LIB_USE_ROLLOFF
	rolloff_exit(isp_gen->rolloff_entity_ctx.rolloff_entity);
#endif
	if(isp_gen->load_reg_base != NULL) {
		free(isp_gen->load_reg_base);
		isp_gen->load_reg_base = NULL;
	}
	if(isp_gen->module_cfg.table_mapping1 != NULL) {
		free(isp_gen->module_cfg.table_mapping1);
		isp_gen->module_cfg.table_mapping1 = NULL;
	}
	if(isp_gen->module_cfg.table_mapping2!= NULL) {
		free(isp_gen->module_cfg.table_mapping2);
		isp_gen->module_cfg.table_mapping2 = NULL;
	}
	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	pthread_mutex_destroy(&(isp_gen->ctx_lock));

	return 0;
}

HW_S32 isp_ctx_update_ae_tbl(struct isp_lib_context *isp_gen, int sensor_fps)
{
	static int tbl_cnt = 0;

	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.define_ae_table = isp_gen->isp_ini_cfg.isp_3a_settings.define_ae_table;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_max_lv = isp_gen->isp_ini_cfg.isp_3a_settings.ae_max_lv;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].length = isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_preview_length;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].length = isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_capture_length;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].length = isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_video_length;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_hist_mod_en = isp_gen->isp_ini_cfg.isp_3a_settings.ae_hist_mod_en;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_ki = isp_gen->isp_ini_cfg.isp_3a_settings.ae_ki;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_ConvDataIndex = isp_gen->isp_ini_cfg.isp_3a_settings.ae_ConvDataIndex;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_blowout_pre_en = isp_gen->isp_ini_cfg.isp_3a_settings.ae_blowout_pre_en;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_blowout_attr = isp_gen->isp_ini_cfg.isp_3a_settings.ae_blowout_attr;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.ae_delay_frame;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.exp_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.exp_delay_frame;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.gain_delay_frame = isp_gen->isp_ini_cfg.isp_3a_settings.gain_delay_frame;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.exp_comp_step = isp_gen->isp_ini_cfg.isp_3a_settings.exp_comp_step;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_touch_dist_ind = isp_gen->isp_ini_cfg.isp_3a_settings.ae_touch_dist_ind;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_iso2gain_ratio = isp_gen->isp_ini_cfg.isp_3a_settings.ae_iso2gain_ratio;
	isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_gain_favor = isp_gen->isp_ini_cfg.isp_3a_settings.ae_gain_favor;
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_gain_range[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_gain_range[0], 4*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_fno_step[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_fno_step[0], 16*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.wdr_cfg[0], &isp_gen->isp_ini_cfg.isp_3a_settings.wdr_cfg[0], 3*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_preview[0], 42*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_CAPTURE].ae_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_capture[0], 42*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_VIDEO].ae_tbl[0],
		&isp_gen->isp_ini_cfg.isp_3a_settings.ae_table_video[0], 42*sizeof(int));
	memcpy(&isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_win_weight[0], &isp_gen->isp_ini_cfg.isp_3a_settings.ae_win_weight[0], 64*sizeof(int));

	for(tbl_cnt = 0; tbl_cnt < 10; tbl_cnt++) {
		if(isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[tbl_cnt].max_exp < sensor_fps)
			isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[tbl_cnt].max_exp = sensor_fps;
		if(isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[tbl_cnt].min_exp < sensor_fps)
			isp_gen->ae_entity_ctx.ae_param.u.ae_ini.ae_tbl_scene[SCENE_MODE_PREVIEW].ae_tbl[tbl_cnt].min_exp = sensor_fps;
	}

	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_INI_DATA);
	clear(isp_gen->ae_entity_ctx.ae_param);
	isp_ae_set_params_helper(&isp_gen->ae_entity_ctx, ISP_AE_UPDATE_AE_TABLE);

	return 0;
}

int isp_ctx_config_update(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));

	__isp_ctx_update(isp_gen);

	pthread_mutex_unlock(&(isp_gen->ctx_lock));

	return 0;
}

int isp_ctx_config_reset(struct isp_lib_context *isp_gen)
{
	isp_ctx_config_init(isp_gen);
	return 0;
}

int isp_ctx_config_init(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));
	if (isp_gen->sensor_info.sensor_height == 0) {
		pthread_mutex_unlock(&(isp_gen->ctx_lock));
		ISP_ERR("sensor attribute is not init.\n");
		return -1;
	}
	FUNCTION_LOG;

	__isp_ctx_config(isp_gen);

	//for isp_apply_settings
	isp_gen->isp_3a_change_flags = 0xffffffff;
	isp_gen->isp_3a_change_flags &= ~ISP_SET_BRIGHTNESS;
	isp_gen->isp_3a_change_flags &= ~ISP_SET_CONTRAST;
	isp_gen->isp_3a_change_flags &= ~ISP_SET_GAIN_STR;

	isp_hardware_update(&isp_gen->module_cfg);

	memset(&isp_gen->defog_ctx.min_rgb_pre[0], 0, 8*sizeof(int));
	pthread_mutex_unlock(&(isp_gen->ctx_lock));
	FUNCTION_LOG;

	return 0;
}

HW_S32 isp_ctx_config_exit(struct isp_lib_context *isp_gen)
{
	pthread_mutex_lock(&(isp_gen->ctx_lock));
	pthread_mutex_unlock(&(isp_gen->ctx_lock));
	return 0;
}

