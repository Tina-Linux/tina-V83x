/*****************************************************************************
**
**  Name:           app_tm.c
**
**  Description:    Bluetooth Test Module application
**
**  Copyright (c) 2009-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "bsa_api.h"
#include "bsa_trace.h"
#include "app_tm.h"
#include "app_tm_vsc.h"
#include "app_utils.h"

/*
 * Definitions
 */

/* Item definitions for Static Section parsing */
#define APP_TM_SS_ITEM_IFPLL            0x01    /* IF_PLL Item */
#define APP_TM_SS_ITEM_IFPLL_LEN        0x08    /* IF_PLL Item length */
#define APP_TM_SS_ITEM_BDADDR           0x40    /* BdAddr Item */
#define APP_TM_SS_ITEM_BDADDR_LEN       0x06    /* BdAddr Item length */
#define APP_TM_SS_ITEM_INVALID_TAG      0xFF    /* Invalid Tag */
#define APP_TM_SS_SERIAL_ADDR           0xFF000000  /* Flash SS location */

#define APP_TM_S_TO_U32(p) (UINT32)(*(p)) + ((UINT32)(*(p+1))<<8) + ((UINT32)(*(p+2))<<16) + ((UINT32)(*(p+3))<<24)
#define APP_TM_S_TO_U16(p) (UINT16)(*(p)) + ((UINT16)(*(p+1))<<8)
#define APP_TM_S_TO_U8(p) (UINT8)(*(p))

/*******************************************************************************
 **
 ** Function         app_tm_set_test_mode
 **
 ** Description      This function is used to set the Bluetooth Test Mode
 **
 ** Parameters
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
int app_tm_set_test_mode(BOOLEAN test_mode)
{
    tBSA_TM_SET_TEST_MODE  bsa_set_test_mode;
    tBSA_STATUS bsa_status;

    /*
     * Set Test Mode
     */
    bsa_status = BSA_TmSetTestModeInit(&bsa_set_test_mode);
    bsa_set_test_mode.test_mode_enable = test_mode;
    bsa_status = BSA_TmSetTestMode(&bsa_set_test_mode);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmSetTestMode failed mode_requested:%d status:%d", test_mode, bsa_status);
        return(-1);
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         app_tm_disconnect
 **
 ** Description      Disconnect blindly a device
 **
 ** Parameters       bda: address of the device to disconnect - if NULL, prompted
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
int app_tm_disconnect(BD_ADDR bda)
{
    tBSA_TM_DISCONNECT DisconnectReq;

    /* Disconnect this link */
    BSA_TmDisconnectInit(&DisconnectReq);

    if (bda == NULL)
    {
        APP_INFO0("Enter the BD address to disconnect(AA.BB.CC.DD.EE.FF): ");
        /* coverity[SECURE_CODING] False-positive: used with precision specifiers */
        if (scanf("%hhx.%hhx.%hhx.%hhx.%hhx.%hhx",
                &DisconnectReq.bd_addr[0], &DisconnectReq.bd_addr[1],
                &DisconnectReq.bd_addr[2], &DisconnectReq.bd_addr[3],
                &DisconnectReq.bd_addr[4], &DisconnectReq.bd_addr[5]) != 6)
        {
            APP_ERROR0("BD address not entered correctly");
            return -1;
        }
    }
    else
    {
        bdcpy(DisconnectReq.bd_addr, bda);
    }
    if (BSA_TmDisconnect(&DisconnectReq) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_TmDisconnect failed");
        return -1;
    }
    return 0;
}


