#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <pthread.h>
#include <memory.h>

#include <netinet/ether.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if.h>
#include <sys/ioctl.h>
#include <sys/types.h>


#ifdef ANDROID_ENV
#include <net/if_arp.h>
#endif

#include "airkiss.h"
#include "rtw_cusdef.h"

#include "connect.h"
int debug_enable = 1;

struct _cmd c;
struct _info *info = &c.info;

airkiss_context_t akcontex;
const airkiss_config_t akconf =
{
	(airkiss_memset_fn)&memset,
	(airkiss_memcpy_fn)&memcpy,
	(airkiss_memcmp_fn)&memcmp,
	(airkiss_printf_fn)&printf
};
airkiss_status_t ak_status;
airkiss_result_t ak_result;

int	g_debuglv;
u8	g_stoprecv = FALSE, g_reinit;
char	ifName[IFNAMSIZ];
char	g_bssid[6]={0, 0, 0, 0, 0, 0};
char	g_wpafile_path[512];
int	g_config_timeoute = -1;		/*-1 is always parse, 0 stop parse, >0 parse all packets.*/
char	g_ak_key[512];
struct timeval g_tv;
const unsigned char bc_mac[6]={0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

void wifi_do_scan(void);
void wifi_disable(void);
void wifi_enable(void);

//   =====================	Custimize part	====================================

//const u8 def_chPlan[]= {1,6,11,2,3,4,5,7,8,9,10,12,13,36,40,44,48,52,56,60,64,100,104,108,112,116,132,136,140,149,153,157,161,165};
const u8 def_chPlan[]= {1,6,11,2,3,4,5,7,8,9,10,12,13};
#define MAX_CHNUM		50

#define DEFAULT_IF		"wlan0"
#define DEFAULT_WPAFILE_PATH	"./wpa.conf"
#define DEFAULT_AK_KEY		"1234567890123456"
#define ACK_DEST_PORT		10000
#define SEND_ACK_TIMES		30

#define CFGFILE_OPEN				0
#define CFGFILE_WPA				1
#define CFGFILE_WEP				2


#define	PATTERN_WEP		"[WEP]"
#define	PATTERN_WPA		"[WPA]"
#define	PATTERN_WPA2		"[WPA2]"
#define	PATTERN_WPS		"[WPS]"
#define	PATTERN_IBSS		"[IBSS]"
#define	PATTERN_ESS		"[ESS]"
#define	PATTERN_P2P		"[P2P]"
#define FLAG_WPA		0x0001
#define FLAG_WPA2		0x0002
#define FLAG_WEP		0x0004
#define FLAG_WPS		0x0008
#define FLAG_IBSS		0x0010
#define FLAG_ESS		0x0020
#define FLAG_P2P		0x0040

#define MAX_LINE_LEN		1024
#define MAX_SCAN_TIMES		10

inline void RTK_SYSTEM(const char *cmd)
{
	DEBUG_PRINT("shell:  %s\n", cmd);
	system(cmd);
}

/*
description:	the way of system to connect to tartget AP.
*/
int connect_ap(void)
{
	char cmdstr[200];

#ifndef ANDROID_ENV
	//  reinit module status
	wifi_disable();
	wifi_enable();
#ifdef CONFIG_IOCTL_CFG80211
	sprintf(cmdstr, "wpa_supplicant -i %s -c %s -Dnl80211 &", ifName, g_wpafile_path);
#else
	sprintf(cmdstr, "./wpa_supplicant -i %s -c %s -Dwext &", ifName, g_wpafile_path);
#endif
	RTK_SYSTEM(cmdstr);
	sleep(5);
#else
	sprintf(cmdstr, "rmmod %s", WIFI_MODULE_NAME);
	RTK_SYSTEM(cmdstr);
	RTK_SYSTEM("svc wifi enable");
#endif
	return 0;
}

/*
description:	the way of system to get IP address.
*/
int request_ip(void)
{
	char cmdstr[250];

	sprintf(cmdstr, "ifconfig %s 192.168.0.130", ifName);
	//sprintf(cmdstr, "dhclient %s", ifName);
	RTK_SYSTEM(cmdstr);
	sleep(3);
	return 0;
}

//=====================		Custimize part ending	====================================





char *survey_info_path()
{
	static char path[200];

	memset(path, 0, 200);
	sprintf(path, "/proc/net/%s/%s/survey_info", PROC_MODULE_PATH, ifName);
	return path;
}


/*
input:	@before		: string before unicode procedure.

description:	process unicode(ex chinese word)  SSID of AP,
*/

void wifi_do_scan(void)
{
	char cmdstr[250], fwstats[200], readline[MAX_LINE_LEN];
	FILE *fd = NULL;
	char *ch = NULL;
	int fwstatus = 0, i;

	sprintf(cmdstr,"echo 1 > /proc/net/%s/%s/survey_info\n",
			PROC_MODULE_PATH, ifName);
	RTK_SYSTEM(cmdstr);

	//  poll /proc/../fwstatus and wait for scan done. wait 10sec at most.
	sprintf(fwstats,"/proc/net/%s/%s/fwstate", PROC_MODULE_PATH, ifName);
	for (i = 0; i< 10; i++) {
		sleep(1);
		fd = fopen(fwstats, "r");
		if( fd == NULL ) {
			printf("file[%s] open can not create file !!! \n", fwstats);
			return;
		}
		memset(readline, 0, sizeof(char)*MAX_LINE_LEN );
		fgets(readline, MAX_LINE_LEN , fd);
		fclose(fd);
		if (strlen(readline) == 0)
			continue;
		if ((ch = strstr(readline, "=")) != NULL)
			fwstatus = (int)strtol(ch+1, NULL, 0);
		if ((fwstatus & _FW_UNDER_SURVEY) == 0)
			break;
	}
}

void wifi_disable(void)
{
	char cmdstr[250];
#ifdef ANDROID_ENV
	RTK_SYSTEM("svc wifi disable");
	RTK_SYSTEM("sleep 5");
#else
	RTK_SYSTEM("killall wpa_supplicant");
	sprintf(cmdstr, "rmmod %s", WIFI_MODULE_NAME);
	RTK_SYSTEM(cmdstr);
#endif
}

void wifi_enable(void)
{
	char cmdstr[250];
	sprintf(cmdstr, "insmod %s", WIFI_MODULE_NAME);
	RTK_SYSTEM(cmdstr);
	sleep(1);

	//interface UP and enter monitor mode
	sprintf(cmdstr,"ifconfig %s up\n",ifName);
	RTK_SYSTEM(cmdstr);
	sleep(2);
}

char *ssid_rework(const char *before)
{
	char *ch2, *ch1, *ch3;
	unsigned int i, idx = 0;
	static char after[32 * 4 + 1];

	memset(after, 0, 32 * 4 + 1);
	if (before == NULL)
		return after;
	// process unicode SSID, without '"'
	if (((ch1 = strstr(before, "\\x")) != NULL)
		&&((ch2 = strstr(ch1+1, "\\x")) != NULL)
		&&((ch3 = strstr(ch2+1, "\\x")) != NULL)
		&&(ch1 == before)
		&&((ch2-ch1) == 4)
		&&((ch3-ch2) == 4)) {
		for (i = 0; i < strlen(before); i++) {
			if ((before[i]=='\\') && (before[i+1]=='x')) {
				i++;
				continue;
			}
			after[idx++] = before[i];
		}
	}else {	// ASCII SSID, need '"'
		after[0] ='"';
		strcpy(after+1, before);
		after[strlen(before) + 1] = '"';
	}
	return after;
}

/*
input:	@filepath		: path of wpa_supplicant config file which need to be stored.
	@ssid		: AP's ssid
	@cfgfile_type	: the security type of AP

description:	store the result as  wpa_supplicant config file  in given file path,
*/
static int store_cfgfile(const char *filepath, char *ssid, int cfgfile_type)
{
	FILE *fd;
	char commset[]={"update_config=1\nctrl_interface=%s\neapol_version=1\nfast_reauth=1\n"};
	char WPAstr[]={"ap_scan=1\nnetwork={\n\tssid=%s\n\tscan_ssid=1\n\tpsk=\"%s\"\n}\n"};
	char OPENstr[]={"ap_scan=1\nnetwork={\n\tssid=%s\n\tkey_mgmt=NONE\n\tscan_ssid=1\n}\n"};
	char WEPstr[]={"ap_scan=1\nnetwork={\n\tssid=%s\n\tkey_mgmt=NONE\n\twep_key0=\"%s\"\n\twep_tx_keyidx=0\n\tscan_ssid=1\n}\n"};
	char CmdStr[2048], passwd[65];

	memset(passwd, 0, 65);
	fd=fopen(filepath, "w+");
	if( fd == NULL ) {
		printf("file open can not create file !!! \n");
		return -ENOENT;
	}
	sprintf(CmdStr, commset, ifName);
	fprintf(fd,"%s", CmdStr);

	strcpy(info->base_info.ssid,ssid_rework(ssid));
	info->protocol = PROTO_AKISS;
	info->airkiss_random = ak_result.random;

	switch (cfgfile_type) {
	case CFGFILE_OPEN :
		sprintf(CmdStr, OPENstr, ssid_rework(ssid));
		info->base_info.security = SECURITY_NONE;

		break;
	case CFGFILE_WPA :
		sprintf(CmdStr, WPAstr, ssid_rework(ssid), ak_result.pwd);
		info->base_info.security = SECURITY_WPA;
		strcpy(info->base_info.password,ak_result.pwd);
		break;
	case CFGFILE_WEP :
		sprintf(CmdStr, WEPstr, ssid_rework(ssid), ak_result.pwd);
		info->base_info.security = SECURITY_WEP;
		strcpy(info->base_info.password,ak_result.pwd);
		break;
	default :
		fclose(fd);
		return -EINVAL;
		break;
	}
	DEBUG_PRINT("%s\n",CmdStr);
	fprintf(fd,"%s", CmdStr);
	fclose(fd);
	return 0;
}

void printf_encode(char *txt, size_t maxlen, const u8 *data, size_t len)
{
	char *end = txt + maxlen;
	size_t i;

	for (i = 0; i < len; i++) {
		if (txt + 4 > end)
			break;

		switch (data[i]) {
		case '\"':
			*txt++ = '\\';
			*txt++ = '\"';
			break;
		case '\\':
			*txt++ = '\\';
			*txt++ = '\\';
			break;
		case '\e':
			*txt++ = '\\';
			*txt++ = 'e';
			break;
		case '\n':
			*txt++ = '\\';
			*txt++ = 'n';
			break;
		case '\r':
			*txt++ = '\\';
			*txt++ = 'r';
			break;
		case '\t':
			*txt++ = '\\';
			*txt++ = 't';
			break;
		default:
			if (data[i] >= 32 && data[i] <= 127) {
				*txt++ = data[i];
			} else {
				txt += snprintf(txt, end - txt, "\\x%02x",
						   data[i]);
			}
			break;
		}
	}

	*txt = '\0';
}

char* wpa_ssid_txt(const u8 *ssid, size_t ssid_len)
{
	static char ssid_txt[32 * 4 + 1];

	if (ssid == NULL) {
		ssid_txt[0] = '\0';
		return ssid_txt;
	}

	printf_encode(ssid_txt, sizeof(ssid_txt), ssid, ssid_len);
	return ssid_txt;
}
/*
input:	@scan_res	: path of scan_result file
	@target_bssid	: bssid of target AP
output:	@		: AP's ssid, do nothing if strlen() is not zero.
	@flag		: indicate property of WPA, WPA2, WPS, IBSS, ESS, P2P.
	@channel		: AP's channel.

description:	parse scan_result to get the information of corresponding AP by BSSID.
*/
static int parse_scanres(char *scan_res, char *target_bssid, char *channel,
			 const char *ssid, u16 *flag)
{
	char readline[MAX_LINE_LEN];
	char *ch, *idx, *bssid, *flag_s;
	FILE* fp = NULL;
	int found = -EAGAIN;

	fp = fopen(scan_res, "r");
	if (!fp) {
	    printf("%s:Unable to open file [%s] \n", __func__, scan_res);
	    return -ENOENT;
	}

	while (! feof(fp)) {
		*flag = 0;
		*channel = 0;
		if (!fgets(readline, MAX_LINE_LEN , fp))
			break;
		idx = strtok(readline, " \r\n");
		if (!idx)
			continue;
		bssid = strtok(NULL, " \r\n");
		if (!bssid || (strlen(bssid) < 17)
			|| (strcmp(bssid, target_bssid) != 0))
			continue;
		found = TRUE;
		// Channel
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		strcpy(channel, ch);
		// RSSI
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		// SdBm
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		// Noise
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		// age
		ch = strtok(NULL, " \r\n");
		if (!ch)
			continue;
		// flag
		flag_s = strtok(NULL, " \r\n");
		if (!flag_s)
			continue;
		if ((ch = strstr(flag_s, PATTERN_WPA)) != NULL)
			*flag |= FLAG_WPA;
		if ((ch = strstr(flag_s, PATTERN_WPA2)) != NULL)
			*flag |= FLAG_WPA2;
		if ((ch = strstr(flag_s, PATTERN_WEP)) != NULL)
			*flag |= FLAG_WEP;
		if ((ch = strstr(flag_s, PATTERN_WPS)) != NULL)
			*flag |= FLAG_WPS;
		if ((ch = strstr(flag_s, PATTERN_IBSS)) != NULL)
			*flag |= FLAG_IBSS;
		if ((ch = strstr(flag_s, PATTERN_ESS)) != NULL)
			*flag |= FLAG_ESS;
		if ((ch = strstr(flag_s, PATTERN_P2P)) != NULL)
			*flag |= FLAG_P2P;

		DEBUG_PRINT("bssid = [%s],  channel=[%s] flag=%04X \n",
					bssid, channel, *flag);
		break;
	}
	fclose(fp);
	return found;
}

int collect_scanres(void)
{
#ifdef ANDROID_ENV
	char cmdstr[250];
#endif
	char ch[5], ssid[64], bssid_str[64];
	int ret = -100, i;
	u16 flag = 0;

	memset(ssid, 0, 64);
	strcpy(ssid, ak_result.ssid);
	memset(bssid_str, 0, 64);
	memset(ch, 0, 5);
	sprintf(bssid_str, "%02x:%02x:%02x:%02x:%02x:%02x",
                        (u8)g_bssid[0], (u8)g_bssid[1], (u8)g_bssid[2],
                        (u8)g_bssid[3], (u8)g_bssid[4], (u8)g_bssid[5]);
	printf("%s() target_bssid=[%s] \n",__func__, bssid_str);
	for (i = 0; i < MAX_SCAN_TIMES; i++) {
		wifi_do_scan();
		if ((parse_scanres(survey_info_path(), bssid_str, ch, ssid, &flag))!=TRUE)
			continue;
		if ((flag & FLAG_WPA) || (flag & FLAG_WPA2))
			ret = store_cfgfile(g_wpafile_path, wpa_ssid_txt((const u8*)ssid, strlen(ssid)), CFGFILE_WPA);
		else if (flag & FLAG_WEP)
			ret = store_cfgfile(g_wpafile_path, wpa_ssid_txt((const u8*)ssid, strlen(ssid)), CFGFILE_WEP);
		else
			ret = store_cfgfile(g_wpafile_path, wpa_ssid_txt((const u8*)ssid, strlen(ssid)), CFGFILE_OPEN);
		break;
	}
#ifdef ANDROID_ENV
	sprintf(cmdstr, "cat %s > /data/misc/wifi/wpa_supplicant.conf", g_wpafile_path);
	RTK_SYSTEM(cmdstr);
	RTK_SYSTEM("cat /data/misc/wifi/wpa_supplicant.conf");
	RTK_SYSTEM("chmod 777 /data/misc/wifi/wpa_supplicant.conf");
#endif
	if (ret == -100)
		printf("Error!! ssid:%s not found \n", ssid);
	return ret;
}

/*
	description : open & parse the file (scan_res), collect the ap channel list and channel number
	@ap_chlist : A channel list of all AP's channel by parsing scan result
	@ap_chcnt  : How many channels in @ap_chlist
	Note : same channel may be showed several times because the source not be sorted.
*/
static int get_ap_ch_list(char *scan_res, u8 *ap_chlist, int *ap_chcnt)
{
	char readline[MAX_LINE_LEN];
	char *ch, *value;
	FILE* fp = NULL;
	int  channel, last_channel = 0, linecnt = 0;

	fp = fopen(scan_res, "r");
	if (!fp) {
	    perror("Error opening file");
	    return -ENOENT;
	}
	if ((!ap_chlist) || (!ap_chcnt)) {
	    fclose(fp);
	    return -EINVAL;
	}
	*ap_chcnt = 0;
	while (!feof(fp)) {
		if (!fgets(readline, MAX_LINE_LEN , fp))
			break;
		linecnt++;
		if ((ch = strtok(readline, " \r\n"))!=NULL
			&& ((ch = strtok(NULL, " \r\n"))!=NULL)
			&& ((value = strtok(NULL, " \r\n"))!=NULL)) {

			if ((strcmp(value, "ch") != 0)
				&& (channel = atoi(value))
				&& (last_channel != channel)) {
				ap_chlist[*ap_chcnt] = channel;
				last_channel = channel;
				(*ap_chcnt)++;
			}
		}
	}
	if (linecnt < 2) {
		DEBUG_PRINT("%s: scan result empty \n", scan_res);
		fclose(fp);
		return -EPERM;
	}
	fclose(fp);
	return 0;
}
/*
	1. Perform site-survey procedure for 3 times.
	2. Parse scan result to collect all AP's channel in air
	3. Intersect channel list in step2 and default channel plan,
	    then we got a sniffer channel list.
*/
void probe_chplan(u8 *chlist, int *chnum)
{
	u8 ap_chlist[MAX_CHNUM];
	int i, j, ap_chcnt = 0 , chidx=0;
	int def_chnum = sizeof(def_chPlan)/sizeof(def_chPlan[0]);

	wifi_do_scan();
	memset(ap_chlist, 0, MAX_CHNUM*sizeof(u8));
        if (get_ap_ch_list(survey_info_path(), ap_chlist, &ap_chcnt)==0) {
		for (i=0; i<def_chnum; i++) {
			for (j=0; j<ap_chcnt; j++) {
				if (ap_chlist[j] == def_chPlan[i]) {
					chlist[chidx++] = def_chPlan[i];
					break;
				}
			}
		}
		*chnum = chidx;
	} else {
		memcpy(chlist, def_chPlan, sizeof(u8)*def_chnum);
		*chnum = def_chnum;
	}

	if(g_debuglv > 0) {
		for (i=0; i<ap_chcnt; i++)
			printf("ap_chlist[%d]:%d \n", i, ap_chlist[i]);
		for (i=0; i<*chnum; i++)
			printf("probe_chplan[%d]:%d \n", i, chlist[i]);
	}
}

void wifi_monitor_mode_onoff(u8 onoff, const char *ifName)
{
	char cmdstr[200];

	memset(cmdstr, 0, sizeof(char)*200);

	//iwconfig wlanX mode monitor
	//iw dev wlanX set type monitor
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
	RTK_SYSTEM(cmdstr);
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

void wifi_set_channel(u8 cur_channel)
{
	char cmdstr[200];

	sprintf(cmdstr,"echo %d 0 0 > /proc/net/%s/%s/monitor \n",
			cur_channel, PROC_MODULE_PATH, ifName);
	RTK_SYSTEM(cmdstr);
}


static int airkiss_finish(void)
{
	int8_t err;

	err = airkiss_get_result(&akcontex, &ak_result);
	if (err == 0) {
		printf("airkiss_get_result() ok!");
		printf( "ssid = \"%s\", pwd = \"%s\", ssid_length = %d,"
			"pwd_length = %d, random = 0x%02x\r\n",	ak_result.ssid,
			 ak_result.pwd, ak_result.ssid_length, ak_result.pwd_length, ak_result.random);
	}
	else {
		printf("airkiss_get_result() failed !\r\n");
	}
	return err;
}

void start_airkiss(char *key)
{
	int8_t ret;
	printf("Start airkiss!\r\n");

	ret = airkiss_init(&akcontex, &akconf);

	if (ret < 0) {
		printf("Airkiss init failed!\r\n");
		return;
	}
	#if AIRKISS_ENABLE_CRYPT
	airkiss_set_key(&akcontex, (const unsigned char*)key, strlen(key));
	#endif
	printf("Finish init airkiss!\r\n");
	ak_status = AIRKISS_STATUS_CONTINUE;
	//wifi_station_disconnect();
	//wifi_set_opmode(STATION_MODE);
	//cur_channel = 1;
	//wifi_set_channel(cur_channel);
	//os_timer_setfn(&time_serv, (os_timer_func_t*)time_callback, NULL);
	//os_timer_arm(&time_serv, 100, 1);
	//wifi_set_promiscuous_rx_cb(wifi_promiscuous_rx);
	//wifi_promiscuous_enable(1);
	printf("===============\n%s\n===============\n", airkiss_version());
}



static void print_usage(void)
{
	printf("\n\nUsage: \n\trtw_ak \t-i<ifname> -c<wpa_cfgfile> -p <ak_key> -t <timeout>\n\t\t\t\t[-dD] [-v]");
	printf("\n\nOPTIONS:\n\t");
	printf("-i = interface name\n\t");
	printf("-c = the path of wpa_supplicant config file\n\t");
	printf("-p = airkiss key\n\t");
	printf("-d = enable debug message, -D = more message.\n\t");
	printf("-t = timeout (seconds)\n\t");
	printf("-v = version\t\n\n");
	printf("example:\n\t");
	printf("rtw_ak -i wlan0 -c ./wpa_conf -p 1234567890123456 -D\n");
	return;
}

int parse_argv(int argc, char **argv)
{

	int opt;

	/*	initial variable by default valve	*/
	memset(ifName, 0, IFNAMSIZ);
	strcpy(ifName, DEFAULT_IF);
	memset(g_ak_key, 0, 512);
	strcpy(g_ak_key, DEFAULT_AK_KEY);
//	rtk_sc_set_value(SC_DURATION_TIME, -1);
	strcpy(g_wpafile_path, DEFAULT_WPAFILE_PATH);
	g_debuglv = 0;

	while ((opt = getopt(argc, argv, "i:c:n:t:p:P:m:hdDv")) != -1) {
		switch (opt) {
		case 'i':	// wlan interface
			strcpy(ifName, optarg);
			break;
		case 'c':	// wpa_supplicant config file path
			strcpy(g_wpafile_path, optarg);
			break;
		case 'p':	// ak_key
			strcpy(g_ak_key, optarg);
			break;
		case 'd':	// enable debug message
			g_debuglv = 1;
			break;
		case 'D':	// enable move debug message
			g_debuglv = 2;
			break;
		case 't':	// timeout
			g_config_timeoute = atoi(optarg);
			break;
		case 'v':
			printf("%s -- %s\n", argv[0], PROGRAM_VERSION);
			exit(EXIT_SUCCESS);
		case 'h':
		default: /* '?' */
			print_usage();
			return -EINVAL;
		}
	}

	if(g_debuglv) {
//		char dbg_str[256], dbg_int = 0;

		DEBUG_PRINT("========option parse========\n");
		DEBUG_PRINT("g_ak_key = %s\n", g_ak_key);
		DEBUG_PRINT("timeout = %d\n", g_config_timeoute);
		DEBUG_PRINT("ifName = %s\n",  ifName);
		DEBUG_PRINT("g_wpafile_path = %s\n",  g_wpafile_path);
		DEBUG_PRINT("========================\n");
	}
	return 0;
}

int get_addr(int sockfd, char *mac_addr, u32 *self_ipaddr)
{
	struct ifreq ifopts;

	strncpy(ifopts.ifr_name, ifName, IFNAMSIZ-1);
	if (ioctl(sockfd, SIOCGIFHWADDR, &ifopts)!=0) {
                //printf("%s:SIOCGIFHWADDR error, program exit !! ", __func__);
                return -EIO;
	}
	memcpy(mac_addr, ifopts.ifr_hwaddr.sa_data, ETH_ALEN);
	if (ioctl(sockfd, SIOCGIFADDR, &ifopts)!=0) {
                //printf("%s:SIOCGIFADDR error, program exit !! ", __func__);
                return -EIO;
	}

	*self_ipaddr = ((struct sockaddr_in *)&ifopts.ifr_addr)->sin_addr.s_addr;
	return 0;
}

/*
description:	Continue sending broadcast UDP ack which the payload is ak_result.random.
*/
int rtw_ak_sendack()
{
	struct sockaddr_in s;
	int sockfd, addr_len, broadcast = 1;
	unsigned int self_ipaddr = 0, i;
	unsigned char payload = 0;
	char smac[6];

	DEBUG_PRINT("Broadcast airkiss UDP ack\n ");
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("socket");
		exit(1);
	}

	// this call is what allows broadcast packets to be sent:
	if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcast, sizeof broadcast) == -1) {
		perror("setsockopt (SO_BROADCAST)");
		exit(1);
	}

	get_addr(sockfd, smac, &self_ipaddr);
	self_ipaddr |= htonl(0x000000FF);
	bzero(&s,sizeof(struct sockaddr_in));
	s.sin_family = AF_INET;
	s.sin_port = htons(ACK_DEST_PORT);
	s.sin_addr.s_addr = self_ipaddr;   //htonl(INADDR_BROADCAST);
	payload = ak_result.random;
	addr_len = sizeof(struct sockaddr);
	for(i=0; i < SEND_ACK_TIMES; i++)
		sendto(sockfd,
			(unsigned char *)&payload,
			sizeof(unsigned char),
			0,
			(struct sockaddr *)&s,
			addr_len);
	close(sockfd);
	return 0;
}

