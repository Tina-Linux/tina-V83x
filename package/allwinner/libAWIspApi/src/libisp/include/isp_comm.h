
/*
 ******************************************************************************
 *
 * isp_comm.h
 *
 * Hawkview ISP - isp_comm.h module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/03/31	VIDEO INPUT
 *
 *****************************************************************************
 */

#ifndef __BSP__ISP__COMM__H
#define __BSP__ISP__COMM__H
#include "isp_type.h"

#define ALIGN_4K(x)	(((x) + (4095)) & ~(4095))
#define ALIGN_32B(x)	(((x) + (31)) & ~(31))
#define ALIGN_16B(x)	(((x) + (15)) & ~(15))

#define ISP_REG_TBL_LENGTH	33
#define ISP_REG_TBL_NODE_LENGTH	9
#define ISP_GAMMA_TBL_LENGTH	(3*256)

//stat size configs

#define ISP_STAT_TOTAL_SIZE         0xAB00

#define ISP_STAT_HIST_MEM_SIZE      0x0200
#define ISP_STAT_AE_MEM_SIZE        0x4800
#define ISP_STAT_AF_CNT_MEM_SIZE    0x0100
#define ISP_STAT_AF_VH_MEM_SIZE     0x0400
#define ISP_STAT_AFS_MEM_SIZE       0x0200
#define ISP_STAT_AWB_RGB_MEM_SIZE   0x4800
#define ISP_STAT_AWB_CNT_MEM_SIZE   0x0C00
#define ISP_STAT_PLTM_LST_MEM_SIZE  0x0600

#define ISP_STAT_HIST_MEM_OFS       0x0000
#define ISP_STAT_AE_MEM_OFS         0x0200
#define ISP_STAT_AF_MEM_OFS         0x4a00
#define ISP_STAT_AFS_MEM_OFS        0x4f00
#define ISP_STAT_AWB_MEM_OFS        0x5100
#define ISP_STAT_PLTM_LST_MEM_OFS   0xa500

//table size configs

#define ISP_TABLE_MAPPING1_SIZE		0x5a00
#define ISP_LSC_MEM_SIZE		(256*8)
#define ISP_GAMMA_MEM_SIZE		(256*4)
#define ISP_LINEAR_MEM_SIZE		(256*6)
#define ISP_WDR_GAMMA_FE_MEM_SIZE	(4096*2)
#define ISP_WDR_GAMMA_BE_MEM_SIZE	(4096*2)
#define ISP_TDNF_DIFF_MEM_SIZE		(256*1)

#define ISP_WDR_MEM_SIZE (ISP_WDR_GAMMA_FE_MEM_SIZE + ISP_WDR_GAMMA_BE_MEM_SIZE)

#define ISP_PLTM_H_MEM_SIZE		(256*1)
#define ISP_PLTM_V_MEM_SIZE		(256*1)
#define ISP_PLTM_POW_MEM_SIZE		(256*2)
#define ISP_PLTM_F_MEM_SIZE		(256*2)

#define ISP_PLTM_MEM_SIZE (ISP_PLTM_H_MEM_SIZE + ISP_PLTM_V_MEM_SIZE + \
				ISP_PLTM_POW_MEM_SIZE + ISP_PLTM_F_MEM_SIZE)

#define ISP_CONTRAST_PE_MEM_SIZE	(128*2)

#define ISP_TABLE_MAPPING2_SIZE		0x1f00
#define ISP_DRC_MEM_SIZE		(256*2)
#define ISP_SATURATION_MEM_SIZE		(256*2)
#define ISP_CEM_MEM_SIZE		(736*8)

#define ISP_LSC_MEM_OFS			0x0
#define ISP_GAMMA_MEM_OFS		0x0800
#define ISP_LINEAR_MEM_OFS		0x0c00
#define ISP_WDR_GAMMA_FE_MEM_OFS	0x1200
#define ISP_WDR_GAMMA_BE_MEM_OFS	0x3200
#define ISP_TDNF_DIFF_MEM_OFS		0x5200
#define ISP_PLTM_H_MEM_OFS		0x5300
#define ISP_PLTM_V_MEM_OFS		0x5400
#define ISP_PLTM_POW_MEM_OFS		0x5500
#define ISP_PLTM_F_MEM_OFS		0x5700
#define ISP_CONTRAST_PE_MEM_OFS		0x1200

