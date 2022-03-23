#include "mpp_osd.h"

int create_osd(OSD_Params*     pOSDParams)
{

	// 初始化区域参数
	strcpy((char*)pOSDParams->szText, "XXXX年XX月XX日 XX:XX:XX");
    pOSDParams->pTextBuffer = (unsigned char*)malloc(FONTSIZE * FONTSIZE / 2 / 8 * strlen((char*)pOSDParams->szText));
	memset(pOSDParams->pTextBuffer, 0, FONTSIZE * FONTSIZE / 2 / 8 * strlen((char*)pOSDParams->szText));
	
    int date_region_w = strlen((char*)pOSDParams->szText) * FONTSIZE / 2;
    int date_region_h = FONTSIZE;
    //pOSDParams->stRegionDate;
	//memset(&pOSDParams->stRegionDate, 0, sizeof(RGN_ATTR_S));
	pOSDParams->stRegionDate.enType                        = OVERLAY_RGN;
	pOSDParams->stRegionDate.unAttr.stOverlay.mPixelFmt    = MM_PIXEL_FORMAT_RGB_8888;
	pOSDParams->stRegionDate.unAttr.stOverlay.mSize.Width  = date_region_w;
	pOSDParams->stRegionDate.unAttr.stOverlay.mSize.Height = date_region_h;

	// 创建区域对象
    // RGN_HANDLE mOverlayHandleDate = 0;
	AW_MPI_RGN_Create(pOSDParams->mOverlayHandle, &pOSDParams->stRegionDate);

	// 配置区域对应的位图属性	
	// BITMAP_S stBmpDate;
	memset(&pOSDParams->stBmpDate, 0, sizeof(BITMAP_S));
	pOSDParams->stBmpDate.mPixelFormat = pOSDParams->stRegionDate.unAttr.stOverlay.mPixelFmt;
	pOSDParams->stBmpDate.mWidth       = pOSDParams->stRegionDate.unAttr.stOverlay.mSize.Width;
	pOSDParams->stBmpDate.mHeight      = pOSDParams->stRegionDate.unAttr.stOverlay.mSize.Height;
    int nSizeDate                      = BITMAP_S_GetdataSize(&pOSDParams->stBmpDate);
	pOSDParams->stBmpDate.mpData       = malloc(nSizeDate);
    

	// 将该区域叠加到viDev和ViCh绑定的通道
    RGN_CHN_ATTR_S stRgnChnAttr2;
    memset(&stRgnChnAttr2, 0, sizeof(RGN_CHN_ATTR_S));
	stRgnChnAttr2.bShow                            = TRUE;
	stRgnChnAttr2.enType                           = pOSDParams->stRegionDate.enType;
	stRgnChnAttr2.unChnAttr.stOverlayChn.stPoint.X = 32;
	stRgnChnAttr2.unChnAttr.stOverlayChn.stPoint.Y = 32;
	stRgnChnAttr2.unChnAttr.stOverlayChn.mLayer    = 0;
	AW_MPI_RGN_AttachToChn(pOSDParams->mOverlayHandle,  &pOSDParams->mMppChn, &stRgnChnAttr2);
    //////////////////////////////////////////	

	return 0;
}

int update_osd(OSD_Params*     pOSDParams, void* pTime)
{

	time_t curr_time = 0;
	time_t pre_time  = (time_t)pTime;

	time(&curr_time);
	if (curr_time - pre_time>= 1)
	{
		// 更新日期
		struct tm *time_info = gmtime(&curr_time);
		sprintf((char *)pOSDParams->szText, "%04d年%02d月%02d日 %02d:%02d:%02d"
			,time_info->tm_year + 1900
			,time_info->tm_mon + 1
			,time_info->tm_mday
			,time_info->tm_hour
			,time_info->tm_min
			,time_info->tm_sec
			);

		int count = strlen((char*)pOSDParams->szText);
		GenTextBuffer((char *)pOSDParams->szText, (char *)pOSDParams->pTextBuffer, pOSDParams->stBmpDate.mpData);
		AW_MPI_RGN_SetBitMap(pOSDParams->mOverlayHandle, &pOSDParams->stBmpDate);
		pre_time = curr_time;
	}

	return 0;
}

int destroy_osd(OSD_Params*     pOSDParams)
{
    // 销毁OSD
    AW_MPI_RGN_DetachFromChn(pOSDParams->mOverlayHandle, &pOSDParams->mMppChn);
    AW_MPI_RGN_Destroy(pOSDParams->mOverlayHandle);
    free(pOSDParams->pTextBuffer);
    free(pOSDParams->stBmpDate.mpData);

	return 0;
}

