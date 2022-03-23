#include "features_menu.h"
#include "button_view.h"
#include "washing_res_cn.h"
#include "resource.h"

#define TIMER_ID 10
#define SLIDE_OUT_SPEED 16
static int ID = 0;
static ButtonView *buttonData0[8];
static BOOL isFullOpen = FALSE;
BOOL lightMenuOpen;

int i = 0;
static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	HWND parent = GetParent(hwnd);
	if (nc == STN_CLICKED) {
		printf("id = %d\n", id);
		lightMenuOpen = TRUE;
		yuyue_menu(parent);
	}
}

static ButtonView* MenuButtonDataInit(ButtonView *buttonData, int num, int id) {
	buttonData = (ButtonView*) malloc(sizeof(ButtonView));
	if (NULL == buttonData) {
		printf("malloc ButtonView data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonView));

	buttonData->selectStatus = SELECT_STATUS_NO;
	buttonData->switchMode = SWITCH_MODE_NORMAL;
	//buttonData->logfontText = getLogFont(ID_FONT_SIMSUN_20);
	//buttonData->textColor = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData->textX = 35;
	buttonData->textY = 70;
	setCurrentIconValue(ID_FETURES1_BTN1 + num, 0);
	getResBmp(ID_FETURES1_BTN1 + num, BMPTYPE_BASE, &buttonData->bmpNormal);
	setCurrentIconValue(ID_FETURES1_BTN1 + num, 1);
	getResBmp(ID_FETURES1_BTN1 + num, BMPTYPE_BASE, &buttonData->bmpSelect);
	return buttonData;
}

static void openMenu(HWND hwnd) {

	isFullOpen = FALSE;

	MSG msg;
	HDC sec_dc_active;

	sec_dc_active = GetSecondaryDC(hwnd);
	SetSecondaryDC(hwnd, sec_dc_active, ON_UPDSECDC_DONOTHING);

	ShowWindow(hwnd, SW_SHOWNORMAL);
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
	SetSecondaryDC(hwnd, sec_dc_active, ON_UPDSECDC_DEFAULT);
	isFullOpen = TRUE;
}

void CreateFeaturesZero(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 4; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 0, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, XIDIJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 10, 125, 125,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);

	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 1, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, ROUSHUNJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 125, 125,
			hWnd, (DWORD) buttonData0[1]);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);

	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 2, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, HONGGAN_M,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 10, 155, 125, 125,
			hWnd, (DWORD) buttonData0[2]);
	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);

	buttonData0[3] = MenuButtonDataInit(buttonData0[3], 3, id);
	HWND button3 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, 155, 155, 125, 125,
			hWnd, (DWORD) buttonData0[3]);
	SetNotificationCallback(button3, my_notif_proc);
	SetWindowFont(button3, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button3, lanse);
}
#if 1
void CreateFeaturesTwo(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 8; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 0, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, XIDIJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 20, 10, 100, 100,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);
	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 1, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, ROUSHUNJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 100, 100,
			hWnd, (DWORD) buttonData0[1]);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);
	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 2, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, HONGGAN_M,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 20, 120, 100, 100,
			hWnd, (DWORD) buttonData0[2]);
	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);

	buttonData0[3] = MenuButtonDataInit(buttonData0[3], 3, id);
	HWND button3 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, 155, 120, 100, 100,
			hWnd, (DWORD) buttonData0[3]);
	SetNotificationCallback(button3, my_notif_proc);

	SetWindowFont(button3, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button3, lanse);
	buttonData0[4] = MenuButtonDataInit(buttonData0[4], 4, id);
	HWND button4 = CreateWindowEx(BUTTON_VIEW, JIASUXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 4, 20, 230, 100, 100,
			hWnd, (DWORD) buttonData0[4]);
	SetWindowFont(button4, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button4, lanse);

	buttonData0[5] = MenuButtonDataInit(buttonData0[5], 5, id);
	HWND button5 = CreateWindowEx(BUTTON_VIEW, TEZIXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 5, 155, 230, 100, 100,
			hWnd, (DWORD) buttonData0[5]);
	SetWindowFont(button5, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button5, lanse);

	buttonData0[6] = MenuButtonDataInit(buttonData0[6], 6, id);
	HWND button6 = CreateWindowEx(BUTTON_VIEW, WUTAIXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 6, 20, 340, 100, 100,
			hWnd, (DWORD) buttonData0[6]);
	SetWindowFont(button6, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button6, lanse);

	buttonData0[7] = MenuButtonDataInit(buttonData0[7], 7, id);
	HWND button7 = CreateWindowEx(BUTTON_VIEW, JINGYINXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 7, 155, 340, 100, 100,
			hWnd, (DWORD) buttonData0[7]);
	SetWindowFont(button7, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button7, lanse);
}