#define ISP_DRC_MEM_OFS			0x0
#define ISP_SATURATION_MEM_OFS		0x0600
#define ISP_CEM_MEM_OFS			0x0800

#define ISP_REG_SIZE                0x1000
#define ISP_LOAD_REG_SIZE           0x1000
#define ISP_SAVED_REG_SIZE          0x1000


enum isp_raw_ch {
	ISP_RAW_CH_R =  0,
	ISP_RAW_CH_GR,
	ISP_RAW_CH_GB,
	ISP_RAW_CH_G,
	ISP_RAW_CH_MAX,
};
enum isp_sharp_cfg {
	ISP_SHARP_MIN_VAL     = 0,
	ISP_SHARP_MAX_VAL     = 1,
	ISP_SHARP_BLACK_LEVEL     = 2,
	ISP_SHARP_WHITE_LEVEL     = 3,
	ISP_SHARP_BLACK_CLIP     = 4,
	ISP_SHARP_WHITE_CLIP     = 5,
	ISP_SHARP_BLACK_GAIN     = 6,
	ISP_SHARP_BLACK_OFFSET     = 7,
	ISP_SHARP_WHITE_GAIN     = 8,
	ISP_SHARP_WHITE_OFFSET     = 9,
	ISP_SHARP_MAX,
};

enum isp_contrast_cfg {
	ISP_CONTRAST_MIN_VAL = 0,
	ISP_CONTRAST_MAX_VAL = 1,
	ISP_CONTRAST_BLACK_LEVEL = 2,
	ISP_CONTRAST_WHITE_LEVEL = 3,
	ISP_CONTRAST_BLACK_CLIP = 4,
	ISP_CONTRAST_WHITE_CLIP = 5,
	ISP_CONTRAST_PLAT_TH = 6,
	ISP_CONTRAST_BLACK_GAIN = 7,
	ISP_CONTRAST_BLACK_OFFSET = 8,
	ISP_CONTRAST_WHITE_GAIN = 9,
	ISP_CONTRAST_WHITE_OFFSET = 10,
	ISP_CONTRAST_MAX,
};

enum isp_denoise_cfg {
	ISP_DENOISE_BLACK_GAIN     = 0,
	ISP_DENOISE_BLACK_OFFSET = 1,
	ISP_DENOISE_WHITE_GAIN = 2,
	ISP_DENOISE_WHITE_OFFSET = 3,
	ISP_DENOISE_MAX,
};
enum sensor_offset {
	ISP_SO_R_OFFSET = 0,
	ISP_SO_GR_OFFSET = 1,
	ISP_SO_GB_OFFSET = 2,
	ISP_SO_B_OFFSET = 3,
	ISP_SO_MAX,
};
enum black_level {
	ISP_BLC_R_OFFSET = 0,
	ISP_BLC_GR_OFFSET = 1,
	ISP_BLC_GB_OFFSET = 2,
	ISP_BLC_B_OFFSET = 3,
	ISP_BLC_MAX,
};
enum dpc_cfg {
	ISP_DPC_TH_SLOP = 0,
	ISP_DPC_OTF_MIN_TH = 1,
	ISP_DPC_OTF_MAX_TH = 2,
	ISP_DPC_OTF_MODE = 3,
	ISP_DPC_MAX,
};
enum pltm_dynamic_cfg {
	ISP_PLTM_DYNAMIC_AUTO_STREN = 0,
	ISP_PLTM_DYNAMIC_MANUL_STREN = 1,
	ISP_PLTM_DYNAMIC_AE_COMP = 2,
	ISP_PLTM_DYNAMIC_MAX,
};


enum isp_tdf_cfg {
	ISP_TDF_NOISE_CLIP_RATIO = 0,
	ISP_TDF_DIFF_CLIP_RATIO = 1,
	ISP_TDF_K_3D_S = 2,
	ISP_TDF_DIFF_CAL_MODE = 3,
	ISP_TDF_BLACK_GAIN = 4,
	ISP_TDF_BLACK_OFFSET = 5,
	ISP_TDF_WHITE_GAIN = 6,
	ISP_TDF_WHITE_OFFSET = 7,
	ISP_TDF_REF_BLACK_GAIN = 8,
	ISP_TDF_REF_BLACK_OFFSET = 9,
	ISP_TDF_REF_WHITE_GAIN = 10,
	ISP_TDF_REF_WHITE_OFFSET = 11,

