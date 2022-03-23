
/*
 ******************************************************************************
 *
 * MPI_ISP.h
 *
 * Hawkview ISP - mpi_isp.h module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   1.0		  yuanxianfeng   2016/04/01	    ISP
 *
 *****************************************************************************
 */
 //#define LOG_NDEBUG
#define LOG_TAG "mpi_isp"
#include <utils/plat_log.h>

#include "mpi_isp.h"
#include "isp_tuning.h"
#include "isp.h"

#define MEDIA_DEVICE_0 "/dev/media0"
#define MEDIA_DEVICE_1 "/dev/media1"

/* ======================================= */
/* function list */
/* ======================================= */
#if 0
AW_S32 AW_MPI_ISP_SetRegister(ISP_DEV IspDev, AW_U32 u32Addr, AW_U32 u32Value);
AW_S32 AW_MPI_ISP_GetRegister(ISP_DEV IspDev, AW_U32 u32Addr, AW_U32 *pu32Value);
//AW_S32 AW_MPI_ISP_SetFlash(ISP_DEV IspDev,  ISP_FLASH_S *pFlash);
//AW_S32 AW_MPI_ISP_GetFlash(ISP_DEV IspDev, ISP_FLASH_S *pFlash);
//AW_S32 AW_MPI_ISP_SetFlicker(ISP_DEV IspDev,  ISP_FLICKER_S *pFlicker);
//AW_S32 AW_MPI_ISP_GetFlicker(ISP_DEV IspDev, ISP_FLICKER_S *pFlicker);
AW_S32 AW_MPI_ISP_SetDeFogAttr(ISP_DEV IspDev,  ISP_DEFOG_ATTR_S *pstDefogAttr);
AW_S32 AW_MPI_ISP_GetDeFogAttr(ISP_DEV IspDev, ISP_DEFOG_ATTR_S *pstDefogAttr);
AW_S32 AW_MPI_ISP_SetDPC(ISP_DEV IspDev, ISP_OTF *pDpcOtf);
AW_S32 AW_MPI_ISP_GetDPC(ISP_DEV IspDev, ISP_OTF *pDpcOtf);
AW_S32 AW_MPI_ISP_SetBlackLevel(ISP_DEV IspDev,  ISP_BLACK_LEVEL_S *pstBlackLevel);
AW_S32 AW_MPI_ISP_GetBlackLevel(ISP_DEV IspDev, ISP_BLACK_LEVEL_S *pstBlackLevel);
AW_S32 AW_MPI_ISP_SetShadingAttr(ISP_DEV IspDev,  ISP_SHADING_ATTR_S *pstShadingAttr);
AW_S32 AW_MPI_ISP_GetShadingAttr(ISP_DEV IspDev, ISP_SHADING_ATTR_S *pstShadingAttr);
AW_S32 AW_MPI_ISP_SetGamma(ISP_DEV IspDev,  ISP_GAMMA_ATTR_S *pstGammaAttr);
AW_S32 AW_MPI_ISP_GetGamma(ISP_DEV IspDev, ISP_GAMMA_ATTR_S *pstGammaAttr);
AW_S32 AW_MPI_ISP_SetLinearity(ISP_DEV IspDev,  ISP_LINEARITY_S *pLinearity);
AW_S32 AW_MPI_ISP_GetLinearity(ISP_DEV IspDev, ISP_LINEARITY_S *pLinearity);
AW_S32 AW_MPI_ISP_SetDistortion(ISP_DEV IspDev,  ISP_DISTORTION_S *pDistortion);
AW_S32 AW_MPI_ISP_GetDistortion(ISP_DEV IspDev, ISP_DISTORTION_S *pDistortion);
// AW_S32 AW_MPI_ISP_SetTrigger(ISP_DEV IspDev, ISP_TRIGER_S *pTriger);
// AW_S32 AW_MPI_ISP_GetTrigger(ISP_DEV IspDev, ISP_TRIGER_S *pTriger);
// AW_S32 AW_MPI_ISP_SetLumMapPoint(ISP_DEV IspDev, ISP_SINGLE_S *plumP);
// AW_S32 AW_MPI_ISP_GetLumMapPoint(ISP_DEV IspDev, ISP_SINGLE_S *plumP);
// AW_S32 AW_MPI_ISP_SetGainMapPoint(ISP_DEV IspDev, ISP_SINGLE_S *pGainP);
// AW_S32 AW_MPI_ISP_GetGainMapPoint(ISP_DEV IspDev, ISP_SINGLE_S *pGainP);
AW_S32 AW_MPI_ISP_AWB_SetCCMAttr(ISP_DEV IspDev, int color_temp, ISP_COLORMATRIX_ATTR_S *pstCCMAttr);
AW_S32 AW_MPI_ISP_AWB_GetCCMAttr(ISP_DEV IspDev, int color_temp, ISP_COLORMATRIX_ATTR_S *pstCCMAttr);
/*
AW_S32 AW_MPI_ISP_SetBrightness(ISP_DEV IspDev, ISP_BRIGHTNESS_S *pBrightness);
AW_S32 AW_MPI_ISP_GetBrightness(ISP_DEV IspDev, ISP_BRIGHTNESS_S *pBrightness);
AW_S32 AW_MPI_ISP_SetContrast(ISP_DEV IspDev, ISP_CONTRAST_S *pContrast);
AW_S32 AW_MPI_ISP_GetContrast(ISP_DEV IspDev, ISP_CONTRAST_S *pContrast);
AW_S32 AW_MPI_ISP_SetSaturation(ISP_DEV IspDev, ISP_SATURATION_S *pSta);
AW_S32 AW_MPI_ISP_GetSaturation(ISP_DEV IspDev, ISP_SATURATION_S *pSta);
AW_S32 AW_MPI_ISP_SetHue(ISP_DEV IspDev, ISP_HUE_S *pHue);
AW_S32 AW_MPI_ISP_GetHue(ISP_DEV IspDev, ISP_HUE_S *pHue);
*/
AW_S32 AW_MPI_ISP_SetSharpness(ISP_DEV IspDev,  ISP_SHARPEN_ATTR_S *pstSharpenAttr);
AW_S32 AW_MPI_ISP_GetSharpness(ISP_DEV IspDev, ISP_SHARPEN_ATTR_S *pstSharpenAttr);
AW_S32 AW_MPI_ISP_SetNRAttr(ISP_DEV IspDev,  ISP_NR_ATTR_S *pstNRAttr);
AW_S32 AW_MPI_ISP_GetNRAttr(ISP_DEV IspDev, ISP_NR_ATTR_S *pstNRAttr);
AW_S32 AW_MPI_ISP_Set3NRAttr(ISP_DEV IspDev, ISP_3NR_ATTR_S *pTdf);
AW_S32 AW_MPI_ISP_Get3NRAttr(ISP_DEV IspDev, ISP_3NR_ATTR_S *pTdf);
AW_S32 AW_MPI_ISP_SetVisualAngle(ISP_DEV IspDev,  ISP_VISUAL_ANGLE_S *pVisualAngle);
AW_S32 AW_MPI_ISP_GetVisualAngle(ISP_DEV IspDev, ISP_VISUAL_ANGLE_S *pVisualAngle);
// AW_S32 AW_MPI_ISP_SetAe(ISP_DEV IspDev, ISP_AE_S *pAe);
// AW_S32 AW_MPI_ISP_GetAe(ISP_DEV IspDev, ISP_AE_S *pAe);
AW_S32 AW_MPI_ISP_SetDynamicGtm(ISP_DEV IspDev, ISP_DYNAMIC_GTM_S *pGtm);
AW_S32 AW_MPI_ISP_GetDynamicGtm(ISP_DEV IspDev, ISP_DYNAMIC_GTM_S *pGtm);
AW_S32 AW_MPI_ISP_SetDynamicPltm(ISP_DEV IspDev, ISP_DYNAMIC_PLTM_S *pPltm);
AW_S32 AW_MPI_ISP_GetDynamicPltm(ISP_DEV IspDev, ISP_DYNAMIC_PLTM_S *pPltm);
// AW_S32 AW_MPI_ISP_SetGtm(ISP_DEV IspDev, ISP_GTM_S *pGtm);
// AW_S32 AW_MPI_ISP_GetGtm(ISP_DEV IspDev, ISP_GTM_S *pGtm);
#endif

