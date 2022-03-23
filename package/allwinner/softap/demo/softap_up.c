#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <aw_softap_intf.h>

/*FOR COMPLETE PROCESS for up and dow softap, please refer to the demo: "softap_up_down_longtime_test".
*/
int main(int argc, char **argv)
{
    int i = 0;
    int len = 0;
    tSOFTAP_KEY_MGMT key_mgmt = SOFTAP_WPA2_PSK;
    int ret = 0;
    char *psk = NULL;
    char *broadcast_hidden = NULL;

    /*No argruments or only the one who is for help*/
    if((argc == 1) || (argc == 2 && (!strcmp(argv[1],"-help") || !strcmp(argv[1], "-h"))))
    {
        printf("******************************\n");
        printf("softap_up -help/-h: for usage details\n");
        printf("Usage:\n");
        printf("softap_up \"ssid\" \"psk/open\" \"broadcast/hidden\"\n");
        printf("******************************\n");
        return 0;
    }

    ret = aw_softap_init();
    if(0 != ret)
    {
        printf("init softap failed!\n");
        return -1;
    }


    printf("***************************\n");
    printf("Start hostapd test!\n");
    printf("***************************\n");
#ifdef BCMDHD

    ret = aw_softap_reload_firmware("AP");
    if(0 != ret)
    {
        printf("reload firmware failed!\n");
        return -1;
    }
#endif
/*To keep consistent with the older version demo,
this demo do no set <interface> <channel>, but
application of the user's can set them correctly by
themselves*/
    if((argc >= 3) && (argv[2][0] != '\0') && strcmp(argv[2], "open"))
    {
        psk = (char *)argv[2];
        key_mgmt = SOFTAP_WPA2_PSK;
        printf("wpa2-psk!\n");
    }
    else
    {
	key_mgmt = SOFTAP_NONE;
	printf("none key_mgmt!\n");
    }

    if(argc >= 4)
        broadcast_hidden = (char *)argv[3];
    else
        broadcast_hidden = (char *)"broadcast"; //defaul set


    ret = aw_softap_config(argv[1], psk, key_mgmt, "wlan0", "6", broadcast_hidden);
    if(0 != ret)
    {
        printf("config softap failed!\n");
        return -1;
    }

    ret = aw_softap_save_config();
    if(0 != ret)
    {
        printf("save config failed!\n");
        return -1;
    }

    ret = aw_softap_enable();
    if(0 != ret)
    {
        printf("enable softap failed!\n");
        return -1;
    }

    ret = aw_softap_router_config("192.168.5.1", "255.255.255.0");
    if(0 != ret)
    {
        printf("config IP and maskcode failed!\n");
        return -1;
    }
//	system("udhcpd /etc/wifi/udhcpd.conf");
//	usleep(500000);
    ret = aw_softap_start_udhcp_dns_server();
    if(0 != ret)
    {
        printf("start udhcp and dns server failed!\n");
        return -1;
    }

    ret = aw_softap_enable_data_forward("eth0");
    if(0 != ret)
    {
        printf("enable data forward failed!\n");
        return -1;
    }

    printf("***************************\n");
    printf("Hostapd test successed!\n");
    printf("***************************\n");

    ret = aw_softap_deinit();
    if(0 != ret)
    {
        printf("deinit failed!\n");
        return -1;
    }
    return 0;
}