void CreateFeaturesThree(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 5; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 0, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, XIDIJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 10, 125, 125,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);

	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 1, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, ROUSHUNJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 125, 125,
			hWnd, (DWORD) buttonData0[1]);
	SetNotificationCallback(button1, my_notif_proc);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);
	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 3, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 10, 155, 125, 125,
			hWnd, (DWORD) buttonData0[2]);
	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);

	buttonData0[3] = MenuButtonDataInit(buttonData0[3], 7, id);
	HWND button3 = CreateWindowEx(BUTTON_VIEW, JINGYINXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, 155, 155, 125, 125,
			hWnd, (DWORD) buttonData0[3]);
	SetWindowFont(button3, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button3, lanse);
	buttonData0[4] = MenuButtonDataInit(buttonData0[4], 6, id);
	HWND button4 = CreateWindowEx(BUTTON_VIEW, WUTAIXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 4, 10, 300, 125, 125,
			hWnd, (DWORD) buttonData0[4]);
	SetWindowFont(button4, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button4, lanse);
}

void CreateFeaturesOne(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 7; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 0, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, XIDIJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 20, 10, 100, 100,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);
	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 1, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, ROUSHUNJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 100, 100,
			hWnd, (DWORD) buttonData0[1]);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);
	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 2, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, HONGGAN_M,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 20, 120, 100, 100,
			hWnd, (DWORD) buttonData0[2]);
	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);
	buttonData0[3] = MenuButtonDataInit(buttonData0[3], 3, id);
	HWND button3 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, 155, 120, 100, 100,
			hWnd, (DWORD) buttonData0[3]);
	SetNotificationCallback(button3, my_notif_proc);

	SetWindowFont(button3, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button3, lanse);
	buttonData0[4] = MenuButtonDataInit(buttonData0[4], 4, id);
	HWND button4 = CreateWindowEx(BUTTON_VIEW, JIASUXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 4, 20, 230, 100, 100,
			hWnd, (DWORD) buttonData0[4]);
	SetWindowFont(button4, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button4, lanse);

	buttonData0[5] = MenuButtonDataInit(buttonData0[5], 5, id);
	HWND button5 = CreateWindowEx(BUTTON_VIEW, TEZIXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 5, 155, 230, 100, 100,
			hWnd, (DWORD) buttonData0[5]);
	SetWindowFont(button5, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button5, lanse);

	buttonData0[6] = MenuButtonDataInit(buttonData0[6], 6, id);
	HWND button6 = CreateWindowEx(BUTTON_VIEW, WUTAIXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 6, 20, 340, 100, 100,
			hWnd, (DWORD) buttonData0[6]);
	SetWindowFont(button6, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button6, lanse);
}

void CreateFeaturesSix(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 6; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 0, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, XIDIJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 10, 125, 125,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);

	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 1, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, ROUSHUNJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 125, 125,
			hWnd, (DWORD) buttonData0[1]);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);

	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 3, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 10, 155, 125, 125,
			hWnd, (DWORD) buttonData0[2]);
	SetNotificationCallback(button2, my_notif_proc);

	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);
	buttonData0[3] = MenuButtonDataInit(buttonData0[3], 4, id);
	HWND button3 = CreateWindowEx(BUTTON_VIEW, JIASUXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, 155, 155, 125, 125,
			hWnd, (DWORD) buttonData0[3]);
	SetWindowFont(button3, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button3, lanse);
	buttonData0[4] = MenuButtonDataInit(buttonData0[4], 5, id);
	HWND button4 = CreateWindowEx(BUTTON_VIEW, WUTAIXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 4, 10, 300, 125, 125,
			hWnd, (DWORD) buttonData0[4]);
	SetWindowFont(button4, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button4, lanse);
	buttonData0[5] = MenuButtonDataInit(buttonData0[5], 7, id);
	HWND button5 = CreateWindowEx(BUTTON_VIEW, JINGYINXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 5, 155, 300, 125, 125,
			hWnd, (DWORD) buttonData0[5]);
	SetWindowFont(button5, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button5, lanse);
}

