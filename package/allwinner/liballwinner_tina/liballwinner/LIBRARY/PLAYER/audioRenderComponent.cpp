
#include "log.h"

#include "audioRenderComponent.h"
#include "messageQueue.h"
#include "soundControl.h"

extern SoundControlOpsT mSoundControlOps;

typedef struct AudioRenderCompContext
{
    //* created at initialize time.
    MessageQueue*               mq;
    sem_t                       startMsgReplySem;
    sem_t                       stopMsgReplySem;
    sem_t                       pauseMsgReplySem;
    sem_t                       resetMsgReplySem;
    sem_t                       eosMsgReplySem;
    sem_t                       quitMsgReplySem;
    sem_t                       setAudioSinkReplySem;

    int                         nStartReply;
    int                         nStopReply;
    int                         nPauseReply;
    int                         nResetReply;
    int                         nAudioSinkReply;

    pthread_t                   sRenderThread;

    enum EPLAYERSTATUS          eStatus;
    void*                       pAudioSink;
    SoundCtrl*                  pSoundCtrl;
	SoundCtrl*                  pRawSndCtrl;
    AudioDecComp*               pDecComp;

    SoundControlOpsT*           pSoundCtrlOps;
    SoundControlOpsT*           pRawSndCtrlOps;

    //* objects set by user.
    AvTimer*                    pAvTimer;
    PlayerCallback              callback;
    void*                       pUserData;
    int                         bEosFlag;

    //* audio balance, return 1 means left channel, 2 means right channel, 3 means stereo, 0 means default.
    int                         nConfigOutputBalance;
    //* audio mute, 1 means mute the audio.
    int                         bMute;

	int                         bForceWriteToDeviceFlag;
	float						volume;
}AudioRenderCompContext;


static void* AudioRenderThread(void* arg);
static void PostRenderMessage(MessageQueue* mq);
static void ProcessBalance(unsigned char* pData, int nDataLen, int nBitsPerSample, int nChannelCount, int nOutBalance);
static void ProcessMute(unsigned char* pData, int nDataLen, int bMute);

static int SoundCallback(void* pUserData, int eMessageId, void* param)
{
    AudioRenderCompContext* p;

    p = (AudioRenderCompContext*)pUserData;

    if(eMessageId == MESSAGE_ID_SOUND_NOTIFY_BUFFER)
    {
	if(p->callback)
		p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_AUDIO_BUFFER, param);
    }

    return 0;
}


AudioRenderComp* AudioRenderCompCreate(void)
{
    AudioRenderCompContext* p;
    int                     err;

    p = (AudioRenderCompContext*)malloc(sizeof(AudioRenderCompContext));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }
    memset(p, 0, sizeof(*p));
	//p->volume = -1.0;
	p->volume = 0.0;

	#if(CONFIG_OS == OPTION_OS_ANDROID)
	p->pSoundCtrlOps = &mSoundControlOps;
	p->pRawSndCtrlOps = &mRawSoundControlOps;
	#else
	p->pSoundCtrlOps = &mSoundControlOps;
	p->pRawSndCtrlOps = &mSoundControlOps;
	#endif

    p->mq = MessageQueueCreate(4, "AudioRenderMq");
    if(p->mq == NULL)
    {
        loge("audio render component create message queue fail.");
        free(p);
        return NULL;
    }

    sem_init(&p->startMsgReplySem, 0, 0);
    sem_init(&p->stopMsgReplySem, 0, 0);
    sem_init(&p->pauseMsgReplySem, 0, 0);
    sem_init(&p->resetMsgReplySem, 0, 0);
    sem_init(&p->eosMsgReplySem, 0, 0);
    sem_init(&p->quitMsgReplySem, 0, 0);
    sem_init(&p->setAudioSinkReplySem, 0, 0);

    p->eStatus = PLAYER_STATUS_STOPPED;

    err = pthread_create(&p->sRenderThread, NULL, AudioRenderThread, p);
    if(err != 0)
    {
        loge("audio render component create thread fail.");
        sem_destroy(&p->startMsgReplySem);
        sem_destroy(&p->stopMsgReplySem);
        sem_destroy(&p->pauseMsgReplySem);
        sem_destroy(&p->resetMsgReplySem);
        sem_destroy(&p->eosMsgReplySem);
        sem_destroy(&p->quitMsgReplySem);
        sem_destroy(&p->setAudioSinkReplySem);
        MessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    return (AudioRenderComp*)p;
}

