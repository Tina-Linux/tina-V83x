/*****************************************************************************
 **
 **  Name:           bsa_tm_api.h
 **
 **  Description:    This is the public interface file for the Test Module
 **                  (TM) subsystem of BSA, Broadcom's Bluetooth simple api
 **                  layer.
 **
 **  Copyright (c) 2003-2014, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/

#ifndef BSA_TM_API_H
#define BSA_TM_API_H

/* for tBSA_STATUS */
#include "bsa_status.h"
/* for BSA_TM_VSC_DATA_LEN_MAX */
#include "btm_api.h"

/* BSA TM callback events */
#define BSA_TM_VSE_EVT          0 /* VSE (Vendor Specific Event) event */

/* TM Event type */
typedef UINT8 tBSA_TM_EVT;

/* Maximum size of VSE data */
#define BSA_TM_VSE_SIZE_MAX     254

/* Callback event data for BSA_DM_3DTV_MULTICAST_RX_EVT event */
typedef struct
{
    UINT8 length;
    UINT8 sub_event;
    UINT8 data[BSA_TM_VSE_SIZE_MAX];
} tBSA_TM_VSE_MSG;

/* Union of data associated with tBSA_TM_EVT */
typedef union
{
    tBSA_TM_VSE_MSG vse; /* BSA_DM_3DTV_MULTICAST_RX_EVT */
} tBSA_TM_MSG;

#define BSA_TM_TASK_MAX         15   /* Max number of task returned */
#define BSA_TM_TASK_NAME_MAX    20  /* Max length of a Task Name (BTU, etc.) */
#define BSA_TM_GKI_BUF_POOL_MAX 16  /* Max number of GKI Buffer Pool returned */
#define BSA_TM_VSC_DATA_LEN_MAX BTM_MAX_VENDOR_SPECIFIC_LEN
#define BSA_TM_BSA_SERVER_VERSION_LEN 50  /* Length of BSA_SERVER_VERSION */

/* Location (Server/Client) for GetMemUsage */
#define BSA_TM_SERVER           0
#define BSA_TM_CLIENT           1

/* Define trace levels */
#define BSA_TM_TRACE_LEVEL_NONE     BT_TRACE_LEVEL_NONE     /* No trace messages to be generated    */
#define BSA_TM_TRACE_LEVEL_ERROR    BT_TRACE_LEVEL_ERROR    /* Error condition trace messages       */
#define BSA_TM_TRACE_LEVEL_WARNING  BT_TRACE_LEVEL_WARNING  /* Warning condition trace messages     */
#define BSA_TM_TRACE_LEVEL_API      BT_TRACE_LEVEL_API      /* API traces                           */
#define BSA_TM_TRACE_LEVEL_EVENT    BT_TRACE_LEVEL_EVENT    /* Debug messages for events            */
#define BSA_TM_TRACE_LEVEL_DEBUG    BT_TRACE_LEVEL_DEBUG    /* Full debug messages                  */

/* Status of one task */
typedef struct
{
    BOOLEAN task_used;
    UINT8 task_id;
    UINT8 task_state;
    UINT16 stack_size;
    UINT16 stack_used;
    INT8 task_name[BSA_TM_TASK_NAME_MAX];
} tBSA_TM_TASK_STATUS;

/* Status of one GKI Buffer pool */
typedef struct
{
    BOOLEAN pool_used;
    UINT16 pool_size;
    UINT16 pool_number;
    UINT16 pool_cur_use;
    UINT16 pool_max_use;
    UINT8 pool_id;
} tBSA_TM_GKI_BUF_STATUS;

/* Dummy parameter */
typedef struct
{
    UINT8 dummy;
} tBSA_TM_DUMMY;


/* Parameter to Enable/Disable Test Mode */
typedef struct
{
    BOOLEAN test_mode_enable;
} tBSA_TM_SET_TEST_MODE;

/* Parameters returned by Get Mem Usage API */
typedef struct
{
    UINT8 location;
    BOOLEAN task_missing;
    BOOLEAN gki_buf_missing;
    tBSA_TM_TASK_STATUS task[BSA_TM_TASK_MAX];
    tBSA_TM_GKI_BUF_STATUS gki_buffer[BSA_TM_GKI_BUF_POOL_MAX];
} tBSA_TM_GET_MEM_USAGE;

/* Parameter used for Ping API */
typedef tBSA_TM_DUMMY tBSA_TM_PING;