	ISP_TDF_MAX,
};

enum isp_ae_hist_cfg {
	ISP_AE_HIST_DARK_WEIGHT_MIN = 0,
	ISP_AE_HIST_DARK_WEIGHT_MAX = 1,
	ISP_AE_HIST_BRIGHT_WEIGHT_MIN = 2,
	ISP_AE_HIST_BRIGHT_WEIGHT_MAX = 3,
	ISP_AE_HIST_CFG_MAX,
};

enum isp_gtm_comm_cfg {
	ISP_GTM_GAIN = 0,
	ISP_GTM_EQ_RATIO = 1,
	ISP_GTM_EQ_SMOOTH = 2,
	ISP_GTM_BLACK = 3,
	ISP_GTM_WHITE = 4,
	ISP_GTM_BLACK_ALPHA = 5,
	ISP_GTM_WHITE_ALPHA = 6,
	ISP_GTM_GAMMA_IND = 7,

	ISP_GTM_GAMMA_PLUS = 8,
	ISP_GTM_HEQ_MAX,
};

enum isp_pltm_comm_cfg {
	ISP_PLTM_MODE = 0,

	ISP_PLTM_ORIPIC_RATIO = 1,
	ISP_PLTM_TR_ORDER = 2,
	ISP_PLTM_LAST_ORDER_RATIO = 3,
	ISP_PLTM_POW_TBL = 4,
	ISP_PLTM_F_TBL = 5,

	ISP_PLTM_LSS_SWITCH = 6,
	ISP_PLTM_LUM_RATIO = 7,
	ISP_PLTM_LP_HALO_RES = 8,
	ISP_PLTM_WHITE_LEVEL = 9,
	ISP_PLTM_SPATIAL_ASM = 10,
	ISP_PLTM_INTENS_ASYM = 11,
	ISP_PLTM_BLOCK_V_NUM = 12,
	ISP_PLTM_BLOCK_H_NUM = 13,
	ISP_PLTM_CONTRAST = 14,

	ISP_PLTM_MAX,
};


enum isp_saturation_cfg {
	ISP_SATURATION_SATU_R = 0,
	ISP_SATURATION_SATU_G = 1,
	ISP_SATURATION_SATU_B = 2,
	ISP_SATURATION_SATU_MODE = 3,
	ISP_SATURATION_SATU_TBL_SG1 = 4,
	ISP_SATURATION_SATU_TBL_SG2 = 5,
	ISP_SATURATION_SATU_TBL_TH = 6,

	ISP_SATURATION_MAX,
};

enum isp_platform {
	ISP_PLATFORM_SUN8IW6P1 =  0,
	ISP_PLATFORM_SUN8IW12P1,

	ISP_PLATFORM_NUM,
};
/*
 *  update table
 */
#define LINEAR_UPDATE	(1 << 3)
#define LENS_UPDATE	(1 << 4)
#define GAMMA_UPDATE	(1 << 5)
#define DRC_UPDATE	(1 << 6)
#define DISC_UPDATE	(1 << 7)
#define SATU_UPDATE	(1 << 8)
#define WDR_UPDATE	(1 << 9)
#define TDNF_UPDATE	(1 << 10)
#define PLTM_UPDATE	(1 << 11)
#define CEM_UPDATE	(1 << 12)
#define CONTRAST_UPDATE	(1 << 13)

#define TABLE_UPDATE_ALL 0xffffffff

/*
 *  ISP Module enable
 */

