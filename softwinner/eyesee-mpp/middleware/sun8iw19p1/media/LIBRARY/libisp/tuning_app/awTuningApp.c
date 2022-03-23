#include "log_handle.h"
#include "./server/server_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SERVER_PORT          8848
#define BUILD_VERSION        "Build Version: 0.9.2"
int main(int argc, const char *argv[])
{
	int ret = -1;
	int port = SERVER_PORT;
	
#if SERVER_DEBUG_EN
	if (argc >= 2) {
		init_logger(argv[1], "wb");
	} else {
		init_logger("/data/hawkview_server.log", "wb");
	}
#endif

	LOG(                  "==================================================================================\n"
		"                  ==========   Welcome to Hawkview Tools Tuning Server                    ==========\n"
		"                  ==========   %s, %s %s                 ==========\n"
		"                  ==========   Copyright (c) 2017 by Allwinnertech Co., Ltd.              ==========\n"
		"                  ==========   http://www.allwinnertech.com                               ==========\n",
		BUILD_VERSION, __TIME__, __DATE__
		);

	if (argc > 1) {
		port = atoi(argv[1]);
	}
	
	ret = init_server(port);
	if (!ret) {
		LOG(              "==========   Hawkview Tools Tuning Server Starts up, Enjoy tuning now!  ==========\n"
		//"                  ==================================================================================\n"
			);
		ret = run_server();
	}

	exit_server();
	LOG(                  "==========   Hawkview Tools Tuning Server Exits, Bye-bye!               ==========\n"
		"                  ==================================================================================\n"
		);

#if SERVER_DEBUG_EN
	close_logger();
#endif

	return 0;
}