int AudioRenderSetSoundCtlOps(AudioRenderComp* a, SoundControlOpsT* ops)
{
	AudioRenderCompContext* p;

    p = (AudioRenderCompContext*)a;
    p->pSoundCtrlOps = ops;
    p->pRawSndCtrlOps = ops;
	logd("=== set sound control ops");
    return 0;
}


int AudioRenderCompDestroy(AudioRenderComp* a)
{
    void*                   status;
    AudioRenderCompContext* p;
    Message                 msg;

    p = (AudioRenderCompContext*)a;

    msg.messageId = MESSAGE_ID_QUIT;
    msg.params[0] = (uintptr_t)&p->quitMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio render component post message fail.");
        abort();
    }

    SemTimedWait(&p->quitMsgReplySem, -1);
    pthread_join(p->sRenderThread, &status);

    sem_destroy(&p->startMsgReplySem);
    sem_destroy(&p->stopMsgReplySem);
    sem_destroy(&p->pauseMsgReplySem);
    sem_destroy(&p->resetMsgReplySem);
    sem_destroy(&p->eosMsgReplySem);
    sem_destroy(&p->quitMsgReplySem);
    sem_destroy(&p->setAudioSinkReplySem);
    MessageQueueDestroy(p->mq);
    free(p);

    return 0;
}


int AudioRenderCompStart(AudioRenderComp* a)
{
    AudioRenderCompContext* p;
    Message                 msg;

    p = (AudioRenderCompContext*)a;

    logv("audio render component starting");

    msg.messageId = MESSAGE_ID_START;
    msg.params[0] = (uintptr_t)&p->startMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStartReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->startMsgReplySem, -1) < 0)
    {
        loge("audio render component wait for start finish timeout.");
        return -1;
    }

    return p->nStartReply;
}


int AudioRenderCompStop(AudioRenderComp* a)
{
    AudioRenderCompContext* p;
    Message                 msg;

    p = (AudioRenderCompContext*)a;

    logv("audio render component stopping");

    msg.messageId = MESSAGE_ID_STOP;
    msg.params[0] = (uintptr_t)&p->stopMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStopReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->stopMsgReplySem, -1) < 0)
    {
        loge("audio render component wait for stop finish timeout.");
        return -1;
    }

    return p->nStopReply;
}


int AudioRenderCompPause(AudioRenderComp* a)
{
    AudioRenderCompContext* p;
    Message                 msg;

    p = (AudioRenderCompContext*)a;

    logv("audio render component pausing");

    msg.messageId = MESSAGE_ID_PAUSE;
    msg.params[0] = (uintptr_t)&p->pauseMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nPauseReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->pauseMsgReplySem, -1) < 0)
    {
        loge("audio render component wait for pause finish timeout.");
        return -1;
    }

    return p->nPauseReply;
}


enum EPLAYERSTATUS AudioRenderCompGetStatus(AudioRenderComp* a)
{
    AudioRenderCompContext* p;
    p = (AudioRenderCompContext*)a;
    return p->eStatus;
}


int AudioRenderCompReset(AudioRenderComp* a)
{
    AudioRenderCompContext* p;
    Message                 msg;

    p = (AudioRenderCompContext*)a;

    logv("audio render component reseting");

    msg.messageId = MESSAGE_ID_RESET;
    msg.params[0] = (uintptr_t)&p->resetMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nResetReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->resetMsgReplySem, -1) < 0)
    {
        loge("audio render component wait for reset finish timeout.");
        return -1;
    }

    return p->nResetReply;
}


