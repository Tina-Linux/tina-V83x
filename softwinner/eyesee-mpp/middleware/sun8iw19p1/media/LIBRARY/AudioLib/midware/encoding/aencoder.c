//#define LOG_NDEBUG 0
#define LOG_TAG "AudioEncApi.c"

#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "alib_log.h"
#include "aenc_sw_lib.h"
#include "aacencApi.h"
#include "mp3encApi.h"
#include "pcm_enc.h"

#include "aencoder.h"

typedef struct AudioOutBuf_tag
{
	void * buf;
	int size;
	unsigned int timeStamp;
}AudioOutBuf_t;

typedef struct OutBufManager_tag
{
	AudioOutBuf_t	*out_buf;   //[FIFO_LEVEL]
    int mBufCnt;    //FIFO_LEVEL
	int				write_id;
	int				read_id;
    int             prefetch_id;
	int				buf_unused;
}OutBufManager_t;

typedef struct audio_Enc_data audio_Enc_data;
struct audio_Enc_data
{
	struct __AudioENC_AC320		*pAudioEnc;
	__pcm_buf_manager_t			PcmManager;
	__audio_enc_inf_t			AudioBsEncInf;
	__com_internal_prameter_t	EncInternal;
	OutBufManager_t				gOutBufManager;
	pthread_mutex_t				out_buf_lock;
	pthread_mutex_t				in_buf_lock;
	int				encode_type; // 0 aac; 1 LPCM
};

typedef struct AudioEncoderContext
{
	AudioEncoder common;
	void *priv_data;    //audio_Enc_data
	int (*RequestWriteBuf)(AudioEncoder *handle, void * pInbuf, int inSize);
	int (*AudioEncPro)(AudioEncoder *handle);
	int (*GetAudioEncBuf)(AudioEncoder *handle, void ** pOutbuf, unsigned int * outSize, long long * timeStamp, int* pBufId);
    int (*ReleaseAudioEncBuf)(AudioEncoder *handle, void* pOutbuf, unsigned int outSize, long long timeStamp, int nBufId);

    int (*GetValidPcmDataSize)(AudioEncoder *handle);
    int (*GetTotalPcmBufSize)(AudioEncoder *handle);
    int (*GetEmptyFrameNum)(AudioEncoder *handle);
    int (*GetTotalFrameNum)(AudioEncoder *handle);
}AudioEncoderContext;


static int RequestWriteBuf(AudioEncoder *handle, void * pInbuf, int inSize)
{

	AudioEncoderContext *gAEncContent = (AudioEncoderContext *)handle;
	audio_Enc_data * audioEncData = (audio_Enc_data *)gAEncContent->priv_data;

	pthread_mutex_lock(&audioEncData->in_buf_lock);
	if (audioEncData->PcmManager.uFreeBufSize < inSize)
	{
		pthread_mutex_unlock(&audioEncData->in_buf_lock);
		alib_logw("not enough buffer to write, audioEncData->PcmManager.uFreeBufSize: %d", audioEncData->PcmManager.uFreeBufSize);
		return -1;
	}

	if ((audioEncData->PcmManager.pBufWritPtr + inSize)
		> (audioEncData->PcmManager.pBufStart + audioEncData->PcmManager.uBufTotalLen))
	{
		int endSize = audioEncData->PcmManager.pBufStart + audioEncData->PcmManager.uBufTotalLen
			- audioEncData->PcmManager.pBufWritPtr;
        if(endSize > 0)
        {
            memcpy(audioEncData->PcmManager.pBufWritPtr, pInbuf, endSize);
        }
		memcpy(audioEncData->PcmManager.pBufStart, (char*)pInbuf + endSize, inSize - endSize);

		audioEncData->PcmManager.pBufWritPtr = audioEncData->PcmManager.pBufWritPtr
			+ inSize - audioEncData->PcmManager.uBufTotalLen;
	}
	else
	{
		memcpy(audioEncData->PcmManager.pBufWritPtr, pInbuf, inSize);
		audioEncData->PcmManager.pBufWritPtr += inSize;
        if(audioEncData->PcmManager.pBufWritPtr == audioEncData->PcmManager.pBufStart+audioEncData->PcmManager.uBufTotalLen)
        {
            //ALOGD("(f:%s, l:%d) oh my god, we meet a return head, but don't worry.", __FUNCTION__, __LINE__);
            //audioEncData->PcmManager.pBufWritPtr = audioEncData->PcmManager.pBufStart;
        }
	}

	audioEncData->PcmManager.uFreeBufSize -= inSize;
	audioEncData->PcmManager.uDataLen += inSize;

	alib_logv("(f:%s, l:%d) after wr: uBufTotalLen: %d, uDataLen: %d, uFreeBufSize: %d, pBufReadPtr: %x, pBufWritPtr: %x", __FUNCTION__, __LINE__,
		audioEncData->PcmManager.uBufTotalLen, audioEncData->PcmManager.uDataLen, audioEncData->PcmManager.uFreeBufSize,
		audioEncData->PcmManager.pBufReadPtr, audioEncData->PcmManager.pBufWritPtr);

	pthread_mutex_unlock(&audioEncData->in_buf_lock);

	alib_logv("RequestWriteBuf --");

	return 0;
}

