/*****************************************************************************
 **
 **  Name:           bsa_mgt_api.h
 **
 **  Description:    This is the public interface file for Management part of
 **                  the Bluetooth simplified API
 **
 **  Copyright (c) 2009-2010, Broadcom Corp., All Rights Reserved.
 **  Broadcom Bluetooth Core. Proprietary and confidential.
 **
 *****************************************************************************/
#ifndef BSA_MGT_H
#define BSA_MGT_H

/* For BSA_MGT_UIPC_PATH_MAX */
#include "bt_target.h"

/* for tBSA_STATUS */
#include "bsa_status.h"

/* Management Callback Events */
typedef enum
{
    BSA_MGT_DISCONNECT_EVT, /* Disconnect Event */
    BSA_MGT_STATUS_EVT   /* Bluetooth status event (enable/disable) */
} tBSA_MGT_EVT;

/* Structure associated with BSA_MGT_DISCONNECT_EVT */
typedef struct
{
    tBSA_STATUS reason; /* Disconnection reason */
} tBSA_MSG_DISCONNECT_MSG;

/* Structure associated with BSA_MGT_STATUS_EVT */
typedef struct
{
    BOOLEAN enable; /* Bluetooth Enable parameter */
} tBSA_MSG_STATUS_MSG;


/* Union of all Management messages structures (parameters) */
typedef union
{
    tBSA_MSG_DISCONNECT_MSG disconnect; /* disconnection parameter */
    tBSA_MSG_STATUS_MSG status; /* status parameter */
} tBSA_MGT_MSG;

/* Management Callback */
typedef void ( tBSA_MGT_CBACK)(tBSA_MGT_EVT event, tBSA_MGT_MSG *p_data);

/*
 * Structures use to pass parameters to BSA API functions
 */
#ifndef BSA_MGT_UIPC_PATH_MAX
#define BSA_MGT_UIPC_PATH_MAX 100
#endif
typedef struct
{
    /* UIPC path configuration */
    char uipc_path[BSA_MGT_UIPC_PATH_MAX];
    tBSA_MGT_CBACK *callback;
} tBSA_MGT_OPEN;

typedef struct
{
    int dummy; /* May be needed for some compilers */
} tBSA_MGT_KILL_SERVER;

typedef struct
{
    int dummy; /* May be needed for some compilers */
} tBSA_MGT_CLOSE;

/*****************************************************************************
**  External Function Declarations
*****************************************************************************/
#ifdef __cplusplus
extern "C"
{
#endif

/*******************************************************************************
 **
 ** Function         BSA_MgtOpenInit
 **
 ** Description      This function initializes the BSA API structure.
 **
 ** Parameters       Pointer to the allocated structure.
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtOpenInit(tBSA_MGT_OPEN *p_mgt_open);

/*******************************************************************************
 **
 ** Function         BSA_MgtOpen
 **
 ** Description      This function opens a connection to the Bluetooth Daemon.
 **                  The callback function is called when a disconnection
 **                  occurs (e.g. daemon dies).
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtOpen(tBSA_MGT_OPEN *p_mgt_open);

/*******************************************************************************
 **
 ** Function         BSA_MgtKillServerInit
 **
 ** Description      This function initializes the BSA API structure.
 **
 ** Parameters       Pointer to the allocated structure.
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtKillServerInit(tBSA_MGT_KILL_SERVER *p_mgt_kill_server);

/*******************************************************************************
 **
 ** Function         BSA_MgtKillServer
 **
 ** Description      This function kills the Bluetooth Daemon.
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtKillServer(tBSA_MGT_KILL_SERVER *p_mgt_kill_server);

/*******************************************************************************
 **
 ** Function         BSA_MgtCloseInit
 **
 ** Description      This function initializes the BSA API structure.
 **
 ** Parameters       Pointer to the allocated structure.
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtCloseInit(tBSA_MGT_CLOSE *p_mgt_close);

/*******************************************************************************
 **
 ** Function         BSA_MgtClose
 **
 ** Description      This function close a connection to the Bluetooth Daemon.
 **                  The callback function (of the BSA_MgtOpen) is not called
 **
 ** Returns          Status of the execution.
 **
 *******************************************************************************/
tBSA_STATUS BSA_MgtClose(tBSA_MGT_CLOSE *p_mgt_close);

#ifdef __cplusplus
}
#endif

#endif
