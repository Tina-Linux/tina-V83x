
#include "log.h"
#ifdef __cplusplus
extern "C"
{
#endif
#include "subtitleNativeDisplayApi.h"
#ifdef __cplusplus
}
#endif

#include <pthread.h>
#include <semaphore.h>
#include <malloc.h>
#include <memory.h>
#include <time.h>

#include "subtitleRenderComponent.h"
#include "subtitleDecComponent.h"
#include "messageQueue.h"

#define MAX_SUBTITLE_ITEM (32)  //* support 32 subtitle items show at the same time at most.

//* record subtitle time info.
typedef struct SubtitleItemInfo
{
    int     nId;
    int     bValid;
    int64_t nPts;
    int64_t nDuration;
}SubtitleItemInfo;

typedef struct SubtitleRenderCompContext
{
    //* created at initialize time.
    MessageQueue*       mq;
    sem_t               startMsgReplySem;
    sem_t               stopMsgReplySem;
    sem_t               pauseMsgReplySem;
    sem_t               resetMsgReplySem;
    sem_t               eosMsgReplySem;
    sem_t               quitMsgReplySem;
    sem_t               timeshiftMsgReplySem;

    int                 nStartReply;
    int                 nStopReply;
    int                 nPauseReply;
    int                 nResetReply;

    pthread_t           sRenderThread;

    enum EPLAYERSTATUS  eStatus;
    SubtitleDecComp*    pDecComp;

    //* objects set by user.
    AvTimer*            pAvTimer;
    PlayerCallback      callback;
    void*               pUserData;
    int                 bEosFlag;
    int64_t             nTimeShiftUs;  //* adjustment set by user.
    int64_t				nVideoOrAudioFirstPts;
	int					bExternalFlag;

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES)
	//* AliYUNOS Idx+Sub subtitle render in CedarX
	SubtitleStreamInfo*     pStreamInfo;
	int                     nStreamCount;
	int                     nStreamSelected;
	bool					bIdxSubFlag;
#endif

}SubtitleRenderCompContext;


static void* SubtitleRenderThread(void* arg);
static void PostRenderMessage(MessageQueue* mq);
static void FlushExpiredItems(SubtitleRenderCompContext* p, SubtitleItemInfo* pItemInfo, int64_t nTimeDelayUs);
static void FlushAllItems(SubtitleRenderCompContext* p, SubtitleItemInfo* pItemInfo);
static int AddItemInfo(SubtitleItemInfo* pItemInfo, SubtitleItem* pItem, int nId);

SubtitleRenderComp* SubtitleRenderCompCreate(void)
{
    SubtitleRenderCompContext* p;
    int                        err;

    p = (SubtitleRenderCompContext*)malloc(sizeof(SubtitleRenderCompContext));
    if(p == NULL)
    {
        loge("memory alloc fail.");
        return NULL;
    }
    memset(p, 0, sizeof(*p));

    p->mq = MessageQueueCreate(4, "SubtitleRenderMq");
    if(p->mq == NULL)
    {
        loge("subtitle render component create message queue fail.");
        free(p);
        return NULL;
    }

    sem_init(&p->startMsgReplySem, 0, 0);
    sem_init(&p->stopMsgReplySem, 0, 0);
    sem_init(&p->pauseMsgReplySem, 0, 0);
    sem_init(&p->resetMsgReplySem, 0, 0);
    sem_init(&p->eosMsgReplySem, 0, 0);
    sem_init(&p->quitMsgReplySem, 0, 0);
    sem_init(&p->timeshiftMsgReplySem, 0, 0);

    p->eStatus = PLAYER_STATUS_STOPPED;

    err = pthread_create(&p->sRenderThread, NULL, SubtitleRenderThread, p);
    if(err != 0)
    {
        loge("subtitle render component create thread fail.");
        sem_destroy(&p->startMsgReplySem);
        sem_destroy(&p->stopMsgReplySem);
        sem_destroy(&p->pauseMsgReplySem);
        sem_destroy(&p->resetMsgReplySem);
        sem_destroy(&p->eosMsgReplySem);
        sem_destroy(&p->quitMsgReplySem);
        sem_destroy(&p->timeshiftMsgReplySem);
        MessageQueueDestroy(p->mq);
        free(p);
        return NULL;
    }

    return (SubtitleRenderComp*)p;
}


