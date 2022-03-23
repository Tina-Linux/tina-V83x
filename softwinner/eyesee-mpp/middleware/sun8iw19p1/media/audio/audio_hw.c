/******************************************************************************
  Copyright (C), 2001-2016, Allwinner Tech. Co., Ltd.
 ******************************************************************************
  File Name     : audio_hw.c
  Version       : Initial Draft
  Author        : Allwinner BU3-PD2 Team
  Created       : 2016/05/25
  Last Modified :
  Description   : mpi functions implement
  Function List :
  History       :
******************************************************************************/

#define LOG_NDEBUG 0
#define LOG_TAG "audio_hw"
#include <utils/plat_log.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <pthread.h>
#include <stdbool.h>
#include <memory.h>
#include <SystemBase.h>

#include <audio_hw.h>
#include <aec_lib.h> 
#if (MPPCFG_ANS == OPTION_ANS_ENABLE)
#include <ans_lib.h>
#endif

#include <ConfigOption.h>

#include "cdx_list.h" 

//#define SOUND_CARD  "default:CARD=audiocodec"

#define SOUND_CARD_AUDIOCODEC   "default"
#define SOUND_CARD_SNDHDMI      "hw:1,0"

#define SOUND_MIXER_AUDIOCODEC   "hw:0"
#define SOUND_MIXER_SNDDAUDIO0   "hw:1"

#define SOUND_CARD_SNDDAUDIo0   "hw:1,0"

//#define AI_HW_AEC_DEBUG_EN

typedef enum AI_STATES_E
{
    AI_STATE_INVALID = 0,
    AI_STATE_CONFIGURED,
    AI_STATE_STARTED,
} AI_STATES_E;

typedef enum AO_STATES_E
{
    AO_STATE_INVALID = 0,
    AO_STATE_CONFIGURED,
    AO_STATE_STARTED,
} AO_STATES_E;


typedef struct AudioInputDevice
{
    AI_STATES_E mState;

    AIO_ATTR_S mAttr;
    PCM_CONFIG_S mCfg;
    AUDIO_TRACK_MODE_E mTrackMode;
    pthread_t mThdId;
    pthread_t mThdLoopId;
    volatile BOOL mThdRunning;

    struct list_head mChnList;
    pthread_mutex_t mChnListLock; 
    pthread_mutex_t mApiCallLock;   // to protect the api call,when used in two thread asynchronously.


    short *tmpBuf;
    int tmpBufLen;

    void *aecmInst;
    short *near_buff;                       // buffer used as internal buffer to store near data for conjunction.
    unsigned int near_buff_len;             // the length of the near buffer, normally is 2 x chunkbytesize.
    unsigned int near_buff_data_remain_len; // the length of the valid data that stored in near buffer.
    short *ref_buff;                        // buffer used as internal buffer to store reference data for conjunction.
    unsigned int ref_buff_len;              // the length of the ref buffer, normally is 2 x chunkbytesize.
    unsigned int ref_buff_data_remain_len;  // the length of the valid data that stored in ref buffer.
    
    short *out_buff;                        // buffer used as internal buffer to store aec produced data for conjunction.                    
    unsigned int out_buff_len;              // the length of the out buffer, normally is 2 x chunkbytesize.
    unsigned int out_buff_data_remain_len;  // the length of the valid data that stored in out buffer.

    // flag used to indicate whether one valid frm has sent to next module or not when aec feature enabled. 
    int aec_first_frm_sent_to_next_module;  
    long long aec_first_frm_pts;            // used to store the first frm pts got from audio_hw.c
    int aec_valid_frm;                      // flag used to indicate one valid output frame is ready or not.
    int ai_sent_to_ao_frms;                 // used to count the number of frame sent to ao at beginning. 

    FILE *tmp_pcm_fp_in;
    int tmp_pcm_in_size;
    FILE *tmp_pcm_fp_ref;
    int tmp_pcm_ref_size;
    FILE *tmp_pcm_fp_out;
    int tmp_pcm_out_size;

    char *pCapBuf;
    char *pRefBuf;

    int ai_chl_for_aec;                     // used to select ai channel binded with ao,used in soft loopback solution
    
    int first_ref_frm_got_flag;
    int snd_card_id; 

    // for ans common process 

    short *tmpBuf_ans;
    int tmpBufLen_ans; 
    
    short *in_buff_ans;                       // buffer used as internal buffer to store near data for conjunction.
    unsigned int in_buff_len_ans;             // the length of the near buffer, normally is 2 x chunkbytesize.
    unsigned int in_buff_data_remain_len_ans; // the length of the valid data that stored in near buffer.
    
    short *out_buff_ans;                        // buffer used as internal buffer to store aec produced data for conjunction.                    
    unsigned int out_buff_len_ans;              // the length of the out buffer, normally is 2 x chunkbytesize.
    unsigned int out_buff_data_remain_len_ans;  // the length of the valid data that stored in out buffer.

    int ans_valid_frm;                      // flag used to indicate one valid output frame is ready or not.

    // for ans process
    void *ans_int;
    
    int  filter_state1[6];
    int  filter_state12[6];
    int  Synthesis_state1[6];
    int  Synthesis_state12[6];

    // for ans lstm library
    void **ans_int_lstm;
    void **ans_state_lstm;
    
} AudioInputDevice;

typedef struct AudioOutputDevice
{
    AO_STATES_E mState;

    AIO_ATTR_S mAttr;
    PCM_CONFIG_S mCfg;
    AUDIO_TRACK_MODE_E mTrackMode;
    pthread_t mThdId;
    volatile BOOL mThdRunning;

    struct list_head mChnList;
    pthread_mutex_t mChnListLock; 
    pthread_mutex_t mAOApiCallLock;   // to protect the api call,when used in two thread asynchronously.
    
    int snd_card_id;
} AudioOutputDevice;

typedef struct AudioHwDevice
{
    BOOL mEnableFlag;
    AIO_MIXER_S mMixer;
    AudioInputDevice mCap;
    AudioOutputDevice mPlay;
} AudioHwDevice;

static AudioHwDevice gAudioHwDev[AIO_DEV_MAX_NUM];

// 0-cap; 1-play
ERRORTYPE audioHw_Construct(void)
{
    int i;
    int err;
    //memset(&gAudioHwDev, 0, sizeof(AudioHwDevice)*AIO_DEV_MAX_NUM);
    for (i = 0; i < AIO_DEV_MAX_NUM; ++i) {
        AudioHwDevice *pDev = &gAudioHwDev[i];
        if (TRUE == pDev->mEnableFlag) {
            alogw("audio_hw has already been constructed!");
            return SUCCESS;
        }
        memset(pDev, 0, sizeof(AudioHwDevice));
        if(0 == i)
        {
            err = alsaOpenMixer(&pDev->mMixer, SOUND_MIXER_AUDIOCODEC);
            if (err != 0) {
                aloge("AIO device %d open mixer failed, err[%d]!", i, err);
            }
            pDev->mMixer.snd_card_id = 0;
            pDev->mCap.snd_card_id = 0;
            pDev->mCap.mCfg.snd_card_id = 0;
            pDev->mPlay.snd_card_id = 0;
            pDev->mPlay.mCfg.snd_card_id = 0;

        }
        else if(1 == i)
        {
            err = alsaOpenMixer(&pDev->mMixer, SOUND_MIXER_SNDDAUDIO0);
            if (err != 0) {
                aloge("AIO device %d open mixer failed!", i);
            }
            pDev->mMixer.snd_card_id = 1;
            pDev->mCap.snd_card_id = 1;
            pDev->mCap.mCfg.snd_card_id = 1;
            pDev->mPlay.snd_card_id = 1;
            pDev->mPlay.mCfg.snd_card_id = 1;
        } 
        
        pDev->mCap.mState = AI_STATE_INVALID;
        pDev->mPlay.mState = AO_STATE_INVALID;
        INIT_LIST_HEAD(&pDev->mCap.mChnList);
        INIT_LIST_HEAD(&pDev->mPlay.mChnList); 
        pthread_mutex_init(&pDev->mCap.mApiCallLock, NULL);
        pthread_mutex_init(&pDev->mPlay.mAOApiCallLock, NULL);
        pDev->mEnableFlag = TRUE;
    }
    return SUCCESS;
}

ERRORTYPE audioHw_Destruct(void)
{
    int i;
    for (i = 0; i < AIO_DEV_MAX_NUM; ++i) {
        AudioHwDevice *pDev = &gAudioHwDev[i];
        if (FALSE == pDev->mEnableFlag) {
            alogw("audio_hw has already been destructed!");
            return SUCCESS;
        }
        if (pDev->mMixer.handle != NULL) {
            if (AI_STATE_STARTED==pDev->mCap.mState || AO_STATE_STARTED==pDev->mPlay.mState)
                aloge("Why AIO still running? CapState:%d, PlayState:%d", pDev->mCap.mState, pDev->mPlay.mState);
            alsaCloseMixer(&pDev->mMixer);
        }
        
        pthread_mutex_destroy(&pDev->mCap.mApiCallLock);
        pthread_mutex_destroy(&pDev->mPlay.mAOApiCallLock);
        pDev->mCap.mState = AI_STATE_INVALID;
        pDev->mPlay.mState = AO_STATE_INVALID;
        pDev->mEnableFlag = FALSE;
    }
    return SUCCESS;
}


/**************************************AI_DEV*****************************************/
ERRORTYPE audioHw_AI_Dev_lock(AUDIO_DEV AudioDevId)
{
    return pthread_mutex_lock(&gAudioHwDev[AudioDevId].mCap.mChnListLock);
}

ERRORTYPE audioHw_AI_Dev_unlock(AUDIO_DEV AudioDevId)
{
    return pthread_mutex_unlock(&gAudioHwDev[AudioDevId].mCap.mChnListLock);
}

