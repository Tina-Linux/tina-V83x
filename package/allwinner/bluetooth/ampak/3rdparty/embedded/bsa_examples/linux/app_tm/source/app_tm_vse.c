/*****************************************************************************
**
**  Name:           app_tm_vse.c
**
**  Description:    Bluetooth Vendor Specific Event handing
**
**  Copyright (c) 2015, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>     /* For unlink */
#include <sys/types.h>  /* For open */
#include <sys/stat.h>   /* For open */
#include <fcntl.h>      /* For open */
#include <errno.h>      /* For errno */
#include <sys/time.h>   /* For gettimeofday */
#include <time.h>

#include "bsa_api.h"
#include "bsa_trace.h"
#include "app_tm_vse.h"
#include "app_utils.h"


/*
 * Definitions
 */

/*
 * Global variables
 */
static char const * const app_tm_vse_4C_battery_type[16] =
{
                                        /* 0123 */
    "Primary cell: Unknown",            /* 0000 = 0 */
    "Secondary cell: Li-Ion 1(normal)", /* 1000 = 1 */
    "Primary cell: Reserved",           /* 0100 = 2 */
    "Secondary cell: Ni-MH",            /* 1100 = 3 */
    "Primary cell: Alcaline(Zn-C)",     /* 0010 = 4 */
    "Secondary cell: Li-Ion 3(LiFEPo4)",/* 1010 = 5 */
    "Primary cell: Reserved",           /* 0110 = 6 */
    "Secondary cell: Others",           /* 1110 = 7 */
    "Primary cell: Lithium",            /* 0001 = 8 */
    "Secondary cell: Li-Ion 2",         /* 1001 = 9 */
    "Primary cell: Reserved",           /* 0101 = 10 */
    "Secondary cell: Li other",         /* 1101 = 11 */
    "Primary cell: Others",             /* 0011 = 12 */
    "Secondary cell: Li-Ion 4",         /* 1011 = 13 */
    "Primary cell: Reserved",           /* 0111 = 14 */
    "Secondary cell: Unknown"           /* 1111 = 15 */
};

tAPP_TM_VSE_CB app_tm_vse_cb;

/*
 * Local functions
 */
static void app_tm_vse_generic_callback (tBSA_TM_EVT event, tBSA_TM_MSG *p_data);
static void app_tm_vse_decode_param (tBSA_TM_VSE_MSG *p_vse);

/*******************************************************************************
 **
 ** Function        app_tm_vse_init
 **
 ** Description     Initialize VSE module
 **
 ** Parameters      None
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_init (void)
{
    memset(&app_tm_vse_cb, 0, sizeof(app_tm_vse_cb));

    /* This indicate that when the first F7 (FW core dump) will be received */
    /* the core dump file must be erased */
    app_tm_vse_cb.erase_fw_core_dump = TRUE;

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vse_get_prox_pairing_type_desc
 **
 ** Description     Get the name of a Proximity Pairing type Event
 **
 ** Parameters      type: Proximity Pairing type
 **
 ** Returns         Type Description (string)
 **
 *******************************************************************************/
char *app_tm_vse_get_prox_pairing_type_desc (UINT8 type)
{
    switch (type)
    {
    case APP_TM_VSE_PROX_PAIR_TYPE_3DG:
        return "3DG";
    case APP_TM_VSE_PROX_PAIR_TYPE_RC:
        return "RC";
    default:
        return ("Unknown Type");
    }
}

/*******************************************************************************
 **
 ** Function       app_tm_vse_get_prox_pairing_type_desc
 **
 ** Description    Get the name of a Proximity Pairing type Event
 **
 ** Parameters     type: Proximity Pairing type
 **                sub_type: Proximity Pairing sub-type
 **
 ** Returns        Type Description (string)
 **
 *******************************************************************************/
