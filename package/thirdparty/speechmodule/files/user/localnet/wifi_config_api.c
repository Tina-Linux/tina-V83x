#include "wifi_config.h"

//#define AP62XX //delete by 926 @20190624
#define RK8723 //add by 926 @20190624
#define CONFIG_IOCTL_CFG80211 //add by 926 @20190624

int sockfd;
sniffer_status g_sniffer_status;
struct _chplan g_chPlan[] = {
		{ .ch = 1,	.bw_os = '+', },
		{ .ch = 6,	.bw_os = '-', },
		{ .ch = 11,	.bw_os = '-', },
		{ .ch = 2,	.bw_os = '+', },
		{ .ch = 3,	.bw_os = '+', },
		{ .ch = 4,	.bw_os = '+', },
		{ .ch = 5,	.bw_os = '-', },
		{ .ch = 7,	.bw_os = '-', },
		{ .ch = 8,	.bw_os = '-', },
		{ .ch = 9,	.bw_os = '-', },
		{ .ch = 10,	.bw_os = '-', },
		{ .ch = 12,	.bw_os = '-', },
		{ .ch = 13,	.bw_os = '-', },
};

char ifName[IFNAMSIZ] = "wlan0";
char Softap_Name[IFNAMSIZ] = "p2p0";
char wifi_proc_path[64] = "rtl8723ds";
//char g_wpafile_path[512] = "/data/cfg/wpa_supplicant.conf"; //delete by 926 @20190619
char g_wpafile_path[512] = "/etc/wifi/wpa_supplicant.conf";

void RK_SYSTEM(const char *cmd)
{
	printf("sh:  %s\n", cmd);
	system(cmd);
}

/* dnsmasq.conf
 * user=root
 * listen-address=10.201.126.1
 * dhcp-range=10.201.126.50,10.201.126.150
 * server=/google/8.8.8.8
 */
//static const char DNSMASQ_CONF_DIR[] = "/data/dnsmasq.conf"; //delete by 926 @20190619
//static const char HOSTAPD_CONF_DIR[] = "/data/hostapd.conf"; //delete by 926 @20190619
static const char DNSMASQ_CONF_DIR[] = "/etc/dnsmasq.conf";
static const char HOSTAPD_CONF_DIR[] = "/etc/wifi/hostapd.conf";
static int create_dnsmasq_file(void)
{
    FILE* fp;

    fp = fopen(DNSMASQ_CONF_DIR, "wt+");

    if (fp != 0) {
        fputs("user=root\n", fp);
        fputs("listen-address=10.201.126.1\n", fp);
        fputs("dhcp-range=10.201.126.50,10.201.126.150\n", fp);
        fputs("server=/google/8.8.8.8\n", fp);
        fclose(fp);
        return 0;
    }
    return -1;
}

static int create_hostapd_file(const char *name, const char *password, int hide_ssid)
{
    FILE* fp;
    char cmdline[256] = {0};

    fp = fopen(HOSTAPD_CONF_DIR, "wt+");

    if (fp != 0) {
        sprintf(cmdline, "interface=%s\n", Softap_Name);
        fputs(cmdline, fp);
        //fputs("ctrl_interface=/var/run/hostapd\n", fp); //delete by 926 @20190624
        fputs("ctrl_interface=/etc/wifi/hostapd\n", fp);
        fputs("driver=nl80211\n", fp);
        fputs("ssid=", fp);
        fputs(name, fp);
        fputs("\n", fp);
        fputs("channel=6\n", fp);
        fputs("hw_mode=g\n", fp);
        fputs("ieee80211n=1\n", fp);
		sprintf(cmdline, "ignore_broadcast_ssid=%d\n", hide_ssid);
        fputs(cmdline, fp);
#if 1
		if (password) {
	        fputs("auth_algs=1\n", fp);
	        fputs("wpa=3\n", fp);
	        fputs("wpa_passphrase=", fp);
	        fputs(password, fp);
	        fputs("\n", fp);
	        fputs("wpa_key_mgmt=WPA-PSK\n", fp);
	        fputs("wpa_pairwise=TKIP\n", fp);
	        fputs("rsn_pairwise=CCMP", fp);
		}
#endif
        fclose(fp);
        return 0;
    }
    return -1;
}
/*
 * if SSID contain Chinese word, convert all string into Hex format.
 */
