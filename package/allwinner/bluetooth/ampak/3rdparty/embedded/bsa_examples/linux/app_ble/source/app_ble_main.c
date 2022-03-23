/*****************************************************************************
**
**  Name:           app_ble_main.c
**
**  Description:    Bluetooth BLE Main application
**
**  Copyright (c) 2015, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#include <stdlib.h>

#include "app_ble.h"
#include "app_utils.h"
#include "app_disc.h"
#include "app_mgt.h"
#include "app_dm.h"
#include "app_ble_client.h"
#include "app_ble_server.h"
#if defined(APP_BLE_OTA_FW_DL_INCLUDED) && (APP_BLE_OTA_FW_DL_INCLUDED == TRUE)
#include "app_ble_client_otafwdl.h"
#endif
/*
 * Defines
 */
#define APP_BLE_MAIN_DEFAULT_APPL_UUID    9000
#define APP_BLE_MAIN_INVALID_UUID         0
#define APP_BLE_ADV_VALUE_LEN             6  /*This is temporary value, Total Adv data including all fields should be <31bytes*/
#define APP_BLE_ADDR_LEN                  20

/* BLE Menu items */
enum
{
    APP_BLE_MENU_ABORT_DISC = 1,
    APP_BLE_MENU_DISCOVERY,
    APP_BLE_MENU_CONFIG_BLE_BG_CONN,
    APP_BLE_MENU_CONFIG_BLE_SCAN_PARAM,
    APP_BLE_MENU_CONFIG_BLE_CONN_PARAM,
    APP_BLE_MENU_CONFIG_BLE_ADV_PARAM,
    APP_BLE_MENU_WAKE_ON_BLE,

    APP_BLECL_MENU_REGISTER,
    APP_BLECL_MENU_DEREGISTER,
    APP_BLECL_MENU_OPEN,
    APP_BLECL_MENU_CLOSE,
    APP_BLECL_MENU_REMOVE,
    APP_BLECL_MENU_READ,
    APP_BLECL_MENU_WRITE,
    APP_BLECL_MENU_SERVICE_DISC,
    APP_BLECL_MENU_REG_FOR_NOTI,
    APP_BLECL_MENU_DEREG_FOR_NOTI,
    APP_BLECL_MENU_DISPLAY_CLIENT,
    APP_BLECL_MENU_SEARCH_DEVICE_INFORMATION_SERVICE,
    APP_BLECL_MENU_READ_MFR_NAME,
    APP_BLECL_MENU_READ_MODEL_NUMBER,
    APP_BLECL_MENU_READ_SERIAL_NUMBER,
    APP_BLECL_MENU_READ_HARDWARE_REVISION,
    APP_BLECL_MENU_READ_FIRMWARE_REVISION,
    APP_BLECL_MENU_READ_SOFTWARE_REVISION,
    APP_BLECL_MENU_READ_SYSTEM_ID,
    APP_BLECL_MENU_SEARCH_BATTERY_SERVICE,
    APP_BLECL_MENU_READ_BATTERY_LEVEL,
#if defined(APP_BLE_OTA_FW_DL_INCLUDED) && (APP_BLE_OTA_FW_DL_INCLUDED == TRUE)
    APP_BLECL_MENU_FW_UPGRADE,
#endif
    APP_BLESE_MENU_REGISTER,
    APP_BLESE_MENU_DEREGISTER,
    APP_BLESE_MENU_OPEN,
    APP_BLESE_MENU_CLOSE,
    APP_BLESE_MENU_CREATE_SERVICE,
    APP_BLESE_MENU_ADD_CHAR,
    APP_BLESE_MENU_START_SERVICE,
    APP_BLESE_MENU_CONFIG_BLE_ADV_DATA,
    APP_BLESE_MENU_DISPLAY_SERVER,
    APP_BLESE_MENU_SEND_IND,

    APP_BLE_MENU_QUIT = 99
};

/*
 * Global Variables
 */


/*
 * Local functions
 */