/*******************************************************************************
 **
 ** Function        app_tm_qos_setup
 **
 ** Description     This function sends the following VSC:
 **                 QoS Setup Command
 **
 ** Parameters      Pointer on structure containing the parameters.
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_qos_setup(void)
{
    tBSA_TM_QOS QosReq;

    /* Configure QoS of this link */
    BSA_TmQosInit(&QosReq);

    QosReq.flag = app_get_choice("flag");
    QosReq.service_type  = app_get_choice("service_type");
    QosReq.token_rate = app_get_choice("token_rate");
    QosReq.peak_bandwidth = app_get_choice("peak_bandwidth");
    QosReq.latency = app_get_choice("latency");
    QosReq.delay_variation = app_get_choice("delay_variation");

    APP_INFO0("Enter the BD address to configure QoS (AA.BB.CC.DD.EE.FF): ");
    /* coverity[SECURE_CODING] False-positive: used with precision specifiers */
    if (scanf("%hhx.%hhx.%hhx.%hhx.%hhx.%hhx",
            &QosReq.bd_addr[0], &QosReq.bd_addr[1],
            &QosReq.bd_addr[2], &QosReq.bd_addr[3],
            &QosReq.bd_addr[4], &QosReq.bd_addr[5]) != 6)
    {
        APP_ERROR0("BD address not entered correctly");
        return -1;
    }

    if (BSA_TmQos(&QosReq) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_TmQos failed");
        return -1;
    }
    return 0;
}
/*******************************************************************************
 **
 ** Function        app_tm_vsc_read_raw_rssi
 **
 ** Description     This function sends the following VSC:
 **                 Read Raw RSSI
 **
 ** Parameters      Pointer on structure containing the parameters.
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_read_raw_rssi(BD_ADDR bda)
{
    tBSA_TM_READRAWRSSI ReadRssiReq;
    UINT8 *p_data;
    UINT8 status = 0;
    INT8 rssi = 0;
    UINT16 handle = 0;

    /* Disconnect this link */
    BSA_TmReadRawRssiInit(&ReadRssiReq);

    if (bda == NULL)
    {
        APP_INFO0("Enter the BD address of remote connected device(AA.BB.CC.DD.EE.FF): ");
        /* coverity[SECURE_CODING] False-positive: used with precision specifiers */
        if (scanf("%hhx.%hhx.%hhx.%hhx.%hhx.%hhx",
                &ReadRssiReq.bd_addr[0], &ReadRssiReq.bd_addr[1],
                &ReadRssiReq.bd_addr[2], &ReadRssiReq.bd_addr[3],
                &ReadRssiReq.bd_addr[4], &ReadRssiReq.bd_addr[5]) != 6)
        {
            APP_ERROR0("BD address not entered correctly");
            return -1;
        }
    }
    else
    {
        bdcpy(ReadRssiReq.bd_addr, bda);
    }

    ReadRssiReq.opcode = HCI_VSC_OPCODE_READ_RAW_RSSI;
    ReadRssiReq.length = 0x02;

    if (BSA_TmReadRawRssi(&ReadRssiReq) != BSA_SUCCESS)
    {
        APP_ERROR0("BSA_TmReadRawRssi failed");
        return -1;
    }

    scru_dump_hex(ReadRssiReq.data, "Read Raw RSSI result",
            ReadRssiReq.length, TRACE_LAYER_NONE, TRACE_TYPE_DEBUG);

    p_data = ReadRssiReq.data;
    STREAM_TO_UINT8(status, p_data);
    STREAM_TO_UINT16(handle, p_data);
    rssi = ReadRssiReq.data[3];

    APP_INFO1("\tStatus:\t\t\t%d", status);
    APP_INFO1("\tConnection handle:\t0x%02X (%d)", handle, handle);
    APP_INFO1("\tRSSI:\t\t\t%d (dB)", rssi);

    return 0;
}