/* ======================================= */
/* function come true */
/* ======================================= */
/* AW_S32 AW_MPI_ISP_Init(ISP_DEV IspDev)
{
	printf("Donot use this API.\r\n");
	printf("AW_MPI_ISP_Init(ISP_DEV IspDev) rename -> \
		AW_S32 AW_MPI_ISP_Init();\r\n");
	return SUCCESS;
} */

AW_S32 AW_MPI_ISP_Init()
{
	return media_dev_init();
}


AW_S32 AW_MPI_ISP_Run(ISP_DEV IspDev)
{
    AW_S32 eErr;

    eErr = isp_init(IspDev); // 0, 1
    if (eErr == EN_ERR_EFUSE_ERROR) {
        return ERR_ISP_EFUSE_ERR;
    }
    if (eErr) {
        return eErr;
    }

    return isp_run(IspDev);
}

AW_S32 AW_MPI_ISP_Stop(ISP_DEV IspDev)
{
	isp_stop(IspDev);
	isp_pthread_join(IspDev); 
	isp_exit(IspDev);
	return SUCCESS;
}

/*AW_S32 AW_MPI_ISP_Exit(ISP_DEV IspDev)
{
    printf("Donot use this API.\r\n");
	printf("AW_MPI_ISP_Exit(ISP_DEV IspDev) rename -> \
		AW_S32 AW_MPI_ISP_Exit();\r\n");
	return SUCCESS;
}*/