ERRORTYPE audioHw_AI_searchChannel_l(AUDIO_DEV AudioDevId, AI_CHN AiChn, AI_CHANNEL_S** pChn)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    ERRORTYPE ret = FAILURE;
    AI_CHANNEL_S *pEntry;
    list_for_each_entry(pEntry, &pCap->mChnList, mList)
    {
        if(pEntry->mId == AiChn) {
            if(pChn) {
                *pChn = pEntry;
            }
            ret = SUCCESS;
            break;
        }
    }
    return ret;
}

ERRORTYPE audioHw_AI_searchChannel(AUDIO_DEV AudioDevId, AI_CHN AiChn, AI_CHANNEL_S** pChn)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    ERRORTYPE ret = FAILURE;
    AI_CHANNEL_S *pEntry;
    pthread_mutex_lock(&pCap->mChnListLock);
    ret = audioHw_AI_searchChannel_l(AudioDevId, AiChn, pChn);
    pthread_mutex_unlock(&pCap->mChnListLock);
    return ret;
}

ERRORTYPE audioHw_AI_AddChannel_l(AUDIO_DEV AudioDevId, AI_CHANNEL_S* pChn)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    list_add_tail(&pChn->mList, &pCap->mChnList);
    struct list_head* pTmp;
    int cnt = 0;
    list_for_each(pTmp, &pCap->mChnList)
        cnt++;
    updateDebugfsByChnCnt(0, cnt);
    return SUCCESS;
}

ERRORTYPE audioHw_AI_AddChannel(AUDIO_DEV AudioDevId, AI_CHANNEL_S* pChn)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    pthread_mutex_lock(&pCap->mChnListLock);
    ERRORTYPE ret = audioHw_AI_AddChannel_l(AudioDevId, pChn);
    pthread_mutex_unlock(&pCap->mChnListLock);
    return SUCCESS;
}

ERRORTYPE audioHw_AI_RemoveChannel(AUDIO_DEV AudioDevId, AI_CHANNEL_S* pChn)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    pthread_mutex_lock(&pCap->mChnListLock);
    list_del(&pChn->mList);
    struct list_head* pTmp;
    int cnt = 0;
    list_for_each(pTmp, &pCap->mChnList)
        cnt++;
    updateDebugfsByChnCnt(0, cnt);
    pthread_mutex_unlock(&pCap->mChnListLock);
    return SUCCESS;
}

MM_COMPONENTTYPE *audioHw_AI_GetChnComp(PARAM_IN MPP_CHN_S *pMppChn)
{
    AI_CHANNEL_S *pChn = NULL;
    if (SUCCESS != audioHw_AI_searchChannel(pMppChn->mDevId, pMppChn->mChnId, &pChn)) {
        return NULL;
    }
    return pChn->mpComp;
}

BOOL audioHw_AI_IsDevStarted(AUDIO_DEV AudioDevId)
{
    return (gAudioHwDev[AudioDevId].mCap.mState == AI_STATE_STARTED);
}

#if (MPPCFG_ANS == OPTION_ANS_ENABLE) 
static ERRORTYPE audioHw_AI_AnsProcess(void *pThreadData, AUDIO_FRAME_S *pFrm)
{
    AudioInputDevice *pCap = (AudioInputDevice*)pThreadData;
    int ret = 0; 

    if(1 != pCap->mCfg.chnCnt)    
    {
        aloge("ans_invalid_chl_number:%d",pCap->mCfg.chnCnt);
        return FAILURE;
    } 

#if (MPPCFG_ANS_LIB == OPTION_ANS_LIBRARY_WEBRTC)
    if(pCap->mAttr.ai_ans_en && NULL == pCap->ans_int)
    {
        aloge("aec_ans_to_init:%d",pCap->mCfg.sampleRate);
        ret = WebRtcNs_Create(&pCap->ans_int);  // instance
        if(NULL == pCap->ans_int || 0 != ret)
        {
            aloge("aec_ans_instance_create_fail:%d",ret);
            return FAILURE;
        }

        ret = WebRtcNs_Init(pCap->ans_int,pCap->mCfg.sampleRate);
        if(0 != ret)
        {
            aloge("aec_ans_init_failed:%d",ret);
        }

        ret = WebRtcNs_set_policy(pCap->ans_int,pCap->mAttr.ai_ans_mode);
        if(0 != ret)
        {
            aloge("aec_ans_cfg_failed");
        }
        
        memset(pCap->filter_state1,0,sizeof(pCap->filter_state1));
        memset(pCap->filter_state12,0,sizeof(pCap->filter_state12));
        memset(pCap->Synthesis_state1,0,sizeof(pCap->Synthesis_state1));
        memset(pCap->Synthesis_state12,0,sizeof(pCap->Synthesis_state12));
    }
#elif(MPPCFG_ANS_LIB == OPTION_ANS_LIBRARY_LSTM)
    if(pCap->mAttr.ai_ans_en && NULL == pCap->ans_int_lstm)
    {
        aloge("aec_ans_lstm_to_init:%d",pCap->mCfg.sampleRate);
        ret = Rnn_Process_Create(&pCap->ans_int_lstm,&pCap->ans_state_lstm);
        if(NULL == pCap->ans_int_lstm || 0 != ret)
        {
            aloge("aec_ans_lstm_instance_create_fail:%d",ret);
            return FAILURE;
        }
        ret = Rnn_Process_init(pCap->ans_int_lstm,pCap->ans_state_lstm,pCap->mCfg.sampleRate);
        if(0 != ret)
        {
            aloge("aec_ans_lstm_init_failed:%d",ret);
        } 
    } 
#endif

    
    memset(pCap->tmpBuf_ans, 0, pCap->tmpBufLen_ans); 

    // move data in near buffer and reference buffer to internal buffer for conjunction with remaining data for last process. 
    if(pCap->in_buff_data_remain_len_ans+pFrm->mLen <= pCap->mCfg.chunkBytes*2)
    {
        memcpy((char*)pCap->in_buff_ans+pCap->in_buff_data_remain_len_ans,(char*)pFrm->mpAddr,pFrm->mLen);
        pCap->in_buff_data_remain_len_ans += pFrm->mLen;
            
    }
    else
    {
        aloge("fatal_err_in_buff_over_flow:%d-%d-%d-%d",pCap->in_buff_data_remain_len_ans,pCap->in_buff_len_ans,
                                                            pFrm->mLen,pCap->mCfg.chunkBytes);
    }

#if (MPPCFG_ANS_LIB == OPTION_ANS_LIBRARY_WEBRTC)    
    int frm_size = 320;         // 160 samples as one unit processed by aec library     
    short tmp_near_buffer[320];
    
    short tmp_ans_shInL[160] = {0};
    short tmp_ans_shInH[160] = {0};
    short tmp_ans_shOutL[160] = {0};
    short tmp_ans_shOutH[160] = {0}; 
#elif (MPPCFG_ANS_LIB == OPTION_ANS_LIBRARY_LSTM)
    int frm_size = 160;         // 160 samples as one unit processed by aec library     
    short tmp_near_buffer[160];

#endif 
    short *near_frm_ptr = (short *)pCap->in_buff_ans;
    short *processed_frm_ptr = (short *)pCap->tmpBuf_ans; 

    int left = pCap->in_buff_data_remain_len_ans / sizeof(short); 

    // start to process
    while(left >= frm_size)
    {
        memcpy((char *)tmp_near_buffer,(char *)near_frm_ptr,frm_size*sizeof(short));
        
#if (MPPCFG_ANS_LIB == OPTION_ANS_LIBRARY_WEBRTC)
        WebRtcSpl_AnalysisQMF(tmp_near_buffer,tmp_ans_shInL,tmp_ans_shInH,pCap->filter_state1,pCap->filter_state12);
        if (0 == WebRtcNs_Process(pCap->ans_int ,tmp_ans_shInL,tmp_ans_shInH ,tmp_ans_shOutL , tmp_ans_shOutH))
        {
            WebRtcSpl_SynthesisQMF(tmp_ans_shOutL,tmp_ans_shOutH,processed_frm_ptr,pCap->Synthesis_state1,pCap->Synthesis_state12);
        }
#elif (MPPCFG_ANS_LIB == OPTION_ANS_LIBRARY_LSTM)
        Lstm_process_frame(pCap->ans_int_lstm, pCap->ans_state_lstm[0], processed_frm_ptr, tmp_near_buffer);
#endif 

        near_frm_ptr += frm_size;
        processed_frm_ptr += frm_size;
        left -= frm_size; 

        pCap->in_buff_data_remain_len_ans -= frm_size*sizeof(short);
    } 

    // move remaining data in internal buffer to the beginning of the buffer 
    if(left > 0)
    { 
        unsigned int near_buff_offset = (unsigned int)near_frm_ptr-(unsigned int)pCap->in_buff_ans;
        if( near_buff_offset < pCap->in_buff_data_remain_len_ans)
        {
            aloge("ans_fatal_err_buff_left:%d-%d-%d-%d",near_buff_offset,pCap->in_buff_data_remain_len_ans);
        }

        memcpy((char*)pCap->in_buff_ans,(char*)near_frm_ptr,pCap->in_buff_data_remain_len_ans);
    }

    unsigned int out_offset = (unsigned int)processed_frm_ptr - (unsigned int)pCap->tmpBuf_ans; 

    if(out_offset + pCap->out_buff_data_remain_len_ans > pCap->out_buff_len_ans)
    {
        aloge("ans_fatal_err_out_buff_over_flow:%d-%d-%d",out_offset, pCap->out_buff_data_remain_len_ans, pCap->out_buff_len_ans);
    }
    else
    {
        memcpy((char *)pCap->out_buff_ans+pCap->out_buff_data_remain_len_ans,(char *)pCap->tmpBuf_ans,out_offset); 
        pCap->out_buff_data_remain_len_ans += out_offset;
    } 

    // fetch one valid output frame from output internal buffer, the length of valid frame must equal to chunsize.
    if(pCap->out_buff_data_remain_len_ans >= pCap->mCfg.chunkSize*sizeof(short))
    {
        memcpy((char *)pFrm->mpAddr, (char *)pCap->out_buff_ans, pCap->mCfg.chunkSize*sizeof(short));
        pCap->out_buff_data_remain_len_ans -= pCap->mCfg.chunkSize*sizeof(short);

        if(pFrm->mLen != pCap->mCfg.chunkSize*sizeof(short))
        {
            aloge("ans_fatal_error:%d-%d",pCap->mCfg.chunkSize*sizeof(short),pFrm->mLen);
        }

        
        pCap->ans_valid_frm = 1;
        if(pCap->out_buff_data_remain_len_ans > pCap->mCfg.chunkSize*sizeof(short))
        {
            aloge("ans_fatal_err_out_buff_data_mov:%d-%d",pCap->mCfg.chunkSize*sizeof(short),pCap->out_buff_data_remain_len_ans);
        }
        else
        {
            memcpy((char *)pCap->out_buff_ans,((char *)pCap->out_buff_ans+pCap->mCfg.chunkSize*sizeof(short)), 
                                                                                pCap->out_buff_data_remain_len_ans);
        }
    }
    else
    {
        pCap->ans_valid_frm = 0;
    } 

    return SUCCESS;    
}
#endif 