int AudioRenderCompSetEOS(AudioRenderComp* a)
{
    AudioRenderCompContext* p;
    Message                 msg;

    p = (AudioRenderCompContext*)a;

    logv("audio render component setting EOS.");

    msg.messageId = MESSAGE_ID_EOS;
    msg.params[0] = (uintptr_t)&p->eosMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->eosMsgReplySem, -1) < 0)
    {
        loge("audio render component wait for setting eos finish timeout.");
        return -1;
    }

    return 0;
}


int AudioRenderCompSetCallback(AudioRenderComp* a, PlayerCallback callback, void* pUserData)
{
    AudioRenderCompContext* p;

    p = (AudioRenderCompContext*)a;

    p->callback  = callback;
    p->pUserData = pUserData;

    return 0;
}


int AudioRenderCompSetTimer(AudioRenderComp* a, AvTimer* timer)
{
    AudioRenderCompContext* p;
    p = (AudioRenderCompContext*)a;
    p->pAvTimer  = timer;
    return 0;
}


int AudioRenderCompSetAudioSink(AudioRenderComp* a, void* pAudioSink)
{
    AudioRenderCompContext* p;
    Message                 msg;

    p = (AudioRenderCompContext*)a;

    logv("audio render component setting window.");

    msg.messageId = MESSAGE_ID_SET_AUDIO_SINK;
    msg.params[0] = (uintptr_t)&p->setAudioSinkReplySem;
    msg.params[1] = 0;
    msg.params[2] = (uintptr_t)pAudioSink;
    msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, audio render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->setAudioSinkReplySem, -1) < 0)
    {
        loge("audio render component wait for setting window finish timeout.");
        return -1;
    }

    return 0;
}


int AudioRenderCompSetDecodeComp(AudioRenderComp* a, AudioDecComp* d)
{
    AudioRenderCompContext* p;
    p = (AudioRenderCompContext*)a;
    p->pDecComp  = d;
    return 0;
}


int64_t AudioRenderCompCacheTimeUs(AudioRenderComp* a)
{
    AudioRenderCompContext* p;
    int64_t                 nCachedTimeUs;

    p = (AudioRenderCompContext*)a;

    if(p->pSoundCtrl == NULL)
        return 0;

    nCachedTimeUs = p->pSoundCtrlOps->SoundDeviceGetCachedTime(p->pSoundCtrl);
    return nCachedTimeUs;
}


