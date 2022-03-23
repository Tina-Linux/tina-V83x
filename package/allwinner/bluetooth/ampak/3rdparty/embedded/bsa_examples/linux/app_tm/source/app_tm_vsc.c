/*****************************************************************************
**
**  Name:           app_tm_vsc.c
**
**  Description:    Bluetooth Test Module VSC functions
**
**  Copyright (c) 2011-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsa_api.h"
#include "bsa_trace.h"
#include "app_tm_vsc.h"
#include "app_utils.h"


/*******************************************************************************
 **
 ** Function        app_tm_vsc_set_tx_carrier_frequency
 **
 ** Description     This function is used to send set_tx_carrier_frequency_arm VSC
 **
 ** Parameters      enable: 0x00:carrier_on / 0x01:carrier_off
 **                 frequency: frequency selected - 2400 (50 => 2450Mhz)
 **                 mode: 0x00:Unmodulated / 0x01:PRBS9 / 0x02:PRBS15 /
 **                       0x03 : All '0'/ 0x04:All '1' / 0x05:Incremented symbols
 **                 modulation_type:0x00:GFSK / 0x01:QPSK / 0x02:8PSK
 **                 power_selection:0x00:0dBm / 0x01:-4dBm / 0x02:-8dBm / 0x03:-12dBm /
 **                                 0x04:-16dBm / 0x05:-20dBm / 0x06:-24dBm / 0x07:-28dBm /
 **                                 0x08:use power_dBm / 0x09:use power_index
 **                 power_dBm:Transmit power in dBm
 **                 power_index: power_dBm:Transmit power index
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_set_tx_carrier_frequency(BOOLEAN enable, UINT8 frequency, UINT8 mode,
        UINT8 modulation_type, UINT8 power_selection, INT8 power_dBm, UINT8 power_index)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;

    /* Prepare VSC request */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_SET_TX_CARRIER_FREQUENCY;
    bsa_vsc.length = 7 * sizeof(UINT8);

    p_data = bsa_vsc.data;
    UINT8_TO_STREAM(p_data, enable);
    UINT8_TO_STREAM(p_data, frequency);
    UINT8_TO_STREAM(p_data, mode);
    UINT8_TO_STREAM(p_data, modulation_type);
    UINT8_TO_STREAM(p_data, power_selection);
    UINT8_TO_STREAM(p_data, power_dBm);
    UINT8_TO_STREAM(p_data, power_index);

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vsc_bfc_read_params
 **
 ** Description     This function sends a VSC to read the BFC parameters
 **
 ** Parameters      Pointer on structure to return the parameter values.
 **                 Note that this parameter can be NULL.
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_bfc_read_params(tAPP_TM_TBFC_PARAM *p_param)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;
    UINT8 bfc_enable, freq1, freq2, freq3, ac_len, hid_scan_retry, dont_disturb, wake_up_mask;
    UINT16 host_scan_interval, host_trigger_timeout, hid_scan_interval;

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_BFC_READ_PARAMS;
    bsa_vsc.length = 0;

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    p_data = bsa_vsc.data;
    p_data++; /* Skip Status (already read) */
    STREAM_TO_UINT8(bfc_enable, p_data);
    STREAM_TO_UINT8(freq1, p_data);
    STREAM_TO_UINT8(freq2, p_data);
    STREAM_TO_UINT8(freq3, p_data);
    STREAM_TO_UINT8(ac_len, p_data);
    STREAM_TO_UINT16(host_scan_interval, p_data);
    STREAM_TO_UINT16(host_trigger_timeout, p_data);
    STREAM_TO_UINT16(hid_scan_interval, p_data);
    STREAM_TO_UINT8(hid_scan_retry, p_data);
    /* Check if the last two (optional) parameters are present */
    if (bsa_vsc.length > 13)
    {
        STREAM_TO_UINT8(dont_disturb, p_data);
        STREAM_TO_UINT8(wake_up_mask, p_data);
    }
    else
    {
        dont_disturb = 0;
        wake_up_mask = 0;
    }

    APP_INFO0("VSC BFC read params complete:");
    if (bsa_vsc.status != 0)
    {
        APP_INFO1("    status: failed(%d)", bsa_vsc.status);
    }
    else
    {
        APP_INFO0("    status: successful");
    }
    APP_INFO1("    BFC Enable           : %d", bfc_enable);
    APP_INFO1("    Frequency1           : %d", freq1);
    APP_INFO1("    Frequency2           : %d", freq2);
    APP_INFO1("    Frequency3           : %d", freq3);
    APP_INFO1("    AccessCode length    : %d", ac_len);
    APP_INFO1("    Host scan interval   : %d", host_scan_interval);
    APP_INFO1("    Host trigger timeout : %d", host_trigger_timeout);
    APP_INFO1("    HID scan interval    : %d", hid_scan_interval);
    APP_INFO1("    HID scan retry       : %d", hid_scan_retry);
    APP_INFO1("    Don't disturb        : %d", dont_disturb);
    APP_INFO1("    Wake Up mask         : %d", wake_up_mask);

    /* If the caller provided a pointer to have access to the read parameters */
    if (p_param)
    {
        p_param->bfc_enable = bfc_enable;
        p_param->frequency1 = freq1;
        p_param->frequency2 = freq2;
        p_param->frequency3 = freq3;
        p_param->access_code_length = ac_len;
        p_param->host_scan_interval = host_scan_interval;
        p_param->host_trigger_timeout = host_trigger_timeout;
        p_param->hid_scan_interval = hid_scan_interval;
        p_param->hid_scan_retry = hid_scan_retry;
        p_param->dont_disturb = dont_disturb;
        p_param->wake_up_mask = wake_up_mask;
    }
    return (0);
}