#if (MPPCFG_AEC == OPTION_AEC_ENABLE)
#if AEC_SOFT_LOOPBACK_SOLUTION
static void audioHw_AI_ChlForAecSelect(void *pThreadData, int *ai_chl_aec)
{
    AudioInputDevice *pCap = (AudioInputDevice*)pThreadData;
    AI_CHANNEL_S *pEntry = NULL; 
    BOOL tunnel_flag = 0;
    
    list_for_each_entry(pEntry, &pCap->mChnList, mList)
    {
        pEntry->mpComp->GetConfig(pEntry->mpComp,COMP_IndexVendorAIChnInportTunneled, &tunnel_flag);
        if(TRUE == tunnel_flag)
        {
            *ai_chl_aec = pEntry->mId;
        }
    } 
}

static int audioHw_AI_ChlForAecFetchFrmRef(void *pThreadData, int ai_chl_for_aec,AUDIO_FRAME_S *PstFrmRef)
{
    AudioInputDevice *pCap = (AudioInputDevice*)pThreadData;
    AI_CHANNEL_S *pEntry = NULL; 
    int ret = 0;
    
    list_for_each_entry(pEntry, &pCap->mChnList, mList)
    {
        if(ai_chl_for_aec == pEntry->mId)
        { 
            ret = pEntry->mpComp->GetConfig(pEntry->mpComp,COMP_IndexVendorAIChnGetValidFrameRef, PstFrmRef);
            if(SUCCESS != ret)
            {
//                alogd("aec_get_ref_frm_fail:%d",ret);
            }
        }
        return ret;
    }
}
#endif 

static ERRORTYPE audioHw_AI_AecProcess(void *pThreadData, AUDIO_FRAME_S *pFrm, AUDIO_FRAME_S *pRefFrm)
{
    AudioInputDevice *pCap = (AudioInputDevice*)pThreadData;
    int ret = 0; 

    if(1 != pCap->mCfg.chnCnt)    
    {
        aloge("aec_invalid_chl_number:%d",pCap->mCfg.chnCnt);
        return FAILURE;
    } 

    if(NULL == pCap->aecmInst)
    { 
        aloge("aec_to_init:%d-%d",pCap->mCfg.sampleRate,pCap->mCfg.aec_delay_ms);
       ret = WebRtcAec_Create(&pCap->aecmInst);
        if(NULL == pCap->aecmInst || 0 != ret)
        {
            aloge("aec_instance_create_fail:%d",ret);
            return FAILURE;
        }
        ret = WebRtcAec_Init(pCap->aecmInst, pCap->mCfg.sampleRate, pCap->mCfg.sampleRate);
        if(0 != ret)
        {
            aloge("aec_init_failed:%d",ret);
        }
        
        AecConfig config;
        
        memset(&config,0,sizeof(AecConfig));
        config.nlpMode = kAecNlpConservative;   
        ret = WebRtcAec_set_config(pCap->aecmInst, config);
        if(0 != ret)
        {
            aloge("aec_cfg_failed:%d",ret);
        }
        
    } 
    
    memset(pCap->tmpBuf, 0, pCap->tmpBufLen); 

    // move data in near buffer and reference buffer to internal buffer for conjunction with remaining data for last process. 
    if(pCap->near_buff_data_remain_len+pFrm->mLen <= pCap->mCfg.chunkBytes*2)
    {
        memcpy((char*)pCap->near_buff+pCap->near_buff_data_remain_len,(char*)pFrm->mpAddr,pFrm->mLen);
        pCap->near_buff_data_remain_len += pFrm->mLen;
            
    }
    else
    {
        aloge("fatal_err_near_buff_over_flow:%d-%d-%d-%d",pCap->near_buff_data_remain_len,pCap->near_buff_len,
                                                            pFrm->mLen,pCap->mCfg.chunkBytes);
    }
    
    if(pCap->ref_buff_data_remain_len+pRefFrm->mLen <= pCap->mCfg.chunkBytes*2)
    {
        memcpy((char*)pCap->ref_buff+pCap->ref_buff_data_remain_len,(char*)pRefFrm->mpAddr, pRefFrm->mLen);
        pCap->ref_buff_data_remain_len += pRefFrm->mLen;
    }
    else
    {
        aloge("fatal_err_ref_buff_over_flow:%d-%d-%d-%d",pCap->ref_buff_data_remain_len,pCap->ref_buff_len,
                                                                pRefFrm->mLen,pCap->mCfg.chunkBytes);
    }

    int frm_size = 160;         // 160 samples as one unit processed by aec library     
    short tmp_near_buffer[160];
    short tmp_far_buffer[160]; 

    short *near_frm_ptr = (short *)pCap->near_buff;
    short *ref_frm_ptr = (short *)pCap->ref_buff;
    short *processed_frm_ptr = (short *)pCap->tmpBuf; 

    int size = (pCap->near_buff_data_remain_len < pCap->ref_buff_data_remain_len)? pCap->near_buff_data_remain_len: pCap->ref_buff_data_remain_len;
    int left = size / sizeof(short); 

    // start to process
    while(left >= frm_size)
    {
        memcpy((char *)tmp_near_buffer,(char *)near_frm_ptr,frm_size*sizeof(short));
        memcpy((char*)tmp_far_buffer,(char*)ref_frm_ptr,frm_size*sizeof(short));
        
        ret = WebRtcAec_BufferFarend(pCap->aecmInst, tmp_far_buffer, frm_size);
        if(0 != ret)
        {
            aloge("aec_insert_far_data_failed:%d-%d",ret,((aecpc_t*)pCap->aecmInst)->lastError);
        } 

        ret = WebRtcAec_Process(pCap->aecmInst, tmp_near_buffer, NULL, processed_frm_ptr, NULL, frm_size,pCap->mCfg.aec_delay_ms,0);
        if(0 != ret)
        {
            aloge("aec_process_failed:%d-%d",ret,((aecpc_t*)pCap->aecmInst)->lastError);
        } 

        #ifdef AI_HW_AEC_DEBUG_EN
        if(NULL != pCap->tmp_pcm_fp_in)
        {
            fwrite(tmp_near_buffer, 1, frm_size*sizeof(short), pCap->tmp_pcm_fp_in);
            pCap->tmp_pcm_in_size += frm_size*sizeof(short);
        }
        
        if(NULL != pCap->tmp_pcm_fp_ref)
        {
            fwrite(tmp_far_buffer, 1, frm_size*sizeof(short), pCap->tmp_pcm_fp_ref);
            pCap->tmp_pcm_ref_size += frm_size*sizeof(short);
        }

//        aloge("zjx_tbin:%d-%d",pCap->tmp_pcm_in_size,pCap->tmp_pcm_ref_size); 
        #endif

        near_frm_ptr += frm_size;
        ref_frm_ptr += frm_size;
        processed_frm_ptr += frm_size;
        left -= frm_size; 

        pCap->near_buff_data_remain_len -= frm_size*sizeof(short);
        pCap->ref_buff_data_remain_len -= frm_size*sizeof(short);
    } 

    // move remaining data in internal buffer to the beginning of the buffer 
    if(left > 0)
    { 
        unsigned int near_buff_offset = (unsigned int)near_frm_ptr-(unsigned int)pCap->near_buff;
        unsigned int far_buff_offset = (unsigned int)ref_frm_ptr-(unsigned int)pCap->ref_buff;
        if( near_buff_offset < pCap->near_buff_data_remain_len || 
                       far_buff_offset < pCap->ref_buff_data_remain_len)
        {
            aloge("aec_fatal_err_buff_left:%d-%d-%d-%d",near_buff_offset,pCap->near_buff_data_remain_len,
                                                            far_buff_offset, pCap->ref_buff_data_remain_len);
        }

        memcpy((char*)pCap->near_buff,(char*)near_frm_ptr,pCap->near_buff_data_remain_len);
        memcpy((char*)pCap->ref_buff,(char*)ref_frm_ptr,pCap->ref_buff_data_remain_len); 
    }

    unsigned int out_offset = (unsigned int)processed_frm_ptr - (unsigned int)pCap->tmpBuf; 

    // move the out data produced by aec library to the internal buffer for conjunction with remaining data left for last process
    if(out_offset + pCap->out_buff_data_remain_len > pCap->out_buff_len)
    {
        aloge("aec_fatal_err_out_buff_over_flow:%d-%d-%d",out_offset, pCap->out_buff_data_remain_len, pCap->out_buff_len);
    }
    else
    {
        memcpy((char *)pCap->out_buff+pCap->out_buff_data_remain_len,(char *)pCap->tmpBuf,out_offset); 
        pCap->out_buff_data_remain_len += out_offset;
    } 

    // fetch one valid output frame from output internal buffer, the length of valid frame must equal to chunsize.
    if(pCap->out_buff_data_remain_len >= pCap->mCfg.chunkSize*sizeof(short))
    {
        memcpy((char *)pFrm->mpAddr, (char *)pCap->out_buff, pCap->mCfg.chunkSize*sizeof(short));
        pCap->out_buff_data_remain_len -= pCap->mCfg.chunkSize*sizeof(short);

        if(pFrm->mLen != pCap->mCfg.chunkSize*sizeof(short))
        {
            aloge("aec_fatal_error:%d-%d",pCap->mCfg.chunkSize*sizeof(short),pFrm->mLen);
        }

        #ifdef AI_HW_AEC_DEBUG_EN
        if(NULL != pCap->tmp_pcm_fp_out)
        {
            fwrite(pFrm->mpAddr, 1, pFrm->mLen, pCap->tmp_pcm_fp_out);
            pCap->tmp_pcm_out_size += pFrm->mLen; 
        }
//        aloge("zjx_tbo:%d-%d-%d",pCap->tmp_pcm_out_size,pFrm->mLen,pCap->mCfg.chunkSize*sizeof(short));
        #endif
        
        pCap->aec_valid_frm = 1;
        if(pCap->out_buff_data_remain_len > pCap->mCfg.chunkSize*sizeof(short))
        {
            aloge("aec_fatal_err_out_buff_data_mov:%d-%d",pCap->mCfg.chunkSize*sizeof(short),pCap->out_buff_data_remain_len);
        }
        else
        {
            memcpy((char *)pCap->out_buff,((char *)pCap->out_buff+pCap->mCfg.chunkSize*sizeof(short)), pCap->out_buff_data_remain_len);
        }
    }
    else
    {
        pCap->aec_valid_frm = 0;
    } 

    return SUCCESS;    
}