static void* AudioRenderThread(void* arg)
{
    AudioRenderCompContext* p;
    Message                 msg;
    int                     ret;
    sem_t*                  pReplySem;
    int*                    pReplyValue;
    int64_t                 nCurTime;
    int64_t                 nPts;
    int                     bFirstFrameSend;
    unsigned char*          pPcmData;
    unsigned int            nPcmDataLen;
    unsigned int            nWritten;
    unsigned int            nSampleRate;
    unsigned int            nChannelNum;
    unsigned int            nBitsPerSample;
    unsigned int			nNewSampleRate;
    unsigned int			nNewChannelNum;
    unsigned int            nFrameSize;
    unsigned int            nBitRate;
	unsigned int			nNewBitRate;
	int						audioInfoNotified;
	cedar_raw_data          raw_data;

    p = (AudioRenderCompContext*)arg;
    bFirstFrameSend = 0;
    pPcmData        = NULL;
    nPcmDataLen     = 0;
    nSampleRate     = 0;
    nChannelNum     = 0;
    nBitsPerSample  = 16;
    nNewSampleRate  = 0;
    nNewChannelNum	= 0;
    nFrameSize      = 0;
    nBitRate        = 0;
	nNewBitRate		= 0;
	audioInfoNotified = 0;

    while(1)
    {
get_message:
        if(MessageQueueGetMessage(p->mq, &msg) < 0)
        {
            loge("get message fail.");
            continue;
        }

process_message:
        pReplySem   = (sem_t*)msg.params[0];
        pReplyValue = (int*)msg.params[1];

        if(msg.messageId == MESSAGE_ID_START)
        {
            logi("audio render process start message.");
            if(p->eStatus == PLAYER_STATUS_STARTED)
            {
                logw("already in started status.");
                PostRenderMessage(p->mq);
                *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            if(p->eStatus == PLAYER_STATUS_STOPPED)
            {
                bFirstFrameSend = 0;
                nSampleRate     = 0;
                nChannelNum     = 0;
                p->bEosFlag     = 0;
            }
            else
            {
                //* resume from pause status.
                if(p->pSoundCtrl)
                {
                    logd("start sound devide.");
                    p->pSoundCtrlOps->SoundDeviceStart(p->pSoundCtrl);
                    //SoundDeviceStart(p->pSoundCtrl);
                }
				if(p->pRawSndCtrl)
                {
                    logd("start raw sound devide.");
                    //SoundDeviceStart_raw(p->pRawSndCtrl);
                    p->pRawSndCtrlOps->SoundDeviceStart_raw(p->pRawSndCtrl);
                }
            }

            //* send a render message to start decoing.
            PostRenderMessage(p->mq);

            p->eStatus = PLAYER_STATUS_STARTED;
            *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_STOP)
        {
            logi("audio render process stop message.");
            if(p->eStatus == PLAYER_STATUS_STOPPED)
            {
                logw("already in stopped status.");
                *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            //* release the sound device.
            if(p->pSoundCtrl)
            {
                logd("stop and release sound devide.");
                p->pSoundCtrlOps->SoundDeviceStop(p->pSoundCtrl);
                p->pSoundCtrlOps->SoundDeviceRelease(p->pSoundCtrl);
                //SoundDeviceStop(p->pSoundCtrl);
                //SoundDeviceRelease(p->pSoundCtrl);
                p->pSoundCtrl = NULL;
            }
            if(p->pRawSndCtrl)
			{
			    logd("stop and release raw sound devide.");
				p->pRawSndCtrlOps->SoundDeviceStop_raw(p->pRawSndCtrl);
                p->pRawSndCtrlOps->SoundDeviceRelease_raw(p->pRawSndCtrl);
                p->pRawSndCtrl = NULL;
			}
            //* set status to stopped.
            p->eStatus = PLAYER_STATUS_STOPPED;
            *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_PAUSE)
        {
            logi("audio render process pause message.");
            if(p->eStatus != PLAYER_STATUS_STARTED)
            {
                logw("not in started status, pause operation invalid.");
                *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            //* pause the sound device.
            if(p->pSoundCtrl)
            {
                logd("pause sound devide.");
                p->pSoundCtrlOps->SoundDevicePause(p->pSoundCtrl);
                //SoundDevicePause(p->pSoundCtrl);
            }
			if(p->pRawSndCtrl)
            {
                logd("pause raw sound devide.");
                p->pRawSndCtrlOps->SoundDevicePause_raw(p->pRawSndCtrl);
            }

            //* set status to paused.
            p->eStatus = PLAYER_STATUS_PAUSED;
            *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_QUIT)
        {
            logi("audio render process quit message.");
            if(p->pSoundCtrl)
            {
                logd("stop and release sound devide.");
                p->pSoundCtrlOps->SoundDeviceStop(p->pSoundCtrl);
                p->pSoundCtrlOps->SoundDeviceRelease(p->pSoundCtrl);
                //SoundDeviceStop(p->pSoundCtrl);
                //SoundDeviceRelease(p->pSoundCtrl);
                p->pSoundCtrl = NULL;
            }
			if(p->pRawSndCtrl)
			{
			    logd("stop and release raw sound devide.");
				p->pRawSndCtrlOps->SoundDeviceStop_raw(p->pRawSndCtrl);
                p->pRawSndCtrlOps->SoundDeviceRelease_raw(p->pRawSndCtrl);
                p->pRawSndCtrl = NULL;
			}

            sem_post(pReplySem);
            p->eStatus = PLAYER_STATUS_STOPPED;
            break;
        }
        else if(msg.messageId == MESSAGE_ID_RESET)
        {
            logi("audio render process reset message.");
            //* reset the sound device to flush data.
            if(p->pSoundCtrl)
            {
                logd("reset sound devide.");
                p->pSoundCtrlOps->SoundDeviceReset(p->pSoundCtrl);
                //SoundDeviceReset(p->pSoundCtrl);
            }
            if(0)//p->pRawSndCtrl)
			{
			    logd("reset raw_sound devide");
				p->pRawSndCtrlOps->SoundDeviceReset_raw(p->pRawSndCtrl);
			}
            //* clear the eos flag.
            p->bEosFlag = 0;
            bFirstFrameSend = 0;
            *pReplyValue = 0;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED)
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_EOS)
        {
            logi("audio render process set_eos message.");
            p->bEosFlag = 1;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED)
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_SET_AUDIO_SINK)
        {
            logi("audio render process set_audiosink message.");
            p->pAudioSink = (void*)msg.params[2];
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED)
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_RENDER)
        {
            logi("audio render process render message.");

            if(p->eStatus != PLAYER_STATUS_STARTED)
            {
                logw("not in started status, render message ignored.");
                continue;
            }

            //************************************************************************
            //* get the audio sample rate and channel num, set it to the sound device.
            //************************************************************************
            if(bFirstFrameSend == 0 && (nSampleRate == 0 || nChannelNum == 0))
            {
                do
                {
                    ret = AudioDecCompGetAudioSampleRate(p->pDecComp, &nSampleRate, &nChannelNum, &nBitRate);
                    if(ret == -1 || nSampleRate == 0 || nChannelNum == 0)
                    {
                        //* check whether stream end.
                        if(p->bEosFlag)
                            p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_EOS, NULL);

                        //* get stream info fail, decoder not initialized yet.
                        ret = MessageQueueTryGetMessage(p->mq, &msg, 10); //* wait for 10ms if no message come.
                        if(ret == 0)    //* new message come, quit loop to process.
                        {
                            if(msg.messageId != MESSAGE_ID_PAUSE &&
                               msg.messageId != MESSAGE_ID_STOP  &&
                               msg.messageId != MESSAGE_ID_QUIT)
                            {
                                //* post a render message to continue the rendering job after message processed.
                                PostRenderMessage(p->mq);
                            }
                            goto process_message;
                        }
                    }
                    else
                    {
                        logd("init sound device.");
                        p->pSoundCtrl = p->pSoundCtrlOps->SoundDeviceInit(p->pAudioSink);
                        if(p->pSoundCtrl == NULL)
                        {
                            loge("can not initialize the sound device.");
                            abort();
                        }

						p->pSoundCtrlOps->SoundDeviceSetCallback(p->pSoundCtrl, (SndCallback)SoundCallback, (void*)p);
						p->pSoundCtrlOps->SoundDeviceSetVolume(p->pSoundCtrl, p->volume);

                        logd("set sound devide param, sample rate = %d, channel num = %d.",
                                nSampleRate, nChannelNum);
                        p->pSoundCtrlOps->SoundDeviceSetFormat(p->pSoundCtrl, nSampleRate, nChannelNum);
                        nFrameSize = nChannelNum*nBitsPerSample/8;
                        break;
                    }
                }while(1);
            }

            //*******************************************
            //* 1. request pcm data and pts from decoder.
            //*******************************************
            do
            {
                nPcmDataLen = (nSampleRate*40/1000)*nFrameSize; //* pcm data of 40ms
                pPcmData    = NULL;
				memset(&raw_data,0,sizeof(cedar_raw_data));
                ret = AudioDecCompRequestPcmData(p->pDecComp, &pPcmData, &nPcmDataLen, &nPts,&raw_data);
                logv("audio render, ret = %d, pPcmData = %p",ret,pPcmData);
                if(ret < 0 || pPcmData == NULL || nPcmDataLen < nFrameSize)
                {
                    //* check whether stream end.
                    if(p->bEosFlag)
                    {
						//check we how much cache we have outside cedarx
						int cacheout = p->pSoundCtrlOps->SoundDeviceGetCachedTime(p->pSoundCtrl);
						logd("we have %d ms",cacheout/1000);
						while(1)
                        {
							ret = MessageQueueTryGetMessage(p->mq, &msg, 5);
							if(ret == 0){
								if(msg.messageId != MESSAGE_ID_PAUSE &&
								msg.messageId != MESSAGE_ID_STOP  &&
								msg.messageId != MESSAGE_ID_QUIT)
								{
									PostRenderMessage(p->mq);
								}
								goto process_message;
							}
	                        cacheout = p->pSoundCtrlOps->SoundDeviceGetCachedTime(p->pSoundCtrl);
	                        logd("we still have %d ms",cacheout/1000);
	                        if(cacheout < 10*1000)
	                        {
	                            loge("we quit but still have %d ms",cacheout/1000);
	                            break;
	                        }
                        }
                        p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_EOS, NULL);
                        goto get_message;
                    }

                    //* if no pcm data, wait some time.
                    ret = MessageQueueTryGetMessage(p->mq, &msg, 10); //* wait for 5ms if no message come.
                    if(ret == 0)    //* new message come, quit loop to process.
                    {
                        if(msg.messageId != MESSAGE_ID_PAUSE &&
                           msg.messageId != MESSAGE_ID_STOP  &&
                           msg.messageId != MESSAGE_ID_QUIT)
                        {
                            //* post a render message to continue the rendering job after message processed.
                            PostRenderMessage(p->mq);
                        }
                        goto process_message;
                    }
                }
                else
		{
		    if(p->pRawSndCtrl == NULL)
			{
				if(p->pRawSndCtrlOps->SoundDeviceInit_raw)
				{
					p->pRawSndCtrl = p->pRawSndCtrlOps->SoundDeviceInit_raw(&raw_data,p->pDecComp,AudioDecRawSendCmdToHalClbk);
				}

                        if(p->pRawSndCtrl)
                        {
							p->pRawSndCtrlOps->SoundDeviceStart_raw(p->pRawSndCtrl);
                        }
					}
					break;  //* get frame success.
                }
            }while(1);

            ret = AudioDecCompGetAudioSampleRate(p->pDecComp,&nNewSampleRate,&nNewChannelNum,&nNewBitRate);
            if(ret == 0)
            {
		logv("$$$$$$$%d, %d, %d, %d, %d, %d, %d", nNewSampleRate, nSampleRate, nNewChannelNum, nChannelNum,
					nNewBitRate, nBitRate, audioInfoNotified);
				if((nNewSampleRate != nSampleRate) || (nNewChannelNum != nChannelNum) || (nNewBitRate != nBitRate) || !audioInfoNotified)
				{
					int nAudioInfo[3];
					nAudioInfo[0] = nNewSampleRate;
					nAudioInfo[1] = nNewChannelNum;
					nAudioInfo[2] = nBitRate = nNewBitRate;

					ret = p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_AUDIO_INFO, (void*)nAudioInfo);
					audioInfoNotified = 1;
				}
		if((nNewSampleRate != nSampleRate) || (nNewChannelNum != nChannelNum))
		{
		    logw("sample rate change from %d to %d.", nSampleRate, nNewSampleRate);
		    logw("channel num change from %d to %d.", nChannelNum, nNewChannelNum);
			ret = p->pSoundCtrlOps->SoundDeviceStop(p->pSoundCtrl);
			if(ret == 0)
			{
				p->pSoundCtrlOps->SoundDeviceSetFormat(p->pSoundCtrl, nNewSampleRate, nNewChannelNum);
                        nSampleRate = nNewSampleRate;
				nChannelNum = nNewChannelNum;
                        nFrameSize  = nChannelNum*nBitsPerSample/8;
				if(bFirstFrameSend == 1)
				{
					logw("start sound devide again because samplaRate or channelNum change");
					p->pSoundCtrlOps->SoundDeviceStart(p->pSoundCtrl);
				}
			}
		}
            }

            //**********************************************************
            //* 2. If first frame, call back and start the sound device.
            //**********************************************************
            if(bFirstFrameSend == 0)
            {
                ret = p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_FIRST_FRAME, (void*)&nPts);
                if(ret == TIMER_DROP_AUDIO_DATA)
                {
                    AudioDecCompReleasePcmData(p->pDecComp, nPcmDataLen);
                    //* post a render message to continue the rendering job after message processed.
                    PostRenderMessage(p->mq);
                    continue;
                }
                else if(ret == TIMER_NEED_NOTIFY_AGAIN)
                {
                    //* waiting process for first frame sync with video is broken by a new message to player, so the player tell us to notify again later.
                    //* post a render message to continue the rendering job after message processed.
                    ret = MessageQueueTryGetMessage(p->mq, &msg, 10); //* wait for 40ms if no message come.
                    if(ret == 0)    //* new message come, quit loop to process.
                        goto process_message;
                    PostRenderMessage(p->mq);
                    continue;
                }

                logd("start sound device.");
				if(p->pRawSndCtrl)
		{
			//SoundDeviceStart_raw(p->pRawSndCtrl);   we have open once   dont need open pcm again
		}
				else
				{
					p->pSoundCtrlOps->SoundDeviceStart(p->pSoundCtrl);
				}
                bFirstFrameSend = 1;
            }


            //******************************************************************
            //* 3. report timer difference for the player to adjust timer speed.
            //******************************************************************
            if(nPts >= 0)
            {
                int64_t nCachedTime;
                int64_t callbackParam[2];
                nCachedTime = p->pSoundCtrlOps->SoundDeviceGetCachedTime(p->pSoundCtrl);
                callbackParam[0] = nPts;
                callbackParam[1] = nCachedTime;
                ret = p->callback(p->pUserData, PLAYER_AUDIO_RENDER_NOTIFY_PTS_AND_CACHETIME, (void*)callbackParam);
                if(ret == TIMER_DROP_AUDIO_DATA)
                {
                    AudioDecCompReleasePcmData(p->pDecComp, nPcmDataLen);
                    //* post a render message to continue the rendering job after message processed.
                    PostRenderMessage(p->mq);
                    continue;
                }
            }

            //********************************
            //* 4. Write data to sound device.
            //********************************
            ProcessBalance(pPcmData, nPcmDataLen, 16, nChannelNum, p->nConfigOutputBalance);
            ProcessMute(pPcmData, nPcmDataLen, p->bMute);
            if((nPcmDataLen % nFrameSize != 0)&&(!p->pRawSndCtrl))
            {
                logw("!!! invalid pcm length, nPcmDataLen = %d, nFrameSize = %d", nPcmDataLen, nFrameSize);
                nPcmDataLen /= nFrameSize;
                nPcmDataLen *= nFrameSize;
            }

			//* we set data to 0 when the bForceWriteToDeviceFlag is 1
			if(p->bForceWriteToDeviceFlag == 1)
				memset(pPcmData, 0, nPcmDataLen);

            while(nPcmDataLen > 0)
            {
				if(p->pRawSndCtrl)
		{
			nWritten = p->pRawSndCtrlOps->SoundDeviceWrite_raw(p->pRawSndCtrl,pPcmData, nPcmDataLen);
		}
				else
				{
					nWritten = p->pSoundCtrlOps->SoundDeviceWrite(p->pSoundCtrl, pPcmData, nPcmDataLen);
				}
                if(nWritten > 0)
                {
                    nPcmDataLen -= nWritten;
                    pPcmData    += nWritten;
                    AudioDecCompReleasePcmData(p->pDecComp, nWritten);
                }
                else
                {
                    //* if no buffer to write, wait some time.
                    ret = MessageQueueTryGetMessage(p->mq, &msg, 40); //* wait for 40ms if no message come.
                    if(ret == 0)    //* new message come, quit loop to process.
                    {
                        if(msg.messageId != MESSAGE_ID_PAUSE &&
                           msg.messageId != MESSAGE_ID_STOP  &&
                           msg.messageId != MESSAGE_ID_QUIT)
                        {
                            //* post a render message to continue the rendering job after message processed.
                            PostRenderMessage(p->mq);
                            AudioDecCompReleasePcmData(p->pDecComp, nPcmDataLen);
                        }
                        goto process_message;
                    }
                }
            }

            PostRenderMessage(p->mq);
        }
        else
        {
            //* unknown message.
            if(pReplyValue != NULL)
                *pReplyValue = -1;
            if(pReplySem)
                sem_post(pReplySem);
        }
    }

    ret = 0;
    pthread_exit(&ret);

    return NULL;
}



