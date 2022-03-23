/*****************************************************************************
**
**  Name:           app_ble_blp.h
**
**  Description:     Bluetooth BLE Blood Pressure Collector include file
**
**  Copyright (c) 2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/
#ifndef APP_BLE_BLP_H
#define APP_BLE_BLP_H

#include "bsa_api.h"
#include "app_ble.h"
#include "app_ble_client_db.h"

/*
 * BLE common functions
 */

/*******************************************************************************
 **
 ** Function        app_ble_blp_main
 **
 ** Description     main function of Blood Pressure collector
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_blp_main(void);

/*******************************************************************************
 **
 ** Function        app_ble_blp_register
 **
 ** Description     Register Blood Pressure collector services
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_blp_register(void);

/*******************************************************************************
 **
 ** Function        app_ble_blp_read_blood_pressure_feature
 **
 ** Description     Read blood pressure feature from BLE blood pressure server
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_blp_read_blood_pressure_feature(void);

/*******************************************************************************
 **
 ** Function        app_ble_blp_enable_blood_pressure_measurement
 **
 ** Description     This is the register function to receive a indication
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_blp_enable_blood_pressure_measurement(BOOLEAN enable);

/*******************************************************************************
 **
 ** Function        app_ble_blp_intermediate_cuff_pressure
 **
 ** Description     Measure Intermediate Cuff Pressure
 **
 ** Parameters      None
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_ble_blp_intermediate_cuff_pressure(BOOLEAN enable);
#endif