/*******************************************************************************
 **
 ** Function        app_tm_read_rssi
 **
 ** Description     This function sends the Read RSSI HCI command
 **
 ** Parameters      bda: address of the device to read RSSI
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_read_rssi(BD_ADDR bda)
{
    tBSA_TM_GET_HANDLE conn_handle;
    tBSA_TM_HCI_CMD hci_cmd;

    UINT8 *p_data;
    INT8 rssi = 0;
    UINT16 handle = 0;

    BSA_TmGetHandleInit(&conn_handle);

    conn_handle.transport = app_get_choice("Select transport 1:BR_EDR, 2:LE");
    if ((conn_handle.transport != BT_TRANSPORT_BR_EDR) &&
        (conn_handle.transport != BT_TRANSPORT_LE))
    {
        APP_ERROR1("Wrong Transport value:%d", conn_handle.transport);
        return -1;
    }
    if (bda == NULL)
    {
        APP_INFO0("Enter the BD address of remote connected device(AA.BB.CC.DD.EE.FF): ");
        /* coverity[SECURE_CODING] False-positive: used with precision specifiers */
        if (scanf("%hhx.%hhx.%hhx.%hhx.%hhx.%hhx",
                &conn_handle.bd_addr[0], &conn_handle.bd_addr[1],
                &conn_handle.bd_addr[2], &conn_handle.bd_addr[3],
                &conn_handle.bd_addr[4], &conn_handle.bd_addr[5]) != 6)
        {
            APP_ERROR0("BD address not entered correctly");
            return -1;
        }
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

    APP_DEBUG1("handle value:0x%x", conn_handle.handle);

    BSA_TmHciInit(&hci_cmd);

    hci_cmd.opcode = HCI_READ_RSSI;
    hci_cmd.length = 2;
    hci_cmd.no_opcode_swap = TRUE;
    p_data = hci_cmd.data;

    UINT16_TO_STREAM(p_data, conn_handle.handle);

    BSA_TmHciCmd(&hci_cmd);

    p_data = hci_cmd.data;
    STREAM_TO_UINT16(handle, p_data);
    STREAM_TO_UINT8(rssi, p_data);

    APP_INFO1("\tStatus:\t\t\t%d", hci_cmd.status);
    APP_INFO1("\tHandle:\t\t\t%d", handle);
    APP_INFO1("\tRSSI:\t\t\t%d (dB)", rssi);

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_tm_get_mem_usage
 **
 ** Description      This function is used to get Server's task and memory usage
 **
 ** Parameters
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_get_mem_usage(UINT8 location)
{
    tBSA_TM_GET_MEM_USAGE  bsa_get_mem_usage;
    tBSA_STATUS bsa_status;
    int index;
    /*
     * Set Test Mode
     */
    bsa_status = BSA_TmGetMemUsageInit(&bsa_get_mem_usage);
    bsa_get_mem_usage.location = location;
    bsa_status = BSA_TmGetMemUsage(&bsa_get_mem_usage);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmGetMemUsage failed status:%d", bsa_status);
        return(-1);
    }

    /* Print Task info */
    if (location == BSA_TM_SERVER)
    {
        APP_INFO0("BSA Server's task info");
    }
    else
    {
        APP_INFO0("BSA Client's task info");
    }
    APP_INFO1("%6s%20s%10s%10s%10s", "TaskId", "TaskName", "TaskState",
            "StackSize", "StackUsed");
    for (index = 0 ; index < BSA_TM_TASK_MAX ; index++)
    {
        if (bsa_get_mem_usage.task[index].task_used)
        {
            APP_INFO1("%6d%20s%10d%10d%10d",
                   bsa_get_mem_usage.task[index].task_id,
                   bsa_get_mem_usage.task[index].task_name,
                   bsa_get_mem_usage.task[index].task_state,
                   bsa_get_mem_usage.task[index].stack_size,
                   bsa_get_mem_usage.task[index].stack_used);
        }
    }
    if (bsa_get_mem_usage.task_missing)
    {
        APP_INFO0("One or several Server task info are missing");
    }

    /* Print GKI Buffer info */
    if (location == BSA_TM_SERVER)
    {
        APP_INFO0("BSA Server's GKI Buffer info:");
    }
    else
    {
        APP_INFO0("BSA Client's GKI Buffer info:");
    }
    APP_INFO0("PoolId BufferSize NbBuffer CurUsed MaxUsed");
    for (index = 0 ; index < BSA_TM_TASK_MAX ; index++)
    {
        if (bsa_get_mem_usage.gki_buffer[index].pool_used)
        {
            APP_INFO1("%6d%11d%9d%8d%8d",
                   bsa_get_mem_usage.gki_buffer[index].pool_id,
                   bsa_get_mem_usage.gki_buffer[index].pool_size,
                   bsa_get_mem_usage.gki_buffer[index].pool_number,
                   bsa_get_mem_usage.gki_buffer[index].pool_cur_use,
                   bsa_get_mem_usage.gki_buffer[index].pool_max_use);
        }
    }
    if (bsa_get_mem_usage.gki_buf_missing)
    {
        APP_ERROR0("One or several Server GKI Buffer Pool info are missing");
    }

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_tm_ping
 **
 ** Description      This function is used to ping the BSA server
 **
 ** Parameters
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_ping(void)
{
    tBSA_TM_PING bsa_ping;
    tBSA_STATUS bsa_status;

    /*
     * Ping
     */
    bsa_status = BSA_TmPingInit(&bsa_ping);
    bsa_status = BSA_TmPing(&bsa_ping);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmPing failed status:%d", bsa_status);
        return(-1);
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         app_tm_vsc
 **
 ** Description      This function is used to send a VendorSpecificCommand
 **
 ** Parameters
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_vsc(void)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;

    /* Send VSC */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = 0x0001;          /* Write BD_ADDR SCV */
    bsa_vsc.length = sizeof(BD_ADDR); /* Data Len (BdAddr) */
    memset(bsa_vsc.data, 0x55, sizeof(BD_ADDR)); /* Addr = 55 55 ... 55 */
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed status:%d", bsa_status);
        return(-1);
    }

    APP_DEBUG1("VSC Server status:%d", bsa_vsc.status);
    APP_DEBUG1("VSC opcode:0x%03X", bsa_vsc.opcode);
    APP_DEBUG1("VSC length:%d", bsa_vsc.length);
    APP_DEBUG1("VSC Chip/HCI status:0x%02X", bsa_vsc.data[0]);
