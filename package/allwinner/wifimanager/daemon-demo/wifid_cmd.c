#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>

#include "wmg_debug.h"
#include "wifid_cmd.h"

int main(int argc,char *argv[])
{
	int opt;
	const char *opts = "c:hslrtdo";
	char scan_results[SCAN_MAX];
	char list_net_results[LIST_NETWORK_MAX];
	int debug_level = 0;
	struct wifi_status status = {
		.state = STATE_UNKNOWN,
		.ssid = {'\0'},
	};

	int ret = -1;
	enum cn_event event = DA_UNKNOWN;
	struct option longopts[] = {
		{"connect",required_argument,NULL,'c'},
		{"scan",no_argument,NULL,'s'},
		{"list_network",no_argument,NULL,'l'},
		{"remove_network",no_argument,NULL,'r'},
		{"status",no_argument,NULL,'t'},
		{"help",no_argument,NULL,'h'},
		{"close",no_argument,NULL,'d'},
		{"open",no_argument,NULL,'o'},
		{"debug",required_argument,NULL,1},
		{ 0, 0, 0, 0 },
	};
	//wmg_set_debug_level(MSG_EXCESSIVE);
	if(argc == 1)
		goto usage;

	while ((opt = getopt_long(argc, argv, opts, longopts, NULL)) != -1){
		switch(opt){
			case 1:
				debug_level = atoi(optarg);
				printf("debug log level :%d\n",debug_level);
				wmg_set_debug_level(debug_level);
				break;
			case 'c'/* --commnad */:
				if(argc < 4 || argc > 6){
					printf("  -c, --connect\t\tconnect AP,-c <ssid> <password>\n");
					return EXIT_SUCCESS;
				}
				printf("==============================================\n");
				printf("connecting ssid:%s passward:%s\n",argv[optind-1],argv[optind]);
				aw_wifid_connect_ap(argv[optind-1],argv[optind],&event);
				if(event == DA_CONNECTED){
					wmg_printf(MSG_INFO,"connected ap successful\n");
				}else {
					wmg_printf(MSG_INFO,"connected ap failed:%s\n",connect_event_txt(event));
				}
				printf("==============================================\n");
				return EXIT_SUCCESS;

			case 'h'/* --help */:
usage:

				printf("  -h, --help\t\tprint this help and exit\n");
				printf("  -c, --connect\t\tconnect AP,-c <ssid> <password>\n");
				printf("  -s, --scan\t\tscan AP\n");
				printf("  -l, --list_network\tlist network\n");
				printf("  -t, --status\t\tget wifi status\n");
				printf("  -r, --remove_net\tremove network in config,-r <ssid>\n");
				printf("  -o, --open\t\topen wifi daemon\n");
				printf("  -d, --close\t\tclose wifi daemon\n");
				printf("  --debug, \t\tset debug log level,--debug <num>\n");
				return EXIT_SUCCESS;

			case 's'/*--scan*/:
				ret = aw_wifid_get_scan_results(scan_results,SCAN_MAX);
				if(ret != -1){
					wmg_printf(MSG_INFO,"scan results:\n%s\n",scan_results);
				}
				return EXIT_SUCCESS;

			case 'l'/*--list_network*/:
				ret = aw_wifid_list_networks(list_net_results,LIST_NETWORK_MAX);
				if(ret != -1){
					wmg_printf(MSG_INFO,"list networks results:\n%s\n",list_net_results);
				}
				return EXIT_SUCCESS;

			case 't'/*--disconnect*/:
				ret = aw_wifid_get_status(&status);
				if(ret >= 0) {
					wmg_printf(MSG_INFO,"wifi state:   %s\n",wmg_state_txt(status.state));
					if(status.state == NETWORK_CONNECTED)
						wmg_printf(MSG_INFO,"connected ssid:%s\n",status.ssid);
				}
				return EXIT_SUCCESS;

			case 'r'/*--remove_net*/:
				if(argc < 2){
					printf("  -r, --remove_net\tremove network in config,-r <ssid>\n");
					return EXIT_SUCCESS;
				}
				printf("removing ssid:%s\n",argv[optind]);
				aw_wifid_remove_networks(argv[optind],strlen(argv[optind]));
				return EXIT_SUCCESS;
			case 'o'/*--remove_net*/:
				aw_wifid_open();
				return EXIT_SUCCESS;

			case 'd'/*--remove_net*/:

				aw_wifid_close();
				return EXIT_SUCCESS;
			default:
				fprintf(stderr, "Try '%s --help' for more information.\n", argv[0]);
				return EXIT_FAILURE;
			}
		if (optind == argc)
			goto usage;
		}
}