int SubtitleRenderCompDestroy(SubtitleRenderComp* s)
{
    void*                      status;
    SubtitleRenderCompContext* p;
    Message                    msg;

    p = (SubtitleRenderCompContext*)s;

    msg.messageId = MESSAGE_ID_QUIT;
    msg.params[0] = (uintptr_t)&p->quitMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle render component post message fail.");
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
    sem_destroy(&p->timeshiftMsgReplySem);

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES)
    if(p->pStreamInfo != NULL)
    {
		free(p->pStreamInfo);
		p->pStreamInfo = NULL;
    }
#endif

    MessageQueueDestroy(p->mq);
    free(p);

    return 0;
}


int SubtitleRenderCompStart(SubtitleRenderComp* s)
{
    SubtitleRenderCompContext* p;
    Message                    msg;

    p = (SubtitleRenderCompContext*)s;

    logv("subtitle render component starting");

    msg.messageId = MESSAGE_ID_START;
    msg.params[0] = (uintptr_t)&p->startMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStartReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->startMsgReplySem, -1) < 0)
    {
        loge("subtitle render component wait for start finish timeout.");
        return -1;
    }

#if( (CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES) && (CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_4))
	if(p->bIdxSubFlag)
#else
	if(ENABLE_SUBTITLE_DISPLAY_IN_CEDARX == 1)
#endif
	{
		SubRenderCreate();
		//SubRenderSetZorderTop();
	}

    return p->nStartReply;
}


int SubtitleRenderCompStop(SubtitleRenderComp* s)
{
    SubtitleRenderCompContext* p;
    Message                    msg;

    p = (SubtitleRenderCompContext*)s;

    logv("subtitle render component stopping");

    msg.messageId = MESSAGE_ID_STOP;
    msg.params[0] = (uintptr_t)&p->stopMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nStopReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->stopMsgReplySem, -1) < 0)
    {
        loge("video render component wait for stop finish timeout.");
        return -1;
    }

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES && CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_4)
	if(p->bIdxSubFlag)
#else
	if(ENABLE_SUBTITLE_DISPLAY_IN_CEDARX == 1)
#endif
	{
		SubRenderHide(0xFFFFFFFF, NULL);
		SubRenderDestory();
	}

    return p->nStopReply;
}


int SubtitleRenderCompPause(SubtitleRenderComp* s)
{
    SubtitleRenderCompContext* p;
    Message                    msg;

    p = (SubtitleRenderCompContext*)s;

    logv("subtitle render component pausing");

    msg.messageId = MESSAGE_ID_PAUSE;
    msg.params[0] = (uintptr_t)&p->pauseMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nPauseReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->pauseMsgReplySem, -1) < 0)
    {
        loge("subtitle render component wait for pause finish timeout.");
        return -1;
    }

    return p->nPauseReply;
}


enum EPLAYERSTATUS SubtitleRenderCompGetStatus(SubtitleRenderComp* s)
{
    SubtitleRenderCompContext* p;
    p = (SubtitleRenderCompContext*)s;
    return p->eStatus;
}


int SubtitleRenderCompReset(SubtitleRenderComp* s)
{
    SubtitleRenderCompContext* p;
    Message                    msg;

    p = (SubtitleRenderCompContext*)s;

    logv("subtitle render component reseting");

    msg.messageId = MESSAGE_ID_RESET;
    msg.params[0] = (uintptr_t)&p->resetMsgReplySem;
    msg.params[1] = (uintptr_t)&p->nResetReply;
    msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->resetMsgReplySem, -1) < 0)
    {
        loge("subtitle render component wait for reset finish timeout.");
        return -1;
    }

    return p->nResetReply;
}


int SubtitleRenderCompSetEOS(SubtitleRenderComp* s)
{
    SubtitleRenderCompContext* p;
    Message                    msg;

    p = (SubtitleRenderCompContext*)s;

    logv("subtitle render component setting EOS.");

    msg.messageId = MESSAGE_ID_EOS;
    msg.params[0] = (uintptr_t)&p->eosMsgReplySem;
    msg.params[1] = msg.params[2] = msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->eosMsgReplySem, -1) < 0)
    {
        loge("subtitle render component wait for setting eos finish timeout.");
        return -1;
    }

    return 0;
}