#ifndef APP_TRACE_NODEBUG
    /* Dump the Received buffer */
    if ((bsa_vsc.length - 1) > 0)
    {
        scru_dump_hex(&bsa_vsc.data[1], "VSC Data", bsa_vsc.length - 1, TRACE_LAYER_NONE,
                TRACE_TYPE_DEBUG);
    }
#endif

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_tm_set_trace
 **
 ** Description      This function is used to set the trace level
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_set_trace(int level)
{
    tBSA_TM_SET_TRACE_LEVEL trace_level_req;
    tBSA_STATUS status;

    BSA_TmSetTraceLevelInit(&trace_level_req);

    if ((level < BSA_TM_TRACE_LEVEL_NONE) ||
        (level > BSA_TM_TRACE_LEVEL_DEBUG))
    {
        APP_ERROR1("app_tm_set_trace bad trace level:%d", level);
        return -1;
    }
    trace_level_req.all_trace_level = level;

    status = BSA_TmSetTraceLevel(&trace_level_req);
    if (status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmSetTraceLevel failed status:%d", status);
        return -1;
    }
    return 0;

}

/*******************************************************************************
 **
 ** Function         app_tm_read_version
 **
 ** Description      This function is used to read version
 **
 ** Parameters
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_read_version(void)
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
 ** Function         app_tm_read_ram
 **
 ** Description      Read and display RAM
 **
 ** Returns          Status
 **
 *******************************************************************************/
int app_tm_read_ram(void)
{
    tBSA_TM_VSC vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;
    UINT32 addr;
    UINT32 length;

    APP_DEBUG0("");

    /* Initialize Static Section address for Serial Flash */
    addr = app_get_choice("Enter the address to read at");
    length = app_get_choice("Enter the length to read");

    /* Send a Read RAM VSC */
    bsa_status = BSA_TmVscInit(&vsc);
    vsc.opcode = HCI_READ_RAM_VSC;
    vsc.length = 5;
    p_data = &vsc.data[0];
    UINT32_TO_STREAM(p_data, addr);
    UINT8_TO_STREAM(p_data, length);
    bsa_status = BSA_TmVsc(&vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed status:%d", bsa_status);
        return(-1);
    }

    /* Check the command status */
    if (vsc.data[0] != 0)
    {
        APP_ERROR1("VSC command returned failure: %d", vsc.data[0]);
        return(-1);
    }

    /* Dump received buffer for debug purpose */
    APP_DUMP("VSC", &vsc.data[1], length);

    return 0;
}

/*******************************************************************************
 **
 ** Function         app_tm_read_conn
 **
 ** Description      Read and parse the ACL table
 **
 ** Returns          Status
 **
 *******************************************************************************/
