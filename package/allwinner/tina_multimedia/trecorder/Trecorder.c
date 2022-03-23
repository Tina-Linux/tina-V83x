#define LOG_TAG "TRecorder"
#include <stdio.h>
#include <stddef.h>
#include <sys/ioctl.h>
#include "tlog.h"
#include "TRlog.h"
#include "Trecorder.h"
#include "TinaRecorder.h"
#include "pcm_multi.h"
#include "audioInPort.h"
#include "TParseConfig.h"
#include "TRcommon.h"
#include "dataqueue.h"
#include "tina_multimedia_version.h"

/* mark:
 * TR_PROMPT | TR_WARN | TR_PRINT | TR_LOG | TR_LOG_VIDEO
 * | TR_LOG_AUDIO | TR_LOG_DISPLAY | TR_LOG_ENCODER
 * | TR_LOG_MUXER | TR_LOG_DECODER
 */
unsigned int TR_log_mask = 0;

typedef enum
{
    T_RECORD_STATUS_INIT,
    T_RECORD_STATUS_INITIALIZED,
    T_RECORD_STATUS_DATA_SOURCE_CONFIGURED,
    T_RECORD_STATUS_PREPARED,
    T_RECORD_STATUS_RECORDING,
    T_RECORD_STATUS_RELEASED,
}TRecordStatus;

typedef struct TrecoderContext
{
    TinaRecorder*               mTinaRecorder;
    vInPort*                    mVideoInPort;
    aInPort*                    mAudioInPort;
    dispOutPort*                mDispOutPort;
    rOutPort*                   mRecorderOutPort;
    int                         mStatus;
    int                         mFlags;

    //input video args:
    int                         mVideoIndex;
    int                         mEnableVideoInput;
    int                         mInputVideoFormat;
    int                         mInputVideoFramerate;
    int                         mInputVideoYuvWidth;
    int                         mInputVideoYuvHeight;

    //input audio args:
    int                         mAudioIndex;
    int                         mEnableAudioInput;
    int                         mInputAudioFormat;
    int                         mInputAudioSamplerate;
    int                         mInputAudioChannels;
    int                         mInputAudioSampleBits;

    //preview args:
    int                         mEnablePreview;
    int                         mPreviewRotate;
	int                         route;
    videoZorder                 mPreviewZorder;
    VoutRect                    mPreviewRect;
	VoutRect                    mPreviewSrcRect;
    VoutRect                    mWindowRect;

    //decoder args:
    int                         mEnableDecoder;

    void*                       mUserData;
    TRecorderCallback           mCallback;

    //default flags
    int                         mUserInitFlag;
    int                         mUserLinkFlag;
}TrecoderContext;

static int TRparseVideoConfig(void *hdl);
static int TRparseAudioConfig(void *hdl);
static int TRparseDispConfig(void *hdl);
static int TRparseOutputConfig(void *hdl);

int CallbackFromTinaRecorder(void* pUserData, int msg, void* param)
{
    TrecoderContext *trContext = (TrecoderContext *)pUserData;
    if(trContext == NULL){
        TRerr("[%s] trecorderContext is null", __func__);
        return -1;
    }
    int appMsg = -1;
    switch(msg)
    {
        case TINA_RECORD_ONE_FILE_COMPLETE:
        {
            appMsg = T_RECORD_ONE_FILE_COMPLETE;
	    break;
        }
        case TINA_RECORD_ONE_AAC_FILE_COMPLETE:
        {
            appMsg = T_RECORD_ONE_AAC_FILE_COMPLETE;
	    break;
        }
        default:
        {
            TRwarn("[%s] warning: unknown callback from tinarecorder", __func__);
            break;
        }
    }
    if((appMsg != -1) && (trContext->mCallback != NULL)){
        trContext->mCallback(trContext->mUserData,appMsg,param);
    }
    return 0;
}


TrecorderHandle *CreateTRecorder()
{
	TrecoderContext* trecorderContext = (TrecoderContext*) \
										malloc(sizeof(TrecoderContext));
	if(trecorderContext == NULL){
		TRerr("[%s] CreateTRecorder fail", __func__);
		return NULL;
	}
    LogTinaVersionInfo();
	memset(trecorderContext,0,sizeof(TrecoderContext));
	trecorderContext->mTinaRecorder = CreateTinaRecorder();
	if(trecorderContext->mTinaRecorder == NULL){
		TRerr("[%s]CreateTinaRecorder ERR", __func__);
		free(trecorderContext);
		return NULL;
	}

	return (TrecorderHandle*)trecorderContext;
}

int TRsetCamera(TrecorderHandle *hdl,int index)
{
	TrecoderContext* trContext = (TrecoderContext*)hdl;
	int ret = 0;
	CHECK_ERROR_RETURN(trContext);
	/* create the vInPort */
	if(index != T_CAMERA_DISABLE) {
		trContext->mVideoInPort = CreateTinaVinport();
		if(trContext->mVideoInPort == NULL) {
			TRerr("[%s] CreateTinaVinport err\n", __func__);
			ret = -1;
		} else {
			trContext->mVideoInPort->open(trContext->mVideoInPort,index);
			ret = trContext->mTinaRecorder->setVideoInPort(trContext->mTinaRecorder,
															trContext->mVideoInPort);
			trContext->mVideoIndex = index;
			trContext->mEnableVideoInput = 1;
            TRparseVideoConfig(trContext);
		}
	} else {
		ret = trContext->mTinaRecorder->setVideoInPort(trContext->mTinaRecorder,NULL);
		trContext->mVideoInPort = NULL;
		TRlog(TR_LOG, "[%s] index not right\n", __func__);
	}

	return ret;
}

int TRsetAudioSrc(TrecorderHandle *hdl,int index){
	TrecoderContext* trContext = (TrecoderContext*)hdl;
	int ret = 0;
	CHECK_ERROR_RETURN(trContext);
	/* create the aInPort */
	if(index != T_AUDIO_MIC_DISABLE) {
		trContext->mAudioInPort = CreateAlsaAport(index);
		if(trContext->mAudioInPort == NULL) {
			TRerr("[%s] CreateAlsaAport ERR\n", __func__);
			return -1;
		} else {
			ret = trContext->mTinaRecorder->setAudioInPort(trContext->mTinaRecorder,
															trContext->mAudioInPort);
            trContext->mAudioIndex = index;
			trContext->mEnableAudioInput = 1;
            TRparseAudioConfig(trContext);
		}
	} else {
		ret = trContext->mTinaRecorder->setAudioInPort(trContext->mTinaRecorder,NULL);
		trContext->mAudioInPort = NULL;
		TRlog(TR_LOG, "[%s] index not right\n", __func__);
	}

	return ret;
}