static int AudioEncPro(AudioEncoder *handle)
{
	int ret = 0;
	void *pbuf = 0;
	int size = 0;

	AudioEncoderContext *gAEncContent = (AudioEncoderContext *)handle;
	audio_Enc_data * audioEncData = (audio_Enc_data *)gAEncContent->priv_data;

	// write out buf
	pthread_mutex_lock(&audioEncData->out_buf_lock);
	if(audioEncData->gOutBufManager.buf_unused <= 1)
	{
		pthread_mutex_unlock(&audioEncData->out_buf_lock);
		alib_logv("===AudioEncPro: no fifo to write, audioEncData->gOutBufManager.buf_unused: %d\n", audioEncData->gOutBufManager.buf_unused);
		return ERR_AUDIO_ENC_OUTFRAME_UNDERFLOW;
	}
    //ALOGD("(f:%s, l:%d) readyBufCnt[%d]", __FUNCTION__, __LINE__, FIFO_LEVEL-audioEncData->gOutBufManager.buf_unused);
    pthread_mutex_unlock(&audioEncData->out_buf_lock);

	pbuf = audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.write_id].buf;

	if (pbuf == NULL)
	{
		alib_loge("AudioEncPro: error get out buf");
		ret = ERR_AUDIO_ENC_UNKNOWN;
		goto EXIT;
	}

	// now start encoder audio buffer
	ret = audioEncData->pAudioEnc->EncFrame(audioEncData->pAudioEnc, pbuf, &size);

	if (size == 0)
	{
		alib_logv("(f:%s, l:%d) audio not encoder, ret=%d", __FUNCTION__, __LINE__, ret);
		//ret = -1;
		goto EXIT;
	}
	alib_logv("a enc ok: size: %d", size);

	pthread_mutex_lock(&audioEncData->out_buf_lock);
	audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.write_id].size = size;
	audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.write_id].timeStamp = audioEncData->EncInternal.ulNowTimeMS;

	audioEncData->gOutBufManager.buf_unused--;
	audioEncData->gOutBufManager.write_id++;
	audioEncData->gOutBufManager.write_id %= audioEncData->gOutBufManager.mBufCnt;
	pthread_mutex_unlock(&audioEncData->out_buf_lock);

	alib_logv("a wr: buf_unused: %d, read_id: %d, prefetch_id:%d, write_id: %d\n",
		audioEncData->gOutBufManager.buf_unused, audioEncData->gOutBufManager.read_id, audioEncData->gOutBufManager.prefetch_id, audioEncData->gOutBufManager.write_id);
EXIT:
	return ret;
}