#define AE_EN		(1 << 0)
#define LC_EN           (1 << 1)
#define WDR_EN          (1 << 2)
#define OTF_DPC_EN      (1 << 3)
#define BDNF_EN         (1 << 4)
#define TDNF_EN         (1 << 5)
#define AWB_EN		(1 << 6)
#define WB_EN           (1 << 7)
#define LSC_EN          (1 << 8)
#define BGC_EN          (1 << 9)
#define SAP_EN          (1 << 10)
#define AF_EN           (1 << 11)
#define RGB2RGB_EN      (1 << 12)
#define RGB_DRC_EN      (1 << 13)
#define PLTM_EN         (1 << 14)
#define CEM_EN          (1 << 15)
#define AFS_EN          (1 << 16)
#define HIST_EN         (1 << 17)
#define BLC_EN          (1 << 18)
#define DG_EN           (1 << 19)
#define SO_EN           (1 << 20)
#define CTC_EN          (1 << 21)
#define CONTRAST_EN     (1 << 22)
#define CNR_EN          (1 << 23)
#define SATU_EN		(1 << 24)
#define SRC0_EN         (1 << 31)
#define ISP_MODULE_EN_ALL	(0xffffffff)

/*
 *  ISP interrupt enable
 */
#define FINISH_INT_EN		(1 << 0)
#define START_INT_EN		(1 << 1)
#define PARA_SAVE_INT_EN	(1 << 2)
#define PARA_LOAD_INT_EN	(1 << 3)
#define SRC0_FIFO_INT_EN	(1 << 4)
#define SRC1_FIFO_INT_EN	(1 << 5)
#define DISC_FIFO_INT_EN	(1 << 5) //for sun8iw8p1 is means DISC overflow HW_S32
#define ROT_FINISH_INT_EN	(1 << 6)
#define N_LINE_START_INT_EN	(1 << 7)

#define ISP_IRQ_EN_ALL	0xffffffff

struct isp_size {
	HW_U32 width;
	HW_U32 height;
};

struct coor {
	HW_U32 hor;
	HW_U32 ver;
};

struct isp_size_settings {
	struct coor ob_start;
	struct isp_size ob_black_size;
	struct isp_size ob_valid_size;
};

enum enable_flag {
	DISABLE    = 0,
	ENABLE     = 1,
};

/*
 *  ISP interrupt status
 */
enum isp_irq_status {
	FINISH_PD	= 0,
	START_PD	= 1,
	PARA_SAVE_PD	= 2,
	PARA_LOAD_PD	= 3,
	SRC0_FIFO_OF_PD	= 4,
	SRC1_FIFO_OF_PD	= 5,
	ROT_FINISH_PD	= 6,
};

enum isp_input_tables {
	LUT_LENS_GAMMA_TABLE = 0,
	DRC_TABLE = 1,
};

enum wdr_mode {
	DOL_WDR = 0,
	COMANDING_WDR = 1,
};

enum otf_dpc_mode {
	OTF_DPC_NORMAL = 0,
	OTF_DPC_STRONG = 1,
	OTF_DPC_PEPPER = 2,
};

enum saturation_mode {
	SATU_NORM_MODE = 0,
	SATU_STRONG_MODE = 1,
};

enum isp_hist_mode {
	AVG_MODE = 0,
	MIN_MODE = 1,
	MAX_MODE = 2,
};

enum isp_hist_src {
	HIST_AFTER_WDR = 0,
	HIST_AFTER_CNR = 1,
};

enum isp_cfa_mode {
	CFA_NORM_MODE   = 0,
	CFA_BW_MODE = 1,
};

enum isp_ae_mode {
	AE_BEFORE_WDR = 0,
	AE_AFTER_WDR = 1,
	AE_AFTER_CNR = 2,
};

enum isp_awb_mode {
	AWB_AFTER_WDR = 0,
	AWB_AFTER_PLTM = 1,
};

enum isp_dg_mode {
	DG_AFTER_WDR = 0,
	DG_AFTER_SO = 1,
};

enum isp_input_seq {
	ISP_BGGR = 4,
	ISP_RGGB = 5,
	ISP_GBRG = 6,
	ISP_GRBG = 7,
};

struct isp_ctc_config {
	HW_U16 ctc_th_max;
	HW_U16 ctc_th_min;
	HW_U16 ctc_th_slope;
	HW_U16 ctc_dir_wt;
	HW_U16 ctc_dir_th;
};

struct isp_wdr_config {
	HW_U16 wdr_low_th;
	HW_U16 wdr_hi_th;
	HW_U16 wdr_exp_ratio;
	HW_U16 wdr_slope;
	HW_U8 wdr_table[ISP_WDR_MEM_SIZE];
};

struct isp_otf_dc_config {
	HW_U16 th_slop;
	HW_U16 min_th;
	HW_U16 max_th;
};