/*******************************************************************************
 **
 ** Function        app_tm_vsc_bfc_write_params
 **
 ** Description     This function sends a VSC to read the BFC parameters
 **
 ** Parameters      Pointer on structure containing the parameters.
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_bfc_write_params(tAPP_TM_TBFC_PARAM *p_param)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;

    if (p_param == NULL)
    {
        APP_ERROR0("Null parameter");
        return -1;
    }

    APP_INFO0("Write TBFC Parameters");
    APP_INFO1("    BFC Enable           : %d", p_param->bfc_enable);
    APP_INFO1("    Frequency1           : %d", p_param->frequency1);
    APP_INFO1("    Frequency2           : %d", p_param->frequency2);
    APP_INFO1("    Frequency3           : %d", p_param->frequency3);
    APP_INFO1("    AccessCode length    : %d", p_param->access_code_length);
    APP_INFO1("    Host scan interval   : %d", p_param->host_scan_interval);
    APP_INFO1("    Host trigger timeout : %d", p_param->host_trigger_timeout);
    APP_INFO1("    HID scan interval    : %d", p_param->hid_scan_interval);
    APP_INFO1("    HID scan retry       : %d", p_param->hid_scan_retry);
    APP_INFO1("    Don't disturb        : %d", p_param->dont_disturb);
    APP_INFO1("    Wake Up mask         : %d", p_param->wake_up_mask);

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_BFC_WRITE_PARAMS;
    bsa_vsc.length = 8 * sizeof(UINT8) + 3 * sizeof(UINT16);

    p_data = bsa_vsc.data;
    UINT8_TO_STREAM(p_data, p_param->bfc_enable);
    UINT8_TO_STREAM(p_data, p_param->frequency1);
    UINT8_TO_STREAM(p_data, p_param->frequency2);
    UINT8_TO_STREAM(p_data, p_param->frequency3);
    UINT8_TO_STREAM(p_data, p_param->access_code_length);
    UINT16_TO_STREAM(p_data, p_param->host_scan_interval);
    UINT16_TO_STREAM(p_data, p_param->host_trigger_timeout);
    UINT16_TO_STREAM(p_data, p_param->hid_scan_interval);
    UINT8_TO_STREAM(p_data, p_param->hid_scan_retry);
    UINT8_TO_STREAM(p_data, p_param->dont_disturb);
    UINT8_TO_STREAM(p_data, p_param->wake_up_mask);

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    return (0);
}


/*******************************************************************************
 **
 ** Function        app_tm_vsc_write_collaboration_mode
 **
 ** Description     This function sends a VSC to enable/disable  BT/WLAN
 **                 Coexistance (collaboration)
 **
 ** Parameters      Pointer on structure containing the parameters.
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_write_collaboration_mode(tAPP_TM_WLAN_COLLABORATION_PARAM *p_param)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;

    if (p_param == NULL)
    {
        APP_ERROR0("Null parameter");
        return -1;
    }

    APP_INFO1("Write Collaboration %s", p_param->enable==FALSE?"Disabled":"Enabled");

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_WRITE_COLLABORATION_MODE;

    p_data = bsa_vsc.data;
    /* Type */
    if (p_param->enable == FALSE)
    {
        /* Disabled */
        UINT8_TO_STREAM(p_data, APP_TM_COEX_ARCH_2045);
    }
    else
    {
        /* 3 pin 2 pin solution */
        UINT8_TO_STREAM(p_data, APP_TM_COEX_ARCH_2045 | APP_TM_COEX_SOL_3P_2P);
    }

    /* Priority  */
    UINT32_TO_STREAM(p_data, APP_TM_COEX_PRIO_TPOLL |
                             APP_TM_COEX_PRIO_INQSCAN |
                             APP_TM_COEX_PRIO_PAGESCAN |
                             APP_TM_COEX_PRIO_ROLE_SWITCH |
                             APP_TM_COEX_PRIO_NEW_CONN |
                             APP_TM_COEX_PRIO_SNIFF |
                             APP_TM_COEX_PRIO_SCO |
                             APP_TM_COEX_PRIO_DEFER_HIGH_PRIO_FRAME |
                             APP_TM_COEX_PRIO_NON_CONN_HW_SUPPORT |
                             APP_TM_COEX_PRIO_PAGE_SCAN_HW_BIT_0 |
                             APP_TM_COEX_PRIO_PAGE_SCAN_HW_BIT_1);

    /* Other parameters  */
    UINT16_TO_STREAM(p_data, 0x0000); /* Reserved */

    UINT8_TO_STREAM(p_data, 0xFA); /* Priority_Inverse_Threshold */

    /* Configuration Flag 1 */
    UINT32_TO_STREAM(p_data, APP_TM_COEX_CFG1_A2DP_ACP_PRIO_INV |
                             APP_TM_COEX_CFG1_HW_CX_MODE |
                             APP_TM_COEX_CFG1_CONN_HW_SUPP |
                             APP_TM_COEX_CFG1_5WIRE |
                             APP_TM_COEX_CFG1_DYNAMIC_LCU_RESET);

    UINT32_TO_STREAM(p_data, APP_TM_COEX_CFG2_NONE); /* Configuration Flag 2 */

    bsa_vsc.length = (UINT8)(p_data - &bsa_vsc.data[0]);
    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    return (0);
}