int SubtitleRenderCompSetCallback(SubtitleRenderComp* s, PlayerCallback callback, void* pUserData)
{
    SubtitleRenderCompContext* p;

    p = (SubtitleRenderCompContext*)s;

    p->callback  = callback;
    p->pUserData = pUserData;

    return 0;
}


int SubtitleRenderCompSetTimer(SubtitleRenderComp* s, AvTimer* timer)
{
    SubtitleRenderCompContext* p;
    p = (SubtitleRenderCompContext*)s;
    p->pAvTimer  = timer;
    return 0;
}


int SubtitleRenderCompSetDecodeComp(SubtitleRenderComp* s, SubtitleDecComp* d)
{
    SubtitleRenderCompContext* p;
    p = (SubtitleRenderCompContext*)s;
    p->pDecComp  = d;
    return 0;
}


int SubtitleRenderSetShowTimeAdjustment(SubtitleRenderComp* s, int nTimeMs)
{
    SubtitleRenderCompContext* p;
    Message                    msg;

    p = (SubtitleRenderCompContext*)s;

    logv("subtitle render component starting");

    msg.messageId = MESSAGE_ID_SET_TIMESHIFT;
    msg.params[0] = (uintptr_t)&p->timeshiftMsgReplySem;
    msg.params[1] = 0;
    msg.params[2] = (uintptr_t)nTimeMs;
    msg.params[3] = 0;

    if(MessageQueuePostMessage(p->mq, &msg) != 0)
    {
        loge("fatal error, subtitle render component post message fail.");
        abort();
    }

    if(SemTimedWait(&p->timeshiftMsgReplySem, -1) < 0)
    {
        loge("subtitle render component wait for time shift setting finish timeout.");
        return -1;
    }

    return 0;
}


int SubtitleRenderGetShowTimeAdjustment(SubtitleRenderComp* s)
{
    SubtitleRenderCompContext* p;
    p = (SubtitleRenderCompContext*)s;
    return p->nTimeShiftUs;
}

int SubtitleRenderCompSetVideoOrAudioFirstPts(SubtitleRenderComp* s,int64_t nFirstPts)
{
	SubtitleRenderCompContext* p;
    p = (SubtitleRenderCompContext*)s;
	p->nVideoOrAudioFirstPts = nFirstPts;
	return 0;
}

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES)
int SubtitleRenderCompSetSubtitleStreamInfo(SubtitleRenderComp*	s,
                                         SubtitleStreamInfo*  pStreamInfo,
                                         int                  nStreamCount,
                                         int                  nDefaultStreamIndex)
{
    SubtitleRenderCompContext*		p;
    int	i;

    p = (SubtitleRenderCompContext*)s;

    //* free old SubtitleStreamInfo.
    if(p->pStreamInfo != NULL)
    {
        free(p->pStreamInfo);
        p->pStreamInfo = NULL;
    }

    p->nStreamSelected = 0;
    p->nStreamCount = 0;
	p->bIdxSubFlag = false;

    //* set SubtitleStreamInfo.
    p->pStreamInfo = (SubtitleStreamInfo*)malloc(sizeof(SubtitleStreamInfo)*nStreamCount);
    if(p->pStreamInfo == NULL)
    {
        loge("memory malloc fail!");
        return -1;
    }
    memset(p->pStreamInfo, 0, sizeof(SubtitleStreamInfo)*nStreamCount);

    for(i=0; i<nStreamCount; i++)
    {
        memcpy(&p->pStreamInfo[i], &pStreamInfo[i], sizeof(SubtitleStreamInfo));
    }

    if(i != nStreamCount)
    {
		free(p->pStreamInfo);
		p->pStreamInfo = NULL;
		return -1;
    }

    p->nStreamSelected = nDefaultStreamIndex;
    p->nStreamCount = nStreamCount;

	if(p->pStreamInfo[p->nStreamSelected].eCodecFormat == SUBTITLE_CODEC_IDXSUB)
	{
		p->bIdxSubFlag = true;
		logi("This subtitle is IdxSub.");
	}

    return 0;
}