/*******************************************************************************
 **
 ** Function         app_display_ble_menu
 **
 ** Description      This is the BLE menu
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_display_menu (void)
{
    APP_INFO1("\t%d => Abort Discovery", APP_BLE_MENU_ABORT_DISC);
    APP_INFO1("\t%d => Start BLE Discovery", APP_BLE_MENU_DISCOVERY);
    APP_INFO1("\t%d => Configure BLE Background Connection Type", APP_BLE_MENU_CONFIG_BLE_BG_CONN);
    APP_INFO1("\t%d => Configure BLE Scan Parameter",APP_BLE_MENU_CONFIG_BLE_SCAN_PARAM);
    APP_INFO1("\t%d => Configure BLE Connection Parameter",APP_BLE_MENU_CONFIG_BLE_CONN_PARAM);
    APP_INFO1("\t%d => Configure BLE Advertisement Parameters",APP_BLE_MENU_CONFIG_BLE_ADV_PARAM);
    APP_INFO1("\t%d => Configure for Wake on BLE",APP_BLE_MENU_WAKE_ON_BLE);

    APP_INFO0("\t**** Bluetooth Low Energy Client menu ****");
    APP_INFO1("\t%d => Register client app", APP_BLECL_MENU_REGISTER);
    APP_INFO1("\t%d => Deregister Client app", APP_BLECL_MENU_DEREGISTER);
    APP_INFO1("\t%d => Connect to Server", APP_BLECL_MENU_OPEN);
    APP_INFO1("\t%d => Close Connection", APP_BLECL_MENU_CLOSE);
    APP_INFO1("\t%d => Remove BLE device", APP_BLECL_MENU_REMOVE);
    APP_INFO1("\t%d => Read information", APP_BLECL_MENU_READ);
    APP_INFO1("\t%d => Write information", APP_BLECL_MENU_WRITE);
    APP_INFO1("\t%d => Service Discovery", APP_BLECL_MENU_SERVICE_DISC);
    APP_INFO1("\t%d => Register Notification", APP_BLECL_MENU_REG_FOR_NOTI);
    APP_INFO1("\t%d => Deregister Notification", APP_BLECL_MENU_DEREG_FOR_NOTI);
    APP_INFO1("\t%d => Display Clients", APP_BLECL_MENU_DISPLAY_CLIENT);
    APP_INFO1("\t%d => Search Device Information Service", APP_BLECL_MENU_SEARCH_DEVICE_INFORMATION_SERVICE);
    APP_INFO1("\t%d => Read Manufacturer Name", APP_BLECL_MENU_READ_MFR_NAME);
    APP_INFO1("\t%d => Read Model Number", APP_BLECL_MENU_READ_MODEL_NUMBER);
    APP_INFO1("\t%d => Read Serial Number", APP_BLECL_MENU_READ_SERIAL_NUMBER);
    APP_INFO1("\t%d => Read Hardware Revision", APP_BLECL_MENU_READ_HARDWARE_REVISION);
    APP_INFO1("\t%d => Read Firmware Revision", APP_BLECL_MENU_READ_FIRMWARE_REVISION);
    APP_INFO1("\t%d => Read Software Revision", APP_BLECL_MENU_READ_SOFTWARE_REVISION);
    APP_INFO1("\t%d => Read System ID", APP_BLECL_MENU_READ_SYSTEM_ID);
    APP_INFO1("\t%d => Search Battery Service", APP_BLECL_MENU_SEARCH_BATTERY_SERVICE);
    APP_INFO1("\t%d => Read Battery Level", APP_BLECL_MENU_READ_BATTERY_LEVEL);
#if defined(APP_BLE_OTA_FW_DL_INCLUDED) && (APP_BLE_OTA_FW_DL_INCLUDED == TRUE)
    APP_INFO1("\t%d => Upgrade FW by LE",APP_BLECL_MENU_FW_UPGRADE);
#endif
    APP_INFO0("\t**** Bluetooth Low Energy Server menu ****");
    APP_INFO1("\t%d => Register server app", APP_BLESE_MENU_REGISTER);
    APP_INFO1("\t%d => Deregister server app", APP_BLESE_MENU_DEREGISTER);
    APP_INFO1("\t%d => Connect to client", APP_BLESE_MENU_OPEN);
    APP_INFO1("\t%d => Close connection", APP_BLESE_MENU_CLOSE);
    APP_INFO1("\t%d => Create service", APP_BLESE_MENU_CREATE_SERVICE);
    APP_INFO1("\t%d => Add character", APP_BLESE_MENU_ADD_CHAR);
    APP_INFO1("\t%d => Start service", APP_BLESE_MENU_START_SERVICE);
    APP_INFO1("\t%d => Configure BLE Advertisement data",APP_BLESE_MENU_CONFIG_BLE_ADV_DATA);
    APP_INFO1("\t%d => Display Servers", APP_BLESE_MENU_DISPLAY_SERVER);
    APP_INFO1("\t%d => Send indication", APP_BLESE_MENU_SEND_IND);

    APP_INFO1("\t%d => QUIT", APP_BLE_MENU_QUIT);
}


/*******************************************************************************
 **
 ** Function         app_ble_menu
 **
 ** Description      Handle the BLE menu
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_menu(void)
{
    int choice, type,i;
    UINT16 ble_scan_interval, ble_scan_window;
    tBSA_DM_BLE_CONN_PARAM conn_param;
    tBSA_DM_BLE_ADV_CONFIG adv_conf;
    tBSA_DM_BLE_ADV_PARAM adv_param;
    UINT16 number_of_services;
    UINT8 app_ble_adv_value[APP_BLE_ADV_VALUE_LEN] = {0x2b, 0x1a, 0xaa, 0xbb, 0xcc, 0xdd}; /*First 2 byte is Company Identifier Eg: 0x1a2b refers to Bluetooth ORG, followed by 4bytes of data*/

    do
    {
        app_ble_display_menu();

        choice = app_get_choice("Select action");

        switch(choice)
        {
        case APP_BLE_MENU_ABORT_DISC:
            app_disc_abort();
            break;

        case APP_BLE_MENU_DISCOVERY:
            app_disc_start_ble_regular(NULL);
            break;

        case APP_BLE_MENU_CONFIG_BLE_BG_CONN:
            type = app_get_choice("Select conn type(0 = None, 1 = Auto)");
            if(type == 0 || type == 1)
            {
                app_dm_set_ble_bg_conn_type(type);
            }
            else
            {
                APP_ERROR1("Unknown type:%d", type);
            }
            break;

        case APP_BLE_MENU_CONFIG_BLE_SCAN_PARAM:
            ble_scan_interval = app_get_choice("BLE scan interval(N x 625us)");
            ble_scan_window = app_get_choice("BLE scan window(N x 625us)");
            app_dm_set_ble_scan_param(ble_scan_interval, ble_scan_window);
            break;

        case APP_BLE_MENU_CONFIG_BLE_CONN_PARAM:
            conn_param.min_conn_int = app_get_choice("min_conn_int(N x 1.25 msec)");
            conn_param.max_conn_int = app_get_choice("max_conn_int(N x 1.25 msec)");
            conn_param.slave_latency = app_get_choice("slave_latency");
            conn_param.supervision_tout = app_get_choice("supervision_tout(N x 10 msec)");
            APP_INFO0("Enter the BD address to configure Conn Param (AA.BB.CC.DD.EE.FF): ");
            if (scanf("%hhx.%hhx.%hhx.%hhx.%hhx.%hhx",
                &conn_param.bd_addr[0], &conn_param.bd_addr[1],
                &conn_param.bd_addr[2], &conn_param.bd_addr[3],
                &conn_param.bd_addr[4], &conn_param.bd_addr[5]) != 6)
            {
                APP_ERROR0("BD address not entered correctly");
                break;
            }
            app_dm_set_ble_conn_param(&conn_param);
            break;

        case APP_BLE_MENU_CONFIG_BLE_ADV_PARAM:
        {
            char addrstr[APP_BLE_ADDR_LEN] = {0};
            memset(&adv_param, 0, sizeof(tBSA_DM_BLE_ADV_PARAM));
            adv_param.adv_int_min = app_get_choice("min_adv_int(N x 0.625 msec)");
            adv_param.adv_int_max = app_get_choice("max_adv_int(N x 0.625 msec)");

            APP_INFO0("Enter the BD address to configure Conn Param (AA.BB.CC.DD.EE.FF): ");

            int len = app_get_string("Enter BD Addr for directed advertisement", addrstr, sizeof(addrstr));

            if (len && (sscanf(addrstr, "%hhx.%hhx.%hhx.%hhx.%hhx.%hhx",
                &adv_param.dir_bda.bd_addr[0], &adv_param.dir_bda.bd_addr[1],
                &adv_param.dir_bda.bd_addr[2], &adv_param.dir_bda.bd_addr[3],
                &adv_param.dir_bda.bd_addr[4], &adv_param.dir_bda.bd_addr[5]) != 6))
            {
                APP_ERROR0("BD address not entered correctly");
                break;
            }
            app_dm_set_ble_adv_param(&adv_param);
        }
            break;

        case APP_BLE_MENU_WAKE_ON_BLE:
            app_ble_wake_configure();
            break;

        case APP_BLECL_MENU_REGISTER:
            app_ble_client_register(APP_BLE_MAIN_INVALID_UUID);
            break;

        case APP_BLECL_MENU_OPEN:
            app_ble_client_open();
            break;

        case APP_BLECL_MENU_SERVICE_DISC:
            app_ble_client_service_search();
            break;

        case APP_BLECL_MENU_READ:
            app_ble_client_read();
            break;

        case APP_BLECL_MENU_WRITE:
            app_ble_client_write();
            break;

        case APP_BLECL_MENU_REMOVE:
            app_ble_client_unpair();
            break;

        case APP_BLECL_MENU_REG_FOR_NOTI:
            app_ble_client_register_notification();
            break;

        case APP_BLECL_MENU_CLOSE:
            app_ble_client_close();
            break;

        case APP_BLECL_MENU_DEREGISTER:
            app_ble_client_deregister();
            break;

        case APP_BLECL_MENU_DEREG_FOR_NOTI:
            app_ble_client_deregister_notification();
            break;

        case APP_BLECL_MENU_DISPLAY_CLIENT:
            app_ble_client_display(1);
            break;

        case APP_BLECL_MENU_SEARCH_DEVICE_INFORMATION_SERVICE:
            app_ble_client_service_search_execute(BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION);
            break;

        case APP_BLECL_MENU_READ_MFR_NAME:
            app_ble_client_read_mfr_name();
            break;

        case APP_BLECL_MENU_READ_MODEL_NUMBER:
            app_ble_client_read_model_number();
            break;

        case APP_BLECL_MENU_READ_SERIAL_NUMBER:
            app_ble_client_read_serial_number();
            break;

        case APP_BLECL_MENU_READ_HARDWARE_REVISION:
            app_ble_client_read_hardware_revision();
            break;

        case APP_BLECL_MENU_READ_FIRMWARE_REVISION:
            app_ble_client_read_firmware_revision();
            break;

        case APP_BLECL_MENU_READ_SOFTWARE_REVISION:
            app_ble_client_read_software_revision();
            break;

        case APP_BLECL_MENU_READ_SYSTEM_ID:
            app_ble_client_read_system_id();
            break;

        case APP_BLECL_MENU_SEARCH_BATTERY_SERVICE:
            app_ble_client_service_search_execute(BSA_BLE_UUID_SERVCLASS_BATTERY_SERVICE);
            break;

        case APP_BLECL_MENU_READ_BATTERY_LEVEL:
            app_ble_client_read_battery_level();
            break;
#if (defined(APP_BLE_OTA_FW_DL_INCLUDED) && (APP_BLE_OTA_FW_DL_INCLUDED == TRUE))
        case APP_BLECL_MENU_FW_UPGRADE:
             if(app_ble_client_fw_upgrade()!=0)
             {
                APP_DEBUG0("fw upgrade failed");
             }
             break;
#endif

        case APP_BLESE_MENU_REGISTER:
            app_ble_server_register(APP_BLE_MAIN_INVALID_UUID, NULL);
            break;

        case APP_BLESE_MENU_DEREGISTER:
            app_ble_server_deregister();
            break;

        case APP_BLESE_MENU_OPEN:
            app_ble_server_open();
            break;

        case APP_BLESE_MENU_CLOSE:
            app_ble_server_close();
            break;

        case APP_BLESE_MENU_CREATE_SERVICE:
            app_ble_server_create_service();
            break;

        case APP_BLESE_MENU_ADD_CHAR:
            app_ble_server_add_char();
            break;

        case APP_BLESE_MENU_START_SERVICE:
            app_ble_server_start_service();
            break;

        case APP_BLESE_MENU_CONFIG_BLE_ADV_DATA:
            /* This is just sample code to show how BLE Adv data can be sent from application */
            /*Adv.Data should be < 31bytes including Manufacturer data,Device Name, Appearance data, Services Info,etc.. */
            /* We are not receving all fields from user to reduce the complexity */
            memset(&adv_conf, 0, sizeof(tBSA_DM_BLE_ADV_CONFIG));
           /* start advertising */
           adv_conf.len = APP_BLE_ADV_VALUE_LEN;
           adv_conf.flag = BSA_DM_BLE_ADV_FLAG_MASK;
           memcpy(adv_conf.p_val, app_ble_adv_value, APP_BLE_ADV_VALUE_LEN);
           /* All the masks/fields that are set will be advertised*/
           adv_conf.adv_data_mask = BSA_DM_BLE_AD_BIT_FLAGS|BSA_DM_BLE_AD_BIT_SERVICE|BSA_DM_BLE_AD_BIT_APPEARANCE|BSA_DM_BLE_AD_BIT_MANU;
           adv_conf.appearance_data = app_get_choice("Enter appearance value Eg:0x1122");
           number_of_services = app_get_choice("Enter number of services between <1-6> Eg:2");
           adv_conf.num_service = number_of_services;
           for(i=0; i< adv_conf.num_service; i++)
           {
                adv_conf.uuid_val[i]= app_get_choice("Enter service UUID eg:0xA108");
           }
           adv_conf.is_scan_rsp = app_get_choice("Is this scan response? (0:FALSE, 1:TRUE)");
           app_dm_set_ble_adv_data(&adv_conf);
           break;

        case APP_BLESE_MENU_DISPLAY_SERVER:
            app_ble_server_display();
            break;

        case APP_BLESE_MENU_SEND_IND:
            app_ble_server_send_indication();
            break;

        case APP_BLE_MENU_QUIT:
            APP_INFO0("Quit");
            break;

        default:
            APP_ERROR1("Unknown choice:%d", choice);
            break;
        }
    } while (choice != APP_BLE_MENU_QUIT); /* While user don't exit application */
}