void CreateFeaturesEight(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 7; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 0, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, XIDIJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 20, 10, 100, 100,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);

	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 1, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, ROUSHUNJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 100, 100,
			hWnd, (DWORD) buttonData0[1]);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);

	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 2, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, HONGGAN_M,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 20, 120, 100, 100,
			hWnd, (DWORD) buttonData0[2]);
	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);
	buttonData0[3] = MenuButtonDataInit(buttonData0[3], 3, id);
	HWND button3 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, 155, 120, 100, 100,
			hWnd, (DWORD) buttonData0[3]);
	SetNotificationCallback(button3, my_notif_proc);
	SetWindowFont(button3, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button3, lanse);
	buttonData0[4] = MenuButtonDataInit(buttonData0[4], 4, id);
	HWND button4 = CreateWindowEx(BUTTON_VIEW, JIASUXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 4, 20, 230, 100, 100,
			hWnd, (DWORD) buttonData0[4]);
	SetWindowFont(button4, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button4, lanse);
	buttonData0[5] = MenuButtonDataInit(buttonData0[5], 7, id);
	HWND button5 = CreateWindowEx(BUTTON_VIEW, JINGYINXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 5, 155, 230, 100, 100,
			hWnd, (DWORD) buttonData0[5]);
	SetWindowFont(button5, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button5, lanse);
	buttonData0[6] = MenuButtonDataInit(buttonData0[6], 6, id);
	HWND button6 = CreateWindowEx(BUTTON_VIEW, WUTAIXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 6, 20, 340, 100, 100,
			hWnd, (DWORD) buttonData0[6]);
	SetWindowFont(button6, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button6, lanse);
}

void CreateFeaturestwelve(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 4; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 1, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, ROUSHUNJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 10, 125, 125,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);
	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 3, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 125, 125,
			hWnd, (DWORD) buttonData0[1]);
	SetNotificationCallback(button1, my_notif_proc);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);
	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 6, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, WUTAIXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 10, 155, 125, 125,
			hWnd, (DWORD) buttonData0[2]);
	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);
	buttonData0[3] = MenuButtonDataInit(buttonData0[3], 7, id);
	HWND button3 = CreateWindowEx(BUTTON_VIEW, JINGYINXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, 155, 155, 125, 125,
			hWnd, (DWORD) buttonData0[3]);
	SetWindowFont(button3, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button3, lanse);
}

void CreateFeaturesthirteen(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 3; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 6, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, WUTAIXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 10, 125, 125,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);
	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 7, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, JINGYINXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 125, 125,
			hWnd, (DWORD) buttonData0[1]);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);
	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 3, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 10, 155, 125, 125,
			hWnd, (DWORD) buttonData0[2]);
	SetNotificationCallback(button2, my_notif_proc);
	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);
}

void CreateFeaturesfourteen(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 3; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 2, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, HONGGAN_M,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 10, 125, 125,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);
	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 3, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 125, 125,
			hWnd, (DWORD) buttonData0[1]);
	SetNotificationCallback(button1, my_notif_proc);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);
	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 7, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, JINGYINXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 10, 155, 125, 125,
			hWnd, (DWORD) buttonData0[2]);
	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);
}

void CreateFeaturesfifteen(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 4; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 0, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, XIDIJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 10, 125, 125,
			hWnd, (DWORD) buttonData0[0]);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);
	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 1, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, ROUSHUNJI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 125, 125,
			hWnd, (DWORD) buttonData0[1]);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);
	buttonData0[2] = MenuButtonDataInit(buttonData0[2], 2, id);
	HWND button2 = CreateWindowEx(BUTTON_VIEW, JINGYINXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 2, 10, 155, 125, 125,
			hWnd, (DWORD) buttonData0[2]);
	SetWindowFont(button2, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button2, lanse);
	buttonData0[3] = MenuButtonDataInit(buttonData0[3], 3, id);
	HWND button3 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 3, 155, 155, 125, 125,
			hWnd, (DWORD) buttonData0[3]);
	SetNotificationCallback(button3, my_notif_proc);
	SetWindowFont(button3, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button3, lanse);
}

void CreateFeaturessixteen(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 1; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);
	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 0, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 10, 125, 125,
			hWnd, (DWORD) buttonData0[0]);
	SetNotificationCallback(button0, my_notif_proc);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);
}

