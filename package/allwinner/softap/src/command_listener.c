#include <string.h>
#include <stdio.h>
#include <stdlib.h>



#include "netd_softap_controller.h"
#include "response_code.h"
#include "command_listener.h"


static tRESPONSE_CODE ret_response;

/*
*code: return code
*msg: return message
*command: command to run
*len: the length of the buf to take the message/the length of message that return
*argc: the number of arguments for the command which needs
*argv: the arguments for the command which needs
*/
//int softap_command(tRESPONSE_CODE *code, char *msg, char *command, int *len, int argc, char *argv[]) {
int run_softap_command(tRESPONSE_CODE *code, char *msg, char *command, int *len, int argc, char *argv[]) {
    tRESPONSE_CODE rc = SOFTAP_STATUS_RESULT;
    int flag = 0;
    char *retbuf = NULL;
	tRESPONSE_CODE *ret_code = (tRESPONSE_CODE *)code;
	char *ret_msg = msg;
	char *cmd = command;

/*
    if (sSoftapCtrl == NULL) {

      cli->sendMsg(SERVICE_START_FAILED, "SoftAP is not available", false);
      return -1;
    }
	*/
    if (strcmp(cmd,"") == 0) {
	*ret_code = COMMAND_SYNTAX_ERROR;
        *len = snprintf(msg,*len,"Missing argument in a SoftAP command");
        return 0;
    }

    if (!strcmp(cmd, "startap")) {
        rc = start_softap();
    } else if (!strcmp(cmd, "stopap")) {
        rc = stop_softap();
    } else if (!strcmp(cmd, "fwreload")) {
#ifndef WIFI_ESP8089
        rc = fw_reload_softap(argc, argv);
#else
        rc = 0;
#endif
    } else if (!strcmp(cmd, "status")) {
        asprintf(&retbuf, "Softap service %s running",
                 (is_softap_started() ? "is" : "is not"));
//        cli->sendMsg(rc, retbuf, false);
        free(retbuf);
        return 0;
    } else if (!strcmp(cmd, "set")) {
        rc = set_softap(argc, argv);
    } else {
        *ret_code = COMMAND_SYNTAX_ERROR;
		*len = snprintf(msg,*len,"Unrecognized SoftAP command");
        return 0;
    }

    if (rc >= 400 && rc < 600){
		*ret_code = rc;
		*len = snprintf(msg,*len,"SoftAP command has failed");
    }
    else{
		*ret_code = rc;
		*len = snprintf(msg,*len,"OK");
    }

    return 0;
}
/*
static const aw_softap_interface_t aw_softap_interface = {
	run_softap_command = softap_command,
}

const aw_softap_interface_t * command_listener()
{
	return &aw_softap_interface;
}
*/
