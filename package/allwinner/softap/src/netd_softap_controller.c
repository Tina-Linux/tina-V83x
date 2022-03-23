/*

 */

#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <sys/wait.h>
#include <unistd.h>


/*
#include <netinet/in.h>
#include <arpa/inet.h>
*/

#include <linux/wireless.h>

#include <openssl/evp.h>
#include <openssl/sha.h>

/*
#define LOG_TAG "SoftapController"
*/
/*
#include <cutils/log.h>
*/
/*
#include <netutils/ifc.h>
#include <private/android_filesystem_config.h>
*/

#include "wifi.h"
#include "response_code.h"
/*#include "wifi_property.h"*/
#include "netd_softap_controller.h"
#include "filesystem_config.h"

#if defined(CUSTOMIZED_SOCKET_PATH)
static const char HOSTAPD_CONF_FILE[]    = "/data/config/wifi/hostapd.conf";
#else
static const char HOSTAPD_CONF_FILE[]    = "/etc/wifi/hostapd.conf";
#endif
static const char HOSTAPD_BIN_FILE[]    = "/usr/bin/hostapd";

#ifndef SHA256_DIGEST_LENGTH
#define SHA256_DIGEST_LENGTH 32
#endif

#ifndef WIFI_GET_FW_PATH_AP
#define WIFI_GET_FW_PATH_AP 1
#endif

#ifndef WIFI_GET_FW_PATH_P2P
#define WIFI_GET_FW_PATH_P2P 2
#endif

#ifndef WIFI_GET_FW_PATH_STA
#define WIFI_GET_FW_PATH_STA 0
#endif

#define FW_BUF_SIZE 4096


static pid_t mPid = 0;


/*static char temp_property[PROPERTY_VALUE_MAX] = {0};*/

static void generate_psk(char *ssid, char *passphrase, char *psk_str) {
    unsigned char psk[SHA256_DIGEST_LENGTH];  /*SHA256_DIGEST_LENGTH is declarated in sha.h*/
    int j;
    /* Use the PKCS#5 PBKDF2 with 4096 iterations, declaration in evp.h*/
    PKCS5_PBKDF2_HMAC_SHA1(passphrase, strlen(passphrase),
            (unsigned char *)(ssid), strlen(ssid),
            4096, SHA256_DIGEST_LENGTH, psk);
    for (j=0; j < SHA256_DIGEST_LENGTH; j++) {
        sprintf(&psk_str[j<<1], "%02x", psk[j]);
    }
}

/*
SoftapController::SoftapController()
    : mPid(0) {}

SoftapController::~SoftapController() {
}
*/
static void sig_chld(int signo)
{
    int status;

    printf("Having catch sig_chld!\n");
    waitpid(-1, &status, WNOHANG); /*wait for all child, but do not block*/
}

int start_softap() {
    pid_t pid = 1;
	tRESPONSE_CODE response_code;
	int ret = 0, i = 0;
	int fd0, fd1, fd2;
	struct rlimit rl;
	struct sigaction sa;
/*
	wifi_property_get("hostapd_status", temp_property, "original");
	if (strcmp("running",temp_property) == 0) {
		char path[30];
		wifi_property_get("hostapd_mpid", temp_property, "original");
		sprintf(path ,"%s/%s","/proc",temp_property);
		if(access(path, F_OK)==0){
			printf("Softap already started\n");
			response_code = SOFTAP_STATUS_RESULT;
			return response_code;
		}
	}
*/
/*
    if (mPid) {
        printf("SoftAP is already running");
	response_code = SOFTAP_STATUS_RESULT;
        return response_code;
    }
*/
/*Run this function for the first time in the process, register signal handler*/
    if(!mPid)
    {
        if(signal(SIGCHLD, sig_chld) == SIG_ERR)
            printf("%s: Can't catch SIGCHLD!\n", __func__);
    }

	umask(0);

	if (getrlimit(RLIMIT_NOFILE, &rl) < 0) {
		printf("can't get file limit!\n");
		return -1;
	}

    if ((pid = fork()) < 0) {
        printf("fork failed (%s)", strerror(errno));
		response_code = SERVICE_START_FAILED;
        return response_code;
    }

    if (!pid) {
		setsid();

		sa.sa_handler = SIG_IGN;
		sigemptyset(&sa.sa_mask);
		sa.sa_flags = 0;

		if (sigaction(SIGHUP, &sa, NULL) < 0) {
			printf("can't ignore SIGHUP!\n");
			return -1;
		}

		if ((pid = fork()) < 0) {
			printf("can't fork!\n");
			return -1;
		} else if (pid != 0)
			exit(0);

		if (chdir("/") < 0) {
			printf("can't change directory to /!\n");
			return -1;
		}

		if (rl.rlim_max == RLIM_INFINITY)
			rl.rlim_max = 1024;

		for (i = 0; i < rl.rlim_max; i++)
			close(i);

		fd0 = open("/dev/null", O_RDWR);
		fd1 = dup(0);
		fd2 = dup(0);

        ensure_entropy_file_exists();
        if (execl(HOSTAPD_BIN_FILE, HOSTAPD_BIN_FILE,
                  "-e", WIFI_ENTROPY_FILE,
                  HOSTAPD_CONF_FILE, (char *) NULL)) {
            printf("execl failed (%s)", strerror(errno));
        }
        printf("SoftAP failed to start");
		response_code = SERVICE_START_FAILED;
        return response_code;
    } else {
        mPid = pid;
        printf("SoftAP started successfully");
        usleep(AP_BSS_START_DELAY);
    }
	response_code = SOFTAP_STATUS_RESULT;
    return response_code;
}