static int GetAudioEncBuf(AudioEncoder *handle, void ** pOutbuf, unsigned int * outSize, long long * timeStamp, int* pBufId)
{
	AudioEncoderContext *gAEncContent = (AudioEncoderContext *)handle;
	audio_Enc_data * audioEncData = (audio_Enc_data *)gAEncContent->priv_data;

	pthread_mutex_lock(&audioEncData->out_buf_lock);
	if(audioEncData->gOutBufManager.buf_unused >= audioEncData->gOutBufManager.mBufCnt)
	{
		pthread_mutex_unlock(&audioEncData->out_buf_lock);
		alib_logv("=== GetAudioEncBuf: no valid fifo, buf_unused: %d\n", audioEncData->gOutBufManager.buf_unused);
		return -1;
	}

    if(audioEncData->gOutBufManager.write_id > audioEncData->gOutBufManager.read_id)
    {
        if(audioEncData->gOutBufManager.prefetch_id>=audioEncData->gOutBufManager.read_id
            && audioEncData->gOutBufManager.prefetch_id<=audioEncData->gOutBufManager.write_id)
        {
        }
        else
        {
            alib_loge("(f:%s, l:%d) fatal error! read_id[%d], prefechId[%d], writeId[%d], check code!\n", __FUNCTION__, __LINE__,
                audioEncData->gOutBufManager.read_id, audioEncData->gOutBufManager.prefetch_id, audioEncData->gOutBufManager.write_id);
            pthread_mutex_unlock(&audioEncData->out_buf_lock);
            return -1;
        }
    }
    else if(audioEncData->gOutBufManager.write_id < audioEncData->gOutBufManager.read_id)
    {
        if(audioEncData->gOutBufManager.prefetch_id>=audioEncData->gOutBufManager.read_id
            || audioEncData->gOutBufManager.prefetch_id<=audioEncData->gOutBufManager.write_id)
        {
        }
        else
        {
            alib_loge("(f:%s, l:%d) fatal error! read_id[%d], prefechId[%d], writeId[%d], check code!\n", __FUNCTION__, __LINE__,
                audioEncData->gOutBufManager.read_id, audioEncData->gOutBufManager.prefetch_id, audioEncData->gOutBufManager.write_id);
            pthread_mutex_unlock(&audioEncData->out_buf_lock);
            return -1;
        }
    }
    else
    {
        if(audioEncData->gOutBufManager.buf_unused!=0)
        {
            alib_loge("(f:%s, l:%d) fatal error! read_id[%d], prefechId[%d], writeId[%d], check code!\n", __FUNCTION__, __LINE__,
                audioEncData->gOutBufManager.read_id, audioEncData->gOutBufManager.prefetch_id, audioEncData->gOutBufManager.write_id);
            pthread_mutex_unlock(&audioEncData->out_buf_lock);
            return -1;
        }
    }
	if(audioEncData->gOutBufManager.prefetch_id==audioEncData->gOutBufManager.write_id)
	{
        alib_logv("(f:%s, l:%d) prefechId[%d]==writeId[%d], readId[%d], all outAudioFrames are requested\n", __FUNCTION__, __LINE__,
		    audioEncData->gOutBufManager.prefetch_id, audioEncData->gOutBufManager.write_id, audioEncData->gOutBufManager.read_id);
        pthread_mutex_unlock(&audioEncData->out_buf_lock);
        return -1;
	}
	*pOutbuf	= audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.prefetch_id].buf;
	*outSize	= (unsigned int)(audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.prefetch_id].size);
	*timeStamp	= (long long)(audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.prefetch_id].timeStamp);
    *pBufId = audioEncData->gOutBufManager.prefetch_id;
    alib_logv("(f:%s, l:%d)buf_unused: %d, read_id: %d, prefetch_id: %d, write_id: %d\n", __FUNCTION__, __LINE__,
		audioEncData->gOutBufManager.buf_unused, audioEncData->gOutBufManager.read_id, audioEncData->gOutBufManager.prefetch_id, audioEncData->gOutBufManager.write_id);
    audioEncData->gOutBufManager.prefetch_id++;
    audioEncData->gOutBufManager.prefetch_id %= audioEncData->gOutBufManager.mBufCnt;
	pthread_mutex_unlock(&audioEncData->out_buf_lock);
	return 0;
}

