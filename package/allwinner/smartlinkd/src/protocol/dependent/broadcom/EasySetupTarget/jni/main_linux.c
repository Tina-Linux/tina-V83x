#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include "easy_setup.h"
#include "aw_smartlinkd_connect.h"

int killed = 0;
int debug_enable = 0;
extern int tries;
int signal_exit=0;
int f_send_finished = 0;
void usage() {
    printf("-h: show help message\n");
    printf("-d: show debug message\n");
    printf("-k <v>: set 16-char key for all protocols\n");
    printf("-p <v>: bitmask of protocols to enable\n");
	printf("-s,\tWhether to connect smartlinkd server(yes or no),default yes\n");
    printf("  0x%04x - bcast\n", 1<<EASY_SETUP_PROTO_BCAST);
    printf("  0x%04x - neeze\n", 1<<EASY_SETUP_PROTO_NEEZE);
    printf("  0x%04x - Air Kiss\n", 1<<EASY_SETUP_PROTO_AKISS);
    printf("  0x%04x - Xiaoyi\n", 1<<EASY_SETUP_PROTO_XIAOYI);
    printf("  0x%04x - changhong\n", 1<<EASY_SETUP_PROTO_CHANGHONG);
    printf("  0x%04x - jingdong\n", 1<<EASY_SETUP_PROTO_JINGDONG);
    printf("  0x%04x - jd JoyLink\n", 1<<EASY_SETUP_PROTO_JD);
}

static void signal_handler(int sig) {
    printf("aborted\n");
    killed = 1;
}
void localsigroutine(int dunno){
	LOGE("sig: %d coming!\n",dunno);
	switch(dunno){
		case SIGINT:
		case SIGQUIT:
		case SIGHUP:
		{
            tries = 0;
            signal_exit =1;
			//exit(0);
            break;
		}
		case SIGPIPE:
		{
			//When the client is closed after start scaning and parsing,
			//this signal will come, ignore it!
			LOGD("do nothings for PIPE signal\n");
            break;
		}
	}
}

int thread_handle(char *buf, int length)
{
    int ret;

    ret = strcmp(buf,"OK");
    if(0 == ret)
    {
	f_send_finished = 1;
	printf("thread_handle of cooee: recieve OK from server!\n");
	return THREAD_EXIT;
    }
    return THREAD_CONTINUE;
}

