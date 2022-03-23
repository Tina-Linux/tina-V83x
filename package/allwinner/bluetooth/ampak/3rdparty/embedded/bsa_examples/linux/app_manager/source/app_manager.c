/*****************************************************************************
**
**  Name:           app_manager.c
**
**  Description:    Bluetooth Manager application
**
**  Copyright (c) 2010-2015, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/time.h>
#include <pthread.h>

#include "gki.h"
#include "uipc.h"

#include "bsa_api.h"

#include "app_xml_utils.h"

#include "app_disc.h"
#include "app_utils.h"
#include "app_dm.h"

#include "app_services.h"
#include "app_mgt.h"

#include "app_manager.h"

/* Default BdAddr */
#define APP_DEFAULT_BD_ADDR             {0xBE, 0xEF, 0xBE, 0xEF, 0x00, 0x01}

/* Default local Name */
#define APP_DEFAULT_BT_NAME             "My BSA Bluetooth Device"

/* Default COD SetTopBox (Major Service = none) (MajorDevclass = Audio/Video) (Minor=STB) */
#define APP_DEFAULT_CLASS_OF_DEVICE     {0x00, 0x04, 0x24}

#define APP_DEFAULT_ROOT_PATH           "./pictures"

#define APP_DEFAULT_PIN_CODE            "0000"
#define APP_DEFAULT_PIN_LEN             4

#define APP_XML_CONFIG_FILE_PATH            "./bt_config.xml"
#define APP_XML_REM_DEVICES_FILE_PATH       "./bt_devices.xml"

/*
 * Simple Pairing Input/Output Capabitilies:
 * BSA_SEC_IO_CAP_OUT  =>  DisplayOnly
 * BSA_SEC_IO_CAP_IO   =>  DisplayYesNo
 * BSA_SEC_IO_CAP_IN   =>  KeyboardOnly
 * BSA_SEC_IO_CAP_NONE =>  NoInputNoOutput
* * #if BLE_INCLUDED == TRUE && SMP_INCLUDED == TRUE
 * BSA_SEC_IO_CAP_KBDISP =>  KeyboardDisplay
 * #endif
 */
#ifndef APP_SEC_IO_CAPABILITIES
#define APP_SEC_IO_CAPABILITIES BSA_SEC_IO_CAP_NONE
#endif

/*
 * Global variables
 */
extern char bta_conf_path[];
BD_NAME local_device_conf_name;
BD_ADDR local_device_set_addr;
tAPP_XML_CONFIG         app_xml_config;
BD_ADDR                 app_sec_db_addr;    /* BdAddr of peer device requesting SP */

tAPP_MGR_CB app_mgr_cb;

tBSA_SEC_SET_REMOTE_OOB bsa_sec_set_remote_oob;

tBSA_SEC_PASSKEY_REPLY g_passkey_reply;

/*
 * Extern functions
 */
extern void bta_load_addr(const char *p_path);

/*
 * Local functions
 */
int app_mgr_config(void);
char *app_mgr_get_dual_stack_mode_desc(void);
static tBSA_SEC_CBACK  *s_pCallback = NULL;

/*******************************************************************************
 **
 ** Function         app_mgr_read_config
 **
 ** Description      This function is used to read the XML bluetooth configuration file
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_read_config(void)
{
    int status;

    status = app_xml_read_cfg(APP_XML_CONFIG_FILE_PATH, &app_xml_config);
    if (status < 0)
    {
        APP_ERROR1("app_xml_read_cfg failed:%d", status);
        return -1;
    }
    else
    {
        APP_DEBUG1("Enable:%d", app_xml_config.enable);
        APP_DEBUG1("Discoverable:%d", app_xml_config.discoverable);
        APP_DEBUG1("Connectable:%d", app_xml_config.connectable);
        APP_DEBUG1("Name:%s", app_xml_config.name);
        APP_DEBUG1("Bdaddr %02x:%02x:%02x:%02x:%02x:%02x",
                app_xml_config.bd_addr[0], app_xml_config.bd_addr[1],
                app_xml_config.bd_addr[2], app_xml_config.bd_addr[3],
                app_xml_config.bd_addr[4], app_xml_config.bd_addr[5]);
        APP_DEBUG1("ClassOfDevice:%02x:%02x:%02x => %s", app_xml_config.class_of_device[0],
                app_xml_config.class_of_device[1], app_xml_config.class_of_device[2],
                app_get_cod_string(app_xml_config.class_of_device));
        APP_DEBUG1("RootPath:%s", app_xml_config.root_path);

        APP_DEBUG1("Default PIN (%d):%s", app_xml_config.pin_len, app_xml_config.pin_code);
    }
    return 0;
}



/*******************************************************************************
 **
 ** Function         app_mgr_write_config
 **
 ** Description      This function is used to write the XML bluetooth configuration file
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_write_config(void)
{
    int status;

    status = app_xml_write_cfg(APP_XML_CONFIG_FILE_PATH, &app_xml_config);
    if (status < 0)
    {
        APP_ERROR1("app_xml_write_cfg failed:%d", status);
        return -1;
    }
    else
    {
        APP_DEBUG1("Enable:%d", app_xml_config.enable);
        APP_DEBUG1("Discoverable:%d", app_xml_config.discoverable);
        APP_DEBUG1("Connectable:%d", app_xml_config.connectable);
        APP_DEBUG1("Name:%s", app_xml_config.name);
        APP_DEBUG1("Bdaddr %02x:%02x:%02x:%02x:%02x:%02x",
                app_xml_config.bd_addr[0], app_xml_config.bd_addr[1],
                app_xml_config.bd_addr[2], app_xml_config.bd_addr[3],
                app_xml_config.bd_addr[4], app_xml_config.bd_addr[5]);
        APP_DEBUG1("ClassOfDevice:%02x:%02x:%02x", app_xml_config.class_of_device[0],
                app_xml_config.class_of_device[1], app_xml_config.class_of_device[2]);
        APP_DEBUG1("RootPath:%s", app_xml_config.root_path);
    }
    return 0;
}

static pthread_t write_thread_id;
static pthread_cond_t  write_cond;
static pthread_mutex_t write_mutex;
static int  write_thread_running = 0;
static int  write_running = 0;

/*******************************************************************************
 **
 ** Function         app_mgr_read_remote_devices
 **
 ** Description      This function is used to read the XML bluetooth remote device file
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_read_remote_devices(void)
{
    int status;
    int index;

    pthread_mutex_lock(&write_mutex);

    for (index = 0 ; index < APP_NUM_ELEMENTS(app_xml_remote_devices_db) ; index++)
    {
        app_xml_remote_devices_db[index].in_use = FALSE;
    }

    status = app_xml_read_db(APP_XML_REM_DEVICES_FILE_PATH, app_xml_remote_devices_db,
            APP_NUM_ELEMENTS(app_xml_remote_devices_db));

    if (status < 0)
    {
        APP_ERROR1("app_xml_read_db failed:%d", status);
        pthread_mutex_unlock(&write_mutex);
        return -1;
    }

    pthread_mutex_unlock(&write_mutex);
    return 0;
}

void *write_remote_devices_thread(void *args)
{
    int status;

    while (write_thread_running == 1) {
	  pthread_mutex_lock(&write_mutex);

        while(write_running == 0){
            pthread_cond_wait(&write_cond, &write_mutex);
        }

        status = app_xml_write_db(APP_XML_REM_DEVICES_FILE_PATH, app_xml_remote_devices_db,
            APP_NUM_ELEMENTS(app_xml_remote_devices_db));

        if (status < 0)
        {
            APP_ERROR1("app_xml_write_db failed:%d", status);
        }
        else
        {
            APP_DEBUG0("app_xml_write_db ok");
        }

        write_running = 0;
        pthread_mutex_unlock(&write_mutex);
    }

    pthread_exit(NULL);
}


/*******************************************************************************
 **
 ** Function         app_mgr_write_remote_devices
 **
 ** Description      This function is used to write the XML bluetooth remote device file
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_write_remote_devices(void)
{
    pthread_mutex_lock(&write_mutex);
    write_running = 1;
    pthread_mutex_unlock(&write_mutex);
    pthread_cond_signal(&write_cond);
    return 0;
}

void app_mgr_init_write_thread(void)
{
    pthread_mutex_init(&write_mutex, NULL);
    pthread_cond_init(&write_cond, NULL);
    write_running = 0;
    write_thread_running = 1;
    pthread_create(&write_thread_id, NULL, &write_remote_devices_thread, NULL);
}

void app_mgr_destroy_write_thread(void)
{
	  write_thread_running = 0;

	  pthread_mutex_lock(&write_mutex);
    write_running = 1;
    pthread_mutex_unlock(&write_mutex);
    pthread_cond_signal(&write_cond);

    pthread_join(write_thread_id, NULL);
    pthread_cond_destroy(&write_cond);
    pthread_mutex_destroy(&write_mutex);
}


/*******************************************************************************
 **
 ** Function         app_mgr_set_bt_config
 **
 ** Description      This function is used to get the bluetooth configuration
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_set_bt_config(BOOLEAN enable)
{
    int status;
    tBSA_DM_SET_CONFIG bt_config;

    /* Init config parameter */
    status = BSA_DmSetConfigInit(&bt_config);

    /*
     * Set bluetooth configuration (from XML file)
     */
    bt_config.enable = enable;
    bt_config.discoverable = app_xml_config.discoverable;
    bt_config.connectable = app_xml_config.connectable;
    bdcpy(bt_config.bd_addr, app_xml_config.bd_addr);
    strncpy((char *)bt_config.name, (char *)app_xml_config.name, sizeof(bt_config.name));
    bt_config.name[sizeof(bt_config.name) - 1] = '\0';
    memcpy(bt_config.class_of_device, app_xml_config.class_of_device, sizeof(DEV_CLASS));

    APP_DEBUG1("Enable:%d", bt_config.enable);
    APP_DEBUG1("Discoverable:%d", bt_config.discoverable);
    APP_DEBUG1("Connectable:%d", bt_config.connectable);
    APP_DEBUG1("Name:%s", bt_config.name);
    APP_DEBUG1("Bdaddr %02x:%02x:%02x:%02x:%02x:%02x",
            bt_config.bd_addr[0], bt_config.bd_addr[1],
            bt_config.bd_addr[2], bt_config.bd_addr[3],
            bt_config.bd_addr[4], bt_config.bd_addr[5]);
    APP_DEBUG1("ClassOfDevice:%02x:%02x:%02x", bt_config.class_of_device[0],
            bt_config.class_of_device[1], bt_config.class_of_device[2]);
    APP_DEBUG1("First host disabled channel:%d", bt_config.first_disabled_channel);
    APP_DEBUG1("Last host disabled channel:%d", bt_config.last_disabled_channel);

    /* Apply BT config */
    status = BSA_DmSetConfig(&bt_config);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DmSetConfig failed:%d", status);
        return(-1);
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function         app_mgr_get_bt_config
 **
 ** Description      This function is used to get the bluetooth configuration
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_get_bt_config(void)
{
    int status;
    tBSA_DM_GET_CONFIG bt_config;

    /*
     * Get bluetooth configuration
     */
    status = BSA_DmGetConfigInit(&bt_config);
    status = BSA_DmGetConfig(&bt_config);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DmGetConfig failed:%d", status);
        return(-1);
    }
    APP_DEBUG1("Enable:%d", bt_config.enable);
    APP_DEBUG1("Discoverable:%d", bt_config.discoverable);
    APP_DEBUG1("Connectable:%d", bt_config.connectable);
    APP_DEBUG1("Name:%s", bt_config.name);
    APP_DEBUG1("Bdaddr %02x:%02x:%02x:%02x:%02x:%02x",
             bt_config.bd_addr[0], bt_config.bd_addr[1],
             bt_config.bd_addr[2], bt_config.bd_addr[3],
             bt_config.bd_addr[4], bt_config.bd_addr[5]);
    APP_DEBUG1("ClassOfDevice:%02x:%02x:%02x", bt_config.class_of_device[0],
            bt_config.class_of_device[1], bt_config.class_of_device[2]);
    APP_DEBUG1("First host disabled channel:%d", bt_config.first_disabled_channel);
    APP_DEBUG1("Last host disabled channel:%d", bt_config.last_disabled_channel);

    return 0;

}
/*******************************************************************************
 **
 ** Function         app_mgr_get_bd_addr
 **
 ** Description      This function is used to get the bluetooth address
 **
 ** Parameters
 **
 ** Returns          0--successed, 1--failed
 **
 *******************************************************************************/
