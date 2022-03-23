/*****************************************************************************
**
**  Name:           app_ble.c
**
**  Description:    Bluetooth BLE application
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include "app_ble.h"
#include "app_xml_utils.h"
#include "app_utils.h"
#include "app_mgt.h"
#include "app_disc.h"
#include "app_dm.h"
#include "app_ble_client_db.h"
#include "app_ble_client.h"
#include "app_ble_server.h"

/*
 * Global Variables
 */
tAPP_BLE_CB app_ble_cb;

/*
 * Local functions
 */


/*
 * BLE common functions
 */


/*
 * Defines
 */
#define APP_BLE_ENABLE_WAKE_GPIO_VSC 0xFD60
#define APP_BLE_PF_MANU_DATA_CO_ID   0x000F  /* Company ID for BRCM */
#define APP_BLE_PF_MANU_DATA_COND_TYPE 5

/*******************************************************************************
 **
 ** Function         app_av_display_vendor_commands
 **
 ** Description      This function display the name of vendor dependent command
 **
 ** Returns          void
 **
 *******************************************************************************/
char *app_ble_display_service_name(UINT16 uuid)
{
    switch(uuid)
    {
    case BSA_BLE_UUID_SERVCLASS_GAP_SERVER:
        return ("UUID_SERVCLASS_GAP_SERVER");
    case BSA_BLE_UUID_SERVCLASS_GATT_SERVER:
        return ("UUID_SERVCLASS_GATT_SERVER");
    case BSA_BLE_UUID_SERVCLASS_IMMEDIATE_ALERT:
        return("UUID_SERVCLASS_IMMEDIATE_ALERT");
    case BSA_BLE_UUID_SERVCLASS_LINKLOSS:
        return ("UUID_SERVCLASS_LINKLOSS");
    case BSA_BLE_UUID_SERVCLASS_TX_POWER:
        return ("UUID_SERVCLASS_TX_POWER");
    case BSA_BLE_UUID_SERVCLASS_CURRENT_TIME:
        return ("UUID_SERVCLASS_CURRENT_TIME");
    case BSA_BLE_UUID_SERVCLASS_DST_CHG:
        return ("UUID_SERVCLASS_DST_CHG");
    case BSA_BLE_UUID_SERVCLASS_REF_TIME_UPD:
        return ("UUID_SERVCLASS_REF_TIME_UPD");
    case BSA_BLE_UUID_SERVCLASS_GLUCOSE:
        return ("UUID_SERVCLASS_GLUCOSE");
    case BSA_BLE_UUID_SERVCLASS_HEALTH_THERMOMETER:
        return ("UUID_SERVCLASS_HEALTH_THERMOMETER");
    case BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION:
        return ("UUID_SERVCLASS_DEVICE_INFORMATION");
    case BSA_BLE_UUID_SERVCLASS_NWA:
        return ("UUID_SERVCLASS_NWA");
    case BSA_BLE_UUID_SERVCLASS_PHALERT:
        return ("UUID_SERVCLASS_PHALERT");
    case BSA_BLE_UUID_SERVCLASS_HEART_RATE:
        return ("UUID_SERVCLASS_HEART_RATE");
    case BSA_BLE_UUID_SERVCLASS_BATTERY_SERVICE:
        return ("UUID_SERVCLASS_BATTERY_SERVICE");
    case BSA_BLE_UUID_SERVCLASS_BLOOD_PRESSURE:
        return ("UUID_SERVCLASS_BLOOD_PRESSURE");
    case BSA_BLE_UUID_SERVCLASS_ALERT_NOTIFICATION_SERVICE:
        return ("UUID_SERVCLASS_ALERT_NOTIFICATION_SERVICE");
    case BSA_BLE_UUID_SERVCLASS_HUMAN_INTERFACE_DEVICE:
        return ("UUID_SERVCLASS_HUMAN_INTERFACE_DEVICE");
    case BSA_BLE_UUID_SERVCLASS_SCAN_PARAMETERS:
        return ("UUID_SERVCLASS_SCAN_PARAMETERS");
    case BSA_BLE_UUID_SERVCLASS_RUNNING_SPEED_AND_CADENCE:
        return ("UUID_SERVCLASS_RUNNING_SPEED_AND_CADENCE");
    case BSA_BLE_UUID_SERVCLASS_CYCLING_SPEED_AND_CADENCE:
        return ("UUID_SERVCLASS_CYCLING_SPEED_AND_CADENCE");
    case BSA_BLE_UUID_SERVCLASS_TEST_SERVER:
        return ("UUID_SERVCLASS_TEST_SERVER");

    default:
        break;
    }

    return ("UNKNOWN");
}