int TRsetPreview(TrecorderHandle *hdl,int index){
	TrecoderContext* trContext = (TrecoderContext*)hdl;
	int ret = 0;
	CHECK_ERROR_RETURN(trContext);
	/* create the dispOutPort */
	if(index != T_DISP_LAYER_DISABLE) {
		trContext->mDispOutPort = CreateVideoOutport(index);
		if(trContext->mDispOutPort == NULL){
			TRerr("[%s] CreateVideoOutport ERR", __func__);
			return -1;
		} else {
			trContext->mEnablePreview = 1;
			ret = trContext->mTinaRecorder->setDispOutPort(trContext->mTinaRecorder,
															trContext->mDispOutPort);
            TRparseDispConfig(trContext);
		}
	} else {
		ret = trContext->mTinaRecorder->setDispOutPort(trContext->mTinaRecorder,NULL);
		trContext->mDispOutPort = NULL;
		TRlog(TR_LOG, "[%s] index not right\n", __func__);
	}

	return ret;
}

int TRsetOutput(TrecorderHandle *hdl,char *url)
{
	TrecoderContext* trContext = (TrecoderContext*)hdl;
	int ret = 0;

	CHECK_ERROR_RETURN(trContext);
	/* create the OutPort */
	trContext->mRecorderOutPort = (rOutPort*)malloc(sizeof(rOutPort));
	if(trContext->mRecorderOutPort == NULL) {
		TRerr("[%s] TRsetOutput:Create mRecorderOutPort err", __func__);
		return -1;
	}
	/*  */
	if(url == NULL) {
		free(trContext->mRecorderOutPort);
		trContext->mRecorderOutPort = NULL;
		ret = trContext->mTinaRecorder->setOutput(trContext->mTinaRecorder,NULL);
	} else {
		strcpy(trContext->mRecorderOutPort->url,url);
		trContext->mRecorderOutPort->enable = 1;
		ret = trContext->mTinaRecorder->setOutput(trContext->mTinaRecorder,
												trContext->mRecorderOutPort);
        TRparseOutputConfig(trContext);
	}

	return ret;
}

int TRstart(TrecorderHandle *hdl,int flags){
	TrecoderContext* trContext = (TrecoderContext*)hdl;
	int ret = 0;
	int trFlags;
	CHECK_ERROR_RETURN(trContext);
	CHECK_ERROR_RETURN(trContext->mTinaRecorder);
	CHECK_ERROR_RETURN(trContext->mTinaRecorder->start);
	switch(flags)
	{
		case T_PREVIEW:
			trFlags = FLAGS_PREVIEW;
			break;
		case T_RECORD:
			trFlags = FLAGS_RECORDING;
			break;
		case T_ALL:
			trFlags = FLAGS_ALL;
			break;
		default:
			TRerr("[%s] unknow start flags %d!\n", __func__, flags);
			return -1;
	}
	ret = trContext->mTinaRecorder->start(trContext->mTinaRecorder,trFlags);
	return ret;
}

int TRstop(TrecorderHandle *hdl,int flags){
	TrecoderContext* trContext = (TrecoderContext*)hdl;
	int ret = 0;
	int trFlags = 0;
	CHECK_ERROR_RETURN(trContext);
	CHECK_ERROR_RETURN(trContext->mTinaRecorder);
	CHECK_ERROR_RETURN(trContext->mTinaRecorder->stop);
	switch(flags)
	{
		case T_PREVIEW:
			trFlags = FLAGS_PREVIEW;
			break;
		case T_RECORD:
			trFlags = FLAGS_RECORDING;
			break;
		case T_ALL:
			trFlags = FLAGS_ALL;
			break;
		default:
			TRerr("[%s] unknow stop flags %d!\n", __func__, flags);
			return -1;
	}
	trContext->mFlags = trFlags;

	ret = trContext->mTinaRecorder->stop(trContext->mTinaRecorder,(int)trFlags);

	if(trFlags == FLAGS_ALL || (trFlags == trContext->mTinaRecorder->startFlags)) {
		/* deinit aport */
		if(trContext->mAudioInPort) {
			TP_CHECK(trContext->mTinaRecorder->aport.deinit);
			trContext->mTinaRecorder->aport.deinit(&trContext->mTinaRecorder->aport);
			DestroyAlsaAport(trContext->mAudioInPort);
			trContext->mAudioInPort = NULL;
		}
		/* deinit vport */
		if(trContext->mVideoInPort) {
			TP_CHECK(trContext->mTinaRecorder->vport.releasebuf);
			TP_CHECK(trContext->mTinaRecorder->vport.close);
			trContext->mTinaRecorder->vport.releasebuf(&trContext->mTinaRecorder->vport,3);
			trContext->mTinaRecorder->vport.WMRelease(&trContext->mTinaRecorder->vport);
			trContext->mTinaRecorder->vport.close(&trContext->mTinaRecorder->vport);
			//a problem here,so we use free,fixme
			free(trContext->mVideoInPort);
			//DestroyTinaVinport(trContext->mVideoInPort);
			trContext->mVideoInPort = NULL;
		}
		/* deinit dispport */
		if(trContext->mDispOutPort)
		{
			TP_CHECK(trContext->mTinaRecorder->dispport.deinit);
			if(trContext->mTinaRecorder->dispport.disp_fd != 0)
				trContext->mTinaRecorder->dispport.deinit(&trContext->mTinaRecorder->dispport);
			DestroyVideoOutport(trContext->mDispOutPort);
			trContext->mVideoInPort = NULL;
		}

		/* release TinaRecorder */
		if(trContext->mTinaRecorder) {
			TP_CHECK(trContext->mTinaRecorder->release);
			trContext->mTinaRecorder->release(trContext->mTinaRecorder);
			DestroyTinaRecorder(trContext->mTinaRecorder);
			trContext->mTinaRecorder = NULL;
		}
		/* free mRecorderOutPort */
		if(trContext->mRecorderOutPort) {
			free(trContext->mRecorderOutPort);
			trContext->mRecorderOutPort = NULL;
		}
	} else if(trFlags == FLAGS_PREVIEW) {
		//fixme,add here
	} else {
		//fixme,add here
	}

    return ret;
}

int TRrelease(TrecorderHandle *hdl){
	TrecoderContext* trContext = (TrecoderContext*)hdl;
	int ret = 0;
	CHECK_ERROR_RETURN(trContext);
	free(trContext);

	return ret;
}