/*******************************************************************************
 **
 ** Function        app_tm_vsc_read_collaboration_mode
 **
 ** Description     This function sends a VSC to Read BT/WLAN
 **                 Coexistance (collaboration) configuration
 **
 ** Parameters      Pointer on structure containing the parameters.
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_read_collaboration_mode(void)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;

    APP_INFO0("Read Collaboration");

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_READ_COLLABORATION_MODE;
    bsa_vsc.length = 0x00;

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    scru_dump_hex(bsa_vsc.data, "WLAN Coexistence parameters",
            bsa_vsc.length, TRACE_LAYER_NONE, TRACE_TYPE_DEBUG);

    return (0);
}

/*******************************************************************************
 **
 ** Function         app_tm_vsc_enable_uhe
 **
 ** Description      This function sends a VSC to Control UHE mode
 **
 ** Returns          status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_enable_uhe(int enable)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_ENABLE_UHE; /* Enable USB HID Emulation VSC */
    bsa_vsc.length = 1;
    p_data = bsa_vsc.data;

    if (enable)
    {
        APP_DEBUG0("Enabling UHE mode");
        UINT8_TO_STREAM(p_data, 0x01);
    }
    else
    {
        APP_DEBUG0("Disabling UHE mode");
        UINT8_TO_STREAM(p_data, 0x00);
    }

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send VSC status:%d", bsa_status);
        return(-1);
    }

    APP_DEBUG1("VSC Server status:%d", bsa_vsc.status);
    APP_DEBUG1("VSC opcode:0x%03X", bsa_vsc.opcode);
    APP_DEBUG1("VSC length:%d", bsa_vsc.length);
    APP_DEBUG1("VSC Chip/HCI status:0x%02X", bsa_vsc.data[0]);

    if (bsa_vsc.status != 0)
    {
        APP_ERROR0("Bad HCI Return code");
        return (-1);
    }
    return (0);
}