#if AEC_SOFT_LOOPBACK_SOLUTION
static void audioHw_AI_Aec(void *pThreadData)
{ 
    AudioInputDevice *pCap = (AudioInputDevice*)pThreadData; 
    AUDIO_FRAME_S stFrmRef; 
    AUDIO_FRAME_S stFrmCap; 
    int ret = 0;
    
    memset(&stFrmRef,0,sizeof(AUDIO_FRAME_S));
    memset(&stFrmCap,0,sizeof(AUDIO_FRAME_S));
    
    // first ,chose ai chl for referenc frame fetching
    if(-1 == pCap->ai_chl_for_aec) 
    {
        audioHw_AI_ChlForAecSelect(pCap,&pCap->ai_chl_for_aec);
    } 
    
    // second,fetch refrence frame
    if(-1 != pCap->ai_chl_for_aec)
    {
        memset(pCap->pRefBuf,0,pCap->mCfg.chunkBytes);
        stFrmRef.mLen = pCap->mCfg.chunkBytes;
        stFrmRef.mpAddr = pCap->pRefBuf; 
        ret = audioHw_AI_ChlForAecFetchFrmRef(pCap,pCap->ai_chl_for_aec,&stFrmRef);
        while(SUCCESS != ret)
        {
            usleep(2*1000);
            ret = audioHw_AI_ChlForAecFetchFrmRef(pCap,pCap->ai_chl_for_aec,&stFrmRef);
        }
    }
    
    // third, to aec process
    if(-1 != pCap->ai_chl_for_aec)
    {
        stFrmCap.mLen = pCap->mCfg.chunkBytes;
        stFrmCap.mpAddr = pCap->pCapBuf; 
        audioHw_AI_AecProcess(pThreadData,&stFrmCap,&stFrmRef); 
    } 
}
#endif

#endif

#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
static void *audioHw_AI_CapLoopThread(void *pThreadData)
{
    
    AudioInputDevice *pCap = (AudioInputDevice*)pThreadData; 
    AudioInputDevice *pCap_daudio0 = NULL; 
    
    pCap_daudio0 = &gAudioHwDev[pCap->snd_card_id+1].mCap;

    while(!pCap->first_ref_frm_got_flag)
    {
        
        if (alsaReadPcm(&pCap_daudio0->mCfg, pCap->pRefBuf, pCap_daudio0->mCfg.chunkSize) != pCap_daudio0->mCfg.chunkSize) {
            aloge("fatal error! daudio0_fail to read pcm %d bytes-%d", pCap_daudio0->mCfg.chunkBytes,pCap_daudio0->mState);
            usleep(2*1000);
            continue;
        }

        pCap->first_ref_frm_got_flag = 1;
    }

    return NULL;
}
#endif 

static void *audioHw_AI_CapThread(void *pThreadData)
{
    AudioInputDevice *pCap = (AudioInputDevice*)pThreadData; 
    AudioInputDevice *pCap_daudio0 = NULL; 
    
    int cap_ref_first_frm_flag = 1;
    char *pCapBuf = NULL; 
    char *pCapBufLoopBack = NULL; 
    ssize_t ret;
    pCap->pCapBuf = (char*)malloc(pCap->mCfg.chunkBytes);
    if (NULL == pCap->pCapBuf) {
        aloge("Failed to alloc %d bytes(%s)", pCap->mCfg.chunkBytes, strerror(errno));
    }
    pCapBuf = pCap->pCapBuf; 

#if (MPPCFG_AEC == OPTION_AEC_ENABLE)
    if(pCap->mAttr.ai_aec_en)
    {
        pCap->pRefBuf = (char *)malloc(pCap->mCfg.chunkBytes);
        if(NULL == pCap->pRefBuf)
        {
            aloge("fatal_error_to_malloc_ref_frm_buff:%d",pCap->mCfg.chunkBytes);
        } 
#if !AEC_SOFT_LOOPBACK_SOLUTION
        pCapBufLoopBack = pCap->pRefBuf; 
        pCap_daudio0 = &gAudioHwDev[pCap->snd_card_id+1].mCap; 
#endif
    } 
    pCap->ai_chl_for_aec = -1;
#endif

    while (pCap->mThdRunning) 
    { 
    
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
        if(pCap->mAttr.ai_aec_en)
        {
            if(!pCap->first_ref_frm_got_flag)   // just used to start new thread to trigger capture hardware for second snd card
            {
                aloge("aec_to_start_second_cap:%lld",CDX_GetTimeUs());
                pthread_attr_t attr; 
                pthread_attr_init(&attr);
                pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
                pthread_create(&pCap->mThdLoopId, &attr, audioHw_AI_CapLoopThread, pCap);
                pthread_attr_destroy(&attr);
            }
        }
#endif
        ret = alsaReadPcm(&pCap->mCfg, pCapBuf, pCap->mCfg.chunkSize);
        if (ret != pCap->mCfg.chunkSize) {
            if(-EPIPE == ret)
            {
                aloge("aec_xrun_happended:%lld",CDX_GetTimeUs());
                pCap->first_ref_frm_got_flag = 0;
                cap_ref_first_frm_flag = 1;
                usleep(10*1000);    // to wait the sencod capture xrun
            }
            aloge("aec_fatal error! fail to read pcm %d bytes-%d-%x", pCap->mCfg.chunkBytes,pCap->mState,ret);
            usleep(10*1000);
            continue;
        }
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
        if(pCap->mAttr.ai_aec_en)
        {
            while(!pCap->first_ref_frm_got_flag) // to wait for first ref frm ready
            {
                usleep(2*1000);
            }
        }
        
        if(pCap->mAttr.ai_aec_en &!cap_ref_first_frm_flag)
        {
            ret = alsaReadPcm(&pCap_daudio0->mCfg, pCapBufLoopBack, pCap_daudio0->mCfg.chunkSize);
            if (ret != pCap_daudio0->mCfg.chunkSize) {
                if(-EPIPE == ret)
                {
                    aloge("aec_daudio0_xrun_happended:%lld",CDX_GetTimeUs());
                    pCap->first_ref_frm_got_flag = 0;
                    cap_ref_first_frm_flag = 1;
                    usleep(10*1000);    // to wait the first capture xrun
                }
                aloge("aec_fatal error! daudio0_fail to read pcm %d bytes-%d-%x", pCap_daudio0->mCfg.chunkBytes,pCap_daudio0->mState,ret);
                usleep(2*1000);
                continue;
            }
        } 
        cap_ref_first_frm_flag = 0;
#endif

        BOOL new_a_frm = TRUE;
        AI_CHANNEL_S *pEntry = NULL;
        pthread_mutex_lock(&pCap->mChnListLock);
        if (list_empty(&pCap->mChnList))
        {
            pthread_mutex_unlock(&pCap->mChnListLock);
            continue;
        } 

#if (MPPCFG_AEC == OPTION_AEC_ENABLE)
#if AEC_SOFT_LOOPBACK_SOLUTION    
        if(pCap->mAttr.ai_aec_en)
        { 
            audioHw_AI_Aec(pCap);
            if(0 == pCap->aec_valid_frm)
            {
                pthread_mutex_unlock(&pCap->mChnListLock);
                continue;
            }
        } 
 #else
         if(pCap->mAttr.ai_aec_en)
        { 
            AUDIO_FRAME_S stFrmRef; 
            AUDIO_FRAME_S stFrmCap; 
            
            memset(&stFrmRef,0,sizeof(AUDIO_FRAME_S));
            memset(&stFrmCap,0,sizeof(AUDIO_FRAME_S)); 
        
            stFrmRef.mLen = pCap->mCfg.chunkBytes;
            stFrmRef.mpAddr = pCap->pRefBuf; 
            
            stFrmCap.mLen = pCap->mCfg.chunkBytes;
            stFrmCap.mpAddr = pCap->pCapBuf; 

            audioHw_AI_AecProcess(pCap,&stFrmCap,&stFrmRef);
            
            if(0 == pCap->aec_valid_frm)
            {
                pthread_mutex_unlock(&pCap->mChnListLock);
                continue;
            }
        }
 #endif
#endif

#if (MPPCFG_ANS == OPTION_ANS_ENABLE)
        if(pCap->mAttr.ai_ans_en)
        {
            AUDIO_FRAME_S stFrmCap; 
            
            memset(&stFrmCap,0,sizeof(AUDIO_FRAME_S)); 
            stFrmCap.mLen = pCap->mCfg.chunkBytes;
            stFrmCap.mpAddr = pCap->pCapBuf; 
            audioHw_AI_AnsProcess(pCap,&stFrmCap);
            
            if(0 == pCap->ans_valid_frm)
            {
                pthread_mutex_unlock(&pCap->mChnListLock);
                continue;
            }
        }
#endif

        list_for_each_entry(pEntry, &pCap->mChnList, mList)
        {
            AUDIO_FRAME_S frame;
            COMP_BUFFERHEADERTYPE bufferHeader;
            bufferHeader.nOutputPortIndex = AI_CHN_PORT_INDEX_CAP_IN;
            bufferHeader.pOutputPortPrivate = &frame;
            frame.mLen = pCap->mCfg.chunkBytes;
            frame.mBitwidth = pCap->mAttr.enBitwidth;
            frame.mSoundmode = pCap->mAttr.enSoundmode;
            frame.mpAddr = pCapBuf;

            if(TRUE == new_a_frm)
            {
                int64_t tm1 = CDX_GetSysTimeUsMonotonic();
                frame.tmp_pts = tm1 - pCap->mCfg.chunkSize*1000/pCap->mCfg.sampleRate*1000;// Note: the pts timestamp will be used by all ai channels

                new_a_frm = FALSE;
            }
            pEntry->mpComp->EmptyThisBuffer(pEntry->mpComp, &bufferHeader);
        }
        pthread_mutex_unlock(&pCap->mChnListLock);
    }

    alogd("AI_CapThread exit!");
    if(NULL != pCap->pCapBuf)
    {
        free(pCap->pCapBuf);
        pCap->pCapBuf = NULL;
    }
#if (MPPCFG_AEC == OPTION_AEC_ENABLE)
    if(pCap->mAttr.ai_aec_en)
    {
        if(NULL != pCap->pRefBuf)
        {
            free(pCap->pRefBuf);
            pCap->pRefBuf = NULL;
        }
    }
 #endif   
    return NULL;
}

