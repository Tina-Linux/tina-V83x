//#include <netinet/in.h>
#include <arpa/inet.h>

#include "../log_handle.h"
#include "../socket_protocol.h"
#include "../../include/device/isp_dev.h"

#include "isp_handle.h"
#include "capture_image.h"


enum _isp_flag_e {
	ISP_MODULE_INIT_NOT      = 0,
	ISP_MODULE_INIT_YES      = 1,
};
enum _isp_status_e {
	ISP_IDLE                 = 0,
	ISP_RUNNING              = 1,
};

#define VALID_ISP_SEL(id) \
	((id) >= 0 && (id) <= HW_ISP_DEVICE_NUM)


int g_module_init = ISP_MODULE_INIT_NOT;
int g_cur_isp_sel = -1;
int g_isp_status[HW_ISP_DEVICE_NUM];

void get_isp_version(char* version) 
{ 
	isp_get_version(version); 
}

int get_isp_id()
{
	return g_cur_isp_sel;
}


int init_isp_module()
{
	int ret = -1, i = 0;

	if (ISP_MODULE_INIT_YES == g_module_init) {
		exit_isp_module();
	}

	ret = media_dev_init();
	if (ret < 0) {
		LOG("%s: failed to init media dev\n", __FUNCTION__);
	} else {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			g_isp_status[i] = ISP_IDLE;
		}
		g_module_init = ISP_MODULE_INIT_YES;
	}
	return ret;
}

int exit_isp_module()
{
	int i = 0;
	if (ISP_MODULE_INIT_YES == g_module_init) {
		for (i = 0; i < HW_ISP_DEVICE_NUM; i++) {
			if (ISP_RUNNING == g_isp_status[i]) {
				LOG("%s: ready to exit isp %d\n", __FUNCTION__, i);
				isp_exit(i);
				g_isp_status[i] = ISP_IDLE;
			}
		}

		media_dev_exit();
		g_cur_isp_sel = -1;
		g_module_init = ISP_MODULE_INIT_NOT;
	}
	LOG("%s: exits\n", __FUNCTION__);
	return 0;
}

int select_isp(int id)
{
	int ret = -1;

	//LOG("%s: isp - %d\n", __FUNCTION__, id);
	if (VALID_ISP_SEL(id)) {
		if (ISP_MODULE_INIT_YES != g_module_init) {  // init module first
			ret = init_isp_module();
			if (ret < 0) {
				LOG("%s: failed to init isp module\n", __FUNCTION__);
				return -1;
			}
		}

		if (get_vich_status() == 0) {
			LOG("%s: !!!!!!! NO VICH OPENED !!!!!!!\n", __FUNCTION__);
			return -1;
		}

		if (ISP_IDLE == g_isp_status[id]) { // not running
			ret = isp_init(id);
			if (ret) {
				LOG("%s: failed to init isp - %d, %d\n", __FUNCTION__, id, ret);
				return -1;
			}
			ret = isp_run(id);
			if (ret) {
				isp_exit(id);
				LOG("%s: failed to run isp - %d\n", __FUNCTION__, id);
				return -1;
			}
			g_cur_isp_sel = id;
			g_isp_status[id] = ISP_RUNNING;
			ret = 0;
		} else {
			g_cur_isp_sel = id;
			ret = 0;
		}
		//LOG("%s: select isp %d\n", __FUNCTION__, id);
	} else {
		LOG("%s: invalid isp sel - %d\n", __FUNCTION__, id);
	}
	return ret;
}

#if (DO_LOG_EN && 0)
#define DD(fmt, ...)         printf("####### "fmt, ##__VA_ARGS__);
#else
#define DD(fmt, ...)
#endif

typedef HW_S32 (*convert_long_func)(HW_S32);
typedef HW_S16 (*convert_short_func)(HW_S16);