void CreateFeaturesseventeen(HWND hWnd, int id) {
#if 0
	for (i = 0; i < 2; i++) {
		buttonData0[i] = MenuButtonDataInit(buttonData0[i], i, id);
	}
#endif
	gal_pixel lanse = RGB2Pixel(HDC_SCREEN, 45, 83, 126);

	buttonData0[0] = MenuButtonDataInit(buttonData0[0], 0, id);
	HWND button0 = CreateWindowEx(BUTTON_VIEW, YUYUE,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 10, 10, 125, 125,
			hWnd, (DWORD) buttonData0[0]);
	SetNotificationCallback(button0, my_notif_proc);
	SetWindowFont(button0, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button0, lanse);
	buttonData0[1] = MenuButtonDataInit(buttonData0[1], 1, id);
	HWND button1 = CreateWindowEx(BUTTON_VIEW, JINGYINXI,
	WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 1, 155, 10, 125, 125,
			hWnd, (DWORD) buttonData0[1]);
	SetWindowFont(button1, getLogFont(ID_FONT_SIMSUN_20));
	SetWindowBkColor(button1, lanse);
}

#endif
static void PaintWindow(HWND hWnd, int id) {
	switch (id) {
	case 0:
		CreateFeaturesZero(hWnd, id);
		break;
#if 1
	case 1:
		CreateFeaturesTwo(hWnd, id);
		break;
	case 2:
		CreateFeaturesTwo(hWnd, id);
		break;
	case 3:
		CreateFeaturesThree(hWnd, id);
		break;
	case 4:
		CreateFeaturesOne(hWnd, id);
		break;
	case 5:
		CreateFeaturesOne(hWnd, id);
		break;
		//two
	case 6:
		CreateFeaturesSix(hWnd, id);
		break;
	case 7:
		CreateFeaturesSix(hWnd, id);
		break;
	case 8:
		CreateFeaturesEight(hWnd, id);
		break;
	case 9:
		CreateFeaturesEight(hWnd, id);
		break;
	case 10:
		CreateFeaturesSix(hWnd, id);
		break;
	case 11:
		CreateFeaturesThree(hWnd, id);
		break;
#if 1
		//three
	case 12:
		CreateFeaturestwelve(hWnd, id);
		break;
	case 13:
		CreateFeaturesthirteen(hWnd, id);
		break;
	case 14:
		CreateFeaturesfourteen(hWnd, id);
		break;
	case 15:
		CreateFeaturesfifteen(hWnd, id);
		break;
	case 16:
		CreateFeaturessixteen(hWnd, id);
		break;
	case 17:
		CreateFeaturesseventeen(hWnd, id);
		break;
#endif
#endif
	}
}

static int MenuhWinProc(HWND hWnd, int message, WPARAM wParam, LPARAM lParam) {

	static int slideDistance;
	int i;

	switch (message) {
	case MSG_CREATE: {
		PaintWindow(hWnd, ID);
		SetTimer(hWnd, TIMER_ID, 1);
		slideDistance = 0;
		return 0;

	}
	case MSG_CLOSE: {
		if (isFullOpen) {
			for (i = 0; i < 8; i++) {
				if (buttonData0[i]) {
					unloadBitMap(&buttonData0[i]->bmpNormal);
					unloadBitMap(&buttonData0[i]->bmpSelect);
					free(buttonData0[i]);
					buttonData0[i] = NULL;
				}
			}
			DestroyAllControls(hWnd);
			DestroyMainWindow(hWnd);
		}
		return 0;
	}
	}

	return DefaultMainWinProc(hWnd, message, wParam, lParam);
}

HWND fetureMenuWnd;

HWND GetfetureMenuHwnd() {
	return fetureMenuWnd;
}

int features_menu(HWND hosting, int id) {
	MSG Msg;
	MAINWINCREATE CreateInfo;
	ID = id;
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

	fetureMenuWnd = CreateMainWindow(&CreateInfo);

	if (fetureMenuWnd == HWND_INVALID)
		return -1;

	//isFullOpen = FALSE;
	openMenu(fetureMenuWnd);
	while (GetMessage(&Msg, fetureMenuWnd)) {
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}

	MainWindowThreadCleanup(fetureMenuWnd);
	fetureMenuWnd = 0;
	return 0;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