struct isp_tdnf_config {
	HW_U16 noise_clip_ratio;
	HW_U16 diff_clip_ratio;
	HW_U16 k_3d_s;
	HW_U16 diff_cal_mode;
	HW_U16 rec_en;
	HW_U16 tdnf_th[ISP_REG_TBL_LENGTH];
	HW_U16 tdnf_ref_noise[ISP_REG_TBL_LENGTH];
	HW_U8 tdnf_k[ISP_REG_TBL_LENGTH-1];
};

struct isp_contrast_config {
	HW_U16 contrast_min_val;
	HW_U16 contrast_max_val;
	HW_U16 black_clip;
	HW_U16 white_clip;
	HW_U16 black_level;
	HW_U16 white_level;
	HW_U16 plat_th;
	HW_U16 contrast_val[ISP_REG_TBL_LENGTH];
	HW_U16 contrast_lum[ISP_REG_TBL_LENGTH];
};

struct isp_offset {
	HW_S16 r_offset;
	HW_S16 gr_offset;
	HW_S16 gb_offset;
	HW_S16 b_offset;
};

struct isp_dg_gain {
	HW_U16 r_gain;
	HW_U16 gr_gain;
	HW_U16 gb_gain;
	HW_U16 b_gain;
};

struct isp_pltm_config {
	HW_U8 lss_switch;
	HW_U8 cal_en;
	HW_U8 frm_sm_en;
	HW_U8 last_order_ratio;
	HW_U8 tr_order;
	HW_U8 oripic_ratio;

	HW_U8 intens_asym;
	HW_U8 spatial_asm;
	HW_U8 white_level;
	HW_U8 lp_halo_res;
	HW_U8 lum_ratio;

	HW_U8 block_height;
	HW_U8 block_width;
	HW_U8 block_v_num;
	HW_U8 block_h_num;

	HW_U32 statistic_div;

 	HW_U8 pltm_table[ISP_PLTM_MEM_SIZE];
};

struct isp_cfa_config {
	HW_S32 min_rgb;
	HW_U32 dir_th;
};

struct isp_sharp_config {
	HW_U16 sharp_min_val;
	HW_U16 sharp_max_val;
	HW_U16 black_clip;
	HW_U16 white_clip;
	HW_U16 black_level;
	HW_U16 white_level;
	HW_U16 sharp_val[ISP_REG_TBL_LENGTH];
	HW_U16 sharp_lum[ISP_REG_TBL_LENGTH];
};

struct isp_saturation_config {
	HW_S16 satu_r;
	HW_S16 satu_g;
	HW_S16 satu_b;
	HW_S16 satu_gain;
	HW_S16 saturation_table[ISP_SATURATION_MEM_SIZE / 2];
};

enum isp_output_speed {
	ISP_OUTPUT_SPEED_0 = 0,
	ISP_OUTPUT_SPEED_1 = 1,
	ISP_OUTPUT_SPEED_2 = 2,
	ISP_OUTPUT_SPEED_3 = 3,
};

struct isp_bdnf_config {
	HW_U8 in_dis_min;
	HW_U8 in_dis_max;
	HW_U16 bdnf_th[ISP_REG_TBL_LENGTH];//rbw[5]+gw[7]
};

struct isp_bayer_gain_offset {
	HW_U16 r_gain;
	HW_U16 gr_gain;
	HW_U16 gb_gain;
	HW_U16 b_gain;

	HW_S16 r_offset;
	HW_S16 gr_offset;
	HW_S16 gb_offset;
	HW_S16 b_offset;
};

struct isp_wb_gain {
	HW_U16 r_gain;
	HW_U16 gr_gain;
	HW_U16 gb_gain;
	HW_U16 b_gain;
};

/**
 * struct isp_rgb2rgb_gain_offset - RGB to RGB Blending
 * @matrix:
 *              [RR] [GR] [BR]
 *              [RG] [GG] [BG]
 *              [RB] [GB] [BB]
 * @offset: Blending offset value for R,G,B.
 */
struct isp_rgb2rgb_gain_offset {
	HW_S16 matrix[3][3];
	HW_S16 offset[3];
};

struct isp_rgb2yuv_gain_offset {
	HW_S16 matrix[3][3];
	HW_S16 offset[3];
};