HW_S32 hton_long(HW_S32 ret)
{
	HW_S32 ret0 = (HW_S32)htonl(ret);
	DD("%d-%08x-%08x ", ret, ret, ret0);
	return ret0;
}
HW_S32 ntoh_long(HW_S32 ret)
{
	HW_S32 ret0 = (HW_S32)ntohl(ret);
	DD("%d-%08x-%08x ", ret0, ret0, ret);
	return ret0;
}
HW_S16 hton_short(HW_S16 ret)
{
	HW_S16 ret0 = (HW_S16)htons(ret);
	DD("%d-%04x-%04x ", ret, ret, ret0);
	return ret0;
}
HW_S16 ntoh_short(HW_S16 ret)
{
	HW_S16 ret0 = (HW_S16)ntohs(ret);
	DD("%d-%04x-%04x ", ret0, ret0, ret);
	return ret0;
}

void convert_tuning_cfg_func(HW_U8 group_id, HW_U32 cfg_ids, unsigned char *cfg_data,
	convert_long_func cvt_long, convert_short_func cvt_short)
{
	HW_S32 *int_ptr = NULL;
	HW_S16 *short_ptr = NULL;
	HW_U16 *ushort_ptr = NULL;
	HW_U8  *uchar_ptr = NULL;
	int attr_len = 0, i = 0;

	if (!cfg_data || !cvt_long || !cvt_short) {
		return ;
	}

	DD("%s starts\n", __FUNCTION__);
	switch (group_id) {
	case HW_ISP_CFG_TEST:
		if (cfg_ids & HW_ISP_CFG_TEST_PUB) {
			DD("test pub: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_pub_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_EXPTIME) {
			DD("test exp: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_item_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_GAIN) {
			DD("test gain: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_item_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_FOCUS) {
			DD("test focus: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_item_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_item_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_FORCED) {
			DD("test forced: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_forced_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_forced_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TEST_ENABLE) {
			DD("test enable: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_test_enable_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_test_enable_cfg);
		}
		break;
	case HW_ISP_CFG_3A:
		if (cfg_ids & HW_ISP_CFG_AE_PUB) {
			DD("ae pub: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_pub_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_pub_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_PREVIEW_TBL) {
			DD("ae preview tbl: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_table_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_CAPTURE_TBL) {
			DD("ae capture tbl: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_table_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_VIDEO_TBL) {
			DD("ae video tbl: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_table_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_WIN_WEIGHT) {
			DD("ae win weight: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_weight_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_weight_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AE_DELAY) {
			DD("ae delay: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_ae_delay_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_ae_delay_cfg);

		}
		if (cfg_ids & HW_ISP_CFG_AWB_PUB) {
			DD("awb speed: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_pub_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_pub_cfg);

		}
		if (cfg_ids & HW_ISP_CFG_AWB_TEMP_RANGE) {
			DD("awb temp: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_temp_range_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_temp_range_cfg);

		}
		if (cfg_ids & HW_ISP_CFG_AWB_DIST) {
			DD("awb dist: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_dist_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_dist_cfg);

		}
		if (cfg_ids & HW_ISP_CFG_AWB_LIGHT_INFO) {
			DD("awb light: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_temp_info_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_EXT_LIGHT_INFO) {
			DD("awb ext: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_temp_info_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_SKIN_INFO) {
			DD("awb skin: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_temp_info_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_SPECIAL_INFO) {
			DD("awb special: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_temp_info_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_temp_info_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_PRESET_GAIN) {
			DD("awb preset: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_preset_gain_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_preset_gain_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AWB_FAVOR) {
			DD("awb favor: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_awb_favor_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_awb_favor_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_VCM_CODE) {
			DD("af vcm: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_vcm_code_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_vcm_code_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_OTP) {
			DD("af otp: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_otp_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_otp_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_SPEED) {
			DD("af speed: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_speed_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_speed_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_FINE_SEARCH) {
			DD("af fine search: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_fine_search_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_fine_search_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_REFOCUS) {
			DD("af refocus: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_refocus_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_refocus_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_TOLERANCE) {
			DD("af tolerance: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_tolerance_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_tolerance_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_AF_SCENE) {
			DD("af scene: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_af_scene_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_af_scene_cfg);
		}
		break;
	case HW_ISP_CFG_TUNING:
		if (cfg_ids & HW_ISP_CFG_TUNING_FLASH) {
			DD("tuning flash: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_flash_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_flash_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_FLICKER) {
			DD("tuning flicker: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_flicker_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_flicker_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_VISUAL_ANGLE) {
			DD("tuning visual: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_visual_angle_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_visual_angle_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_GTM) {
			DD("tuning gtm: ");
			int_ptr = (HW_S32 *)cfg_data;
			// 1. type, gamma_type, auto_alpha_en, hist_pix_cnt, dark_minval, bright_minval
			for (i = 0; i < 6; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			// 2. plum_var
			short_ptr = (HW_S16 *)int_ptr;
			attr_len = 9*9;
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_gtm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CFA) {
			DD("tuning cfa: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_cfa_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_cfa_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CTC) {
			DD("tuning ctc: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_ctc_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_ctc_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_DIGITAL_GAIN) {
			DD("tuning digital gain: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_blc_gain_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_blc_gain_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_LOW) {
			DD("tuning ccm low: ");
			// 1. temp.
			ushort_ptr = (HW_U16 *)cfg_data;
			*ushort_ptr = cvt_short(*ushort_ptr);
			// 2. value
			short_ptr = (HW_S16 *)(cfg_data + sizeof(HW_U16));
			attr_len = sizeof(struct isp_rgb2rgb_gain_offset) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_MID) {
			DD("tuning ccm mid: ");
			// 1. temp.
			ushort_ptr = (HW_U16 *)cfg_data;
			*ushort_ptr = cvt_short(*ushort_ptr);
			// 2. value
			short_ptr = (HW_S16 *)(cfg_data + sizeof(HW_U16));
			attr_len = sizeof(struct isp_rgb2rgb_gain_offset) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CCM_HIGH) {
			DD("tuning ccm high: ");
			// 1. temp.
			ushort_ptr = (HW_U16 *)cfg_data;
			*ushort_ptr = cvt_short(*ushort_ptr);
			// 2. value
			short_ptr = (HW_S16 *)(cfg_data + sizeof(HW_U16));
			attr_len = sizeof(struct isp_rgb2rgb_gain_offset) / sizeof(HW_S16);
			for (i = 0; i < attr_len; i++, short_ptr++) {
				*short_ptr = cvt_short(*short_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_ccm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_PLTM) {
			DD("tuning pltm: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_pltm_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_pltm_cfg);
		}
#if (ISP_VERSION >= 521)
		if (cfg_ids & HW_ISP_CFG_TUNING_GCA) {
			DD("tuning gca: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_gca_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_gca_cfg);
		}
#endif
		break;
	case HW_ISP_CFG_TUNING_TABLES:
		if (cfg_ids & HW_ISP_CFG_TUNING_LSC) {
			DD("tuning lsc: ");
			// 1. ff_mod, center, rolloff, lsc_mode
			int_ptr = (HW_S32 *)cfg_data;
#if (ISP_VERSION >= 521)
			*int_ptr = cvt_long(*int_ptr);
#endif
			for (i = 0; i < 4; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}

			// 2. value
			ushort_ptr = (HW_U16 *)int_ptr;
			attr_len = 12 *768;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			// 3. temp.
			for (i = 0; i < 6; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_lsc_table_cfg);
		}

		if (cfg_ids & HW_ISP_CFG_TUNING_GAMMA) {
			DD("tuning gamma: ");
			// 1. number
			int_ptr = (HW_S32 *)cfg_data;
			*int_ptr = cvt_long(*int_ptr);
			// 2. value
			ushort_ptr = (HW_U16 *)(cfg_data + sizeof(HW_S32));
			attr_len = 5 * ISP_GAMMA_TBL_LENGTH;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			// 3. trigger
			for (i = 0; i < 5; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_gamma_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_LINEARITY) {
			DD("tuning linearity: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_linearity_table_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_linearity_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_DISTORTION) {
			DD("tuning distortion: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_distortion_table_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_distortion_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_BDNF) {
			DD("tuning bdnf: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_bdnf_table_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_bdnf_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_TDNF) {
			DD("tuning tdnf: ");
			// 1. thres, ref_noise
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = ISP_REG_TBL_LENGTH + ISP_REG_TBL_LENGTH;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			// 2. k, diff
#if DO_LOG_EN
			uchar_ptr = (HW_U8 *)ushort_ptr;
			attr_len = ISP_REG_TBL_LENGTH-1 + 256;
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
#endif
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_tdnf_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CONTRAST) {
			DD("tuning contrast: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_contrast_table_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_contrast_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_SHARP) {
			DD("tuning sharp: ");
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_sharp_table_cfg) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_sharp_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CEM) {
			DD("tuning cem: ");
#if DO_LOG_EN
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_cem_table_cfg) / sizeof(HW_U8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
#endif
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_cem_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_CEM_1) {
			DD("tuning cem_1: ");
#if DO_LOG_EN
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_cem_table_cfg) / sizeof(HW_U8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
#endif
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_cem_table_cfg);
		}
#if (ISP_VERSION != 522)
		if (cfg_ids & HW_ISP_CFG_TUNING_PLTM_TBL) {
			DD("tuning pltm table: ");
#if DO_LOG_EN
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = 512;
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
#endif
			ushort_ptr = (HW_U16 *)(cfg_data + 512);
			attr_len = 512;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
				}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_pltm_table_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_WDR) {
			DD("tuning wdr: ");
#if DO_LOG_EN
			uchar_ptr = (HW_U8 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_wdr_table_cfg) / sizeof(HW_U8);
			for (i = 0; i < attr_len; i++, uchar_ptr++) {
				DD("%d ", *uchar_ptr);
			}
#endif
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_wdr_table_cfg);
		}
#endif
#if (ISP_VERSION >= 521)
		if(cfg_ids & HW_ISP_CFG_TUNING_LCA_TBL){
			DD("tuning lca: ");
			// 2. isp_tuning_lca_pf_satu_lut
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_lca_pf_satu_lut) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}
			cfg_data += sizeof(struct isp_tuning_lca_pf_satu_lut);
			// 3. isp_tuning_lca_gf_satu_lut
			ushort_ptr = (HW_U16 *)cfg_data;
			attr_len = sizeof(struct isp_tuning_lca_gf_satu_lut) / sizeof(HW_U16);
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}
			DD("\n");
			/* offset */
			//cfg_data += sizeof(struct isp_tuning_lca_pf_satu_lut);
			/* offset */
			cfg_data += sizeof(struct isp_tuning_lca_gf_satu_lut);
		}
		if (cfg_ids & HW_ISP_CFG_TUNING_MSC) {
			DD("tuning msc: ");
			// 1. mff_mod, msc_mode
			int_ptr = (HW_S32 *)cfg_data;
			for (i = 0; i < 2; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			// 2. msc_blw_lut, msc_blh_lut
			int_ptr = (HW_S32 *)int_ptr;
			attr_len = ISP_MSC_TBL_LUT_SIZE + ISP_MSC_TBL_LUT_SIZE;
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			// 3. msc_blw_dlt_lut, msc_blh_dlt_lut
			int_ptr = (HW_S32 *)int_ptr;
			attr_len = ISP_MSC_TBL_LUT_DLT_SIZE + ISP_MSC_TBL_LUT_DLT_SIZE;
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			// 4. value
			ushort_ptr = (HW_U16 *)int_ptr;
			attr_len = (ISP_MSC_TEMP_NUM + ISP_MSC_TEMP_NUM) *ISP_MSC_TBL_LENGTH;
			for (i = 0; i < attr_len; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}
			// 5. temp.
			for (i = 0; i < 6; i++, ushort_ptr++) {
				*ushort_ptr = cvt_short(*ushort_ptr);
			}

			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_tuning_msc_table_cfg);
		}
#endif
		break;
	case HW_ISP_CFG_DYNAMIC:
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_LUM_POINT) {
			DD("dynamic lum: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_single_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_single_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_GAIN_POINT) {
			DD("dynamic gain: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_single_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_single_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SHARP) {
			DD("dynamic sharp: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_sharp_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_sharp_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_CONTRAST) {
			DD("dynamic contrast: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_contrast_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_contrast_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DENOISE) {
			DD("dynamic denoise: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_denoise_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_denoise_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SENSOR_OFFSET) {
			DD("dynamic sensor offset: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_sensor_offset_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_sensor_offset_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_BLACK_LV) {
			DD("dynamic blc: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_black_level_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_black_level_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DPC) {
			DD("dynamic dpc: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_dpc_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_dpc_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_PLTM) {
			DD("dynamic pltm: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_pltm_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_pltm_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_DEFOG) {
			DD("dynamic defog: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_defog_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_defog_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_HISTOGRAM) {
			DD("dynamic histogram: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_histogram_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_histogram_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_SATURATION) {
			DD("dynamic saturation: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_saturation_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_saturation_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_CEM) {
			DD("dynamic cem: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_cem_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_cem_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_TDF) {
			DD("dynamic tdf: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_tdf_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_tdf_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_AE) {
			DD("dynamic ae: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_ae_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_ae_cfg);
		}
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_GTM) {
			DD("dynamic gtm: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_gtm_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_gtm_cfg);
		}
#if (ISP_VERSION >= 521)
		if (cfg_ids & HW_ISP_CFG_DYNAMIC_LCA) {
			DD("dynamic lca: ");
			int_ptr = (HW_S32 *)cfg_data;
			attr_len = sizeof(struct isp_dynamic_lca_cfg) / sizeof(HW_S32);
			for (i = 0; i < attr_len; i++, int_ptr++) {
				*int_ptr = cvt_long(*int_ptr);
			}
			DD("\n");
			/* offset */
			cfg_data += sizeof(struct isp_dynamic_lca_cfg);
		}
#endif
		break;
	default:
		break;
	}
}
void convert_tuning_cfg_to_local(HW_U8 group_id, HW_U32 cfg_ids, unsigned char *cfg_data)
{
	convert_tuning_cfg_func(group_id, cfg_ids, cfg_data, &ntoh_long, &ntoh_short);
}
void convert_tuning_cfg_to_network(HW_U8 group_id, HW_U32 cfg_ids, unsigned char *cfg_data)
{
	convert_tuning_cfg_func(group_id, cfg_ids, cfg_data, &hton_long, &hton_short);
}

void output_3a_info(const void *stat_info, int type)
{
#if 1
	if (stat_info) {
		FILE *fp = NULL;
		char *buf = (char *)malloc(1<<20);  // 1M
		int index = 0;
		int i = 0, j = 0;
		if (SOCK_CMD_STAT_AE == type) {
			const struct isp_ae_stats_s *ae_info = (const struct isp_ae_stats_s *)stat_info;
			index += sprintf(&buf[index], "ae stat: win_pix_n %u\n", ae_info->win_pix_n);
			// accum_r
			index += sprintf(&buf[index], "-------------- accum_r ----------------\n");
			for (i = 0; i < ISP_AE_ROW; i++) {
				for (j = 0; j < ISP_AE_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", ae_info->accum_r[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// accum_g
			index += sprintf(&buf[index], "-------------- accum_g ----------------\n");
			for (i = 0; i < ISP_AE_ROW; i++) {
				for (j = 0; j < ISP_AE_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", ae_info->accum_g[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// accum_b
			index += sprintf(&buf[index], "-------------- accum_b ----------------\n");
			for (i = 0; i < ISP_AE_ROW; i++) {
				for (j = 0; j < ISP_AE_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", ae_info->accum_b[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// avg
			index += sprintf(&buf[index], "-------------- avg ----------------\n");
			for (i = 0; i < ISP_AE_ROW; i++) {
				for (j = 0; j < ISP_AE_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", ae_info->avg[i * ISP_AE_COL + j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// hist
			index += sprintf(&buf[index], "-------------- hist ----------------\n");
			for (i = 0; i < 16; i++) {
				for (j = 0; j < 16; j++) {
					index += sprintf(&buf[index], "%06u, ", ae_info->hist[i * 16 + j]);
				}
				index += sprintf(&buf[index], "\n");
			}

			fp = fopen("./stat_3a_info.txt", "a");
			if (fp) {
				fwrite(buf, index, 1, fp);
				fclose(fp);
				fp = NULL;
			}
		} else if (SOCK_CMD_STAT_AWB == type) {
			const struct isp_awb_stats_s *awb_info = (const struct isp_awb_stats_s *)stat_info;
			index += sprintf(&buf[index], "awb stat\n");
			// awb_sum_r
			index += sprintf(&buf[index], "-------------- awb_sum_r ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_sum_r[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_sum_g
			index += sprintf(&buf[index], "-------------- awb_sum_g ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_sum_g[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_sum_b
			index += sprintf(&buf[index], "-------------- awb_sum_b ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_sum_b[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_sum_cnt
			index += sprintf(&buf[index], "-------------- awb_sum_cnt ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_sum_cnt[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_avg_r
			index += sprintf(&buf[index], "-------------- awb_avg_r ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_avg_r[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_avg_g
			index += sprintf(&buf[index], "-------------- awb_avg_g ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_avg_g[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_avg_b
			index += sprintf(&buf[index], "-------------- awb_avg_b ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->awb_avg_b[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// awb_avg
			index += sprintf(&buf[index], "-------------- awb_avg_b ----------------\n");
			for (i = 0; i < ISP_AWB_ROW; i++) {
				for (j = 0; j < ISP_AWB_COL; j++) {
					index += sprintf(&buf[index], "%06u, ", awb_info->avg[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}

			fp = fopen("./stat_3a_info.txt", "a");
			if (fp) {
				fwrite(buf, index, 1, fp);
				fclose(fp);
				fp = NULL;
			}
		} else if (SOCK_CMD_STAT_AF == type) {
			const struct isp_af_stats_s *af_info = (const struct isp_af_stats_s *)stat_info;
			index += sprintf(&buf[index], "af stat:\n");
			// af_count
			index += sprintf(&buf[index], "-------------- af_count ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_count[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// af_h_d1
			index += sprintf(&buf[index], "-------------- af_h_d1 ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_h_d1[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// af_h_d2
			index += sprintf(&buf[index], "-------------- af_h_d2 ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_h_d2[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// af_v_d1
			index += sprintf(&buf[index], "-------------- af_v_d1 ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_v_d1[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}
			// af_v_d2
			index += sprintf(&buf[index], "-------------- af_v_d2 ----------------\n");
			for (i = 0; i < ISP_AF_ROW; i++) {
				for (j = 0; j < ISP_AF_COL; j++) {
					index += sprintf(&buf[index], "%06lu, ", (long unsigned int)af_info->af_v_d2[i][j]);
				}
				index += sprintf(&buf[index], "\n");
			}

			fp = fopen("./stat_3a_info.txt", "a");
			if (fp) {
				fwrite(buf, index, 1, fp);
				fclose(fp);
				fp = NULL;
			}
		}

		free(buf);
		buf = NULL;
	}
#endif
}

void hton_3a_info(void *stat_info, int type)
{
	if (stat_info) {
		HW_U64 *ptr_u64 = NULL;
		HW_U32 *ptr_u32 = NULL;
		int i = 0;
		if (SOCK_CMD_STAT_AE == type) {
			struct isp_ae_stats_s *ae_stat = (struct isp_ae_stats_s *)stat_info;
			ae_stat->win_pix_n = htonl(ae_stat->win_pix_n);
			// accum_r
			ptr_u32 = &ae_stat->accum_r[0][0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// accum_g
			ptr_u32 = &ae_stat->accum_g[0][0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// accum_b
			ptr_u32 = &ae_stat->accum_b[0][0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// avg
			ptr_u32 = &ae_stat->avg[0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// hist
			ptr_u32 = &ae_stat->hist[0];
			for (i = 0; i < ISP_HIST_NUM; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
		} else if (SOCK_CMD_STAT_AWB == type) {
			struct isp_awb_stats_s *awb_stat = (struct isp_awb_stats_s *)stat_info;
			// awb_sum_r
			ptr_u32 = &awb_stat->awb_sum_r[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_sum_g
			ptr_u32 = &awb_stat->awb_sum_g[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_sum_b
			ptr_u32 = &awb_stat->awb_sum_b[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_sum_cnt
			ptr_u32 = &awb_stat->awb_sum_cnt[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_avg_r
			ptr_u32 = &awb_stat->awb_avg_r[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_avg_g
			ptr_u32 = &awb_stat->awb_avg_g[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_avg_b
			ptr_u32 = &awb_stat->awb_avg_b[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
			// awb_avg
			ptr_u32 = &awb_stat->avg[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = htonl(*ptr_u32);
			}
		} else if (SOCK_CMD_STAT_AF == type) {
			struct isp_af_stats_s *af_stat = (struct isp_af_stats_s *)stat_info;
			// af_count
			ptr_u64 = &af_stat->af_count[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_h_d1
			ptr_u64 = &af_stat->af_h_d1[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_h_d2
			ptr_u64 = &af_stat->af_h_d2[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_v_d1
			ptr_u64 = &af_stat->af_v_d1[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
			// af_v_d2
			ptr_u64 = &af_stat->af_v_d2[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = htonl(*ptr_u64);
			}
		}
	}
}

void ntoh_3a_info(void *stat_info, int type)
{
	if (stat_info) {
		HW_U64 *ptr_u64 = NULL;
		HW_U32 *ptr_u32 = NULL;
		int i = 0;
		if (SOCK_CMD_STAT_AE == type) {
			struct isp_ae_stats_s *ae_stat = (struct isp_ae_stats_s *)stat_info;
			ae_stat->win_pix_n = ntohl(ae_stat->win_pix_n);
			// accum_r
			ptr_u32 = &ae_stat->accum_r[0][0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// accum_g
			ptr_u32 = &ae_stat->accum_g[0][0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// accum_b
			ptr_u32 = &ae_stat->accum_b[0][0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// avg
			ptr_u32 = &ae_stat->avg[0];
			for (i = 0; i < ISP_AE_ROW * ISP_AE_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// hist
			ptr_u32 = &ae_stat->hist[0];
			for (i = 0; i < ISP_HIST_NUM; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
		} else if (SOCK_CMD_STAT_AWB == type) {
			struct isp_awb_stats_s *awb_stat = (struct isp_awb_stats_s *)stat_info;
			// awb_sum_r
			ptr_u32 = &awb_stat->awb_sum_r[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_sum_g
			ptr_u32 = &awb_stat->awb_sum_g[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_sum_b
			ptr_u32 = &awb_stat->awb_sum_b[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_sum_cnt
			ptr_u32 = &awb_stat->awb_sum_cnt[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_avg_r
			ptr_u32 = &awb_stat->awb_avg_r[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_avg_g
			ptr_u32 = &awb_stat->awb_avg_g[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_avg_b
			ptr_u32 = &awb_stat->awb_avg_b[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
			// awb_avg
			ptr_u32 = &awb_stat->avg[0][0];
			for (i = 0; i < ISP_AWB_ROW * ISP_AWB_COL; i++, ptr_u32++) {
				*ptr_u32 = ntohl(*ptr_u32);
			}
		} else if (SOCK_CMD_STAT_AF == type) {
			struct isp_af_stats_s *af_stat = (struct isp_af_stats_s *)stat_info;
			// af_count
			ptr_u64 = &af_stat->af_count[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
			// af_h_d1
			ptr_u64 = &af_stat->af_h_d1[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
			// af_h_d2
			ptr_u64 = &af_stat->af_h_d2[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
			// af_v_d1
			ptr_u64 = &af_stat->af_v_d1[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
			// af_v_d2
			ptr_u64 = &af_stat->af_v_d2[0][0];
			for (i = 0; i < ISP_AF_ROW * ISP_AF_COL; i++, ptr_u64++) {
				*ptr_u64 = ntohl(*ptr_u64);
			}
		}
	}
}