int AudioRenderCompGetAudioOutBalance(AudioRenderComp* a)
{
    AudioRenderCompContext* p;
    int                     nBalance;

    p = (AudioRenderCompContext*)a;

    nBalance = p->nConfigOutputBalance;
    if(nBalance == 0)
        nBalance = 3;

    return nBalance;
}

//* set audio balance, 1 means left channel, 2 means right channel, 3 means stereo.
int AudioRenderCompSetAudioOutBalance(AudioRenderComp* a, int nBalance)
{
    AudioRenderCompContext* p;

    p = (AudioRenderCompContext*)a;

    if(nBalance < 0 || nBalance>3)
        return -1;

    p->nConfigOutputBalance = nBalance;
    return 0;
}


int AudioRenderCompSetAudioMute(AudioRenderComp* a, int bMute)
{
    AudioRenderCompContext* p;

    p = (AudioRenderCompContext*)a;

    p->bMute = bMute;
    return 0;
}


int AudioRenderCompGetAudioMuteFlag(AudioRenderComp* a)
{
    AudioRenderCompContext* p;
    p = (AudioRenderCompContext*)a;
    return p->bMute;
}

int AudioRenderCompSetForceWriteToDeviceFlag(AudioRenderComp* a, int bForceFlag)
{
    AudioRenderCompContext* p;
    p = (AudioRenderCompContext*)a;
    p->bForceWriteToDeviceFlag = bForceFlag;

    return 0;
}