int TRprepare(TrecorderHandle *hdl){
	TrecoderContext* trContext = (TrecoderContext*)hdl;
    TinaRecorder *hdlTina = trContext->mTinaRecorder;
	int ret = 0;
	CHECK_ERROR_RETURN(trContext);
	CHECK_ERROR_RETURN(trContext->mTinaRecorder);
	TP_CHECK(trContext->mTinaRecorder->prepare);
	TRlog(TR_LOG, "prepare handle:%p\n",trContext);

    if(trContext->mAudioInPort)
    {
	    TP_CHECK(trContext->mTinaRecorder->aport.init);

		ret = trContext->mTinaRecorder->aport.init(&trContext->mTinaRecorder->aport,trContext->mEnableAudioInput,
                                                trContext->mInputAudioFormat,trContext->mInputAudioSamplerate,
                                                trContext->mInputAudioChannels,trContext->mInputAudioSampleBits);
        if (ret < 0) {
			TRerr("[%s] aport init error, enable %d format %d samplerate %d "
					"channels %d samplebits %d\n", __func__, trContext->mEnableAudioInput,
					trContext->mInputAudioFormat, trContext->mInputAudioSamplerate,
					trContext->mInputAudioChannels,trContext->mInputAudioSampleBits);
			return ret;
		} else
			TRlog(TR_LOG_AUDIO, "aport init ok, enable %d format %d "
					"samplerate %d channels %d samplebits %d\n", trContext->mEnableAudioInput,
					trContext->mInputAudioFormat, trContext->mInputAudioSamplerate,
					trContext->mInputAudioChannels,trContext->mInputAudioSampleBits);
    }

    if(trContext->mVideoInPort)
    {
		TP_CHECK(trContext->mTinaRecorder->vport.init);
		TP_CHECK(trContext->mTinaRecorder->vport.requestbuf);

		ret = trContext->mTinaRecorder->vport.init(&trContext->mTinaRecorder->vport,trContext->mEnableVideoInput,
                                                trContext->mInputVideoFormat,trContext->mInputVideoFramerate,
                                                trContext->mInputVideoYuvWidth,trContext->mInputVideoYuvHeight,
                                                hdlTina->vport.SubWidth,hdlTina->vport.SubHeight,
                                                hdlTina->vport.use_isp,hdlTina->vport.rot_angle,
                                                trContext->mTinaRecorder->use_wm);
        if (ret < 0) {
			TRerr("[%s] vport[%d] init error, enable %d format %d framerate %d "
					"MainWidth %d MainHeight %d scale_down_enable %d SubWith %d "
					"SubHeight %d rot_angle %d use_wm %d\n", __func__,
					hdlTina->vport.mCameraIndex, trContext->mEnableVideoInput,
					trContext->mInputVideoFormat, trContext->mInputVideoFramerate,
					trContext->mInputVideoYuvWidth, trContext->mInputVideoYuvHeight,
					hdlTina->vport.use_isp, hdlTina->vport.SubWidth, hdlTina->vport.SubHeight,
					hdlTina->vport.rot_angle, trContext->mTinaRecorder->use_wm);
			return ret;
		} else{
		    trContext->mInputVideoYuvWidth = trContext->mTinaRecorder->vport.MainWidth;
            trContext->mInputVideoYuvHeight = trContext->mTinaRecorder->vport.MainHeight;
			TRlog(TR_LOG_VIDEO, "vport[%d] init ok, enable %d format %d "
					"framerate %d MainWidth %d MainHeight %d "
					"scale_down_enable %d SubWith %d SubHeight %d "
					"rot_angle %d use_wm %d\n",
					hdlTina->vport.mCameraIndex, trContext->mEnableVideoInput,
					trContext->mInputVideoFormat, trContext->mInputVideoFramerate,
					trContext->mInputVideoYuvWidth, trContext->mInputVideoYuvHeight,
					hdlTina->vport.use_isp, hdlTina->vport.SubWidth, hdlTina->vport.SubHeight,
					hdlTina->vport.rot_angle, trContext->mTinaRecorder->use_wm);
        }
		ret = trContext->mTinaRecorder->vport.WMInit(&trContext->mTinaRecorder->vport.WM_info, "/rom/etc/res/wm_540p_");
        if(ret < 0){
            TRerr("[%s] wm_res init error\n", __func__);
            trContext->mTinaRecorder->use_wm = 0;
        }
		ret = trContext->mTinaRecorder->vport.requestbuf(&trContext->mTinaRecorder->vport,hdlTina->vport.buf_num);
        if(ret < 0){
            TRerr("[%s] vport %d requestbuf error,count is %d\n",
                    __func__, trContext->mTinaRecorder->vport.mCameraIndex, hdlTina->vport.buf_num);
            return ret;
        }

	}

    if(trContext->mDispOutPort)
    {
	    TP_CHECK(trContext->mTinaRecorder->dispport.init);

		trContext->mTinaRecorder->dispport.init(&trContext->mTinaRecorder->dispport,trContext->mEnablePreview,
                                                            trContext->mPreviewRotate,&trContext->mPreviewRect);
        trContext->mWindowRect.width = trContext->mTinaRecorder->dispport.getScreenWidth(&trContext->mTinaRecorder->dispport);
        trContext->mWindowRect.height = trContext->mTinaRecorder->dispport.getScreenHeight(&trContext->mTinaRecorder->dispport);
        if(trContext->mPreviewRect.width > trContext->mWindowRect.width
                || trContext->mPreviewRect.height > trContext->mWindowRect.height){
            TRwarn("[%s] setting Rect is out of range Screen Rect,setting the maximum value\n", __func__);
            TRwarn("ScreenWidth %u ScreenHeight %u PreviewWidth %u PreviewHeight %u\n",
                                                trContext->mWindowRect.width, trContext->mWindowRect.height,
                                                trContext->mPreviewRect.width, trContext->mPreviewRect.height);
            if(trContext->mPreviewRect.width > trContext->mWindowRect.width){
                trContext->mPreviewRect.width = trContext->mWindowRect.width;
                hdlTina->dispport.rect.width = trContext->mWindowRect.width;
            }
            if(trContext->mPreviewRect.height > trContext->mWindowRect.height){
                trContext->mPreviewRect.height = trContext->mWindowRect.height;
                hdlTina->dispport.rect.height = trContext->mWindowRect.height;
            }
        }
        trContext->mTinaRecorder->dispport.setRect(&trContext->mTinaRecorder->dispport,&trContext->mPreviewRect);
		trContext->mTinaRecorder->dispport.setRoute(&trContext->mTinaRecorder->dispport,VIDEO_SRC_FROM_CAM);
        trContext->mTinaRecorder->dispport.SetZorder(&trContext->mTinaRecorder->dispport,trContext->mPreviewZorder);
		trContext->mTinaRecorder->dispport.setEnable(&trContext->mTinaRecorder->dispport, 0);

        TRlog(TR_LOG_DISPLAY, "dispoutport init ok, enable %d rect.x %d rect.y %d "
            "rect.width %d rect.height %d zorder %d ScreenWidth %u ScreenHeight %u\n",
            trContext->mEnablePreview, trContext->mPreviewRect.x, trContext->mPreviewRect.y,
            trContext->mPreviewRect.width, trContext->mPreviewRect.height, trContext->mPreviewZorder,
            trContext->mWindowRect.width, trContext->mWindowRect.height);
    }

	ret = trContext->mTinaRecorder->prepare(trContext->mTinaRecorder);
    if(ret < 0){
        TRerr("[%s] TinaRecorder prepare error\n", __func__);
        return -1;
    }

    if (!trContext->mUserLinkFlag)
		ret = trContext->mTinaRecorder->deflink(trContext->mTinaRecorder);

	return ret;
}

