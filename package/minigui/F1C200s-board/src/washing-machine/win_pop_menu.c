#include "win_pop_menu.h"
#include "button_view.h"
#include "washing_res_cn.h"
#include "resource.h"
#include "SoftKeyBoardDialog.h"

#define TIMER_ID 10
#define SLIDE_OUT_SPEED 16
static ButtonView *buttonData[12];
static BOOL isFullOpen = FALSE;

static void loadMenuViewBitMap(void) {
	int i;
	for (i = 0; i < 12; i++) {
		buttonData[i]->switchMode = SWITCH_MODE_NORMAL;
		setCurrentIconValue(ID_POP_WIN_BTN1 + i, 0);
		getResBmp(ID_POP_WIN_BTN1 + i, BMPTYPE_BASE, &buttonData[i]->bmpNormal);
		setCurrentIconValue(ID_POP_WIN_BTN1 + i, 1);
		getResBmp(ID_POP_WIN_BTN1 + i, BMPTYPE_BASE, &buttonData[i]->bmpSelect);
	}
}

static void unloadMenuViewBitMap(void) {
	int i;
	for (i = 0; i < 12; i++) {
		buttonData[i]->switchMode = SWITCH_MODE_FORBID;
		unloadBitMap(&buttonData[i]->bmpNormal);
		unloadBitMap(&buttonData[i]->bmpSelect);
	}
}

static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED && isFullOpen) {
		printf("buttun id = %d\n", id);

		/*
		if(id == 2)
		{
			unloadMenuViewBitMap();
			HWND parent = GetParent(hwnd);

			printf("###########_1\n");
			showSoftKeyBoard(hwnd);
		}
		else
		*/
		{
			unloadMenuViewBitMap();
			HWND parent = GetParent(hwnd);
			light_menu(parent, id);
		}
	}
}