int app_mgr_get_bd_addr(BD_ADDR bd_addr)
{
    int status;
    int i;
    tBSA_DM_GET_CONFIG bt_config;
    /*
     * Get bluetooth config
     */
    status = BSA_DmGetConfigInit(&bt_config);
    status = BSA_DmGetConfig(&bt_config);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_DmGetConfig failed:%d", status);
        return(-1);
    }
    APP_DEBUG1("Bdaddr %02x:%02x:%02x:%02x:%02x:%02x",
             bt_config.bd_addr[0], bt_config.bd_addr[1],
             bt_config.bd_addr[2], bt_config.bd_addr[3],
             bt_config.bd_addr[4], bt_config.bd_addr[5]);

    for(i = 0; i < 6; i++)
	bd_addr[i] = bt_config.bd_addr[i];

    return 0;

}

/*******************************************************************************
 **
 ** Function         app_mgr_read_oob
 **
 ** Description      This function is used to read local OOB data from local controller
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_read_oob_data()
{
    tBSA_SEC_READ_OOB bsa_sec_read_oob;
    if (BSA_SecReadOOBInit(&bsa_sec_read_oob) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfigInit failed");
        return;
    }

    if (BSA_SecReadOOB(&bsa_sec_read_oob) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfig failed");
        return;
    }
}

/*******************************************************************************
 **
 ** Function         app_mgr_local_oob_evt_data
 **
 ** Description      This function prints out local OOB data returned by the
 **                  local controller
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/

void app_mgr_local_oob_evt_data(tBSA_SEC_LOCAL_OOB_MSG* p_data)
{
    char szt[128];
    BT_OCTET16 p_c;
    BT_OCTET16 p_r;
    memcpy(p_c, p_data->c, 16);
    memcpy(p_r, p_data->r, 16);

    memset(szt,0, sizeof(szt));
    sprintf(szt,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
    p_c[0],p_c[1],p_c[2],p_c[3],p_c[4],p_c[5],p_c[6],p_c[7],
    p_c[8],p_c[9],p_c[10],p_c[11],p_c[12],p_c[13],p_c[14],p_c[15]);
    APP_DEBUG1("app_mgr_local_oob_evt_data: C Hash: %s", szt);

    memset(szt,0, sizeof(szt));
    sprintf(szt,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
    p_r[0],p_r[1],p_r[2],p_r[3],p_r[4],p_r[5],p_r[6],p_r[7],
    p_r[8],p_r[9],p_r[10],p_r[11],p_r[12],p_r[13],p_r[14],p_r[15]);
    APP_DEBUG1("app_mgr_local_oob_evt_data: R Randomizer: %s", szt);

    APP_DEBUG1("app_mgr_local_oob_evt_data: OOB Valid: %d", p_data->valid);
}


/*******************************************************************************
 **
 ** Function         app_mgr_set_remote_oob
 **
 ** Description      This function is used to set OOB data for peer device
 **                  During pairing stack uses this information to pair
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_set_remote_oob()
{
    unsigned int uarr[16];
    char szInput[64];
    int  i;

    if (BSA_SecSetRemoteOOBInit(&bsa_sec_set_remote_oob) != BSA_SUCCESS)
    {
        APP_ERROR0("app_mgr_set_remote_oob failed");
        return;
    }

    APP_INFO0("Enter BDA of Peer device: AABBCCDDEEFF");
    memset(szInput, 0, sizeof(szInput));
    app_get_string("Enter BDA: ", szInput, sizeof(szInput));
    sscanf (szInput, "%02x%02x%02x%02x%02x%02x",
                   &uarr[0], &uarr[1], &uarr[2], &uarr[3], &uarr[4], &uarr[5]);

    for(i=0; i < 6; i++)
        bsa_sec_set_remote_oob.bd_addr[i] = (UINT8) uarr[i];

    bsa_sec_set_remote_oob.result = TRUE;   /* TRUE to accept; FALSE to reject */

    APP_INFO0("Enter 16 byte C Hash in this format: FA87C0D0AFAC11DE8A390800200C9A66");
    memset(szInput, 0, sizeof(szInput));
    app_get_string("C Hash: ", szInput, sizeof(szInput));
    sscanf (szInput, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                   &uarr[0], &uarr[1], &uarr[2], &uarr[3], &uarr[4], &uarr[5], &uarr[6], &uarr[7], &uarr[8],
            &uarr[9], &uarr[10], &uarr[11], &uarr[12], &uarr[13], &uarr[14], &uarr[15]);

    for(i=0; i < 16; i++)
        bsa_sec_set_remote_oob.p_c[i] = (UINT8) uarr[i];

    APP_INFO0("Enter 16 byte R Randomizer in this format: FA87C0D0AFAC11DE8A390800200C9A66");
    memset(szInput, 0, sizeof(szInput));
    app_get_string("R Randomizer: ", szInput, sizeof(szInput));
    sscanf (szInput, "%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
                   &uarr[0], &uarr[1], &uarr[2], &uarr[3], &uarr[4], &uarr[5], &uarr[6], &uarr[7], &uarr[8],
            &uarr[9], &uarr[10], &uarr[11], &uarr[12], &uarr[13], &uarr[14], &uarr[15]);

    for(i=0; i < 16; i++)
        bsa_sec_set_remote_oob.p_r[i] = (UINT8) uarr[i];

    if (BSA_SecSetRemoteOOB(&bsa_sec_set_remote_oob) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_SecSetRemoteOOB failed");
        return;
    }
}

