#include <bottombar_view.h>

#define TIMER_ID 10
#define SLIDE_DOWN_SPEED 10

static int slideDistance;
static smRect winRect;
static BottomBarView *bottomBarData = NULL;

static ButtonView* BottomButtonDataInit(ButtonView *buttonData, int num,
		int mode) {
	buttonData = (ButtonView*) malloc(sizeof(ButtonView));
	if (NULL == buttonData) {
		sm_error("malloc ButtonView data error\n");
	}
	memset((void *) buttonData, 0, sizeof(ButtonView));

	buttonData->selectStatus = SELECT_STATUS_NO;
	buttonData->switchMode = SWITCH_MODE_NORMAL;

	switch (num) {
	case 1:
		setCurrentIconValue(ID_BOTTOM_WIN_BACK, 0);
		getResBmp(ID_BOTTOM_WIN_BACK, BMPTYPE_BASE, &buttonData->bmpNormal);
		setCurrentIconValue(ID_BOTTOM_WIN_BACK, 1);
		getResBmp(ID_BOTTOM_WIN_BACK, BMPTYPE_BASE, &buttonData->bmpSelect);
		break;
	case 2:
		setCurrentIconValue(ID_BOTTOM_WIN_SETUP, 0);
		getResBmp(ID_BOTTOM_WIN_SETUP, BMPTYPE_BASE, &buttonData->bmpNormal);
		setCurrentIconValue(ID_BOTTOM_WIN_SETUP, 1);
		getResBmp(ID_BOTTOM_WIN_SETUP, BMPTYPE_BASE, &buttonData->bmpSelect);
		break;
	case 3:
		switch (mode) {
		case 0:
			setCurrentIconValue(ID_BOTTOM_WIN_PHOTO, 0);
			getResBmp(ID_BOTTOM_WIN_PHOTO, BMPTYPE_BASE,
					&buttonData->bmpNormal);
			setCurrentIconValue(ID_BOTTOM_WIN_PHOTO, 1);
			getResBmp(ID_BOTTOM_WIN_PHOTO, BMPTYPE_BASE,
					&buttonData->bmpSelect);
			break;
		case 1:
			buttonData->switchMode = SWITCH_MODE_STATIC;
			setCurrentIconValue(ID_BOTTOM_WIN_AUDIO, 0);
			getResBmp(ID_BOTTOM_WIN_AUDIO, BMPTYPE_BASE,
					&buttonData->bmpNormal);
			setCurrentIconValue(ID_BOTTOM_WIN_AUDIO, 1);
			getResBmp(ID_BOTTOM_WIN_AUDIO, BMPTYPE_BASE,
					&buttonData->bmpSelect);
			break;
		case 2:
			buttonData->switchMode = SWITCH_MODE_STATIC;
			setCurrentIconValue(ID_BOTTOM_WIN_VIDEO, 0);
			getResBmp(ID_BOTTOM_WIN_VIDEO, BMPTYPE_BASE,
					&buttonData->bmpNormal);
			setCurrentIconValue(ID_BOTTOM_WIN_VIDEO, 1);
			getResBmp(ID_BOTTOM_WIN_VIDEO, BMPTYPE_BASE,
					&buttonData->bmpSelect);
			break;
		}
		break;
	}
	return buttonData;
}

static BottomBarView* BottomBarDataInit(BottomBarView *bottomBarData, int mode) {
	bottomBarData = (BottomBarView*) malloc(sizeof(BottomBarView));
	if (NULL == bottomBarData) {
		sm_error("malloc HeadBarView data error\n");
	}
	memset((void *) bottomBarData, 0, sizeof(BottomBarView));

	bottomBarData->backData = BottomButtonDataInit(bottomBarData->backData, 1,
			mode);
	bottomBarData->setupData = BottomButtonDataInit(bottomBarData->setupData, 2,
			mode);
	bottomBarData->takeData = BottomButtonDataInit(bottomBarData->takeData, 3,
			mode);
	return bottomBarData;
}

static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED) {
		BroadcastMessage(MSG_HBAR_CLICK, id, 0);
	}
}

