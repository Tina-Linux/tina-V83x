/*****************************************************************************
**
**  Name:               app_hs.h
**
**  Description:        This is the header file for the Headset application
**
**  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

/* idempotency */
#ifndef APP_HS_H
#define APP_HS_H

/* self-sufficiency */
#include "bta_hs_api.h"
#include "bsa_sec_api.h"
#include "uipc.h"

#ifdef  __cplusplus
extern "C"
{
#endif


/*  call state */
enum
{
    BSA_HS_CALL_NONE,
    BSA_HS_CALL_INC,
    BSA_HS_CALL_OUT,
    BSA_HS_CALL_CONN
};

/* call indicator values */
#define BSA_HS_CALL_INACTIVE         0   /* Phone call inactive */
#define BSA_HS_CALL_ACTIVE           1   /* Phone call active */

/* callsetup indicator values */
#define BSA_HS_CALLSETUP_NONE        0   /* Not currently in call set up */
#define BSA_HS_CALLSETUP_INCOMING    1   /* Incoming call process ongoing */
#define BSA_HS_CALLSETUP_OUTGOING    2   /* Outgoing call set up is ongoing */
#define BSA_HS_CALLSETUP_ALERTING    3   /* Remote party being alerted in an outgoing call */

/* callheld indicator values */
#define BSA_HS_CALLHELD_NONE         0   /* No calls held */
#define BSA_HS_CALLHELD_ACTIVE_HELD  1   /* AG has both an active and a held call */
#define BSA_HS_CALLHELD_HELD         2   /* AG has call on hold, no active call */

#define BSA_HS_ST_CONNECTABLE        0x0001
#define BSA_HS_ST_CONNECT            0x0002
#define BSA_HS_ST_SCALLSETUP         0x0004 /* if value set , outgoingcall and incomming call value are valid*/
#define BSA_HS_ST_OUTGOINGCALL       0x0008 /* 0 => incomming call*/
#define BSA_HS_ST_SCOOPEN            0x0010 /* 0 => sco close */
#define BSA_HS_ST_VOICEDIALACT       0x0020
#define BSA_HS_ST_RINGACT            0x0040
#define BSA_HS_ST_WAITCALL           0x0080
#define BSA_HS_ST_CALLACTIVE         0x0100
#define BSA_HS_ST_INBANDRINGACT      0x0200
#define BSA_HS_ST_NRECACT            0x0400
#define BSA_HS_ST_CLIPACT            0x0800
#define BSA_HS_ST_MUTE               0x1000
#define BSA_HS_ST_3WAY_HELD          0x2000
#define BSA_HS_ST_CONN_SUSPENDED     0x4000
#define BSA_HS_ST_IN_CALL            (BSA_HS_ST_OUTGOINGCALL | BSA_HS_ST_RINGACT | BSA_HS_ST_CALLACTIVE)
#define BSA_HS_ST_ALL                0x1fff

#define BSA_HS_MAX_CL_IDX            3
#define BSA_HS_CL_BUF_SIZE           32

#define BSA_HS_MAX_NUM_CONN          1

#define BSA_HS_SETSTATUS(p,m)        ((p)->status |= (m))
#define BSA_HS_RESETSTATUS(p,m)      ((p)->status &= (~(m)))
#define BSA_HS_GETSTATUS(p,m)        ((p)->status & m)


/* HS APP main control block */
typedef struct
{

     BD_ADDR           connected_bd_addr;       /* peer bdaddr */
     UINT16            handle;                  /* connection handle */
     tUIPC_CH_ID       uipc_channel;
     BOOLEAN           uipc_connected;
     tBSA_SERVICE_MASK connected_hs_service_id; /* service id for HS connection */
     BOOLEAN           connection_active;       /* TRUE if HS connection is active */
     UINT8             call_state;              /* TRUE if HS connection is active */
     tBTA_HS_FEAT      peer_feature;

     BOOLEAN           indicator_string_received;
     UINT8             call_ind_id;
     UINT8             call_setup_ind_id;
     UINT8             service_ind_id;
     UINT8             battery_ind_id;
     UINT8             signal_strength_ind_id;
     UINT8             callheld_ind_id;
     UINT8             roam_ind_id;

     UINT8             curr_call_ind;
     UINT8             curr_call_setup_ind;
     UINT8             curr_service_ind;
     UINT8             curr_battery_ind;
     UINT8             curr_signal_strength_ind;
     UINT8             curr_callheld_ind;
     UINT8             curr_roam_ind;
     UINT16            status;

     UINT8             call_list_info[BSA_HS_MAX_CL_IDX][BSA_HS_CL_BUF_SIZE];
} tBSA_HS_CONN_CB;


    typedef struct {
        UINT8             curr_call_ind;
        UINT8             curr_call_setup_ind;
        UINT8             curr_service_ind;
        UINT8             curr_battery_ind;
        UINT8             curr_signal_strength_ind;
        UINT8             curr_callheld_ind;
        UINT8             curr_callwait_ind;
        UINT8             curr_roam_ind;
    } tBSA_HS_IND_VALS;

    typedef enum {
        BTHF_VR_STATE_STOPPED = 0,
        BTHF_VR_STATE_STARTED = 1
    }tBSA_BTHF_VR_STATE_T;

    typedef enum {
        BTHF_VOLUME_TYPE_SPK = 0,
        BTHF_VOLUME_TYPE_MIC = 1
    }tBSA_BTHF_VOLUME_TYPE_T;

    typedef enum {
        BTHF_CHLD_TYPE_RELEASEHELD,
        BTHF_CHLD_TYPE_RELEASEACTIVE_ACCEPTHELD,
        BTHF_CHLD_TYPE_HOLDACTIVE_ACCEPTHELD,
        BTHF_CHLD_TYPE_ADDHELDTOCONF
    } tBSA_BTHF_CHLD_TYPE_T;

    /* event callback for registered applications */
    typedef void ( tHsCallback)(tBSA_HS_EVT event, tBSA_HS_MSG *p_data);

    /*******************************************************************************
    **
    ** Function         app_hs_open_rec_file
    **
    ** Description     Open recording file
    **
    ** Parameters      filename to open
    **
    ** Returns          void
    **
    *******************************************************************************/
    void app_hs_open_rec_file(char * filename);

    /*******************************************************************************
    **
    ** Function         app_ag_close_rec_file
    **
    ** Description     Close recording file
    **
    ** Parameters
    **
    ** Returns          void
    **
    *******************************************************************************/
    void app_hs_close_rec_file(void);

    /*******************************************************************************
    **
    ** Function         app_hs_start
    **
    ** Description      Example of function to start the Headset application
    **
    ** Parameters       Callback for event notification (can be NULL, if NULL default will be used)
    **
    ** Returns          bsa status error code
    **
    *******************************************************************************/
    int app_hs_start(tHsCallback cb /* = NULL*/);

    /*******************************************************************************
    **
    ** Function         app_hs_init
    **
    ** Description      Init Headset application
    **
    ** Parameters
    **
    ** Returns          void
    **
    *******************************************************************************/
    void app_hs_init(void);

    /*******************************************************************************
    **
    ** Function         app_hs_open
    **
    ** Description      Establishes mono headset connections
    **
    **
    ** Parameters       BD_ADDR of the deivce to connect (if null, user will be prompted)
    **
    ** Returns          0 if success -1 if failure
    *******************************************************************************/
    int app_hs_open(BD_ADDR *bd_addr_in /*= NULL*/);

    /*******************************************************************************
    **
    ** Function         app_hs_audio_open
    **
    ** Description      Establishes audio connections
    **
    **
    ** Parameters       void
    **
    ** Returns          0 if success -1 if failure
    *******************************************************************************/
    int app_hs_audio_open(void);

    /*******************************************************************************
    **
    ** Function         app_hs_audio_close
    **
    ** Description      Closes audio connections
    **
    **
    ** Parameters       void
    **
    ** Returns          0 if success -1 if failure
    *******************************************************************************/
    int app_hs_audio_close(void);

    /*******************************************************************************
    **
    ** Function         app_hs_close
    **
    ** Description      release mono headset connections
    **
    ** Returns          0 if success -1 if failure
    *******************************************************************************/
    int app_hs_close(void);

    /*******************************************************************************
    **
    ** Function         app_hs_cancel
    **
    ** Description      cancel connections
    **
    ** Returns          0 if success -1 if failure
    *******************************************************************************/
    int app_hs_cancel(void);


    /*******************************************************************************
    **
    ** Function         app_is_open_pending
    **
    **
    ** Returns          TRUE if open if pending
    *******************************************************************************/
    BOOLEAN app_is_open_pending();


    /*******************************************************************************
    **
    ** Function         app_hs_answer_call
    **
    ** Description      example of function to answer the call
    **
    ** Returns          0 if success -1 if failure
    *******************************************************************************/
    int app_hs_answer_call(void);

    /*******************************************************************************
    **
    ** Function         app_hs_hangup
    **
    ** Description      example of function to hang up
    **
    ** Returns          0 if success -1 if failure
    *******************************************************************************/
    int app_hs_hangup(void);

    /*******************************************************************************
    **
    ** Function         app_hs_play_file
    **
    ** Description      Play SCO data from file to SCO OUT channel
    **
    ** Parameters
    **
    ** Returns          0 if success -1 if failure
    **
    *******************************************************************************/
    int app_hs_play_file(char * filename);

    /*******************************************************************************
    **
    ** Function         app_hs_enable
    **
    ** Description      Example of function to start enable Hs service
    **
    ** Parameters
    **
    ** Returns          0 if ok -1 in case of error
    **
    *******************************************************************************/
    int app_hs_enable(void);

    /*******************************************************************************
    **
    ** Function         app_hs_register
    **
    ** Description      Example of function to start register one Hs instance
    **
    ** Parameters
    **
    ** Returns          0 if ok -1 in case of error
    **
    *******************************************************************************/
    int app_hs_register(void);

    /*******************************************************************************
    **
    ** Function         app_hs_stop
    **
    ** Description      This function is used to stop hs services and close all
    **                  UIPC channel.
    **
    ** Parameters
    **
    ** Returns          void
    **
    *******************************************************************************/
    void app_hs_stop(void);

    /*******************************************************************************
    **
    ** Function         app_hs_hold_call
    **
    ** Description      This function is used to hold active call
    **
    ** Parameters       tBSA_BTHF_CHLD_TYPE_T type
    **
    ** Returns          void
    **
    *******************************************************************************/
    int app_hs_hold_call(tBSA_BTHF_CHLD_TYPE_T type);


    /*******************************************************************************
    **
    ** Function         app_hs_last_num_dial
    **
    ** Description      This function is used to redial last number
    **
    ** Parameters
    **
    ** Returns          void
    **
    *******************************************************************************/
    int app_hs_last_num_dial(void);


    /*******************************************************************************
    **
    ** Function         app_hs_dial_num
    **
    ** Description      This function is used dial a number
    **
    ** Parameters
    **
    ** Returns          void
    **
    *******************************************************************************/
    int app_hs_dial_num(const char *num);

    /*******************************************************************************
    **
    ** Function         app_hs_send_unat
    **
    ** Description      This function is used to send an unknown AT command
    **
    ** Parameters       char *cCmd
    **
    ** Returns          int
    **
    *******************************************************************************/
    int app_hs_send_unat(char *cCmd);

    /*******************************************************************************
    **
    ** Function         app_hs_send_ind_cmd
    **
    ** Description      This function is used to send an indicator command
    **
    ** Parameters       None
    **
    ** Returns          int
    **
    *******************************************************************************/
    int app_hs_send_ind_cmd();

    /*******************************************************************************
    **
    ** Function         app_hs_send_cnum
    **
    ** Description      Send CNUM AT Command
    **
    ** Parameters       None
    **
    ** Returns          0 if successful execution, error code else
    **
    *******************************************************************************/
    int app_hs_send_cnum(void);

    /*******************************************************************************
    **
    ** Function         app_hs_send_clcc_cmd
    **
    ** Description      Send AT CLCC Command
    **
    ** Parameters       None
    **
    ** Returns          0 if successful execution, error code else
    **
    *******************************************************************************/
    int app_hs_send_clcc_cmd(void);

    /*******************************************************************************
    **
    ** Function         app_hs_send_cops_cmd
    **
    ** Description      Send AT COPS Command
    **
    ** Parameters       None
    **
    ** Returns          0 if successful execution, error code else
    **
    *******************************************************************************/
    int app_hs_send_cops_cmd(char *cCmd);

    /*******************************************************************************
    **
    ** Function         app_hs_send_ind_cmd
    **
    ** Description      Send Indicator Command to obtain details
    **
    ** Parameters       None
    **
    ** Returns          0 if successful execution, error code else
    **
    *******************************************************************************/
    int app_hs_send_ind_cmd(void);

    /*******************************************************************************
    **
    ** Function         app_hs_send_dtmf
    **
    ** Description      Send DTMF Command
    **
    ** Parameters       None
    **
    ** Returns          0 if successful execution, error code else
    **
    *******************************************************************************/
    int app_hs_send_dtmf(char dtmf);

    /*******************************************************************************
    **
    ** Function         app_hs_send_cnum
    **
    ** Description      Send AT CNUM Command
    **
    ** Parameters       None
    **
    ** Returns          0 if successful execution, error code else
    **
    *******************************************************************************/
    int app_hs_send_cnum(void);

    /*******************************************************************************
    **
    ** Function         app_hs_getallIndicatorValues
    **
    ** Description      This function is used to get all indicator values
    **
    ** Parameters       tBSA_HS_IND_VALS *pIndVals
    **
    ** Returns          int
    **
    *******************************************************************************/
    int app_hs_getallIndicatorValues(tBSA_HS_IND_VALS *pIndVals);

    /*******************************************************************************
    **
    ** Function         app_hs_set_volume
    **
    ** Description      This function is used to set volume
    **
    ** Parameters       tBSA_BTHF_VOLUME_TYPE_T type, int volume
    **
    ** Returns          int
    **
    *******************************************************************************/
    int app_hs_set_volume(tBSA_BTHF_VOLUME_TYPE_T type, int volume);


    /*******************************************************************************
    **
    ** Function         app_hs_send_keypress_evt
    **
    ** Description      This function is used to send keypress event
    **
    ** Parameters       char *cCmd - Keyboard sequence (0-9, * , #)
    **
    ** Returns          int
    **
    *******************************************************************************/
    int app_hs_send_keypress_evt(char *cCmd);

    /*******************************************************************************
    **
    ** Function         app_hs_start_voice_recognition
    **
    ** Description      Start voice recognition
    **
    ** Parameters       None
    **
    ** Returns          0 if successful execution, error code else
    **
    *******************************************************************************/
    int app_hs_start_voice_recognition(void);

    /*******************************************************************************
    **
    ** Function         app_hs_stop_voice_recognition
    **
    ** Description      Stop voice recognition
    **
    ** Parameters       None
    **
    ** Returns          0 if successful execution, error code else
    **
    *******************************************************************************/
    int app_hs_stop_voice_recognition(void);

    /*******************************************************************************
    **
    ** Function         app_hs_mute_unmute_microphone
    **
    ** Description      Mute/unmute Microphone
    **
    ** Parameters       bool bMute
    **
    ** Returns          0 if successful execution, error code else
    **
    *******************************************************************************/
    int app_hs_mute_unmute_microphone(BOOLEAN bMute);

    /*******************************************************************************
    **
     ** Function         app_hs_get_conn_by_handle_external
     **
     ** Description      Find a connection control block by its handle
     **
     ** Returns          Pointer to the found connection, NULL if not found
     *******************************************************************************/
	 tBSA_HS_CONN_CB *app_hs_get_conn_by_handle_external(UINT16 handle);
#ifdef  __cplusplus
}
#endif

#endif /* __APP_HS_H__ */