int app_tm_read_conn(void)
{
    tBSA_TM_VSC vsc1;
    tBSA_TM_VSC vsc2;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;
    UINT32 addr;
    UINT32 length = 32*7;
    int i;
    const char *txstates[] = {"new", "noconn", "lmpsent", "lmpack", "l2capsent", "zlpsent"};
    UINT8 tmp8;
    UINT16 tmp16;

    APP_DEBUG0("");

    /* Initialize the address at which the connection structure is located */
    addr = 0x8297c;

    /* Send a Read RAM VSC */
    bsa_status = BSA_TmVscInit(&vsc1);
    vsc1.opcode = HCI_READ_RAM_VSC;
    vsc1.length = 5;
    p_data = &vsc1.data[0];
    UINT32_TO_STREAM(p_data, addr);
    UINT8_TO_STREAM(p_data, length);
    bsa_status = BSA_TmVsc(&vsc1);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed status:%d", bsa_status);
        return(-1);
    }

    /* Check the command status */
    if (vsc1.data[0] != 0)
    {
        APP_ERROR1("VSC command returned failure: %d", vsc1.data[0]);
        return(-1);
    }

    bsa_status = BSA_TmVscInit(&vsc2);
    vsc2.opcode = HCI_READ_RAM_VSC;
    vsc2.length = 5;
    p_data = &vsc2.data[0];
    UINT32_TO_STREAM(p_data, addr+length);
    UINT8_TO_STREAM(p_data, length);
    bsa_status = BSA_TmVsc(&vsc2);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed status:%d", bsa_status);
        return(-1);
    }

    /* Check the command status */
    if (vsc2.data[0] != 0)
    {
        APP_ERROR1("VSC command returned failure: %d", vsc2.data[0]);
        return(-1);
    }

    for (i = 0; i < 13; i++)
    {
        if (i < 7)
        {
            p_data = &vsc1.data[1 + (32*i)];
        }
        else
        {
            p_data = &vsc2.data[1 + (32*(i-7))];
        }
        /* Check if the channel is opened */
        tmp8 = APP_TM_S_TO_U8(p_data+31);
        if (tmp8 != 1)
        {
            APP_INFO1("ch %d -> %s", i, txstates[tmp8]);
            APP_INFO1("  hs = x%08x, he = x%08x, ll = x%08x, lfl = x%08x",
                    APP_TM_S_TO_U32(p_data), APP_TM_S_TO_U32(p_data+4), APP_TM_S_TO_U32(p_data+8), APP_TM_S_TO_U32(p_data+12));
            p_data += 16;
            APP_INFO1("  fasttrack = %d, start_time = x%08x",
                    APP_TM_S_TO_U8(p_data), APP_TM_S_TO_U32(p_data+4));
            p_data += 8;
            APP_INFO1("  flush_mode = %d, flush_state = %d",
                    APP_TM_S_TO_U8(p_data), APP_TM_S_TO_U8(p_data+1));
            p_data += 2;
            tmp8 = APP_TM_S_TO_U8(p_data);
            APP_INFO1("  pktArrIndex = %d, pktDecIndex = %d", tmp8 & 0xF, tmp8 >> 4);
            p_data += 1;
            APP_INFO1("  hostCredits = %d, NumBufOnList = %d", APP_TM_S_TO_U8(p_data), APP_TM_S_TO_U8(p_data));
            p_data += 2;
            tmp16 = APP_TM_S_TO_U16(p_data);
            APP_INFO1("v=x%04x -> txstate = %d, l2cflowl = %d, l2cflowp = %d", tmp16, tmp16 & 7, (tmp16 >> 3) & 1, (tmp16 >> 4) & 1);
            APP_INFO1("v=x%04x -> rxstate = %d, l2cflow  = %d, l2cflowc = %d, l2crel = %d, zlpsent = %d", tmp16,
                    (tmp16 >>5) & 7, (tmp16 >> 8) & 1, (tmp16 >> 9) & 1, (tmp16 >> 10) & 1, (tmp16 >> 11) & 1);
        }
    }
    return 0;
}

/*******************************************************************************
 **
 ** Function         app_tm_read_flash_bd_addr
 **
 ** Description      This function is used to read BD address from flash
 **
 ** Parameters       None
 **
 ** Returns          status
 **
 *******************************************************************************/