/* Parameters used for VendorSpecificCmd API*/
typedef struct
{
    UINT16 opcode;
    UINT16 length;
    tBSA_STATUS status;
    UINT8 data[BSA_TM_VSC_DATA_LEN_MAX];
} tBSA_TM_VSC;

/* Parameter used for TM HCI Cmd API */
typedef struct
{
    UINT16 opcode;
    UINT16 length;
    tBSA_STATUS status;
    BOOLEAN no_opcode_swap;
    UINT8 data[BSA_TM_VSC_DATA_LEN_MAX];
} tBSA_TM_HCI_CMD;

/* Parameters used for SetTraceLevel API */
typedef struct
{
    UINT8 all_trace_level;
} tBSA_TM_SET_TRACE_LEVEL;


/* FW Version structure */
typedef struct
{
    UINT8 major;
    UINT8 minor;
    UINT8 build;
    UINT16 config;
} tBSA_TM_FW_VERSION;

/* Read Version API */
typedef struct
{
    tBSA_STATUS status;
    tBSA_TM_FW_VERSION fw_version;
    UINT8 bsa_server_version[BSA_TM_BSA_SERVER_VERSION_LEN];
} tBSA_TM_READ_VERSION;

/* Read BD address stored in configuration */
typedef struct
{
    tBSA_STATUS status;
    BD_ADDR bd_addr;
} tBSA_TM_READ_CFG_BD_ADDR;

/* BSA TM callback function */
typedef void (tBSA_TM_CBACK)(tBSA_TM_EVT event, tBSA_TM_MSG *p_data);

#define BTA_TM_VSE_SUB_EVENT_ALL    0xFFFF

typedef struct
{
    UINT16 sub_event; /* Sub_event to register (0..FF or FFFF for all) */
    tBSA_TM_CBACK *p_callback; /* VSE Callback function */
} tBSA_TM_VSE_REGISTER;

typedef struct
{
    UINT16 sub_event; /* Sub_event to deregister (0..FF or FFFF for all) */
    BOOLEAN deregister_callback; /* Indicates if the Callback must be deregistered too */
} tBSA_TM_VSE_DEREGISTER;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr to Disconnect */
} tBSA_TM_DISCONNECT;


typedef struct
{
    BD_ADDR bd_addr;         /* BdAddr to setup QoS */
    UINT8   flag;            /* reserved for future use */
    UINT8   service_type;    /* 0:no traffic, 1:Best Effort, 2:Guaranteed */
    UINT32  token_rate;      /* Token rate in octets per second */
    UINT32  peak_bandwidth;  /* Peak bandwidth in octets per second */
    UINT32  latency;         /* Latency in microseconds */
    UINT32  delay_variation; /* Delay Variation in microseconds */
} tBSA_TM_QOS;


typedef struct
{
    tBSA_STATUS status;
    BD_ADDR bd_addr; /* BdAddr used to get connection handle */
    tBT_TRANSPORT transport;
    UINT16 handle;
} tBSA_TM_GET_HANDLE;

typedef struct
{
    BD_ADDR bd_addr; /* BdAddr to Read Raw RSSI */
    UINT16 opcode;
    UINT16 length;
    tBSA_STATUS status;
    UINT8 data[BSA_TM_VSC_DATA_LEN_MAX];
} tBSA_TM_READRAWRSSI;

#define BSA_LE_TX_TEST_CMD  0
#define BSA_LE_RX_TEST_CMD  1
#define BSA_LE_END_TEST_CMD  2

typedef struct
{
    UINT8 test;
    UINT8 freq;
    UINT8 pattern;
    UINT8 payload_len;
    UINT16 retcount;
} tBSA_TM_LE_CMD;

typedef struct
{
    UINT8 status;
    UINT16 count;
} tBSA_TM_LE_CMD_RESULT;

/* BSA Diag Stats commands */
#define BSA_TM_DIAG_RESET_STATS_CMD   0
#define BSA_TM_DIAG_BASIC_ACL_CMD     1
#define BSA_TM_DIAG_EDR_ACL_CMD       2
#define BSA_TM_DIAG_AUX_BER_CMD       3
#define BSA_TM_DIAG_MAX_CMD           4

typedef UINT8 tBSA_TM_DIAG_CMD;

