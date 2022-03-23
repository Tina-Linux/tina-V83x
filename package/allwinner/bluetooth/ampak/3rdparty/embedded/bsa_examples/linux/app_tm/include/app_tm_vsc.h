/*****************************************************************************
**
**  Name:           app_tm_vsc.h
**
**  Description:    Bluetooth Test Module VSC functions
**
**  Copyright (c) 2011-2014, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/

#ifndef APP_TM_VSC_H
#define APP_TM_VSC_H

/*
 * Definitions
 */
#define HCI_VSC_OPCODE_SET_BD_ADDR                  0x01
#define HCI_VSC_OPCODE_SET_TX_CARRIER_FREQUENCY     0x14
#define HCI_VSC_OPCODE_GPIO_CONFIG_AND_WRITE        0x19
#define HCI_VSC_OPCODE_WRITE_SCO_PCM_INT_PARAM      0x1C
#define HCI_VSC_OPCODE_WRITE_PCM_CONFIG_PARAM       0x1E
#define HCI_VSC_OPCODE_SET_TX_POWER                 0x26
#define HCI_VSC_OPCODE_ENABLE_UHE                   0x3B
#define HCI_VSC_OPCODE_READ_COLLABORATION_MODE      0x40
#define HCI_VSC_OPCODE_WRITE_COLLABORATION_MODE     0x41
#define HCI_VSC_OPCODE_READ_RAW_RSSI                0x48
#define HCI_VSC_OPCODE_SET_ACL_PRIORITY             0x57
#define HCI_VSC_OPCODE_ENCAP_HCI_CMD                0xA3
#define HCI_VSC_OPCODE_BFC_WRITE_PARAMS             0xC2
#define HCI_VSC_OPCODE_BFC_READ_PARAMS              0xC3

#define HCI_VSC_OPCODE_ACCESS_OTP                   0xF3
#define HCI_VSC_OPCODE_ACCESS_OTP_READ_OPERATION    0x5
#define HCI_VSC_OPCODE_ACCESS_OTP_WRITE_OPERATION   0x4

#define HCI_VSC_OPCODE_LLR_WRITE_SCAN_ENABLE        0xFA


typedef struct
{
    UINT8 bfc_enable;
    UINT8 frequency1;
    UINT8 frequency2;
    UINT8 frequency3;
    UINT8 access_code_length;
    UINT16 host_scan_interval;
    UINT16 host_trigger_timeout;
    UINT16 hid_scan_interval;
    UINT8 hid_scan_retry;
    UINT8 dont_disturb;
    UINT8 wake_up_mask;
} tAPP_TM_TBFC_PARAM;

/*
 * WLAN/Bluetooth Coexistence definitions
 */

/* Architecture */
#define APP_TM_COEX_ARCH_2035               0x00
#define APP_TM_COEX_ARCH_2045               0x80

/* Coexistence solution */
#define APP_TM_COEX_SOL_OFF                 0x00
#define APP_TM_COEX_SOL_2P_2P               0x01
#define APP_TM_COEX_SOL_3P_2P               0x02
#define APP_TM_COEX_SOL_WCS                 0x03

/* Priorities */
#define APP_TM_COEX_PRIO_NONE               0x00000000
#define APP_TM_COEX_PRIO_STANDBY            0x00000001
#define APP_TM_COEX_PRIO_LOW_PRI_CONN       0x00000002
#define APP_TM_COEX_PRIO_AFH_RSSISCAN       0x00000004
#define APP_TM_COEX_PRIO_BROADCAST          0x00000008
#define APP_TM_COEX_PRIO_INQUIRY            0x00000010
#define APP_TM_COEX_PRIO_PAGE               0x00000020
#define APP_TM_COEX_PRIO_CONNECTION         0x00000040
#define APP_TM_COEX_PRIO_CONN_WITH_DATA     0x00000080
#define APP_TM_COEX_PRIO_TPOLL              0x00000100
#define APP_TM_COEX_PRIO_INQSCAN            0x00000200
#define APP_TM_COEX_PRIO_PAGESCAN           0x00000400
#define APP_TM_COEX_PRIO_ROLE_SWITCH        0x00000800
#define APP_TM_COEX_PRIO_NEW_CONN           0x00001000
#define APP_TM_COEX_PRIO_SETUP_CONN         0x00002000
#define APP_TM_COEX_PRIO_MODULO             0x00004000
#define APP_TM_COEX_PRIO_HOLD               0x00008000
#define APP_TM_COEX_PRIO_SNIFF              0x00010000
#define APP_TM_COEX_PRIO_PARK               0x00020000
#define APP_TM_COEX_PRIO_SCO                0x00040000
#define APP_TM_COEX_PRIO_ESCO               0x00080000
#define APP_TM_COEX_PRIO_SLAVE_POLL_LMP     0x00100000
#define APP_TM_COEX_PRIO_DEFER_HIGH_PRIO_FRAME     0x01000000 /*  3 wires only */
#define APP_TM_COEX_PRIO_INC_PRIO_AFTER_DEFFER_ENB 0x02000000
#define APP_TM_COEX_PRIO_DONT_RESET_LCU     0x04000000 /* 2048 only */
#define APP_TM_COEX_PRIO_NON_CONN_HW_SUPPORT 0x08000000 /* 2048 only */
#define APP_TM_COEX_PRIO_PAGE_SCAN_HW_BIT_0 0x40000000 /*2048 only */
#define APP_TM_COEX_PRIO_PAGE_SCAN_HW_BIT_1 0x80000000 /*2048 only */

