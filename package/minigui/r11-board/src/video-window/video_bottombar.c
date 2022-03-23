#include "video_bottombar.h"
#include "resource.h"
#include "washing_res_cn.h"
#include "include.h"

#define TIMER_ID 10

#ifdef ALLWINNERTECH_R6
#define SLIDE_OUT_SPEED 10
#else
#define SLIDE_OUT_SPEED 4
#endif

static int slideDistance = 0;

int GetVideoBottomBarDistance() {
	return slideDistance;
}

static void my_notif_proc(HWND hwnd, int id, int nc, DWORD add_data) {
	if (nc == STN_CLICKED && slideDistance >= 100) {
		switch (id) {
		case ID_BUTTON_PRE:
			app_debug("id=%d, status=%d\n", (int)id, (int) add_data);
			break;
		case ID_BUTTON_NEXT:
			app_debug("id=%d, status=%d\n", (int)id, (int) add_data);
			break;
	     case ID_BUTTON_START:
			app_debug("id=%d, status=%d\n", (int)id, (int) add_data);
			break;
		}
	}
}

static int VideoBottomBarProc(HWND hwnd, int message, WPARAM wParam,
		LPARAM lParam) {

	switch (message) {
	case MSG_CREATE: {
		VideoBarView *bottomBarData = NULL;
		bottomBarData = (VideoBarView*) GetWindowAdditionalData(hwnd);

		if (bottomBarData->preButton) {
			HWND btn_pre = CreateWindowEx(BUTTON_VIEW, "",
			WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT,
			ID_BUTTON_PRE, 515+50, 30, 51, 70, hwnd,
					(DWORD) bottomBarData->preButton);
			SetNotificationCallback(btn_pre, my_notif_proc);
		}
		if (bottomBarData->nextButton) {
			HWND btn_next = CreateWindowEx(BUTTON_VIEW, "",
			WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT,
			ID_BUTTON_NEXT, 686+50, 30, 51, 70, hwnd,
					(DWORD) bottomBarData->nextButton);
			SetNotificationCallback(btn_next, my_notif_proc);
		}
		if (bottomBarData->beginButton) {
			HWND btn_start = CreateWindowEx(BUTTON_VIEW, "",
			WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT,
			ID_BUTTON_START, 576+50, 30, 100, 70, hwnd,
					(DWORD) bottomBarData->beginButton);
			SetNotificationCallback(btn_start, my_notif_proc);
		}

		slideDistance = 0;
		return 0;
	}
	case MSG_TIMER: {
		MoveWindow(hwnd, 0, 480 - slideDistance, 800, slideDistance, FALSE);
		slideDistance += SLIDE_OUT_SPEED;
		if (slideDistance >= 100) {
			MoveWindow(hwnd, 0, 380, 800, 100, FALSE);
			KillTimer(hwnd, TIMER_ID);
		}
		if (slideDistance == SLIDE_OUT_SPEED) {
			ShowWindow(hwnd, SW_SHOWNORMAL);
		}
		break;
	}
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

HWND VideoBottomBarViewInit(HWND hParent, VideoBarView *bottomBarData) {
	HWND mHwnd;
	mHwnd = CreateWindowEx(VIDEO_BOTTOM_BAR, "",
	WS_CHILD, WS_EX_TRANSPARENT, 0, 0, 380, 800, 100, hParent,
			(DWORD) bottomBarData);

	if (mHwnd == HWND_INVALID) {
		return HWND_INVALID;
	}

	return mHwnd;
}

BOOL RegisterVideoBottomBar() {
	WNDCLASS MyClass;
	MyClass.spClassName = VIDEO_BOTTOM_BAR;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = VideoBottomBarProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterVideoBottomBar(void) {
	UnregisterWindowClass(VIDEO_BOTTOM_BAR);
}
