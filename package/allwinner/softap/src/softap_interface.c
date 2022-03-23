#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "aw_softap_intf.h"
#include "netd_softap_controller.h"
#include "response_code.h"
#include "command_listener.h"

#define MSG_LEN_MX 256
#define CMD_LEN_MX 128

/*49-bytes space is enough*/
#define IPV4_SET_BUF 64
#define DATA_FORWARD_BUF 128
#define CHECK_HOSTAPD_SHELL "ps | grep hostapd"

int agc = 0;
char **agv = (char **)NULL;

int check_hostapd_shell()
{
    int bytes;
    char buf[256];
    FILE *stream;
    char *p = NULL;
    char *p1 = NULL;
    char *p2 = NULL;
    char *p3 = NULL;
    stream = popen(CHECK_HOSTAPD_SHELL, "r");
    if(!stream)
        return -1;

    bytes = fread(buf, sizeof(char), sizeof(buf), stream);
//    printf("buf is %s", buf);
    p = buf;
    while((p1 = strchr(p, '\n')) != NULL)
    {
        *p1 = '\0';
        p2 =strstr(p,"grep");
	if(p2 == NULL)
        {
	    p3 = strstr(p, "hostapd");
            if(p3 != NULL)
                break;
            else
            {
                    *p1 = '\n';
                    p = p1+1;
                    continue;
            }
        }
        else
        {
                *p1 = '\n';
                p = p1+1;
                continue;
        }
    }

    if(p3 == NULL)
    {
        if((p2 =strstr(p,"grep")) == NULL)
            p3 = strstr(p, "hostapd");
    }

    pclose(stream);

    if(p3 != NULL)
    {
        printf("hostapd is up\n");
        return 1;
    }
    else
    {
        printf("hostapd is down\n");
        return 0;
    }
}

int aw_softap_reload_firmware(char *ap_sta)
{
    tRESPONSE_CODE ret_code = SOFTAP_STATUS_RESULT;
    char msg[MSG_LEN_MX] = {0};
    char cmd[CMD_LEN_MX] = {0};
    int len = 0;

    printf("Start to reload firmware!\n");
    agv[0] = (char *)ap_sta;
    sprintf(cmd,"fwreload");
    len = sizeof(msg);
    run_softap_command(&ret_code,msg,cmd,&len,agc,agv);
    if(ret_code != SOFTAP_STATUS_RESULT)
    {
        printf("Reload firmware failed! Code is %d\n",ret_code);
        return -1;
    }
    printf("Message is: %s\n",msg);
    printf("Reload firmware finished!\n");

    return 0;
}

int aw_is_softap_started()
{
    return check_hostapd_shell();
}

/*init softap with default configuration*/
int aw_softap_init()
{
    agc = 8;
    agv = (char **)malloc(sizeof(char *) * agc);

    agv[agc-1] = NULL;
    agv[0] = (char *)"AP"; /*setsoftap*/
    agv[1] = (char *)"set";
    agv[2] = (char *)"wlan0";
    agv[3] = (char *)"Smart-AW-HOSTAPD";
    agv[4] = (char *)"broadcast"; /*broadcast or hidden*/
    agv[5] = (char *)"6";
    agv[6] = (char *)"wpa2-psk";
    agv[7] = (char *)"wifi1111";

    return 0;
}

int aw_softap_deinit()
{
    agc = 0;
    free(agv);
    agv = (char **)NULL;
    return 0;
}

int aw_softap_config(char *ssid, char *psk, tSOFTAP_KEY_MGMT key_mgmt, char *interface, char *channel, char *broadcast_hidden)
{
    int chan = 0;
    int psk_is_ok = 0;

    if(interface && interface[0] != '\0')
        agv[2] = (char *)interface;

    if(ssid && ssid[0] != '\0')
        agv[3] = (char *)ssid;
    else
        printf("%s: No ssid input, use default configuration:%s", __func__, agv[3]);

    /*If there is no passwords, or the key_mgmt is NONE, set it as an open ap*/
    if(psk && (psk[0] != '\0') && (key_mgmt != SOFTAP_NONE))
    {
	if(strlen(psk) >= 8)
        {
            agv[7] = (char *)psk;
            psk_is_ok = 1;
        }
	else
	{
	    printf("%s PARAM_Error: the length of password must not be less than 8!\n", __func__);
	    return -1;
	}
    }
    else
    {
	agv[6] = (char *)"open";
        psk_is_ok = 0;
    }

    if(key_mgmt == SOFTAP_WPA2_PSK && psk_is_ok)
        agv[6] = (char *)"wpa2-psk";
    else if(key_mgmt == SOFTAP_WPA_PSK && psk_is_ok)
        agv[6] = (char *)"wpa-psk";

    if(channel && (channel[0] != '\0'))
        chan = atoi(channel);

    if(chan <= 0 || chan > 13)
    {
	printf("%s PARAM_ERROR: the channel to set is not reasonable, use default configuration: channel %s instead\n", __func__, agv[5]);
    }
    else
    {
        agv[5] = (char *)channel;
    }

    /*If the ap is hidden*/
    if(broadcast_hidden && (broadcast_hidden[0] != '\0'))
    {
        if(!strcmp(broadcast_hidden,"hidden") || !strcmp(broadcast_hidden,"broadcast"))
            agv[4] = (char *)broadcast_hidden;
        else
        {
            printf("%s PARAM_ERROR: Please use \"broadcast\" or \"hidden\" for the last parameter!\n", __func__);
            return -1;
	}
    }
    else
        printf("%s: No broadcast_hidden parameter input, use default configuration:%s", __func__, agv[4]);

    return 0;
}

