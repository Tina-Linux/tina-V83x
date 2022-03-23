#include "headbar_view.h"
#include "resource.h"

#define TIMER_ID 10
#ifdef ALLWINNERTECH_R6
#define SLIDE_DOWN_SPEED 10
#else
#define SLIDE_DOWN_SPEED 4
#endif

static HWND bottomBarHwnd;
static int slideDistance;

static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	HeadBarView *headBarData = NULL;
	headBarData = (HeadBarView*) GetWindowAdditionalData(GetParent(hwnd));
	if (nc == STN_CLICKED && slideDistance >= 100) {
		switch (id) {
		case ID_BUTTON_FAVOR:
			if (headBarData->winDes) {
				printf("%s\n", headBarData->winDes);
			}
			break;
		case ID_BUTTON_HOME:
			printf("ID_BUTTON_HOME_1\n");
			if (strcmp(headBarData->winDes, WIN_DES_SENSOR)==0 || strcmp(headBarData->winDes, WIN_DES_VIDEO)==0
			  || strcmp(headBarData->winDes, WIN_DES_WIFI)==0)
			{
				printf("ID_BUTTON_HOME_2\n");
				headBarData->winDes = WIN_DES_MAIN;
				PostMessage(GetParent(GetParent(hwnd)), MSG_CLOSE, 0, 0);
			}
			else if (headBarData->winDes) {
				BroadcastMessage(MSG_HOME_CLICK, 0,
						(LPARAM) headBarData->winDes);
				if (strcmp(headBarData->winDes, WIN_DES_MAIN) != 0) {
					headBarData->winDes = WIN_DES_MAIN;
					PostMessage(GetParent(GetParent(hwnd)), MSG_CLOSE, 0, 0);
				}
			}
			break;
		case ID_BUTTON_SETUP:
			if (headBarData->winDes) {
				BroadcastMessage(MSG_HOME_SETUP, 0,
						(LPARAM) headBarData->winDes);
			}
			break;

		case ID_BUTTON_VIDEO:
			if (headBarData->winDes)
			{
				BroadcastMessage(MSG_HOME_VIDEO, 0, (LPARAM) headBarData->winDes);
			}
			break;

		case ID_BUTTON_SENSOR:
			if (headBarData->winDes)
			{
				BroadcastMessage(MSG_HOME_SENSOR, 0, (LPARAM) headBarData->winDes);
			}
			break;
		}
	}
}

static int HeadBarViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

	HeadBarView *headBarData = NULL;
	headBarData = (HeadBarView*) GetWindowAdditionalData(hwnd);

	switch (message) {
	case MSG_CREATE: {

		HWND btn_favor = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_FAVOR,
				20, 10, 109, 60, hwnd, (DWORD) headBarData->favorData);
		SetNotificationCallback(btn_favor, my_notif_proc);

		HWND btn_home = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_HOME,
				320, 10, 109, 60, hwnd, (DWORD) headBarData->homeData);
		SetNotificationCallback(btn_home, my_notif_proc);

		/*
		if (strcmp(headBarData->winDes, WIN_DES_VIDEO) != 0
		&&  strcmp(headBarData->winDes, WIN_DES_SENSOR) != 0)
		{
			HWND btn_setup = CreateWindowEx(BUTTON_VIEW, "",
			WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_SETUP,
					550, 10, 109, 60, hwnd, (DWORD) headBarData->setupData);
			SetNotificationCallback(btn_setup, my_notif_proc);
		}

		if (strcmp(headBarData->winDes, WIN_DES_MAIN) == 0)
		{
			HWND btn_video = CreateWindowEx(BUTTON_VIEW, "",
			WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_VIDEO,
					139, 17, 40, 40, hwnd, (DWORD) headBarData->videoData);
			SetNotificationCallback(btn_video, my_notif_proc);

			HWND btn_sensor = CreateWindowEx(BUTTON_VIEW, "",
			WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_SENSOR,
					209, 15, 48, 48, hwnd, (DWORD) headBarData->sensorData);
			SetNotificationCallback(btn_sensor, my_notif_proc);
		}
		*/

		SetTimer(hwnd, TIMER_ID, 1);
		slideDistance = 0;
		return 0;
	}
	case MSG_TIMER: {
		MoveWindow(hwnd, 0, slideDistance - 100, 800, slideDistance, FALSE);
		slideDistance += SLIDE_DOWN_SPEED;
		if (slideDistance >= 100) {
			MoveWindow(hwnd, 0, 0, 800, 100, FALSE);
			KillTimer(hwnd, TIMER_ID);
			if (bottomBarHwnd != 0)
				SetTimer(bottomBarHwnd, TIMER_ID, 1);
		}
		if (slideDistance == SLIDE_DOWN_SPEED) {
			ShowWindow(hwnd, SW_SHOWNORMAL);
		}
		break;
	}
	case MSG_PAINT: {
		HDC hdc = BeginPaint(hwnd);
		RECT rect;
		GetClientRect(hwnd, &rect);
		FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
				&headBarData->bmpBg);
		FillBoxWithBitmap(hdc, 660, 30, 30, 19, &headBarData->bmpWifi);

		SetBkMode(hdc, BM_TRANSPARENT);
		SetTextColor(hdc, COLOR_lightwhite);
		SelectFont(hdc, GetWindowFont(hwnd));
		TextOut(hdc, 710, 30, "15:30");
		EndPaint(hwnd, hdc);
		return 0;
	}
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

HWND HeadBarViewInit(HWND hParent, HWND myBottomBarHwnd,
		HeadBarView *headBarData) {
	HWND mHwnd;
	bottomBarHwnd = myBottomBarHwnd;
	mHwnd = CreateWindowEx(HEAD_BAR_VIEW, "",
	WS_CHILD, WS_EX_TRANSPARENT, 0, 0, 0, 800, 100, hParent,
			(DWORD) headBarData);
	SetWindowFont(mHwnd, getLogFont(ID_FONT_SIMSUN_20));

	if (mHwnd == HWND_INVALID) {
		return HWND_INVALID;
	}

	return mHwnd;
}

BOOL RegisterHeadBarView() {
	WNDCLASS MyClass;
	MyClass.spClassName = HEAD_BAR_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = HeadBarViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterHeadBarView(void) {
	UnregisterWindowClass(HEAD_BAR_VIEW);
}