static int ReleaseAudioEncBuf(AudioEncoder *handle, void* pOutbuf, unsigned int outSize, long long timeStamp, int nBufId)
{
	AudioEncoderContext *gAEncContent = (AudioEncoderContext *)handle;
	audio_Enc_data * audioEncData = (audio_Enc_data *)gAEncContent->priv_data;

	pthread_mutex_lock(&audioEncData->out_buf_lock);
	if(audioEncData->gOutBufManager.buf_unused == audioEncData->gOutBufManager.mBufCnt)
	{
		pthread_mutex_unlock(&audioEncData->out_buf_lock);
		alib_loge("(f:%s, l:%d) fatal error! AudioEncPro: no valid fifo\n", __FUNCTION__, __LINE__);
		return -1;
	}
    if(nBufId != audioEncData->gOutBufManager.read_id)
    {
        pthread_mutex_unlock(&audioEncData->out_buf_lock);
        alib_loge("(f:%s, l:%d) fatal error! nReleaseId[%d]!=readId[%d]\n", __FUNCTION__, __LINE__, nBufId, audioEncData->gOutBufManager.read_id);
		return -1;
    }

    //verify.
    if(pOutbuf != audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.read_id].buf
        || outSize != (unsigned int)(audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.read_id].size)
	    //|| timeStamp != (long long)(audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.read_id].timeStamp)
	  )
    {
        alib_loge("(f:%s, l:%d) fatal error, check code!buf[%p->%p]size[%d->%d]pts[%lld->%lld]", __FUNCTION__, __LINE__,
            pOutbuf, audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.read_id].buf,
            outSize, (unsigned int)(audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.read_id].size),
            timeStamp, (long long)(audioEncData->gOutBufManager.out_buf[audioEncData->gOutBufManager.read_id].timeStamp));
    }
	audioEncData->gOutBufManager.buf_unused++;
	audioEncData->gOutBufManager.read_id++;
	audioEncData->gOutBufManager.read_id %= audioEncData->gOutBufManager.mBufCnt;
	alib_logv("(f:%s, l:%d) a rd: buf_unused: %d, read_id: %d, write_id: %d\n", __FUNCTION__, __LINE__,
		audioEncData->gOutBufManager.buf_unused, audioEncData->gOutBufManager.read_id, audioEncData->gOutBufManager.write_id);
	pthread_mutex_unlock(&audioEncData->out_buf_lock);
	return 0;
}

static int GetValidPcmDataSize(AudioEncoder *handle)
{
    AudioEncoderContext *pAEncContent = (AudioEncoderContext *)handle;
    audio_Enc_data * audioEncData = (audio_Enc_data *)pAEncContent->priv_data;
    int dataSize;
	pthread_mutex_lock(&audioEncData->in_buf_lock);
    dataSize = audioEncData->PcmManager.uDataLen;
	pthread_mutex_unlock(&audioEncData->in_buf_lock);
    return dataSize;
}

static int GetTotalPcmBufSize(AudioEncoder *handle)
{
    AudioEncoderContext *pAEncContent = (AudioEncoderContext *)handle;
    audio_Enc_data * audioEncData = (audio_Enc_data *)pAEncContent->priv_data;
    return audioEncData->PcmManager.uBufTotalLen;
}

static int GetEmptyFrameNum(AudioEncoder *handle)
{
    AudioEncoderContext *pAEncContent = (AudioEncoderContext *)handle;
    audio_Enc_data * audioEncData = (audio_Enc_data *)pAEncContent->priv_data;
    int emptyNum;
    pthread_mutex_lock(&audioEncData->out_buf_lock);
	emptyNum = audioEncData->gOutBufManager.buf_unused;
    pthread_mutex_unlock(&audioEncData->out_buf_lock);
    return emptyNum;
}

static int GetTotalFrameNum(AudioEncoder *handle)
{
    if(handle)
    {
        audio_Enc_data * audioEncData = (audio_Enc_data *)((AudioEncoderContext *)handle)->priv_data;
        return audioEncData->gOutBufManager.mBufCnt;
    }
    else
    {
        alib_loge("fatal error! NULL ptr!");
        return 0;
    }
}