AW_S32 AW_MPI_ISP_Exit()
{
	media_dev_exit();
	return SUCCESS;
}

AW_S32 AW_MPI_ISP_SetModuleOnOff(ISP_DEV IspDev, ISP_MODULE_ONOFF *pstIspModuleOnOff)
{
	struct isp_test_enable_cfg isp_enable;
	memcpy(&isp_enable, pstIspModuleOnOff, sizeof(struct isp_test_enable_cfg));
	isp_set_cfg(IspDev, HW_ISP_CFG_TEST, HW_ISP_CFG_TEST_ENABLE, &isp_enable);
	isp_update(IspDev);

	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetModuleOnOff(ISP_DEV IspDev, ISP_MODULE_ONOFF *pstIspModuleOnOff)
{
	struct isp_test_enable_cfg isp_enable;
	isp_get_cfg(IspDev, HW_ISP_CFG_TEST, HW_ISP_CFG_TEST_ENABLE, &isp_enable);
	memcpy(pstIspModuleOnOff, &isp_enable, sizeof(struct isp_test_enable_cfg));

	return SUCCESS;
}

AW_S32 AW_MPI_ISP_SetSaveCTX(ISP_DEV IspDev)
{
    return isp_set_saved_ctx(IspDev);
}

AW_S32 AW_MPI_ISP_SetAe(ISP_DEV IspDev, ISP_AE_S *pAe)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_AE, pAe);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetAe(ISP_DEV IspDev, ISP_AE_S *pAe)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_AE, pAe);
	return ret;
}

AW_S32 AW_MPI_ISP_SwitchIspCofig(ISP_DEV IspDev, ISP_CFG_MODE ModeFlag)    // 0 : no_ir__no_wdr 1 : no_ir__wdr  2 : ir__no_wdr  3 : ir__wdr
{
    AW_S32 ret = FAILURE;

    if((ModeFlag < 0) || (ModeFlag > 3))
    {
        aloge("Invalid ModeFlag\n");
        return ret;
    }
    ret = isp_reset(IspDev, (int)ModeFlag);
    return ret;
}

int AW_MPI_ISP_GetEnvLV(ISP_DEV IspDev)
{
    return isp_get_lv(IspDev);
}

int AW_MPI_ISP_GetEvLvAdj(ISP_DEV IspDev)
{
    return isp_get_ev_lv_adj(IspDev);
}

int AW_MPI_ISP_GetTemperature(ISP_DEV IspDev)
{
    return isp_get_temp(IspDev);
}

AW_S32 AW_MPI_ISP_SetLocalExposureArea(ISP_DEV IspDev, SIZE_S Res, RECT_S RoiRgn)
{
    AW_S32 ret = FAILURE;
    struct isp_h3a_coor_win coor;
    POINT_S PtRightBottom;

    PtRightBottom.X = RoiRgn.X + RoiRgn.Width;
    PtRightBottom.Y = RoiRgn.Y + RoiRgn.Height;
    coor.x1 = (int)( (float)RoiRgn.X*2000/Res.Width  - 1000 );
    coor.y1 = (int)( (float)RoiRgn.Y*2000/Res.Height - 1000 );
    coor.x2 = (int)( (float)PtRightBottom.X*2000/Res.Width  - 1000 );
    coor.y2 = (int)( (float)PtRightBottom.Y*2000/Res.Height - 1000 );

    ret = isp_set_attr_cfg(IspDev, ISP_CTRL_AE_ROI, &coor);
    return ret;
}

