#include "TRlog.h"
#include "TRcommon.h"
#include "TParseConfig.h"
#include "TinaRecorder.h"

long long GetNowMs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)(tv.tv_sec * 1000 + tv.tv_usec / 1000);
}

long long GetNowUs()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (long long)(tv.tv_sec * 1000000 + tv.tv_usec);
}

int GetCaptureFormat(int format)
{
	switch(format)
	{
		case TR_PIXEL_YUV420SP:
			return V4L2_PIX_FMT_NV12;
		case TR_PIXEL_YVU420SP:
			return V4L2_PIX_FMT_NV21;
		case TR_PIXEL_YUV420P:
			return V4L2_PIX_FMT_YUV420;
		case TR_PIXEL_YVU420P:
			return V4L2_PIX_FMT_YVU420;
		case TR_PIXEL_YUV422SP:
			return V4L2_PIX_FMT_NV16;
		case TR_PIXEL_YVU422SP:
			return V4L2_PIX_FMT_NV61;
		case TR_PIXEL_YUV422P:
			return V4L2_PIX_FMT_YUV422P;
		case TR_PIXEL_YUYV422:
			return V4L2_PIX_FMT_YUYV;
		case TR_PIXEL_UYVY422:
			return V4L2_PIX_FMT_UYVY;
		case TR_PIXEL_YVYU422:
			return V4L2_PIX_FMT_YVYU;
		case TR_PIXEL_VYUY422:
			return V4L2_PIX_FMT_VYUY;
        case TR_PIXEL_MJPEG:
			return V4L2_PIX_FMT_MJPEG;
        case TR_PIXEL_H264:
			return V4L2_PIX_FMT_H264;
		default:
			TRerr("[%s] Capture pixel format %d is not supported\n", __func__, format);
			return -1;
	}
}

char *get_format_name(unsigned int format)
{
    if(format == V4L2_PIX_FMT_YUV422P)
        return "YUV422P";
    else if(format == V4L2_PIX_FMT_YUV420)
        return "YUV420";
    else if(format == V4L2_PIX_FMT_YVU420)
        return "YVU420";
    else if(format == V4L2_PIX_FMT_NV16)
        return "NV16";
    else if(format == V4L2_PIX_FMT_NV12)
        return "NV12";
    else if(format == V4L2_PIX_FMT_NV61)
        return "NV61";
    else if(format == V4L2_PIX_FMT_NV21)
        return "NV21";
    else if(format == V4L2_PIX_FMT_HM12)
        return "MB YUV420";
    else if(format == V4L2_PIX_FMT_YUYV)
        return "YUYV";
    else if(format == V4L2_PIX_FMT_YVYU)
        return "YVYU";
    else if(format == V4L2_PIX_FMT_UYVY)
        return "UYVY";
    else if(format == V4L2_PIX_FMT_VYUY)
        return "VYUY";
    else if(format == V4L2_PIX_FMT_MJPEG)
        return "MJPEG";
    else if(format == V4L2_PIX_FMT_H264)
        return "H264";
    else
        return NULL;
}

int GetAudioTRFormat(char *name)
{
    if(!name)
        return -1;

    if (strcmp(name, "PCM") == 0)
        return TR_AUDIO_PCM;
    else if (strcmp(name, "AAC") == 0)
        return TR_AUDIO_AAC;
    else if (strcmp(name, "MP3") == 0)
        return TR_AUDIO_MP3;
    else if (strcmp(name, "LPCM") == 0)
        return TR_AUDIO_LPCM;
    else {
        TRerr("[%s] not support audio format %s\n", __func__, name);
        return -1;
    }
}