int main(int argc, char* argv[])
{
    int ret;
    int len;
    uint16 val;
	char server_state[5]="yes";
//add by henrisk --allwinnertech

	signal(SIGHUP,localsigroutine);
	signal(SIGQUIT,localsigroutine);
	signal(SIGINT,localsigroutine);
	signal(SIGPIPE,localsigroutine);

    struct _cmd c;
    struct _info *info = &c.info;
    memset(info,0,sizeof(struct _info));
    info->protocol = AW_SMARTLINKD_PROTO_FAIL;

    for (;;) {
        int c = getopt(argc, argv, "dhk:p:s:");
        if (c < 0) {
            break;
        }

        switch (c) {
            case 'd':
                debug_enable = 1;
                break;
            case 'k':
                bcast_set_key(optarg);
                bcast_set_key_qqcon(optarg);
                neeze_set_key(optarg);
                neeze_set_key_qqcon(optarg);
                akiss_set_key(optarg);
                //jingdong_set_key(optarg);
                //jd_set_key(optarg);
                break;
            case 'p':
                sscanf(optarg, "%04x", (uint32*)&val);
                easy_setup_enable_protocols(val);
                break;
			case 's':
                strncpy(server_state,optarg,sizeof(server_state));
                break;
            case 'h':
                usage();
                return 0;
            default:
                usage();
                return 0;
        }
    }

    //signal(SIGINT, signal_handler);
    //signal(SIGTERM, signal_handler);

    ret = easy_setup_start();
    if (ret) goto end;

    /* query for result, blocks until bcast comes or times out */
    ret = easy_setup_query();
    if (!ret) {
        char ssid[33]; /* ssid of 32-char length, plus trailing '\0' */
        ret = easy_setup_get_ssid(ssid, sizeof(ssid));
        if (!ret) {
            printf("ssid: %s\n", ssid);
            memcpy(info->base_info.ssid,ssid,sizeof(ssid));
        }

        char password[65]; /* password is 64-char length, plus trailing '\0' */
        ret = easy_setup_get_password(password, sizeof(password));
        if (!ret) {
            printf("password: %s\n", password);
            memcpy(info->base_info.password,password,sizeof(password));
        }

        uint8 protocol;
        ret = easy_setup_get_protocol(&protocol);
        if (ret) {
            printf("failed getting protocol.\n");
        } else if (protocol == EASY_SETUP_PROTO_BCAST) {
			info->protocol = AW_SMARTLINKD_PROTO_COOEE;
            char ip[16]; /* ipv4 max length */
            ret = bcast_get_sender_ip(ip, sizeof(ip));
            if (!ret) {
                printf("sender ip: %s\n", ip);
                memcpy(info->ip_info.ip,ip,sizeof(ip));
            }

            uint16 port;
            ret = bcast_get_sender_port(&port);
            if (!ret) {
                printf("sender port: %d\n", port);
                info->ip_info.port = port;
            }
        } else if (protocol == EASY_SETUP_PROTO_NEEZE) {
            char ip[16]; /* ipv4 max length */
            ret = neeze_get_sender_ip(ip, sizeof(ip));
            if (!ret) {
                printf("sender ip: %s\n", ip);
            }

            uint16 port;
            ret = neeze_get_sender_port(&port);
            if (!ret) {
                printf("sender port: %d\n", port);
            }
        } else if (protocol == EASY_SETUP_PROTO_AKISS) {

            info->protocol = AW_SMARTLINKD_PROTO_AKISS;

            uint8 random;
            ret = akiss_get_random(&random);
            if (!ret) {
                printf("random: 0x%02x\n", random);
                info->airkiss_random = random;
            }
        } else if (protocol == EASY_SETUP_PROTO_CHANGHONG) {
            uint8 sec;
            ret = changhong_get_sec_mode(&sec);
            if (!ret) {
                printf("sec mode: 0x%02x\n", sec);
            }
	#if 0
        } else if (protocol == EASY_SETUP_PROTO_JINGDONG) {
            uint8 sec;
            ret = jingdong_get_sec_mode(&sec);
            if (!ret) {
                printf("sec mode: 0x%02x\n", sec);
            }
        } else if (protocol == EASY_SETUP_PROTO_JD) {
            uint8 crc;
            ret = jd_get_crc(&crc);
            if (!ret) {
                printf("crc: 0x%02x\n", crc);
            }

            uint32 ip;
            ret = jd_get_ip(&ip);
            if (!ret) {
                printf("ip: 0x%08x\n", ip);
            }

            uint16 port;
            ret = jd_get_port(&port);
            if (!ret) {
                printf("port: 0x%04x\n", port);
            }
        } else if (protocol == EASY_SETUP_PROTO_XIAOYI) {
            uint8 buf[128];
            uint8 len = sizeof(buf);
            ret = xiaoyi_get_buffer(&len, buf);
            if (!ret) {
                printf("buf(%d) - ", len);
                int i;
                for (i=0; i<len; i++) printf("%02x ", buf[i]);
                printf("\n");
            }
	#endif
        }

#if 1
        /* if easy_setup_get_security() returns -1, try it more times */
        int tries = 3;
        while (tries--) {
	    printf("main of cooee: The %drd time trying to get security\n", (3-tries));
            ret = easy_setup_get_security();
            if (ret != -1) break;
        }
	if(-1 != ret)
	{
            printf("security: ");
            if (ret == WLAN_SECURITY_WPA2) printf("wpa2\n");
            else if (ret == WLAN_SECURITY_WPA) printf("wpa\n");
            else if (ret == WLAN_SECURITY_WEP) printf("wep\n");
            else if (ret == WLAN_SECURITY_NONE) printf("none\n");
            else printf("wpa2");
	    info->base_info.security = ret;
	}
	else
	{
	    printf("main of cooee: Failed to get security!\n");
	}
#endif
    }

    /* must do this! */
    easy_setup_stop();
end:
    if(signal_exit == 1){
        printf("signal exit!!\n");
        exit(0);
    }

	//wlan0 down && wlan0 up
	usleep(300*1000);
	system("ifconfig wlan0 down");
	usleep(100*1000);
	system("ifconfig wlan0 up");
	usleep(300*1000);

    //add by henrisk -- allwinnertech
    if(strcmp(server_state,"yes")==0){
	aw_smartlinkd_init(0,thread_handle);
	if(aw_easysetupfinish(&c) != sizeof(struct _cmd))
			printf("main of cooee: failed to send the information!\n");
	while(f_send_finished == 0)
			sleep(1);
    }
    return 0;
}