#if 0
// WDR
AW_S32 AW_MPI_ISP_SetPltmWDR(ISP_DEV IspDev, ISP_DYNAMIC_PLTM_S *pPltm)
{
	AW_S32 ret = FAILURE;
	ISP_DYNAMIC_PLTM_S isp_pltm;
	// struct isp_tuning_pltm_cfg pltm_cfg;//HW_ISP_CFG_TUNING_PLTM
	// struct isp_tuning_pltm_table_cfg pltm_table_cfg; // HW_ISP_CFG_TUNING_PLTM_TBL
	memcpy(&isp_pltm.pltm_cfg, &pPltm->pltm_cfg, sizeof(struct isp_tuning_pltm_cfg));
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_PLTM, &isp_pltm.pltm_cfg);
	// ret = isp_set_cfg(IspDev, HW_ISP_CFG_TUNING_TABLES, HW_ISP_CFG_TUNING_PLTM_TBL, &isp_pltm.pltm_table_cfg);
    isp_update(IspDev);

	return 0;
}
AW_S32 AW_MPI_ISP_GetPltmWDR(ISP_DEV IspDev, ISP_DYNAMIC_PLTM_S *pPltm)
{
	AW_S32 ret = FAILURE;
	ISP_DYNAMIC_PLTM_S isp_pltm;
	// struct isp_tuning_pltm_cfg;//HW_ISP_CFG_TUNING_PLTM
	// struct isp_tuning_pltm_table_cfg;// HW_ISP_CFG_TUNING_PLTM_TBL
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_PLTM, &isp_pltm.pltm_cfg);
	memcpy(&pPltm->pltm_cfg, &isp_pltm.pltm_cfg, sizeof(struct isp_tuning_pltm_cfg));
	// ret = isp_get_cfg(IspDev, HW_ISP_CFG_TUNING_TABLES, HW_ISP_CFG_TUNING_PLTM_TBL, &isp_pltm.pltm_table_cfg);
	// memcpy(pPltm, &isp_pltm, sizeof(struct isp_dynamic_pltm_cfg));
	return 0;
}

// 2DNR
AW_S32 AW_MPI_ISP_SetNRAttr(ISP_DEV IspDev, ISP_NR_ATTR_S *pstNRAttr)
{
	struct isp_dynamic_denoise_cfg isp_denoise_cfg;
	if (NULL == pstNRAttr)
		return FALSE;
    memcpy(&isp_denoise_cfg, pstNRAttr, sizeof(struct isp_dynamic_denoise_cfg));
	isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_DENOISE, &isp_denoise_cfg);
    isp_update(IspDev);
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetNRAttr(ISP_DEV IspDev, ISP_NR_ATTR_S *pstNRAttr)
{
	struct isp_dynamic_denoise_cfg isp_denoise_cfg;
	isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_DENOISE, &isp_denoise_cfg);
    memcpy(pstNRAttr, &isp_denoise_cfg, sizeof(struct isp_dynamic_denoise_cfg));
	return SUCCESS;
}
// 3DNR
AW_S32 AW_MPI_ISP_Set3NRAttr(ISP_DEV IspDev, ISP_3NR_ATTR_S *pstNRAttr)
{
	struct isp_dynamic_tdf_cfg isp_tdf_cfg;
	if (NULL == pstNRAttr)
		return FALSE;
    memcpy(&isp_tdf_cfg, pstNRAttr, sizeof(struct isp_dynamic_tdf_cfg));
	isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_TDF, &isp_tdf_cfg);
    isp_update(IspDev);
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_Get3NRAttr(ISP_DEV IspDev, ISP_3NR_ATTR_S *pstNRAttr)
{
	struct isp_dynamic_tdf_cfg isp_tdf_cfg;
	isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_TDF, &isp_tdf_cfg);
    memcpy(pstNRAttr, &isp_tdf_cfg, sizeof(struct isp_dynamic_tdf_cfg));
	return SUCCESS;
}
#else

#endif

