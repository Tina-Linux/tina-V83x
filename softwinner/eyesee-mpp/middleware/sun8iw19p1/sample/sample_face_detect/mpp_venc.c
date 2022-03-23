#include "mpp_venc.h"

// VENC 回调函数
static ERRORTYPE MPPCallbackWrapper(void *cookie, MPP_CHN_S *pChn, MPP_EVENT_TYPE event, void *pEventData)
{
    VIDEO_FRAME_INFO_S *pFrame = (VIDEO_FRAME_INFO_S *)pEventData;

    switch (event)
    {
    case MPP_EVENT_RELEASE_VIDEO_BUFFER:
        if (pFrame != NULL)
        {
			alogv(" MPP_EVENT_RELEASE_VIDEO_BUFFER ");
        }
        break;

    default:
        break;
    }

    return SUCCESS;
}

int create_venc(VENC_Params* pVENCParams, VENC_CHN chn, int type)
{
    // 配置编码通道参数
    VENC_CHN_ATTR_S mVencChnAttr;
    memset(&mVencChnAttr, 0, sizeof(VENC_CHN_ATTR_S));
    mVencChnAttr.VeAttr.Type         = type;
    mVencChnAttr.VeAttr.SrcPicWidth  = pVENCParams->srcWidth;
    mVencChnAttr.VeAttr.SrcPicHeight = pVENCParams->srcHeight;
    mVencChnAttr.VeAttr.Rotate       = pVENCParams->rotate;
    if (pVENCParams->srcPixFmt == MM_PIXEL_FORMAT_YUV_AW_AFBC)
    {// 全志私有格式
        mVencChnAttr.VeAttr.PixelFormat = MM_PIXEL_FORMAT_YUV_AW_AFBC;
        pVENCParams->dstWidth  = pVENCParams->srcWidth;  //cannot compress_encoder
        pVENCParams->dstHeight = pVENCParams->srcHeight; //cannot compress_encoder
    }
    else
    {
        mVencChnAttr.VeAttr.PixelFormat = MM_PIXEL_FORMAT_YVU_SEMIPLANAR_420;  //TODO:为何不Pass数值
    }
    mVencChnAttr.VeAttr.Field = VIDEO_FIELD_FRAME;

    if (PT_H264 == mVencChnAttr.VeAttr.Type)
    {
        mVencChnAttr.VeAttr.AttrH264e.bByFrame  = TRUE;
        mVencChnAttr.VeAttr.AttrH264e.Profile   = pVENCParams->mEncUseProfile;
        mVencChnAttr.VeAttr.AttrH264e.PicWidth  = pVENCParams->dstWidth;
        mVencChnAttr.VeAttr.AttrH264e.PicHeight = pVENCParams->dstHeight;
        switch (pVENCParams->mRcMode)
        {
        case 1:
            mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264VBR;
            mVencChnAttr.RcAttr.mAttrH264Vbr.mMinQp = 10;
            mVencChnAttr.RcAttr.mAttrH264Vbr.mMaxQp = 40;
            break;
        case 2:
            mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264FIXQP;
            mVencChnAttr.RcAttr.mAttrH264FixQp.mIQp = 35;
            mVencChnAttr.RcAttr.mAttrH264FixQp.mPQp = 35;
            break;
        case 3:
            mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264QPMAP;
            break;
        case 0:
        default:
            mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H264CBR;
            mVencChnAttr.RcAttr.mAttrH264Cbr.mSrcFrmRate    = pVENCParams->mVideoFrameRate;
            mVencChnAttr.RcAttr.mAttrH264Cbr.fr32DstFrmRate = pVENCParams->mVideoFrameRate;
            mVencChnAttr.RcAttr.mAttrH264Cbr.mBitRate       = pVENCParams->mVideoBitRate;
            break;
        }
    }
    else if (PT_H265 == mVencChnAttr.VeAttr.Type)
    {
        mVencChnAttr.VeAttr.AttrH265e.mbByFrame  = TRUE;
        mVencChnAttr.VeAttr.AttrH265e.mProfile   = pVENCParams->mEncUseProfile;
        mVencChnAttr.VeAttr.AttrH265e.mPicWidth  = pVENCParams->dstWidth;
        mVencChnAttr.VeAttr.AttrH265e.mPicHeight = pVENCParams->dstHeight;
        switch (pVENCParams->mRcMode)
        {
        case 1:
            mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265VBR;
            mVencChnAttr.RcAttr.mAttrH265Vbr.mMinQp = 10;
            mVencChnAttr.RcAttr.mAttrH265Vbr.mMaxQp = 40;
            break;
        case 2:
            mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265FIXQP;
            mVencChnAttr.RcAttr.mAttrH265FixQp.mIQp = 35;
            mVencChnAttr.RcAttr.mAttrH265FixQp.mPQp = 35;
            break;
        case 3:
            mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265QPMAP;
            break;
        case 0:
        default:
            mVencChnAttr.RcAttr.mRcMode = VENC_RC_MODE_H265CBR;
            mVencChnAttr.RcAttr.mAttrH265Cbr.mSrcFrmRate    = pVENCParams->mVideoFrameRate;
            mVencChnAttr.RcAttr.mAttrH265Cbr.fr32DstFrmRate = pVENCParams->mVideoFrameRate;
            mVencChnAttr.RcAttr.mAttrH265Cbr.mBitRate       = pVENCParams->mVideoBitRate;
            break;
        }
    }
    else if (PT_MJPEG == mVencChnAttr.VeAttr.Type)
    {
        mVencChnAttr.VeAttr.AttrMjpeg.mbByFrame     = TRUE;
        mVencChnAttr.VeAttr.AttrMjpeg.mPicWidth     = pVENCParams->dstWidth;
        mVencChnAttr.VeAttr.AttrMjpeg.mPicHeight    = pVENCParams->dstHeight;
        mVencChnAttr.RcAttr.mRcMode                 = VENC_RC_MODE_MJPEGCBR;
        mVencChnAttr.RcAttr.mAttrMjpegeCbr.mBitRate = pVENCParams->mVideoBitRate;
    }
    else if (PT_JPEG == mVencChnAttr.VeAttr.Type)
    {
        mVencChnAttr.VeAttr.AttrJpeg.bByFrame     = TRUE;
        mVencChnAttr.VeAttr.AttrJpeg.PicWidth     = pVENCParams->srcWidth;
        mVencChnAttr.VeAttr.AttrJpeg.PicHeight    = pVENCParams->srcHeight;
        mVencChnAttr.VeAttr.AttrJpeg.BufSize      = ((((pVENCParams->srcWidth * pVENCParams->srcHeight * 3) >> 2) + 1023) >> 10) << 10;
        mVencChnAttr.VeAttr.AttrJpeg.MaxPicWidth  = 0;
        mVencChnAttr.VeAttr.AttrJpeg.MaxPicHeight = 0;
        mVencChnAttr.VeAttr.AttrJpeg.bSupportDCF  = FALSE;
    }

    // 创建编码通道
    VENC_CHN mVeChn = chn;
    AW_MPI_VENC_CreateChn(mVeChn, &mVencChnAttr);

    // 设置FrameRate
    VENC_FRAME_RATE_S stFrameRate;
    stFrameRate.SrcFrmRate = stFrameRate.DstFrmRate = pVENCParams->mVideoFrameRate;
    AW_MPI_VENC_SetFrameRate(mVeChn, &stFrameRate);

    // 设置回调函数
    MPPCallbackInfo cbInfo;
    cbInfo.cookie   = pVENCParams;
    cbInfo.callback = (MPPCallbackFuncType)&MPPCallbackWrapper;
    AW_MPI_VENC_RegisterCallback(mVeChn, &cbInfo);
   
    // 启动编码通道
    AW_MPI_VENC_StartRecvPic(mVeChn);

    return 0;
}