char *app_tm_vse_get_prox_pairing_sub_type_desc (UINT8 type, UINT8 sub_type)
{
    switch (type)
    {
    case APP_TM_VSE_PROX_PAIR_TYPE_3DG:
        switch(sub_type)
        {
        case APP_TM_VSE_PROX_PAIR_SUB_TYPE_3DG_MCAST:
            return "Multicast";
        case APP_TM_VSE_PROX_PAIR_SUB_TYPE_3DG_MIP:
            return "MIP";
        default:
            return "Unknown 3DG SubType";
        }
        break;
    case APP_TM_VSE_PROX_PAIR_TYPE_RC:
        switch(sub_type)
        {
        case APP_TM_VSE_PROX_PAIR_SUB_TYPE_RC:
            return "Standard";
        case APP_TM_VSE_PROX_PAIR_SUB_TYPE_RC_VOICE:
            return "Voice";
        default:
            return "Unknown RC SubType";
        }
        break;
    default:
        return ("Unknown Type");
    }
}

/*******************************************************************************
 **
 ** Function        app_tm_vse_decode_association
 **
 ** Description     Decode Association message
 **
 ** Parameters      p_data: Pointer on the buffer to decode
 **                 data_length: Length of the buffer
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_decode_association(UINT8 *p_data, UINT8 data_length)
{
    BD_ADDR bd_addr;
    UINT8 type, reserved, batt_version, batt_remain;
    UINT16 color_shift;

    if (data_length < 8)
    {
        APP_ERROR1("Association message too short length:%d (expected at least 8)", data_length);
        return -1;
    }
    /* Extract BdAddr from data */
    STREAM_TO_BDADDR(bd_addr, p_data);
    STREAM_TO_UINT8(type, p_data);
    STREAM_TO_UINT8(reserved, p_data);

    APP_INFO1("    Association message (Connect) BdAddr:%02X-%02X-%02X-%02X-%02X-%02X",
            bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
    APP_INFO1("    Type:%s(0x%X) SubType:%s(0x%X)",
            app_tm_vse_get_prox_pairing_type_desc(type), type,
            app_tm_vse_get_prox_pairing_sub_type_desc(type, reserved), reserved);

    /* The following fields are decoded for 3DG Multicast (4C) only */
    if ((type != APP_TM_VSE_PROX_PAIR_TYPE_3DG) || (reserved != APP_TM_VSE_PROX_PAIR_SUB_TYPE_3DG_MCAST))
    {
        return 0;
    }

    if (data_length < 13)
    {
        APP_ERROR1("Association message too short length:%d (expected at least 13)", data_length);
        return -1;
    }
    /* Read 3DG Version and Battery Type */
    STREAM_TO_UINT8(batt_version, p_data);

    APP_INFO1("    3D Glasses version=%s(0x%x)",
            (batt_version & 0x0F) == 0x01?"V1.0":"Unknown", batt_version & 0x0F);
    APP_INFO1("    Battery type=%s(0x%x)",
            app_tm_vse_4C_battery_type[batt_version >> 4], batt_version >> 4);

    /* Read Battery remaining */
    STREAM_TO_UINT8(batt_remain, p_data);
    if (batt_remain != 0xFF)
    {
        APP_INFO1("    Voltage: %dmV(0x%x)", batt_remain * 25, batt_remain);
    }
    else
    {
        APP_INFO1("    Voltage: No information(0x%x)", batt_remain);
    }

    /*  Read Battery charging and Remaining ratio/time */
    STREAM_TO_UINT8(reserved, p_data);
    /* Decode Charging field */
    switch(reserved & 0x03)
    {
    case 0x00:
        APP_INFO1("    Charging: not in charging(0x%x)", reserved & 0x03);
        break;
    case 0x02:
        APP_INFO1("    Charging: charging(0x%x)", reserved & 0x03);
        break;
    case 0x1:
        APP_INFO1("    Charging: reserved(0x%x)", reserved & 0x03);
        break;
    default:
        APP_INFO1("    Charging: no info(0x%x)", reserved & 0x03);
        break;
    }
    /* Decode Level (ratio) field */
    reserved = reserved >> 2;
    switch(reserved & 0x07)
    {
    case 0x00:
        APP_INFO1("    Level: Almost 0%%(0x%x)", reserved & 0x07);
        break;
    case 0x04:
        APP_INFO1("    Level: 25%%(0x%x)", reserved & 0x07);
        break;
    case 0x02:
        APP_INFO1("    Level: 50%%(0x%x)", reserved & 0x07);
        break;
    case 0x06:
        APP_INFO1("    Level: Full(0x%x)", reserved & 0x07);
        break;
    case 0x07:
        APP_INFO1("    Level: No info(0x%x)", reserved & 0x07);
        break;
    default:
        APP_INFO1("    Level: reserved(0x%x)", reserved & 0x07);
        break;
    }
    /* Decode Level (time) field */
    reserved = reserved >> 3;
    switch(reserved & 0x07)
    {
    case 0x00:
        APP_INFO1("    Time: < 2 hrs(0x%x)", reserved & 0x07);
        break;
    case 0x04:
        APP_INFO1("    Time: ~ 5 hrs(0x%x)", reserved & 0x07);
        break;
    case 0x02:
        APP_INFO1("    Time: ~ 10 hrs(0x%x)", reserved & 0x07);
        break;
    case 0x06:
        APP_INFO1("    Time: > 10 hrs(0x%x)", reserved & 0x07);
        break;
    case 0x07:
        APP_INFO1("    Time: No info(0x%x)", reserved & 0x07);
        break;
    default:
        APP_INFO1("    Time: reserved(0x%x)", reserved & 0x07);
        break;
    }

    /* Read Color Shift parameter */
    STREAM_TO_UINT16(color_shift, p_data);
    if (color_shift == 0)
    {
        APP_INFO1("    Color: No information(0x%04x)", color_shift);
    }
    else
    {
        APP_INFO1("    Color: dU'=%c%u dV'=%c%u (0x%04x)",
                (color_shift & 0x0080)?'-':'+', color_shift & 0x7F,
                (color_shift & 0x8000)?'-':'+', (color_shift >> 8) & 0x7F,
                color_shift);
    }

    /* Skip the 13 bytes already read */
    data_length -= 13;

    /* Read and print trailing reserved bytes (if present) */
    while(data_length--)
    {
        STREAM_TO_UINT8(reserved, p_data);
        APP_INFO1("    Reserved byte: 0x%02X", reserved);
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vse_decode_mcast_param
 **
 ** Description     Decode VSE Parameters
 **
 ** Parameters      p_vse: VSE data to decode
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_decode_mcast_param(tBSA_TM_VSE_MSG *p_vse)
{
    UINT8 length;
    UINT8 sub_length;
    UINT8 report_length;
    UINT8 *p_data;
    UINT8 report_id;
    UINT8 battery_level;
    UINT8 rssi_level;
    UINT8 model1, model2;
    UINT8 vendor1, vendor2;
    UINT8 version1, version2, version3, version4, version5;
    UINT8 debug;
    BD_ADDR bd_addr;

    if (p_vse == NULL)
    {
        APP_ERROR0("Null VSE pointer");
        return -1;
    }

    length = p_vse->length;
    p_data = p_vse->data;

    /* Extract sub-length from data */
    STREAM_TO_UINT8(sub_length, p_data);
    length--;

    while (length >= 2)   /* Enough remaining bytes in the buffer */
    {
        /* Extract report_length from data */
        STREAM_TO_UINT8(report_length, p_data);

        if (report_length == 0)
        {
            /* This indicates that this is the end */
            break;
        }

        /* Extract report_id from data */
        STREAM_TO_UINT8(report_id, p_data);

        /* Skip report_length, report_id and data fields */
        length -= sizeof(report_length) + report_length;

        switch (report_id)
        {
        /* Battery Level Report */
        case APP_3DG_REPORT_BAT_LEVEL:
            /* Extract battery_level from data */
            STREAM_TO_UINT8(battery_level, p_data);
            APP_INFO1("=> Battery level[id:0x%x]:%d (%d%%)", report_id,
                    battery_level, (battery_level * 100) / 256);
            break;

        /* RSSI Level Report */
        case APP_3DG_REPORT_RSSI:
            /* Extract rssi_level from data */
            STREAM_TO_UINT8(rssi_level, p_data);
            APP_INFO1("=> RSSI level[id:0x%x]:%d", report_id, rssi_level);
            break;

        /* Model Name Report */
        case APP_3DG_REPORT_MODEL:
            /* Extract model1 & model2 from data */
            STREAM_TO_UINT8(model1, p_data);
            STREAM_TO_UINT8(model2, p_data);
            APP_INFO1("=> Model[id:0x%x]:0x%x 0x%x", report_id, model1, model2);
            break;

        /* Vendor Name Report */
        case APP_3DG_REPORT_VENDOR:
            /* Extract vendor1 & vendor2 from data */
            STREAM_TO_UINT8(vendor1, p_data);
            STREAM_TO_UINT8(vendor2, p_data);
            APP_INFO1("=> Vendor[id:0x%x]:0x%x 0x%x", report_id, vendor1, vendor2);
            break;

        /* FW Version Report */
        case APP_3DG_REPORT_FW_VERSION:
            /* Extract FW Version from data */
            STREAM_TO_UINT8(version1, p_data);
            STREAM_TO_UINT8(version2, p_data);
            STREAM_TO_UINT8(version3, p_data);
            STREAM_TO_UINT8(version4, p_data);
            STREAM_TO_UINT8(version5, p_data);
            APP_INFO1("=> FW Version[id:0x%x]:%d.%d.%d.%d.%d", report_id, version1, version2,
                    version3, version4, version5);
            break;

        /* Button Press Report */
        case APP_3DG_REPORT_BUTTON:
            APP_INFO1("=> Button Press[id:0x%x]", report_id);
            break;

        /* BdAddr Report */
        case APP_3DG_REPORT_BDADDR:
            /* Extract BdAddr from data */
            STREAM_TO_BDADDR(bd_addr, p_data);
            APP_INFO1("=> BdAddr[id:0x%x]:%02X:%02X:%02X:%02X:%02X:%02X", report_id,
                    bd_addr[0], bd_addr[1], bd_addr[2], bd_addr[3], bd_addr[4], bd_addr[5]);
            break;

        /* Debug Report */
        case APP_3DG_REPORT_DEBUG:
            /* Extract debug info from data */
            STREAM_TO_UINT8(debug, p_data);
            APP_INFO1("=> Debug[id:0x%x]:%d.%d.%d.%d.%d", report_id, debug);
            break;

        case APP_3DG_REPORT_ASSOCIATION:
            APP_INFO1("=> Reconnection Notification report[id:0x%x] length:%d", report_id, report_length);
            app_tm_vse_decode_association(p_data, report_length - 1);
            break;

        /* Unknown Report Id */
        default:
            APP_INFO1("=> Unknown Report Id[id:0x%x] length:%d", report_id, sub_length);
            APP_DUMP("data", p_data, sub_length);
            break;
        }
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function       app_tm_vse_decode_param
 **
 ** Description    Decode VSE Parameters
 **
 ** Parameters     event: TM Event received
 **                p_data: TM Event's data
 **
 ** Returns        void
 **
 *******************************************************************************/
static void app_tm_vse_decode_param (tBSA_TM_VSE_MSG *p_vse)
{
    UINT8 length;
    UINT8 *p_data = p_vse->data;
    UINT16 frame_period;
    UINT8 frame_period_fraction;
    UINT8 frame_period_initial;
    UINT8 lock_status;

    APP_DEBUG1("BSA_TM_VSE_EVT (generic) received SubEvent:0x%X", p_vse->sub_event);

    length = p_vse->length;

    switch (p_vse->sub_event)
    {
    case APP_TM_VSE_3D_MODE_CHANGE:
        /* No parameters */
        APP_INFO0("3D_MODE_CHANGE");
        break;

    case APP_TM_VSE_3D_FRAME_PERIOD:
        /* Extract TAvg, TAvg_fraction and Innitial_measurement parameters */
        STREAM_TO_UINT16(frame_period, p_data);
        STREAM_TO_UINT8(frame_period_fraction, p_data);
        STREAM_TO_UINT8(frame_period_initial, p_data);
        APP_INFO1("3D_FRAME_PERIOD Period:%d PeriodFraction:%d PeriodInitial:%d",
                frame_period, frame_period_fraction, frame_period_initial);
        break;

    case APP_TM_VSE_3D_CLOCK_SNAPSHOT:
        /* No more used */
        APP_INFO0("3D_CLOCK_SNAPSHOT");
        break;

    case APP_TM_VSE_3D_MULTICAST_LOCK_CPLT:
        /* Extract Lock Status */
        STREAM_TO_UINT8(lock_status, p_data);
        APP_INFO1("3D_MULTICAST_LOCK_CPLT LockStatus:%d", lock_status);
        break;

    case APP_TM_VSE_3D_MULTICAST_SYNC_TO:
        /* No parameters */
        APP_INFO0("3D_MULTICAST_SYNC_TO");
        break;

    case APP_TM_VSE_3D_EIR_HANDSHAKE_CPLT:
        APP_INFO0("3D_EIR_HANDSHAKE_CPLT (Association Notification)");
        app_tm_vse_decode_association(p_data, length);
        break;

    case APP_TM_VSE_3D_MULTICAST_RX_DATA:
        APP_INFO0("3D_MULTICAST_RX_DATA");
        /* Decode Received Multicast Data */
        app_tm_vse_decode_mcast_param(p_vse);
        break;

    case APP_TM_VSE_NOTI_WLAN_RESET:
        APP_INFO0("WLAN RESET notification!");
        break;

    case APP_TM_VSE_FW_CORE_DUMP:
        APP_INFO0("FW_CORE_DUMP");

        if (app_tm_vse_cb.erase_fw_core_dump)
        {
            app_tm_vse_cb.erase_fw_core_dump = FALSE;

            APP_INFO1("Writing FW Core Dump in %s", APP_TM_VSE_FW_CORE_DUMP_FILENAME);

            /* Erase last FW Core Dump log file */
            app_tm_vse_fw_core_dump_erase();

            /* Open the FW Core Dump log file (in append mode) */
            app_tm_vse_fw_core_dump_open();

            /* Write Header in FW Core Dump log file */
            app_tm_vse_fw_core_dump_write_header();
        }
        else
        {
            /* Open the FW Core Dump log file (in append mode) */
            app_tm_vse_fw_core_dump_open();

            /* Write the data into the FW Core Dump log file */
            app_tm_vse_fw_core_dump_write_data(p_vse->data, p_vse->length);
        }

        /* Close the FW Core Dump log file */
        app_tm_vse_fw_core_dump_close();
        break;

    default:
        APP_DEBUG1("Unknown SubEvent:0x%x", p_vse->sub_event);
        if (length > 0)
        {
            APP_DUMP("SubEvent data", p_data, length);
        }
        break;
    }
}

/*******************************************************************************
 **
 ** Function       app_tm_vse_generic_callback
 **
 ** Description    Default TM Callback
 **
 ** Parameters     event: TM Event received
 **                p_data: TM Event's data
 **
 ** Returns        void
 **
 *******************************************************************************/
static void app_tm_vse_generic_callback (tBSA_TM_EVT event, tBSA_TM_MSG *p_data)
{
    BOOLEAN exit_generic_cback= FALSE;

    if (p_data == NULL)
    {
        APP_ERROR0("Null data pointer");
        return;
    }

    /* If Application provided a TM Callback */
    if (app_tm_vse_cb.p_callback != NULL)
    {
        exit_generic_cback = app_tm_vse_cb.p_callback(event, p_data);
    }

    if (exit_generic_cback != FALSE)
    {
        return;
    }

    switch(event)
    {
    case BSA_TM_VSE_EVT:
        /* Decode VSE Parameters (for debug) */
        app_tm_vse_decode_param(&p_data->vse);
        break;

    default:
        APP_ERROR1("Unknown Event:%d received", event);
        APP_DUMP("Data", p_data->vse.data, p_data->vse.length);
        break;
    }

}

/*******************************************************************************
 **
 ** Function        app_tm_vse_register
 **
 ** Description     This function is used to Register Vendor Specific Events
 **
 ** Parameters      sub_event: sub_event to register. If this parameter is -1,
 **                            the function will prompt the user to enter a value
 **                 p_callback: VSE Callback function to call
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_register(int sub_event, tAPP_TM_CUSTOM_CBACK p_callback)
{
    tBSA_TM_VSE_REGISTER bsa_register;
    tBSA_STATUS bsa_status;

    APP_DEBUG1("sub_event=%d", sub_event);

    if (sub_event < 0)
    {
        APP_INFO0("Enter SubEvent to register");
        APP_INFO0("\t0..255 to register one SubEvent Only");
        APP_INFO0("\t65535 to register every SubEvents");
        sub_event = app_get_choice("");
    }

    /* Save user's callback */
    app_tm_vse_cb.p_callback = p_callback;

    /* Initialize parameters */
    BSA_TmVseRegisterInit(&bsa_register);

    /*
     * We use a default callback for demo purpose (decode and print messages),
     * but the callback passed in parameter is called in every case
     */
    bsa_register.p_callback = app_tm_vse_generic_callback;
    bsa_register.sub_event = sub_event;
    bsa_status = BSA_TmVseRegister(&bsa_register);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVseRegister failed:%d", bsa_status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vse_deregister
 **
 ** Description     This function is used to Deregister Vendor Specific Events
 **
 ** Parameters      sub_event: sub_event to deregister. If this parameter is -1,
 **                            the function will prompt the user to enter a value
 **                 deregister_callback: indicates if the VSE callback must be deregistered
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_deregister(int sub_event, BOOLEAN deregister_callback)
{
    tBSA_TM_VSE_DEREGISTER bsa_deregister;
    tBSA_STATUS bsa_status;

    APP_DEBUG0("");
    if (sub_event < 0)
    {
        APP_INFO0("Enter SubEvent to deregister");
        APP_INFO0("\t0..255 to deregister one SubEvent Only");
        APP_INFO0("\t65535 to deregister every SubEvents");
        sub_event = app_get_choice("");
    }

    /* Save user's callback */
    if (deregister_callback)
    {
        app_tm_vse_cb.p_callback = NULL;
    }

    /* Initialize parameters */
    BSA_TmVseDeregisterInit(&bsa_deregister);
    bsa_deregister.sub_event = sub_event;
    bsa_deregister.deregister_callback = deregister_callback;
    bsa_status = BSA_TmVseDeregister(&bsa_deregister);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVseDeregister failed status:%d", bsa_status);
        return -1;
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_erase
 **
 ** Description     Erase last FW core dump file
 **
 ** Parameters      None
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_erase(void)
{
    APP_DEBUG1("Erasing FW Core Dump file:%s", APP_TM_VSE_FW_CORE_DUMP_FILENAME);
    unlink(APP_TM_VSE_FW_CORE_DUMP_FILENAME);
    app_tm_vse_cb.fw_core_dump_file = NULL;
    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_open
 **
 ** Description     Open FW core dump file in Append mode
 **
 ** Parameters      None
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_open(void)
{
    if (app_tm_vse_cb.fw_core_dump_file != NULL)
    {
        APP_ERROR1("FW Core Dump file:%s was already open", APP_TM_VSE_FW_CORE_DUMP_FILENAME);
        return 0;
    }

    app_tm_vse_cb.fw_core_dump_file = fopen(APP_TM_VSE_FW_CORE_DUMP_FILENAME, "a");
    if (app_tm_vse_cb.fw_core_dump_file == NULL)
    {
        APP_ERROR1("fopen(%s) failed (errno:%d)", APP_TM_VSE_FW_CORE_DUMP_FILENAME, errno);
        return -1;
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_close
 **
 ** Description     Close FW core dump file
 **
 ** Parameters      None
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_close(void)
{
    int rv;

    if (app_tm_vse_cb.fw_core_dump_file == NULL)
    {
        APP_ERROR1("FW Core Dump file:%s is not opened", APP_TM_VSE_FW_CORE_DUMP_FILENAME);
        return 1;
    }

    rv = fclose(app_tm_vse_cb.fw_core_dump_file);
    app_tm_vse_cb.fw_core_dump_file = NULL;
    if (rv < 0)
    {
        APP_ERROR1("Unable to close:%s (errno:%d)", APP_TM_VSE_FW_CORE_DUMP_FILENAME, errno);
        return -1;
    }

    return 0;

}

/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_write_header
 **
 ** Description     Write header into FW core dump file
 **
 ** Parameters      None
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_write_header(void)
{
    struct timeval timeofday;
    struct tm timeinfo;
    char time_string[200];

    if (app_tm_vse_cb.fw_core_dump_file == NULL)
    {
        APP_ERROR1("FW Core Dump file:%s not open", APP_TM_VSE_FW_CORE_DUMP_FILENAME);
        return 1;
    }

    gettimeofday(&timeofday, NULL);
    localtime_r(&timeofday.tv_sec, &timeinfo);

    strftime(time_string,sizeof(time_string),"%d/%m/%y  %H:%M:%S", &timeinfo);

    fprintf(app_tm_vse_cb.fw_core_dump_file, "#\n# FW Core Dump Generated\n");

    fprintf(app_tm_vse_cb.fw_core_dump_file,
            "#\n# Date (d/m/y) %s.%03d\n#\n",
            time_string, (int)(timeofday.tv_usec / 1000));

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_vse_fw_core_dump_write_data
 **
 ** Description     Write data into FW core dump file
 **
 ** Parameters      p_buf:pointer on the VSE data
 **                 buf_length: data length
 **
 ** Returns         0 if success/-1 otherwise
 **
 *******************************************************************************/
int app_tm_vse_fw_core_dump_write_data(UINT8 *p_buf, int buf_length)
{
    struct timeval timeofday;
    struct tm timeinfo;
    char time_string[200];
    int index;

    if (app_tm_vse_cb.fw_core_dump_file == NULL)
    {
        APP_ERROR1("FW Core Dump file:%s not open", APP_TM_VSE_FW_CORE_DUMP_FILENAME);
        return 1;
    }

    gettimeofday(&timeofday, NULL);
    localtime_r(&timeofday.tv_sec, &timeinfo);

    strftime(time_string,sizeof(time_string),"%d/%m/%y  %H:%M:%S", &timeinfo);

    buf_length++;

    fprintf(app_tm_vse_cb.fw_core_dump_file,
            "\nusb0: %s.%03d\nHCIEvent\n{[FF %02X]:\n",
            time_string, (int)(timeofday.tv_usec / 1000), buf_length);

    for (index = 0; index < buf_length; index++)
    {
        if (index == 0)
        {
            fprintf(app_tm_vse_cb.fw_core_dump_file, " F7");
        }
        else
        {
            /* Newline every 32 bytes */
            if ((index % 32) == 0)
            {
                fprintf(app_tm_vse_cb.fw_core_dump_file, "\n");
            }
            fprintf(app_tm_vse_cb.fw_core_dump_file, " %02X", *p_buf++);
        }
    }
    fprintf(app_tm_vse_cb.fw_core_dump_file, "}\n\n#----------------------------------------------------------------------------\n");

    return 0;
}