static char *ssid_rework(const char *before)
{
    unsigned int i;
    char *ch1;
    int flag_hex = FALSE;
    static char after[32 * 4 + 1];

    memset(after, 0, 32 * 4 + 1);
    if (before == NULL)
        return after;
    // process unicode SSID, without '"'
    for (i = 0; i < strlen(before); i++) {
        if (before[i] < 32 || before[i] > 127) {
            flag_hex = TRUE;
            break;
        }
    }

    ch1 = &after[0];
    if (flag_hex) {
        i = 0;
        for (;;) {
            if (before[i] == 0)
                break;
            sprintf(ch1,"%02x", (unsigned char)before[i]);
            ch1 += 2;
            i++;
        }
    } else { // ASCII SSID, need '"'
        after[0] ='"';
        strcpy(after+1, before);
        after[strlen(before) + 1] = '"';
    }

    return after;
}

/*
input:
	@filepath		: path of wpa_supplicant config file which need to be stored.
	@ssid			: AP's ssid
	@cfgfile_type	: the security type of AP

description:	store the result as  wpa_supplicant config file  in given file path,
*/
static int store_cfgfile(const char *filepath, char *ssid, char *passwd, int cfgfile_type)
{
	FILE *fd;
	//char commset[]={"update_config=1\nctrl_interface=/var/run/wpa_supplicant\neapol_version=1\nfast_reauth=1\n"};
    //delete by 926 @20190624
	char commset[]={"update_config=1\nctrl_interface=/etc/wifi/wpa_supplicant\neapol_version=1\nfast_reauth=1\n"};
	char WPAstr[]={"ap_scan=1\nnetwork={\n\tssid=%s\n\tscan_ssid=1\n\tpsk=\"%s\"\n}\n"};
	char OPENstr[]={"ap_scan=1\nnetwork={\n\tssid=%s\n\tkey_mgmt=NONE\n\tscan_ssid=1\n}\n"};
	char WEPstr[]={"ap_scan=1\nnetwork={\n\tssid=%s\n\tkey_mgmt=NONE\n\twep_key0=%s\n\twep_tx_keyidx=0\n\tscan_ssid=1\n}\n"};
	char CmdStr[2048];

	fd = fopen(filepath, "w+");
    printf("store_cfgfile:%s\n", filepath); //add by 926 @20190625
	if (fd == NULL) {
		printf("file open can not create file !!!\n");
		return -ENOENT;
	}
	sprintf(CmdStr, commset, ifName);
	fprintf(fd,"%s", CmdStr);
	switch (cfgfile_type) {
	case CFGFILE_OPEN :
		sprintf(CmdStr, OPENstr, ssid_rework(ssid));
		break;
	case CFGFILE_WPA :
		sprintf(CmdStr, WPAstr, ssid_rework(ssid), passwd);
		break;
	case CFGFILE_WEP :
		sprintf(CmdStr, WEPstr, ssid_rework(ssid), passwd);
		break;
	default :
	    fclose(fd);
		return -EINVAL;
		break;
	}
	printf("%s\n",CmdStr);
	fprintf(fd,"%s", CmdStr);
	fclose(fd);
	return 0;
}

/* Sniffer mode func
 * onoff: 0: managed, 1: monitor
 * ifName: wlan0
 */
