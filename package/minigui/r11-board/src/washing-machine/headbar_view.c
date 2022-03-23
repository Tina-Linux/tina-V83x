#include "headbar_view.h"
#include "resource.h"
#include "light_menu.h"
#include "WifiWindow.h"
#include "wifiactivity.h"
#include "win_pop_menu.h"
#include "VideoWnd.h"
#define TIMER_ID 10
#ifdef ALLWINNERTECH_R6
#define SLIDE_DOWN_SPEED 10
#else
#define SLIDE_DOWN_SPEED 4
#endif

HDC hdc;
mWidget* mWidgetself;
static HWND bottomBarHwnd;
static int slideDistance;
extern int pthread_stop_refreash;
static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	HeadBarView *headBarData = NULL;
	headBarData = (HeadBarView*) GetWindowAdditionalData(GetParent(hwnd));
	if (nc == STN_CLICKED && slideDistance >=100) {
		switch (id) {
		case ID_BUTTON_FAVOR:
			if (headBarData->winDes) {
				printf("button_favor %s\n", headBarData->winDes);
			}
			break;
		case ID_BUTTON_HOME:
			if (strcmp(headBarData->winDes, WIN_DES_SENSOR)==0 || strcmp(headBarData->winDes, WIN_DES_VIDEO)==0)
			{
				loadManyViewBitMap();
				headBarData->winDes = WIN_DES_MAIN;
				PostMessage(GetParent(GetParent(hwnd)), MSG_CLOSE, 0, 0);
			}else if(strcmp(headBarData->winDes, WIN_DES_VIDEO_PLAY) == 0){
				headBarData->winDes = WIN_DES_VIDEO;
				SendMessage(GetVideoWnd(), MSG_CLOSE, NULL, NULL);
			}else if(strcmp(headBarData->winDes, WIN_DES_WIFIWND) == 0){
			/*wifi headbar*/
				SendMessage(GetPopMenuHwnd(), MSG_LIGHT_MENU_CLOSE, NULL, NULL);
				if(status_switchbutton == STATUS_ON){
					WIFIWIN_OPEN = FALSE;
					headBarData->winDes = WIN_DES_MAIN;
					pthread_stop_refreash = STATUS_ON;
					pthread_cond_signal(&cond2);
					ShowWindow(GetWifiWnd(), SW_HIDE);
				}else if(status_switchbutton == STATUS_OFF){
					bool_connect = -1;
					WIFIWIN_OPEN = FALSE;
					status_wifilist = STATUS_OFF;
					STATUS_WIFI_ON_OFF = STATUS_OFF;
					ctnr_wnd_close = STATUS_ON;
					headBarData->winDes = WIN_DES_MAIN;
					PostMessage(GetParent(GetParent(hwnd)), MSG_CLOSE, 0, 0);
					SendMessage(GetWifiWnd(), MSG_CLOSE, 0, 0);
				}
			}
			else if (headBarData->winDes) {
				BroadcastMessage(MSG_HOME_CLICK, 0,
						(LPARAM) headBarData->winDes);
				if (strcmp(headBarData->winDes, WIN_DES_MAIN) != 0) {
					headBarData->winDes = WIN_DES_MAIN;
					SendMessage(GetParent(GetParent(hwnd)), MSG_CLOSE, 0, 0);
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
				unloadManyViewBitMap();
				BroadcastMessage(MSG_HOME_VIDEO, 0, (LPARAM) headBarData->winDes);
			}
			break;

		case ID_BUTTON_SENSOR:
			if (headBarData->winDes)
			{
				unloadManyViewBitMap();
				BroadcastMessage(MSG_HOME_SENSOR, 0, (LPARAM) headBarData->winDes);
			}
			break;
		}
	}
}