/*******************************************************/
// called by audio encoder
int GetPcmDataSize(__pcm_buf_manager_t *pPcmMan)
{
	audio_Enc_data * audioEncData = (audio_Enc_data *)pPcmMan->parent;

	alib_logd("GetPcmDataSize : %d", audioEncData->PcmManager.uDataLen);
    return audioEncData->PcmManager.uDataLen;
}

int ReadPcmDataForEnc(void *pBuf, int uGetLen, __pcm_buf_manager_t *pPcmMan)
{
	audio_Enc_data * audioEncData = (audio_Enc_data *)pPcmMan->parent;

	alib_logv("ReadPcmDataForEnc ++, getLen: %d", uGetLen);

    pthread_mutex_lock(&audioEncData->in_buf_lock);

    if(audioEncData->PcmManager.uDataLen < uGetLen)
    {
        alib_logw("pcm is not enough for audio encoder! uGetLen: %d, uDataLen: %d\n",
			uGetLen, audioEncData->PcmManager.uDataLen);
        pthread_mutex_unlock(&audioEncData->in_buf_lock);
        return 0;
    }

    if((audioEncData->PcmManager.pBufReadPtr + uGetLen)
		> (audioEncData->PcmManager.pBufStart + audioEncData->PcmManager.uBufTotalLen))
    {
        int len1 = audioEncData->PcmManager.pBufStart
			+ audioEncData->PcmManager.uBufTotalLen - audioEncData->PcmManager.pBufReadPtr;
        memcpy((void *)pBuf,(void *)audioEncData->PcmManager.pBufReadPtr,len1);
        memcpy((void *)((char *)pBuf + len1), (void *)audioEncData->PcmManager.pBufStart, uGetLen - len1);
    }
    else
    {
        memcpy(pBuf, audioEncData->PcmManager.pBufReadPtr, uGetLen);
    }

    audioEncData->PcmManager.pBufReadPtr += uGetLen;

    if(audioEncData->PcmManager.pBufReadPtr
		>= audioEncData->PcmManager.pBufStart + audioEncData->PcmManager.uBufTotalLen)
    {
        audioEncData->PcmManager.pBufReadPtr -= audioEncData->PcmManager.uBufTotalLen;
    }
    audioEncData->PcmManager.uDataLen -= uGetLen;
    audioEncData->PcmManager.uFreeBufSize += uGetLen;

	alib_logv("after rd: uBufTotalLen: %d, uDataLen: %d, uFreeBufSize: %d, pBufReadPtr: %x, pBufWritPtr: %x",
		audioEncData->PcmManager.uBufTotalLen, audioEncData->PcmManager.uDataLen, audioEncData->PcmManager.uFreeBufSize,
		audioEncData->PcmManager.pBufReadPtr, audioEncData->PcmManager.pBufWritPtr);

	pthread_mutex_unlock(&audioEncData->in_buf_lock);
	alib_logv("ReadPcmDataForEnc --");

    return uGetLen;
}
/*************************************************************************/