int TRreset(TrecorderHandle *hdl) {
	TrecoderContext* trContext = (TrecoderContext*)hdl;
	TinaRecorder *hdlTina = trContext->mTinaRecorder;
	int ret = 0;

	CHECK_ERROR_RETURN(trContext);
	TP_CHECK(trContext->mTinaRecorder);
	TP_CHECK(trContext->mTinaRecorder->reset);

	if(trContext->mAudioInPort) {
		TP_CHECK(hdlTina->aport.deinit);
		hdlTina->aport.deinit(&hdlTina->aport);
		DestroyAlsaAport(trContext->mAudioInPort);
		trContext->mAudioInPort = NULL;
	}

	if(trContext->mVideoInPort) {
		TP_CHECK(hdlTina->vport.releasebuf);
		TP_CHECK(hdlTina->vport.close);
		hdlTina->vport.releasebuf(&hdlTina->vport,3);
		hdlTina->vport.close(&hdlTina->vport);
		DestroyTinaVinport(trContext->mVideoInPort);
		trContext->mVideoInPort = NULL;
		if (hdlTina->vport.use_wm == 1)
			hdlTina->vport.WMRelease(&hdlTina->vport);
	}

	if(trContext->mDispOutPort) {
	    TP_CHECK(hdlTina->dispport.deinit);
		hdlTina->dispport.deinit(&hdlTina->dispport);
		DestroyVideoOutport(trContext->mDispOutPort);
		trContext->mDispOutPort = NULL;
	}

	if(trContext->mRecorderOutPort) {
		free(trContext->mRecorderOutPort);
		trContext->mRecorderOutPort = NULL;
	}

	ret = trContext->mTinaRecorder->reset(trContext->mTinaRecorder);
	return ret;
}

static int TRparseVideoConfig(void *hdl)
{
    char str[32];
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    if(!trContext)
        return -1;
    TinaRecorder *hdlTina = (TinaRecorder *)trContext->mTinaRecorder;

    if (readKey(trContext->mVideoIndex, kCAMERA_TYPE, str)) {
        hdlTina->vport.cameraType = atoi(str);
        if (hdlTina->vport.cameraType == raw_camera) {
            hdlTina->vport.use_isp = 1;
        }else
            hdlTina->vport.use_isp = 0;
    }
    if (readKey(trContext->mVideoIndex, kVIDEO_ENABLE, str))
        trContext->mEnableVideoInput = atoi(str);
    if (readKey(trContext->mVideoIndex, kVIDEO_WIDTH, str))
        trContext->mInputVideoYuvWidth = atoi(str);
    if (readKey(trContext->mVideoIndex, kVIDEO_HEIGHT, str))
        trContext->mInputVideoYuvHeight = atoi(str);
    if (readKey(trContext->mVideoIndex, kVIDEO_FRAMERATE, str))
        trContext->mInputVideoFramerate = atoi(str);
    if (readKey(trContext->mVideoIndex, kVIDEO_FORMAT, str)) {
        trContext->mInputVideoFormat = GetVideoTRFormat(str);
        if(trContext->mInputVideoFormat < 0){
            TRerr("[%s] not support video format.\n", __func__);
            return -1;
        }
    }
    if (readKey(trContext->mVideoIndex, kVIDEO_ROTATION, str))
        hdlTina->vport.rot_angle = atoi(str);
    if (readKey(trContext->mVideoIndex, kVIDEO_SCALE_DOWN_ENABLE, str)) {
        hdlTina->vport.scale_down_enable = atoi(str);
        if (hdlTina->vport.scale_down_enable) {
            if (readKey(trContext->mVideoIndex, kVIDEO_SUB_WIDTH, str))
                hdlTina->vport.SubWidth = atoi(str);
            if (readKey(trContext->mVideoIndex, kVIDEO_SUB_HEIGHT, str))
                hdlTina->vport.SubHeight = atoi(str);
        }
    }
    if (readKey(trContext->mVideoIndex, kVIDEO_USE_WM, str)) {
        hdlTina->use_wm = atoi(str);
        if (hdlTina->use_wm) {
            if (readKey(trContext->mVideoIndex, kVIDEO_WM_POS_X, str))
                hdlTina->wm_width = atoi(str);
            if (readKey(trContext->mVideoIndex, kVIDEO_WM_POS_Y, str))
                hdlTina->wm_height = atoi(str);
        }
    }
    if (readKey(trContext->mVideoIndex, kVIDEO_BUF_NUM, str))
        hdlTina->vport.buf_num = atoi(str);

    return 0;
}

static int TRparseAudioConfig(void *hdl)
{
    char str[32];
    TrecoderContext* trContext = (TrecoderContext*)hdl;

    if(!trContext)
        return -1;

    if (readKey(trContext->mAudioIndex, kAUDIO_ENABLE, str))
		trContext->mEnableAudioInput = atoi(str);
	if (readKey(trContext->mAudioIndex, kAUDIO_FORMAT, str)) {
		trContext->mInputAudioFormat = GetAudioTRFormat(str);
        if(trContext->mInputAudioFormat < 0){
			TRerr("[%s] not support audio format.\n", __func__);
			return -1;
        }
	}
	if (readKey(trContext->mAudioIndex, kAUDIO_SAMPLERATE, str))
		trContext->mInputAudioSamplerate = atoi(str);
	if (readKey(trContext->mAudioIndex, kAUDIO_CHANNELS, str))
		trContext->mInputAudioChannels = atoi(str);
	if (readKey(trContext->mAudioIndex, kAUDIO_SAMPLEBITS, str))
		trContext->mInputAudioSampleBits = atoi(str);

    return 0;
}