int stop_softap() {
	tRESPONSE_CODE response_code;

    if (mPid == 0) {
        printf("SoftAP is not running");
	response_code = SOFTAP_STATUS_RESULT;
	return response_code;
    }

    printf("Stopping the SoftAP service...");
    kill(mPid, SIGTERM);
    waitpid(mPid, NULL, 0);

    mPid = 0;
    printf("SoftAP stopped successfully");
    usleep(AP_BSS_STOP_DELAY);
	response_code = SOFTAP_STATUS_RESULT;
    return response_code;

}

int is_softap_started() {
    return (mPid != 0);
}

/*
 * Arguments:
 *  agrv[0] - "softap"
 *  argv[1] - "set"
 *  argv[2] - wlan interface
 *  argv[3] - SSID
 *  argv[4] - Broadcast/Hidden
 *  argv[5] - Channel
 *  argv[6] - Security:wpa-psk/wpa2-psk/open
 *  argv[7] - Key
 */
int set_softap(int set_num, char *argv[]) {
    char psk_str[2*SHA256_DIGEST_LENGTH+1];
    tRESPONSE_CODE ret = SOFTAP_STATUS_RESULT;
    int i = 0;
    int fd;
    int hidden = 0;
    int channel = AP_CHANNEL_DEFAULT;
/*
    char *wbuf = NULL;
    char *fbuf = NULL;
*/
	char wbuf[FW_BUF_SIZE] = {0};
	char fbuf[FW_BUF_SIZE] = {0};

    if (set_num < 5) {
        printf("Softap set is missing arguments. Please use:");
        printf("softap <wlan iface> <SSID> <hidden/broadcast> <channel> <wpa2?-psk|open> <passphrase>");
		return COMMAND_SYNTAX_ERROR;
    }

    if (!strcasecmp(argv[4], "hidden"))
        hidden = 1;

    if (set_num >= 5) {
        channel = atoi(argv[5]);
        if (channel <= 0)
            channel = AP_CHANNEL_DEFAULT;
    }
/*
    asprintf(&wbuf, "interface=%s\ndriver=nl80211\nctrl_interface="
            "/data/misc/wifi/hostapd\nssid=%s\nchannel=%d\nieee80211n=1\n"
            "hw_mode=g\nignore_broadcast_ssid=%d\n",
            argv[2], argv[3], channel, hidden);
*/
#if defined(CUSTOMIZED_SOCKET_PATH)
    sprintf(wbuf, "interface=%s\ndriver=nl80211\nctrl_interface="
            "/data/config/wifi/hostapd\nssid=%s\nchannel=%d\nieee80211n=1\n"
            "hw_mode=g\nignore_broadcast_ssid=%d\n",
            argv[2], argv[3], channel, hidden);
#else
    sprintf(wbuf, "interface=%s\ndriver=nl80211\nctrl_interface="
            "/etc/wifi/hostapd\nssid=%s\nchannel=%d\nieee80211n=1\n"
            "hw_mode=g\nignore_broadcast_ssid=%d\n",
            argv[2], argv[3], channel, hidden);
#endif

    if (set_num > 7) {
        if (!strcmp(argv[6], "wpa-psk")) {
            generate_psk(argv[3], argv[7], psk_str);
/*
            asprintf(&fbuf, "%swpa=1\nwpa_pairwise=TKIP CCMP\nwpa_psk=%s\n", wbuf, psk_str);
*/
            sprintf(fbuf, "%swpa=1\nwpa_pairwise=TKIP CCMP\nwpa_psk=%s\n", wbuf, psk_str);
        } else if (!strcmp(argv[6], "wpa2-psk")) {
            generate_psk(argv[3], argv[7], psk_str);
/*
			asprintf(&fbuf, "%swpa=2\nrsn_pairwise=CCMP\nwpa_psk=%s\n", wbuf, psk_str);
*/
            sprintf(fbuf, "%swpa=2\nrsn_pairwise=CCMP\nwpa_psk=%s\n", wbuf, psk_str);

        } else if (!strcmp(argv[6], "open")) {
/*
			asprintf(&fbuf, "%s", wbuf);
*/
			sprintf(fbuf, "%s", wbuf);
        }
    } else if (set_num > 6) {
        if (!strcmp(argv[6], "open")) {
/*
			asprintf(&fbuf, "%s", wbuf);
*/
            sprintf(fbuf, "%s", wbuf);
        }
    } else {
/*
		asprintf(&fbuf, "%s", wbuf);
*/
		sprintf(fbuf, "%s", wbuf);

    }

    fd = open(HOSTAPD_CONF_FILE, O_CREAT | O_TRUNC | O_WRONLY | O_NOFOLLOW, 0660);
    if (fd < 0) {
        printf("Cannot update \"%s\": %s", HOSTAPD_CONF_FILE, strerror(errno));
/*        free(wbuf);
        free(fbuf);
 */
		return OPERATION_FAILED;
    }
    if (write(fd, fbuf, strlen(fbuf)) < 0) {
        printf("Cannot write to \"%s\": %s", HOSTAPD_CONF_FILE, strerror(errno));
        ret = OPERATION_FAILED;
    }
/*		  free(wbuf);
			free(fbuf);
*/


    /* Note: apparently open can fail to set permissions correctly at times */
    if (fchmod(fd, 0660) < 0) {
        printf("Error changing permissions of %s to 0660: %s",
                HOSTAPD_CONF_FILE, strerror(errno));
        close(fd);
        unlink(HOSTAPD_CONF_FILE);
		return OPERATION_FAILED;
    }

    if (fchown(fd, AID_SYSTEM, AID_WIFI) < 0) {
        printf("Error changing group ownership of %s to %d: %s",
                HOSTAPD_CONF_FILE, AID_WIFI, strerror(errno));
        close(fd);
        unlink(HOSTAPD_CONF_FILE);
        return OPERATION_FAILED;
    }

    close(fd);
    return ret;
}