static int AudioEncInit(AudioEncoder *pEncoder, __audio_enc_inf_t * audio_inf, int encode_type, int nInBufSize, int nOutBufCnt)
{
	int i = 0;
	audio_Enc_data *audioEncData = NULL;

	if(encode_type != AUDIO_ENCODER_AAC_TYPE &&
       encode_type != AUDIO_ENCODER_PCM_TYPE &&
       encode_type != AUDIO_ENCODER_LPCM_TYPE &&
       encode_type != AUDIO_ENCODER_MP3_TYPE)
	{
		alib_loge("(f:%s, l:%d) not support audio encode type[%d]", __FUNCTION__, __LINE__, encode_type);
		return -1;
	}
	//init audio encode content
	AudioEncoderContext	*gAEncContent = (AudioEncoderContext *)pEncoder;

	audioEncData = (audio_Enc_data *)malloc(sizeof(audio_Enc_data));
	if(audioEncData == NULL)
	{
		alib_loge("malloc audioEncData fail");
		return -1;
	}

	memset((void *)audioEncData, 0 , sizeof(audio_Enc_data));
	gAEncContent->priv_data = (void *)audioEncData;

    audioEncData->gOutBufManager.mBufCnt = nOutBufCnt>0?nOutBufCnt:FIFO_LEVEL;
    audioEncData->gOutBufManager.buf_unused = audioEncData->gOutBufManager.mBufCnt;
    audioEncData->gOutBufManager.out_buf = malloc(sizeof(AudioOutBuf_t)*audioEncData->gOutBufManager.mBufCnt);
    if(NULL == audioEncData->gOutBufManager.out_buf)
    {
        alib_loge("fatal error! malloc fail!");
        goto _err0;
    }
	for (i = 0; i < audioEncData->gOutBufManager.mBufCnt; i++)
	{
		audioEncData->gOutBufManager.out_buf[i].buf = (void *)malloc(OUT_ENCODE_BUFFER_SIZE);
		if (audioEncData->gOutBufManager.out_buf[i].buf == NULL)
		{
			alib_loge("AudioEncInit: malloc out buffer failed");
			//return -1;
			goto _err1;
		}
	}

    audioEncData->PcmManager.uBufTotalLen = nInBufSize>0?nInBufSize:BS_BUFFER_SIZE;
	audioEncData->PcmManager.pBufStart = (unsigned char *)malloc(audioEncData->PcmManager.uBufTotalLen);
	if (audioEncData->PcmManager.pBufStart == NULL)
	{
		alib_loge("AudioEncInit: malloc PcmManager failed");
		//return -1;
		goto _err1;
	}

	audioEncData->PcmManager.pBufWritPtr		= audioEncData->PcmManager.pBufStart;
	audioEncData->PcmManager.pBufReadPtr		= audioEncData->PcmManager.pBufStart;
	audioEncData->PcmManager.uFreeBufSize	= audioEncData->PcmManager.uBufTotalLen;
	audioEncData->PcmManager.uDataLen		= 0;
	audioEncData->PcmManager.parent				= (void *)audioEncData;

	// set audio encode information for audio encode lib
	memcpy((void *)&audioEncData->AudioBsEncInf, (void *)audio_inf, sizeof(__audio_enc_inf_t));

	// get __AudioENC_AC320
	if(encode_type == AUDIO_ENCODER_AAC_TYPE)
	{
		alib_logv("++++++++++++ encode aac encode ++++++++++++");
		audioEncData->pAudioEnc = AudioAACENCEncInit();
	}
    else if(encode_type == AUDIO_ENCODER_PCM_TYPE || encode_type == AUDIO_ENCODER_LPCM_TYPE)
    {
        alib_logv("(f:%s, l:%d) encode pcm[%d]", __FUNCTION__, __LINE__, encode_type);
		audioEncData->pAudioEnc = AudioPCMEncInit();
    }
    else if(encode_type == AUDIO_ENCODER_MP3_TYPE)
    {
        alib_logv("(f:%s, l:%d) encode[%d]", __FUNCTION__, __LINE__, encode_type);
		audioEncData->pAudioEnc = AudioMP3ENCEncInit();
    }
	else
	{
		alib_loge("(f:%s, l:%d) not support other audio encode type[%d]", __FUNCTION__, __LINE__, encode_type);
		//return -1;
	}

	if (audioEncData->pAudioEnc == NULL)
	{
		alib_loge("AudioEncInit: EncInit failed");
		//return -1;
		goto _err2;
	}

    audioEncData->encode_type = encode_type;

	audioEncData->pAudioEnc->pPcmBufManager	= &(audioEncData->PcmManager);
	audioEncData->pAudioEnc->AudioBsEncInf	= &(audioEncData->AudioBsEncInf);
	audioEncData->pAudioEnc->EncoderCom		= &(audioEncData->EncInternal);
	audioEncData->pAudioEnc->EncInit(audioEncData->pAudioEnc);

	gAEncContent->AudioEncPro		  = AudioEncPro;
	gAEncContent->RequestWriteBuf	  = RequestWriteBuf;
	gAEncContent->GetAudioEncBuf	  = GetAudioEncBuf;
    gAEncContent->ReleaseAudioEncBuf  = ReleaseAudioEncBuf;
    gAEncContent->GetValidPcmDataSize = GetValidPcmDataSize;
    gAEncContent->GetTotalPcmBufSize  = GetTotalPcmBufSize;
    gAEncContent->GetEmptyFrameNum    = GetEmptyFrameNum;
    gAEncContent->GetTotalFrameNum    = GetTotalFrameNum;

	pthread_mutex_init(&audioEncData->in_buf_lock,NULL);
	pthread_mutex_init(&audioEncData->out_buf_lock,NULL);

	return 0;

_err2:
    if (audioEncData->PcmManager.pBufStart != NULL)
    {
        free(audioEncData->PcmManager.pBufStart);
        audioEncData->PcmManager.pBufStart = NULL;
    }
_err1:
    for (i = 0; i < audioEncData->gOutBufManager.mBufCnt; i++)
    {
        if (audioEncData->gOutBufManager.out_buf[i].buf != NULL)
        {
            free(audioEncData->gOutBufManager.out_buf[i].buf);
            audioEncData->gOutBufManager.out_buf[i].buf = NULL;
        }
    }
    if(audioEncData->gOutBufManager.out_buf != NULL)
    {
        free(audioEncData->gOutBufManager.out_buf);
        audioEncData->gOutBufManager.out_buf = NULL;
    }
_err0:
    if(audioEncData)
    {
        free(audioEncData);
        audioEncData = NULL;
    }
}