static void wifi_monitor_mode_onoff(u8 onoff, const char *ifName)
{
	char cmdstr[200];

	memset(cmdstr, 0, sizeof(char)*200);

	//iwconfig wlanX mode monitor
	//iw dev wlanX set type monitor
#ifdef RK8723
#ifdef	CONFIG_IOCTL_CFG80211
	if (onoff)
	    sprintf(cmdstr,"iw dev %s set type monitor\n", ifName);
	else
	    sprintf(cmdstr,"iw dev %s set type managed\n", ifName);
#else
	if (onoff)
	    sprintf(cmdstr,"iwconfig %s mode monitor\n", ifName);
	else
	    sprintf(cmdstr,"iwconfig %s mode managed\n", ifName);
#endif
#elif defined(AP62XX)
        sprintf(cmdstr,"dhd_priv monitor %d\n", onoff);
#endif
	RK_SYSTEM(cmdstr);
}

/* Initialization channel */
static void init_chplan(int chnum)
{
	int i;

	for (i=0; i<chnum; i++) {
		if (g_chPlan[i].bw_os == '+')
			g_chPlan[i].bw_os = HAL_PRIME_CHNL_OFFSET_LOWER;
		else if (g_chPlan[i].bw_os == '-')
			g_chPlan[i].bw_os = HAL_PRIME_CHNL_OFFSET_UPPER;
		else
			g_chPlan[i].bw_os = HAL_PRIME_CHNL_OFFSET_DONT_CARE;
	}

	for (i = 0; i < chnum; i++)
		printf("probe_chplan[%d]:\tch=%d,\t bw_offset=%d\n",
			i, g_chPlan[i].ch, g_chPlan[i].bw_os);
}

/* switch channel & bandwidth */
static void switch_channel(int chidx)
{
	char cmdstr[256];

	#ifdef RK8723
        sprintf(cmdstr,"echo %d %d 1 > /proc/net/%s/%s/monitor\n", \
            g_chPlan[chidx].ch, g_chPlan[chidx].bw_os, wifi_proc_path, ifName);
    #elif defined(AP62XX)
        sprintf(cmdstr,"dhd_priv channel %d\n", chidx);
    #endif
    RK_SYSTEM(cmdstr);
	usleep(TIME_CHSWITCH);
}

/*
 * description:	the way of system to connect to tartget AP.
 */
static int connect_ap(void)
{
	char cmdstr[200];

    printf("connect_ap\n");
	/* wifi disable */
	RK_SYSTEM("killall wpa_supplicant");
    printf("killall wpa_supplicant\n");
	sleep(1);
	/* interface UP and enter monitor mode */
	sprintf(cmdstr,"ifconfig %s up\n", ifName);
    printf("%s\n", cmdstr);
	RK_SYSTEM(cmdstr);
	sleep(2);

	sprintf(cmdstr, "wpa_supplicant -i %s -c %s -Dnl80211 &", ifName, g_wpafile_path);
    printf("wpa_supplicatn -i ...\n");

	RK_SYSTEM(cmdstr);
	sleep(5);

	return 0;
}

/*
 * description:	the way of system to get IP address.
 */
static int request_ip(void)
{
	char cmdstr[250];

	//sprintf(cmdstr, "dhcpcd %s", ifName);
    //sprintf(cmdstr, "/etc/init.d/S41dhcpcd restart"); //delete by 926 @20190624
    printf("check file :/etc/wifi/udhcpc_wlan0 !!!!\n");
    sprintf(cmdstr, "/etc/wifi/udhcpc_wlan0 restart");
    //sprintf(cmdstr, "udhcpc wlan0");
	RK_SYSTEM(cmdstr);
	sleep(3);
	return 0;
}
static int get_rtheader_len(u8 *buf, size_t len)
{
	struct ieee80211_radiotap_header *rt_header;
	u16 rt_header_size;

	rt_header = (struct ieee80211_radiotap_header *)buf;
	/* check the radiotap header can actually be present */
	if (len < sizeof(struct ieee80211_radiotap_header))
		return -EILSEQ;
	 /* Linux only supports version 0 radiotap format */
	 if (rt_header->it_version)
		return -EILSEQ;
	rt_header_size = le16toh(rt_header->it_len);
	 /* sanity check for allowed length and radiotap length field */
	if (len < rt_header_size)
		return -EILSEQ;
	return rt_header_size;
}
//寮�濮媠niffer锛屽苟璁剧疆鍥炶皟鍑芥暟
typedef void (*msm_sniffer_callback_t)(unsigned char *buf, int len,void* user_data);

