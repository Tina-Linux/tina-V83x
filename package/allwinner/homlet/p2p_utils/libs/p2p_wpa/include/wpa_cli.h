/*
 * WPA Supplicant - command line interface for wpa_supplicant daemon
 * Copyright (c) 2004-2016, Jouni Malinen <j@w1.fi>
 *
 * This software may be distributed under the terms of the BSD license.
 * See README for more details.
 */

#ifndef WPA_CLI_H
#define WPA_CLI_H

#include"includes.h"

#ifdef  __cplusplus
extern "C" {
#endif

typedef enum WFDP2PCBEVENT
{
    WFD_CB_CONNECTED_GO = 0,
    WFD_CB_CONNECTED_CLIENT,
    WFD_CB_DISCONNECTED,
    WFD_CB_QUITTED,
    WFD_CB_DEVICE_FOUND,
    WFD_CB_DEVICE_LOST
} WFDP2PCBEVENT;
typedef int (*wfdP2PCallback)(WFDP2PCBEVENT wfdP2PCBEvent, const char *elm1, const int elm2);

int wpa_cli_main(int argc, char *argv[], wfdP2PCallback callback);
int wpa_cli_cmd_handler(char *cmd);
void* wpa_cli_get_ctrl_conn();
void* wpa_cli_get_mon_conn();
char * wpa_cli_get_ifname(char *ifname);
char * wpa_cli_get_default_ifname(void);
int wpa_cli_cmd_with_reply(const char *cmd, char *reply, size_t *reply_len);

#ifdef  __cplusplus
}
#endif

#endif /* WPA_CLI_H */