/*******************************************************************************
 **
 ** Function         app_ble_mgt_callback
 **
 ** Description      This callback function is called in case of server
 **                  disconnection (e.g. server crashes)
 **
 ** Parameters
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN app_ble_mgt_callback(tBSA_MGT_EVT event, tBSA_MGT_MSG *p_data)
{
    switch(event)
    {
    case BSA_MGT_STATUS_EVT:
        APP_DEBUG0("BSA_MGT_STATUS_EVT");
        if (p_data->status.enable)
        {
            APP_DEBUG0("Bluetooth restarted => re-initialize the application");
            app_ble_start();
        }
        break;

    case BSA_MGT_DISCONNECT_EVT:
        APP_DEBUG1("BSA_MGT_DISCONNECT_EVT reason:%d", p_data->disconnect.reason);
        /* Connection with the Server lost => Just exit the application */
        exit(-1);
        break;

    default:
        break;
    }
    return FALSE;
}

/*******************************************************************************
 **
 ** Function        main
 **
 ** Description     This is the main function
 **
 ** Parameters      Program's arguments
 **
 ** Returns         status
 **
 *******************************************************************************/
int main(int argc, char **argv)
{
    int status;

    /* Initialize BLE application */
    status = app_ble_init();
    if (status < 0)
    {
        APP_ERROR0("Couldn't Initialize BLE app, exiting");
        exit(-1);
    }

    /* Open connection to BSA Server */
    app_mgt_init();
    if (app_mgt_open(NULL, app_ble_mgt_callback) < 0)
    {
        APP_ERROR0("Unable to connect to server");
        return -1;
    }

    /* Start BLE application */
    status = app_ble_start();
    if (status < 0)
    {
        APP_ERROR0("Couldn't Start BLE app, exiting");
        return -1;
    }

    /* register one application */
    app_ble_client_register(APP_BLE_MAIN_DEFAULT_APPL_UUID);

    /* The main BLE loop */
    app_ble_menu();

    /* Exit BLE mode */
    app_ble_exit();

    /* Close BSA Connection before exiting (to release resources) */
    app_mgt_close();

    exit(0);
}