#if 0
AW_S32 AW_MPI_ISP_SetRegister(ISP_DEV IspDev, AW_U32 u32Addr, AW_U32 u32Value)
{ 
    isp_update(IspDev);
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetRegister(ISP_DEV IspDev, AW_U32 u32Addr, AW_U32 *pu32Value)
{
	return SUCCESS;
}
// on_off
AW_S32 AW_MPI_ISP_SetModuleOnOff(ISP_DEV IspDev, ISP_MODULE_ONOFF *pstIspModuleOnOff)
{
	struct isp_test_enable_cfg isp_enable;
	memcpy(&isp_enable, pstIspModuleOnOff, sizeof(struct isp_test_enable_cfg));
	isp_set_cfg(IspDev, HW_ISP_CFG_TEST, HW_ISP_CFG_TEST_ENABLE, &isp_enable);
	isp_update(IspDev);

	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetModuleOnOff(ISP_DEV IspDev, ISP_MODULE_ONOFF *pstIspModuleOnOff)
{
	struct isp_test_enable_cfg isp_enable;
	isp_get_cfg(IspDev, HW_ISP_CFG_TEST, HW_ISP_CFG_TEST_ENABLE, &isp_enable);
	memcpy(pstIspModuleOnOff, &isp_enable, sizeof(struct isp_test_enable_cfg));

	return SUCCESS;
}
// Flash 
AW_S32 AW_MPI_ISP_SetFlash(ISP_DEV IspDev, ISP_FLASH_S *pFlash)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_FLASH, pFlash);
    isp_update(IspDev);
    return ret;
}
AW_S32 AW_MPI_ISP_GetFlash(ISP_DEV IspDev, ISP_FLASH_S *pFlash)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_FLASH, pFlash);
	return ret;
}

