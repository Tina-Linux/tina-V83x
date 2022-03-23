#include "videowindow.h"
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"
#include "headbar_view.h"
#include "include.h"
//#include "middle_ware.h"
#include "washing_res_cn.h"
//#include "video_bottombar.h"

static HeadBarView *headBarData = NULL;
#define __ID_TIMER_SLIDER 100

#define NETWORK_FILE_HEAD   "http://"
#define ASCIILINESZ     (1024)
#define MAX_FILE_NUM    (15)
int mediaFileNum = 0;
static char mediaFile[MAX_FILE_NUM][ASCIILINESZ] = {
    {"http://bbhlt.shoujiduoduo.com/bb/video/first/20000373.mp4"}
};

#if 0
// wifi connect
#define TAG "wifi"
#include <tina_log.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <wifi_intf.h>
#include <pthread.h>
#include <light_menu.h>
static pthread_t  app_scan_tid;
static int event = WIFIMG_NETWORK_DISCONNECTED;

static void wifi_event_handle(tWIFI_EVENT wifi_event, void *buf, int event_label)
{
    printf("event_label 0x%x\n", event_label);
    switch(wifi_event)
    {
        case WIFIMG_WIFI_ON_SUCCESS:
        {
            printf("WiFi on success!\n");
            event = WIFIMG_WIFI_ON_SUCCESS;
            break;
        }

        case WIFIMG_WIFI_ON_FAILED:
        {
            printf("WiFi on failed!\n");
            event = WIFIMG_WIFI_ON_FAILED;
            break;
        }

        case WIFIMG_WIFI_OFF_FAILED:
        {
            printf("wifi off failed!\n");
            event = WIFIMG_WIFI_OFF_FAILED;
            break;
        }

        case WIFIMG_WIFI_OFF_SUCCESS:
        {
            printf("wifi off success!\n");
            event = WIFIMG_WIFI_OFF_SUCCESS;
            break;
        }

        case WIFIMG_NETWORK_CONNECTED:
        {
            printf("WiFi connected ap!\n");
            event = WIFIMG_NETWORK_CONNECTED;
            break;
        }

        case WIFIMG_NETWORK_DISCONNECTED:
        {
            printf("WiFi disconnected!\n");
            event = WIFIMG_NETWORK_DISCONNECTED;
            break;
        }

        case WIFIMG_PASSWORD_FAILED:
        {
            printf("Password authentication failed!\n");
            event = WIFIMG_PASSWORD_FAILED;
            break;
        }

        case WIFIMG_CONNECT_TIMEOUT:
        {
            printf("Connected timeout!\n");
            event = WIFIMG_CONNECT_TIMEOUT;
            break;
        }

        case WIFIMG_NO_NETWORK_CONNECTING:
        {
            printf("It has no wifi auto connect when wifi on!\n");
            event = WIFIMG_NO_NETWORK_CONNECTING;
            break;
        }

        case WIFIMG_CMD_OR_PARAMS_ERROR:
        {
            printf("cmd or params error!\n");
            event = WIFIMG_CMD_OR_PARAMS_ERROR;
            break;
        }

        case WIFIMG_KEY_MGMT_NOT_SUPPORT:
        {
            printf("key mgmt is not supported!\n");
            event = WIFIMG_KEY_MGMT_NOT_SUPPORT;
            break;
        }

        case WIFIMG_OPT_NO_USE_EVENT:
        {
            printf("operation no use!\n");
            event = WIFIMG_OPT_NO_USE_EVENT;
            break;
        }

        case WIFIMG_NETWORK_NOT_EXIST:
        {
            printf("network not exist!\n");
            event = WIFIMG_NETWORK_NOT_EXIST;
            break;
        }

        case WIFIMG_DEV_BUSING_EVENT:
        {
            printf("wifi device busing!\n");
            event = WIFIMG_DEV_BUSING_EVENT;
            break;
        }

        default:
        {
            printf("Other event, no care!\n");
        }
    }
}

int OpenWifi(char *ip_adr, char *ip_psw)
{
    int ret = 0, len = 0;
    int times = 0, event_label = 0;
    char ssid[256] = {0}, scan_results[4096] = {0};
    int  wifi_state = WIFIMG_WIFI_DISABLED;
    const aw_wifi_interface_t *p_wifi_interface = NULL;

    printf("\n*********************************\n");
    printf("***Start wifi connect ap test!***\n");
    printf("*********************************\n");

    event_label = rand();
    p_wifi_interface = aw_wifi_on(wifi_event_handle, event_label);
    if(p_wifi_interface == NULL){
        printf("wifi on failed event 0x%x\n", event);
        return -1;
    }

    while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
        printf("wifi state busing,waiting\n");
        usleep(2000000);
    }

    //pthread_create(&app_scan_tid, NULL, &app_scan_task,(void *)p_wifi_interface);

    event_label++;
    p_wifi_interface->connect_ap(ip_adr, ip_psw, event_label);

    while(aw_wifi_get_wifi_state() == WIFIMG_WIFI_BUSING){
        printf("wifi state busing,waiting\n");
        usleep(2000000);
    }

    printf("******************************\n");
    printf("Wifi connect ap test: Success!\n");
    printf("******************************\n");

    return 0;
}
#endif

