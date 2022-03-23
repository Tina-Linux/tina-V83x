/*****************************************************************************
**
**  Name:           app_tm.h
**
**  Description:    Bluetooth Test Module application
**
**  Copyright (c) 2010-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

/* idempotency */
#ifndef APP_TM_H
#define APP_TM_H

/* self-sufficiency */
#include "bt_target.h"
#include "bd.h"
#include "bsa_tm_api.h"
/*
 * Definitions
 */
#define APP_TM_TRACE_DISABLE_ALL          BSA_TM_TRACE_LEVEL_NONE
#define APP_TM_TRACE_ENABLE_ALL           BSA_TM_TRACE_LEVEL_DEBUG

/* Vendor Specific Opcode definitions */
#define HCI_READ_RAM_VSC 0x004D   /* Read RAM */


/*
 * Functions
 */

/*******************************************************************************
 **
 ** Function         app_tm_set_test_mode
 **
 ** Description      This function is used to set the Bluetooth Test Mode
 **
 ** Returns          0 if successful, error code otherwise
 **
 *******************************************************************************/
int app_tm_set_test_mode(BOOLEAN test_mode);

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
int app_tm_disconnect(BD_ADDR bda);

/*******************************************************************************
 **
 ** Function         app_tm_get_mem_usage
 **
 ** Description      This function is used to get Server's task and memory usage
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_get_mem_usage(UINT8 location);

/*******************************************************************************
 **
 ** Function         app_tm_ping
 **
 ** Description      This function is used to ping the BSA server
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_ping(void);

/*******************************************************************************
 **
 ** Function         app_tm_vsc
 **
 ** Description      This function is used to send a VendorSpecificCommand
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_vsc(void);

/*******************************************************************************
 **
 ** Function         app_tm_set_trace
 **
 ** Description      This function is used to set the trace level
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_set_trace(int trace_level);

/*******************************************************************************
 **
 ** Function         app_tm_read_version
 **
 ** Description      This function is used to read version
 **
 ** Parameters       None
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_read_version(void);

/*******************************************************************************
 **
 ** Function         app_tm_read_ram
 **
 ** Description      Read and display RAM
 **
 ** Returns          Status
 **
 *******************************************************************************/
int app_tm_read_ram(void);

/*******************************************************************************
 **
 ** Function         app_tm_read_conn
 **
 ** Description      Read and parse the ACL table
 **
 ** Returns          Status
 **
 *******************************************************************************/
int app_tm_read_conn(void);

/*******************************************************************************
 **
 ** Function         app_tm_read_flash_bd_addr
 **
 ** Description      This function is used to read BD address from flash
 **
 ** Parameters       None
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_read_flash_bd_addr(void);

/*******************************************************************************
 **
 ** Function         app_tm_read_serial_flash_bd_addr
 **
 ** Description      This function is used to read BD address from a Serial Flash
 **
 ** Parameters       bd_addr: BdAddr buffer to be updated
 **
 ** Returns          status
 **
 *******************************************************************************/
int app_tm_read_serial_flash_bd_addr(BD_ADDR bd_addr);

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
int app_tm_set_tx_carrier_frequency_test(void);

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
int app_tm_vsc_read_raw_rssi(BD_ADDR bda);

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
int app_tm_read_rssi(BD_ADDR bda);

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
int app_tm_qos_setup(void);

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
tBSA_STATUS app_tm_le_test(tBSA_TM_LE_CMD *pCMD);

#endif
