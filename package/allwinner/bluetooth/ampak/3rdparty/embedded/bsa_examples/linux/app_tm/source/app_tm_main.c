/*****************************************************************************
**
**  Name:           app_tm_main.c
**
**  Description:    Bluetooth Test Module main application
**
**  Copyright (c) 2010-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "app_tm.h"
#include "app_tm_vse.h"
#include "app_tm_vsc.h"
#include "app_utils.h"
#include "app_mgt.h"


/* Menu items */
enum
{
    APP_TM_MENU_ENB_TST_MODE = 1,
    APP_TM_MENU_DIS_TST_MODE,
    APP_TM_MENU_DISCONNECT,
    APP_TM_MENU_SV_MEM_USAGE,
    APP_TM_MENU_CL_MEM_USAGE,
    APP_TM_MENU_PING,
    APP_TM_MENU_VSC,
    APP_TM_MENU_TRACE_CONTROL,
    APP_TM_MENU_READ_VERSION,
    APP_TM_MENU_READ_RAM,
    APP_TM_MENU_READ_FLASH_BD_ADDR,
    APP_TM_MENU_READ_CONN,
    APP_TM_MENU_VSE_REGISTER,
    APP_TM_MENU_VSE_DEREGISTER,
    APP_TM_MENU_VSE_SET_TX_CARRIER,
    APP_TM_MENU_READ_BFC_PARAMS,
    APP_TM_MENU_READ_WLAN_COEX,
    APP_TM_MENU_WRITE_WLAN_COEX,
    APP_TM_MENU_READ_RSSI,
    APP_TM_MENU_READ_RAWRSSI,
    APP_TM_MENU_SET_TX_POWER,
    APP_TM_MENU_READ_TX_POWER,
    APP_TM_MENU_LE_TEST,
    APP_TM_MENU_CONFIG_GPIO,
    APP_TM_MENU_READ_OTP,
    APP_TM_MENU_WRITE_OTP,
    APP_TM_MENU_CONFIGURE_PCM,
    APP_TM_MENU_CONFIGURE_SCO,
    APP_TM_MENU_CHANGE_PRIORITY,
    APP_TM_MENU_QUIT = 99
};