ERRORTYPE audioHw_AI_SetPubAttr(AUDIO_DEV AudioDevId, const AIO_ATTR_S *pstAttr)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    
    pthread_mutex_lock(&pCap->mApiCallLock);
    if (pstAttr == NULL) {
        aloge("pstAttr is NULL!");
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return ERR_AI_ILLEGAL_PARAM;
    }
    if (AI_STATE_INVALID != pCap->mState) {
        alogw("audioHw AI PublicAttr has been set!");
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return SUCCESS;
    }
    pCap->mAttr = *pstAttr;
    pCap->mState = AI_STATE_CONFIGURED;
    pthread_mutex_unlock(&pCap->mApiCallLock);
    return SUCCESS;
}

ERRORTYPE audioHw_AI_GetPubAttr(AUDIO_DEV AudioDevId, AIO_ATTR_S *pstAttr)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    
    pthread_mutex_lock(&pCap->mApiCallLock);
    if (pstAttr == NULL) {
        aloge("pstAttr is NULL!");
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return ERR_AI_ILLEGAL_PARAM;
    }

    if (pCap->mState == AI_STATE_INVALID) {
        aloge("get attr when attr is not set!");
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return ERR_AI_NOT_PERM;
    }

    *pstAttr = pCap->mAttr;
    pthread_mutex_unlock(&pCap->mApiCallLock);
    return SUCCESS;
}

ERRORTYPE audioHw_AI_ClrPubAttr(AUDIO_DEV AudioDevId)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    
    pthread_mutex_lock(&pCap->mApiCallLock);
    if (pCap->mState == AI_STATE_STARTED) {
        aloge("please clear attr after AI disable!");
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return ERR_AI_NOT_PERM;
    }
    memset(&pCap->mAttr, 0, sizeof(AIO_ATTR_S));
    pCap->mState = AI_STATE_INVALID;
    pthread_mutex_unlock(&pCap->mApiCallLock);
    return SUCCESS;
}

ERRORTYPE audioHw_AI_Enable(AUDIO_DEV AudioDevId)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer; 
    
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
    AudioInputDevice *pCap_daudio0 = &gAudioHwDev[AudioDevId+1].mCap;
    AIO_MIXER_S *pMixer_daudio0 = &gAudioHwDev[AudioDevId+1].mMixer;
#endif
    int ret;
    
    pthread_mutex_lock(&pCap->mApiCallLock);

    if (pCap->mState == AI_STATE_INVALID) {
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return ERR_AI_NOT_CONFIG;
    }
    if (pCap->mState == AI_STATE_STARTED) {
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return SUCCESS;
    }

    pCap->mCfg.chnCnt = pCap->mAttr.u32ChnCnt;
    pCap->mCfg.sampleRate = pCap->mAttr.enSamplerate;

    pCap->mCfg.aec_delay_ms = pCap->mAttr.aec_delay_ms;
    
    if (pCap->mAttr.enBitwidth == AUDIO_BIT_WIDTH_32) {
        pCap->mCfg.format = SND_PCM_FORMAT_S32_LE;
    } else if (pCap->mAttr.enBitwidth == AUDIO_BIT_WIDTH_24) {
        pCap->mCfg.format = SND_PCM_FORMAT_S24_LE;
    } else if (pCap->mAttr.enBitwidth == AUDIO_BIT_WIDTH_16) {
        pCap->mCfg.format = SND_PCM_FORMAT_S16_LE;
    } else if (pCap->mAttr.enBitwidth == AUDIO_BIT_WIDTH_8) {
        pCap->mCfg.format = SND_PCM_FORMAT_S8;
    } else {
        pCap->mCfg.format = SND_PCM_FORMAT_S16_LE;
    }
    pCap->mCfg.bitsPerSample = (pCap->mAttr.enBitwidth+1)*8;
    
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
    memcpy(&pCap_daudio0->mAttr,&pCap->mAttr,sizeof(AIO_ATTR_S)); 
    memcpy(&pCap_daudio0->mCfg,&pCap->mCfg,sizeof(PCM_CONFIG_S));
#endif

    ret = alsaOpenPcm(&pCap->mCfg, SOUND_CARD_AUDIOCODEC, 0);
    if (ret != 0) {
        aloge("%s,l:%d,open_pcm failed",__FUNCTION__,__LINE__);
        pthread_mutex_unlock(&pCap->mApiCallLock); 
        return FAILURE;
    }
    
    pCap->mCfg.read_pcm_aec = 0;

#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
    if(pCap->mAttr.ai_aec_en)
    {
        ret = alsaOpenPcm(&pCap_daudio0->mCfg, SOUND_CARD_SNDDAUDIo0, 0);   // to open snd card daudio0
        if (ret != 0) {
            aloge("%s,l:%d,open_pcm failed",__FUNCTION__,__LINE__);
            pthread_mutex_unlock(&pCap->mApiCallLock); 
            return FAILURE;
        }
        pCap->mCfg.read_pcm_aec = 1;
        pCap_daudio0->mCfg.read_pcm_aec = 1;
    } 
#endif 
    
    ret = alsaSetPcmParams(&pCap->mCfg);
    if (ret < 0) {
        aloge("%s,l:%d,SetPcmParams failed",__FUNCTION__,__LINE__);
        goto ERR_SET_PCM_PARAM;
    }

#if (MPPCFG_AEC == OPTION_AEC_ENABLE)
#if !AEC_SOFT_LOOPBACK_SOLUTION
    if(pCap->mAttr.ai_aec_en)
    {
        ret = alsaSetPcmParams(&pCap_daudio0->mCfg);
        if (ret < 0) {
            aloge("%s,l:%d,SetPcmParams failed",__FUNCTION__,__LINE__);
            goto ERR_SET_PCM_PARAM2;
        }
    }
#endif
    if(pCap->mAttr.ai_aec_en)
    {
        pCap->first_ref_frm_got_flag = 0;
        
        alsaMixerSetCapPlaySyncMode(pMixer,1); 
        
        pCap->near_buff_len = pCap->mCfg.chunkBytes*2;
        pCap->near_buff = (short *)malloc(pCap->near_buff_len);
        if(NULL == pCap->near_buff)
        {
            aloge("aec_fatal_error_near_buff_malloc_failed:%d",pCap->near_buff_len);
            goto ERR_SET_PCM_PARAM2;
        }
        pCap->near_buff_data_remain_len = 0;

        pCap->ref_buff_len = pCap->mCfg.chunkBytes*2;
        pCap->ref_buff = (short *)malloc(pCap->ref_buff_len);
        if(NULL == pCap->ref_buff)
        {
            aloge("aec_fatal_error_ref_buff_malloc_failed:%d",pCap->ref_buff_len);
            goto ERR_EXIT0;
        }
        pCap->ref_buff_data_remain_len = 0;

        pCap->out_buff_len = pCap->mCfg.chunkBytes*2;
        pCap->out_buff = (short *)malloc(pCap->out_buff_len);
        if(NULL == pCap->out_buff)
        {
            aloge("aec_fatal_error_out_buff_malloc_failed:%d",pCap->out_buff_len);
            goto ERR_EXIT1;
        }
        pCap->out_buff_data_remain_len = 0;

        pCap->aec_first_frm_pts = -1;
        pCap->aec_first_frm_sent_to_next_module = 0;
        pCap->aec_valid_frm = 0;
        pCap->ai_sent_to_ao_frms = 0;

        pCap->tmpBufLen = pCap->mCfg.chunkBytes*2;
        pCap->tmpBuf = (short *)malloc(pCap->tmpBufLen);
        if(pCap->tmpBuf == NULL)
        {
            aloge("aec_fatal_error_out_tmp_buff_malloc_failed:%d",pCap->tmpBufLen);
            goto ERR_EXIT2;
        }

#ifdef AI_HW_AEC_DEBUG_EN 
        pCap->tmp_pcm_fp_in = fopen("/mnt/extsd/tmp_in_ai_pcm", "wb"); 
        pCap->tmp_pcm_fp_ref = fopen("/mnt/extsd/tmp_ref_ai_pcm", "wb");
        pCap->tmp_pcm_fp_out = fopen("/mnt/extsd/tmp_out_ai_pcm", "wb");
        if(NULL==pCap->tmp_pcm_fp_in || NULL==pCap->tmp_pcm_fp_ref || NULL==pCap->tmp_pcm_fp_out)
        {
            aloge("aec_debug_file_create_failed");
        }
#endif
    }
    else
    {
        alsaMixerSetCapPlaySyncMode(pMixer,0); 
    }
