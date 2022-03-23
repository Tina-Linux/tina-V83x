#include <minigui/common.h>
#include <minigui/minigui.h>
#include <minigui/gdi.h>
#include <minigui/window.h>
#include <minigui/control.h>
#include "resource.h"
#include "headbar_view.h"
#include "bottombar_view.h"
#include "rotate_add_view.h"
#include "washing_res_cn.h"
#include "headbar_view.h"
#include "WifiWindow.h"
#include "light_menu.h"

static HeadBarView *headBarData = NULL;
HWND getActivity();
static ButtonView* ButtonDataInit(ButtonView *buttonData) {
	buttonData = (ButtonView*) malloc(sizeof(ButtonView));
	if (NULL == buttonData) {
		sm_error("malloc ButtonView data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonView));
	setCurrentIconValue(ID_FUN_WIN_BTN3, 0);
	getResBmp(ID_FUN_WIN_BTN3, BMPTYPE_BASE, &buttonData->bmpNormal);
	setCurrentIconValue(ID_FUN_WIN_BTN3, 1);
	getResBmp(ID_FUN_WIN_BTN3, BMPTYPE_BASE, &buttonData->bmpSelect);
	buttonData->textSelect = JIESHU;
	buttonData->selectStatus = SELECT_STATUS_NO;
	buttonData->switchMode = SWITCH_MODE_STATIC;
	buttonData->textX = 30;
	buttonData->textY = 25;
	return buttonData;
}

static BottomBarView* BottomBarDataInit(BottomBarView *bottomBarData) {
	bottomBarData = (BottomBarView*) malloc(sizeof(BottomBarView));
	if (NULL == bottomBarData) {
		sm_error("malloc BottomBarView data error\n");
	}
	memset((void *) bottomBarData, 0, sizeof(BottomBarView));

	bottomBarData->beginButton = ButtonDataInit(bottomBarData->beginButton);
	return bottomBarData;
}

static RotateAddView* RotateAddDataInit(RotateAddView *rotateAddData) {
	rotateAddData = (RotateAddView*) malloc(sizeof(RotateAddView));
	if (NULL == rotateAddData) {
		sm_error("malloc RotateAddView data error\n");
	}
	memset((void *) rotateAddData, 0, sizeof(RotateAddView));

	setCurrentIconValue(ID_FUN_WIN_BTN4, 0);
	getResBmp(ID_FUN_WIN_BTN4, BMPTYPE_BASE, &rotateAddData->bmpBgMin);
	setCurrentIconValue(ID_FUN_WIN_BTN4, 1);
	getResBmp(ID_FUN_WIN_BTN4, BMPTYPE_BASE, &rotateAddData->bmpBgBig);
	setCurrentIconValue(ID_FUN_WIN_ANIM, 0);
	getResBmp(ID_FUN_WIN_ANIM, BMPTYPE_BASE, &rotateAddData->bmpAnim[0]);
	setCurrentIconValue(ID_FUN_WIN_ANIM, 1);
	getResBmp(ID_FUN_WIN_ANIM, BMPTYPE_BASE, &rotateAddData->bmpAnim[1]);
	return rotateAddData;
}

static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		features_menu(hwnd, 0);
	}
}