msm_sniffer_callback_t ProcessPacket;
#ifdef RK8723
void doRecvfrom(void)
{
	int NumTotalPkts;
	int sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	u8 buf[BUF_SIZ];
	int rt_header_len = 0;

	//Create a raw socket that shall sniff
	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
	printf("doRecvfrom\n");
	if ((sockfd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) == -1) {
		printf("listener: socket");
		return;
	}
	/* Set interface to promiscuous mode - do we need to do this every time? */
	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	ioctl(sockfd, SIOCGIFFLAGS, &ifopts);

	ifopts.ifr_flags |= IFF_PROMISC;
	ioctl(sockfd, SIOCSIFFLAGS, &ifopts);
	/* Allow the socket to be reused - incase connection is closed prematurely */
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &sockopt, sizeof sockopt) == -1) {
		printf("setsockopt");
		close(sockfd);
		exit(EXIT_FAILURE);
	}
	/* Bind to device */
	if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, ifName, IFNAMSIZ-1) == -1)	{
		printf("SO_BINDTODEVICE");
		close(sockfd);
		exit(EXIT_FAILURE);
	}

	while(1) {
		numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
		//printf("listener: got packet %d bytes , total packet Num :%d\n", numbytes,NumTotalPkts);
		if (numbytes < 0) {
			printf("Recvfrom error , failed to get packets\n");
			goto thr_exit;
		}

		// Get length of Radiotap header
		if ((rt_header_len = get_rtheader_len(buf, (size_t)numbytes)) < 1){
			printf("Receive nothing,continue\n");
			continue;
		}

		if(g_sniffer_status == RTW_PROMISC_DISABLE){
			goto thr_exit;
		}
		//Now process the packet
		//ProcessPacket 闇�瑕佷綘浠嚜宸卞幓瀹炵幇鍜岃В鏋�
                ProcessPacket(buf + rt_header_len, numbytes-rt_header_len, NULL);
	}