/*******************************************************************************
 **
 ** Function         app_tm_display_main_menu
 **
 ** Description      This is the main menu
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_tm_display_main_menu(void)
{
    APP_INFO0("Bluetooth Test Module Application Main menu:\n");
    APP_INFO1("    %d => Enable Test Mode", APP_TM_MENU_ENB_TST_MODE);
    APP_INFO1("    %d => Disable Test Mode", APP_TM_MENU_DIS_TST_MODE);
    APP_INFO1("    %d => Disconnect a device", APP_TM_MENU_DISCONNECT);
    APP_INFO1("    %d => Get Server Memory Usage", APP_TM_MENU_SV_MEM_USAGE);
    APP_INFO1("    %d => Get Client Memory Usage", APP_TM_MENU_CL_MEM_USAGE);
    APP_INFO1("    %d => Ping", APP_TM_MENU_PING);
    APP_INFO1("    %d => Sent VendorSpecificCommand", APP_TM_MENU_VSC);
    APP_INFO1("    %d => Control trace level(0 ~ 5)", APP_TM_MENU_TRACE_CONTROL);
    APP_INFO1("    %d => Read Version", APP_TM_MENU_READ_VERSION);
    APP_INFO1("    %d => Read RAM", APP_TM_MENU_READ_RAM);
    APP_INFO1("    %d => Read BD address from flash", APP_TM_MENU_READ_FLASH_BD_ADDR);
    APP_INFO1("    %d => Read ACL connections table", APP_TM_MENU_READ_CONN);
    APP_INFO1("    %d => VSE Register", APP_TM_MENU_VSE_REGISTER);
    APP_INFO1("    %d => VSE Deregister", APP_TM_MENU_VSE_DEREGISTER);
    APP_INFO1("    %d => Set Tx Carrier Frequency", APP_TM_MENU_VSE_SET_TX_CARRIER);
    APP_INFO1("    %d => Read BFC parameters", APP_TM_MENU_READ_BFC_PARAMS);
    APP_INFO1("    %d => Read WLAN Coexistence Parameters (require specific FW)", APP_TM_MENU_READ_WLAN_COEX);
    APP_INFO1("    %d => Write WLAN Coexistence Parameters (require specific FW)", APP_TM_MENU_WRITE_WLAN_COEX);
    APP_INFO1("    %d => Read RSSI", APP_TM_MENU_READ_RSSI);
    APP_INFO1("    %d => Read Raw RSSI", APP_TM_MENU_READ_RAWRSSI);
    APP_INFO1("    %d => Set Tx Power Level", APP_TM_MENU_SET_TX_POWER);
    APP_INFO1("    %d => Read Tx Power Level", APP_TM_MENU_READ_TX_POWER);
    APP_INFO1("    %d => LE Tests", APP_TM_MENU_LE_TEST);
    APP_INFO1("    %d => Config GPIO", APP_TM_MENU_CONFIG_GPIO);
    APP_INFO1("    %d => Read OTP", APP_TM_MENU_READ_OTP);
    APP_INFO1("    %d => Write OTP", APP_TM_MENU_WRITE_OTP);
    APP_INFO1("    %d => Configure PCM interface", APP_TM_MENU_CONFIGURE_PCM);
    APP_INFO1("    %d => Configure SCO", APP_TM_MENU_CONFIGURE_SCO);
    APP_INFO1("    %d => Change connection priority", APP_TM_MENU_CHANGE_PRIORITY);
    APP_INFO1("    %d => Quit", APP_TM_MENU_QUIT);
}

/*******************************************************************************
 **
 ** Function         app_tm_mgt_callback
 **
 ** Description      This callback function is called in case of server
 **                  disconnection (e.g. server crashes)
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
static BOOLEAN app_tm_mgt_callback(tBSA_MGT_EVT event, tBSA_MGT_MSG *p_data)
{
    switch(event)
    {
    case BSA_MGT_DISCONNECT_EVT:
        APP_INFO1("app_tm_mgt_callback BSA_MGT_DISCONNECT_EVT reason:%d", p_data->disconnect.reason);
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
 ** Function         main
 **
 ** Description      This is the main function
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
int main (int argc, char **argv)
{
    int choice;
    int level;
    BD_ADDR bd_addr;
    INT8 power;
    tAPP_TM_WLAN_COLLABORATION_PARAM wlan_coex_param;
    tBSA_TM_LE_CMD cmd;
    UINT8 aux_gpio, pin_number, pad_config, value;

    /* Open connection to BSA Server */
    app_mgt_init();
    if (app_mgt_open(NULL, app_tm_mgt_callback) < 0)
    {
        APP_ERROR0("Unable to connect to server");
        return -1;
    }

    app_tm_vse_init();

    do
    {
        app_tm_display_main_menu();

        choice = app_get_choice("Select action");

        switch(choice)
        {
        case APP_TM_MENU_ENB_TST_MODE:
            app_tm_set_test_mode(TRUE);
            break;

        case APP_TM_MENU_DIS_TST_MODE:
            app_tm_set_test_mode(FALSE);
            break;

        case APP_TM_MENU_DISCONNECT:
            app_tm_disconnect(NULL);
            break;

        case APP_TM_MENU_SV_MEM_USAGE:
            app_tm_get_mem_usage(BSA_TM_SERVER);
            break;

        case APP_TM_MENU_CL_MEM_USAGE:
            app_tm_get_mem_usage(BSA_TM_CLIENT);
            break;

        case APP_TM_MENU_PING:
            app_tm_ping();
            break;

        case APP_TM_MENU_VSC:
            app_tm_vsc();
            break;

        case APP_TM_MENU_TRACE_CONTROL:
            APP_INFO0("Trace level supported:");
            APP_INFO0("\t0: None");
            APP_INFO0("\t1: Error");
            APP_INFO0("\t2: Warning");
            APP_INFO0("\t3: API");
            APP_INFO0("\t4: Event");
            APP_INFO0("\t5: Debug (full)");
            level = app_get_choice("level");
            app_tm_set_trace(level);
            break;

        case APP_TM_MENU_READ_VERSION:
            app_tm_read_version();
            break;

        case APP_TM_MENU_READ_RAM:
            app_tm_read_ram();
            break;

        case APP_TM_MENU_READ_FLASH_BD_ADDR:
            app_tm_read_flash_bd_addr();
            break;

        case APP_TM_MENU_READ_CONN:
            app_tm_read_conn();
            break;

        case APP_TM_MENU_VSE_REGISTER:
            app_tm_vse_register(-1, NULL);
            break;

        case APP_TM_MENU_VSE_DEREGISTER:
            app_tm_vse_deregister(-1, FALSE);
            break;

        case APP_TM_MENU_VSE_SET_TX_CARRIER:
            app_tm_set_tx_carrier_frequency_test();
            break;

        case APP_TM_MENU_READ_BFC_PARAMS:
            app_tm_vsc_bfc_read_params(NULL);
            break;

        case APP_TM_MENU_READ_WLAN_COEX:
            app_tm_vsc_read_collaboration_mode();
            break;

        case APP_TM_MENU_WRITE_WLAN_COEX:
            APP_INFO0("WLAN Coexistence control (specific FW required)\n");
            APP_INFO0("    0: to Disable\n");
            APP_INFO0("    1: to Enable\n");
            choice = app_get_choice("Coexistence control");
            switch (choice)
            {
            case 0:
                wlan_coex_param.enable = FALSE;
                app_tm_vsc_write_collaboration_mode(&wlan_coex_param);
                break;
            case 1:
                wlan_coex_param.enable = TRUE;
                app_tm_vsc_write_collaboration_mode(&wlan_coex_param);
                break;
            default:
                APP_ERROR1("Bad choice:%d", choice);
                break;
            }
            break;

        case APP_TM_MENU_READ_RSSI:
            app_tm_read_rssi(NULL);
            break;

        case APP_TM_MENU_READ_RAWRSSI:
            app_tm_vsc_read_raw_rssi(NULL);
            break;

        case APP_TM_MENU_SET_TX_POWER:
            APP_INFO0("Enter the BD address of remote connected device(AA.BB.CC.DD.EE.FF):");

            if (scanf("%hhx.%hhx.%hhx.%hhx.%hhx.%hhx",
                    &bd_addr[0], &bd_addr[1],
                    &bd_addr[2], &bd_addr[3],
                    &bd_addr[4], &bd_addr[5]) != 6)
            {
                APP_ERROR0("BD address not entered correctly");
                break;
            }
            getchar();
            power = app_get_choice("Enter Max Power Level");
            app_tm_vsc_set_tx_power(bd_addr, power);
            break;

        case APP_TM_MENU_READ_TX_POWER:
            APP_INFO0("Enter the BD address of remote connected device(AA.BB.CC.DD.EE.FF):");
            if (scanf("%hhx.%hhx.%hhx.%hhx.%hhx.%hhx",
                    &bd_addr[0], &bd_addr[1],
                    &bd_addr[2], &bd_addr[3],
                    &bd_addr[4], &bd_addr[5]) != 6)
            {
                APP_ERROR0("BD address not entered correctly");
                break;
            }
            getchar();
            app_tm_vsc_read_tx_power(bd_addr);
            break;

        case APP_TM_MENU_LE_TEST:
            APP_INFO0("LE Cmd Tests:\n");
            APP_INFO0("    0: to run tx test\n");
            APP_INFO0("    1: to run rx test\n");
            APP_INFO0("    2: to end LE test\n");
            memset(&cmd,0,sizeof(tBSA_TM_LE_CMD));
            cmd.test = app_get_choice("LE test");
            if (cmd.test > 2)
                break;
            if (cmd.test == BSA_LE_RX_TEST_CMD)
            {
                cmd.freq = app_get_choice("Enter rx frequency (0 - 39)");
            }
            else if (cmd.test == BSA_LE_TX_TEST_CMD)
            {
                cmd.freq = app_get_choice("Enter tx frequency (0 - 39)");
                cmd.payload_len = app_get_choice("Enter test_data_len (0 - 37)");
                cmd.pattern = app_get_choice("Enter payload_pattern (0 - 7)");
            }

            if(app_tm_le_test(&cmd)==BSA_SUCCESS)
            {
                if(cmd.test==BSA_LE_END_TEST_CMD)
                {
                    APP_INFO1("LE Test result. count=%d\n",cmd.retcount);
                }
            }
            else
            {
                APP_INFO0("LE Test is failed");
            }
            break;

        case APP_TM_MENU_CONFIG_GPIO:
            aux_gpio = app_get_choice("Auxiliary GPIO Control(0:disable, 1:enable)");
            pin_number = app_get_choice("Select GPIO Pin number(0-7)");
            pad_config = app_get_choice("Pad configure(Input|Pullup|Pulldown|Hysteresis|Sel0|Sel1)");
            value = app_get_choice("Select value(0:low, 1:high)");
            app_tm_vsc_config_gpio(aux_gpio, pin_number, pad_config, value);
            break;

        case APP_TM_MENU_READ_OTP:
            app_tm_vsc_read_otp();
            break;

        case APP_TM_MENU_WRITE_OTP:
            app_tm_vsc_write_otp();
            break;

        case APP_TM_MENU_CONFIGURE_PCM:
            app_tm_vsc_configure_pcm();
            break;

        case APP_TM_MENU_CONFIGURE_SCO:
            app_tm_vsc_configure_sco();
            break;

        case APP_TM_MENU_CHANGE_PRIORITY:
            app_tm_vsc_set_connection_priority();
            break;

        default:
            APP_ERROR1("Unknown choice:%d", choice);
            break;
        }
    } while (choice != APP_TM_MENU_QUIT); /* While user don't exit application */

    /* Close BSA Connection before exiting (to release resources) */
    app_mgt_close();

    return 0;
}