/* Data struct for Basic ACL Diag Stats */
typedef struct
{
    UINT16 null_packets_rcvd;
    UINT16 poll_packets_rcvd;
    UINT16 dm1_packets_rcvd;
    UINT16 dh1_packets_rcvd;
    UINT16 dv_packets_rcvd;
    UINT16 aux1_packets_rcvd;
    UINT16 dm3_packets_rcvd;
    UINT16 dh3_packets_rcvd;
    UINT16 dm5_packets_rcvd;
    UINT16 dh5_packets_rcvd;
    UINT16 null_packets_tx;
    UINT16 poll_packets_tx;
    UINT16 dm1_packets_tx;
    UINT16 dh1_packets_tx;
    UINT16 dv_packets_tx;
    UINT16 aux1_packets_tx;
    UINT16 dm3_packets_tx;
    UINT16 dh3_packets_tx;
    UINT16 dm5_packets_tx;
    UINT16 dh5_packets_tx;
    UINT32 total_rx_acl_bytes;
    UINT32 total_tx_acl_bytes;
    UINT16 hec_errors;
    UINT16 crc_errors;
    UINT16 seqn_repeat;
    UINT16 soft_reset;
    UINT16 testmode_tx_pkts;
    UINT16 testmode_rx_pkts;
    UINT16 testmode_pkt_errors;
} tBSA_TM_BASIC_ACL_STATS_DATA;

/* Data struct for EDR ACL Diag Stats */
typedef struct
{
    UINT16 null_packets_rcvd;
    UINT16 poll_packets_rcvd;
    UINT16 dm1_packets_rcvd;
    UINT16 _2_dh1_packets_rcvd;
    UINT16 _3_dh1_packets_rcvd;
    UINT16 _2_dh3_packets_rcvd;
    UINT16 _3_dh3_packets_rcvd;
    UINT16 _2_dh5_packets_rcvd;
    UINT16 _3_dh5_packets_rcvd;
    UINT16 null_packets_tx;
    UINT16 poll_packets_tx;
    UINT16 dm1_packets_tx;
    UINT16 _2_dh1_packets_tx;
    UINT16 _3_dh1_packets_tx;
    UINT16 _2_dh3_packets_tx;
    UINT16 _3_dh3_packets_tx;
    UINT16 _2_dh5_packets_tx;
    UINT16 _3_dh5_packets_tx;
    UINT32 total_rx_acl_bytes;
    UINT32 total_tx_acl_bytes;
    UINT16 hec_errors;
    UINT16 crc_errors;
    UINT16 seqn_repeat;
    UINT16 soft_reset;
    UINT16 testmode_tx_pkts;
    UINT16 testmode_rx_pkts;
    UINT16 testmode_pkt_errors;
} tBSA_TM_EDR_ACL_STATS_DATA;

/* Data struct for Aux BER Diag Stats */
typedef struct
{
    UINT32 packets_sent;
    UINT32 packets_received;
    UINT32 hec_errors;
    UINT32 sync_errors;
    UINT32 packet_errors;
    UINT32 bits_received;
    UINT32 bit_errors;
} tBSA_TM_AUX_BER_STATS_DATA;

/* Union of data associated with Diag Stats */
typedef union
{
    tBSA_TM_BASIC_ACL_STATS_DATA basic_acl;
    tBSA_TM_EDR_ACL_STATS_DATA edr_acl;
    tBSA_TM_AUX_BER_STATS_DATA aux_ber;
} tBSA_TM_DIAG_DATA;