/*
void OpenWifi(void)
{
	int len1, len2;
	char str_adr[64];
	char str_psw[64];
	int fb;
	int ret;
	char str[256];

	len1 = GetIpAdr(str_adr);
	if(len1 != -1)
	{
		str_adr[len1] = '\0';
	}
	app_debug("len1=%d, str_adr=%s\n", len1, str_adr);

	len2 = GetIpPsw(str_psw);
	if(len2 != -1)
	{
		str_psw[len2] = '\0';
	}
	app_debug("len2=%d, str_psw=%s\n", len2, str_psw);
	if(len1!=-1 && len2!=-1)
	{
		sprintf(str, "ctrl_interface=/var/log/wpa_supplicant\nupdate_config=1\nnetwork={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n}", str_adr, str_psw);
		fb = open("/etc/wpa_supplicant.conf", O_RDWR | O_TRUNC);
		ret = write(fb, str, strlen(str));
		close(fb);

		system("killall wpa_supplicant");
		system("wpa_supplicant  -Dnl80211 -iwlan0 -c /etc/wpa_supplicant.conf -B");
		system("udhcpc -iwlan0");
		usleep(100000);
	}
	else
	{
		printf("defualt wifi\n");
	}
}
*/

#if 0
int getMediaFile(void)
{
    FILE * in ;
    int last = 0;
    int len = 0;
    int lineno = 0;
    char line [ASCIILINESZ+1] = {0};
    char ininame[128] = {"/etc/mediafile.txt"};


    printf("open :%s\n", ininame);
    in=fopen(ininame, "r");
    if (in==NULL) {
        fprintf(stderr, "iniparser: cannot open%s\n", ininame);
        return -1 ;
    }
    last=0 ;
    mediaFileNum = 0;

    while (fgets(line+last, ASCIILINESZ-last, in)!=NULL) {
        printf("lineno:%d, line:%s\n", lineno, line);
        lineno++ ;
        len = (int)strlen(line)-1;
        if (len==0)
            continue;
        /* Safety check against buffer overflows */
        if (line[len]!='\n' && !feof(in)) {
            fprintf(stderr,
                    "iniparser: input line too long in %s (%d)\n",
                    ininame,
                    lineno);
            fclose(in);
            return -1 ;
        }
        /* Get rid of \n and spaces at end of line */
        while ((len>=0) &&
                ((line[len]=='\n') || (isspace(line[len])))) {
            line[len]=0 ;
            len-- ;
        }
        if (len < 0) { /* Line was entirely \n and/or spaces */
            len = 0;
        }
        /* Detect multi-line */
        if (line[len]=='\\') {
            /* Multi-line value */
            last=len ;
            continue ;
        } else {
            last=0 ;
        }
        mediaFileNum++;
        printf("mediaFileNum:%d len:%d %s\n", mediaFileNum, len, line);
        if (mediaFileNum <= MAX_FILE_NUM)
        {
            strncpy(mediaFile[mediaFileNum-1], line, ASCIILINESZ);
            printf("mediaFile[%d]: %s\n", mediaFileNum-1, mediaFile[mediaFileNum-1]);
        }
    }
    fclose(in);
    return 0;
}
#endif

/*
static ButtonView* ButtonDataInit(ButtonView *buttonData, int num) {
	int err_code;
	buttonData = (ButtonView*) malloc(sizeof(ButtonView));
	if (NULL == buttonData) {
		sm_error("malloc ButtonView data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonView));

	switch (num) {
	case ID_BUTTON_PRE:
		setCurrentIconValue(ID_FUN_WIN_BTN2, 1);
		getResBmp(ID_FUN_WIN_BTN2, BMPTYPE_BASE, &buttonData->bmpNormal);
		setCurrentIconValue(ID_FUN_WIN_BTN2, 1);
		getResBmp(ID_FUN_WIN_BTN2, BMPTYPE_BASE, &buttonData->bmpSelect);
		buttonData->selectStatus = SELECT_STATUS_NO;
		buttonData->switchMode = SWITCH_MODE_STATIC;
		break;

	case ID_BUTTON_NEXT:
		setCurrentIconValue(ID_FUN_WIN_BTN2, 0);
		getResBmp(ID_FUN_WIN_BTN2, BMPTYPE_BASE, &buttonData->bmpNormal);
		setCurrentIconValue(ID_FUN_WIN_BTN2, 0);
		getResBmp(ID_FUN_WIN_BTN2, BMPTYPE_BASE, &buttonData->bmpSelect);
		buttonData->selectStatus = SELECT_STATUS_NO;
		buttonData->switchMode = SWITCH_MODE_STATIC;
		break;

	case ID_BUTTON_START:
		err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->bmpNormal, "/usr/res/other/begin.png");
		if (err_code != ERR_BMP_OK) {
			app_error("load bitmap failed\n");
		}

		err_code = LoadBitmapFromFile(HDC_SCREEN, &buttonData->bmpSelect, "/usr/res/other/pause.png");
		if (err_code != ERR_BMP_OK) {
			app_error("load bitmap failed\n");
		}

		buttonData->selectStatus = SELECT_STATUS_NO;
		buttonData->switchMode = SWITCH_MODE_STATIC;
		break;
	}
	return buttonData;
}
*/