struct isp_lsc_config {
	HW_U16 ct_x;
	HW_U16 ct_y;
	HW_U16 rs_val;
};

struct isp_disc_config {
	HW_U16 disc_ct_x;
	HW_U16 disc_ct_y;
	HW_U16 disc_rs_val;
};

struct isp_h3a_reg_win {
	HW_U8 hor_num;
	HW_U8 ver_num;
	HW_U32 width;
	HW_U32 height;
	HW_U32 hor_start;
	HW_U32 ver_start;
};

struct isp_h3a_coor_win {
	HW_S32 x1;
	HW_S32 y1;
	HW_S32 x2;
	HW_S32 y2;
};

typedef struct isp_sensor_info {
	/*frome sensor*/
	char *name;
	HW_S32 hflip;
	HW_S32 vflip;
	HW_U32 hts;
	HW_U32 vts;
	HW_U32 pclk;
	HW_U32 fps_fixed;
	HW_U32 bin_factor;
	HW_U32 gain_min;
	HW_U32 gain_max;
	HW_S32 sensor_width;
	HW_S32 sensor_height;
	HW_U32 hoffset;
	HW_U32 voffset;
	HW_U32 input_seq;
	HW_U32 wdr_mode;

	/*from ae*/
	HW_U32 exp_line;
	HW_U32 ang_gain;
	HW_U32 dig_gain;

	HW_U32 lum_idx;
	HW_U32 gain_idx;

	HW_U32 ae_tbl_idx;
	HW_U32 ae_tbl_idx_max;

	HW_U32 fps;
	HW_U32 frame_time;
	HW_S32 ae_gain;

	/*from motion detect*/
	HW_S32 motion_flag;

	/*awb*/
	HW_S32 ae_lv;
	struct isp_bayer_gain_offset gain_offset;

	/*from af*/
	HW_S32 is_af_busy;
} isp_sensor_info_t;

enum exposure_cfg_type {
	ANTI_EXP_WIN_OVER     = 0,
	ANTI_EXP_WIN_UNDER = 1,
	ANTI_EXP_HIST_OVER = 2,
	ANTI_EXP_HIST_UNDER = 3,

	AE_PREVIEW_SPEED = 4,
	AE_CAPTURE_SPEED = 5,
	AE_VIDEO_SPEED = 6,
	AE_TOUCH_SPEED = 7,
	AE_TOLERANCE = 8,
	AE_TARGET = 9,

	AE_HIST_DARK_WEIGHT_MIN = 10,
	AE_HIST_DARK_WEIGHT_MAX = 11,
	AE_HIST_BRIGHT_WEIGHT_MIN = 12,
	AE_HIST_BRIGHT_WEIGHT_MAX = 13,

	ISP_EXP_CFG_MAX,
};

enum wdr_cfg_type {
	WDR_EXP_RATIO     = 0,
	WDR_LOW_TH = 1,
	WDR_HI_TH = 2,

	ISP_WDR_CFG_MAX,
};

enum ae_table_mode {
	SCENE_MODE_PREVIEW = 0,
	SCENE_MODE_CAPTURE,
	SCENE_MODE_VIDEO,

	SCENE_MODE_BACKLIGHT,
	SCENE_MODE_BEACH_SNOW,
	SCENE_MODE_FIREWORKS,
	SCENE_MODE_LANDSCAPE,
	SCENE_MODE_NIGHT,
	SCENE_MODE_SPORTS,

	SCENE_MODE_USER_DEF0,
	SCENE_MODE_USER_DEF1,
	SCENE_MODE_USER_DEF2,
	SCENE_MODE_USER_DEF3,
	SCENE_MODE_USER_DEF4,
	SCENE_MODE_USER_DEF5,
	SCENE_MODE_SENSOR_DRIVER,

	SCENE_MODE_MAX,
};
struct ae_table {
	HW_U32 min_exp;  //us
	HW_U32 max_exp;
	HW_U32 min_gain;
	HW_U32 max_gain;
	HW_U32 min_iris;
	HW_U32 max_iris;
};

struct ae_table_info {
	struct ae_table ae_tbl[10];
	HW_S32 length;
	HW_S32 ev_step;
	HW_S32 shutter_shift;
};
#endif //__BSP__ISP__COMM__H

