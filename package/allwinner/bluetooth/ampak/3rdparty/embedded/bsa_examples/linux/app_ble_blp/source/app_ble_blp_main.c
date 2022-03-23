/*****************************************************************************
**
**  Name:           app_ble_blp_main.c
**
**  Description:    Bluetooth BLE Blood Pressure Collector application
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
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
#include "app_ble_blp.h"

/*
 * Defines
 */

/* BLE BLP menu items */
enum
{
    APP_BLE_BLP_MENU_REGISTER = 1,
    APP_BLE_BLP_MENU_DISCOVERY,
    APP_BLE_MENU_CONFIG_BLE_BG_CONN,
    APP_BLE_BLP_MENU_OPEN,
    APP_BLE_BLP_MENU_SEARCH_BLOOD_PRESSURE_SERVICE,
    APP_BLE_BLP_MENU_SEARCH_DEVICE_INFOR_SERVICE,
    APP_BLE_BLP_MENU_READ_BLOOD_PRESSURE_FEATURE,
    APP_BLE_BLP_MENU_ENABLE_BLOOD_PRESSURE_MEASUREMENT,
    APP_BLE_BLP_MENU_DISBLE_BLOOD_PRESSURE_MEASUREMENT,
    APP_BLE_BLP_MENU_START_INTERMEDIATE_CUFF_PRESSURE,
    APP_BLE_BLP_MENU_STOP_INTERMEDIATE_CUFF_PRESSURE,
    APP_BLE_BLP_MENU_DISPLAY_CLIENT,
    APP_BLE_BLP_MENU_QUIT = 99
};

/*
 * Global Variables
 */


/*
 * Local functions
 */
static void app_ble_blp_menu(void);



/*******************************************************************************
 **
 ** Function         app_ble_blp_display_menu
 **
 ** Description      This is the BLP menu
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_blp_display_menu (void)
{
    APP_INFO0("\t**** BLE BLP menu ***");
    APP_INFO1("\t%d => Register Blood Pressure Collector", APP_BLE_BLP_MENU_REGISTER);
    APP_INFO1("\t%d => Discover Blood Pressure Sensor", APP_BLE_BLP_MENU_DISCOVERY);
    APP_INFO1("\t%d => Configure BLE Background Connection Type", APP_BLE_MENU_CONFIG_BLE_BG_CONN);
    APP_INFO1("\t%d => Connect Blood Pressure Sensor", APP_BLE_BLP_MENU_OPEN);
    APP_INFO1("\t%d => Search Blood Pressure Service",APP_BLE_BLP_MENU_SEARCH_BLOOD_PRESSURE_SERVICE);
    APP_INFO1("\t%d => Search Device Information Service",APP_BLE_BLP_MENU_SEARCH_DEVICE_INFOR_SERVICE);
    APP_INFO1("\t%d => Read Blood Pressure Feature",APP_BLE_BLP_MENU_READ_BLOOD_PRESSURE_FEATURE);
    APP_INFO1("\t%d => Enable Blood pressure measurement",APP_BLE_BLP_MENU_ENABLE_BLOOD_PRESSURE_MEASUREMENT);
    APP_INFO1("\t%d => Disable Blood pressure measurement",APP_BLE_BLP_MENU_DISBLE_BLOOD_PRESSURE_MEASUREMENT);
    APP_INFO1("\t%d => Start Intermediate Cuff Pressure measurement",APP_BLE_BLP_MENU_START_INTERMEDIATE_CUFF_PRESSURE);
    APP_INFO1("\t%d => Stop Intermediate Cuff Pressure measurement",APP_BLE_BLP_MENU_STOP_INTERMEDIATE_CUFF_PRESSURE);
    APP_INFO1("\t%d => Show all attributes", APP_BLE_BLP_MENU_DISPLAY_CLIENT);
    APP_INFO1("\t%d => Quit", APP_BLE_BLP_MENU_QUIT);
}

/*******************************************************************************
 **
 ** Function         app_ble_blp_menu
 **
 ** Description      Blood Pressure Collector sub menu
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_ble_blp_menu(void)
{
    int choice, type;

    do
    {
        app_ble_blp_display_menu();

        choice = app_get_choice("Sub Menu");

        switch(choice)
        {
        case APP_BLE_BLP_MENU_REGISTER:
            app_ble_blp_main();
            break;

        case APP_BLE_BLP_MENU_DISCOVERY:
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

        case APP_BLE_BLP_MENU_OPEN:
            app_ble_client_open();
            break;

        case APP_BLE_BLP_MENU_SEARCH_BLOOD_PRESSURE_SERVICE:
            app_ble_client_service_search_execute(BSA_BLE_UUID_SERVCLASS_BLOOD_PRESSURE);
            break;

        case APP_BLE_BLP_MENU_SEARCH_DEVICE_INFOR_SERVICE:
            app_ble_client_service_search_execute(BSA_BLE_UUID_SERVCLASS_DEVICE_INFORMATION);
            break;

        case APP_BLE_BLP_MENU_READ_BLOOD_PRESSURE_FEATURE:
            app_ble_blp_read_blood_pressure_feature();
            break;

        case APP_BLE_BLP_MENU_ENABLE_BLOOD_PRESSURE_MEASUREMENT:
            app_ble_blp_enable_blood_pressure_measurement(TRUE);
            break;

        case APP_BLE_BLP_MENU_DISBLE_BLOOD_PRESSURE_MEASUREMENT:
            app_ble_blp_enable_blood_pressure_measurement(FALSE);
            break;

        case APP_BLE_BLP_MENU_START_INTERMEDIATE_CUFF_PRESSURE:
            app_ble_blp_intermediate_cuff_pressure(TRUE);
            break;

        case APP_BLE_BLP_MENU_STOP_INTERMEDIATE_CUFF_PRESSURE:
            app_ble_blp_intermediate_cuff_pressure(FALSE);

        case APP_BLE_BLP_MENU_DISPLAY_CLIENT:
            app_ble_client_display(1);
            break;

        case APP_BLE_BLP_MENU_QUIT:
            APP_INFO0("Quit");
            break;

        default:
            APP_ERROR1("Unknown choice:%d", choice);
            break;
        }
    } while (choice != APP_BLE_BLP_MENU_QUIT); /* While user don't exit sub-menu */
}

/*******************************************************************************
 **
 ** Function         app_ble_blp_mgt_callback
 **
 ** Description      This callback function is called in case of server
 **                  disconnection (e.g. server crashes)
 **
 ** Parameters
 **
 ** Returns          BOOLEAN
 **
 *******************************************************************************/
BOOLEAN app_ble_blp_mgt_callback(tBSA_MGT_EVT event, tBSA_MGT_MSG *p_data)
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
        APP_DEBUG1("app_management_callback BSA_MGT_DISCONNECT_EVT reason:%d", p_data->disconnect.reason);
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

    app_mgt_init();
    /* Open connection to BSA Server */
    status = app_mgt_open(NULL, app_ble_blp_mgt_callback);
    if (status < 0)
    {
        APP_ERROR0("Unable to Connect to server");
        return -1;
    }

    /* Start BLE application */
    status = app_ble_start();
    if (status < 0)
    {
        APP_ERROR0("Couldn't Start BLE app, exiting");
        exit(-1);
    }

    /* The main BLE loop */
    app_ble_blp_menu();

    /* Exit BLE mode */
    app_ble_exit();

    /* Close BSA Connection before exiting (to release resources) */
    app_mgt_close();

    return 0;
}
