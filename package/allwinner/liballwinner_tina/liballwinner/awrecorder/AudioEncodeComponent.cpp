
#define LOG_TAG "AudioEncoderComponent"

#include "log.h"
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include "AudioEncodeComponent.h"
#include "awMessageQueue.h"

//* demux status, same with the awplayer.
static const int AUDIO_ENC_STATUS_STARTED     = 1;   //* parsing and sending data.
static const int AUDIO_ENC_STATUS_PAUSED      = 1<<1;   //* sending job paused.
static const int AUDIO_ENC_STATUS_STOPPED     = 1<<2;   //* parser closed.

//* command id in encode thread
static const int AUDIO_ENC_COMMAND_ENCODE        = 0x101;
static const int AUDIO_ENC_COMMAND_START         = 0x102;
static const int AUDIO_ENC_COMMAND_STOP          = 0x103;
static const int AUDIO_ENC_COMMAND_PAUSE         = 0x104;
static const int AUDIO_ENC_COMMAND_RESET         = 0x105;
static const int AUDIO_ENC_COMMAND_QUIT          = 0x106;


typedef struct AudioEncodeCompContex
{
	AudioEncoder*		pAudioEncode;
	AwMessageQueue*		mMessageQueue;
	pthread_t		mThreadId;
	int			mStatus;
	AudioEncodeCallback     mCallback;
	void*                   mUserData;
	int					mThreadCreated;

	sem_t                   mSemStart;
	sem_t                   mSemStop;
	sem_t                   mSemReset;
	sem_t                   mSemPause;
	sem_t                   mSemQuit;

	int				    mStartReply;
	int				    mStopReply;
	int				    mResetReply;
	int				    mPauseReply;
	int				    mQuitReply;
}AudioEncodeCompContex;

static void PostEncodeMessage(AwMessageQueue* mq)
{
	if(AwMessageQueueGetCount(mq)<=0)
	{
		AwMessage msg;
		msg.messageId = AUDIO_ENC_COMMAND_ENCODE;
		msg.params[0] = msg.params[1] = msg.params[2] = msg.params[3] = 0;
		if(AwMessageQueuePostMessage(mq, &msg) != 0)
		{
			loge("fatal error, audio decode component post message fail.");
			abort();
		}

		return;
	}
}