/*
	description : Get IP address or not?
	return : ip address obtained. 0
		 otherwise, error code.
*/
int check_ip_timeout(const int timeout)
{
	int sockfd, ret = 0;
	unsigned int self_ipaddr=0;
	char smac[6];
	int i;

	if ((sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1) {
		perror("socket");
		return errno;
	}

	for (i=0; i<timeout; i++) {
		ret = get_addr(sockfd, smac, &self_ipaddr);
		if (self_ipaddr != 0) {
			close(sockfd);
			return 0;
		}
		DEBUG_PRINT("Getting IP address(%02ds) ... \n", i);
		sleep(1);
	}
	close(sockfd);
	return ret;
}

/*
	description : check if execution time is exceeded?
	return : timeout 1, otherwise 0.
*/
static int check_config_timeout(void)
{
	struct timeval local_tv;

	if (g_config_timeoute == -1)
		return 0;
	gettimeofday(&local_tv, NULL);
	if ((local_tv.tv_sec - g_tv.tv_sec) > g_config_timeoute)
		return 1;
	else {
		//DEBUG_PRINT("[%d] seconds pass\n", g_config_timeoute);
		return 0;
	}
}

static void ProcessPacket(u8 *buffer, int size)
{
	u8	*da;
	//u8	*sa;
	u8	*bssid;
	u8	to_fr_ds;
	u8	type;
	airkiss_status_t ret;

	/*	80211 header format
		ver:	2bit
		type:	2bit
		subtype:	4bit
		tods:	1bit
		frds:	1bit
		other:	6bit		*/

	type = *buffer & TYPE_MASK;
//	subtype = (*buffer & SUBTYPE_MASK) >> 4;
	if ((type != TYPE_DATA) || (size < 21))
		return ;

	to_fr_ds = *(buffer + 1) & FRTODS_MASK;
	if (to_fr_ds == 1) {
		//sa = GetAddr2Ptr(buffer);
		bssid = GetAddr1Ptr(buffer);
		da = GetAddr3Ptr(buffer);
	} else if (to_fr_ds == 2) {
		//sa = GetAddr3Ptr(buffer);
		bssid = GetAddr2Ptr(buffer);
		da = GetAddr1Ptr(buffer);
	} else {
		//sa = NULL;
		da = NULL;
		return;
	}

	ret = memcmp(da, bc_mac, ETH_ALEN);

	if (ret == 0) {
		ret = airkiss_recv(&akcontex, buffer, size);
		if (ret == AIRKISS_STATUS_CHANNEL_LOCKED){
			memcpy(g_bssid, bssid,6);
			DEBUG_PRINT("g_bssid=[%02x:%02x:%02x:%02x:%02x:%02x]\n",
					(u8)g_bssid[0], (u8)g_bssid[1], (u8)g_bssid[2],
					(u8)g_bssid[3], (u8)g_bssid[4], (u8)g_bssid[5]);
		}
		printf("ak_sta=%d \n", ak_status);
		if (ret != AIRKISS_STATUS_CONTINUE)
			ak_status = ret;
	}
	return;
}


void doRecvfrom(void)
{
	int NumTotalPkts;
	int sockfd, sockopt;
	ssize_t numbytes;
	struct ifreq ifopts;	/* set promiscuous mode */
	u8 buf[BUF_SIZ], *pkt;
	int rt_header_len = 0;

	//Create a raw socket that shall sniff
	/* Open PF_PACKET socket, listening for EtherType ETHER_TYPE */
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

	NumTotalPkts=0;
	while(1) {
		numbytes = recvfrom(sockfd, buf, BUF_SIZ, 0, NULL, NULL);
		if (numbytes < 0) {
			printf("Recvfrom error , failed to get packets\n");
			goto thr_exit;
		}

		if (g_stoprecv)
			goto thr_exit;

		// Get length of Radiotap header
		if ((rt_header_len = get_rtheader_len(buf, (size_t)numbytes)) < 1)
			continue;
		//Now process the packet
		pkt = (u8 *)buf + rt_header_len;
#ifdef CONFIG_NOFCS
		ProcessPacket(pkt, numbytes - rt_header_len - 4);
#else
		ProcessPacket(pkt, numbytes - rt_header_len);
#endif
		NumTotalPkts++;
	}
thr_exit:
	close(sockfd);
	return ;
}


int main(int argc, char *argv[])
{
	u8 chPlan[MAX_CHNUM];
	int chnum = sizeof(def_chPlan)/sizeof(def_chPlan[0]);
	int err, chidx = 0;
	char cmdstr[256];
	pthread_t tid[2];

	memset(&c,0,sizeof(c));

	// system init
	if ( parse_argv(argc, argv))
		return -EINVAL;
	start_airkiss(g_ak_key);
	memcpy(chPlan, def_chPlan, chnum * sizeof(u8));
	memset(cmdstr,'\0',sizeof(cmdstr));
	gettimeofday(&g_tv, NULL);
	sprintf(cmdstr, "rm -rf %s&", g_wpafile_path);
	RTK_SYSTEM(cmdstr);
	wifi_disable();
	wifi_enable();
//	if (chidx == 0)
//	probe_chplan(chPlan, &chnum);

	wifi_monitor_mode_onoff(TRUE, ifName);
	if (!g_reinit) {
		err = pthread_create(&(tid[0]), NULL,(void *)&doRecvfrom, NULL);
		printf("after pthread_create...\n");
		if (err != 0)
			printf("\ncan't create thread :[%s]", strerror(err));
		else
			printf("\n doRecvfrom Thread created successfully\n");
	}
	g_reinit = FALSE;
	while (1) {
		static u8 channel_lock = FALSE;

		usleep(TIME_CHSWITCH);
		if (check_config_timeout()){
			g_stoprecv = TRUE;
			break;
		}

		if (ak_status == AIRKISS_STATUS_CHANNEL_LOCKED){
			char bssid_str[64], ssid[64], ch[5];
			u16 flag = 0;

			if (channel_lock)		// first AIRKISS_STATUS_CHANNEL_LOCKED
				continue;
			memset(bssid_str, 0, 64);
			memset(ch, 0, 5);
			sprintf(bssid_str, "%02x:%02x:%02x:%02x:%02x:%02x",
					(u8)g_bssid[0], (u8)g_bssid[1], (u8)g_bssid[2],
					(u8)g_bssid[3], (u8)g_bssid[4], (u8)g_bssid[5]);
			if ((parse_scanres(survey_info_path(), bssid_str, ch, ssid, &flag))==TRUE) {
				if (strlen(ch) == 0)
					sprintf(ch, "%d", chPlan[chidx]);
				wifi_set_channel(atoi(ch));
				channel_lock = TRUE;
				DEBUG_PRINT("Channel locked!! staying [ch:%s]\n", ch);
			} else {
				// scan again
				DEBUG_PRINT("Channel locked!! but [%s] not found, scan again.\n", bssid_str);
				wifi_monitor_mode_onoff(FALSE, ifName);
				wifi_do_scan();
				wifi_monitor_mode_onoff(TRUE, ifName);
				wifi_set_channel(chPlan[chidx]);
			}

		} else if (ak_status == AIRKISS_STATUS_CONTINUE) {
			// switch channel & bandwidth
			wifi_set_channel(chPlan[chidx++]);
			if (chidx == chnum) {
				chnum = sizeof(def_chPlan)/sizeof(def_chPlan[0]);	// scan fully channel plan if  first round doesn't success.
				memcpy(chPlan, def_chPlan, chnum*sizeof(u8));		// scan fully channel plan if  first round doesn't success.
				chidx = 0;						// start next scan round,
			}
		} else if (AIRKISS_STATUS_COMPLETE == ak_status) {
				if (airkiss_finish()) {
					// ak_key mismatch
					start_airkiss(g_ak_key);
					chidx = 0;
					ak_status = AIRKISS_STATUS_CONTINUE;
					memset(g_bssid, 0, 6);
					channel_lock = FALSE;
				}else {
					g_stoprecv = TRUE;
					break;
				}
		}
	}
	if (AIRKISS_STATUS_COMPLETE == ak_status) {
		wifi_monitor_mode_onoff(FALSE, ifName);

		//return for linkd
		if (!collect_scanres()) {
			init(0,NULL);
			printf_info(&c);
			easysetupfinish(&c);
			sleep(5);
		}
		return 0;

		if (!collect_scanres()) {
			connect_ap();
#ifndef ANDROID_ENV
			request_ip();
#endif
			if (check_ip_timeout(TIME_CHECK_IP)) {
				printf("WiFi link ready, but unable to get IP address!! program exit!\n");
				exit(EXIT_FAILURE);
			}
			rtw_ak_sendack();
		}
	}

	return 0;
}