/*******************************************************************************
 **
 ** Function        app_ble_init
 **
 ** Description     This is the main init function
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_init(void)
{
    int index;

    memset(&app_ble_cb, 0, sizeof(app_ble_cb));

    for (index = 0;index < BSA_BLE_CLIENT_MAX;index++)
    {
        app_ble_cb.ble_client[index].client_if = BSA_BLE_INVALID_IF;
        app_ble_cb.ble_client[index].conn_id = BSA_BLE_INVALID_CONN;
    }

    for (index = 0;index < BSA_BLE_SERVER_MAX;index++)
    {
        app_ble_cb.ble_server[index].server_if = BSA_BLE_INVALID_IF;
        app_ble_cb.ble_server[index].conn_id = BSA_BLE_INVALID_CONN;
    }

    app_xml_init();

    app_ble_client_db_init();

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_exit
 **
 ** Description     This is the ble exit function
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_exit(void)
{
    tBSA_STATUS status;
    tBSA_BLE_DISABLE param;

    /* make device non discoverable & non connectable */
    app_dm_set_ble_visibility(FALSE, FALSE);

    /* Deregister all applications */
    app_ble_client_deregister_all();

    BSA_BleDisableInit(&param);

    status = BSA_BleDisable(&param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to disable BLE status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_start
 **
 ** Description     DTV Start function
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_start(void)
{
    tBSA_STATUS status;
    tBSA_BLE_ENABLE enable_param;

    APP_INFO0("app_ble_start");

    BSA_BleEnableInit(&enable_param);

    status = BSA_BleEnable(&enable_param);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to enable BLE status:%d", status);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_ble_wake_configure
 **
 ** Description     Configure for Wake on BLE
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_wake_configure(void)
{
    tBSA_BLE_WAKE_CFG bsa_ble_wake_cfg;
    tBSA_BLE_WAKE_ENABLE bsa_ble_wake_enable;
    tBSA_STATUS bsa_status;
    tBSA_TM_VSC bsa_vsc;
    char manu_pattern[] = "WAKEUP";

    bsa_status = BSA_BleWakeCfgInit(&bsa_ble_wake_cfg);

    bsa_ble_wake_cfg.cond.manu_data.company_id = APP_BLE_PF_MANU_DATA_CO_ID;
    memcpy(bsa_ble_wake_cfg.cond.manu_data.pattern, manu_pattern,sizeof(manu_pattern));
    bsa_ble_wake_cfg.cond.manu_data.data_len = sizeof(manu_pattern) - 1;
    bsa_ble_wake_cfg.cond_type = APP_BLE_PF_MANU_DATA_COND_TYPE;

    bsa_status = BSA_BleWakeCfg(&bsa_ble_wake_cfg);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleWakeCfg failed status:%d", bsa_status);
        return(-1);
    }

    bsa_status = BSA_BleWakeEnableInit(&bsa_ble_wake_enable);
    bsa_ble_wake_enable.enable = TRUE;
    bsa_status = BSA_BleWakeEnable(&bsa_ble_wake_enable);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_BleWake failed status:%d", bsa_status);
        return(-1);
    }

    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = APP_BLE_ENABLE_WAKE_GPIO_VSC;
    bsa_vsc.length = 1;
    bsa_vsc.data[0] = 1; /* 1=ENABLE 0=DISABLE */

    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send Enable Wake on BLE GPIO VSC status:%d", bsa_status);
        return(-1);
    }

    return 0;
}