int GetVideoTRFormat(char *name)
{
    if(!name)
        return -1;

    if (strcmp(name, "YVU420SP") == 0)
        return TR_PIXEL_YVU420SP;
    else if(strcmp(name, "YUV420SP") == 0)
        return TR_PIXEL_YUV420SP;
    else if(strcmp(name, "YUV420P") == 0)
        return TR_PIXEL_YUV420P;
    else if(strcmp(name, "YVU420P") == 0)
        return TR_PIXEL_YVU420P;
    else if(strcmp(name, "YUV422SP") == 0)
        return TR_PIXEL_YUV422SP;
    else if(strcmp(name, "YVU422SP") == 0)
        return TR_PIXEL_YVU422SP;
    else if(strcmp(name, "YUV422P") == 0)
        return TR_PIXEL_YUV422P;
    else if(strcmp(name, "YVU422P") == 0)
        return TR_PIXEL_YVU422P;
    else if(strcmp(name, "YUYV422") == 0)
        return TR_PIXEL_YUYV422;
    else if(strcmp(name, "UYVY422") == 0)
        return TR_PIXEL_UYVY422;
    else if(strcmp(name, "YVYU422") == 0)
        return TR_PIXEL_YVYU422;
    else if(strcmp(name, "VYUY422") == 0)
        return TR_PIXEL_VYUY422;
    else if(strcmp(name, "MJPEG") == 0)
        return TR_PIXEL_MJPEG;
    else if(strcmp(name, "H264") == 0)
        return TR_PIXEL_H264;
    else {
        TRerr("[%s] not support video format %s\n", __func__, name);
        return -1;
    }
    return -1;
}

char *ReturnVideoTRFormatText(int format)
{
    switch(format)
	{
		case TR_PIXEL_YUV420SP:
			return "YUV420SP";
		case TR_PIXEL_YVU420SP:
			return "YVU420SP";
		case TR_PIXEL_YUV420P:
			return "YUV420P";
		case TR_PIXEL_YVU420P:
			return "YVU420P";
		case TR_PIXEL_YUV422SP:
			return "YUV422SP";
		case TR_PIXEL_YVU422SP:
			return "YVU422SP";
		case TR_PIXEL_YUV422P:
			return "YUV422P";
		case TR_PIXEL_YUYV422:
			return "YUYV422";
		case TR_PIXEL_UYVY422:
			return "UYVY422";
		case TR_PIXEL_YVYU422:
			return "YVYU422";
		case TR_PIXEL_VYUY422:
			return "VYUY422";
        case TR_PIXEL_MJPEG:
			return "MJPEG";
        case TR_PIXEL_H264:
			return "H264";
		default:
			TRerr("[%s] format %d is not supported\n", __func__, format);
			return NULL;
	}
}

int GetJpegSourceFormat(int TRformat)
{
    switch(TRformat)
    {
        case TR_PIXEL_YUV420SP:
            return VENC_PIXEL_YUV420SP;
        case TR_PIXEL_YVU420SP:
            return VENC_PIXEL_YVU420SP;
        case TR_PIXEL_YUV420P:
            return VENC_PIXEL_YUV420P;
        case TR_PIXEL_YVU420P:
            return VENC_PIXEL_YVU420P;
        case TR_PIXEL_YUV422SP:
            return VENC_PIXEL_YUV422SP;
        case TR_PIXEL_YVU422SP:
            return VENC_PIXEL_YVU422SP;
        case TR_PIXEL_YUV422P:
            return VENC_PIXEL_YUV422P;
        case TR_PIXEL_YVU422P:
            return VENC_PIXEL_YVU422P;
        case TR_PIXEL_YUYV422:
            return VENC_PIXEL_YUYV422;
        case TR_PIXEL_UYVY422:
            return VENC_PIXEL_UYVY422;
        case TR_PIXEL_YVYU422:
            return VENC_PIXEL_YVYU422;
        case TR_PIXEL_VYUY422:
            return VENC_PIXEL_VYUY422;
        case TR_PIXEL_ARGB:
            return VENC_PIXEL_ARGB;
        case TR_PIXEL_RGBA:
            return VENC_PIXEL_RGBA;
        case TR_PIXEL_ABGR:
            return VENC_PIXEL_ABGR;
        case TR_PIXEL_BGRA:
            return VENC_PIXEL_BGRA;
        /* if format is MJPEG or H264,decoder output format is NV21 */
        case TR_PIXEL_MJPEG:
            return VENC_PIXEL_YVU420SP;
        case TR_PIXEL_H264:
            return VENC_PIXEL_YVU420SP;
        default:
			TRerr("[%s] input pixel format %d is not supported!\n", __func__, TRformat);
			return -1;
    }
}

