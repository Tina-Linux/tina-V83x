/**
* @file         mcrc_speech_user_intf.h
* @brief        Midea CRC speech module user interface.
* @author       jiahui.xie
* @date         2019-01-10
* @version      v1.1
*/

#ifndef _MCRC_SPEECH_USER_INTF_H
#define _MCRC_SPEECH_USER_INTF_H

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "kmsg.h"
#include "cJSON.h"
#include "debug.h"
#include <stdbool.h>
#include "vol-control.h"

/** Callback handler event  for mspeech to demo.
* NOTE: Send json to aicloud please use m_aicloud_trans interface in m_ai_app.h.*/
typedef enum MSUI_EVENT_T{
    MSUI_EV_OUT_CMD,                ///<Called when need to send device control cmd.
    MSUI_EV_OUT_NLU,                ///<reserved
    MSUI_EV_OUT_OUTER_DATA,         ///<reserved
    MSUI_EV_OUT_NATIVE_CALL,        ///<reserved
    MSUI_EV_IN_DEVICE_STATUS,       ///<Called when need to send device status check request.
    MSUI_EV_OUT_SPEECH_STATE,       ///<Called when speech working state changed, see enum MSUI_STATE_ below.
    MSUI_EV_OUT_ERROR               ///<reserved
}msui_event_t;

typedef enum MSUI_CB_RETURN_T{
    MSUI_CB_RET_SUCCESS,
    MSUI_CB_RET_FAILED,
    MSUI_CB_RET_TIMEOUT
}msui_cb_return_t;

/**Reserved*/
typedef enum MSUI_RET{
    MSUI_STOP_BY_USER,
    MSUI_ERR_NO_INTERNET,
    MSUI_ERR_ENGINE_CRASH,
    MSUI_ERR_AUTH_FAILED,
    MSUI_ERR_ALSA_FAILED
}msui_ret_t;

/** Mode flag used in mcrc_speech_play_request interface*/
typedef enum PLAY_FLAG{
    MSUI_PLAY_LOCAL_RIGHTNOW,   ///<Break all speaking players then play immediately beyond state machine, but please do not use on http stream audio.(Note : Carefully use this flag)
    MSUI_PLAY_SEQUENTIALLY,     ///<Played sequentially on IDLE state, support setting temp volume for only one time broadcast.
    MSUI_PLAY_LOCAL_TEST,       ///<Played immediately and then switch to LINSTEN state when should_end_session=0, please used in THINK mode.
}play_flag_t;

/** Enum for speech_usrdata.speech_state_now. */
enum{
    MSUI_STATE_WAIT_WAKEUP = 0,     ///<Idel state, waitting for wakeup.
    MSUI_STATE_IN_DIALOGUE,         ///<Receiving audio input.
    MSUI_STATE_ONLINE_THINK,        ///<ASR and NLP processing.
    MSUI_STATE_TTS_SPEAK,           ///<Playing NLP result or broadcastting audio, but greeting playing is not considered.
    MSUI_STATE_INITIALIZING         ///<Speaker is initializing, never play anying thing in this state.
};

/** Callback handler data struct for mspeech and demo.*/
struct speech_usrdata{
    char out_res[16384];        ///<Buffer for data transmitted from mspeech in cJSON format.
    char in_req[16384];         ///<Buffer for data transmitted to mspeech in cJSON format.
    int speech_state_now;       ///<Show mspeech state, see enum MSUI_STATE_.
    msui_event_t event;         ///<Event called from mspeech, seemsui_event_t.
};

/** Callback handler function typedef for mspeech, please refer to mcrc_speech_user_demo.c.*/
typedef msui_cb_return_t (*msui_event_cb_func_t)(struct speech_usrdata *usrdata) ;

/** User struct mcrc_speech_user, which is used for data and event transmission with mspeechs. */
struct mcrc_speech_user {
    pthread_t tid;              ///<Mcrc_speech_process routine tid.
    int running;                ///<Mcrc_speech_process routine running switch, only for read.
    struct kmsg *msg;           ///<Message pointer linked to mspeech kmsg for speech event transmission.
    char config[1024 * 5];      ///<Mspeech onfig json buffer for /oem/res/config.json.
	bool is_mute;               ///<Mic mute control.
	bool keep_silence;
    int is_online_process;      ///<For online/offline asr mode switch, never modifed this after init.
    msui_event_cb_func_t _handler;  ///<Callback handler for mspeech, please refer to mcrc_speech_user_demo.c.
    char fwver[48];
};


/**
 * Mspeech main routine, which contains mspeech front and player state machine and aicloud service.
 * @param[in]       user        User struct mcrc_speech_user, which is used for data and event transmission.
 * @return      msui_ret_t      No return while running, return MSUI_RET when stopped.
 * @note
 */
msui_ret_t mcrc_speech_process(struct mcrc_speech_user *user);

/**
 * Set user.running to 0, in order to stop mspeech main routine.
 * @param[in]       user        User struct mcrc_speech_user, which is used for data and event transmission.
 * @note
 */
void mcrc_speech_stop(struct mcrc_speech_user *user);

/**
 * Mspeech player interface, which supports to play mp3/aac/http stream format audio in RIGHTNOW/SEQUENTIALLY/LOCAL_TEST mode.
 * @param[in]       user        User struct mcrc_speech_user, which is used for data and event transmission.
 * @param[in]       audio_souce     Audio path or http link.
 * @param[in]       pflag       Playing mode, please refer to play_flag_t.
 * @param[in]       should_end_session      Only used in LOCAL_TEST mode for scheldue next round conversion(1-no next round, 1-have next round), and please set 1 when used in other mode.
 * @param[in]       temp_vol        Only use in SEQUENTIALLY mode for setting temp volume to broadcast once.
 * @return      -1:fail/0:success
 * @note
 */
int mcrc_speech_play_request(struct mcrc_speech_user *user, const char *audio_souce, play_flag_t pflag, int should_end_session, int temp_vol);

/**
 * Mspeech aicould net state broadcast switch interface.
 * @param[in]       iPlayOn 0:off 1:on
 * @return      Always 0.
 * @note
 */
int mcrc_speech_netstatus_play_onoff(int iPlayOn);

/**
 * System suspend interface.
 * @return      0:Resumed/-1:Resume failed/blocked:In suspend.
 * @note        Must be used in MSUI_STATE_WAIT_WAKEUP state.
 */
int mcrc_speech_vad_suspend_system(void);

#endif