/*
 * Arguments:
 *	argv[2] - interface name
 *	argv[3] - AP or P2P or STA
 */
int fw_reload_softap(int set_num, char *argv[])
{
    int i = 0;
    char *fwpath = NULL;

    if (set_num < 1) {
        printf("SoftAP fwreload is missing arguments. Please use: softap <wlan iface> <AP|P2P|STA>");
	printf("fw_reload_softap: argc is %d\n",set_num);
        return COMMAND_SYNTAX_ERROR;
    }

    if (strcmp(argv[0], "AP") == 0) {
        fwpath = (char *)wifi_get_fw_path(WIFI_GET_FW_PATH_AP);
    } else if (strcmp(argv[0], "P2P") == 0) {
        fwpath = (char *)wifi_get_fw_path(WIFI_GET_FW_PATH_P2P);
    } else if (strcmp(argv[0], "STA") == 0) {
        fwpath = (char *)wifi_get_fw_path(WIFI_GET_FW_PATH_STA);
    }
    if (!fwpath)
        return COMMAND_PARAMETER_ERROR;
    if (wifi_change_fw_path((const char *)fwpath)) {
        printf("Softap fwReload failed\n");
        return OPERATION_FAILED;
    }
    else {
        printf("Softap fwReload - Ok\n");
    }
    return SOFTAP_STATUS_RESULT;
}
