#include "videoDecorder.h"
#include "TRlog.h"

int DecoderInit(void *decoder)
{
	VideoStreamInfo videoInfo;
	VConfig vConfig;

	decoder_handle* dec = (decoder_handle*)decoder;

	/* create videodecoder */
	dec->pVideo = CreateVideoDecoder();
	if (dec->pVideo == NULL) {
		TRerr("[%s] create video decoder failed\n", __func__);
		return -1;
	}

	/* set the videoInfo */
	memset(&videoInfo, 0x00, sizeof(VideoStreamInfo));
	videoInfo.eCodecFormat = dec->Input_Fmt;

	if(dec->pCodecSpecificData) {
		videoInfo.pCodecSpecificData = dec->pCodecSpecificData;
		videoInfo.nCodecSpecificDataLen = dec->nCodecSpecificDataLen;
	}

	/* allocate mem */
	dec->memops = MemAdapterGetOpsS();
	if (dec->memops == NULL) {
		TRerr("[%s] MemAdapterGetOpsS failed\n", __func__);
		return -1;
	}
	CdcMemOpen(dec->memops);

	/* set the vConfig */
	memset(&vConfig, 0x00, sizeof(VConfig));
	vConfig.bDisable3D = 0;
	vConfig.bDispErrorFrame = 0;
	vConfig.bNoBFrames = 0;
	vConfig.bRotationEn = 0;
	vConfig.bScaleDownEn = 0;
	vConfig.nHorizonScaleDownRatio = 0;
	vConfig.nVerticalScaleDownRatio = 0;
	vConfig.nDeInterlaceHoldingFrameBufferNum = 0;
	vConfig.nDisplayHoldingFrameBufferNum = 0;
	vConfig.nRotateHoldingFrameBufferNum = 0;
	vConfig.nDecodeSmoothFrameBufferNum = 1;
	vConfig.nVbvBufferSize = 2*1024*1024;
	vConfig.eOutputPixelFormat = dec->Output_Fmt;
	vConfig.bThumbnailMode = 0;
	vConfig.memops = dec->memops;
	if (dec->ratio) {
		vConfig.bScaleDownEn = 1;
		vConfig.nHorizonScaleDownRatio = dec->ratio;
		vConfig.nVerticalScaleDownRatio = dec->ratio;
	}
    vConfig.nFrameBufferNum = 5;
	/* Load video decoder libs */
	AddVDPlugin();
	if ((InitializeVideoDecoder(dec->pVideo, &videoInfo, &vConfig)) != 0) {
		TRerr("[%s] Initialize Video Decoder failed\n", __func__);
		return -1;
	}

    dec->enable = 1;
	TRlog(TR_LOG_DECODER, " Initialize Video Decoder ok, eCodecFormat0x%x eOutputPixelFormat0x%x PhyOffset0x%x\n",
                                                    dec->Input_Fmt, dec->Output_Fmt, CONF_VE_PHY_OFFSET);

    return 0;
}