int aw_softap_save_config()
{
    tRESPONSE_CODE ret_code = SOFTAP_STATUS_RESULT;
    char msg[MSG_LEN_MX] = {0};
    char cmd[CMD_LEN_MX] = {0};
    int len = 0;

    printf("Start to set softap!\n");
    sprintf(cmd,"set");
    len = MSG_LEN_MX;
    run_softap_command(&ret_code,msg,cmd,&len,agc,agv);
    if(ret_code != SOFTAP_STATUS_RESULT)
    {
	printf("Set softap failed! Code is %d\n",ret_code);
        return -1;
    }
    printf("Message is: %s\n",msg);
    printf("Set softap finished!\n");
    return 0;
}

int aw_softap_enable()
{
    tRESPONSE_CODE ret_code = SOFTAP_STATUS_RESULT;
    char msg[MSG_LEN_MX] = {0};
    char cmd[CMD_LEN_MX] = {0};
    int len = 0;
    int ret = 0;
/*If there are more than one interface, and the interface which used as softap
is not wlan0, do not kill supplicant*/
    if(!strcmp(agv[2], "wlan0"))
    {
        system("ifconfig wlan0 down");
        usleep(1000000);
        system("killall wpa_supplicant");
        usleep(1000000);
    }

    system("ifconfig wlan0 up");
    usleep(1000000);
/*If up softap and station on wlan1 and wlan0 on broadcom wifi module, respectively.
it needs to up wlan0 firstly, and then use iw to add wlan1*/
#if IW_UP_BROADCOM_WLAN1
    ret = system("iw wlan0 interface add wlan1 type managed");
    if(0 != ret)
    {
	printf("%s EORROR: iw wlan0 add wlan1 failed! make sure the corresponding components are OK\n", __func__);
        return -1;
    }
    usleep(1000000);
#endif

    printf("Start to start softap!\n");
    sprintf(cmd,"startap");
    len = MSG_LEN_MX;
    run_softap_command(&ret_code,msg,cmd,&len,agc,agv);
    if(ret_code != SOFTAP_STATUS_RESULT)
    {
	printf("Start softap failed! Code is %d\n",ret_code);
	return -1;
    }
    printf("Message is: %s\n",msg);
    printf("Start softap finished!\n");

    return 0;
}

int aw_softap_disable()
{
    int ret = 0;

    system("killall hostapd");
    system("killall dnsmasq");
    usleep(1000000);

#if IW_UP_BROADCOM_WLAN1
    ret = system("iw wlan1 del");
    if(0 != ret)
    {
	printf("%s EORROR: iw delete wlan1 failed! make sure the corresponding components are OK\n", __func__);
        return -1;
    }
    usleep(1000000);
#endif

#if (IW_UP_BROADCOM_WLAN1 == 0)
	printf("wlan0 reset requst...\n");
    system("ifconfig wlan0 down");
    usleep(1000000);
    system("ifconfig wlan0 up");
    usleep(1000000);
	system("/etc/init.d/wpa_supplicant start");
#endif

    system("echo 0 > /proc/sys/net/ipv4/ip_forward");
    return 0;

}

int aw_softap_router_config(char *ip, char *netmask)
{
    char ipv4_set_buf[IPV4_SET_BUF] = {0};
    int ret;

    sprintf(ipv4_set_buf,"ifconfig %s %s netmask %s", agv[2], ip, netmask);
    ret = system(ipv4_set_buf);
    if(ret == 0) {
	return 0;
    } else {
	return -1;
    }
}

int aw_softap_start_udhcp_dns_server()
{
#if defined(CUSTOMIZED_SOCKET_PATH)
	system("/usr/bin/dnsmasq -C /data/config/wifi/dnsmasq.conf -k -x /tmp/dnsmasq.pid &");
#else
    system("/etc/init.d/dnsmasq restart");
#endif
    return 0;
}

int aw_softap_enable_data_forward(char *interface)
{
    char data_forward[DATA_FORWARD_BUF] = {0};
    int ret;

    if(interface == NULL)
	return -1;

    ret = system("echo 1 > /proc/sys/net/ipv4/ip_forward");
    if(ret != 0)
	return -1;

    sprintf(data_forward,"iptables -A FORWARD -i %s -o %s -m state --state ESTABLISHED,RELATED -j ACCEPT", agv[2], interface);
    ret = system(data_forward);
    if(ret != 0)
	return -1;

    sprintf(data_forward,"iptables -A FORWARD -i %s -o %s -j ACCEPT", agv[2], interface);
    ret = system(data_forward);
    if(ret != 0)
	return -1;

    sprintf(data_forward,"iptables -A FORWARD -i %s -o %s -j ACCEPT", interface, agv[2]);
    ret = system(data_forward);
    if(ret != 0)
	return -1;

    sprintf(data_forward,"iptables -t nat -A POSTROUTING -o %s -j MASQUERADE", interface);
    ret = system(data_forward);
    if(ret != 0)
	return -1;

    return 0;
}