/* Configuration flags 1 */
#define APP_TM_COEX_CFG1_NONE               0x00000000
#define APP_TM_COEX_CFG1_AUTO_SEL           0x00000001 /* Coexistence auto selection */
#define APP_TM_COEX_CFG1_AUTO_PAD_CFG       0x00000002 /* Coexistence auto pad config */
#define APP_TM_COEX_CFG1_BACK_PW_WORKAROUND 0x00100000
#define APP_TM_COEX_CFG1_SAMPLE_RX          0x00200000
#define APP_TM_COEX_CFG1_DYNAMIC_LCU_RESET  0x00400000
#define APP_TM_COEX_CFG1_SIMURX_WORKAROUND  0x00800000
#define APP_TM_COEX_CFG1_5WIRE              0x01000000 /* 5WIRE support */
#define APP_TM_COEX_CFG1_A2DP_ACP_PRIO_INV  0x02000000 /* A2DP ACL priority inverse */
#define APP_TM_COEX_CFG1_A2DP_DAUL_ANT      0x04000000 /* A2DP Dual Antenna Mode */
#define APP_TM_COEX_CFG1_NONCONN_LCU_RST    0x08000000 /* Nonconnection LCU reset */
#define APP_TM_COEX_CFG1_CONN_HW_SUPP       0x10000000 /* Connection hardware support */
#define APP_TM_COEX_CFG1_HW_CX_MODE         0x80000000 /* Hardware coexistence mode */
#define APP_TM_COEX_CFG1_BLE_NOK            0x00010000 /* BLE COEX Nokia Mode */
#define APP_TM_COEX_CFG1_BLE_ENB            0x00008000 /* BLE COEX Enable */

/* Configuration flags 2 */
#define APP_TM_COEX_CFG2_NONE               0x00000000
#define APP_TM_COEX_CFG2_BLE_CONN_H_PRIO    0x02000000 /* BLE Connection high priority */
#define APP_TM_COEX_CFG2_BLE_INIT_H_PRIO    0x01000000 /* BLE INIT high priority */
#define APP_TM_COEX_CFG2_BLE_SCAN_H_PRIO    0x00800000 /* BLE SCAN high priority */
#define APP_TM_COEX_CFG2_BLE_ADV_H_PRIO     0x00400000 /* BLE ADV high priority */
#define APP_TM_COEX_CFG2_SLAVE_POLL_PRIO    0x20000000 /* Slave poll priority */
#define APP_TM_COEX_CFG2_PS_FHS_L_PRIO      0x40000000 /* PS and FHS low priority */
#define APP_TM_COEX_CFG2_LMP_H_PRIO         0x80000000 /* LMP high priority */

typedef struct
{
    BOOLEAN enable;
} tAPP_TM_WLAN_COLLABORATION_PARAM;

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
        UINT8 modulation_type, UINT8 power_selection, INT8 power_dBm, UINT8 power_index);

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
int app_tm_vsc_bfc_read_params(tAPP_TM_TBFC_PARAM *p_param);

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
int app_tm_vsc_bfc_write_params(tAPP_TM_TBFC_PARAM *p_param);

/*******************************************************************************
 **
 ** Function         app_tm_vsc_enable_uhe
 **
 ** Description      This function sends a VSC to Control UHE mode
 **
 ** Returns          status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_enable_uhe(int enable);

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
int app_tm_vsc_write_collaboration_mode(tAPP_TM_WLAN_COLLABORATION_PARAM *p_param);

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
int app_tm_vsc_read_collaboration_mode(void);

/*******************************************************************************
 **
 ** Function         app_tm_vsc_control_llr_scan
 **
 ** Description      Start/Stop LLR Page Scan mode
 **
 ** Returns          int
 **
 *******************************************************************************/
int app_tm_vsc_control_llr_scan(BOOLEAN start);

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
int app_tm_vsc_set_tx_power(BD_ADDR bda, INT8 power);

/*******************************************************************************
 **
 ** Function        app_tm_vsc_read_tx_power
 **
 ** Description     This function sends the following VSC:
 **                 Read Transmit Power
 **
 ** Parameters      bd_addr: BD address of the device to read tx power
 **
 ** Returns         status: 0 if success / -1 otherwise
 **
 *******************************************************************************/
int app_tm_vsc_read_tx_power(BD_ADDR bda);

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
int app_tm_vsc_config_gpio(UINT8 aux_gpio, UINT8 pin_number, UINT8 pad_config, UINT8 value);

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
int app_tm_vsc_read_otp();

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
int app_tm_vsc_write_otp();

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
int app_tm_vsc_configure_pcm();

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
int app_tm_vsc_configure_sco();

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
int app_tm_vsc_set_connection_priority();

#endif /* APP_TM_VSC_H_ */
