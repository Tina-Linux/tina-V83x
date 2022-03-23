#include "sensorwindow.h"
#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"
#include "headbar_view.h"
#include "include.h"
#include "recorder_int.h"

static HeadBarView *headBarData = NULL;
#define __ID_TIMER_SLIDER 100

s32 HomeDvInit(void)
{
	s32 ret = 0;
	rec_media_info_t info;
	printf("HomeDvInit.\n");
	memset(&info, 0, sizeof(info));
	info.show_rect.x = 0;
	info.show_rect.y = 0;
	info.show_rect.width  = 800;
	info.show_rect.height = 480;

	info.mute_en[0] = 0;
	info.source_size[0].width = 1920;
	info.source_size[0].height = 1080;
	info.pre_mode[0] = PREVIEW_HOST;
	info.cycle_rec_time[0]   = CYCLE_REC_TIME_1_MIM;
	info.cam_quality_mode[0] = CAMERA_QUALITY_800;
	info.rec_quality_mode[0] = RECORD_QUALITY_1920_1080;
	info.source_frate[0] = 30;
	recorder_init_info(&info);
	recorder_init(0);

	recorder_start_preview(0);

	recorder_preview_en(0,0);
	recorder_set_preview_mode(0,PREVIEW_HOST);
	recorder_preview_en(0,1);

	return 0;
}

s32 HomeDvExit(void)
{
	printf("HomeDvExit\n");
	recorder_exit(0);
	recorder_exit_info();

	return 0;
}

static int ActivitySensorProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam)
{
	static BITMAP bmpBg;
	static char strDes[64];

	switch (message) {
	case MSG_TIMER: {
        system("dd if=/dev/zero of=/dev/fb0");
		KillTimer(hWnd, __ID_TIMER_SLIDER);
		wifi_icon_change = TRUE;
		headbarbtnwifi[19] = HeadBarViewInit(hWnd, NULL, headBarData);
		SendMessage(headbarbtnwifi[19], MSG_UPDATETIME, NULL, NULL);
		memcpy(strDes, headBarData->winDes, 64);
		app_debug("winDes=%s\n", headBarData->winDes);
		HomeDvInit();
		break;
	}
	case MSG_CREATE: {
		SetTimer(hWnd, __ID_TIMER_SLIDER, 1);
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
		HomeDvExit();
		break;
	}

	case MSG_CLOSE:
		BroadcastMessage(MSG_HOME_CLICK, 0, (LPARAM)strDes);
		DestroyMainWindow(hWnd);
		break;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int ActivitySensor(HWND hosting, HeadBarView *myHeadBarData)
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
	CreateInfo.MainWindowProc = ActivitySensorProc;
	CreateInfo.lx = 0;
	CreateInfo.ty = 0;
	CreateInfo.rx = g_rcScr.right;
	CreateInfo.by = g_rcScr.bottom;
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