/*
static VideoBarView* VideoBottomBarDataInit(VideoBarView *bottomBarData) {
	bottomBarData = (VideoBarView*) malloc(sizeof(VideoBarView));
	if (NULL == bottomBarData) {
		app_error("malloc BottomBarView data error\n");
	}
	memset((void *) bottomBarData, 0, sizeof(VideoBarView));

	bottomBarData->preButton = ButtonDataInit(bottomBarData->preButton, ID_BUTTON_PRE);
	bottomBarData->nextButton = ButtonDataInit(bottomBarData->nextButton, ID_BUTTON_NEXT);
	bottomBarData->beginButton = ButtonDataInit(bottomBarData->beginButton, ID_BUTTON_START);
	return bottomBarData;
}
*/

static int ActivityVideoProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
    int ret = 0;
	static BITMAP bmpBg;
	static char strDes[64];
    char *curPlayFile = NULL;
	//static VideoBarView *bottomBarData = NULL;
    static int curPlayIndex = 0;

	switch (message) {
	case MSG_TIMER: {
        //system("dd if=/dev/zero of=/dev/fb0");
		KillTimer(hWnd, __ID_TIMER_SLIDER);
		//HWND bottomBar = VideoBottomBarViewInit(hWnd, bottomBarData);
		HeadBarViewInit(hWnd, NULL, headBarData);
		memcpy(strDes, headBarData->winDes, 64);
		app_debug("winDes=%s\n", headBarData->winDes);

		/*
        curPlayFile = mediaFile[curPlayIndex];
        printf("curPlayFile(%d):%s\n", curPlayIndex, curPlayFile);
        if (curPlayIndex < (mediaFileNum-1))
        {
           curPlayIndex++;
        }
        else
        {
            curPlayIndex = 0;
        }
        if (0 == strncmp(curPlayFile, NETWORK_FILE_HEAD, strlen(NETWORK_FILE_HEAD)))
        {
		OpenWifi();
            ret = system("ping -c 1 www.baidu.com");
        }
        printf("ping ret :%d\n", ret);
        if (0 == ret)
        {
            tplayer_play_url(curPlayFile);
            tplayer_play();
            tplayer_setlooping(TRUE);
        }
        */
		break;
	}
	case MSG_CREATE: {
		//bottomBarData = VideoBottomBarDataInit(bottomBarData);
		SetTimer(hWnd, __ID_TIMER_SLIDER, 1);
        //getMediaFile();
        //tplayer_init(TPLAYER_VIDEO_ROTATE_DEGREE_0);
		break;
	}
	case MSG_PAINT: {
		break;
	}
	case MSG_ERASEBKGND: {
		break;
	}
	case MSG_DESTROY: {
		DestroyAllControls(hWnd);
		break;
	}

	case MSG_CLOSE:
        //tplayer_stop();
        //tplayer_exit();

		BroadcastMessage(MSG_HOME_CLICK, 0, (LPARAM)strDes);
		DestroyMainWindow(hWnd);
		break;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int ActivityVideo(HWND hosting, HeadBarView *myHeadBarData)
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	//OpenWifi(str_adr, str_psw);
	//OpenWifi("Aw-Guest-SZ", "Aw.guest6688");
	//OpenWifi();

	RegisterVideoBottomBar();

	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ActivityVideoProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	//CreateInfo.iBkColor = PIXEL_lightgray;
	CreateInfo.iBkColor = PIXEL_black;

	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hosting;

	headBarData = myHeadBarData;

	hMainWnd = CreateMainWindow(&CreateInfo);

	if (hMainWnd == HWND_INVALID)
		return -1;

	ShowWindow(hMainWnd, SW_SHOWNORMAL);

	while (GetMessage(&Msg, hMainWnd)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	UnregisterVideoBottomBar();

	MainWindowThreadCleanup(hMainWnd);
	return 0;
}