/*******************************************************************************
 **
 ** Function         app_tm_vsc_control_llr_scan
 **
 ** Description      Start/Stop LLR Page Scan mode
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_vsc_control_llr_scan(BOOLEAN start)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_LLR_WRITE_SCAN_ENABLE;
    bsa_vsc.length = 1;

    bsa_vsc.data[0] = start;

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send VSC status:%d", bsa_status);
        return(-1);
    }

    APP_DEBUG1("VSC Server status:%d", bsa_vsc.status);
    APP_DEBUG1("VSC opcode:0x%03X", bsa_vsc.opcode);
    APP_DEBUG1("VSC length:%d", bsa_vsc.length);
    APP_DEBUG1("VSC Chip/HCI status:0x%02X", bsa_vsc.data[0]);

    if (bsa_vsc.status != 0)
    {
        APP_ERROR0("Bad HCI Return code");
        return (-1);
    }
    return (0);
}

/*******************************************************************************
 **
 ** Function        app_tm_vsc_set_tx_power
 **
 ** Description     This function sends the following VSC:
 **                 Set Transmit Power
 **
 ** Parameters      bd_addr: BD address of the device to read tx power
 **                 power : tx power level
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_set_tx_power(BD_ADDR bda, INT8 power)
{
    tBSA_TM_GET_HANDLE conn_handle;
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;

    BSA_TmGetHandleInit(&conn_handle);

    if (bda == NULL)
    {
        APP_ERROR0("BD address is not correct");
        return -1;
    }
    else
    {
        bdcpy(conn_handle.bd_addr, bda);
    }

    BSA_TmGetHandle(&conn_handle);

    if(conn_handle.status != BSA_SUCCESS)
    {
        APP_INFO1("\tBad parameter (err=%d)",conn_handle.status);
        return(-1);
    }

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_SET_TX_POWER;
    bsa_vsc.length = 0x03;
    bsa_vsc.data[0] = (UINT16)conn_handle.handle;
    bsa_vsc.data[2] = power;


    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send VSC status:%d", bsa_status);
        return(-1);
    }
    else
    {
        APP_INFO1("\tStatus:                   %d", bsa_status);
        APP_INFO1("\tConnection handle:        0x%02X (%d)", conn_handle.handle, conn_handle.handle);
        APP_INFO1("\tSet Tranmsit Power Level: %d (dBm)", power);
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vsc_read_tx_power
 **
 ** Description     This function sends the following VSC:
 **
 ** Parameters      bd_addr: BD address of the device to read tx power
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_read_tx_power(BD_ADDR bda)
{
    tBSA_TM_GET_HANDLE conn_handle;
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;

    UINT16 encap_opcode = 0;
    UINT8 encap_length = 0;
    UINT8 length;

    UINT8 status = 0;
    UINT8 *p_data;
    UINT16 handle = 0;
    INT8 power = 0;

    BSA_TmGetHandleInit(&conn_handle);

    if (bda == NULL)
    {
        APP_ERROR0("BD address is not correct");
        return -1;
    }
    else
    {
        bdcpy(conn_handle.bd_addr, bda);
    }

    BSA_TmGetHandle(&conn_handle);

    if(conn_handle.status != BSA_SUCCESS)
    {
        APP_INFO1("\tBad handle parameter (err=%d)",conn_handle.status);
        return(-1);
    }

    bsa_status = BSA_TmVscInit(&bsa_vsc);

    length = 6;
    encap_opcode = 3117;
    encap_length = 3;

    bsa_vsc.opcode = HCI_VSC_OPCODE_ENCAP_HCI_CMD;
    bsa_vsc.length = length;
    memcpy(&bsa_vsc.data[0],&encap_opcode, sizeof(UINT16));
    memcpy(&bsa_vsc.data[2],&encap_length, sizeof(UINT8));
    bsa_vsc.data[3] = (UINT16)conn_handle.handle;
    bsa_vsc.data[5] = 0;  /* Type = 0 for Read Tx Power Level */

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("Unable to Send VSC status:%d", bsa_status);
        return(-1);
    }
    else
    {
        p_data = bsa_vsc.data;
        STREAM_TO_UINT8(status, p_data);
        STREAM_TO_UINT16(handle, p_data);
        power = bsa_vsc.data[3];

        APP_INFO1("\tStatus:               %d", status);
        APP_INFO1("\tConnection handle:    0x%02X (%d)", handle, handle);
        APP_INFO1("\tTranmsit Power Level: %d (dBm)", power);
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vsc_config_gpio
 **
 ** Description     This function sends a VSC to configure GPIO
 **
 ** Parameters      Pointer on structure containing the parameters.
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_config_gpio(UINT8 aux_gpio, UINT8 pin_number, UINT8 pad_config, UINT8 value)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;

    APP_INFO1("app_tm_vsc_config_gpio pin:%d value:%d", pin_number, value);

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_GPIO_CONFIG_AND_WRITE;

    p_data = bsa_vsc.data;
    if(aux_gpio)
    {
        pin_number = pin_number|0x80;
    }

    UINT8_TO_STREAM(p_data, pin_number); /* Pin number */
    UINT8_TO_STREAM(p_data, pad_config); /* Pad config */
    UINT8_TO_STREAM(p_data, value); /* value */
    bsa_vsc.length = (UINT8)(p_data - &bsa_vsc.data[0]);

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    return (0);
}