static int BottomBarViewProc(HWND hwnd, int message, WPARAM wParam,
		LPARAM lParam) {

	switch (message) {
	case MSG_CREATE: {
		smRect rect;
		getResRect(ID_BOTTOM_WIN_BACK, &rect);
		HWND btn_favor = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_BACK,
				rect.x, rect.y, rect.w, rect.h, hwnd,
				(DWORD) bottomBarData->backData);
		SetNotificationCallback(btn_favor, my_notif_proc);

		getResRect(ID_BOTTOM_WIN_SETUP, &rect);
		HWND btn_setup = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_SETUP,
				rect.x, rect.y, rect.w, rect.h, hwnd,
				(DWORD) bottomBarData->setupData);
		SetNotificationCallback(btn_setup, my_notif_proc);

		getResRect(ID_BOTTOM_WIN_PHOTO, &rect);
		HWND btn_take = CreateWindowEx(BUTTON_VIEW, "",
		WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, ID_BUTTON_TAKE,
				rect.x, rect.y, rect.w, rect.h, hwnd,
				(DWORD) bottomBarData->takeData);
		SetNotificationCallback(btn_take, my_notif_proc);

		/* SetTimer(hwnd, TIMER_ID, 1); */
		slideDistance = 0;
		return 0;
	}
	case MSG_TIMER: {
		MoveWindow(hwnd, winRect.x, g_rcScr.bottom - slideDistance, winRect.w,
				winRect.h, FALSE);
		slideDistance += SLIDE_DOWN_SPEED;
		if (slideDistance >= winRect.h) {
			MoveWindow(hwnd, winRect.x, winRect.y, winRect.w, winRect.h,
			FALSE);
			KillTimer(hwnd, TIMER_ID);
		}
		if (slideDistance == SLIDE_DOWN_SPEED) {
			ShowWindow(hwnd, SW_SHOWNORMAL);
		}
		break;
	}
	case MSG_DESTROY:
		unloadBitMap(&bottomBarData->backData->bmpNormal);
		unloadBitMap(&bottomBarData->backData->bmpSelect);
		unloadBitMap(&bottomBarData->setupData->bmpNormal);
		unloadBitMap(&bottomBarData->setupData->bmpSelect);
		unloadBitMap(&bottomBarData->takeData->bmpNormal);
		unloadBitMap(&bottomBarData->takeData->bmpSelect);
		if (bottomBarData->backData != NULL) {
			free(bottomBarData->backData);
			bottomBarData->backData = NULL;
		}
		if (bottomBarData->setupData != NULL) {
			free(bottomBarData->setupData);
			bottomBarData->setupData = NULL;
		}
		if (bottomBarData->takeData != NULL) {
			free(bottomBarData->takeData);
			bottomBarData->takeData = NULL;
		}
		if (bottomBarData != NULL) {
			free(bottomBarData);
			bottomBarData = NULL;
		}
		return 0;
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

/**
 * Initialize the bottom status bar
 *
 * @hParent		Parent window handle
 * @mode		0 pictures, 1 recording, 2 video
 */
HWND BottomBarViewInit(HWND hParent, int mode) {
	HWND mHwnd;
	getResRect(ID_BOTTOM_WIN, &winRect);
	bottomBarData = BottomBarDataInit(bottomBarData, mode);
	mHwnd = CreateWindowEx(BOTTOM_BAR_VIEW, "",
	WS_CHILD | WS_VISIBLE, WS_EX_TRANSPARENT, 0, winRect.x, winRect.y,
			winRect.w, winRect.h, hParent, 0);

	if (mHwnd == HWND_INVALID) {
		return HWND_INVALID;
	}

	return mHwnd;
}

BOOL RegisterBottomBarView() {
	gal_pixel color;
	color = getResColor(ID_BOTTOM_WIN, COLOR_BGC);	/*  argb */
	WNDCLASS MyClass;
	MyClass.spClassName = BOTTOM_BAR_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = GetSystemCursor(0);
	MyClass.iBkColor = color;
	MyClass.WinProc = BottomBarViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterBottomBarView(void) {
	UnregisterWindowClass(BOTTOM_BAR_VIEW);
}