thr_exit:
    printf("Process socket exit,close the socket\n");
	close(sockfd);
	return ;
}
#elif defined(AP62XX)
unsigned short protocol = 0x0003;
void doRecvfrom(void){
    struct ifreq ifr;
    struct sockaddr_ll ll;
    int fd;


    fd = socket(PF_PACKET, SOCK_RAW, htons(protocol));
    printf("fd = %d\n", fd);

    memset(&ifr, 0, sizeof(ifr));

    strncpy(ifr.ifr_name, ifName, sizeof(ifr.ifr_name));

    printf("ifr.ifr_name = %s\n", ifr.ifr_name);

    if (ioctl(fd, SIOCGIFINDEX, &ifr) < 0) {
        close(fd);
        printf("get infr fail\n");
        return -1;
    }

    memset(&ll, 0, sizeof(ll));
    ll.sll_family = PF_PACKET;
    ll.sll_ifindex = ifr.ifr_ifindex;
    ll.sll_protocol = htons(protocol);
    if (bind(fd, (struct sockaddr *) &ll, sizeof(ll)) < 0) {
        printf("bind fail\n");
        close(fd);
        return -1;
    }


    while(1) {

        unsigned char buf[2300];
        int res;
        socklen_t fromlen;
        int i = 0;

        memset(&ll, 0, sizeof(ll));
        fromlen = sizeof(ll);
        res = recvfrom(fd, buf, sizeof(buf), 0, (struct sockaddr *) &ll,&fromlen);

        if (res < 0) {
            printf("res < 0\n");
            return -1;
        } else {
            //Now process the packet
            //ProcessPacket 闇�瑕佷綘浠嚜宸卞幓瀹炵幇鍜岃В鏋�
            ProcessPacket(buf,res, NULL);
        }
    }

    close(fd);

    return 0;
}
#endif
int app_getStaIpAddr(int *ipAddr,char *inf)
{
        char buffer[128] = { 0 }; int offset = 0;
        unsigned char *ptr;
        char *shell_cmd = malloc(128);
        if(shell_cmd == 0){
		return -1;
        }
        memset(shell_cmd, 0 ,128);
        memcpy(shell_cmd,"ifconfig ",strlen("ifconfig "));
        strncat(shell_cmd,inf,strlen(inf));
        strcat(shell_cmd," | head -n 2 | tail -n 1");
//        FILE *fp = popen("ifconfig repeat0 | head -n 2 | tail -n 1", "r"); //wired repeater
        FILE *fp = popen(shell_cmd, "r"); //wired repeater
        free(shell_cmd);
        if (fp)
        {
                memset(buffer, 0x00, sizeof(buffer));
                fgets(buffer, sizeof(buffer) - 1, fp); offset = strlen(buffer); buffer[offset - 1] = '\0';
                if(buffer){
                        ptr = strstr(buffer, "inet addr:");
                }else{
                        return -1;
                }
                if (ptr)
                {
                        ptr = strtok(ptr + 10, " ");

                        //puts(ptr);
                        sscanf(ptr, "%d.%d.%d.%d", &ipAddr[3], &ipAddr[2], &ipAddr[1], &ipAddr[0]);
                }
                printf("ms_devDiscovery_getStaIpAddr: %d.%d.%d.%d\n", ipAddr[3],ipAddr[2],ipAddr[1],ipAddr[0]);
        }
        if(((ipAddr[0]|ipAddr[1]|ipAddr[2]|ipAddr[3])==0)||((ipAddr[0]>255)||(ipAddr[1]>255)||(ipAddr[2]>255)||(ipAddr[3]>255))){
                return -1;
        }
        if (fp)
                pclose(fp);
        return 0;
}
/* start hostapd */
int wlan_accesspoint_start(const char *ssid, const char *password, int hide_ssid)
{
    char cmdline[256] = {0};
/*
    create_hostapd_file(ssid, password, hide_ssid);

    RK_SYSTEM("iw phy0 interface add p2p0 type managed");
    RK_SYSTEM("killall dnsmasq");
    RK_SYSTEM("killall hostapd");
    sprintf(cmdline, "ifconfig %s up", Softap_Name);
    RK_SYSTEM(cmdline);
    sprintf(cmdline, "ifconfig %s 10.201.126.1 netmask 255.255.255.0", Softap_Name);
    RK_SYSTEM(cmdline);
    sprintf(cmdline, "route add default gw 10.201.126.1 %s", Softap_Name);
    RK_SYSTEM(cmdline);

	create_dnsmasq_file();
	RK_SYSTEM("killall dnsmasq");

    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline, "dnsmasq -C %s --interface=%s", DNSMASQ_CONF_DIR, Softap_Name);
    RK_SYSTEM(cmdline);

    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline, "hostapd %s &", HOSTAPD_CONF_DIR);
*/
	/* wifi disable */
	RK_SYSTEM("killall wpa_supplicant");
	sleep(1);
    //sprintf(cmdline, "softapDemo %s %s &", ssid, password); //delete by 926 @20190624
    printf("softap_up %s %s\n", ssid, password);
    sprintf(cmdline, "softap_up %s %s", ssid, password);
    RK_SYSTEM(cmdline);

    sleep(3);//add by 926 @20190625 for debug
    return 1;
}