static void AudioEncExit(void *handle)
{
	int i = 0;
	AudioEncoderContext *gAEncContent = NULL;
	audio_Enc_data * audioEncData = NULL;
	
	gAEncContent = (AudioEncoderContext *)handle;
	if(gAEncContent == NULL)
	{
		alib_loge("gAEncContent == NULL");
		return;
	}
	else
	{	
		audioEncData = (audio_Enc_data *)gAEncContent->priv_data;
	}

	for (i = 0; i < audioEncData->gOutBufManager.mBufCnt; i++)
	{
		if (audioEncData->gOutBufManager.out_buf[i].buf != NULL)
		{
			free(audioEncData->gOutBufManager.out_buf[i].buf);
			audioEncData->gOutBufManager.out_buf[i].buf = NULL;
		}
	}
    if(audioEncData->gOutBufManager.out_buf != NULL)
    {
        free(audioEncData->gOutBufManager.out_buf);
        audioEncData->gOutBufManager.out_buf = NULL;
    }
	if (audioEncData->PcmManager.pBufStart != NULL)
	{
		free(audioEncData->PcmManager.pBufStart);
		audioEncData->PcmManager.pBufStart = NULL;
	}

	if (audioEncData->pAudioEnc != NULL)
	{
		audioEncData->pAudioEnc->EncExit(audioEncData->pAudioEnc);

		if(audioEncData->encode_type == AUDIO_ENCODER_AAC_TYPE)
		{
			AudioAACENCEncExit(audioEncData->pAudioEnc);
		}
        else if(audioEncData->encode_type == AUDIO_ENCODER_MP3_TYPE)
		{
			AudioMP3ENCEncExit(audioEncData->pAudioEnc);
		}
		else
		{
			AudioPCMEncExit(audioEncData->pAudioEnc);
		}

		audioEncData->pAudioEnc = NULL;
	}

	pthread_mutex_destroy(&audioEncData->in_buf_lock);
	pthread_mutex_destroy(&audioEncData->out_buf_lock);

	if(audioEncData)
	{
		free(audioEncData);
		audioEncData = NULL;
	}

	if(gAEncContent)
	{
		free(gAEncContent);
		gAEncContent = NULL;
	}

	alib_logv("AudioEncExit --");
}


AudioEncoder* CreateAudioEncoder()
{
	AudioEncoderContext *pAudioEncoder = (AudioEncoderContext*)malloc(sizeof(AudioEncoderContext));
	if(NULL == pAudioEncoder)
	{
		alib_loge("create audio encoder failed");
		return NULL;
	}
	memset(pAudioEncoder, 0x00, sizeof(AudioEncoderContext));

	return (AudioEncoder*)pAudioEncoder;
}