/*// Flicker
AW_S32 AW_MPI_ISP_SetFlicker(ISP_DEV IspDev, ISP_FLICKER_S *pFlicker)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_FLICKER, pFlicker);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetFlicker(ISP_DEV IspDev, ISP_FLICKER_S *pFlicker)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_FLICKER, pFlicker);
	return ret;
}*/
// defog
AW_S32 AW_MPI_ISP_SetDeFogAttr(ISP_DEV IspDev, ISP_DEFOG_ATTR_S *pstDefogAttr)
{
	struct isp_tuning_defog_cfg isp_defog;

	if (NULL == pstDefogAttr)
		return FALSE;

	isp_defog.strength = pstDefogAttr->strength;
	isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_DEFOG , &isp_defog);
    isp_update(IspDev);
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetDeFogAttr(ISP_DEV IspDev, ISP_DEFOG_ATTR_S *pstDefogAttr)
{
	struct isp_tuning_defog_cfg isp_defog;
	isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_DEFOG , &isp_defog);
	pstDefogAttr->strength = isp_defog.strength;

	return SUCCESS;
}
// dpc
AW_S32 AW_MPI_ISP_SetDPC(ISP_DEV IspDev, ISP_OTF *pDpcOtf)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_DPC_OTF , pDpcOtf);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetDPC(ISP_DEV IspDev, ISP_OTF *pDpcOtf)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_DPC_OTF , pDpcOtf);
	return ret;
}
// blc
AW_S32 AW_MPI_ISP_SetBlackLevel(ISP_DEV IspDev, ISP_BLACK_LEVEL_S *pstBlackLevel)
{
	struct isp_tuning_blc_gain_cfg  isp_gain_offset;

	isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_BLACK_LV , &isp_gain_offset);
	isp_gain_offset.value[0] = pstBlackLevel->au16BlackLevel[0];
	isp_gain_offset.value[1] = pstBlackLevel->au16BlackLevel[1];
	isp_gain_offset.value[2] = pstBlackLevel->au16BlackLevel[2];
	isp_gain_offset.value[3] = pstBlackLevel->au16BlackLevel[3];
	isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_BLACK_LV , &isp_gain_offset);
    isp_update(IspDev);
    
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetBlackLevel(ISP_DEV IspDev, ISP_BLACK_LEVEL_S *pstBlackLevel)
{
	struct isp_tuning_blc_gain_cfg  isp_gain_offset;

	isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_BLACK_LV , &isp_gain_offset);
	pstBlackLevel->au16BlackLevel[0] = isp_gain_offset.value[0];
	pstBlackLevel->au16BlackLevel[1] = isp_gain_offset.value[1];
	pstBlackLevel->au16BlackLevel[2] = isp_gain_offset.value[2];
	pstBlackLevel->au16BlackLevel[3] = isp_gain_offset.value[3];

	return SUCCESS;
}
// lsc
AW_S32 AW_MPI_ISP_SetShadingAttr(ISP_DEV IspDev, ISP_SHADING_ATTR_S *pstShadingAttr)
{
/*	struct isp_tuning_lens_shading_cfg isp_lsc;

	if (NULL == pstShadingAttr)
		return FALSE;

	isp_lsc.ff_mod = pstShadingAttr->ff_mod;
	isp_lsc.center_x = pstShadingAttr->center_x;
	isp_lsc.center_y = pstShadingAttr->center_y;
    isp_lsc.rolloff_ratio = pstShadingAttr->rolloff_ratio;
	memcpy(&(isp_lsc.value[0][0]), &(pstShadingAttr->value[0][0]), sizeof(isp_lsc.value));
	memcpy(isp_lsc.color_temp_triggers, pstShadingAttr->color_temp_triggers, sizeof(isp_lsc.color_temp_triggers));

	isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_LSC , &isp_lsc);
    isp_update(IspDev);
*/
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetShadingAttr(ISP_DEV IspDev, ISP_SHADING_ATTR_S *pstShadingAttr)
{
/*	struct isp_tuning_lens_shading_cfg isp_lsc;

	if (NULL == pstShadingAttr)
		return FALSE;

	isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_LSC , &isp_lsc);
	pstShadingAttr->ff_mod= isp_lsc.ff_mod;
	pstShadingAttr->center_x= isp_lsc.center_x;
	pstShadingAttr->center_y= isp_lsc.center_y;
    pstShadingAttr->rolloff_ratio = isp_lsc.rolloff_ratio;
	memcpy(&(pstShadingAttr->value[0][0]), &(isp_lsc.value[0][0]), sizeof(isp_lsc.value));
	memcpy(pstShadingAttr->color_temp_triggers, isp_lsc.color_temp_triggers, sizeof(isp_lsc.color_temp_triggers));
*/
	return SUCCESS;
}
// gamma
AW_S32 AW_MPI_ISP_SetGamma(ISP_DEV IspDev, ISP_GAMMA_ATTR_S *pstGammaAttr)
{
    struct isp_tuning_gamma_table_cfg isp_tuning_gamma;
    memcpy(&isp_tuning_gamma, pstGammaAttr, sizeof(struct isp_tuning_gamma_table_cfg));

	isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_GAMMA , &isp_tuning_gamma);
    isp_update(IspDev);
    
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetGamma(ISP_DEV IspDev, ISP_GAMMA_ATTR_S *pstGammaAttr)
{
	struct isp_tuning_gamma_table_cfg isp_tuning_gamma;
	isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_GAMMA , &isp_tuning_gamma);
    memcpy(pstGammaAttr, &isp_tuning_gamma, sizeof(struct isp_tuning_gamma_table_cfg));

	return SUCCESS;
}
// linear
AW_S32 AW_MPI_ISP_SetLinearity(ISP_DEV IspDev, ISP_LINEARITY_S *pLinearity)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_LINEARITY, pLinearity);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetLinearity(ISP_DEV IspDev, ISP_LINEARITY_S *pLinearity)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_LINEARITY, pLinearity);
	return ret;
}
// dis
AW_S32 AW_MPI_ISP_SetDistortion(ISP_DEV IspDev, ISP_DISTORTION_S *pDistortion)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_DISTORTION, pDistortion);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetDistortion(ISP_DEV IspDev, ISP_DISTORTION_S *pDistortion)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_DISTORTION, pDistortion);
	return ret;
}
#if 0
AW_S32 AW_MPI_ISP_SetTrigger(ISP_DEV IspDev, ISP_TRIGER_S *pTriger)
{
	AW_S32 ret = FAILURE;
	// ret = isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_TRIGGER, pTriger);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetTrigger(ISP_DEV IspDev, ISP_TRIGER_S *pTriger)
{
	AW_S32 ret = FAILURE;
	// ret = isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_TRIGGER, pTriger);
	return ret;
}
AW_S32 AW_MPI_ISP_SetLumMapPoint(ISP_DEV IspDev, ISP_SINGLE_S *plumP)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_LUM_POINT, plumP);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetLumMapPoint(ISP_DEV IspDev, ISP_SINGLE_S *plumP)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_LUM_POINT, plumP);
	return ret;
}
AW_S32 AW_MPI_ISP_SetGainMapPoint(ISP_DEV IspDev, ISP_SINGLE_S *pGainP)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_GAIN_POINT, pGainP);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetGainMapPoint(ISP_DEV IspDev, ISP_SINGLE_S *pGainP)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_GAIN_POINT, pGainP);
	return ret;
}
#endif
AW_S32 AW_MPI_ISP_AWB_SetCCMAttr(ISP_DEV IspDev, int color_temp, ISP_COLORMATRIX_ATTR_S *pstCCMAttr)
{
    switch (color_temp) {
    case 0:
        isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_CCM_LOW, pstCCMAttr);
        break;
    case 1:
        isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_CCM_MID, pstCCMAttr);
        break;
    case 2:
        isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_CCM_HIGH, pstCCMAttr);
        break;
    }
    isp_update(IspDev);
    return SUCCESS;
}
AW_S32 AW_MPI_ISP_AWB_GetCCMAttr(ISP_DEV IspDev, int color_temp, ISP_COLORMATRIX_ATTR_S *pstCCMAttr)
{
    struct isp_tuning_ccm_cfg isp_ccm;
    switch (color_temp) {
    case 0:
        isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_CCM_LOW, &isp_ccm);
        break;
    case 1:
        isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_CCM_MID, &isp_ccm);
        break;
    case 2:
        isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_CCM_HIGH, &isp_ccm);
        break;
    }
    memcpy(pstCCMAttr, &isp_ccm, sizeof(struct isp_tuning_ccm_cfg));
    
    return SUCCESS;
}
/*
// brightness
AW_S32 AW_MPI_ISP_SetBrightness(ISP_DEV IspDev, ISP_BRIGHTNESS_S *pBrightness)
{
	struct isp_dynamic_brightness_cfg isp_brightness;
    memcpy(&isp_brightness, pBrightness, sizeof(struct isp_dynamic_brightness_cfg));
	isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_BRIGHTNESS, &isp_brightness);
    isp_update(IspDev);
	return 0;
}
AW_S32 AW_MPI_ISP_GetBrightness(ISP_DEV IspDev, ISP_BRIGHTNESS_S *pBrightness)
{
	AW_S32 ret = FAILURE;
    struct isp_dynamic_brightness_cfg isp_brightness;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_BRIGHTNESS, pBrightness);
    memcpy(pBrightness, &isp_brightness, sizeof(struct isp_dynamic_brightness_cfg));
	return ret;
}
// contrast
AW_S32 AW_MPI_ISP_SetContrast(ISP_DEV IspDev, ISP_CONTRAST_S *pContrast)
{
	struct isp_dynamic_contrast_cfg isp_contrast;
    if (NULL == pContrast)
        return FALSE;
    memcpy(&isp_contrast, pContrast, sizeof(struct isp_dynamic_contrast_cfg));
	isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_CONTRAST, &isp_contrast);
    isp_update(IspDev);
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetContrast(ISP_DEV IspDev, ISP_CONTRAST_S *pContrast)
{
	struct isp_dynamic_contrast_cfg isp_contrast;
	isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_CONTRAST, &isp_contrast);
    memcpy(pContrast, &isp_contrast, sizeof(struct isp_dynamic_contrast_cfg));
	return SUCCESS;
}
// saturation
AW_S32 AW_MPI_ISP_SetSaturation(ISP_DEV IspDev, ISP_SATURAT\ION_S *pSta)
{
    int ret ;
	struct isp_dynamic_saturation_cfg isp_saturation;
    memcpy(&isp_saturation, pSta, sizeof(struct isp_dynamic_saturation_cfg));
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_SATURATION, &isp_saturation);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetSaturation(ISP_DEV IspDev, ISP_SATURATION_S *pSta)
{
	AW_S32 ret = FAILURE;
	struct isp_dynamic_saturation_cfg isp_saturation;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_SATURATION, &isp_saturation);
    memcpy(pSta, &isp_saturation, sizeof(struct isp_dynamic_saturation_cfg));
	return ret;
}
// hue
AW_S32 AW_MPI_ISP_SetHue(ISP_DEV IspDev, ISP_HUE_S *pHue)
{
    aloge("need implement");
    isp_update(IspDev);
    return FAILURE;
}
AW_S32 AW_MPI_ISP_GetHue(ISP_DEV IspDev, ISP_HUE_S *pHue)
{
    aloge("need implement");
    return FAILURE;
}
*/
// sharp
AW_S32 AW_MPI_ISP_SetSharpness(ISP_DEV IspDev, ISP_SHARPEN_ATTR_S *pstSharpenAttr)
{
	struct isp_dynamic_sharp_cfg isp_sharp_cfg;
	if (NULL == pstSharpenAttr)
		return FALSE;
	memcpy(&isp_sharp_cfg, pstSharpenAttr, sizeof(struct isp_dynamic_sharp_cfg));
	isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_SHARP , &isp_sharp_cfg);
    isp_update(IspDev); 
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetSharpness(ISP_DEV IspDev, ISP_SHARPEN_ATTR_S *pstSharpenAttr)
{
	struct isp_dynamic_sharp_cfg isp_sharp_cfg;
	isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_SHARP , &isp_sharp_cfg);
	memcpy(pstSharpenAttr, &isp_sharp_cfg, sizeof(struct isp_dynamic_sharp_cfg));
	return SUCCESS;
}
// 2DNR
AW_S32 AW_MPI_ISP_SetNRAttr(ISP_DEV IspDev, ISP_NR_ATTR_S *pstNRAttr)
{
	struct isp_dynamic_denoise_cfg isp_denoise_cfg;
	if (NULL == pstNRAttr)
		return FALSE;
    memcpy(&isp_denoise_cfg, pstNRAttr, sizeof(struct isp_dynamic_denoise_cfg));
	isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_DENOISE, &isp_denoise_cfg);
    isp_update(IspDev);
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_GetNRAttr(ISP_DEV IspDev, ISP_NR_ATTR_S *pstNRAttr)
{
	struct isp_dynamic_denoise_cfg isp_denoise_cfg;
	isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_DENOISE, &isp_denoise_cfg);
    memcpy(pstNRAttr, &isp_denoise_cfg, sizeof(struct isp_dynamic_denoise_cfg));
	return SUCCESS;
}
// 3DNR
AW_S32 AW_MPI_ISP_Set3NRAttr(ISP_DEV IspDev, ISP_3NR_ATTR_S *pstNRAttr)
{
	struct isp_dynamic_tdf_cfg isp_tdf_cfg;
	if (NULL == pstNRAttr)
		return FALSE;
    memcpy(&isp_tdf_cfg, pstNRAttr, sizeof(struct isp_dynamic_tdf_cfg));
	isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_TDF, &isp_tdf_cfg);
    isp_update(IspDev);
	return SUCCESS;
}
AW_S32 AW_MPI_ISP_Get3NRAttr(ISP_DEV IspDev, ISP_3NR_ATTR_S *pstNRAttr)
{
	struct isp_dynamic_tdf_cfg isp_tdf_cfg;
	isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_TDF, &isp_tdf_cfg);
    memcpy(pstNRAttr, &isp_tdf_cfg, sizeof(struct isp_dynamic_tdf_cfg));
	return SUCCESS;
}
// visualangle
AW_S32 AW_MPI_ISP_SetVisualAngle(ISP_DEV IspDev, ISP_VISUAL_ANGLE_S *pVisualAngle)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_VISUAL_ANGLE, pVisualAngle);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetVisualAngle(ISP_DEV IspDev, ISP_VISUAL_ANGLE_S *pVisualAngle)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_VISUAL_ANGLE, pVisualAngle);
	return ret;
}

