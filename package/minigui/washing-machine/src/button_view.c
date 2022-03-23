#include "button_view.h"

static int ButtonViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

	ButtonView *buttonData = NULL;
	buttonData = (ButtonView*) GetWindowAdditionalData(hwnd);
	static BOOL doubleClick = FALSE;
	static BOOL isUpClick = FALSE;
	static BOOL isDownClick = FALSE;
	const char* spCaption;

	switch (message) {
	case MSG_PAINT: {
		HDC hdc = BeginPaint(hwnd);
		RECT rect;
		GetClientRect(hwnd, &rect);
		spCaption = GetWindowCaption(hwnd);
		SetBkMode(hdc, BM_TRANSPARENT);
		SetTextColor(hdc, GetWindowBkColor(hwnd));
		SelectFont(hdc, GetWindowFont(hwnd));

		if (buttonData->selectStatus == SELECT_STATUS_YES) {
			if (buttonData->bmpSelect.bmBits)
				FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
						&buttonData->bmpSelect);
			if (buttonData->textSelect) {
				TextOut(hdc, buttonData->textX, buttonData->textY,
						buttonData->textSelect);
			} else if (spCaption) {
				TextOut(hdc, buttonData->textX, buttonData->textY, spCaption);
			}
		} else {
			if (buttonData->bmpNormal.bmBits)
				FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
						&buttonData->bmpNormal);
			if (spCaption) {
				TextOut(hdc, buttonData->textX, buttonData->textY, spCaption);
			}
		}

		EndPaint(hwnd, hdc);

		if (isUpClick && (GetWindowStyle(hwnd) & SS_NOTIFY)) {
			isUpClick = FALSE;
			NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED,
					(DWORD) buttonData->selectStatus);
		}
		if (isDownClick && (GetWindowStyle(hwnd) & SS_NOTIFY)
				&& buttonData->switchMode == SWITCH_MODE_STATIC) {
			isDownClick = FALSE;
			NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED,
					(DWORD) buttonData->selectStatus);
		}
		return 0;
	}
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
		if (!doubleClick) {
			if (GetCapture() == hwnd) {
				ReleaseCapture();
				if (buttonData->selectStatus == SELECT_STATUS_YES
						&& buttonData->switchMode == SWITCH_MODE_NORMAL) {
					buttonData->selectStatus = SELECT_STATUS_NO;
					isUpClick = TRUE;
					isDownClick = FALSE;
					InvalidateRect(hwnd, NULL, TRUE);
				}
			}
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
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = ButtonViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterButtonView(void) {
	UnregisterWindowClass(BUTTON_VIEW);
}