int destroy_venc(VENC_CHN chn)
{
     // 关闭编码通道
    AW_MPI_VENC_StopRecvPic(chn);

    // 重置编码通道
    AW_MPI_VENC_ResetChn(chn);

    // 销毁编码通道
    AW_MPI_VENC_DestroyChn(chn);
    chn = MM_INVALID_CHN;

    return 0;
}

int save_jpeg_venc(VENC_CHN mVeChn, VENC_CROP_CFG_S* pCrop_cfg, VIDEO_FRAME_INFO_S *pVideoFrame, char* szPicPath)
{
    // Crop尺寸16对齐
	pCrop_cfg->Rect.X      = ALIGN(pCrop_cfg->Rect.X, 16);
	pCrop_cfg->Rect.Y      = ALIGN(pCrop_cfg->Rect.Y, 16);
	pCrop_cfg->Rect.Width  = ALIGN(pCrop_cfg->Rect.Width, 16);
	pCrop_cfg->Rect.Height = ALIGN(pCrop_cfg->Rect.Height, 16);

    // 重置编码通道
    AW_MPI_VENC_StopRecvPic(mVeChn);
    AW_MPI_VENC_ResetChn(mVeChn);
    AW_MPI_VENC_StartRecvPic(mVeChn);
    
    // 更新编码通道参数
    VENC_CHN_ATTR_S mVencChnAttr;
    AW_MPI_VENC_GetChnAttr(mVeChn, &mVencChnAttr);
    mVencChnAttr.VeAttr.AttrJpeg.PicWidth     = pCrop_cfg->Rect.Width;
    mVencChnAttr.VeAttr.AttrJpeg.PicHeight    = pCrop_cfg->Rect.Height;
    AW_MPI_VENC_SetChnAttr(mVeChn, &mVencChnAttr);
    AW_MPI_VENC_SetCrop(mVeChn, pCrop_cfg);
    
	// 送入编码通道
	AW_MPI_VENC_SendFrame(mVeChn, pVideoFrame, 0);
    
	// 读取编码结果
    VENC_PACK_S mpPack;
	VENC_STREAM_S vencFrame;
	memset(&vencFrame, 0, sizeof(VENC_STREAM_S));
	vencFrame.mpPack = &mpPack;
	vencFrame.mPackCount = 1;
	int result = AW_MPI_VENC_GetStream(mVeChn, &vencFrame, 100);
	if (SUCCESS == result)
	{
        alogv("Got frame, pts = %llu, seq = %d", vencFrame.mpPack->mPTS, vencFrame.mSeq);
		FILE* fd_out = fopen(szPicPath, "wb");
        if (fd_out == NULL)
        {
            aloge("Can't open file %s", szPicPath);
            exit(-1);
        }
		if (vencFrame.mpPack->mLen0)
		{
			fwrite(vencFrame.mpPack->mpAddr0, 1, vencFrame.mpPack->mLen0, fd_out);
		}
		if (vencFrame.mpPack->mLen1)
		{
			fwrite(vencFrame.mpPack->mpAddr1, 1, vencFrame.mpPack->mLen1, fd_out);
		}
		fclose(fd_out);
		AW_MPI_VENC_ReleaseStream(mVeChn, &vencFrame);
	}
	return 0;
}