static tBSA_SEC_PIN_CODE_REPLY g_pin_code_reply;

/*******************************************************************************
 **
 ** Function         app_mgr_security_callback
 **
 ** Description      Security callback
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_security_callback(tBSA_SEC_EVT event, tBSA_SEC_MSG *p_data)
{
    int status;
    int indexDisc;
    tBSA_SEC_PIN_CODE_REPLY pin_code_reply;
    tBSA_SEC_AUTH_REPLY autorize_reply;

    APP_DEBUG1("event:%d", event);

    switch(event)
    {
    case BSA_SEC_LINK_UP_EVT:       /* A device is physically connected (for info) */
        APP_DEBUG1("BSA_SEC_LINK_UP_EVT bd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->link_up.bd_addr[0], p_data->link_up.bd_addr[1],
                p_data->link_up.bd_addr[2], p_data->link_up.bd_addr[3],
                p_data->link_up.bd_addr[4], p_data->link_up.bd_addr[5]);
        APP_DEBUG1("ClassOfDevice:%02x:%02x:%02x => %s",
                p_data->link_up.class_of_device[0],
                p_data->link_up.class_of_device[1],
                p_data->link_up.class_of_device[2],
                app_get_cod_string(p_data->link_up.class_of_device));
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
        APP_DEBUG1("LinkType: %d", p_data->link_up.link_type);
#endif
        break;
    case BSA_SEC_LINK_DOWN_EVT:     /* A device is physically disconnected (for info)*/
        APP_DEBUG1("BSA_SEC_LINK_DOWN_EVT bd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->link_down.bd_addr[0], p_data->link_down.bd_addr[1],
                p_data->link_down.bd_addr[2], p_data->link_down.bd_addr[3],
                p_data->link_down.bd_addr[4], p_data->link_down.bd_addr[5]);
        APP_DEBUG1("Reason: %d", p_data->link_down.status);
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
        APP_DEBUG1("LinkType: %d", p_data->link_down.link_type);
#endif
        break;

    case BSA_SEC_PIN_REQ_EVT:
        APP_DEBUG1("BSA_SEC_PIN_REQ_EVT (Pin Code Request) received from: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->pin_req.bd_addr[0], p_data->pin_req.bd_addr[1],
                p_data->pin_req.bd_addr[2], p_data->pin_req.bd_addr[3],
                p_data->pin_req.bd_addr[4],p_data->pin_req.bd_addr[5]);

        APP_DEBUG1("call BSA_SecPinCodeReply pin:%s len:%d",
                app_xml_config.pin_code, app_xml_config.pin_len);

        BSA_SecPinCodeReplyInit(&pin_code_reply);
        bdcpy(pin_code_reply.bd_addr, p_data->pin_req.bd_addr);
        pin_code_reply.pin_len = app_xml_config.pin_len;
        strncpy((char *)pin_code_reply.pin_code, (char *)app_xml_config.pin_code,
                app_xml_config.pin_len);
        /* note that this code will not work if pin_len = 16 */
        pin_code_reply.pin_code[PIN_CODE_LEN-1] = '\0';
        status = BSA_SecPinCodeReply(&pin_code_reply);
        break;

    case BSA_SEC_AUTH_CMPL_EVT:
        APP_DEBUG1("BSA_SEC_AUTH_CMPL_EVT (name=%s,success=%d)",
                   p_data->auth_cmpl.bd_name, p_data->auth_cmpl.success);
        if (!p_data->auth_cmpl.success)
        {
            APP_DEBUG1("    fail_reason=%d", p_data->auth_cmpl.fail_reason);
        }
        APP_DEBUG1("    bd_addr:%02x:%02x:%02x:%02x:%02x:%02x",
                p_data->auth_cmpl.bd_addr[0], p_data->auth_cmpl.bd_addr[1], p_data->auth_cmpl.bd_addr[2],
                p_data->auth_cmpl.bd_addr[3], p_data->auth_cmpl.bd_addr[4], p_data->auth_cmpl.bd_addr[5]);
        if (p_data->auth_cmpl.key_present != FALSE)
        {
            APP_DEBUG1("    LinkKey:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x",
                    p_data->auth_cmpl.key[0], p_data->auth_cmpl.key[1], p_data->auth_cmpl.key[2], p_data->auth_cmpl.key[3],
                    p_data->auth_cmpl.key[4], p_data->auth_cmpl.key[5], p_data->auth_cmpl.key[6], p_data->auth_cmpl.key[7],
                    p_data->auth_cmpl.key[8], p_data->auth_cmpl.key[9], p_data->auth_cmpl.key[10], p_data->auth_cmpl.key[11],
                    p_data->auth_cmpl.key[12], p_data->auth_cmpl.key[13], p_data->auth_cmpl.key[14], p_data->auth_cmpl.key[15]);
        }

        /* If success */
        if (p_data->auth_cmpl.success != 0)
        {
            /* Read the Remote device xml file to have a fresh view */
            app_mgr_read_remote_devices();

            app_xml_update_name_db(app_xml_remote_devices_db,
                                   APP_NUM_ELEMENTS(app_xml_remote_devices_db),
                                   p_data->auth_cmpl.bd_addr,
                                   p_data->auth_cmpl.bd_name);

            if (p_data->auth_cmpl.key_present != FALSE)
            {
                app_xml_update_key_db(app_xml_remote_devices_db,
                                      APP_NUM_ELEMENTS(app_xml_remote_devices_db),
                                      p_data->auth_cmpl.bd_addr,
                                      p_data->auth_cmpl.key,
                                      p_data->auth_cmpl.key_type);
            }

            /* Unfortunately, the BSA_SEC_AUTH_CMPL_EVT does not contain COD, let's look in the Discovery database */
            for (indexDisc = 0 ; indexDisc < APP_DISC_NB_DEVICES ; indexDisc++)
            {
                if ((app_discovery_cb.devs[indexDisc].in_use != FALSE) &&
                    (bdcmp(app_discovery_cb.devs[indexDisc].device.bd_addr, p_data->auth_cmpl.bd_addr) == 0))

                {
                    app_xml_update_cod_db(app_xml_remote_devices_db,
                                          APP_NUM_ELEMENTS(app_xml_remote_devices_db),
                                          p_data->auth_cmpl.bd_addr,
                                          app_discovery_cb.devs[indexDisc].device.class_of_device);
                }
            }

            status = app_mgr_write_remote_devices();
            if (status < 0)
            {
                APP_ERROR1("app_mgr_write_remote_devices failed:%d", status);
            }

#ifdef BSA_PEER_IOS
            /* Start device info discovery */
            app_disc_start_dev_info(p_data->auth_cmpl.bd_addr, NULL);
#endif
        }
        break;

    case BSA_SEC_BOND_CANCEL_CMPL_EVT:
        APP_DEBUG1("BSA_SEC_BOND_CANCEL_CMPL_EVT status=%d",
                   p_data->bond_cancel.status);
        break;

    case BSA_SEC_AUTHORIZE_EVT:  /* Authorization request */
        APP_DEBUG0("BSA_SEC_AUTHORIZE_EVT");
        APP_DEBUG1("    Remote device:%s", p_data->authorize.bd_name);
        APP_DEBUG1("    bd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->authorize.bd_addr[0], p_data->authorize.bd_addr[1],
                p_data->authorize.bd_addr[2], p_data->authorize.bd_addr[3],
                p_data->authorize.bd_addr[4],p_data->authorize.bd_addr[5]);
        APP_DEBUG1("    Request access to service:%x (%s)",
                (int)p_data->authorize.service,
                app_service_id_to_string(p_data->authorize.service));
        APP_DEBUG0("    Access Granted (permanently)");
        status = BSA_SecAuthorizeReplyInit(&autorize_reply);
        bdcpy(autorize_reply.bd_addr, p_data->authorize.bd_addr);
        autorize_reply.trusted_service = p_data->authorize.service;
        autorize_reply.auth = BSA_SEC_AUTH_PERM;
        status = BSA_SecAuthorizeReply(&autorize_reply);
        /*
         * Update XML database
         */
        /* Read the Remote device xml file to have a fresh view */
        app_mgr_read_remote_devices();
        /* Add AV service for this devices in XML database */
        app_xml_add_trusted_services_db(app_xml_remote_devices_db,
                APP_NUM_ELEMENTS(app_xml_remote_devices_db), p_data->authorize.bd_addr,
                1 << p_data->authorize.service);

        if (strlen((char *)p_data->authorize.bd_name) > 0)
            app_xml_update_name_db(app_xml_remote_devices_db,
                    APP_NUM_ELEMENTS(app_xml_remote_devices_db),
                    p_data->authorize.bd_addr, p_data->authorize.bd_name);

        /* Update database => write on disk */
        status = app_mgr_write_remote_devices();
        if (status < 0)
        {
            APP_ERROR1("app_mgr_write_remote_devices failed:%d", status);
        }
        break;

    case BSA_SEC_SP_CFM_REQ_EVT: /* Simple Pairing confirm request */
        APP_DEBUG0("BSA_SEC_SP_CFM_REQ_EVT");
        APP_DEBUG1("    Remote device:%s", p_data->cfm_req.bd_name);
        APP_DEBUG1("    bd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->cfm_req.bd_addr[0], p_data->cfm_req.bd_addr[1],
                p_data->cfm_req.bd_addr[2], p_data->cfm_req.bd_addr[3],
                p_data->cfm_req.bd_addr[4], p_data->cfm_req.bd_addr[5]);
        APP_DEBUG1("    ClassOfDevice:%02x:%02x:%02x => %s",
                p_data->cfm_req.class_of_device[0], p_data->cfm_req.class_of_device[1], p_data->cfm_req.class_of_device[2],
                app_get_cod_string(p_data->cfm_req.class_of_device));
        APP_DEBUG1("    Just Work:%s", p_data->cfm_req.just_works == TRUE ? "TRUE" : "FALSE");
        APP_DEBUG1("    Numeric Value:%d", p_data->cfm_req.num_val);
        APP_DEBUG0("    You must accept or refuse using menu\n");
        bdcpy(app_sec_db_addr, p_data->cfm_req.bd_addr);

        /* Auto accept simple Pairing*/
        app_mgr_sp_cfm_reply(TRUE, app_sec_db_addr);
        break;

    case BSA_SEC_SP_KEY_NOTIF_EVT: /* Simple Pairing Passkey Notification */
        APP_DEBUG0("BSA_SEC_SP_KEY_NOTIF_EVT");
        APP_DEBUG1("    bd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->key_notif.bd_addr[0], p_data->key_notif.bd_addr[1], p_data->key_notif.bd_addr[2],
                p_data->key_notif.bd_addr[3], p_data->key_notif.bd_addr[4], p_data->key_notif.bd_addr[5]);
        APP_DEBUG1("    ClassOfDevice:%02x:%02x:%02x => %s",
                p_data->key_notif.class_of_device[0], p_data->key_notif.class_of_device[1], p_data->key_notif.class_of_device[2],
                app_get_cod_string(p_data->key_notif.class_of_device));
        APP_DEBUG1("    Numeric Value:%d", p_data->key_notif.passkey);
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
        APP_DEBUG1("    LinkType: %d", p_data->key_notif.link_type);