static int TRparseDispConfig(void *hdl)
{
    char str[32];
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    if(!trContext)
        return -1;
    TinaRecorder *hdlTina = (TinaRecorder *)trContext->mTinaRecorder;

    if (readKey(trContext->mVideoIndex, kDISPLAY_ENABLE, str))
        trContext->mEnablePreview = atoi(str);
    if (readKey(trContext->mVideoIndex, kDISPLAY_ROTATION, str))
        trContext->route = atoi(str);
    if (readKey(trContext->mVideoIndex, kDISPLAY_RECT_X, str))
        trContext->mPreviewRect.x = atoi(str);
    if (readKey(trContext->mVideoIndex, kDISPLAY_RECT_Y, str))
        trContext->mPreviewRect.y = atoi(str);
    if (readKey(trContext->mVideoIndex, kDISPLAY_RECT_WIDTH, str))
        trContext->mPreviewRect.width = atoi(str);
    if (readKey(trContext->mVideoIndex, kDISPLAY_RECT_HEIGHT, str))
        trContext->mPreviewRect.height = atoi(str);
    if (readKey(trContext->mVideoIndex, kDISPLAY_ZORDER, str))
        trContext->mPreviewZorder = atoi(str);

    if (readKey(trContext->mVideoIndex, kDISPLAY_SRC_WIDTH, str))
        hdlTina->dispSrcWidth = atoi(str);
    if (readKey(trContext->mVideoIndex, kDISPLAY_SRC_HEIGHT, str))
        hdlTina->dispSrcHeight = atoi(str);

    hdlTina->dispport.rect.x = trContext->mPreviewRect.x;
    hdlTina->dispport.rect.y = trContext->mPreviewRect.y;
    hdlTina->dispport.rect.width = trContext->mPreviewRect.width;
    hdlTina->dispport.rect.height = trContext->mPreviewRect.height;
    hdlTina->zorder = trContext->mPreviewZorder;

    return 0;
}

static int TRparseOutputConfig(void *hdl)
{
    char str[32];
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    if(!trContext)
        return -1;
    TinaRecorder *hdlTina = (TinaRecorder *)trContext->mTinaRecorder;

    if (readKey(trContext->mVideoIndex, kMUXER_OUTPUT_TYPE, str)){
        hdlTina->outputFormat = ReturnMuxerOuputType(str);
        if(hdlTina->outputFormat < 0){
            TRerr("[%s] video muxer type error:type %s!\n", __func__, str);
        }
    }

    if (readKey(trContext->mVideoIndex, kENCODER_VOUTPUT_TYPE, str)){
        hdlTina->encodeVFormat = GetEncoderVideoType(str);
        if(hdlTina->encodeVFormat < 0)
            TRerr("[%s] video encoder format error:format %s!\n", __func__, str);
    }
    if (readKey(trContext->mVideoIndex, kENCODER_VOUTPUT_WIDTH, str))
        hdlTina->encodeWidth = atoi(str);
    if (readKey(trContext->mVideoIndex, kENCODER_VOUTPUT_HEIGHT, str))
        hdlTina->encodeHeight = atoi(str);
    if (readKey(trContext->mVideoIndex, kENCODER_VOUTPUT_BITRATE, str))
        hdlTina->encodeBitrate = atoi(str);
    if (readKey(trContext->mVideoIndex, kENCODER_VOUTPUT_FRAMERATE, str))
        hdlTina->encodeFramerate = atoi(str);
    if (readKey(trContext->mVideoIndex, kENCODER_SCALE_DOWN_RATIO, str))
        hdlTina->scaleDownRatio = atoi(str);

    if (readKey(trContext->mVideoIndex, kENCODER_AOUTPUT_TYPE, str)){
        hdlTina->encodeAFormat = GetEncoderAudioType(str);
        if(hdlTina->encodeAFormat < 0)
            TRerr("[%s] audio encoder type error:type %s!\n", __func__, str);
    }

    return 0;
}

int TRresume(TrecorderHandle *hdl){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->resume);
    int ret = 0;
    ret = trContext->mTinaRecorder->resume(trContext->mTinaRecorder,trContext->mFlags);
    return ret;
}

int TRsetVideoEncodeSize(TrecorderHandle *hdl,int width,int height){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setVideoEncodeSize);
    int ret = 0;
    ret = trContext->mTinaRecorder->setVideoEncodeSize(trContext->mTinaRecorder,width,height);
    return ret;
}
int TRsetOutputFormat(TrecorderHandle *hdl,int format){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
	int trFormat;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setOutputFormat);
    int ret = 0;

	switch(format)
	{
		case T_OUTPUT_TS:
			trFormat = TR_OUTPUT_TS;
			break;
		case T_OUTPUT_MOV:
			trFormat = TR_OUTPUT_MOV;
			break;
		case T_OUTPUT_JPG:
			trFormat = TR_OUTPUT_JPG;
			break;
		case T_OUTPUT_AAC:
			trFormat = TR_OUTPUT_AAC;
			break;
		case T_OUTPUT_MP3:
			trFormat = TR_OUTPUT_MP3;
			break;
		default:
			return -1;
	}
    ret = trContext->mTinaRecorder->setOutputFormat(trContext->mTinaRecorder,trFormat);
    return ret;
}
int TRsetRecorderCallback(TrecorderHandle *hdl,TRecorderCallback callback,void* pUserData){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setCallback);
    int ret = 0;
    trContext->mCallback = callback;
    trContext->mUserData = pUserData;
    ret = trContext->mTinaRecorder->setCallback(trContext->mTinaRecorder,CallbackFromTinaRecorder,(void*)trContext);
    return ret;
}
int TRsetMaxRecordTimeMs(TrecorderHandle *hdl,int ms){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setMaxRecordTime);
    int ret = 0;
    ret = trContext->mTinaRecorder->setMaxRecordTime(trContext->mTinaRecorder,ms);
    return ret;
}
int TRsetVideoEncoderFormat(TrecorderHandle *hdl,int VFormat){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
	int trFormat;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setEncodeVideoFormat);
    int ret = 0;
	switch(VFormat)
	{
		case T_VIDEO_H264:
			trFormat = TR_VIDEO_H264;
			break;
		case T_VIDEO_MJPEG:
			trFormat = TR_VIDEO_MJPEG;
			break;
		default:
			return -1;
	}
    ret = trContext->mTinaRecorder->setEncodeVideoFormat(trContext->mTinaRecorder,trFormat);
    return ret;
}
int TRsetAudioEncoderFormat(TrecorderHandle *hdl,int AFormat){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
	int trFormat;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setEncodeAudioFormat);
    int ret = 0;

	switch(AFormat)
	{
		case T_AUDIO_PCM:
			trFormat = TR_AUDIO_PCM;
			break;
		case T_AUDIO_AAC:
			trFormat = TR_AUDIO_AAC;
			break;
		case T_AUDIO_MP3:
			trFormat = TR_AUDIO_MP3;
			break;
		case T_AUDIO_LPCM:
			trFormat = TR_AUDIO_LPCM;
			break;
		default:
			return -1;
	}
    ret = trContext->mTinaRecorder->setEncodeAudioFormat(trContext->mTinaRecorder,trFormat);
    return ret;
}
int TRsetEncoderBitRate(TrecorderHandle *hdl,int bitrate){//only for video
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setVideoEncodeBitRate);
    int ret = 0;
    ret = trContext->mTinaRecorder->setVideoEncodeBitRate(trContext->mTinaRecorder,bitrate);
    return ret;
}
int TRsetRecorderEnable(TrecorderHandle *hdl,int enable){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
	if(enable)
	{
	TP_CHECK(trContext->mRecorderOutPort);
	}

    int ret = 0;
	if(trContext->mRecorderOutPort)
	{
	    ret = trContext->mRecorderOutPort->enable = enable;
	}
    return ret;
}
int TRsetEncodeFramerate(TrecorderHandle *hdl,int framerate){//only for video
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setEncodeFramerate);
    int ret = 0;
    ret = trContext->mTinaRecorder->setEncodeFramerate(trContext->mTinaRecorder,framerate);
    return ret;
}
int TRgetRecordTime(TrecorderHandle *hdl){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->getRecorderTime);
    int recordtime = 0;
    recordtime = trContext->mTinaRecorder->getRecorderTime(trContext->mTinaRecorder);
    return recordtime;
}
int TRCaptureCurrent(TrecorderHandle *hdl,TCaptureConfig *config){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(config);
    TP_CHECK(trContext->mTinaRecorder->captureCurrent);
    int ret = 0;
    captureConfig capConfig;
	switch(config->captureFormat)
	{
		case T_CAPTURE_JPG:
			capConfig.capFormat = TR_CAPTURE_JPG;
			break;
		default:
            TRerr("[%s] unknow capture format %d!\n", __func__, config->captureFormat);
			return -1;
	}
    strcpy(capConfig.capturePath,config->capturePath);
    capConfig.captureWidth = config->captureWidth;
    capConfig.captureHeight = config->captureHeight;
    ret = trContext->mTinaRecorder->captureCurrent(trContext->mTinaRecorder,&capConfig);
    return ret;
}

