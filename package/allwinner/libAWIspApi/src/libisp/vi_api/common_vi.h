
/*
 ******************************************************************************
 *
 * common_vi.h
 *
 * Hawkview ISP - common_vi.h module
 *
 * Copyright (c) 2016 by Allwinnertech Co., Ltd.  http://www.allwinnertech.com
 *
 * Version		  Author         Date		    Description
 *
 *   3.0		  Yang Feng   	2016/04/01	VIDEO INPUT
 *
 *****************************************************************************
 */

#ifndef _AW_COMMON_VI_H_
#define _AW_COMMON_VI_H_

#include "../include/V4l2Camera/sunxi_camera_v2.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif  /* __cplusplus */

#define AW_ERR_VI_INVALID_CHNID		-2

typedef unsigned char           AW_U8;
typedef unsigned short          AW_U16;
typedef unsigned int            AW_U32;

typedef signed char             AW_S8;
typedef short                   AW_S16;
typedef int                     AW_S32;
typedef AW_S32			AW_DEV;
typedef AW_S32			AW_CHN;
typedef unsigned long long	AW_U64;
typedef long long		AW_S64;

typedef void			AW_VOID;

#define SUCCESS  0
#define FAILURE  (-1)

typedef struct awVI_FRAME_BUF_INFO_S
{
	AW_U32          u32PoolId;
	AW_U32          u32Width;
	AW_U32          u32Height;
	AW_U32		u32PixelFormat;
	AW_U32		u32field;

	AW_U32          u32PhyAddr[3];
	AW_VOID         *pVirAddr[3];
	AW_U32          u32Stride[3];

	struct timeval stTimeStamp;
} VI_FRAME_BUF_INFO_S;

typedef struct awVI_ATTR_S
{
	enum v4l2_buf_type type;
	enum v4l2_memory memtype;
	struct v4l2_pix_format_mplane format;
	unsigned int nbufs;
	unsigned int nplanes;
	unsigned int fps;
	unsigned int capturemode;
	unsigned int use_current_win;
} VI_ATTR_S;

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif /*_AW_COMMON_VI_H_*/