#endif 

#if (MPPCFG_ANS == OPTION_ANS_ENABLE)
    if(pCap->mAttr.ai_ans_en)
    {
        pCap->in_buff_len_ans = pCap->mCfg.chunkBytes*2;
        pCap->in_buff_ans = (short *)malloc(pCap->in_buff_len_ans);
        if(NULL == pCap->in_buff_ans)
        {
            aloge("aec_fatal_error_near_buff_malloc_failed:%d",pCap->in_buff_len_ans);
            goto ERR_EXIT2;
        }
        pCap->in_buff_data_remain_len_ans = 0; 
        
        pCap->out_buff_len_ans = pCap->mCfg.chunkBytes*2;
        pCap->out_buff_ans = (short *)malloc(pCap->out_buff_len_ans);
        if(NULL == pCap->out_buff_ans)
        {
            aloge("ans_fatal_error_out_buff_malloc_failed:%d",pCap->out_buff_len_ans);
            goto ERR_EXIT3;
        }
        pCap->out_buff_data_remain_len_ans = 0;

        
        pCap->tmpBufLen_ans = pCap->mCfg.chunkBytes*2;
        pCap->tmpBuf_ans = (short *)malloc(pCap->tmpBufLen_ans);
        if(pCap->tmpBuf_ans == NULL)
        {
            aloge("ans_fatal_error_out_tmp_buff_malloc_failed:%d",pCap->tmpBufLen_ans);
            goto ERR_EXIT4;
        }
        pCap->ans_valid_frm = 0;
    }
#endif

    pthread_mutex_init(&pCap->mChnListLock, NULL);
    //INIT_LIST_HEAD(&pCap->mChnList);
    pCap->mThdRunning = TRUE;
    pthread_create(&pCap->mThdId, NULL, audioHw_AI_CapThread, pCap);

    pCap->mState = AI_STATE_STARTED;
    
    pthread_mutex_unlock(&pCap->mApiCallLock);

    return SUCCESS;

#if (MPPCFG_ANS == OPTION_ANS_ENABLE)
ERR_EXIT4: 
    if(NULL != pCap->out_buff_ans)
    {
        free(pCap->out_buff_ans);
        pCap->out_buff_ans = NULL;
    }

ERR_EXIT3:
    if(NULL != pCap->in_buff_ans)
    {
        free(pCap->in_buff_ans);
        pCap->in_buff_ans = NULL;
    }
#endif
    
ERR_EXIT2: 
#if (MPPCFG_AEC == OPTION_AEC_ENABLE) 
    if(NULL != pCap->out_buff)
    {
        free(pCap->out_buff);
    }
ERR_EXIT1: 
    if(NULL != pCap->ref_buff)
    {
        free(pCap->ref_buff);
    }
ERR_EXIT0:
    if(NULL != pCap->near_buff)
    {
        free(pCap->near_buff);
    } 
ERR_SET_PCM_PARAM2: 
    if(pCap->mAttr.ai_aec_en)
    {
        alsaMixerSetCapPlaySyncMode(pMixer,0); 
#if !AEC_SOFT_LOOPBACK_SOLUTION        
        alsaClosePcm(&pCap_daudio0->mCfg, 0);   // 0: cap
#endif        
    }
#endif
    
ERR_SET_PCM_PARAM:
    alsaClosePcm(&pCap->mCfg, 0);   // 0: cap
    pthread_mutex_unlock(&pCap->mApiCallLock);
    return FAILURE;
}

ERRORTYPE audioHw_AI_Disable(AUDIO_DEV AudioDevId)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer; 
    
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
    AudioInputDevice *pCap_daudio0 = &gAudioHwDev[AudioDevId+1].mCap;
    AIO_MIXER_S *pMixer_daudio0 = &gAudioHwDev[AudioDevId+1].mMixer;
#endif

    int ret;
    
    pthread_mutex_lock(&pCap->mApiCallLock);
    if (pCap->mState == AI_STATE_INVALID) {
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return ERR_AI_NOT_CONFIG;
    }
    if (pCap->mState != AI_STATE_STARTED) {
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return SUCCESS;
    }
    pthread_mutex_lock(&pCap->mChnListLock);
    if (!list_empty(&pCap->mChnList)) {
        pthread_mutex_unlock(&pCap->mChnListLock);
        pthread_mutex_unlock(&pCap->mApiCallLock);
        return SUCCESS;
    }
    pthread_mutex_unlock(&pCap->mChnListLock);

    pCap->mThdRunning = FALSE;

    pthread_join(pCap->mThdId, (void*) &ret);

    pthread_mutex_destroy(&pCap->mChnListLock);
    
    alsaClosePcm(&pCap->mCfg, 0);   // 0: cap
    pCap->mCfg.read_pcm_aec = 0;

#if (MPPCFG_AEC == OPTION_AEC_ENABLE)
    if(pCap->mAttr.ai_aec_en)
    {
        alsaMixerSetCapPlaySyncMode(pMixer,0);
#if  !AEC_SOFT_LOOPBACK_SOLUTION        
        alsaClosePcm(&pCap_daudio0->mCfg, 0);   // 0: cap
        pCap_daudio0->mCfg.read_pcm_aec = 0;
#endif        
    }
    if(pCap->mAttr.ai_aec_en)
    {
        pCap->first_ref_frm_got_flag = 0;
        
        if(NULL != pCap->aecmInst)
        {
            ret = WebRtcAec_Free(pCap->aecmInst);
            if(0 == ret)
            {
                aloge("aec_released");  
                pCap->aecmInst = NULL;
            }
            else
            {
                aloge("aec_free_failed");
            }
        }
           
        if(NULL != pCap->tmpBuf)
        {
            free(pCap->tmpBuf);
            pCap->tmpBuf = NULL;
        }
        if(NULL != pCap->out_buff)
        {
            free(pCap->out_buff);
            pCap->out_buff = NULL;
        }
        if(NULL != pCap->ref_buff)
        {
            free(pCap->ref_buff);
            pCap->ref_buff = NULL;
        }
        if(NULL != pCap->near_buff)
        {
            free(pCap->near_buff);
            pCap->near_buff = NULL;
        } 
        
#ifdef AI_HW_AEC_DEBUG_EN 
        if(NULL != pCap->tmp_pcm_fp_in)
        {
            close(pCap->tmp_pcm_fp_in);
            pCap->tmp_pcm_fp_in = NULL;
        }
        if(NULL != pCap->tmp_pcm_fp_ref)
        {
            close(pCap->tmp_pcm_fp_ref);
            pCap->tmp_pcm_fp_ref = NULL;
        }
        if(NULL != pCap->tmp_pcm_fp_out)
        {
            close(pCap->tmp_pcm_fp_out);
            pCap->tmp_pcm_fp_out = NULL;
        }
#endif
    }
#endif 

#if (MPPCFG_ANS == OPTION_ANS_ENABLE) 
    #if (MPPCFG_ANS_LIB == OPTION_ANS_LIBRARY_WEBRTC)
            if(pCap->mAttr.ai_ans_en && NULL != pCap->ans_int)
            {
                ret = WebRtcNs_Free(pCap->ans_int);
                if(0 == ret)
                {
                    aloge("aec_ans_released");
                    pCap->ans_int = NULL;
                }
                else
                {
                    aloge("aec_ans_release_failed");
                }
            }
    #elif (MPPCFG_ANS_LIB == OPTION_ANS_LIBRARY_LSTM)
            if(pCap->mAttr.ai_ans_en && NULL != pCap->ans_int_lstm && NULL != pCap->ans_state_lstm)
            {
                ret = Rnn_Process_free(pCap->ans_state_lstm,pCap->ans_int_lstm);
                if(0 == ret)
                {
                    aloge("aec_ans_lstm_released");
                    pCap->ans_int_lstm = NULL;
                    pCap->ans_state_lstm = NULL;
                }
                else
                {
                    aloge("aec_ans_lstm_release_failed");
                }
            }
    #endif

        if(NULL != pCap->tmpBuf_ans)
        {
            free(pCap->tmpBuf_ans);
            pCap->tmpBuf_ans = NULL;
        }
        if(NULL != pCap->out_buff_ans)
        {
            free(pCap->out_buff_ans);
            pCap->out_buff_ans = NULL;
        }
        if(NULL != pCap->in_buff_ans)
        {
            free(pCap->in_buff_ans);
            pCap->in_buff_ans = NULL;
        } 
#endif 

    pCap->mState = AI_STATE_CONFIGURED;
    pthread_mutex_unlock(&pCap->mApiCallLock);
    return SUCCESS;
}

ERRORTYPE audioHw_AI_SetTrackMode(AUDIO_DEV AudioDevId, AUDIO_TRACK_MODE_E enTrackMode)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;

    if (pCap->mState != AI_STATE_STARTED) {
        return ERR_AI_NOT_ENABLED;
    }

    pCap->mTrackMode = enTrackMode;

    return SUCCESS;
}