static ButtonView* MenuButtonDataInit(ButtonView *buttonData, int num) {
	buttonData = (ButtonView*) malloc(sizeof(ButtonView));
	if (NULL == buttonData) {
		printf("malloc ButtonView data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonView));

	buttonData->selectStatus = SELECT_STATUS_NO;
	buttonData->switchMode = SWITCH_MODE_NORMAL;
	buttonData->textX = 35;
	buttonData->textY = 70;
	setCurrentIconValue(ID_POP_WIN_BTN1 + num, 0);
	getResBmp(ID_POP_WIN_BTN1 + num, BMPTYPE_BASE, &buttonData->bmpNormal);
	setCurrentIconValue(ID_POP_WIN_BTN1 + num, 1);
	getResBmp(ID_POP_WIN_BTN1 + num, BMPTYPE_BASE, &buttonData->bmpSelect);

	switch (num) {
	case 6:
		buttonData->textX = 25;
		break;
	case 7:
		buttonData->textX = 15;
		break;
	case 9:
		buttonData->textX = 15;
		break;
	case 10:
		buttonData->textX = 15;
		break;
	case 11:
		buttonData->textX = 15;
		break;
	}
	return buttonData;
}

/* push and poll */
static void openMenu(HWND hwnd) {

	isFullOpen = FALSE;

	MSG msg;
	HDC sec_dc_active;

	sec_dc_active = GetSecondaryDC(hwnd);
	SetSecondaryDC(hwnd, sec_dc_active, ON_UPDSECDC_DONOTHING);

	/* get the content of the active main window */
	ShowWindow(hwnd, SW_SHOWNORMAL);
	/* wait for MSG_IDLE */
	while (GetMessage(&msg, hwnd)) {
		if (msg.message == MSG_IDLE)
			break;
		DispatchMessage(&msg);
	}
	int distance = 0;
	while (distance <= WIDTH) {
		BitBlt(sec_dc_active, 0, 0, WIDTH, 480, HDC_SCREEN, 800 - distance, 0,
				0);
		usleep(10000);
		distance += 60;
		if (distance >= WIDTH) {
			BitBlt(sec_dc_active, 0, 0, WIDTH, 480, HDC_SCREEN, LX, 0, 0);
		}
	}

	/* restore to default behavior */
	SetSecondaryDC(hwnd, sec_dc_active, ON_UPDSECDC_DEFAULT);
	isFullOpen = TRUE;
}

static int MenuhWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam) {

	static int slideDistance;
	int i;

	switch (message) {
	case MSG_CREATE: {
		for (i = 0; i < 12; i++) {
			buttonData[i] = MenuButtonDataInit(buttonData[i], i);
		}
		gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
		HWND button = CreateWindowEx(BUTTON_VIEW, TONSUO,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 5, 108,
				108, hWnd, (DWORD) buttonData[0]);
		SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, TONGDEN,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 138, 5, 108,
				108, hWnd, (DWORD) buttonData[1]);
		//SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, WIFI,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 266, 5, 108,
				108, hWnd, (DWORD) buttonData[2]);
		SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, YINLIAN,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, 10, 123, 108,
				108, hWnd, (DWORD) buttonData[3]);
		SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, LIANDU,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 4, 138, 123, 108,
				108, hWnd, (DWORD) buttonData[4]);
		SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, YUYAN,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 5, 266, 123, 108,
				108, hWnd, (DWORD) buttonData[5]);
		SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, SHUIGUANJIA,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 6, 10, 241, 108,
				108, hWnd, (DWORD) buttonData[6]);
		SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, CAOZUOZHIYIN,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 7, 138, 241, 108,
				108, hWnd, (DWORD) buttonData[7]);
		//SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, SHIPING,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 8, 266, 241, 108,
				108, hWnd, (DWORD) buttonData[8]);
		SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, YIWUBIAOZHI,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 9, 10, 359, 108,
				108, hWnd, (DWORD) buttonData[9]);
		//SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, XIYIJIQIAO,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 10, 138, 359, 108,
				108, hWnd, (DWORD) buttonData[10]);
		//SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);

		button = CreateWindowEx(BUTTON_VIEW, HUIFUMOREN,
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 11, 266, 359, 108,
				108, hWnd, (DWORD) buttonData[11]);
		//SetNotificationCallback(button, my_notif_proc);
		SetWindowFont(button, getLogFont(ID_FONT_SIMSUN_20));
		SetWindowBkColor(button, lanse);
		return 0;
	}
	case MSG_LIGHT_MENU_CLOSE:
		loadMenuViewBitMap();
		break;
	case MSG_DESTROY:
		for (i = 0; i < 12; i++) {
			unloadBitMap(&buttonData[i]->bmpNormal);
			unloadBitMap(&buttonData[i]->bmpSelect);
			if (buttonData[i])
				free(buttonData[i]);
		}
		DestroyAllControls(hWnd);
		PostMessage(GetHosting(hWnd), MSG_WIN_POP_CLOSE, 0, 0);
		return 0;
	case MSG_CLOSE: {
		if (isFullOpen) {
			DestroyMainWindow(hWnd);
		}
		return 0;
	}
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

static HWND popMenuWnd = 0;

HWND GetPopMenuHwnd() {
	return popMenuWnd;
}

int WinPopMenuWin(HWND hosting) {
	if (popMenuWnd != 0)
		return -1;

	MSG Msg;
	MAINWINCREATE CreateInfo;
	CreateInfo.dwStyle = WS_NONE;
	CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC | WS_EX_TOPMOST;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = MenuhWinProc;
	CreateInfo.lx = LX;
	CreateInfo.ty = TY;
	CreateInfo.rx = RX;
	CreateInfo.by = BY;
	CreateInfo.iBkColor = PIXEL_lightgray;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hosting;

	popMenuWnd = CreateMainWindow(&CreateInfo);

	if (popMenuWnd == HWND_INVALID)
		return -1;

	openMenu(popMenuWnd);

	while (GetMessage(&Msg, popMenuWnd)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	MainWindowThreadCleanup(popMenuWnd);
	popMenuWnd = 0;
	return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