int GetDecoderVideoSourceFormat(int format)
{
    switch(format)
    {
        case TR_PIXEL_MJPEG:
            return VIDEO_CODEC_FORMAT_MJPEG;
        case TR_PIXEL_H264:
            return VIDEO_CODEC_FORMAT_H264;
        default:
            TRerr("[%s] video input pixel format %d is not supported!\n",
                                                            __func__, format);
            return -1;
    }
}

int GetDisplayFormat(int format)
{
	switch(format)
	{
		case TR_PIXEL_YUV420SP:
			return VIDEO_PIXEL_FORMAT_NV12;
		case TR_PIXEL_YVU420SP:
			return VIDEO_PIXEL_FORMAT_NV21;
		case TR_PIXEL_YUYV422:
			return VIDEO_PIXEL_FORMAT_YUYV;
		case TR_PIXEL_UYVY422:
			return VIDEO_PIXEL_FORMAT_UYVY;
		case TR_PIXEL_YVYU422:
			return VIDEO_PIXEL_FORMAT_YVYU;
		case TR_PIXEL_VYUY422:
			return VIDEO_PIXEL_FORMAT_VYUY;
		default:
			TRerr("[%s] display format %d is not supported\n", __func__, format);
			return -1;
	}
}

int GetEncoderVideoSourceFormat(int format)
{
    switch(format)
    {
        case TR_PIXEL_YUV420SP:
            return VIDEO_PIXEL_YUV420_NV12;
        case TR_PIXEL_YVU420SP:
            return VIDEO_PIXEL_YUV420_NV21;
        case TR_PIXEL_YUV420P:
            return VIDEO_PIXEL_YUV420_YU12;
        case TR_PIXEL_YVU420P:
            return VIDEO_PIXEL_YVU420_YV12;
        case TR_PIXEL_YUV422SP:
            return VIDEO_PIXEL_YUV422SP;
        case TR_PIXEL_YVU422SP:
            return VIDEO_PIXEL_YVU422SP;
        case TR_PIXEL_YUV422P:
            return VIDEO_PIXEL_YUV422P;
        case TR_PIXEL_YVU422P:
            return VIDEO_PIXEL_YVU422P;
        case TR_PIXEL_YUYV422:
            return VIDEO_PIXEL_YUYV422;
        case TR_PIXEL_UYVY422:
            return VIDEO_PIXEL_UYVY422;
        case TR_PIXEL_YVYU422:
            return VIDEO_PIXEL_YVYU422;
        case TR_PIXEL_VYUY422:
            return VIDEO_PIXEL_VYUY422;
        case TR_PIXEL_ARGB:
            return VIDEO_PIXEL_ARGB;
        case TR_PIXEL_RGBA:
            return VIDEO_PIXEL_RGBA;
        case TR_PIXEL_ABGR:
            return VIDEO_PIXEL_ABGR;
        case TR_PIXEL_BGRA:
            return VIDEO_PIXEL_BGRA;
        case TR_PIXEL_TILE_32X32:
            return VIDEO_PIXEL_YUV420_MB32;
        case TR_PIXEL_TILE_128X32:
            return VIDEO_PIXEL_TILE_128X32;
        /* if format is MJPEG or H264,decoder output format is NV21 */
        case TR_PIXEL_MJPEG:
            return VIDEO_PIXEL_YUV420_NV21;
        case TR_PIXEL_H264:
            return VIDEO_PIXEL_YUV420_NV21;
        default:
            TRerr("[%s] video input pixel format %d is not supported!\n",
                                                            __func__, format);
            return -1;
    }
}

