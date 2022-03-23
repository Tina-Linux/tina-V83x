#include "videowindow.h"
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"
#include "headbar_view.h"
#include "include.h"
#include "middle_ware.h"
#include "washing_res_cn.h"
#include "video_bottombar.h"
#include "light_menu.h"

static HeadBarView *headBarData = NULL;
extern int bool_connect;
#define __ID_TIMER_SLIDER 100

#define NETWORK_FILE_HEAD   "http://"
#define ASCIILINESZ     (1024)
#define MAX_FILE_NUM    (15)
int curPlayIndex = 0;
int paintflag = 0;
void InitRes(void)
{
	LoadBitmap(HDC_SCREEN, &native_video, "/usr/res/menu/native_video.png");
	LoadBitmap(HDC_SCREEN, &url_video, "/usr/res/menu/url_video.png");
}
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
void *play_video(HWND hwnd)
{
	SetTimer(hwnd, __ID_TIMER_SLIDER, 1);
	tplayer_init(TPLAYER_VIDEO_ROTATE_DEGREE_0);
}
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

static void video_notif_proc(HWND hwnd, int id, int nc, DWORD add_data)
{
	HWND parent = GetParent(hwnd);
	if (nc == STN_CLICKED){
		if(id == ID_NATIVE){
			VIDEO_FLAG = NATIVE_VIDEO;
			VideoWnd(parent, VIDEO_FLAG, headBarData);
			//MediafilePopWin(hwnd, VIDEO_FLAG);
		}else if(id == ID_URL){
			VIDEO_FLAG = URL_VIDEO;
			VideoWnd(parent, VIDEO_FLAG, headBarData);
		}
	}
}
#define __ID_TIMER_SLIDER 100
static int ActivityVideoProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	HWND button;
	static char strDes[64];
	static PLOGFONT Font_45;
	Font_45 = getLogFont(ID_FONT_SIMSUN_45);
	HDC hdc;
	switch (message){
	case MSG_CREATE:{
		button = CreateWindowEx(CTRL_STATIC, "",
                WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTER | SS_NOTIFY, WS_EX_TRANSPARENT,
                ID_NATIVE, 100, 150, 200, 200,
                                 hWnd, (DWORD)&native_video);
		SetNotificationCallback(button, video_notif_proc);
		button = CreateWindowEx(CTRL_STATIC, "",
                WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_CENTER | SS_NOTIFY, WS_EX_TRANSPARENT,
                ID_URL, 500, 150, 200, 200,
                                 hWnd, (DWORD)&url_video);
		SetNotificationCallback(button, video_notif_proc);
		SetTimer(hWnd, __ID_TIMER_SLIDER, 1);
		break;
	}
	case MSG_PAINT:{
		hdc = BeginPaint(hWnd);
		SelectFont(hdc, Font_45);
		SetBkMode(hdc, BM_TRANSPARENT);
		SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 0, 0));
		TextOut(hdc, 100, 380, NET_NATIVE);
		TextOut(hdc, 500, 380, NET_URL);
		EndPaint(hWnd, hdc);
		break;
	}
	case MSG_TIMER:{
		KillTimer(hWnd, __ID_TIMER_SLIDER);
		headBarData->winDes = WIN_DES_VIDEO;
		memcpy(strDes, headBarData->winDes, 64);
		headbarbtnwifi[18] = HeadBarViewInit(hWnd, NULL, headBarData);
		SendMessage(headbarbtnwifi[18], MSG_UPDATETIME, NULL, NULL);
		break;
	}
	case MSG_DESTROY:{
		unloadBitMap(&native_video);
                unloadBitMap(&url_video);
                DestroyAllControls(hWnd);
                return 0;
        }
        case MSG_CLOSE: {
		BroadcastMessage(MSG_HOME_CLICK, 0, (LPARAM)strDes);
                DestroyMainWindow(hWnd);
                PostQuitMessage(0);
                return 0;
        }
	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int ActivityVideo(HWND hosting, HeadBarView *myHeadBarData)
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;
	InitRes();
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

	MainWindowThreadCleanup(hMainWnd);
	return 0;
}
