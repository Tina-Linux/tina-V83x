
/*
 ******************************************************************************
 *
 * vi_api.h
 *
 * Hawkview ISP - vi_api.h module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/04/01	VIDEO INPUT
 *
 *****************************************************************************
 */

#ifndef _AW_VI_API_H_
#define _AW_VI_API_H_

#include "common_vi.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif  /* __cplusplus */

AW_S32 AW_MPI_VI_Init(void);
AW_S32 AW_MPI_VI_Exit(void);
AW_S32 AW_MPI_VI_InitCh(AW_CHN ViCh);
AW_S32 AW_MPI_VI_ExitCh(AW_CHN ViCh);
AW_S32 AW_MPI_VI_SetChnAttr(AW_CHN ViCh, VI_ATTR_S *pstAttr);
AW_S32 AW_MPI_VI_GetChnAttr(AW_CHN ViCh, VI_ATTR_S *pstAttr);
AW_S32 AW_MPI_VI_EnableChn(AW_CHN ViCh);
AW_S32 AW_MPI_VI_DisableChn(AW_CHN ViCh);
AW_S32 AW_MPI_VI_GetFrame(AW_CHN ViCh, VI_FRAME_BUF_INFO_S *pstFrameInfo, AW_S32 s32MilliSec);
AW_S32 AW_MPI_VI_ReleaseFrame(AW_CHN ViCh, VI_FRAME_BUF_INFO_S *pstFrameInfo);
AW_S32 AW_MPI_VI_SaveFrame(AW_CHN ViCh, VI_FRAME_BUF_INFO_S *pstFrameInfo, char *path);
AW_S32 AW_MPI_VI_SetControl(AW_CHN ViCh, int cmd, int value);
AW_S32 AW_MPI_VI_GetControl(AW_CHN ViCh, int cmd, int *value);
AW_S32 AW_MPI_VI_GetIspId(AW_CHN ViCh, AW_S32 *id);
AW_S32 AW_MPI_VI_GetEvent(AW_CHN ViCh);

AW_S32 AW_MPI_OSD_SetFmt(AW_CHN ViCh);
AW_S32 AW_MPI_OSD_Update(AW_CHN ViCh, int on_off);

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif  /* __cplusplus */

#endif /*_AW_VI_API_H_*/