static void PostRenderMessage(MessageQueue* mq)
{
    if(MessageQueueGetCount(mq)<=0)
    {
        Message msg;
        msg.messageId = MESSAGE_ID_RENDER;
        msg.params[0] = msg.params[1] = msg.params[2] = msg.params[3] = 0;
        if(MessageQueuePostMessage(mq, &msg) != 0)
        {
            loge("fatal error, audio render component post message fail.");
            abort();
        }

        return;
    }
}


static void ProcessBalance(unsigned char* pData, int nDataLen, int nBitsPerSample, int nChannelCount, int nOutBalance)
{
    int    nSampleCount;
    int    nBytesPerSample;
    int    i;
    short* pShortData;

    if(nOutBalance == 0 || nOutBalance == 3)
        return;

    if(nChannelCount < 2)
        return;

    //* don't know how to do with data more than 2 channels.
    //* currently decoder mix all channel to 2 channel for render.
    if(nChannelCount > 2)
        return;

    //* currently decoder only output 16 bits width data.
    if(nBitsPerSample != 16)
        return;

    nBytesPerSample = nBitsPerSample>>3;
    nSampleCount = nDataLen/nBytesPerSample;
    pShortData   = (short*)pData;

    if(nOutBalance == 1)
    {
        for(i=0; i<nSampleCount; i+=2)
            pShortData[i+1] = pShortData[i];
    }
    else
    {
        for(i=0; i<nSampleCount; i+=2)
            pShortData[i] = pShortData[i+1];
    }
}

static void ProcessMute(unsigned char* pData, int nDataLen, int bMute)
{
    if(bMute != 0)
        memset(pData, 0, nDataLen);
    return;
}
int AudioRenderCompSetVolume(AudioRenderComp* a, float volume)
{
    AudioRenderCompContext* p = (AudioRenderCompContext*)a;
	p->volume = volume;
	if(!p->pSoundCtrl)
	{
		logw("pSoundCtrl == NULL");
		return -1;
	}
	return p->pSoundCtrlOps->SoundDeviceSetVolume(p->pSoundCtrl, p->volume);
}
int AudioRenderCompGetVolume(AudioRenderComp* a, float *volume)
{
    AudioRenderCompContext* p = (AudioRenderCompContext*)a;
	*volume = p->volume;
	if(!p->pSoundCtrl)
	{
		logw("pSoundCtrl == NULL");
		return 0;
	}
	return p->pSoundCtrlOps->SoundDeviceGetVolume(p->pSoundCtrl, volume);
}