static void* AudioEncodeThread(void *arg)
{
	AudioEncodeCompContex *p = (AudioEncodeCompContex*)arg;
	AwMessage			 msg;
	int				 ret;
	sem_t*				 pReplySem;
	int*					pReplyValue;

	while(1)
	{
		if(AwMessageQueueGetMessage(p->mMessageQueue, &msg) < 0)
		{
			loge("get message fail.");
			continue;
		}

		pReplySem	= (sem_t*)msg.params[0];
		pReplyValue = (int*)msg.params[1];

		if(msg.messageId == AUDIO_ENC_COMMAND_START)
		{
			if(p->mStatus == AUDIO_ENC_STATUS_STARTED)
			{
				loge("invalid start operation, already in started status.");
		        if(pReplyValue != NULL)
			        *pReplyValue = -1;
		        sem_post(pReplySem);
		        continue;
			}

			if(p->mStatus == AUDIO_ENC_STATUS_PAUSED)
			{
				PostEncodeMessage(p->mMessageQueue);
				p->mStatus = AUDIO_ENC_STATUS_STARTED;
	            if(pReplyValue != NULL)
	                *pReplyValue = (int)0;
	            if(pReplySem != NULL)
			        sem_post(pReplySem);
			    continue;
			}

			PostEncodeMessage(p->mMessageQueue);
			// reset encoder ******
			p->mStatus = AUDIO_ENC_STATUS_STARTED;
            if(pReplyValue != NULL)
                *pReplyValue = (int)0;
            if(pReplySem != NULL)
		        sem_post(pReplySem);
		    continue;
		} //* end ENC_COMMAND_START.
		else if(msg.messageId == AUDIO_ENC_COMMAND_ENCODE)
		{
			ret = EncodeAudioStream(p->pAudioEncode);
			logv("== audio encode ret: %d", ret);
			if(ret == ERR_AUDIO_ENC_NONE)
			{
				//logd("audio encode ok");
				PostEncodeMessage(p->mMessageQueue);
	            continue;
			}
			else if(ret == ERR_AUDIO_ENC_ABSEND)
			{
				logv("audio encode ERR_AUDIO_ENC_ABSEND");
				p->mCallback(p->mUserData, AUDIO_ENCODE_NOTIFY_EOS, NULL);
				PostEncodeMessage(p->mMessageQueue);
			    continue;
			}
			else if (ret == ERR_AUDIO_ENC_UNKNOWN)
			{
				logv("audio encode ERR_AUDIO_ENC_UNKNOWN");
				p->mCallback(p->mUserData, AUDIO_ENCODE_NOTIFY_ERROR, NULL);
				PostEncodeMessage(p->mMessageQueue);
			    continue;
			}
			else if (ret == ERR_AUDIO_ENC_PCMUNDERFLOW || ret == ERR_AUDIO_ENC_OUTFRAME_UNDERFLOW)
			{
				logv("audio encode ERR_AUDIO_ENC_PCMUNDERFLOW");
				usleep(10*1000); // it is not a good idea to do this
				PostEncodeMessage(p->mMessageQueue);
			    continue;
			}
			else //ERR_AUDIO_ENC_NONE
			{
				logd("unkown ret(%d)", ret);
				continue;
            }
		} //* end ENC_COMMAND_ENCODE.
		else if(msg.messageId == AUDIO_ENC_COMMAND_STOP)
		{
			if(pReplyValue != NULL)
				*pReplyValue = (int)0;
			if(pReplySem != NULL)
				sem_post(pReplySem);
			continue;	//* break the thread.
		} //* end ENC_COMMAND_STOP.
		else if(msg.messageId == AUDIO_ENC_COMMAND_RESET)
		{
			ResetAudioEncoder(p->pAudioEncode);
			if(pReplyValue != NULL)
				*pReplyValue = (int)0;
			if(pReplySem != NULL)
				sem_post(pReplySem);
			continue;	//* break the thread.
		} //* end ENC_COMMAND_STOP.
		else if(msg.messageId == AUDIO_ENC_COMMAND_PAUSE)
		{
			if(p->mStatus == AUDIO_ENC_STATUS_PAUSED)
			{
				loge("invalid pause operation, already in pause status.");
		        if(pReplyValue != NULL)
			        *pReplyValue = -1;
		        sem_post(pReplySem);
		        continue;
			}
			p->mStatus = AUDIO_ENC_STATUS_PAUSED;
            PostEncodeMessage(p->mMessageQueue);   //* post a decode message to decode the first picture.

		    if(pReplyValue != NULL)
			    *pReplyValue = 0;
		    sem_post(pReplySem);
		    continue;
		} //* end ENC_COMMAND_PAUSE.
		else if(msg.messageId == AUDIO_ENC_COMMAND_RESET)
		{
			DestroyAudioEncoder(p->pAudioEncode);
			if(pReplyValue != NULL)
			    *pReplyValue = 0;
		    sem_post(pReplySem);
		    continue;
		}
		else if(msg.messageId == AUDIO_ENC_COMMAND_QUIT)
		{
			if(pReplyValue != NULL)
				*pReplyValue = (int)0;
			if(pReplySem != NULL)
				sem_post(pReplySem);
			break;	//* break the thread.
		}
		else
		{
			logw("unknow message with id %d, ignore.", msg.messageId);
		}
	}

	return NULL;
}