int app_tm_read_flash_bd_addr(void)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p;
    UINT32 addr;
    BD_ADDR flash_bd_addr;
    int fsm;

    /* Initialize the address to search for SS location */
    addr = 0xFC03F000;

    fsm = 0;
    do
    {
        /* Send a Super Peek Poke */
        bsa_status = BSA_TmVscInit(&bsa_vsc);
        bsa_vsc.opcode = 0x0A;
        bsa_vsc.length = 5;
        p = &bsa_vsc.data[0];
        UINT8_TO_STREAM(p, 4);
        UINT32_TO_STREAM(p, addr);

        bsa_status = BSA_TmVsc(&bsa_vsc);
        if (bsa_status != BSA_SUCCESS)
        {
            APP_ERROR1("BSA_TmVsc failed status:%d", bsa_status);
            return(-1);
        }

        /* Check the command status */
        if (bsa_vsc.data[0] != 0)
        {
            APP_ERROR1("VSC command returned failure: %d", bsa_vsc.data[0]);
            return(-1);
        }

        switch (fsm)
        {
        case 0: /* searching for valid SS (starting with IFPLL cfg item) */
            if (bsa_vsc.data[1] == 0x01)
            {
                APP_DEBUG1("Found valid SS location at addr 0x%08lX", addr);
                /* read length to bypass item */
                fsm = 2;
                addr += 1;
            }
            else
            {
                /* Search in the next possible SS location */
                switch(addr)
                {
                case 0xFC03F000:
                    addr = 0xFC07E000;
                    break;
                case 0xFC07E000:
                    addr = 0xFC07F000;
                    break;
                case 0xFC07F000:
                    APP_ERROR0("Could not find valid SS location");
                    return -1;
                    break;
                default:
                    APP_ERROR1("unexpected addr 0x%08lX", addr);
                    return -1;
                    break;
                }
            }
            break;

        case 1: /* reading ID */
            if (bsa_vsc.data[1] == 0x40)
            {
                /* read first byte of BD address */
                fsm = 3;
                addr += 3;
            }
            else
            {
                /* read length to bypass item */
                fsm = 2;
                addr += 1;
            }
            break;

        case 2: /* reading length to bypass current item */
            fsm = 1;
            addr += bsa_vsc.data[1] + 2;
            break;
        case 3: /* read 1st byte of BD address */
        case 4: /* read 2nd byte of BD address */
        case 5: /* read 3rd byte of BD address */
        case 6: /* read 4th byte of BD address */
        case 7: /* read 5th byte of BD address */
        case 8: /* read 6th byte of BD address */
            /* write backwards in BD address to restore original order */
            flash_bd_addr[8 - fsm] = bsa_vsc.data[1];
            fsm++;
            addr++;
            break;
        default:
            APP_ERROR1("unexpected state %d", fsm);
            return -1;
            break;
        }
    } while (fsm != 9);


    APP_INFO1("Flash BD address: %02X-%02X-%02X-%02X-%02X-%02X",
            flash_bd_addr[0], flash_bd_addr[1], flash_bd_addr[2],
            flash_bd_addr[3], flash_bd_addr[4], flash_bd_addr[5]);
    return 0;
}

/*******************************************************************************
 **
 ** Function         app_tm_read_serial_flash_bd_addr
 **
 ** Description      This function is used to read BD address from a Serial Flash
 **
 ** Parameters       bd_addr: BdAddr buffer to be updated
 **
 ** Returns          Status
 **
 *******************************************************************************/
