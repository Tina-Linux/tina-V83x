/*****************************************************************************
**
**  Name:           app_manager.c
**
**  Description:    Bluetooth Manager application
**
**  Copyright (c) 2009, Broadcom Corp., All Rights Reserved.
**  Broadcom Bluetooth Core. Proprietary and confidential.
**
*****************************************************************************/


#include <stdio.h>
#include <stdlib.h>

#include "gki.h"
#include "uipc.h"

#include "bsa_api.h"


/*******************************************************************************
 **
 ** Function         app_management_callback
 **
 ** Description      This callback function is called in case of server
 **                  disconnection (e.g. server crashes)
 **
 ** Parameters
 **
 ** Returns          void
 **
 *******************************************************************************/
void app_management_callback(tBSA_MGT_EVT event, tBSA_MGT_MSG *p_data)
{
    switch(event)
    {
    case BSA_MGT_STATUS_EVT:
        printf("\napp_management_callback BSA_MGT_STATUS_EVT\n");
        if (p_data->status.enable == FALSE)
        {
            APP_INFO0("Bluetooth Stopped");
        }
        else
        {
            APP_INFO0("Bluetooth restarted => re-initialize the application");
        }

        break;
    case BSA_MGT_DISCONNECT_EVT:
        APP_DEBUG1("BSA_MGT_DISCONNECT_EVT reason:%d\n", p_data->disconnect.reason);
        /* Connection with the Server lost => Just exit the application */
        exit(-1);
        break;
    }
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
    tBSA_STATUS bsa_status;
    tBSA_MGT_OPEN bsa_open_param;
    tBSA_MGT_CLOSE bsa_close_param;

    printf("\nHello BSA!!!\n\n");

    /*
     * Connect to Bluetooth daemon
     */
    BSA_MgtOpenInit(&bsa_open_param);
    bsa_open_param.callback = app_management_callback;
    bsa_status = BSA_MgtOpen(&bsa_open_param);
    if (bsa_status != BSA_SUCCESS)
    {
        fprintf(stderr, "main: Unable to connect to BSA server\n");
        exit(1);
    }

    printf("Connected to BSA Server\n");

    BSA_MgtCloseInit(&bsa_close_param);
    BSA_MgtClose(&bsa_close_param);

    printf("\nBye bye BSA!!!\n\n");

    exit(0);
}