AudioEncodeComp* AudioEncodeCompCreate()
{
	AudioEncodeCompContex *p;
	int ret;

	p = (AudioEncodeCompContex*)malloc(sizeof(AudioEncodeCompContex));
	if(!p)
	{
		loge("malloc failed");
		return NULL;
	}
	memset(p, 0x00, sizeof(AudioEncodeCompContex));

	sem_init(&p->mSemStart, 0, 0);
	sem_init(&p->mSemStop, 0, 0);
	sem_init(&p->mSemPause, 0, 0);
	sem_init(&p->mSemQuit, 0, 0);
	sem_init(&p->mSemReset, 0, 0);
	p->mMessageQueue = AwMessageQueueCreate(4);
	if(NULL == p->mMessageQueue)
	{
		loge("create message queue failed");
		free(p);
		return NULL;
	}

	ret = pthread_create(&p->mThreadId, NULL, AudioEncodeThread, p);
	if(ret != 0)
	{
		p->mThreadCreated = 0;
		return NULL;
	}
	p->mThreadCreated = 1;

	return (AudioEncodeComp*)p;
}

int AudioEncodeCompInit(AudioEncodeComp* v, AudioEncodeConfig* config)
{
	AudioEncodeCompContex *p = (AudioEncodeCompContex*)v;

	AudioEncConfig audioConfig;

	p->pAudioEncode = CreateAudioEncoder();
	if(!p->pAudioEncode)
	{
		loge("AudioEncInit failed");
		return -1;
	}

	if(config->nType != AUDIO_ENCODE_PCM_TYPE)
	{
		loge(" cannot support this audio type(%d)", config->nType);
		//return -1;
	}

	switch (config->nType)
	{
		case AUDIO_ENCODE_PCM_TYPE:
			audioConfig.nType = AUDIO_ENCODER_PCM_TYPE;
			break;
		case AUDIO_ENCODE_AAC_TYPE:
			audioConfig.nType = AUDIO_ENCODER_AAC_TYPE;
			audioConfig.nFrameStyle = 1;
			break;
		case AUDIO_ENCODE_MP3_TYPE:
			audioConfig.nType = AUDIO_ENCODER_MP3_TYPE;
			break;
		case AUDIO_ENCODE_LPCM_TYPE:
			audioConfig.nType = AUDIO_ENCODER_LPCM_TYPE;
			break;
		default:
			loge("unlown audio type(%d)", config->nType);
			break;
	}

	audioConfig.nInSamplerate = config->nInSamplerate;
	audioConfig.nInChan       = config->nInChan;
	audioConfig.nBitrate      = config->nBitrate;
	audioConfig.nFrameStyle   = config->nFrameStyle;
	audioConfig.nOutChan      = config->nOutChan;
	audioConfig.nOutSamplerate= config->nOutSamplerate;
	audioConfig.nSamplerBits  = config->nSamplerBits;

	if(InitializeAudioEncoder(p->pAudioEncode, &audioConfig) < 0)
	{
		loge("audioencoder init failed");
		return -1;
	}

	return 0;
}


void AudioEncodeCompDestory(AudioEncodeComp* v)
{
	AudioEncodeCompContex *p = (AudioEncodeCompContex*)v;
	AwMessage msg;

	if(p->mThreadCreated)
	{
		void* status;

		//* send a quit message to quit the main thread.
		setMessage(&msg, AUDIO_ENC_COMMAND_QUIT, (uintptr_t)&p->mSemQuit);
		AwMessageQueuePostMessage(p->mMessageQueue, &msg);
		SemTimedWait(&p->mSemQuit, -1);
		pthread_join(p->mThreadId, &status);
	}


	sem_destroy(&p->mSemStart);
    sem_destroy(&p->mSemStop);
    sem_destroy(&p->mSemReset);
    sem_destroy(&p->mSemPause);
    sem_destroy(&p->mSemQuit);
    if(p->pAudioEncode)
    {
		DestroyAudioEncoder(p->pAudioEncode);
	}

	AwMessageQueueDestroy(p->mMessageQueue);
	free(p);
}