int app_tm_read_serial_flash_bd_addr(BD_ADDR bd_addr)
{
    tBSA_TM_VSC bsa_vsc;
    tBSA_STATUS bsa_status;
    UINT8 *p_data;
    BD_ADDR flash_bd_addr;
    UINT32 addr;
    UINT32 length = 100;

    APP_DEBUG0("");

    /* Initialize Static Section address for Serial Flash */
    addr = APP_TM_SS_SERIAL_ADDR;

    /* Send a Read RAM VSC (to read 100 byte starting at addr) */
    bsa_status = BSA_TmVscInit(&bsa_vsc);
    bsa_vsc.opcode = HCI_READ_RAM_VSC;
    bsa_vsc.length = 5;
    p_data = &bsa_vsc.data[0];
    UINT32_TO_STREAM(p_data, addr);
    UINT8_TO_STREAM(p_data, length);
    bsa_status = BSA_TmVsc(&bsa_vsc);
    if (bsa_status != BSA_SUCCESS)
    {
        APP_ERROR1("BSA_TmVsc failed status:%d", bsa_status);
        return(-1);
    }

    /* Check the command status */
    if (bsa_vsc.data[0] != 0)
    {
        APP_ERROR1("VSC command returned failure: %d", bsa_vsc.data[0]);
        return(-1);
    }

    /* Dump received buffer for debug purpose */
    /* APP_DUMP("Data", bsa_vsc.data, length); */

    /* Get Pointer on the first byte of the data */
    p_data = &bsa_vsc.data[1];
    length--;

    /* Sanity: check that it's a Static Section (must start with IF_PLL tuple) */
    if ((p_data[0] != APP_TM_SS_ITEM_IFPLL) ||   /* Item Id must be 0x01 */
        (p_data[1] != APP_TM_SS_ITEM_IFPLL_LEN)) /* Length must be 0x08 */
    {
        APP_ERROR0("IF_PLL item not found => this is not a Static Section");
        return -1;
    }

    while(length > 0)
    {
        /* If this item is a BdAddr and length is 6 bytes */
        if ((p_data[0] == APP_TM_SS_ITEM_BDADDR) &&
            (p_data[1] == APP_TM_SS_ITEM_BDADDR_LEN))
        {
            /* jump item_id, length, status */
            p_data += 3;
            /* Extract BdAddr (swap bytes) */
            STREAM_TO_BDADDR(flash_bd_addr, p_data);
            APP_INFO1("Flash BD address: %02X-%02X-%02X-%02X-%02X-%02X",
                    flash_bd_addr[0], flash_bd_addr[1], flash_bd_addr[2],
                    flash_bd_addr[3], flash_bd_addr[4], flash_bd_addr[5]);
            if (bd_addr)
            {
                /* Copy BdAddr in parameter's pointer */
                bdcpy(bd_addr, flash_bd_addr);
            }
            return 0;
        }
        else
        {
            APP_DEBUG1("found Tag:0x%X Length:%d", p_data[0], p_data[1]);
            if (p_data[0] == APP_TM_SS_ITEM_INVALID_TAG)
            {
                break;
            }
        }
        /* If it's not a BdAddt item, jump to next one */
        /* jump item_id, length, status and data */
        length -= p_data[1] + 3;
        p_data += p_data[1] + 3;
    }
    APP_ERROR0("BdAddr item not found");
    return -1;
}

/*******************************************************************************
 **
 ** Function        app_tm_set_tx_carrier_frequency_test
 **
 ** Description     This function asks user to TxCarrierFreq VSC parameters and
 **                 sends the VSC
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_set_tx_carrier_frequency_test(void)
{
    BOOLEAN enable;
    UINT8 frequency = 0;
    UINT8 mode = 0;
    UINT8 modulation_type = 0;
    UINT8 power_selection = 0;
    UINT8 power_dBm = 0;
    UINT8 power_index = 0;

    APP_DEBUG0("");

    enable = app_get_choice("Turn On(0) or Off(1)");
    if (enable != 0)
    {
        APP_INFO0("Disable selected");
        enable = 0x01;
    }
    else
    {
        APP_INFO0("Enable selected");

        APP_INFO0("Frequency forced to:2(2402Mhz)");
        frequency = 2;

        APP_INFO0("Mode forced to:Unmodulated(0)");
        mode = 0;

        APP_INFO0("Modulation (unused) set to:GFSK(0)");
        modulation_type = 0;

        APP_INFO0("Transmit Power forced to:PowerTableIndex(0x09)");
        power_selection = 0x09;

        APP_INFO0("Power dBm (unused) set to:0dBm");
        power_dBm = 0;

        APP_INFO0("Power Table Index forced to:0");
        power_index = 0x00;
    }

    /* Send the VSC */
    return app_tm_vsc_set_tx_carrier_frequency(enable, frequency, mode,
            modulation_type, power_selection, power_dBm, power_index);
}

/*******************************************************************************
 **
 ** Function        app_tm_le_test
 **
 ** Description     This function sends the LE test commands to the
 **                 controller
 **
 ** Parameters       test: Test command to send:
 **                     0 - tx test, 1 - rx test, 2 - end test
 **                  freq: Frequency for rx and tx test commands
 **                  test_data_lenght: test_data_lenght for tx test command
 **                  pattern: test pattern for tx test command
 **                  retcount: test result. Its valid only test is END test
 **
 ** Returns         Status.
 **
 *******************************************************************************/
tBSA_STATUS app_tm_le_test(tBSA_TM_LE_CMD *pCMD)
{
   return BSA_TmLeTestCmd(pCMD);
}