ERRORTYPE audioHw_AI_GetTrackMode(AUDIO_DEV AudioDevId, AUDIO_TRACK_MODE_E *penTrackMode)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;

    if (pCap->mState != AI_STATE_STARTED) {
        return ERR_AI_NOT_ENABLED;
    }

    *penTrackMode = pCap->mTrackMode;

    return SUCCESS;
}

ERRORTYPE audioHw_AI_GetPcmConfig(AUDIO_DEV AudioDevId, PCM_CONFIG_S **ppCfg)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;

    if (pCap->mState != AI_STATE_STARTED) {
        return ERR_AI_NOT_ENABLED;
    }
    *ppCfg = &pCap->mCfg;
    return SUCCESS;
}

ERRORTYPE audioHw_AI_GetAIOAttr(AUDIO_DEV AudioDevId, AIO_ATTR_S **ppAttr)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;

    if (pCap->mState != AI_STATE_STARTED) {
        return ERR_AI_NOT_ENABLED;
    }
    *ppAttr = &pCap->mAttr;
    return SUCCESS;
}

ERRORTYPE audioHw_AI_SetVolume(AUDIO_DEV AudioDevId, int s32VolumeDb)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    if (pCap->mState != AI_STATE_STARTED) {
        return ERR_AI_NOT_ENABLED;
    }

    return alsaMixerSetVolume(pMixer, 0, s32VolumeDb);
}

ERRORTYPE audioHw_AI_GetVolume(AUDIO_DEV AudioDevId, int *ps32VolumeDb)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    if (pCap->mState != AI_STATE_STARTED) {
        return ERR_AI_NOT_ENABLED;
    }

    return alsaMixerGetVolume(pMixer, 0, (long*)ps32VolumeDb);
}

ERRORTYPE audioHw_AI_SetMute(AUDIO_DEV AudioDevId, int bEnable)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    if (pCap->mState != AI_STATE_STARTED) {
        return ERR_AI_NOT_ENABLED;
    }

    return alsaMixerSetMute(pMixer, 0, bEnable);
}

ERRORTYPE audioHw_AI_GetMute(AUDIO_DEV AudioDevId, int *pbEnable)
{
    AudioInputDevice *pCap = &gAudioHwDev[AudioDevId].mCap;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    if (pCap->mState != AI_STATE_STARTED) {
        return ERR_AI_NOT_ENABLED;
    }

    return alsaMixerGetMute(pMixer, 0, pbEnable);
}


/**************************************AO_DEV*****************************************/
ERRORTYPE audioHw_AO_Dev_lock(AUDIO_DEV AudioDevId)
{
    return pthread_mutex_lock(&gAudioHwDev[AudioDevId].mPlay.mChnListLock);
}

ERRORTYPE audioHw_AO_Dev_unlock(AUDIO_DEV AudioDevId)
{
    return pthread_mutex_unlock(&gAudioHwDev[AudioDevId].mPlay.mChnListLock);
}

ERRORTYPE audioHw_AO_searchChannel_l(AUDIO_DEV AudioDevId, AO_CHN AoChn, AO_CHANNEL_S** pChn)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    ERRORTYPE ret = FAILURE;
    AO_CHANNEL_S *pEntry;
    list_for_each_entry(pEntry, &pPlay->mChnList, mList)
    {
        if(pEntry->mId == AoChn) {
            if(pChn) {
                *pChn = pEntry;
            }
            ret = SUCCESS;
            break;
        }
    }
    return ret;
}

ERRORTYPE audioHw_AO_searchChannel(AUDIO_DEV AudioDevId, AO_CHN AoChn, AO_CHANNEL_S** pChn)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    ERRORTYPE ret = FAILURE;
    AO_CHANNEL_S *pEntry;
    pthread_mutex_lock(&pPlay->mChnListLock);
    ret = audioHw_AO_searchChannel_l(AudioDevId, AoChn, pChn);
    pthread_mutex_unlock(&pPlay->mChnListLock);
    return ret;
}

ERRORTYPE audioHw_AO_AddChannel_l(AUDIO_DEV AudioDevId, AO_CHANNEL_S* pChn)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    list_add_tail(&pChn->mList, &pPlay->mChnList);
    struct list_head* pTmp;
    int cnt = 0;
    list_for_each(pTmp, &pPlay->mChnList)
        cnt++;
    updateDebugfsByChnCnt(1, cnt);
    return SUCCESS;
}

ERRORTYPE audioHw_AO_AddChannel(AUDIO_DEV AudioDevId, AO_CHANNEL_S* pChn)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    pthread_mutex_lock(&pPlay->mChnListLock);
    ERRORTYPE ret = audioHw_AO_AddChannel_l(AudioDevId, pChn);
    pthread_mutex_unlock(&pPlay->mChnListLock);
    return ret;
}

ERRORTYPE audioHw_AO_RemoveChannel(AUDIO_DEV AudioDevId, AO_CHANNEL_S* pChn)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    pthread_mutex_lock(&pPlay->mChnListLock);
    list_del(&pChn->mList);
    struct list_head* pTmp;
    int cnt = 0;
    list_for_each(pTmp, &pPlay->mChnList)
        cnt++;
    updateDebugfsByChnCnt(1, cnt);
    pthread_mutex_unlock(&pPlay->mChnListLock);
    return SUCCESS;
}

MM_COMPONENTTYPE *audioHw_AO_GetChnComp(PARAM_IN MPP_CHN_S *pMppChn)
{
    AO_CHANNEL_S *pChn = NULL;
    if (SUCCESS != audioHw_AO_searchChannel(pMppChn->mDevId, pMppChn->mChnId, &pChn)) {
        return NULL;
    }
    return pChn->mpComp;
}

BOOL audioHw_AO_IsDevConfigured(AUDIO_DEV AudioDevId)
{
    return (gAudioHwDev[AudioDevId].mPlay.mState == AO_STATE_CONFIGURED);
}

BOOL audioHw_AO_IsDevStarted(AUDIO_DEV AudioDevId)
{
    return (gAudioHwDev[AudioDevId].mPlay.mState == AO_STATE_STARTED);
}

ERRORTYPE AudioHw_AO_SetPubAttr(AUDIO_DEV AudioDevId, const AIO_ATTR_S *pstAttr)
{
    if (pstAttr == NULL) {
        aloge("pstAttr is NULL!");
        return ERR_AO_ILLEGAL_PARAM;
    }
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    if (pPlay->mState == AO_STATE_CONFIGURED) {
        alogw("Update AoAttr? cur_card:%d -> wanted_card:%d", pPlay->mAttr.mPcmCardId, pstAttr->mPcmCardId);
    } else if (pPlay->mState == AO_STATE_STARTED) {
        alogw("Careful for 2 AoChns at the same time! They must have the same param!");
        return SUCCESS;
    }
    pPlay->mAttr = *pstAttr;
    pPlay->mState = AO_STATE_CONFIGURED;
    return SUCCESS;
}

ERRORTYPE AudioHw_AO_GetPubAttr(AUDIO_DEV AudioDevId, AIO_ATTR_S *pstAttr)
{
    if (pstAttr == NULL) {
        aloge("pstAttr is NULL!");
        return ERR_AO_ILLEGAL_PARAM;
    }

    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    if (pPlay->mState == AO_STATE_INVALID) {
        aloge("get attr when attr is not set!");
        return ERR_AO_NOT_PERM;
    }

    *pstAttr = pPlay->mAttr;
    return SUCCESS;
}

ERRORTYPE audioHw_AO_ClrPubAttr(AUDIO_DEV AudioDevId)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;

    if (pPlay->mState == AO_STATE_STARTED) {
        aloge("please clear attr after AI disable!");
        return ERR_AO_NOT_PERM;
    }
    memset(&pPlay->mAttr, 0, sizeof(AIO_ATTR_S));
    pPlay->mState = AO_STATE_INVALID;
    return SUCCESS;
}

ERRORTYPE audioHw_AO_SetTrackMode(AUDIO_DEV AudioDevId, AUDIO_TRACK_MODE_E enTrackMode)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }

    pPlay->mTrackMode = enTrackMode;

    return SUCCESS;
}

ERRORTYPE audioHw_AO_GetTrackMode(AUDIO_DEV AudioDevId, AUDIO_TRACK_MODE_E *penTrackMode)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }

    *penTrackMode = pPlay->mTrackMode;

    return SUCCESS;
}

ERRORTYPE audioHw_AO_Enable(AUDIO_DEV AudioDevId)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay; 
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer; 
    
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION) 
    AudioOutputDevice *pPlay_daudio0 = &gAudioHwDev[AudioDevId+1].mPlay; // related with second snd card
    AIO_MIXER_S *pMixer_daudio0 = &gAudioHwDev[AudioDevId+1].mMixer;
#endif

    int ret;

    pthread_mutex_lock(&pPlay->mAOApiCallLock);
    if (pPlay->mState == AO_STATE_INVALID) {
        aloge("%s failed,error_state:%d",__FUNCTION__,pPlay->mState);
        pthread_mutex_unlock(&pPlay->mAOApiCallLock);
        return ERR_AO_NOT_CONFIG;
    }
    if (pPlay->mState == AO_STATE_STARTED) {
        pthread_mutex_unlock(&pPlay->mAOApiCallLock);
        return SUCCESS;
    }

    pPlay->mCfg.chnCnt = pPlay->mAttr.u32ChnCnt;
    pPlay->mCfg.sampleRate = pPlay->mAttr.enSamplerate;
    if (pPlay->mAttr.enBitwidth == AUDIO_BIT_WIDTH_32) {
        pPlay->mCfg.format = SND_PCM_FORMAT_S32_LE;
    } else if (pPlay->mAttr.enBitwidth == AUDIO_BIT_WIDTH_24) {
        pPlay->mCfg.format = SND_PCM_FORMAT_S24_LE;
    } else if (pPlay->mAttr.enBitwidth == AUDIO_BIT_WIDTH_16) {
        pPlay->mCfg.format = SND_PCM_FORMAT_S16_LE;
    } else if (pPlay->mAttr.enBitwidth == AUDIO_BIT_WIDTH_8) {
        pPlay->mCfg.format = SND_PCM_FORMAT_S8;
    } else {
        pPlay->mCfg.format = SND_PCM_FORMAT_S16_LE;
    }
    pPlay->mCfg.bitsPerSample = (pPlay->mAttr.enBitwidth+1)*8;