typedef struct
{
    UINT8 diag_cmd; /* LMP Diagnostics command */
    tBSA_TM_DIAG_DATA  diag;
} tBSA_TM_DIAG_STATS;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_TmSetTestModeInit
 **
 ** Description      Initialize the tBSA_TM_SET_TEST_MODE structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmSetTestModeInit(tBSA_TM_SET_TEST_MODE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmSetTestMode
 **
 ** Description      Enable/Disable Bluetooth test mode
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmSetTestMode(tBSA_TM_SET_TEST_MODE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmGetMemUsageInit
 **
 ** Description      Initialize the tBSA_TM_GET_MEM_USAGE structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmGetMemUsageInit(tBSA_TM_GET_MEM_USAGE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmGetMemUsage
 **
 ** Description      Get Memory usage (Task's stack size, and GKI buffers)
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmGetMemUsage(tBSA_TM_GET_MEM_USAGE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmVscInit
 **
 ** Description      Initialize the tBSA_TM_VSC structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmVscInit(tBSA_TM_VSC *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmVsc
 **
 ** Description      Function to send a Vendor Specific Command to the chip
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmVsc(tBSA_TM_VSC *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmHciInit
 **
 ** Description      Initialize the tBSA_TM_HCI_CMD structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmHciInit(tBSA_TM_HCI_CMD *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmHciCmd
 **
 ** Description      Function to send a HCI Command to the chip
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmHciCmd(tBSA_TM_HCI_CMD *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmPingInit
 **
 ** Description      Initialize the tBSA_TM_PING structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmPingInit(tBSA_TM_PING *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmPing
 **
 ** Description      Function to Ping the BSA server
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmPing(tBSA_TM_PING *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmSetTraceLevelInit
 **
 ** Description      Initialize the tBSA_TM_SET_TRACE_LEVEL structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmSetTraceLevelInit(tBSA_TM_SET_TRACE_LEVEL *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmSetTraceLevel
 **
 ** Description      Function to Set the Trace Level of the BSA server
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmSetTraceLevel(tBSA_TM_SET_TRACE_LEVEL *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmReadVersionInit
 **
 ** Description      Initialize the tBSA_TM_READ_VERSION structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmReadVersionInit(tBSA_TM_READ_VERSION *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmReadVersion
 **
 ** Description      Function to Read Version
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmReadVersion(tBSA_TM_READ_VERSION *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmVseRegisterInit
 **
 ** Description      Initialize the tBSA_TM_VSE_REGISTER structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmVseRegisterInit(tBSA_TM_VSE_REGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmVseRegister
 **
 ** Description      Function to Register VSE (sub_event and callback)
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmVseRegister(tBSA_TM_VSE_REGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmVseDeregisterInit
 **
 ** Description      Initialize the tBSA_TM_VSE_DEREGISTER structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmVseDeregisterInit(tBSA_TM_VSE_DEREGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmVseDeregister
 **
 ** Description      Function to Deregister VSE (sub_event and callback)
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmVseDeregister(tBSA_TM_VSE_DEREGISTER *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmDisconnectInit
 **
 ** Description      Initialize the tBSA_TM_DISCONNECT structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmDisconnectInit(tBSA_TM_DISCONNECT *p_req);


/*******************************************************************************
 **
 ** Function         BSA_TmDisconnect
 **
 ** Description      Function to Disconnect a remote device
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmDisconnect(tBSA_TM_DISCONNECT *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmQosInit
 **
 ** Description      Initialize the tBSA_TM_QOS structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmQosInit(tBSA_TM_QOS *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmQos
 **
 ** Description      Function to configure QoS to ACL connection
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmQos(tBSA_TM_QOS *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmGetHandleInit
 **
 ** Description      Initialize the tBSA_TM_GET_HANDLE structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmGetHandleInit(tBSA_TM_GET_HANDLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmGetHandle
 **
 ** Description      Function to get connection handle using BDADDR
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmGetHandle(tBSA_TM_GET_HANDLE *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmReadRawRssiInit
 **
 ** Description      Initialize the tBSA_TM_READRAWRSSI structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmReadRawRssiInit(tBSA_TM_READRAWRSSI *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmReadRawRssi
 **
 ** Description      Function to Read Raw RSSI
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmReadRawRssi(tBSA_TM_READRAWRSSI *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmDiagStatsInit
 **
 ** Description      Initialize the tBSA_TM_DIAG_STATS structure to default values.
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmDiagStatsInit(tBSA_TM_DIAG_STATS *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmDiagStats
 **
 ** Description      Function to send LMP Diagostics command to controller
 **
 ** Parameters       Pointer on structure containing API parameters
 **
 ** Returns          Status
 **
 *******************************************************************************/
extern tBSA_STATUS BSA_TmDiagStats(tBSA_TM_DIAG_STATS *p_req);

/*******************************************************************************
 **
 ** Function         BSA_TmLeTestCmd
 **
 ** Description      Function to send HCI LE Test commands to controller
 **
 ** Parameters       tBSA_TM_LE_CMD
 **                  test: Test command to send:
 **                    0 - tx test, 1 - rx test, 2 - end test
 **                  freq: Frequency for rx and tx test commands
 **                  payload_len: test_data_lenght for tx test command
 **                  pattern: test pattern for tx test command
 **                  retcount: test result. Its valid only test is END test
 **
 ** Returns          Status
 **
 *******************************************************************************/
tBSA_STATUS BSA_TmLeTestCmd(tBSA_TM_LE_CMD *pCMD);

#ifdef __cplusplus
}
#endif

#endif /* BSA_TM_API_H */