int TRsetCameraInputFormat(TrecorderHandle *hdl,int format){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
	int trFormat;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mVideoInPort);
    TP_CHECK(trContext->mVideoInPort->setFormat);
    int ret = 0;

	switch(format)
	{
		case T_CAMERA_YUV420SP:
			trFormat = TR_PIXEL_YUV420SP;
			break;
		case T_CAMERA_YVU420SP:
			trFormat = TR_PIXEL_YVU420SP;
			break;
		case T_CAMERA_YUV420P:
			trFormat = TR_PIXEL_YUV420P;
			break;
		case T_CAMERA_YVU420P:
			trFormat = TR_PIXEL_YVU420P;
			break;
		case T_CAMERA_YUV422SP:
			trFormat = TR_PIXEL_YUV422SP;
			break;
		case T_CAMERA_YVU422SP:
			trFormat = TR_PIXEL_YVU422SP;
			break;
		case T_CAMERA_YUV422P:
			trFormat = TR_PIXEL_YUV422P;
			break;
		case T_CAMERA_YVU422P:
			trFormat = TR_PIXEL_YVU422P;
			break;
		case T_CAMERA_YUYV422:
			trFormat = TR_PIXEL_YUYV422;
			break;
		case T_CAMERA_UYVY422:
			trFormat = TR_PIXEL_UYVY422;
			break;
		case T_CAMERA_YVYU422:
			trFormat = TR_PIXEL_YVYU422;
			break;
		case T_CAMERA_VYUY422:
			trFormat = TR_PIXEL_VYUY422;
			break;
		case T_CAMERA_ARGB:
			trFormat = TR_PIXEL_ARGB;
			break;
		case T_CAMERA_RGBA:
			trFormat = TR_PIXEL_RGBA;
			break;
		case T_CAMERA_ABGR:
			trFormat = TR_PIXEL_ABGR;
			break;
		case T_CAMERA_BGRA:
			trFormat = TR_PIXEL_BGRA;
			break;
		case T_CAMERA_TILE_32X32:
			trFormat = TR_PIXEL_TILE_32X32;
			break;
		case T_CAMERA_TILE_128X32:
			trFormat = TR_PIXEL_TILE_128X32;
			break;
		default:
			return -1;
	}

    trContext->mInputVideoFormat = trFormat;
    //ret = trContext->mVideoInPort->setFormat(trContext->mVideoInPort,format);
    return ret;
}
int TRsetCameraFramerate(TrecorderHandle *hdl,int framerate){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mVideoInPort);
    TP_CHECK(trContext->mVideoInPort->setFrameRate);
    int ret = 0;
    trContext->mInputVideoFramerate = framerate;
    //ret = trContext->mVideoInPort->setFrameRate(trContext->mVideoInPort,framerate);
    return ret;
}
int TRsetCameraCaptureSize(TrecorderHandle *hdl,int width,int height){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mVideoInPort);
    TP_CHECK(trContext->mVideoInPort->setSize);
    int ret = 0;
    trContext->mInputVideoYuvWidth = width;
    trContext->mInputVideoYuvHeight = height;
    //ret = trContext->mVideoInPort->setSize(trContext->mVideoInPort,width,height);
    return ret;
}
int TRsetCameraEnableWM(TrecorderHandle *hdl,int width,int height,int enable){
		TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    int ret = 0;
    trContext->mTinaRecorder->use_wm = enable;
    trContext->mTinaRecorder->wm_width = width;
    trContext->mTinaRecorder->wm_height = height;
    return ret;
}
int TRsetCameraDiscardRatio(TrecorderHandle *hdl,int ratio){//do this in tinarecorder?
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    int ret = 0;
    return ret;
}
int TRsetCameraWaterMarkIndex(TrecorderHandle *hdl,int id){//do this in tinarecorder?
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    int ret = 0;
    return ret;
}

int TRsetCameraEnable(TrecorderHandle *hdl,int enable){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mVideoInPort);
    TP_CHECK(trContext->mVideoInPort->setEnable);
    int ret = 0;
    trContext->mEnableVideoInput = enable;
    //ret = trContext->mVideoInPort->setEnable(trContext->mVideoInPort,enable);
    return ret;
}

int TRsetMICInputFormat(TrecorderHandle *hdl,int format){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mAudioInPort);
    TP_CHECK(trContext->mAudioInPort->setFormat);
    int ret = 0;
	//we only support PCM now
	if(format != T_AUDIO_PCM)
		return -1;
	//fixme,use switch case here
    trContext->mInputAudioFormat = format;
    //ret = trContext->mAudioInPort->setFormat(trContext->mAudioInPort,format);
    return ret;
}

