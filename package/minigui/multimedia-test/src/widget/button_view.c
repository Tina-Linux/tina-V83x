#include "button_view.h"
#include <stdio.h>
#include <sys/time.h>
#include <time.h>
static long oldtime = 0;

static int ButtonViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

	ButtonView *buttonData = NULL;
	buttonData = (ButtonView*) GetWindowAdditionalData(hwnd);
	static BOOL doubleClick = FALSE;
	static BOOL isUpClick = FALSE;
	static BOOL isDownClick = FALSE;

	struct timeval tv;

	switch (message) {
	case MSG_CREATE:
		gettimeofday(&tv, NULL);
		oldtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
		return 0;
	case MSG_PAINT: {
		HDC hdc = BeginPaint(hwnd);
		RECT rect;
		GetClientRect(hwnd, &rect);

		if (buttonData->selectStatus == SELECT_STATUS_YES) {
			if (buttonData->bmpSelect.bmBits)
				FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
						&buttonData->bmpSelect);
		} else {
			if (buttonData->bmpNormal.bmBits)
				FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
						&buttonData->bmpNormal);
		}

		EndPaint(hwnd, hdc);

		if (isUpClick && (GetWindowStyle(hwnd) & SS_NOTIFY) && !doubleClick) {
			isUpClick = FALSE;
			NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED,
					(DWORD) buttonData->selectStatus);
		}
		if (isDownClick && (GetWindowStyle(hwnd) & SS_NOTIFY) && !doubleClick
				&& buttonData->switchMode == SWITCH_MODE_STATIC) {
			isDownClick = FALSE;
			NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED,
					(DWORD) buttonData->selectStatus);
		}
		return 0;
	}
	case STM_SETIMAGE:
		if (buttonData->selectStatus == SELECT_STATUS_YES) {
			buttonData->selectStatus = SELECT_STATUS_NO;
		} else {
			buttonData->selectStatus = SELECT_STATUS_YES;
		}
		InvalidateRect(hwnd, NULL, TRUE);
		isUpClick = FALSE;
		isDownClick = FALSE;
		break;
	case MSG_LBUTTONDOWN: {
		if (buttonData->switchMode == SWITCH_MODE_FORBID)
			break;
		if (GetCapture() == hwnd)
			break;
		SetCapture(hwnd);
		if (buttonData->selectStatus == SELECT_STATUS_YES) {
			buttonData->selectStatus = SELECT_STATUS_NO;
		} else {
			buttonData->selectStatus = SELECT_STATUS_YES;
		}
		InvalidateRect(hwnd, NULL, TRUE);
		doubleClick = FALSE;
		isUpClick = FALSE;
		isDownClick = TRUE;
		break;
	}
	case MSG_LBUTTONDBLCLK:
		doubleClick = TRUE;
		break;
	case MSG_LBUTTONUP:
		if (buttonData->switchMode == SWITCH_MODE_FORBID)
			break;
		if (GetCapture() == hwnd)
			ReleaseCapture();

		gettimeofday(&tv, NULL);
		long newtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
		int lentime = newtime - oldtime;
		if (lentime < 300) {
			doubleClick = TRUE;
		}
		oldtime = newtime;

		if (buttonData->selectStatus == SELECT_STATUS_YES
				&& buttonData->switchMode == SWITCH_MODE_NORMAL) {
			buttonData->selectStatus = SELECT_STATUS_NO;
			isUpClick = TRUE;
			isDownClick = FALSE;
			InvalidateRect(hwnd, NULL, TRUE);
		}
		break;
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterButtonView() {
	WNDCLASS MyClass;
	MyClass.spClassName = BUTTON_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = GetSystemCursor(0);
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = ButtonViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterButtonView(void) {
	UnregisterWindowClass(BUTTON_VIEW);
}