int GetEncoderAudioSourceFormat(int format)
{
    switch(format){
	case TR_AUDIO_PCM:
		return AUDIO_ENCODE_PCM_TYPE;
	case TR_AUDIO_AAC:
		return AUDIO_ENCODE_AAC_TYPE;
	case TR_AUDIO_MP3:
		return AUDIO_ENCODE_MP3_TYPE;
	case TR_AUDIO_LPCM:
		return AUDIO_ENCODE_LPCM_TYPE;
	default:
		TRerr("[%s] audio input pixel format %d is not supported!\n",
                                                             __func__, format);
		return -1;
	}
}

int GetEncoderAudioType(char *typename)
{
    int audiotype = -1;

    if (!strcmp(typename, "PCM"))
        audiotype = TR_AUDIO_PCM;
    else if(!strcmp(typename, "AAC"))
        audiotype = TR_AUDIO_AAC;
    else if(!strcmp(typename, "MP3"))
        audiotype = TR_AUDIO_MP3;
    else if(!strcmp(typename, "LPCM"))
        audiotype = TR_AUDIO_LPCM;
    else
        TRerr("[%s] audio encoder video type %s not support!\n", __func__, typename);

    return audiotype;
}

int GetEncoderVideoType(char *typename)
{
    int videotype = -1;

    if (!strcmp(typename, "H264"))
        videotype = TR_VIDEO_H264;
    else if(!strcmp(typename, "JPEG"))
        videotype = TR_VIDEO_MJPEG;
    else
		TRerr("[%s] video encoder video type %s not support!\n", __func__, typename);

    return videotype;
}

int ReturnEncoderVideoType(int format)
{
    switch (format) {
    case TR_VIDEO_H264:
        return VIDEO_ENCODE_H264;
    case TR_VIDEO_MJPEG:
        return VIDEO_ENCODE_JPEG;
    default:
		TRerr("[%s] video encoder video type %d not support!\n", __func__, format);
        return -1;
    }
}

int ReturnMuxerOuputType(char *typename)
{
    int muxertype = -1;

    if (!strcmp(typename, "TS"))
        muxertype = TR_OUTPUT_TS;
    else if(!strcmp(typename, "AAC"))
        muxertype = TR_OUTPUT_AAC;
    else if(!strcmp(typename, "MOV"))
        muxertype = TR_OUTPUT_MOV;
    else if(!strcmp(typename, "JPG"))
        muxertype = TR_OUTPUT_JPG;
    else if(!strcmp(typename, "MP3"))
        muxertype = TR_OUTPUT_MP3;
    else
        TRerr("[%s] output format not supported!\n", __func__);

    return muxertype;
}

int ReturnMediaAudioType(int type)
{
    switch (type) {
        case AUDIO_ENCODE_PCM_TYPE:
            return AUDIO_ENCODER_PCM_TYPE;
        case AUDIO_ENCODE_AAC_TYPE:
            return AUDIO_ENCODER_AAC_TYPE;
        case AUDIO_ENCODE_MP3_TYPE:
            return AUDIO_ENCODER_MP3_TYPE;
        case AUDIO_ENCODE_LPCM_TYPE:
            return AUDIO_ENCODER_LPCM_TYPE;
        default:
            TRerr("[%s] unkown audio type(%d)\n", __func__, type);
            return -1;
    }
}

int ReturnMediaVideoType(int type)
{
    switch (type) {
        case VIDEO_ENCODE_H264:
            return VENC_CODEC_H264;
        case VIDEO_ENCODE_JPEG:
            return VENC_CODEC_JPEG;
        default:
            TRwarn("[%s] warnning:cannot suppot this video type %d,use the default:h264\n",
                                                                                __func__, type);
            return VENC_CODEC_H264;
    }
}