/*******************************************************************************
 **
 ** Function        app_tm_vsc_read_otp
 **
 ** Description     This function sends a VSC to read OTP
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_read_otp()
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;
    UINT8 otp_oper;
    BD_ADDR bd_addr;

    APP_INFO0("app_tm_vsc_read_otp");

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_ACCESS_OTP;

    otp_oper = HCI_VSC_OPCODE_ACCESS_OTP_READ_OPERATION;

    p_data = bsa_vsc.data;

    UINT8_TO_STREAM(p_data, otp_oper);
    bsa_vsc.length = (UINT8)(p_data - &bsa_vsc.data[0]);

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    APP_INFO0("VSC OTP read complete:");
    if (bsa_vsc.status != 0)
    {
        APP_INFO1("    status: failed(%d)", bsa_vsc.status);
        return (-1);
    }
    else
    {
        APP_INFO0("    status: successful");
        scru_dump_hex(bsa_vsc.data, "Read OTP",
                bsa_vsc.length, TRACE_LAYER_NONE, TRACE_TYPE_DEBUG);
        p_data = bsa_vsc.data;
        STREAM_TO_UINT8(bsa_status, p_data);    /* skip status type */
        STREAM_TO_UINT8(bsa_status, p_data);    /* skip return type */
        STREAM_TO_UINT8(bsa_status, p_data);    /* skip read status */
        STREAM_TO_UINT8(bsa_status, p_data);    /* skip read hw status */
        STREAM_TO_UINT8(bsa_status, p_data);    /* skip read hw status */
        STREAM_TO_BDADDR(bd_addr, p_data);

        APP_INFO1("\tBdAddr:  %02X-%02X-%02X-%02X-%02X-%02X",
               bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
    }

    return (0);
}