int AudioEncodeCompStart(AudioEncodeComp *v)
{
	AudioEncodeCompContex *p;
	AwMessage msg;

	p = (AudioEncodeCompContex*)v;

	logd("start");

	//* send a start message.
	setMessage(&msg,
			   AUDIO_ENC_COMMAND_START,		//* message id.
			   (uintptr_t)&p->mSemStart,	   //* params[0] = &mSemStart.
			   (uintptr_t)&p->mStartReply);   //* params[1] = &mStartReply.
	AwMessageQueuePostMessage(p->mMessageQueue, &msg);
	SemTimedWait(&p->mSemStart, -1);
	return (int)p->mStartReply;
}

int AudioEncodeCompStop(AudioEncodeComp* v)
{
	AudioEncodeCompContex* p;
    AwMessage msg;

	p = (AudioEncodeCompContex*)v;
	logd("start");

	//* send a start message.
	setMessage(&msg,
			   AUDIO_ENC_COMMAND_STOP,		//* message id.
			   (uintptr_t)&p->mSemStop,	   //* params[0] = &mSemStart.
			   (uintptr_t)&p->mStopReply);   //* params[1] = &mStartReply.
	AwMessageQueuePostMessage(p->mMessageQueue, &msg);
	SemTimedWait(&p->mSemStop, -1);
	return (int)p->mStopReply;
}

int AudioEncodeCompReset(AudioEncodeComp* v)
{
	AudioEncodeCompContex* p;
    AwMessage msg;

	p = (AudioEncodeCompContex*)v;
	logd("start");

	//* send a start message.
	setMessage(&msg,
			   AUDIO_ENC_COMMAND_RESET,		//* message id.
			   (uintptr_t)&p->mSemReset,	   //* params[0] = &mSemStart.
			   (uintptr_t)&p->mResetReply);   //* params[1] = &mStartReply.
	AwMessageQueuePostMessage(p->mMessageQueue, &msg);
	SemTimedWait(&p->mSemReset, -1);
	return (int)p->mResetReply;
}


int AudioEncodeCompPause(AudioEncodeComp* v)
{
	AudioEncodeCompContex* p;
	AwMessage msg;

	p = (AudioEncodeCompContex*)v;
	logd("pause");

	setMessage(&msg,
	           AUDIO_ENC_COMMAND_PAUSE,
	           (uintptr_t)&p->mSemPause,
	           (uintptr_t)&p->mPauseReply);
	AwMessageQueuePostMessage(p->mMessageQueue, &msg);

    SemTimedWait(&p->mSemPause, -1);
	return (int)p->mPauseReply;
}

int AudioEncodeCompInputBuffer(AudioEncodeComp *v, AudioInputBuffer *buf)
{
	AudioEncodeCompContex* p;

	p = (AudioEncodeCompContex*)v;

	return WriteAudioStreamBuffer(p->pAudioEncode, buf->pData , buf->nLen);
}

int AudioEncodeCompRequestBuffer(AudioEncodeComp *v, AudioEncOutBuffer *bufInfo)
{
	AudioEncodeCompContex* p;
	p = (AudioEncodeCompContex*)v;

	return RequestAudioFrameBuffer(p->pAudioEncode, &bufInfo->pBuf, &bufInfo->len, &bufInfo->pts, &bufInfo->bufId);
}

int AudioEncodeCompReturnBuffer(AudioEncodeComp *v, AudioEncOutBuffer *bufInfo)
{
	AudioEncodeCompContex* p;
	p = (AudioEncodeCompContex*)v;

	return ReturnAudioFrameBuffer(p->pAudioEncode, bufInfo->pBuf, bufInfo->len, bufInfo->pts, bufInfo->bufId);
}

int AudioEncodeCompSetCallback(AudioEncodeComp *v, AudioEncodeCallback notifier, void* pUserData)
{
	AudioEncodeCompContex* p;
	p = (AudioEncodeCompContex*)v;

	p->mCallback = notifier;
	p->mUserData = pUserData;
	return 0;
}