#endif
        APP_DEBUG0("    You must enter this value on peer device's keyboard");
        break;

    case BSA_SEC_SP_KEY_REQ_EVT: /* Simple Pairing Passkey request Notification */
        APP_DEBUG0("BSA_SEC_SP_KEY_REQ_EVT");
        APP_DEBUG1("    bd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->key_notif.bd_addr[0], p_data->key_notif.bd_addr[1], p_data->key_notif.bd_addr[2],
                p_data->key_notif.bd_addr[3], p_data->key_notif.bd_addr[4], p_data->key_notif.bd_addr[5]);
        APP_DEBUG1("    ClassOfDevice:%02x:%02x:%02x => %s",
                p_data->key_notif.class_of_device[0], p_data->key_notif.class_of_device[1], p_data->key_notif.class_of_device[2],
                app_get_cod_string(p_data->key_notif.class_of_device));
        APP_DEBUG1("    Numeric Value:%d", p_data->key_notif.passkey);
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
        APP_DEBUG1("    LinkType: %d", p_data->key_notif.link_type);
#endif
        APP_DEBUG0("    You must enter this value on peer device's keyboard");

        memset(&g_passkey_reply, 0, sizeof(g_passkey_reply));
        bdcpy(g_passkey_reply.bd_addr, p_data->pin_req.bd_addr);
        break;

    case BSA_SEC_SP_KEYPRESS_EVT: /* Simple Pairing Key press notification event. */
        APP_DEBUG1("BSA_SEC_SP_KEYPRESS_EVT (type:%d)", p_data->key_press.notif_type);
        APP_DEBUG1("    bd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->key_press.bd_addr[0], p_data->key_press.bd_addr[1], p_data->key_press.bd_addr[2],
                p_data->key_press.bd_addr[3], p_data->key_press.bd_addr[4], p_data->key_press.bd_addr[5]);
        break;

   case BSA_SEC_SP_RMT_OOB_EVT: /* Simple Pairing Remote OOB Data request. */
       APP_DEBUG0("BSA_SEC_SP_RMT_OOB_EVT received - Handled internally if Peer OOB already set");
       break;

    case BSA_SEC_LOCAL_OOB_DATA_EVT: /* Local OOB Data response */
        APP_DEBUG0("BSA_SEC_LOCAL_OOB_DATA_EVT received");
        app_mgr_local_oob_evt_data(&p_data->local_oob);
        break;

    case BSA_SEC_SUSPENDED_EVT: /* Connection Suspended */
        APP_INFO1("BSA_SEC_SUSPENDED_EVT bd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->suspended.bd_addr[0], p_data->suspended.bd_addr[1],
                p_data->suspended.bd_addr[2], p_data->suspended.bd_addr[3],
                p_data->suspended.bd_addr[4], p_data->suspended.bd_addr[5]);
        break;

    case BSA_SEC_RESUMED_EVT: /* Connection Resumed */
        APP_INFO1("BSA_SEC_RESUMED_EVT bd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->resumed.bd_addr[0], p_data->resumed.bd_addr[1],
                p_data->resumed.bd_addr[2], p_data->resumed.bd_addr[3],
                p_data->resumed.bd_addr[4], p_data->resumed.bd_addr[5]);
        break;