/*******************************************************************************
 **
 ** Function        app_tm_vsc_write_otp
 **
 ** Description     This function sends a VSC to write OTP
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_write_otp()
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;
    UINT8 otp_oper;
    BD_ADDR bd_addr;

    APP_INFO0("app_tm_vsc_write_otp");

    APP_INFO0("Enter the printer BD address(AA.BB.CC.DD.EE.FF): ");
    /* coverity[SECURE_CODING] False-positive: used with precision specifiers */
    if (scanf("%hhx.%hhx.%hhx.%hhx.%hhx.%hhx",
            &bd_addr[0], &bd_addr[1],
            &bd_addr[2], &bd_addr[3],
            &bd_addr[4], &bd_addr[5]) != 6)
    {
        APP_ERROR0("BD address not entered correctly");
        return -1;
    }

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_VSC_OPCODE_ACCESS_OTP;

    otp_oper = HCI_VSC_OPCODE_ACCESS_OTP_WRITE_OPERATION;

    p_data = bsa_vsc.data;

    UINT8_TO_STREAM(p_data, otp_oper);
    BDADDR_TO_STREAM(p_data, bd_addr);
    bsa_vsc.length = 0x2A;

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    return (0);
}


/*******************************************************************************
 **
 ** Function        app_tm_vsc_configure_sco
 **
 ** Description     This function sends a VSC to configure SCO interface
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_configure_sco()
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;

    APP_INFO0("app_tm_vsc_configure_pcm");

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);

    bsa_vsc.opcode = HCI_VSC_OPCODE_WRITE_SCO_PCM_INT_PARAM;

    /* For SCO and PCM interface parameters :
     * Byte0 -- 0 for PCM, 1 for transport, 2 for codec, 3 for I2S
     * Byte1 -- 0 for 128 KHz, 1 for 256, 2 for 512, 3 for 1024, 4 for 2048
     * Byte2 -- 0 for short frame sync, 1 for long
     * Byte3 -- 0 for slave role, 1 for master role for SYNC pin
     * Byte4 -- 0 for slave role, 1 for master role for CLK pin
     */

    APP_INFO0("Byte0 -- 0 for PCM, 1 for transport, 2 for codec, 3 for I2S");
    bsa_vsc.data[0] = app_get_choice("Byte0:");
    APP_INFO0("Byte1 -- 0 for 128 KHz, 1 for 256, 2 for 512, 3 for 1024, 4 for 2048");
    bsa_vsc.data[1] = app_get_choice("Byte1:");
    APP_INFO0("Byte2 -- 0 for short frame sync, 1 for long");
    bsa_vsc.data[2] = app_get_choice("Byte2");
    APP_INFO0("Byte3 -- 0 for slave role, 1 for master role for SYNC pin");
    bsa_vsc.data[3] = app_get_choice("Byte3");
    APP_INFO0("Byte4 -- 0 for slave role, 1 for master role for CLK pin");
    bsa_vsc.data[4] = app_get_choice("Byte4");

    APP_INFO1("app_tm_vsc_configure_sco {%d, %d, %d, %d, %d}",
           bsa_vsc.data[0], bsa_vsc.data[1],
           bsa_vsc.data[2], bsa_vsc.data[3],
           bsa_vsc.data[4]);

    bsa_vsc.length = 5;

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    return (0);
}