int InitializeAudioEncoder(AudioEncoder *pEncoder, AudioEncConfig *pConfig)
{
	AudioEncoderContext *p;

	p = (AudioEncoderContext*)pEncoder;

	if(pConfig->nType != AUDIO_ENCODER_AAC_TYPE &&
	   pConfig->nType != AUDIO_ENCODER_PCM_TYPE &&
	   pConfig->nType != AUDIO_ENCODER_MP3_TYPE)
	{
		alib_loge("cannot support sudio encode type(%d)", pConfig->nType);
		return -1;
	}

	__audio_enc_inf_t audioInfo;
	audioInfo.bitrate	= pConfig->nBitrate;
	audioInfo.frame_style	= pConfig->nFrameStyle;
	audioInfo.InChan	= pConfig->nInChan;
	audioInfo.InSamplerate	= pConfig->nInSamplerate;
	audioInfo.OutChan		= pConfig->nOutChan;
	audioInfo.OutSamplerate = pConfig->nOutSamplerate;
	audioInfo.SamplerBits	= pConfig->nSamplerBits;

	return AudioEncInit(pEncoder, &audioInfo, pConfig->nType, pConfig->mInBufSize, pConfig->mOutBufCnt);
}

int ResetAudioEncoder(AudioEncoder* pEncoder)
{
	AudioEncoderContext *p;

	p = (AudioEncoderContext*)pEncoder;
	AudioEncExit(pEncoder);

	return 0;
}

void DestroyAudioEncoder(AudioEncoder* pEncoder)
{
	AudioEncoderContext *pAudioEncCtx;

	pAudioEncCtx = (AudioEncoderContext*)pEncoder;
	AudioEncExit(pEncoder);

}

int EncodeAudioStream(AudioEncoder *pEncoder)
{
	AudioEncoderContext *pAudioEncCtx;

	pAudioEncCtx = (AudioEncoderContext*)pEncoder;

	return pAudioEncCtx->AudioEncPro(pEncoder);
}

int WriteAudioStreamBuffer(AudioEncoder *pEncoder, char* pBuf, int len)
{
	AudioEncoderContext *pAudioEncCtx;
	pAudioEncCtx = (AudioEncoderContext*)pEncoder;

	return pAudioEncCtx->RequestWriteBuf(pEncoder, pBuf, len);
}

int RequestAudioFrameBuffer(AudioEncoder *pEncoder, char **pOutBuf, unsigned int *size, long long *pts, int *bufId)
{
	AudioEncoderContext *pAudioEncCtx;

	pAudioEncCtx = (AudioEncoderContext*)pEncoder;
	return pAudioEncCtx->GetAudioEncBuf(pEncoder, (void**)pOutBuf, size, pts, bufId);
}

int ReturnAudioFrameBuffer(AudioEncoder *pEncoder, char *pOutBuf, unsigned int size, long long pts, int bufId)
{
	AudioEncoderContext *pAudioEncCtx;

	pAudioEncCtx = (AudioEncoderContext*)pEncoder;
	return pAudioEncCtx->ReleaseAudioEncBuf(pEncoder, pOutBuf, size, pts, bufId);
}

int AudioEncoder_GetValidPcmDataSize(AudioEncoder *pEncoder)
{
    AudioEncoderContext *pAudioEncCtx = (AudioEncoderContext*)pEncoder;
	return pAudioEncCtx->GetValidPcmDataSize(pEncoder);
}

int AudioEncoder_GetTotalPcmBufSize(AudioEncoder *pEncoder)
{
    AudioEncoderContext *pAudioEncCtx = (AudioEncoderContext*)pEncoder;
	return pAudioEncCtx->GetTotalPcmBufSize(pEncoder);
}

int AudioEncoder_GetEmptyFrameNum(AudioEncoder *pEncoder)
{
    AudioEncoderContext *pAudioEncCtx = (AudioEncoderContext*)pEncoder;
	return pAudioEncCtx->GetEmptyFrameNum(pEncoder);
}

int AudioEncoder_GetTotalFrameNum(AudioEncoder *pEncoder)
{
    AudioEncoderContext *pAudioEncCtx = (AudioEncoderContext*)pEncoder;
	return pAudioEncCtx->GetTotalFrameNum(pEncoder);
}