void TRdebugEncoderAudioInfo(void *info)
{
    AudioEncodeConfig *audioConfig = (AudioEncodeConfig *)info;

    TRlog(TR_LOG, "***************** audio info *****************\n");
    TRlog(TR_LOG, "audioConfig.nType             : %d\n", audioConfig->nType);
    TRlog(TR_LOG, "audioConfig.nInChan           : %d\n", audioConfig->nInChan);
    TRlog(TR_LOG, "audioConfig.nInSamplerate     : %d\n", audioConfig->nInSamplerate);
    TRlog(TR_LOG, "audioConfig.nOutChan          : %d\n", audioConfig->nOutChan);
    TRlog(TR_LOG, "audioConfig.nOutSamplerate    : %d\n", audioConfig->nOutSamplerate);
    TRlog(TR_LOG, "audioConfig.nSamplerBits      : %d\n", audioConfig->nSamplerBits);
    TRlog(TR_LOG, "audioConfig.nFrameStyle       : %d\n", audioConfig->nFrameStyle);
}

void TRdebugEncoderVideoInfo(void *info)
{
    VideoEncodeConfig *videoConfig = (VideoEncodeConfig *)info;

    TRlog(TR_LOG, "***************** video info *****************\n");
    TRlog(TR_LOG, "videoConfig.nType             : %d\n", videoConfig->nType);
    TRlog(TR_LOG, "videoConfig.nInputYuvFormat   : %d\n", videoConfig->nInputYuvFormat);
    TRlog(TR_LOG, "videoConfig.nSrcWidth         : %d\n", videoConfig->nSrcWidth);
    TRlog(TR_LOG, "videoConfig.nSrcHeight        : %d\n", videoConfig->nSrcHeight);
    TRlog(TR_LOG, "videoConfig.nOutWidth         : %d\n", videoConfig->nOutWidth);
    TRlog(TR_LOG, "videoConfig.nOutHeight        : %d\n", videoConfig->nOutHeight);
    TRlog(TR_LOG, "videoConfig.nBitRate          : %d\n", videoConfig->nBitRate);
    TRlog(TR_LOG, "videoConfig.nFrameRate        : %d\n", videoConfig->nFrameRate);
    TRlog(TR_LOG, "videoConfig.bUsePhyBuf        : %d\n", videoConfig->bUsePhyBuf);
    TRlog(TR_LOG, "videoConfig.ratio             : %d\n", videoConfig->ratio);
}

void TRdebugMuxerMediaInfo(void *info)
{
    CdxMuxerMediaInfoT *mediainfo = (CdxMuxerMediaInfoT *)info;

    TRlog(TR_LOG_MUXER, "******************* mux mediainfo *****************************\n");
    TRlog(TR_LOG_MUXER, "videoNum                   : %d\n", mediainfo->videoNum);
    TRlog(TR_LOG_MUXER, "videoTYpe                  : %d\n", mediainfo->video.eCodeType);
    TRlog(TR_LOG_MUXER, "framerate                  : %d\n", mediainfo->video.nFrameRate);
    TRlog(TR_LOG_MUXER, "width                      : %d\n", mediainfo->video.nWidth);
    TRlog(TR_LOG_MUXER, "height                     : %d\n", mediainfo->video.nHeight);
    TRlog(TR_LOG_MUXER, "audioNum                   : %d\n", mediainfo->audioNum);
    TRlog(TR_LOG_MUXER, "audioFormat                : %d\n", mediainfo->audio.eCodecFormat);
    TRlog(TR_LOG_MUXER, "audioChannelNum            : %d\n", mediainfo->audio.nChannelNum);
    TRlog(TR_LOG_MUXER, "audioSmpleRate             : %d\n", mediainfo->audio.nSampleRate);
    TRlog(TR_LOG_MUXER, "audioBitsPerSample         : %d\n", mediainfo->audio.nBitsPerSample);
    TRlog(TR_LOG_MUXER, "**************************************************************\n");
}