/*******************************************************************************
 **
 ** Function        app_tm_vsc_configure_pcm
 **
 ** Description     This function sends a VSC to configure PCM
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_configure_pcm()
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;

    APP_INFO0("app_tm_vsc_configure_pcm");

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);

    bsa_vsc.opcode = HCI_VSC_OPCODE_WRITE_PCM_CONFIG_PARAM;

    /* Set PCM interface configuration. The PCM parameters format is as following:
     * Byte0 LSB_First        -- 0 for MSb first
     * Byte1 Fill value(0-1)  -- Specified the value with which to fill unused bits if
     *                           Fill option is set to programmable.
     *                           Use 0x0 for programmed fill bits.
     * Byte2 Fill option(0-3) -- Specifies the method of filling unused data bits values
     *                           0x0:0's, 0x1:1's, 0x2:signed, 0x3:programmable
     * Byte3 Fill bits(0-3)   -- Fill all 3 bits.
     * Byte4 Right justify    -- Use left justify(fill data shifted lout last)
     */

    APP_INFO0("Byte0 LSB_First -- 0 for MSb first");
    bsa_vsc.data[0] = app_get_choice("Byte0:");
    APP_INFO0("Byte1 Fill value(0-1) -- Specified the value with which to fill unused bits if");
    APP_INFO0("\tFill option is set to programmable.");
    APP_INFO0("\tUse 0x0 for programmed fill bits");
    bsa_vsc.data[1] = app_get_choice("Byte1:");
    APP_INFO0("Byte2 Fill option(0-3) -- Specifies the method of filling unused data bits values");
    APP_INFO0("\t0x0:0's, 0x1:1's, 0x2:signed, 0x3:programmable");
    bsa_vsc.data[2] = app_get_choice("Byte2");
    APP_INFO0("Byte3 Fill bits(0-3)   -- Fill all 3 bits.");
    bsa_vsc.data[3] = app_get_choice("Byte3");
    APP_INFO0("Byte4 Right justify    -- Use left justify(fill data shifted lout last)");
    bsa_vsc.data[4] = app_get_choice("Byte4");

    APP_INFO1("app_tm_vsc_configure_pcm {%d, %d, %d, %d, %d}",
           bsa_vsc.data[0], bsa_vsc.data[1],
           bsa_vsc.data[2], bsa_vsc.data[3],
           bsa_vsc.data[4]);

    bsa_vsc.length = 5;

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    return (0);
}




/*******************************************************************************
 **
 ** Function        app_tm_vsc_set_connection_priority
 **
 ** Description     This function sends a VSC to configure priority of connection
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_set_connection_priority()
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;
    tBSA_TM_GET_HANDLE conn_handle;
    UINT8 priority;

    APP_INFO0("app_tm_vsc_set_connection_priority");

    BSA_TmGetHandleInit(&conn_handle);

    conn_handle.transport = app_get_choice("Select transport 1:BR_EDR, 2:LE");
    if ((conn_handle.transport != BT_TRANSPORT_BR_EDR) &&
        (conn_handle.transport != BT_TRANSPORT_LE))
    {
        APP_ERROR1("Wrong Transport value:%d", conn_handle.transport);
        return -1;
    }

    priority = app_get_choice("Priority 0:Normal, 1:High");

    APP_INFO0("Enter the BD address of remote connected device(AA:BB:CC:DD:EE:FF): ");
    /* coverity[SECURE_CODING] False-positive: used with precision specifiers */
    if (scanf("%hhx:%hhx:%hhx:%hhx:%hhx:%hhx",
            &conn_handle.bd_addr[0], &conn_handle.bd_addr[1],
            &conn_handle.bd_addr[2], &conn_handle.bd_addr[3],
            &conn_handle.bd_addr[4], &conn_handle.bd_addr[5]) != 6)
    {
        APP_ERROR0("BD address not entered correctly");
        return -1;
    }

    BSA_TmGetHandle(&conn_handle);
    if(conn_handle.status != BSA_SUCCESS)
    {
        APP_INFO1("\tBad parameter (err=%d)",conn_handle.status);
        return(-1);
    }

    /* Prepare VSC Parameters */
    bsa_status = BSA_TmVscInit(&bsa_vsc);

    bsa_vsc.opcode = HCI_VSC_OPCODE_SET_ACL_PRIORITY;
    p_data = bsa_vsc.data;
    UINT16_TO_STREAM(p_data, conn_handle.handle);
    UINT8_TO_STREAM(p_data, priority);
    bsa_vsc.length = 3;

    /* Send the VSC */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed:%d", bsa_status);
        return(-1);
    }

    return (0);
}