int TRsetMICSampleRate(TrecorderHandle *hdl,int samplerate){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mAudioInPort);
    TP_CHECK(trContext->mAudioInPort->setAudioSampleRate);
    int ret = 0;
    trContext->mInputAudioSamplerate = samplerate;
    //ret = trContext->mAudioInPort->setAudioSampleRate(trContext->mAudioInPort,sampleRate);
    return ret;
}
int TRsetMICChannels(TrecorderHandle *hdl,int channels){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mAudioInPort);
    TP_CHECK(trContext->mAudioInPort->setAudioChannels);
    int ret = 0;
    trContext->mInputAudioChannels = channels;
    //ret = trContext->mAudioInPort->setAudioChannels(trContext->mAudioInPort,channels);
    return ret;
}
int TRsetMICBitSampleBits(TrecorderHandle *hdl,int samplebits){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mAudioInPort);
    TP_CHECK(trContext->mAudioInPort->setAudioSampleBits);
    int ret = 0;
    trContext->mInputAudioSampleBits = samplebits;
    //ret = trContext->mAudioInPort->setAudioBitRate(trContext->mAudioInPort,bitrate);//only support 16
    return ret;
}
int TRsetMICEnable(TrecorderHandle *hdl,int enable){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mAudioInPort);
    TP_CHECK(trContext->mAudioInPort->setEnable);
    int ret = 0;
    trContext->mEnableAudioInput = enable;
    //ret = trContext->mAudioInPort->setEnable(trContext->mAudioInPort,enable);
    return ret;
}

int TRsetPreviewRect(TrecorderHandle *hdl,TdispRect *rect){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
	TinaRecorder *hdlTina = trContext->mTinaRecorder;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mDispOutPort->setRect);
    TP_CHECK(rect);
    int ret = 0;
    trContext->mPreviewRect.x = rect->x;
    trContext->mPreviewRect.y = rect->y;
    trContext->mPreviewRect.width = rect->width;
    trContext->mPreviewRect.height= rect->height;

	if(hdlTina->status == TINA_RECORD_STATUS_RECORDING)
	{
	    ret = hdlTina->dispport.setRect(&hdlTina->dispport,&trContext->mPreviewRect);
	}
    return ret;
}
int TRsetPreviewRotate(TrecorderHandle *hdl,int angle){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
	TinaRecorder *hdlTina = trContext->mTinaRecorder;
	int trAngel;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mDispOutPort);
    TP_CHECK(trContext->mDispOutPort->setRotateAngel);
    int ret = 0;
	switch(angle)
	{
		case T_ROTATION_ANGLE_0:
			trAngel = ROTATION_ANGLE_0;
			break;
		case T_ROTATION_ANGLE_90:
			trAngel = ROTATION_ANGLE_90;
			break;

		case T_ROTATION_ANGLE_180:
			trAngel = ROTATION_ANGLE_180;
			break;
		case T_ROTATION_ANGLE_270:
			trAngel = ROTATION_ANGLE_270;
			break;
		default:
			return -1;
	}
    trContext->mPreviewRotate = trAngel;
	if(hdlTina->status == TINA_RECORD_STATUS_RECORDING)
	{
	    ret = hdlTina->dispport.setRotateAngel(&hdlTina->dispport,trAngel);
	}
    return ret;
}

int TRsetPreviewEnable(TrecorderHandle *hdl,int enable){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
	TinaRecorder *hdlTina = trContext->mTinaRecorder;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mDispOutPort);
    TP_CHECK(trContext->mDispOutPort->setEnable);
    int ret = 0;
    trContext->mEnablePreview = enable;

	if(hdlTina->status == TINA_RECORD_STATUS_RECORDING)
	{
	    ret = hdlTina->dispport.setEnable(&hdlTina->dispport,enable);
	}
    return ret;
}

int TRsetPreviewRoute(TrecorderHandle *hdl,int route)
{
    TrecoderContext* trContext = (TrecoderContext*)hdl;
	int trRoute;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mDispOutPort);
    TP_CHECK(trContext->mDispOutPort->setEnable);
    int ret = 0;
	switch(route)
	{
		case T_ROUTE_ISP:
			trRoute = VIDEO_SRC_FROM_ISP;
			break;
		case T_ROUTE_VE:
			trRoute = VIDEO_SRC_FROM_VE;
			break;
		case T_ROUTE_CAMERA:
			trRoute = VIDEO_SRC_FROM_CAM;
			break;
		default:
			return -1;
			break;
	}
    trContext->route = trRoute;
	return ret;
}

int TRsetVEScaleDownRatio(TrecorderHandle *hdl,int ratio)
{
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setVEScaleDownRatio);
    int ret = 0;
    ret = trContext->mTinaRecorder->setVEScaleDownRatio(trContext->mTinaRecorder,ratio);
    return ret;
}

int TRchangeOutputPath(TrecorderHandle *hdl,char* path){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->changeOutputPath);
    int ret = 0;
    ret = trContext->mTinaRecorder->changeOutputPath(trContext->mTinaRecorder,path);
    return ret;
}

int TRsetAudioMute(TrecorderHandle *hdl,int muteFlag){
    TrecoderContext* trContext = (TrecoderContext*)hdl;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mTinaRecorder);
    TP_CHECK(trContext->mTinaRecorder->setAudioMute);
    int ret = 0;
    ret = trContext->mTinaRecorder->setAudioMute(trContext->mTinaRecorder,muteFlag);
    return ret;
}

int TRsetPreviewZorder(TrecorderHandle *hdl,int zorder)
{
	TrecoderContext* trContext = (TrecoderContext*)hdl;
	TinaRecorder *hdlTina = trContext->mTinaRecorder;
	int ret = -1;
	CHECK_ERROR_RETURN(trContext);
	TP_CHECK(trContext->mDispOutPort);
	TP_CHECK(trContext->mDispOutPort->setEnable);

	switch(zorder)
	{
		case T_PREVIEW_ZORDER_TOP:
			trContext->mPreviewZorder = VIDEO_ZORDER_TOP;
			break;
		case T_PREVIEW_ZORDER_MIDDLE:
			trContext->mPreviewZorder = VIDEO_ZORDER_MIDDLE;
			break;
		case T_PREVIEW_ZORDER_BOTTOM:
			trContext->mPreviewZorder = VIDEO_ZORDER_BOTTOM;
			break;
		default:
			return ret;
	}

	if(hdlTina->status == TINA_RECORD_STATUS_RECORDING)
	{
		ret = hdlTina->dispport.SetZorder(&hdlTina->dispport,trContext->mPreviewZorder);
	}
	return ret;
}