int DecoderDequeue(void *decoder, unsigned char *start, int64_t pts, int length)
{
	int RequireSize = 0;
	char *Buf, *RingBuf;
	int BufSize, RingBufSize;
	int ret, num;
	unsigned char *Data = NULL;
	VideoStreamDataInfo DataInfo;
	decoder_handle* dec = (decoder_handle*)decoder;
	RequireSize = length;
	Data = start;

    if((!Data) || (RequireSize <= 0)){
        TRerr("[%s] input parameter error,data addr %p size %d\n", __func__, Data, RequireSize);
        return -1;
    }

	/* Request Video Stream Buffer */
	if (RequestVideoStreamBuffer(dec->pVideo, RequireSize,
															(char **)&Buf, &BufSize,
															(char **)&RingBuf, &RingBufSize, 0) != 0) {
		TRerr("[%s] RequestVideoStreamBuffer failed\n", __func__);
		return -1;
	}
	/* Copy Video Stream Data to Buf and RingBuf */
	if (BufSize + RingBufSize < RequireSize) {
		TRerr("[%s] Request buffer failed, buffer is not enough\n", __func__);
		return -1;
	}
	if (BufSize >= RequireSize)
		memcpy(Buf, Data, RequireSize);
	else {
		memcpy(Buf, Data, BufSize);
		memcpy(RingBuf, Data+BufSize, RequireSize-BufSize);
	}

	/* Submit Video Stream Data */
	memset(&DataInfo, 0, sizeof(DataInfo));
	DataInfo.pData = Buf;
    DataInfo.nPts = pts;
	DataInfo.nLength = RequireSize;
	DataInfo.bIsFirstPart = 1;
	DataInfo.bIsLastPart = 1;
	if (SubmitVideoStreamData(dec->pVideo, &DataInfo, 0)) {
		TRerr("[%s] Submit Video Stream Data failed!\n", __func__);
		return -1;
	}

	/* decode stream */
	int endofstream = 0;
	int decodekeyframeonly = 0;
	int dropBFrameifdelay = 0;
	int64_t currenttimeus = 0;

	ret = DecodeVideoStream(dec->pVideo, endofstream, decodekeyframeonly,
												dropBFrameifdelay, currenttimeus);

	switch (ret) {
		case VDECODE_RESULT_KEYFRAME_DECODED:
		case VDECODE_RESULT_FRAME_DECODED:
		{
			num = ValidPictureNum(dec->pVideo, 0);
			if (num >= 0) {
				dec->videoPicture = RequestPicture(dec->pVideo, 0);
				if (dec->videoPicture == NULL) {
					TRerr("[%s] RequestPicture fail\n", __func__);
					break;
				}
				/* get Vir_Addr and Phy_Addr */
				dec->bufId = dec->videoPicture->nID;
				dec->nPts = dec->videoPicture->nPts;
				dec->mWidth = dec->videoPicture->nWidth;
				dec->mHeight = dec->videoPicture->nHeight;
				dec->mYuvSize = dec->mWidth * dec->mHeight * 3 / 2;
				// uv combine
				dec->Vir_Y_Addr = (unsigned char *)dec->videoPicture->pData0;
				dec->Vir_U_Addr = (unsigned char *)dec->videoPicture->pData1;
				dec->Phy_Y_Addr = (unsigned char *)dec->videoPicture->phyYBufAddr + CONF_VE_PHY_OFFSET;
				dec->Phy_C_Addr = (unsigned char *)dec->videoPicture->phyCBufAddr + CONF_VE_PHY_OFFSET;

				return 0;
			} else
				TRerr("[%s] ValidPictureNum error and num is %d\n", __func__, num);
			break;
		}
		case VDECODE_RESULT_OK:
		case VDECODE_RESULT_CONTINUE:
		case VDECODE_RESULT_NO_FRAME_BUFFER:
		case VDECODE_RESULT_NO_BITSTREAM:
		case VDECODE_RESULT_RESOLUTION_CHANGE:
		case VDECODE_RESULT_UNSUPPORTED:
		default:
			TRerr("[%s] Video decode stream error, ret is %d\n", __func__, ret);
	}
	return -1;
}

int DecoderaEnqueue(void *decoder, VideoPicture *videoPicture)
{
	decoder_handle* dec = (decoder_handle*)decoder;

	if (!dec->pVideo) {
		TRerr("[%s] pVideo is NULL\n", __func__);
		return -1;
	}
	if (!videoPicture) {
		TRerr("[%s] videoPicture is NULL\n", __func__);
		return -1;
	}

	ReturnPicture(dec->pVideo, videoPicture);

	return 0;
}

int DecoderRelease(void *decoder)
{
	decoder_handle* dec = (decoder_handle*)decoder;

	if (dec->pVideo)
		DestroyVideoDecoder(dec->pVideo);
	if (dec->memops)
		CdcMemClose(dec->memops);

	return 0;
}