#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
    case BSA_SEC_BLE_KEY_EVT: /* BLE KEY event */
        APP_INFO0("BSA_SEC_BLE_KEY_EVT");
        switch (p_data->ble_key.key_type)
        {
        case BSA_LE_KEY_PENC:
            APP_DEBUG1("\t key_type: BTM_LE_KEY_PENC(%d)", p_data->ble_key.key_type);
            APP_DUMP("LTK", p_data->ble_key.key_value.penc_key.ltk, 16);
            APP_DUMP("RAND", p_data->ble_key.key_value.penc_key.rand, 8);
            APP_DEBUG1("ediv: 0x%x:", p_data->ble_key.key_value.penc_key.ediv);
            APP_DEBUG1("sec_level: 0x%x:", p_data->ble_key.key_value.penc_key.sec_level);
            APP_DEBUG1("key_size: %d:", p_data->ble_key.key_value.penc_key.key_size);
            break;
        case BSA_LE_KEY_PID:
            APP_DEBUG1("\t key_type: BTM_LE_KEY_PID(%d)", p_data->ble_key.key_type);
            APP_DUMP("IRK", p_data->ble_key.key_value.pid_key.irk, 16);
            APP_DEBUG1("addr_type: 0x%x:", p_data->ble_key.key_value.pid_key.addr_type);
            APP_DEBUG1("static_addr: %02X-%02X-%02X-%02X-%02X-%02X",
                p_data->ble_key.key_value.pid_key.static_addr[0],
                p_data->ble_key.key_value.pid_key.static_addr[1],
                p_data->ble_key.key_value.pid_key.static_addr[2],
                p_data->ble_key.key_value.pid_key.static_addr[3],
                p_data->ble_key.key_value.pid_key.static_addr[4],
                p_data->ble_key.key_value.pid_key.static_addr[5]);
            break;
        case BSA_LE_KEY_PCSRK:
            APP_DEBUG1("\t key_type: BTM_LE_KEY_PCSRK(%d)", p_data->ble_key.key_type);
            APP_DEBUG1("counter: 0x%x:", p_data->ble_key.key_value.pcsrk_key.counter);
            APP_DUMP("CSRK", p_data->ble_key.key_value.pcsrk_key.csrk, 16);
            APP_DEBUG1("sec_level: %d:", p_data->ble_key.key_value.pcsrk_key.sec_level);
            break;
        case BSA_LE_KEY_LCSRK:
            APP_DEBUG1("\t key_type: BTM_LE_KEY_LCSRK(%d)", p_data->ble_key.key_type);
            APP_DEBUG1("counter: 0x%x:", p_data->ble_key.key_value.lcsrk_key.counter);
            APP_DEBUG1("div: %d:", p_data->ble_key.key_value.lcsrk_key.div);
            APP_DEBUG1("sec_level: 0x%x:", p_data->ble_key.key_value.lcsrk_key.sec_level);
            break;
        case BSA_LE_KEY_LENC:
            APP_DEBUG1("\t key_type: BTM_LE_KEY_LENC(%d)", p_data->ble_key.key_type);
            APP_DEBUG1("div: 0x%x:", p_data->ble_key.key_value.lenc_key.div);
            APP_DEBUG1("key_size: %d:", p_data->ble_key.key_value.lenc_key.key_size);
            APP_DEBUG1("sec_level: 0x%x:", p_data->ble_key.key_value.lenc_key.sec_level);
            break;
        case BSA_LE_KEY_LID:
            APP_DEBUG1("\t key_type: BTM_LE_KEY_LID(%d)", p_data->ble_key.key_type);
            break;
        default:
            APP_DEBUG1("\t key_type: Unknown key(%d)", p_data->ble_key.key_type);
            break;
        }

        /* Read the Remote device xml file to have a fresh view */
        app_mgr_read_remote_devices();

        app_xml_update_device_type_db(app_xml_remote_devices_db,
                APP_NUM_ELEMENTS(app_xml_remote_devices_db), p_data->ble_key.bd_addr,
                BT_DEVICE_TYPE_BLE);

        app_xml_update_ble_key_db(app_xml_remote_devices_db,
                APP_NUM_ELEMENTS(app_xml_remote_devices_db),
                p_data->ble_key.bd_addr,
                p_data->ble_key.key_value,
                p_data->ble_key.key_type);

        status = app_mgr_write_remote_devices();
        if (status < 0)
        {
            APP_ERROR1("app_mgr_write_remote_devices failed:%d", status);
        }
        break;

    case BSA_SEC_BLE_PASSKEY_REQ_EVT:
        APP_INFO0("BSA_SEC_BLE_PASSKEY_REQ_EVT (Passkey Request) received from:");
        APP_INFO1("\tbd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->ble_passkey_req.bd_addr[0], p_data->ble_passkey_req.bd_addr[1],
                p_data->ble_passkey_req.bd_addr[2], p_data->ble_passkey_req.bd_addr[3],
                p_data->ble_passkey_req.bd_addr[4], p_data->ble_passkey_req.bd_addr[5]);

        memset(&g_pin_code_reply, 0, sizeof(g_pin_code_reply));
        bdcpy(g_pin_code_reply.bd_addr, p_data->pin_req.bd_addr);
        break;
#endif

    case BSA_SEC_RSSI_EVT:
        APP_INFO0("BSA_SEC_RSSI_EVT received");
        APP_INFO1("\tbd_addr: %02x:%02x:%02x:%02x:%02x:%02x",
                p_data->sig_strength.bd_addr[0], p_data->sig_strength.bd_addr[1],
                p_data->sig_strength.bd_addr[2], p_data->sig_strength.bd_addr[3],
                p_data->sig_strength.bd_addr[4], p_data->sig_strength.bd_addr[5]);
        APP_INFO1("\tRSSI: %d", p_data->sig_strength.rssi_value);
        break;

    default:
        APP_ERROR1("unknown event:%d", event);
        break;
    }

    /* forward the callback to registered app */
    if(s_pCallback){
        s_pCallback(event, p_data);
    }
}

/*******************************************************************************
 **
 ** Function         app_mgr_send_pincode
 **
 ** Description      Sends simple pairing passkey to server
 **
 ** Parameters       passkey
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_send_pincode(tBSA_SEC_PIN_CODE_REPLY pin_code_reply)
{
    int status;
    bdcpy(pin_code_reply.bd_addr, g_pin_code_reply.bd_addr);
    status = BSA_SecPinCodeReply(&pin_code_reply);
    if (status)
    {
        APP_ERROR1("BSA_SecPinCodeReply ERROR:%d", status);
    }
}


/*******************************************************************************
 **
 ** Function         app_mgr_send_passkey
 **
 ** Description      Sends simple pairing passkey to server
 **
 ** Parameters       passkey
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_send_passkey(UINT32 passkey)
{
    g_passkey_reply.passkey = passkey;
    BSA_SecSpPasskeyReply(&g_passkey_reply);
}

/*******************************************************************************
 **
 ** Function         app_mgr_sec_set_sp_cap
 **
 ** Description      Set simple pairing capabilities
 **
 ** Parameters       sp_cap: simple pairing capabilities
 **
 ** Returns          status
 **
 *******************************************************************************/