void *update_time(void)
{
	pthread_mutex_lock(&mutex3);
	while(1){
	time(&tim);
	ptm = localtime(&tim);
	sprintf(szText, "%02d:%02d:%d", ptm->tm_hour, ptm->tm_min, ptm->tm_sec);
	InvalidateRect(headbarWnd, NULL, TRUE);
	pthread_cond_wait(&cond3, &mutex3);
	}
	pthread_mutex_unlock(&mutex3);
}
void *wifi_change(void)
{
	while(1){
	if(ROTATE_STATUS == STATUS_ON){
                SetTimer(headbarWnd, TIMER_WIFICHANGE, 10);
        }else if(bool_connect == 0 || bool_connect == -1){
                KillTimer(headbarWnd, TIMER_WIFICHANGE);
                ROTATE_STATUS == STATUS_OFF;
        }
        InvalidateRect(headbarWnd, NULL, TRUE);
	pthread_cond_wait(&cond4, &mutex4);
	}
}
static int HeadBarViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

	HeadBarView *headBarData = NULL;
	headBarData = (HeadBarView*) GetWindowAdditionalData(hwnd);
	headbarWnd = hwnd;
	switch (message) {
	case MSG_CREATE: {

		HWND btn_favor = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_FAVOR,
				20, 10, 109, 60, hwnd, (DWORD) headBarData->favorData);
		SetNotificationCallback(btn_favor, my_notif_proc);

		btn_home = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_HOME,
				320, 10, 109, 60, hwnd, (DWORD) headBarData->homeData);
		SetNotificationCallback(btn_home, my_notif_proc);
		if (strcmp(headBarData->winDes, WIN_DES_VIDEO) != 0
		&&  strcmp(headBarData->winDes, WIN_DES_SENSOR) != 0
		&& strcmp(headBarData->winDes, WIN_DES_WIFIWND) != 0
		&& strcmp(headBarData->winDes, WIN_DES_VIDEO_PLAY) != 0)
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
		if(strcmp(headBarData->winDes, WIN_DES_WIFIWND) == 0){
			switch_button = CreateWindowEx(NCSCTRL_SWBUTTON, "ON\nOFF",
			WS_VISIBLE, WS_EX_NONE, ID_SWBUTTON, 550, 10, 90, 50, hwnd, NULL);
			SetNotificationCallback(switch_button, switchbutton_notif_proc);
#if 0
			if(first_wifion == TRUE){
				SendMessage(GetWifiWnd(), MSG_WIFI_READON, NULL, NULL);
				first_wifion = FALSE;
			}
#endif
		}
		if(strcmp(headBarData->winDes, WIN_DES_VIDEO_PLAY) == 0){
			btn_mediafile = CreateWindowEx(CTRL_STATIC, "",
                        WS_CHILD | WS_VISIBLE | SS_NOTIFY | SS_BITMAP, WS_EX_TRANSPARENT, ID_BUTTON_VIDEOMEDIA,
                                        600, 17, 40, 40, hwnd, (DWORD)&headBarData->mediafile );
                        SetNotificationCallback(btn_mediafile, media_notify_proc);
		}
		SetTimer(hwnd, TIMER_ID, 1);
		slideDistance = 0;
		return 0;
	}
	case MSG_TIMER: {
		if(wParam == TIMER_ID){
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
			return 0;
		}
		else if(wParam == TIMER_UPDATETIME){
			pthread_cond_signal(&cond3);
		}
		else if(wParam == TIMER_WIFICHANGE){
			angle += SPINNING_SPEED;
									if (angle >= 46080) {
											angle = 0;
									}
			InvalidateRect(hwnd, NULL, TRUE);
			return 0;
		}
		break;

	}
	case MSG_UPDATETIME:{
		UPDATETIME = STATUS_ON;
		SetTimer(hwnd, TIMER_UPDATETIME, 100);
		return  0;
	}
	case MSG_CHAGE_HEADBAR:{
		if(ROTATE_STATUS == STATUS_ON){
			SetTimer(hwnd, TIMER_WIFICHANGE, 10);
                }else if(bool_connect == 0 || bool_connect == -1){
                        KillTimer(hwnd, TIMER_WIFICHANGE);
                        ROTATE_STATUS == STATUS_OFF;
                }
                InvalidateRect(hwnd, NULL, TRUE);
                return 0;
        }
	case MSG_PAINT: {
		hdc = BeginPaint(hwnd);
		RECT rect;
		GetClientRect(hwnd, &rect);
		if(ROTATE_STATUS == STATUS_ON){
			RotateBitmap(hdc, &headBarData->time_wait, 660, 15, angle);
		}else{
		if(bool_connect == 0){
			wifi_icon_change = FALSE;
			FillBoxWithBitmap(hdc, 665, 20, 32, 32, &headBarData->bmpWifi);
                }else if(bool_connect != 0){
			wifi_icon_change = FALSE;
			FillBoxWithBitmap(hdc, 665, 20, 32, 32, &headBarData->no_bmpWifi);
                }
			wifi_icon_change = FALSE;
		}
		FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
                                &headBarData->bmpBg);
		SetBkMode(hdc, BM_TRANSPARENT);
	        SetTextColor(hdc, COLOR_lightwhite);
	        SelectFont(hdc, GetWindowFont(hwnd));
		TextOut(hdc, 708, 27, szText);
		EndPaint(hwnd, hdc);
		return 0;
	}
	case MSG_CLOSE:{
		DestroyMainWindow(hwnd);
                PostQuitMessage(0);
	}
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

static HWND mHwnd = 0;
HWND getHeadBarHwnd()
{
	return mHwnd;
}

HWND HeadBarViewInit(HWND hParent, HWND myBottomBarHwnd,
		HeadBarView *headBarData) {
	int i = 0;
	UPDATETIME = STATUS_OFF;
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
