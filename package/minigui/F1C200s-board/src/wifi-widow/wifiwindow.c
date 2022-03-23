#include "wifiwindow.h"
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>

#include <mgi/mgi.h>

#include <minigui/control.h>
#include "resource.h"
#include "headbar_view.h"
#include "include.h"
#include "washing_res_cn.h"

static HeadBarView *headBarData = NULL;
#define __ID_TIMER_SLIDER 100

#define IP_CFG_SIZE 64
char wifi_ip_adr[IP_CFG_SIZE] = {'A', 'W', '-', 'G', 'u', 'e', 's', 't'};
char wifi_ip_psw[IP_CFG_SIZE] = {};

void ConnectNewWifi(void)
{
	int ret;
	int fb;
	char str[256];

	printf("Connect New Wifi...\n");
	sprintf(str, "ctrl_interface=/var/log/wpa_supplicant\nupdate_config=1\nnetwork={\n\tssid=\"%s\"\n\tpsk=\"%s\"\n}", wifi_ip_adr, wifi_ip_psw);
	fb = open("/etc/wpa_supplicant.conf", O_RDWR | O_TRUNC);
	ret = write(fb, str, strlen(str));
	close(fb);

	system("killall wpa_supplicant");
	system("wpa_supplicant  -Dnl80211 -iwlan0 -c /etc/wpa_supplicant.conf -B");
	system("udhcpc -iwlan0 &");
	//usleep(100000);
}

static int ActivityWifiProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	static BITMAP bmpBg;
	static char strDes[64];

	HDC hdc;
	HWND wifi_adr;
	HWND wifi_psw;
	char str[IP_CFG_SIZE];
	static PLOGFONT Font_20, Font_45, Font_30;

	switch (message) {
	case MSG_TIMER: {
		KillTimer(hWnd, __ID_TIMER_SLIDER);
		HeadBarViewInit(hWnd, NULL, headBarData);
		memcpy(strDes, headBarData->winDes, 64);
		app_debug("winDes=%s\n", headBarData->winDes);
		break;
	}
	case MSG_CREATE: {
		SetTimer(hWnd, __ID_TIMER_SLIDER, 1);

		Font_20 = getLogFont(ID_FONT_SIMSUN_20);
		Font_30 = getLogFont(ID_FONT_SIMSUN_30);
		Font_45 = getLogFont(ID_FONT_SIMSUN_45);

		wifi_adr = CreateWindowEx(CTRL_SLEDIT, "",
				WS_VISIBLE | WS_TABSTOP | WS_BORDER,
				WS_EX_TRANSPARENT, 1, 50+50, 130, 200, 50, hWnd, NULL);
		SetFocusChild(wifi_adr);

		wifi_psw = CreateWindowEx(CTRL_SLEDIT, "",
				WS_VISIBLE | WS_TABSTOP | WS_BORDER,
				WS_EX_TRANSPARENT, 2, 350+50, 130, 200, 50, hWnd, NULL);

		SetWindowText (GetDlgItem(hWnd, 1), wifi_ip_adr);
		SetWindowText (GetDlgItem(hWnd, 2), wifi_ip_psw);

		if(1)
		{
			HWND soft_key;
			soft_key = 0;
			printf("sk_ime_hwnd=%d\n", soft_key);
	        soft_key = mgiCreateSoftKeypad(NULL);
	        printf("sk_ime_hwnd=%d\n", soft_key);

	        //SetIMEStatus(IME_STATUS_AUTOTRACK, TRUE);
	        SetIMEStatus(IME_STATUS_ENCODING,IME_ENCODING_LOCAL);
	        SetIMEStatus(IME_STATUS_ENABLED, TRUE);
		}
		break;
	}
	case MSG_PAINT: {
		hdc = BeginPaint(hWnd);
		int slideX = LOWORD(lParam);
		SetBkMode(hdc, BM_TRANSPARENT);
		SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 45, 83, 126));

		SelectFont(hdc, Font_30);
		TextOut(hdc, 20+50, 150, "1:");
		SelectFont(hdc, Font_30);
		TextOut(hdc, 320+50, 150, "2:");

		EndPaint(hWnd, hdc);

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
		GetWindowText(GetDlgItem(hWnd, 1), str, IP_CFG_SIZE);
		memcpy(wifi_ip_adr, str, IP_CFG_SIZE);
		printf("len=%d, ip_adr=%s\n", strlen(wifi_ip_adr), wifi_ip_adr);

		GetWindowText(GetDlgItem(hWnd, 2), str, IP_CFG_SIZE);
		memcpy(wifi_ip_psw, str, IP_CFG_SIZE);
		printf("len=%d, ip_psw=%s\n", strlen(wifi_ip_psw), wifi_ip_psw);

		ConnectNewWifi();

		BroadcastMessage(MSG_HOME_CLICK, 0, (LPARAM)strDes);
		DestroyMainWindow(hWnd);
		break;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int ActivityWifi(HWND hosting, HeadBarView *myHeadBarData)
{
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ActivityWifiProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
	CreateInfo.iBkColor = PIXEL_lightgray;
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