int SubtitleRenderCompSwitchStream(SubtitleRenderComp* s, int nStreamIndex)
{
    SubtitleRenderCompContext* p;
    p = (SubtitleRenderCompContext*)s;

    if(p->eStatus != PLAYER_STATUS_STOPPED)
    {
        loge("can not switch status when subtitle decoder is not in stopped status.");
        return -1;
    }

	p->bIdxSubFlag = false;
	p->nStreamSelected = nStreamIndex;

	if(p->pStreamInfo[p->nStreamSelected].eCodecFormat == SUBTITLE_CODEC_IDXSUB)
	{
		p->bIdxSubFlag = true;
		logi("This subtitle is IdxSub.");
	}

    return 0;
}
#endif

static void* SubtitleRenderThread(void* arg)
{
    SubtitleRenderCompContext* p;
    Message                    msg;
    int                        ret;
    int                        i;
    sem_t*                     pReplySem;
    int*                       pReplyValue;
    int64_t                    nCurTime;
    int64_t                    nWaitTimeMs;

    unsigned int               nIdCounter;
    SubtitleItemInfo           pItemInfo[MAX_SUBTITLE_ITEM];
    SubtitleItem*              pCurItem;

    p = (SubtitleRenderCompContext*)arg;
    nIdCounter = 0;
    pCurItem = NULL;
    for(i=0; i<MAX_SUBTITLE_ITEM; i++)
        pItemInfo[i].bValid = 0;

    while(1)
    {
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
            if(p->eStatus == PLAYER_STATUS_STARTED)
            {
                logw("already in started status.");
                *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            if(p->eStatus == PLAYER_STATUS_STOPPED)
            {
                p->bEosFlag = 0;
            }

			if(p->pDecComp != NULL)
				p->bExternalFlag = SubtitleDecCompGetExternalFlag(p->pDecComp);

            //* send a render message to start decoding.
            PostRenderMessage(p->mq);

            p->eStatus = PLAYER_STATUS_STARTED;
            *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_STOP)
        {
            if(p->eStatus == PLAYER_STATUS_STOPPED)
            {
                logw("already in stopped status.");
                *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            //* clear all items.
            FlushAllItems(p, pItemInfo);
            nIdCounter = 0;

            //* set status to stopped.
            p->eStatus = PLAYER_STATUS_STOPPED;
            *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_PAUSE)
        {
            if(p->eStatus != PLAYER_STATUS_STARTED)
            {
                logw("not in started status, pause operation invalid.");
                *pReplyValue = -1;
                sem_post(pReplySem);
                continue;
            }

            //* set status to paused.
            p->eStatus = PLAYER_STATUS_PAUSED;
            *pReplyValue = 0;
            sem_post(pReplySem);
        }
        else if(msg.messageId == MESSAGE_ID_QUIT)
        {
            //* clear all items.
            FlushAllItems(p, pItemInfo);

            sem_post(pReplySem);
            p->eStatus = PLAYER_STATUS_STOPPED;
            break;
        }
        else if(msg.messageId == MESSAGE_ID_RESET)
        {
            //* clear all items.
            FlushAllItems(p, pItemInfo);
            nIdCounter = 0;

            //* clear the eos flag.
            p->bEosFlag = 0;
            *pReplyValue = 0;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED)
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_EOS)
        {
            p->bEosFlag = 1;
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED)
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_SET_TIMESHIFT)
        {
            p->nTimeShiftUs = (int)msg.params[2];
            p->nTimeShiftUs *= 1000; //* transform ms to us.
            sem_post(pReplySem);

            //* send a message to continue the thread.
            if(p->eStatus == PLAYER_STATUS_STARTED)
                PostRenderMessage(p->mq);
        }
        else if(msg.messageId == MESSAGE_ID_RENDER)
        {
            logv("process render message.");
            if(p->eStatus != PLAYER_STATUS_STARTED)
            {
                logw("not in started status, render message ignored.");
                continue;
            }

            //* clear expired subtitle items and return buffer to decoder.
            if(p->bExternalFlag == 1)
		FlushExpiredItems(p, pItemInfo, p->nTimeShiftUs + p->nVideoOrAudioFirstPts);
			else
				FlushExpiredItems(p, pItemInfo, p->nTimeShiftUs);

            //* check whether there is a new item.
            pCurItem = SubtitleDecCompNextSubtitleItem(p->pDecComp);
            if(pCurItem == NULL)
            {
                //* check whether stream end.
                if(p->bEosFlag)
                {
                    p->callback(p->pUserData, PLAYER_SUBTITLE_RENDER_NOTIFY_EOS, NULL);
                    continue;
                }

                nWaitTimeMs = 100;  //* wait for 100ms if no message come.
                ret = MessageQueueTryGetMessage(p->mq, &msg, nWaitTimeMs);
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
                else
                {
                    //* post a render message to continue the rendering job after message processed.
                    PostRenderMessage(p->mq);
                    continue;
                }
            }

            //* check whether it is time to show the item.
            int64_t callbackParam64[3];
			if(p->bExternalFlag == 1)
		callbackParam64[0] = pCurItem->nPts + p->nTimeShiftUs + p->nVideoOrAudioFirstPts;
			else
				callbackParam64[0] = pCurItem->nPts + p->nTimeShiftUs;
            callbackParam64[1] = pCurItem->nDuration;
            callbackParam64[2] = 0; //* this item not showed yet.
            ret = p->callback(p->pUserData, PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_PTS_AND_DURATION, (void*)callbackParam64);
            if(ret > 0)
            {
                //* not the time to show this item, wait.
                //* don't wait in a loop, just wait no more than 100ms, because we need to
                //* notify expired message of other subtitle items, and also we need to
                //* callback again to check pts again.
                //* (there may be a pts jump event which make the nWaitTimeMs jump.)
                nWaitTimeMs = (ret < 100) ? ret : 100;
                ret = MessageQueueTryGetMessage(p->mq, &msg, nWaitTimeMs);
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
            else if(ret < 0)
            {
                //* item expired, flush it.
                logw("skip one subtitle item, start time = %lld us, duration = %lld us.",
                        pCurItem->nPts, pCurItem->nDuration);
                pCurItem = SubtitleDecCompRequestSubtitleItem(p->pDecComp);
                SubtitleDecCompFlushSubtitleItem(p->pDecComp, pCurItem);
            }
            else
            {
                //* time to show the item.
                pCurItem = SubtitleDecCompRequestSubtitleItem(p->pDecComp);

                //* check whether a clear command.
                if(pCurItem->pText == NULL && pCurItem->pBitmapData == NULL)
                {
                    //* it is a subtitle clear command, release all the subtitle item.
                    FlushAllItems(p, pItemInfo);
                    SubtitleDecCompFlushSubtitleItem(p->pDecComp, pCurItem);
                }
                else
                {

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES && CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_4)
					if(p->bIdxSubFlag)
#else
					if(ENABLE_SUBTITLE_DISPLAY_IN_CEDARX == 1)
#endif
					{
						/************************begin****************************/
						//add something for subtilte display in cedarx
						sub_item_inf dispSubItem ;
						memset(&dispSubItem, 0, sizeof(sub_item_inf));

                        dispSubItem.subMode = 0;
						dispSubItem.startx  = -1;             // the invalid value is -1
						dispSubItem.starty  = -1;             // the invalid value is -1
						dispSubItem.endx    = -1;               // the invalid value is -1
						dispSubItem.endy    = -1;               // the invalid value is -1
						dispSubItem.subTextBuf = (unsigned char*)pCurItem->pText;         // the data buffer of the text subtitle
						dispSubItem.subTextLen = (unsigned int)pCurItem->nTextLength;         // the length of the text subtitle
						dispSubItem.encodingType = pCurItem->eTextFormat;       // the encoding tyle of the text subtitle

						dispSubItem.subBitmapBuf = (unsigned char*)pCurItem->pBitmapData;

						if(dispSubItem.subTextBuf)
						{
							dispSubItem.subMode = 0;
						}
						else if(dispSubItem.subBitmapBuf)
						{
							dispSubItem.subMode = 1;
						}
						dispSubItem.subPicWidth = pCurItem->nBitmapWidth;
						dispSubItem.subPicHeight = pCurItem->nBitmapHeight;

						SubRenderDraw(&dispSubItem);
						SubRenderShow();
					}
                    else
                    {
                        uintptr_t callbackParam[2];
                        callbackParam[0] = (uintptr_t)nIdCounter;
                        callbackParam[1] = (uintptr_t)pCurItem;
                        p->callback(p->pUserData, PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_AVAILABLE, (void*)callbackParam);
                    }

					/************************begin****************************/

                    //* add the item to item info array.
                    ret = AddItemInfo(pItemInfo, pCurItem, nIdCounter++);
                    if(ret < 0)
                    {
                        loge("more than 32 items showed, should not run here.");
                        abort();
                    }

                    //* return the item.
                    SubtitleDecCompFlushSubtitleItem(p->pDecComp, pCurItem);
                }
            }

            //* post a render message to continue the rendering job after message processed.
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


static void PostRenderMessage(MessageQueue* mq)
{
    if(MessageQueueGetCount(mq)<=0)
    {
        Message msg;
        msg.messageId = MESSAGE_ID_RENDER;
        msg.params[0] = msg.params[1] = msg.params[2] = msg.params[3] = 0;
        if(MessageQueuePostMessage(mq, &msg) != 0)
        {
            loge("fatal error, subtitle render component post message fail.");
            abort();
        }

        return;
    }
}


static void FlushExpiredItems(SubtitleRenderCompContext* p, SubtitleItemInfo* pItemInfo,int64_t nTimeDelayUs)
{
    int     i;
    int64_t nPts;
    int64_t nDuration;
    int     ret;
    int64_t callbackParam[3];

    for(i=0; i<MAX_SUBTITLE_ITEM; i++)
    {
        if(pItemInfo[i].bValid == 0)
            continue;

        nPts      = pItemInfo[i].nPts;
        nDuration = pItemInfo[i].nDuration;

        callbackParam[0] = nPts + nTimeDelayUs;
        callbackParam[1] = nDuration;
        callbackParam[2] = 1;   //* this item has been showed, we just check whether it is time to clear it.
        ret = p->callback(p->pUserData, PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_PTS_AND_DURATION, (void*)callbackParam);

        if(ret < 0)
        {
            pItemInfo[i].bValid = 0;

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES && CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_4)
			if(p->bIdxSubFlag)
#else
			if(ENABLE_SUBTITLE_DISPLAY_IN_CEDARX == 1)
#endif
			{
				SubRenderHide(0xFFFFFFFF, NULL);
			}
            else
            {
                p->callback(p->pUserData, PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_EXPIRED, (void*)(&pItemInfo[i].nId));
            }
        }
    }

    return;
}


static void FlushAllItems(SubtitleRenderCompContext* p, SubtitleItemInfo* pItemInfo)
{
    int     i;
    int     ret;
    int64_t callbackParam[2];

    for(i=0; i<MAX_SUBTITLE_ITEM; i++)
    {
        if(pItemInfo[i].bValid)
        {
            pItemInfo[i].bValid = 0;

#if( CONFIG_ALI_YUNOS == OPTION_ALI_YUNOS_YES && CONFIG_OS_VERSION == OPTION_OS_VERSION_ANDROID_4_4)
			if(p->bIdxSubFlag)
#else
			if(ENABLE_SUBTITLE_DISPLAY_IN_CEDARX == 1)
#endif
			{
				SubRenderHide(0xFFFFFFFF, NULL);
			}
            else
            {
                p->callback(p->pUserData, PLAYER_SUBTITLE_RENDER_NOTIFY_ITEM_EXPIRED, (void*)(&pItemInfo[i].nId));
            }
        }
    }

    return;
}


static int AddItemInfo(SubtitleItemInfo* pItemInfo, SubtitleItem* pItem, int nId)
{
    int i;

    for(i=0; i<MAX_SUBTITLE_ITEM; i++)
    {
        if(pItemInfo[i].bValid == 0)
        {
            pItemInfo[i].bValid    = 1;
            pItemInfo[i].nId       = nId;
            pItemInfo[i].nPts      = pItem->nPts;
            pItemInfo[i].nDuration = pItem->nDuration;
            break;
        }
    }

    if(i == MAX_SUBTITLE_ITEM)
        return -1;

    return 0;
}
