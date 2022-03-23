#include "HeadWindow.h"
#include "BottomMenu.h"
#include "RightMenu.h"
#include <videoOutPort.h>
#include "multimedia_test.h"
extern WinDataType *g_win_data;
extern RightMenuDataType *RightMenuWnd;
extern BottomMenuDataType *bottom;
SetupDataType *setupData;
HbarWinDataType *headbar = NULL;
extern int file_index;
extern dispOutPort *hdl;

void SetUpInit(void) {
	setupData = (SetupDataType *) malloc(sizeof(SetupDataType));
	setupData->ScreenResolution1280x800 = headbar->is1280x800;
	setupData->ScreenResolution800x480 = headbar->is800x480;
	setupData->ScreenResolution1280x480 = headbar->is1280x480;
	setupData->IsForLoop = headbar->isforloop;
	setupData->IsRotate = headbar->isrotate;
}

void InitheadbarMenu() {
	headbar->isCreateMediaList = FALSE;
	headbar->is1280x800 = 0;
	headbar->is800x480 = 0;
	headbar->is1280x480 = 0;
	headbar->isforloop = 0;
	headbar->isrotate = 0;
}
void head_back(HWND hwnd) {
	HWND parent = GetParent(hwnd);
	if (RightMenuWnd != NULL) {
		RightMenuWnd->boolfreespace = 0;
	}
	if (hdl != NULL) {
		jpeg_dispClose();
	}
	PostMessage(GetHosting(parent), MSG_TAKE_CAMERA_CLOSE, 0, 0);
	SendMessage(bottom->BottomMenuHwnd, MSG_CLOSE, 0, 0);
	SendMessage(headbar->HbarHwnd, MSG_CLOSE, 0, 0);
	file_index = -1;
}
static void headbar_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {

	if (nc == STN_CLICKED) {
		HWND parent = GetParent(hwnd);
		switch (id) {
		case ID_LIST:
			RightMenuInit(parent, ID_LIST, headbar->addID);
			break;
		case ID_SETUP:
			SetUpInit();
			InitheadbarMenu();
			RightMenuInit(parent, ID_SETUP, headbar->addID);
			break;
		case ID_BACK:
			head_back(hwnd);
			break;
		}
	}
}