#define __ID_TIMER_SLIDER 100
static int ActivityZNXYZJProc(HWND hWnd, int message, WPARAM wParam,
		LPARAM lParam) {

	static BottomBarView *bottomBarData = NULL;
	static RotateAddView *rotateAddData = NULL;
	static PLOGFONT logfontStart;
	static PLOGFONT logfontDes;
	static BITMAP bmpBg;

	switch (message) {
	case MSG_TIMER: {
		KillTimer(hWnd, __ID_TIMER_SLIDER);
		wifi_icon_change = TRUE;
		HWND bottomBar = BottomBarViewInit(hWnd, bottomBarData);
		headbarbtnwifi[0] = HeadBarViewInit(hWnd, bottomBar, headBarData);
		SendMessage(headbarbtnwifi[0], MSG_UPDATETIME, NULL, NULL);
		SendMessage(headbarbtnwifi[0], MSG_CHAGE_HEADBAR, NULL, NULL);
		HWND rotateHwnd = CreateWindowEx(ROTATE_ADD_VIEW, GONNENG,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 545, 150, 248,
				122, hWnd, (DWORD) rotateAddData);
		SetWindowFont(rotateHwnd, logfontStart);
		SetNotificationCallback(rotateHwnd, my_notif_proc);
		break;
	}
	case MSG_CREATE: {
		setCurrentIconValue(ID_FUN_SCREEN_BG1, 0);
		getResBmp(ID_FUN_SCREEN_BG1, BMPTYPE_BASE, &bmpBg);

		bottomBarData = BottomBarDataInit(bottomBarData);
		rotateAddData = RotateAddDataInit(rotateAddData);

		logfontStart = getLogFont(ID_FONT_SIMSUN_50);
		logfontDes = getLogFont(ID_FONT_SIMSUN_25);

		SetTimer(hWnd, __ID_TIMER_SLIDER, 1);
		return 0;
	}
	case MSG_PAINT: {
		HDC hdc = BeginPaint(hWnd);
		SetBkMode(hdc, BM_TRANSPARENT);
		SetTextColor(hdc, COLOR_lightwhite);
		SelectFont(hdc, logfontStart);
		TextOut(hdc, 10, 150, ZHINENG_P);

		SelectFont(hdc, logfontDes);
		RECT rcClient;
		rcClient.left = 10;
		rcClient.top = 240;
		rcClient.right = 400;
		rcClient.bottom = 400;
		DrawText(hdc, ZHINENG_N, -1, &rcClient,
				DT_NOCLIP | DT_LEFT | DT_WORDBREAK);

		EndPaint(hWnd, hdc);
		return 0;
	}
	case MSG_ERASEBKGND: {
		HDC hdc = (HDC) wParam;
		const RECT* clip = (const RECT*) lParam;
		BOOL fGetDC = FALSE;
		RECT rcTemp;
		if (hdc == 0) {
			hdc = GetClientDC(hWnd);
			fGetDC = TRUE;
		}
		if (clip) {
			rcTemp = *clip;
			ScreenToClient(hWnd, &rcTemp.left, &rcTemp.top);
			ScreenToClient(hWnd, &rcTemp.right, &rcTemp.bottom);
			IncludeClipRect(hdc, &rcTemp);
		}
		FillBoxWithBitmap(hdc, 0, 0, 0, 0, &bmpBg);
		if (fGetDC)
			ReleaseDC(hdc);
		return 0;
	}
	case MSG_HOME_SETUP:
		if (strcmp((char*) lParam, WIN_DES_MAIN) != 0
				&& GetBottomBarDistance() >= 100) {
			//unloadBitMap(&bmpBg);
			WinPopMenuWin(hWnd);
		}
		break;
	case MSG_WIN_POP_CLOSE:
		setCurrentIconValue(ID_FUN_SCREEN_BG1, 0);
		//getResBmp(ID_FUN_SCREEN_BG1, BMPTYPE_BASE, &bmpBg);
		InvalidateRect(hWnd, NULL, TRUE);
		break;
	case MSG_LBUTTONDOWN: {
		if (LOWORD(lParam) < 416 && IsWindowVisible(GetPopMenuHwnd())
				&& IsWindowEnabled(GetPopMenuHwnd())) {
			return 0;
		}
		if (LOWORD(lParam) < 516 && IsWindowVisible(GetfetureMenuHwnd())
				&& IsWindowEnabled(GetfetureMenuHwnd())) {
			return 0;
		}
		break;
	}
	case MSG_LBUTTONUP: {
		if (LOWORD(lParam) < 416 && IsWindowVisible(GetPopMenuHwnd())
				&& IsWindowEnabled(GetPopMenuHwnd())) {
			if(WIFIWIN_OPEN == TRUE){
				WIFIWIN_OPEN = FALSE;
			}else{
				headBarData->winDes = WIN_DES_ZNXYZJ;
				PostMessage(GetPopMenuHwnd(), MSG_CLOSE, 0, 0);
			}
			return 0;
		}
		if (LOWORD(lParam) < 516 && IsWindowVisible(GetfetureMenuHwnd())
				&& IsWindowEnabled(GetfetureMenuHwnd())) {
			PostMessage(GetfetureMenuHwnd(), MSG_CLOSE, 0, 0);
			return 0;
		}
		break;
	}
	case MSG_DESTROY:
		unloadBitMap(&bmpBg);
		unloadBitMap(&bottomBarData->beginButton->bmpNormal);
		unloadBitMap(&bottomBarData->beginButton->bmpSelect);
		unloadBitMap(&rotateAddData->bmpBgMin);
		unloadBitMap(&rotateAddData->bmpBgBig);
		unloadBitMap(&rotateAddData->bmpAnim[0]);
		unloadBitMap(&rotateAddData->bmpAnim[1]);
		if (bottomBarData){
            if (bottomBarData->swipeData) {
                int i, j;
                for (i = 0; i < 2; i++) {
                    if (bottomBarData->swipeData->laundryData[i]) {
                        for (j = 0; j < 3; j++) {
                            if (bottomBarData->swipeData->laundryData[i]->laundryPartData[j])
                                free(
                                        bottomBarData->swipeData->laundryData[i]->laundryPartData[j]);
                        }
                        free(bottomBarData->swipeData->laundryData[i]);
                    }
                }
                free(bottomBarData->swipeData);
            }
		if (bottomBarData->slideTextData1)
			free(bottomBarData->slideTextData1);
		if (bottomBarData->slideTextData2)
			free(bottomBarData->slideTextData2);
		if (bottomBarData->slideTextData3)
			free(bottomBarData->slideTextData3);
		if (bottomBarData->nextButton)
			free(bottomBarData->nextButton);
		if (bottomBarData->beginButton)
			free(bottomBarData->beginButton);
			free(bottomBarData);
		}
		if (rotateAddData)
			free(rotateAddData);
		DestroyAllControls(hWnd);
		return 0;
	case MSG_CLOSE:
		DestroyMainWindow(hWnd);
		break;
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

int ActivityZNXYZJ(HWND hosting, HeadBarView *myHeadBarData) {
	MSG Msg;
	HWND hMainWnd;
	MAINWINCREATE CreateInfo;

	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = ActivityZNXYZJProc;
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

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