// Global Constrast
AW_S32 AW_MPI_ISP_SetDynamicGtm(ISP_DEV IspDev, ISP_DYNAMIC_GTM_S *pGtm)
{
	AW_S32 ret = FAILURE;
	struct isp_dynamic_gtm_cfg isp_gtm;
	memcpy(&isp_gtm, pGtm, sizeof(struct isp_dynamic_gtm_cfg));
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_GTM, &isp_gtm);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetDynamicGtm(ISP_DEV IspDev, ISP_DYNAMIC_GTM_S *pGtm)
{
	AW_S32 ret = FAILURE;
	struct isp_dynamic_gtm_cfg isp_gtm;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_DYNAMIC, HW_ISP_CFG_DYNAMIC_GTM, &isp_gtm);
	memcpy(pGtm, &isp_gtm, sizeof(struct isp_dynamic_gtm_cfg));
	return ret;
}

#if 0
AW_S32 AW_MPI_ISP_SetGtm(ISP_DEV IspDev, ISP_GTM_S *pGtm)
{
	AW_S32 ret = FAILURE;
	ret = isp_set_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_GTM, pGtm);
    isp_update(IspDev);
	return ret;
}
AW_S32 AW_MPI_ISP_GetGtm(ISP_DEV IspDev, ISP_GTM_S *pGtm)
{
	AW_S32 ret = FAILURE;
	ret = isp_get_cfg(IspDev, HW_ISP_CFG_TUNING, HW_ISP_CFG_TUNING_GTM, pGtm);
	return ret;
}
#endif

#endif