int app_mgr_sec_set_sp_cap(tBSA_SEC_IO_CAP sp_cap)
{
    int status;
    tBSA_SEC_SET_SECURITY set_security;

    BSA_SecSetSecurityInit(&set_security);
    set_security.simple_pairing_io_cap = sp_cap;
    set_security.sec_cback = app_mgr_security_callback;
    status = BSA_SecSetSecurity(&set_security);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_SecSetSecurity failed:%d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         app_mgr_sec_set_security
 **
 ** Description      Security configuration
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_sec_set_security(void)
{
    int status;
    tBSA_SEC_SET_SECURITY set_security;

    APP_DEBUG0("");

    BSA_SecSetSecurityInit(&set_security);
    set_security.simple_pairing_io_cap = app_xml_config.io_cap;
    set_security.sec_cback = app_mgr_security_callback;
    status = BSA_SecSetSecurity(&set_security);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_SecSetSecurity failed:%d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         app_mgr_sec_set_callback
 **
 ** Description      Security callback
 **
 ** Parameters       tBSA_SEC_CBACK callback
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_sec_set_callback(tBSA_SEC_CBACK pcb)
{
    s_pCallback = pcb;
}


/*******************************************************************************
 **
 ** Function         app_mgr_sp_cfm_reply
 **
 ** Description      Function used to accept/refuse Simple Pairing
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_sp_cfm_reply(BOOLEAN accept, BD_ADDR bd_addr)
{
    int status;
    tBSA_SEC_SP_CFM_REPLY app_sec_sp_cfm_reply;

    APP_DEBUG0("");

    BSA_SecSpCfmReplyInit(&app_sec_sp_cfm_reply);
    app_sec_sp_cfm_reply.accept = accept;
    bdcpy(app_sec_sp_cfm_reply.bd_addr, bd_addr);
    status = BSA_SecSpCfmReply(&app_sec_sp_cfm_reply);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_SecSpCfmReply failed:%d", status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         app_mgr_sec_bond
 **
 ** Description      Bond a device
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_sec_bond(void)
{
    int status;
    int device_index;
    tBSA_SEC_BOND sec_bond;

    APP_INFO0("Bluetooth Bond menu:");
    app_disc_display_devices();
    device_index = app_get_choice("Select device");

    if ((device_index >= 0) &&
        (device_index < APP_DISC_NB_DEVICES) &&
        (app_discovery_cb.devs[device_index].in_use != FALSE))
    {
        BSA_SecBondInit(&sec_bond);
        bdcpy(sec_bond.bd_addr, app_discovery_cb.devs[device_index].device.bd_addr);
        status = BSA_SecBond(&sec_bond);
        if (status != BSA_SUCCESS)
        {
            APP_ERROR1("BSA_SecBond failed:%d", status);
            return -1;
        }
    }
    else
    {
        APP_ERROR1("Bad device number:%d", device_index);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_mgr_sec_bond_cancel
 **
 ** Description      Cancel a bonding procedure
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_sec_bond_cancel(void)
{
    int status;
    int device_index;
    tBSA_SEC_BOND_CANCEL sec_bond_cancel;

    APP_INFO0("Bluetooth Bond Cancel menu:");
    app_disc_display_devices();
    device_index = app_get_choice("Select device");

    if ((device_index >= 0) &&
        (device_index < APP_DISC_NB_DEVICES) &&
        (app_discovery_cb.devs[device_index].in_use != FALSE))
    {
        BSA_SecBondCancelInit(&sec_bond_cancel);
        bdcpy(sec_bond_cancel.bd_addr, app_discovery_cb.devs[device_index].device.bd_addr);
        status = BSA_SecBondCancel(&sec_bond_cancel);
        if (status != BSA_SUCCESS)
        {
            APP_ERROR1("BSA_SecBondCancel failed:%d", status);
            return -1;
        }
    }
    else
    {
        APP_ERROR1("Bad device number:%d", device_index);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_mgr_sec_unpair
 **
 ** Description      Unpair a device
 **
 ** Parameters
 **
 ** Returns          0 if success / -1 if error
 **
 *******************************************************************************/
int app_mgr_sec_unpair(void)
{
    int status;
    int device_index;
    tBSA_SEC_REMOVE_DEV sec_remove;

    APP_INFO0("app_mgr_sec_unpair");

    /* Remove the device from Security database (BSA Server) */
    BSA_SecRemoveDeviceInit(&sec_remove);

    /* Read the XML file which contains all the bonded devices */
    app_read_xml_remote_devices();

    /* Display them */
    app_xml_display_devices(app_xml_remote_devices_db,
            APP_MAX_NB_REMOTE_STORED_DEVICES);

    device_index = app_get_choice("Select device to unpair");
    if ((device_index >= 0) &&
        (device_index < APP_MAX_NB_REMOTE_STORED_DEVICES) &&
        (app_xml_remote_devices_db[device_index].in_use != FALSE))
    {
        bdcpy(sec_remove.bd_addr, app_xml_remote_devices_db[device_index].bd_addr);
        status = BSA_SecRemoveDevice(&sec_remove);
    }
    else
    {
        APP_ERROR1("Wrong index [%d]",device_index);
        return -1;
    }

    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_SecRemoveDevice Operation Failed with status [%d]",status);

        return -1;
    }
    else
    {
        /* delete the device from database */
        app_xml_remote_devices_db[device_index].in_use = FALSE;
        app_write_xml_remote_devices();
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_mgr_set_discoverable
 **
 ** Description      Set the device discoverable for a specific time
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_set_discoverable(void)
{
    tBSA_DM_SET_CONFIG bsa_dm_set_config;
    if (BSA_DmSetConfigInit(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfigInit failed");
        return;
    }
    bsa_dm_set_config.discoverable = 1;
    bsa_dm_set_config.config_mask = BSA_DM_CONFIG_VISIBILITY_MASK;
    if (BSA_DmSetConfig(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfig failed");
        return;
    }
}

/*******************************************************************************
 **
 ** Function         app_mgr_set_non_discoverable
 **
 ** Description      Set the device non discoverable
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_set_non_discoverable(void)
{
    tBSA_DM_SET_CONFIG bsa_dm_set_config;
    if (BSA_DmSetConfigInit(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfigInit failed");
        return;
    }
    bsa_dm_set_config.discoverable = 0;
    bsa_dm_set_config.config_mask = BSA_DM_CONFIG_VISIBILITY_MASK;
    if (BSA_DmSetConfig(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfig failed");
        return;
    }
}

/*******************************************************************************
 **
 ** Function         app_mgr_set_connectable
 **
 ** Description      Set the device connectable for a specific time
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_set_connectable(void)
{
    tBSA_DM_SET_CONFIG bsa_dm_set_config;
    if (BSA_DmSetConfigInit(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfigInit failed");
        return;
    }
    bsa_dm_set_config.connectable = 1;
    bsa_dm_set_config.config_mask = BSA_DM_CONFIG_VISIBILITY_MASK;
    if (BSA_DmSetConfig(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfig failed");
        return;
    }
}

/*******************************************************************************
 **
 ** Function         app_mgr_set_non_connectable
 **
 ** Description      Set the device non connectable
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_set_non_connectable(void)
{
    tBSA_DM_SET_CONFIG bsa_dm_set_config;
    if (BSA_DmSetConfigInit(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfigInit failed");
        return;
    }
    bsa_dm_set_config.connectable = 0;
    bsa_dm_set_config.config_mask = BSA_DM_CONFIG_VISIBILITY_MASK;
    if (BSA_DmSetConfig(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfig failed");
        return;
    }
}

/*******************************************************************************
 **
 ** Function         app_mgr_set_bd_name
 **
 ** Description      Set the device bt name
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
 void app_mgr_set_bd_name(const char *bd_name)
{
    if(!bd_name || !bd_name[0])
    {
        APP_ERROR0("set bt NULL name failed");
        return;
    }

    strncpy((char *)app_xml_config.name, (char *)bd_name, BD_NAME_LEN);
    app_xml_config.name[sizeof(app_xml_config.name) - 1] = '\0';

    app_mgr_set_bt_config(TRUE);
    app_mgr_write_config();
}


/*******************************************************************************
 **
 ** Function         app_mgr_config_bd_name
 **
 ** Description      config the device bt name
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
 void app_mgr_config_bd_name(const char *bd_name)
{
    if(!bd_name || !bd_name[0])
    {
        APP_ERROR0("set bt NULL name failed");
        return;
    }

    strncpy((char *)app_xml_config.name, (char *)bd_name, BD_NAME_LEN);
    app_xml_config.name[sizeof(app_xml_config.name) - 1] = '\0';
}

void app_mgr_config_connectable(int enable)
{
    if(enable != 0){
        app_xml_config.connectable = 1;
    }else{
        app_xml_config.connectable = 0;
    }

    return ;
}


void app_mgr_config_discoverable(int enable)
{
    if(enable != 0){
        app_xml_config.discoverable = 1;
    }else{
        app_xml_config.discoverable = 0;
    }

    return ;
}

/*******************************************************************************
 **
 ** Function         app_mgr_di_discovery
 **
 ** Description      Perform a device Id discovery
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int app_mgr_di_discovery(void)
{
    int index, status;
    BD_ADDR bd_addr;

    do
    {
        APP_INFO0("Bluetooth Manager DI Discovery menu:");
        APP_INFO0("    0 Device from XML database (previously paired)");
        APP_INFO0("    1 Device found in last discovery");
        index = app_get_choice("Select source");
        if (index == 0)
        {
            /* Read the Remote device xml file to have a fresh view */
            app_mgr_read_remote_devices();

            app_xml_display_devices(app_xml_remote_devices_db, APP_NUM_ELEMENTS(app_xml_remote_devices_db));
            index = app_get_choice("Select device");
            if ((index >= 0) &&
                (index < APP_NUM_ELEMENTS(app_xml_remote_devices_db)) &&
                (app_xml_remote_devices_db[index].in_use != FALSE))
            {
                bdcpy(bd_addr, app_xml_remote_devices_db[index].bd_addr);
            }
            else
            {
                break;
            }
        }
        /* Devices from Discovery */
        else if (index == 1)
        {
            app_disc_display_devices();
            index = app_get_choice("Select device");
            if ((index >= 0) &&
                (index < APP_DISC_NB_DEVICES) &&
                (app_discovery_cb.devs[index].in_use != FALSE))
            {
                bdcpy(bd_addr, app_discovery_cb.devs[index].device.bd_addr);
            }
            else
            {
                break;
            }
        }
        else
        {
            break;
        }

        /* Start Discovery Device Info */
        status = app_disc_start_dev_info(bd_addr, NULL);
        return status;
    } while (0);

    APP_ERROR1("Bad Device index:%d", index);
    return -1;
}

int app_mgr_config_init(void)
{
    int                 status;

    status = app_mgr_read_config();
    if (status < 0)
    {
        strncpy((char *)app_xml_config.name, APP_DEFAULT_BT_NAME, sizeof(app_xml_config.name));
        app_xml_config.name[sizeof(app_xml_config.name) - 1] = '\0';
    }

    /* prevention config file damanged */
    {
        app_xml_config.enable = TRUE;
        app_xml_config.discoverable = TRUE;
        app_xml_config.connectable = TRUE;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_mgr_config
 **
 ** Description      Configure the BSA server
 **
 ** Parameters       None
 **
 ** Returns          Status of the operation
 **
 *******************************************************************************/
int app_mgr_config(void)
{
    int                 status;
    int                 index;
    BD_ADDR             local_bd_addr = APP_DEFAULT_BD_ADDR;
    BD_ADDR             chip_bd_addr = {0};
    DEV_CLASS           local_class_of_device = APP_DEFAULT_CLASS_OF_DEVICE;
    tBSA_SEC_ADD_DEV    bsa_add_dev_param;
    tBSA_SEC_ADD_SI_DEV bsa_add_si_dev_param;
    struct timeval      tv;
    unsigned int        rand_seed;

    /*
     * The rest of the initialization function must be done
     * for application boot and when Bluetooth is restarted
     */

    /* prevention config file damanged */
    {
        APP_ERROR0("Creating default XML config file");

#if defined(CUSTOM_MAC_ENABLE) && !defined(RANDOM_MAC_ENABLE)

    /*==========================CUSTOM_MAC_ENABLE===================*/
        /* bt config file exist */
        if(bta_conf_path[0] != '\0') {
            bta_load_addr((const char *)bta_conf_path);
            if((local_device_set_addr[0] != 0) || (local_device_set_addr[1] != 0)
                || (local_device_set_addr[2] != 0) || (local_device_set_addr[3] != 0)
                || (local_device_set_addr[3] != 0) || (local_device_set_addr[4] != 0)
                || (local_device_set_addr[5] != 0)) {
                bdcpy(app_xml_config.bd_addr, local_device_set_addr);
            } else {
                bdcpy(app_xml_config.bd_addr, local_bd_addr);
                /* let's use a random number for the last two bytes of the BdAddr */
                gettimeofday(&tv, NULL);
                rand_seed = tv.tv_sec * tv.tv_usec * getpid();
                app_xml_config.bd_addr[4] = rand_r(&rand_seed);
                app_xml_config.bd_addr[5] = rand_r(&rand_seed);
            }
        }
        else {
            bdcpy(app_xml_config.bd_addr, local_bd_addr);
            /* let's use a random number for the last two bytes of the BdAddr */
            gettimeofday(&tv, NULL);
            rand_seed = tv.tv_sec * tv.tv_usec * getpid();
            app_xml_config.bd_addr[4] = rand_r(&rand_seed);
            app_xml_config.bd_addr[5] = rand_r(&rand_seed);
        }

#elif defined(RANDOM_MAC_ENABLE) && !defined(CUSTOM_MAC_ENABLE)

     /*==========RANDOM_MAC_ENABLE====================*/
        bdcpy(app_xml_config.bd_addr, local_bd_addr);
        /* let's use a random number for the last two bytes of the BdAddr */
        gettimeofday(&tv, NULL);
        rand_seed = tv.tv_sec * tv.tv_usec * getpid();
        app_xml_config.bd_addr[4] = rand_r(&rand_seed);
        app_xml_config.bd_addr[5] = rand_r(&rand_seed);

#else

      /* Default action: to set bd_addr from chip setup*/
        status = app_mgr_get_bd_addr(chip_bd_addr);
	if(status < 0)
	{
            APP_ERROR0("Unable to get the bd_addr from chip!");
	}
        if((chip_bd_addr[0] != 0) || (chip_bd_addr[1] != 0)
           || (chip_bd_addr[2] != 0) || (chip_bd_addr[3] != 0)
           || (chip_bd_addr[3] != 0) || (chip_bd_addr[4] != 0)
           || (chip_bd_addr[5] != 0))
	{
	    bdcpy(app_xml_config.bd_addr, chip_bd_addr);
	}
	else
	{
            APP_ERROR0("There is something wrong in the bd_addr from chip!");
	    return status;
	}

#endif

        memcpy(app_xml_config.class_of_device, local_class_of_device, sizeof(DEV_CLASS));
        strncpy(app_xml_config.root_path, APP_DEFAULT_ROOT_PATH, sizeof(app_xml_config.root_path));
        app_xml_config.root_path[sizeof(app_xml_config.root_path) - 1] = '\0';
        strncpy((char *)app_xml_config.pin_code, APP_DEFAULT_PIN_CODE, APP_DEFAULT_PIN_LEN);

        /* The following code will not work if APP_DEFAULT_PIN_LEN is 16 bytes */
        app_xml_config.pin_code[APP_DEFAULT_PIN_LEN] = '\0';
        app_xml_config.pin_len = APP_DEFAULT_PIN_LEN;
        app_xml_config.io_cap = APP_SEC_IO_CAPABILITIES;

        status = app_mgr_write_config();
        if (status < 0)
        {
            APP_ERROR0("Unable to Create default XML config file");
            app_mgt_close();
            return status;
        }
    }

    /* Example of function to read the database of remote devices */
    status = app_mgr_read_remote_devices();
    if (status < 0)
    {
        APP_ERROR0("No remote device database found");
    }
    else
    {
        app_xml_display_devices(app_xml_remote_devices_db,
                APP_NUM_ELEMENTS(app_xml_remote_devices_db));
    }

     /* Example of function to get the Local Bluetooth configuration */
    app_mgr_get_bt_config();

    /* Example of function to set the Local Bluetooth configuration */
    app_mgr_set_bt_config(app_xml_config.enable);

    /* Example of function to set the Bluetooth Security */
    status = app_mgr_sec_set_security();
    if (status < 0)
    {
        APP_ERROR1("app_mgr_sec_set_security failed:%d", status);
        app_mgt_close();
        return status;
    }

    /* Add every devices found in remote device database */
    /* They will be able to connect to our device */
    APP_INFO0("Add all devices found in database");
    for (index = 0 ; index < APP_NUM_ELEMENTS(app_xml_remote_devices_db) ; index++)
    {
        if (app_xml_remote_devices_db[index].in_use != FALSE)
        {
            APP_INFO1("Adding:%s", app_xml_remote_devices_db[index].name);
            BSA_SecAddDeviceInit(&bsa_add_dev_param);
            bdcpy(bsa_add_dev_param.bd_addr,
                    app_xml_remote_devices_db[index].bd_addr);
            memcpy(bsa_add_dev_param.class_of_device,
                    app_xml_remote_devices_db[index].class_of_device,
                    sizeof(DEV_CLASS));
            memcpy(bsa_add_dev_param.link_key,
                    app_xml_remote_devices_db[index].link_key,
                    sizeof(LINK_KEY));
            bsa_add_dev_param.link_key_present = app_xml_remote_devices_db[index].link_key_present;
            bsa_add_dev_param.trusted_services = app_xml_remote_devices_db[index].trusted_services;
            bsa_add_dev_param.is_trusted = TRUE;
            bsa_add_dev_param.key_type = app_xml_remote_devices_db[index].key_type;
            bsa_add_dev_param.io_cap = app_xml_remote_devices_db[index].io_cap;
#if (defined(BLE_INCLUDED) && BLE_INCLUDED == TRUE)
            bsa_add_dev_param.ble_addr_type = app_xml_remote_devices_db[index].ble_addr_type;
            bsa_add_dev_param.device_type = app_xml_remote_devices_db[index].device_type;
            bsa_add_dev_param.inq_result_type = app_xml_remote_devices_db[index].inq_result_type;
            if(app_xml_remote_devices_db[index].ble_link_key_present)
            {
                bsa_add_dev_param.ble_link_key_present = TRUE;
                /* Fill PENC Key */
                memcpy(bsa_add_dev_param.le_penc_key.ltk,
                    app_xml_remote_devices_db[index].penc_ltk,
                    sizeof(app_xml_remote_devices_db[index].penc_ltk));
                memcpy(bsa_add_dev_param.le_penc_key.rand,
                    app_xml_remote_devices_db[index].penc_rand,
                    sizeof(app_xml_remote_devices_db[index].penc_rand));
                bsa_add_dev_param.le_penc_key.ediv = app_xml_remote_devices_db[index].penc_ediv;
                bsa_add_dev_param.le_penc_key.sec_level = app_xml_remote_devices_db[index].penc_sec_level;
                bsa_add_dev_param.le_penc_key.key_size = app_xml_remote_devices_db[index].penc_key_size;
                /* Fill PID Key */
                memcpy(bsa_add_dev_param.le_pid_key.irk,
                    app_xml_remote_devices_db[index].pid_irk,
                    sizeof(app_xml_remote_devices_db[index].pid_irk));
                bsa_add_dev_param.le_pid_key.addr_type = app_xml_remote_devices_db[index].pid_addr_type;
                memcpy(bsa_add_dev_param.le_pid_key.static_addr,
                    app_xml_remote_devices_db[index].pid_static_addr,
                    sizeof(app_xml_remote_devices_db[index].pid_static_addr));
                /* Fill PCSRK Key */
                bsa_add_dev_param.le_pcsrk_key.counter = app_xml_remote_devices_db[index].pcsrk_counter;
                memcpy(bsa_add_dev_param.le_pcsrk_key.csrk,
                    app_xml_remote_devices_db[index].pcsrk_csrk,
                    sizeof(app_xml_remote_devices_db[index].pcsrk_csrk));
                bsa_add_dev_param.le_pcsrk_key.sec_level = app_xml_remote_devices_db[index].pcsrk_sec_level;
                /* Fill LCSRK Key */
                bsa_add_dev_param.le_lcsrk_key.counter = app_xml_remote_devices_db[index].lcsrk_counter;
                bsa_add_dev_param.le_lcsrk_key.div = app_xml_remote_devices_db[index].lcsrk_div;
                bsa_add_dev_param.le_lcsrk_key.sec_level = app_xml_remote_devices_db[index].lcsrk_sec_level;
                /* Fill LENC Key */
                bsa_add_dev_param.le_lenc_key.div = app_xml_remote_devices_db[index].lenc_div;
                bsa_add_dev_param.le_lenc_key.key_size = app_xml_remote_devices_db[index].lenc_key_size;
                bsa_add_dev_param.le_lenc_key.sec_level = app_xml_remote_devices_db[index].lenc_sec_level;
            }
#endif
            BSA_SecAddDevice(&bsa_add_dev_param);
        }
    }

    /* Add stored SI devices to BSA server */
    app_read_xml_si_devices();
    for (index = 0 ; index < APP_NUM_ELEMENTS(app_xml_si_devices_db) ; index++)
    {
        if (app_xml_si_devices_db[index].in_use)
        {
            BSA_SecAddSiDevInit(&bsa_add_si_dev_param);
            bdcpy(bsa_add_si_dev_param.bd_addr, app_xml_si_devices_db[index].bd_addr);
            bsa_add_si_dev_param.platform = app_xml_si_devices_db[index].platform;
            BSA_SecAddSiDev(&bsa_add_si_dev_param);
        }
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_mgr_change_dual_stack_mode
 **
 ** Description     Toggle Dual Stack Mode (BSA <=> MM/Kernel)
 **
 ** Parameters      None
 **
 ** Returns         Status
 **
 *******************************************************************************/
int app_mgr_change_dual_stack_mode(void)
{
    int choice;
    tBSA_DM_DUAL_STACK_MODE dual_stack_mode;

    APP_INFO1("Current DualStack mode: %s", app_mgr_get_dual_stack_mode_desc());
    APP_INFO0("Select the new DualStack mode:");
    APP_INFO0("\t0: DUAL_STACK_MODE_BSA");
    APP_INFO0("\t1: DUAL_STACK_MODE_MM/Kernel (requires specific BTUSB driver)");
    APP_INFO0("\t2: DUAL_STACK_MODE_BTC (requires specific FW)");
    choice = app_get_choice("Choice");

    switch(choice)
    {
    case 0:
        APP_INFO0("Switching Dual Stack to BSA");
        dual_stack_mode = BSA_DM_DUAL_STACK_MODE_BSA;
        break;

    case 1:
        APP_INFO0("Switching Dual Stack to MM/Kernel");
        dual_stack_mode = BSA_DM_DUAL_STACK_MODE_MM;
        break;

    case 2:
        APP_INFO0("Switching Dual Stack to BTC");
        dual_stack_mode = BSA_DM_DUAL_STACK_MODE_BTC;
        break;

    default:
        APP_ERROR1("Bad DualStack Mode=%d", choice);
        return -1;
    }

    if (app_dm_set_dual_stack_mode(dual_stack_mode) < 0)
    {
        APP_ERROR0("DualStack Switch Failed");
        return -1;
    }

    app_mgr_cb.dual_stack_mode = dual_stack_mode;

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_mgr_get_dual_stack_mode_desc
 **
 ** Description     Get Dual Stack Mode description
 **
 ** Parameters      None
 **
 ** Returns         Description string
 **
 *******************************************************************************/
char *app_mgr_get_dual_stack_mode_desc(void)
{
    switch(app_mgr_cb.dual_stack_mode)
    {
    case BSA_DM_DUAL_STACK_MODE_BSA:
        return "DUAL_STACK_MODE_BSA";
    case BSA_DM_DUAL_STACK_MODE_MM:
        return "DUAL_STACK_MODE_MM/Kernel";
    case BSA_DM_DUAL_STACK_MODE_BTC:
        return "DUAL_STACK_MODE_BTC";
    default:
        return "Unknown Dual Stack Mode";
    }
}

/*******************************************************************************
 **
 ** Function         app_mgr_discovery_test
 **
 ** Description      This function performs a specific discovery
 **
 ** Parameters       None
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_mgr_discovery_test(void)
{
    int choice;

    APP_INFO0("Select the parameter to modify:");
    APP_INFO0("\t0: Update type");
    APP_INFO0("\t1: Inquiry TX power");
    choice = app_get_choice("Choice");

    switch(choice)
    {
    case 0:
        APP_INFO0("Select which update is needed:");
        APP_INFO1("\t%d: End of discovery (default)", BSA_DISC_UPDATE_END);
        APP_INFO1("\t%d: Upon any modification", BSA_DISC_UPDATE_ANY);
        APP_INFO1("\t%d: Upon Name Reception", BSA_DISC_UPDATE_NAME);
        choice = app_get_choice("");

        return app_disc_start_update((tBSA_DISC_UPDATE)choice);
        break;

    case 1:
        choice = app_get_choice("Enter new Inquiry TX power");

        return app_disc_start_power((INT8)choice);

        break;

    default:
        APP_ERROR0("Unsupported choice value");
        break;
    }
    return -1;
}

/*******************************************************************************
 **
 ** Function         app_mgr_read_version
 **
 ** Description      This function is used to read BSA and FW version
 **
 ** Parameters
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_mgr_read_version(void)
{
    tBSA_TM_READ_VERSION bsa_read_version;
    tBSA_STATUS bsa_status;

    bsa_status = BSA_TmReadVersionInit(&bsa_read_version);
    bsa_status = BSA_TmReadVersion(&bsa_read_version);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmReadVersion failed status:%d", bsa_status);
        return(-1);
    }

    APP_INFO1("Server status:%d", bsa_read_version.status);
    APP_INFO1("FW Version:%d.%d.%d.%d",
            bsa_read_version.fw_version.major,
            bsa_read_version.fw_version.minor,
            bsa_read_version.fw_version.build,
            bsa_read_version.fw_version.config);
    APP_INFO1("BSA Server Version:%s", bsa_read_version.bsa_server_version);
    return 0;
}


/*******************************************************************************
 **
 ** Function         app_mgr_set_link_policy
 **
 ** Description      Set the device link policy
 **                  This function sets/clears the link policy mask to the given
 **                  bd_addr.
 **                  If clearing the sniff or park mode mask, the link is put
 **                  in active mode.
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_mgr_set_link_policy(BD_ADDR bd_addr, tBSA_DM_LP_MASK policy_mask, BOOLEAN set)
{
    tBSA_DM_SET_CONFIG bsa_dm_set_config;
    if (BSA_DmSetConfigInit(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfigInit failed");
        return;
    }

    bsa_dm_set_config.config_mask = BSA_DM_CONFIG_LINK_POLICY_MASK;

    bdcpy(bsa_dm_set_config.policy_param.link_bd_addr, bd_addr);
    bsa_dm_set_config.policy_param.policy_mask = policy_mask;
    bsa_dm_set_config.policy_param.set = set;

    if (BSA_DmSetConfig(&bsa_dm_set_config) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_DmSetConfig failed");
        return;
    }
}