void CreateHeadBarBtn(HWND hWnd, int id) {
	CvrRectType rect;
	CvrRectType rect_sdcard;

	getResRect(ID_BACK, &headbar->backSize);
	getResBmp(ID_BACK, BMPTYPE_BASE, &headbar->backBmp);
	printf("head bar back:[%d][%d][%d][%d]\n", headbar->backSize.x,
			headbar->backSize.y, headbar->backSize.w, headbar->backSize.h);

	headbar->backHwnd = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY | SS_CENTERIMAGE,
	WS_EX_TRANSPARENT, ID_BACK, headbar->backSize.x, headbar->backSize.y,
			headbar->backSize.w, headbar->backSize.h, hWnd,
			(DWORD) &headbar->backBmp);
	SetNotificationCallback(headbar->backHwnd, headbar_notif_proc);

	if (power_is_charging() != POWER_BAT_ONLY) {
		setCurrentIconValue(ID_BATTERY, 5);
		getResBmp(ID_BATTERY, BMPTYPE_BASE, &headbar->batteryBmp);
	} else {
		switch (power_get_battery_level()) {
		case POWER_LEVEL_0: {
			setCurrentIconValue(ID_BATTERY, 0);
			getResBmp(ID_BATTERY, BMPTYPE_BASE, &headbar->batteryBmp);
			break;
		}

		case POWER_LEVEL_1: {
			setCurrentIconValue(ID_BATTERY, 1);
			getResBmp(ID_BATTERY, BMPTYPE_BASE, &headbar->batteryBmp);
			break;
		}

		case POWER_LEVEL_2: {
			setCurrentIconValue(ID_BATTERY, 2);
			getResBmp(ID_BATTERY, BMPTYPE_BASE, &headbar->batteryBmp);
			break;
		}
		case POWER_LEVEL_3: {
			setCurrentIconValue(ID_BATTERY, 3);
			getResBmp(ID_BATTERY, BMPTYPE_BASE, &headbar->batteryBmp);
			break;
		}

		case POWER_LEVEL_4: {
			setCurrentIconValue(ID_BATTERY, 4);
			getResBmp(ID_BATTERY, BMPTYPE_BASE, &headbar->batteryBmp);
			break;
		}

		case POWER_LEVEL_5: {
			setCurrentIconValue(ID_BATTERY, 5);
			getResBmp(ID_BATTERY, BMPTYPE_BASE, &headbar->batteryBmp);
			break;
		}
		default: {
			setCurrentIconValue(ID_BATTERY, 0);
			getResBmp(ID_BATTERY, BMPTYPE_BASE, &headbar->batteryBmp);
			break;
		}
		}
	}
	getResRect(ID_BATTERY, &headbar->batterySize);
	printf("head battery:[%d][%d][%d][%d]\n", headbar->batterySize.x,
			headbar->batterySize.y, headbar->batterySize.w,
			headbar->batterySize.h);
	headbar->batteryHwnd = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY | SS_CENTERIMAGE,
	WS_EX_TRANSPARENT, ID_BATTERY, headbar->batterySize.x,
			headbar->batterySize.y, headbar->batterySize.w,
			headbar->batterySize.h, hWnd, (DWORD) &headbar->batteryBmp);

	if (sdcard_is_exist() == 1) {
		setCurrentIconValue(ID_SDCARD, 1);
	} else if (sdcard_is_exist() == 0) {
		setCurrentIconValue(ID_SDCARD, 0);
	}
	getResRect(ID_SDCARD, &headbar->tfcardSize);
	getResBmp(ID_SDCARD, BMPTYPE_BASE, &headbar->tfcardBmp);
	printf("head sdcard:[%d][%d][%d][%d]\n", headbar->tfcardSize.x,
			headbar->tfcardSize.y, headbar->tfcardSize.w,
			headbar->tfcardSize.h);
	headbar->tfcardHwnd = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY | SS_CENTERIMAGE,
	WS_EX_TRANSPARENT, ID_SDCARD, headbar->tfcardSize.x, headbar->tfcardSize.y,
			headbar->tfcardSize.w, headbar->tfcardSize.h, hWnd,
			(DWORD) &headbar->tfcardBmp);

	getResRect(ID_LIST, &headbar->listSize);
	getResBmp(ID_LIST, BMPTYPE_BASE, &headbar->listBmp);
	printf("head bar list:[%d][%d][%d][%d]\n", headbar->listSize.x,
			headbar->listSize.y, headbar->listSize.w, headbar->listSize.h);
	headbar->listHwnd = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY | SS_CENTERIMAGE,
	WS_EX_TRANSPARENT, ID_LIST, headbar->listSize.x, headbar->listSize.y,
			headbar->listSize.w, headbar->listSize.h, hWnd,
			(DWORD) &headbar->listBmp);
	SetNotificationCallback(headbar->listHwnd, headbar_notif_proc);

	getResRect(ID_SETUP, &headbar->setupSize);
	setCurrentIconValue(ID_SETUP, 0);
	getResBmp(ID_SETUP, BMPTYPE_BASE, &headbar->setupBmp);
	headbar->setupHwnd = CreateWindowEx(CTRL_STATIC, "",
	WS_CHILD | WS_VISIBLE | SS_BITMAP | SS_NOTIFY | SS_CENTERIMAGE,
	WS_EX_TRANSPARENT, ID_SETUP, headbar->setupSize.x, headbar->setupSize.y,
			headbar->setupSize.w, headbar->setupSize.h, hWnd,
			(DWORD) &headbar->setupBmp);
	printf("head bar setup:[%d][%d][%d][%d]\n", headbar->setupSize.x,
			headbar->setupSize.y, headbar->setupSize.w, headbar->setupSize.h);
	SetNotificationCallback(headbar->setupHwnd, headbar_notif_proc);

}
static int HeadBarProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	switch (message) {
	case MSG_CREATE:
		CreateHeadBarBtn(hWnd, headbar->addID);
		return 0;
	case MSG_KEYUP:
		switch (wParam) {
		case CVR_KEY_POWER:
			sm_debug("headbar CVR_KEY_POWER\n");
			head_back(headbar->backHwnd);
			break;
		}
		return 0;
	case MSG_ERASEBKGND: {
		hdc = BeginPaint(hWnd);
		SetMemDCColorKey(hdc, MEMDC_FLAG_SRCCOLORKEY, PIXEL_transparent);
		EndPaint(hWnd, hdc);
		return 0;
	}
	case MSG_CLOSE:
		DestroyMainWindow(hWnd);
		break;
	case MSG_DESTROY: {
		if (NULL != headbar) {
			unloadBitMap(&headbar->tfcardBmp);
			unloadBitMap(&headbar->batteryBmp);
			unloadBitMap(&headbar->backBmp);
			unloadBitMap(&headbar->listBmp);
			unloadBitMap(&headbar->setupBmp);
			if (RightMenuWnd != NULL) {
				SendMessage(RightMenuWnd->RightMenuHwnd, MSG_CLOSE, 0, 0);
			}
			free(headbar);
			headbar = NULL;
			if (sdcard_is_exist()) {
				printf("back delete sdcard\n");
			}
		}
		return 0;
	}

	}
	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}
HWND HbarWinInit(HWND hosting, int id) {
	MSG Msg;
	MAINWINCREATE CreateInfo;
	if (NULL == headbar) {
		headbar = (HbarWinDataType*) malloc(sizeof(HbarWinDataType));
	}

	headbar->Displayrect.x = 0;
	headbar->Displayrect.y = 0;
	headbar->Displayrect.w = g_rcScr.right;
	headbar->Displayrect.h = g_rcScr.bottom;

	getResRect(ID_HEADBAR, &headbar->hbarSize);
	printf("head bar window:[%d][%d][%d][%d]\n", headbar->hbarSize.x,
			headbar->hbarSize.y, headbar->hbarSize.w, headbar->hbarSize.h);
	CreateInfo.dwStyle = WS_NONE;
	/*CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC | WS_EX_TOPMOST;*/
	CreateInfo.dwExStyle = WS_EX_AUTOSECONDARYDC;
	CreateInfo.spCaption = "";
	CreateInfo.hMenu = 0;
	CreateInfo.hCursor = GetSystemCursor(0);
	CreateInfo.hIcon = 0;
	CreateInfo.MainWindowProc = HeadBarProc;
	CreateInfo.lx = headbar->hbarSize.x;
	CreateInfo.ty = headbar->hbarSize.y;
	CreateInfo.rx = headbar->hbarSize.w;
	CreateInfo.by = headbar->hbarSize.h;
	CreateInfo.iBkColor = PIXEL_transparent;
	CreateInfo.dwAddData = 0;
	CreateInfo.hHosting = hosting;

	headbar->HbarHwnd = CreateMainWindow(&CreateInfo);
	headbar->ParentHwnd = hosting;
	headbar->addID = id;
	headbar->isRightWndOpen = FALSE;

	if (headbar->HbarHwnd == HWND_INVALID)
		return -1;

	ShowWindow(headbar->HbarHwnd, SW_SHOWNORMAL);
	MainWindowThreadCleanup(headbar->HbarHwnd);
	return headbar->HbarHwnd;
}