#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
    memcpy(&pPlay_daudio0->mAttr,&pPlay->mAttr,sizeof(AIO_ATTR_S)); 
    memcpy(&pPlay_daudio0->mCfg,&pPlay->mCfg,sizeof(PCM_CONFIG_S)); 
#endif

    const char *pCardType = (pPlay->mAttr.mPcmCardId==PCM_CARD_TYPE_AUDIOCODEC) ? SOUND_CARD_AUDIOCODEC:SOUND_CARD_SNDHDMI;
    ret = alsaOpenPcm(&pPlay->mCfg, pCardType, 1);
    if (ret != 0) {
        aloge("%s,l:%d,open_pcm failed",__FUNCTION__,__LINE__);
        pthread_mutex_unlock(&pPlay->mAOApiCallLock);
        return FAILURE;
    }

#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
    ret = alsaOpenPcm(&pPlay_daudio0->mCfg, SOUND_CARD_SNDDAUDIo0, 1);  // to open the second snd card
    if (ret != 0) {
        aloge("%s,l:%d,open_pcm failed",__FUNCTION__,__LINE__);
        pthread_mutex_unlock(&pPlay->mAOApiCallLock);
        return FAILURE;
    }

    alsaMixerSetAudioCodecHubMode(pMixer,1);    // to set hub mode for the first snd card
    alsaMixerSetDAudio0HubMode(pMixer_daudio0,2);// to set hub mode for the second snd card
    alsaMixerSetDAudio0LoopBackEn(pMixer_daudio0,1);// to enable loopback for the second snd card
#endif
    
    ret = alsaSetPcmParams(&pPlay->mCfg);
    if (ret < 0) { 
        aloge("%s,l:%d,SetPcmParams failed",__FUNCTION__,__LINE__);
        goto ERR_SET_PCM_PARAM;
    }
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
    ret = alsaSetPcmParams(&pPlay_daudio0->mCfg);   // to set params for the second snd card
    if (ret < 0) {
        aloge("%s,l:%d,SetPcmParams failed",__FUNCTION__,__LINE__);
        goto ERR_SET_PCM_PARAM2;
    }
    alsaPreparePcm(&pPlay_daudio0->mCfg);           // and call the prepare api for the second snd card
#endif

    pthread_mutex_init(&pPlay->mChnListLock, NULL);
    //INIT_LIST_HEAD(&pPlay->mChnList);
    //pPlay->mThdRunning = TRUE;
    //pthread_create(&pPlay->mThdId, NULL, audioHw_AO_PlayThread, &pPlay);

    pPlay->mState = AO_STATE_STARTED;
    pthread_mutex_unlock(&pPlay->mAOApiCallLock);
    return SUCCESS;
    
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
ERR_SET_PCM_PARAM2:
    alsaClosePcm(&pPlay_daudio0->mCfg, 1);  // close the second snd card
#endif

ERR_SET_PCM_PARAM:
    alsaClosePcm(&pPlay->mCfg, 1);  // 1: playback
    pthread_mutex_unlock(&pPlay->mAOApiCallLock);
    return FAILURE;
}

ERRORTYPE audioHw_AO_Disable(AUDIO_DEV AudioDevId)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer; 
    
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
    AudioOutputDevice *pPlay_daudio0 = &gAudioHwDev[AudioDevId+1].mPlay; // related with second snd card
    AIO_MIXER_S *pMixer_daudio0 = &gAudioHwDev[AudioDevId+1].mMixer;

#endif
    pthread_mutex_lock(&pPlay->mAOApiCallLock);
    if (pPlay->mState == AO_STATE_INVALID) {
        pthread_mutex_unlock(&pPlay->mAOApiCallLock);
        return ERR_AO_NOT_CONFIG;
    }
    if (pPlay->mState != AO_STATE_STARTED) {
        pthread_mutex_unlock(&pPlay->mAOApiCallLock);
        return SUCCESS;
    }

    //pPlay->mThdRunning = FALSE;
    //pthread_join(pPlay->mThdId, (void*) &ret);

    if (!list_empty(&pPlay->mChnList)) {
        alogw("When ao_disable, still exist channle in PlayChnList?! list them below:");
        AO_CHANNEL_S *pEntry;
        list_for_each_entry(pEntry, &pPlay->mChnList, mList)
        {
            alogw("AoCardType[%d] AoChn[%d] still run!", pPlay->mAttr.mPcmCardId, pEntry->mId);
        }
        pthread_mutex_unlock(&pPlay->mAOApiCallLock);
        return SUCCESS;
    }

    pthread_mutex_destroy(&pPlay->mChnListLock);
    
#if (MPPCFG_AEC == OPTION_AEC_ENABLE  && !AEC_SOFT_LOOPBACK_SOLUTION)
    alsaMixerSetAudioCodecHubMode(pMixer,0);    // to clean hub mode for the first snd card
    
    alsaMixerSetDAudio0HubMode(pMixer_daudio0,0);// to clean hub mode for the second snd card
    alsaMixerSetDAudio0LoopBackEn(pMixer_daudio0,0);// to disable loopback for the second snd card
    alsaClosePcm(&pPlay_daudio0->mCfg, 1);  // to close the the second snd card 
#endif

    alogd("close pcm! current AoCardType:[%d]", pPlay->mAttr.mPcmCardId);
    alsaClosePcm(&pPlay->mCfg, 1);  // 1: playback
    pPlay->mState = AO_STATE_CONFIGURED;
    pthread_mutex_unlock(&pPlay->mAOApiCallLock);

    return SUCCESS;
}

ERRORTYPE audioHw_AO_SetVolume(AUDIO_DEV AudioDevId, int s32VolumeDb)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }

    return alsaMixerSetVolume(pMixer, 1, s32VolumeDb);
}

ERRORTYPE audioHw_AO_GetVolume(AUDIO_DEV AudioDevId, int *ps32VolumeDb)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }

    return alsaMixerGetVolume(pMixer, 1, (long*)ps32VolumeDb);
}

ERRORTYPE audioHw_AO_SetMute(AUDIO_DEV AudioDevId, BOOL bEnable, AUDIO_FADE_S *pstFade)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }

    return alsaMixerSetMute(pMixer, 1, (int)bEnable);
}

ERRORTYPE audioHw_AO_GetMute(AUDIO_DEV AudioDevId, BOOL *pbEnable, AUDIO_FADE_S *pstFade)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }

    int MainVolVal;
    alsaMixerGetMute(pMixer, 1, &MainVolVal);
    if (MainVolVal > 0)
        *pbEnable = FALSE;
    else
        *pbEnable = TRUE;

    return SUCCESS;
}

ERRORTYPE audioHw_AO_SetPA(AUDIO_DEV AudioDevId, BOOL bHighLevel)
{
    ERRORTYPE ret;
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    ret = alsaMixerSetPlayBackPA(pMixer, (int)bHighLevel);
    if(0 != ret)
    {
        aloge("fatal error! alsaMixer SetPlayBackPA fail[0x%x]!", ret);
    }
    return ret;
}

ERRORTYPE audioHw_AO_GetPA(AUDIO_DEV AudioDevId, BOOL *pbHighLevel)
{
    ERRORTYPE ret;
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    AIO_MIXER_S *pMixer = &gAudioHwDev[AudioDevId].mMixer;

    int bHighLevel = 0;
    ret = alsaMixerGetPlayBackPA(pMixer, &bHighLevel);
    if(0 == ret)
    {
        *pbHighLevel = bHighLevel?TRUE:FALSE;
    }
    else
    {
        aloge("fatal error! alsaMixer GetPlayBackPA fail[0x%x]!", ret);
    }

    return ret;
}

ERRORTYPE audioHw_AO_FillPcmRingBuf(AUDIO_DEV AudioDevId, void* pData, int Len)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;

    size_t frame_cnt = Len / (pPlay->mCfg.bitsPerFrame >> 3);
    ssize_t ret;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }
    ret = alsaWritePcm(&pPlay->mCfg, pData, frame_cnt);
    if (ret != frame_cnt) {
        aloge("alsaWritePcm error!");
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE audioHw_AO_DrainPcmRingBuf(AUDIO_DEV AudioDevId)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    alsaDrainPcm(&pPlay->mCfg);

    return SUCCESS;
}

ERRORTYPE audioHw_AO_FeedPcmData(AUDIO_DEV AudioDevId, AUDIO_FRAME_S *pFrm)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;
    size_t frame_cnt = pFrm->mLen / (pPlay->mCfg.bitsPerFrame >> 3);
    ssize_t ret;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }

    ret = alsaWritePcm(&pPlay->mCfg, pFrm->mpAddr, frame_cnt);
    if (ret != frame_cnt) {
        aloge("alsaWritePcm error!");
        return FAILURE;
    }

    return SUCCESS;
}

ERRORTYPE audioHw_AO_GetPcmConfig(AUDIO_DEV AudioDevId, PCM_CONFIG_S **ppCfg)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }
    *ppCfg = &pPlay->mCfg;
    return SUCCESS;
}

ERRORTYPE audioHw_AO_GetAIOAttr(AUDIO_DEV AudioDevId, AIO_ATTR_S **ppAttr)
{
    AudioOutputDevice *pPlay = &gAudioHwDev[AudioDevId].mPlay;

    if (pPlay->mState != AO_STATE_STARTED) {
        return ERR_AO_NOT_ENABLED;
    }
    *ppAttr = &pPlay->mAttr;
    return SUCCESS;
}