int TRsetPreviewSrcRect(TrecorderHandle *hdl,TdispRect *SrcRect)
{
    TrecoderContext* trContext = (TrecoderContext*)hdl;
	TinaRecorder *hdlTina = trContext->mTinaRecorder;
    CHECK_ERROR_RETURN(trContext);
    TP_CHECK(trContext->mDispOutPort->setSrcRect);
    TP_CHECK(SrcRect);
    int ret = 0;
    trContext->mPreviewSrcRect.x = SrcRect->x;
    trContext->mPreviewSrcRect.y = SrcRect->y;
    trContext->mPreviewSrcRect.width = SrcRect->width;
    trContext->mPreviewSrcRect.height= SrcRect->height;

	if(hdlTina->status == TINA_RECORD_STATUS_RECORDING)
	{
	    ret = hdlTina->dispport.setSrcRect(&hdlTina->dispport,&trContext->mPreviewSrcRect);
	}
    return ret;
}

#define TRoffsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#define TRcontainer_of(ptr, type, member) ({			\
        const typeof(((type *)0)->member) * __mptr = (ptr); \
        (type *)((char *)__mptr - offsetof(type, member)); })

int TRmoduleLink(TrecorderHandle *hdl, TmoduleName *moduleName_1, TmoduleName *moduleName_2, ...)
{
    int ret = -1;
    va_list args;
    enum moduleName *src_moduleName,*sink_moduleName;
    struct moduleAttach *src_module,*sink_module;
    src_moduleName = (enum moduleName *)moduleName_1;
    sink_moduleName = (enum moduleName *)moduleName_2;

    TrecoderContext* trContext = (TrecoderContext*)hdl;
    TinaRecorder *hdlTina = trContext->mTinaRecorder;
    CHECK_ERROR_RETURN(trContext);

    /* internal module initialization */
    hdlTina->moduleDefInit(hdlTina);

    /* do not use the default connection */
    trContext->mUserLinkFlag = 1;
    hdlTina->moduleLinkFlag = 1;

    va_start(args, moduleName_2);

    while(sink_moduleName){
        switch(*src_moduleName){
        case T_CAMERA:
            src_module = &hdlTina->cameraPort;
            break;
        case T_AUDIO:
            src_module = &hdlTina->audioPort;
            break;
        case T_DECODER:
            src_module = &hdlTina->decoderPort;
            break;
        case T_ENCODER:
            src_module = &hdlTina->encoderPort;
            break;
        case T_SCREEN:
            src_module = &hdlTina->screenPort;
            break;
        case T_MUXER:
            src_module = &hdlTina->muxerPort;
            break;
        case T_CUSTOM:
            src_module = TRcontainer_of(src_moduleName, struct moduleAttach, name);
            break;
        default:
            TRerr("[%s] unknown src name 0x%x\n", __func__, *src_moduleName);
            return -1;
        }

        switch(*sink_moduleName){
        case T_CAMERA:
            sink_module = &hdlTina->cameraPort;
            break;
        case T_AUDIO:
            sink_module = &hdlTina->audioPort;
            break;
        case T_DECODER:
            sink_module = &hdlTina->decoderPort;
            break;
        case T_ENCODER:
            sink_module = &hdlTina->encoderPort;
            break;
        case T_SCREEN:
            sink_module = &hdlTina->screenPort;
            break;
        case T_MUXER:
            sink_module = &hdlTina->muxerPort;
            break;
        case T_CUSTOM:
            sink_module = TRcontainer_of(sink_moduleName, struct moduleAttach, name);
            break;
        default:
            TRerr("[%s] unknown sink name 0x%x\n", __func__, *sink_moduleName);
            return -1;
        }

        ret = modules_link(src_module, sink_module, NULL);
        if( ret < 0)
            break;

        src_moduleName = sink_moduleName;
        sink_moduleName = va_arg(args, enum moduleName *);
    }

    va_end(args);

    hdlTina->status = TINA_RECORD_STATUS_DATA_SOURCE_CONFIGURED;

	return ret;
}

int TRmoduleUnlink(TrecorderHandle *hdl, TmoduleName *moduleName_1, TmoduleName *moduleName_2, ...)
{
    int ret = -1;
    va_list args;
    enum moduleName *src_moduleName,*sink_moduleName;
    struct moduleAttach *src_module,*sink_module;
    src_moduleName = (enum moduleName *)moduleName_1;
    sink_moduleName = (enum moduleName *)moduleName_2;

    TrecoderContext* trContext = (TrecoderContext*)hdl;
    TinaRecorder *hdlTina = trContext->mTinaRecorder;
    CHECK_ERROR_RETURN(trContext);

    /* do not use the default connection */
    trContext->mUserLinkFlag = 1;
    hdlTina->moduleLinkFlag = 1;

    va_start(args, moduleName_2);

    while(sink_moduleName){
        switch(*src_moduleName){
        case T_CAMERA:
            src_module = &hdlTina->cameraPort;
            break;
        case T_AUDIO:
            src_module = &hdlTina->audioPort;
            break;
        case T_DECODER:
            src_module = &hdlTina->decoderPort;
            break;
        case T_ENCODER:
            src_module = &hdlTina->encoderPort;
            break;
        case T_SCREEN:
            src_module = &hdlTina->screenPort;
            break;
        case T_MUXER:
            src_module = &hdlTina->muxerPort;
            break;
        case T_CUSTOM:
            src_module = TRcontainer_of(src_moduleName, struct moduleAttach, name);
            break;
        default:
            TRerr("[%s] unknown src name 0x%x\n", __func__, *src_moduleName);
            return -1;
        }

        switch(*sink_moduleName){
        case T_CAMERA:
            sink_module = &hdlTina->cameraPort;
            break;
        case T_AUDIO:
            sink_module = &hdlTina->audioPort;
            break;
        case T_DECODER:
            sink_module = &hdlTina->decoderPort;
            break;
        case T_ENCODER:
            sink_module = &hdlTina->encoderPort;
            break;
        case T_SCREEN:
            sink_module = &hdlTina->screenPort;
            break;
        case T_MUXER:
            sink_module = &hdlTina->muxerPort;
            break;
        case T_CUSTOM:
            sink_module = TRcontainer_of(sink_moduleName, struct moduleAttach, name);
            break;
        default:
            TRerr("[%s] unknown sink name 0x%x\n", __func__, *sink_moduleName);
            return -1;
        }

        ret = modules_unlink(src_module, sink_module, NULL);
        if( ret < 0)
            break;

        src_moduleName = sink_moduleName;
        sink_moduleName = va_arg(args, enum moduleName *);
    }

    va_end(args);

	return ret;
}