int wlan_ap_stop(void)
{
    printf("softap_down\n");
#if 0 //delete by 926 @20190624
	RK_SYSTEM("killall hostapd");
	RK_SYSTEM("killall dnsmasq");
	RK_SYSTEM("dhd_priv iapsta_disable ifname wlan1");
	RK_SYSTEM("ip -f inet addr delete dev wlan1");
	RK_SYSTEM("ifconfig wlan1 down");
	//RK_SYSTEM("iw dev wlan1 del");
#else
    RK_SYSTEM("killall hostapd");
    RK_SYSTEM("killall dnsmasq");
    usleep(1000000);
    system("ifconfig wlan0 down");
    printf("wlan0 down request...\n");
    usleep(1000000);
    RK_SYSTEM("echo 0 > /proc/sys/net/ipv4/ip_forward");
#endif
}

int  wlan_sta_start(const char *ssid, const char *password,int flag){

	int err;//flag = FLAG_WPA2;
#if 10
	wlan_ap_stop();
    if ((flag & FLAG_WPA) || (flag & FLAG_WPA2))
	    err = store_cfgfile(g_wpafile_path, ssid, password, CFGFILE_WPA);
	else if (flag & FLAG_WEP)
		err = store_cfgfile(g_wpafile_path, ssid, password, CFGFILE_WEP);
	else
		err = store_cfgfile(g_wpafile_path, ssid, password, CFGFILE_OPEN);

	connect_ap();
	request_ip();
    sleep(1);
#else
	char cmdstr[200];
    sprintf(cmdstr, "wifi_connect_ap_test %s %s", ssid, password);
    RK_SYSTEM(cmdstr);
#endif
	return 0;
}

//start sniffer mode
int wifi_enter_promisc_mode(){
	return 0;
}

wifi_set_promisc(sniffer_status onoff , msm_sniffer_callback_t msm_sniffer_callback,u8 res){
	pthread_t tid;
	int rc ;
	wifi_monitor_mode_onoff(onoff,ifName);
	ProcessPacket = msm_sniffer_callback;
	g_sniffer_status = onoff;
	if(onoff){
		rc = pthread_create(&tid,NULL,(void *)&doRecvfrom,NULL);
        if(rc){
                printf("ERROR: return code from pthread_create() is %d", rc);
                exit(1);
        }
	}
}
//璁剧疆淇￠亾
int wifi_set_channel(u8 ch){
	switch_channel(ch);
}

//int main(int argc, char *argv[])
//{
//	int err, chidx = 0;
//	char cmdstr[256];
//	int chnum = sizeof(g_chPlan) / sizeof(g_chPlan[0]);
//	char ssid[32];
//	char passwd[32];
//	int flag, count;
//
//	/* wifi disable */
//	RK_SYSTEM("killall wpa_supplicant");
//	sleep(1);
//	/* if driver support concurrent mode, another interface(p2p0) MUST be in idle state. */
//	sprintf(cmdstr,"ifconfig %s down\n", Softap_Name);
//
//	RK_SYSTEM(cmdstr);
//	/* interface UP and enter monitor mode */
//	sprintf(cmdstr,"ifconfig %s up\n", ifName);
//	RK_SYSTEM(cmdstr);
//	sleep(2);
//
//	wifi_monitor_mode_onoff(FALSE, ifName);
//	wifi_monitor_mode_onoff(TRUE, ifName);
//
//	init_chplan(chnum);
//
//	//while (1) {
//	count = 12;
//	while (count--) {
//		switch_channel(chidx);
//
//		//TODO ... ...
//
//		if (++chidx == chnum)
//			chidx = 0;
//	}
//
//	wifi_monitor_mode_onoff(FALSE, ifName);
//
//
//	/* hostapd mode
//	 * char *name: AP name
//	 * const char *password: NULL represents open
//	 * int hide_ssid: whether to hide SSID
//	 */
////	wlan_accesspoint_start("midea_ap_test2", NULL, 0);
//
//	return 0;
//}
