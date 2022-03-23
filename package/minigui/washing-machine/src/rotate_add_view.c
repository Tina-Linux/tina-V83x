#include "rotate_add_view.h"
#include "washing_res_cn.h"
#include "common.h"

#define TIMER_ID 10
#define SELECT_STATUS_YES 1
#define SELECT_STATUS_NO 0

#ifdef ALLWINNERTECH_R6
#define SPINNING_SPEED 1792
#else
#define SPINNING_SPEED 448
#endif

static int RotateAddViewProc(HWND hwnd, int message, WPARAM wParam,
		LPARAM lParam) {

	const char* spCaption;
	static int selectStatus = SELECT_STATUS_NO;
	static BOOL doubleClick = FALSE;
	static int angle = 0;
	static BOOL isRotateEnd = FALSE;
	RotateAddView *rotateAddView = NULL;
	rotateAddView = (RotateAddView*) GetWindowAdditionalData(hwnd);

	switch (message) {
	case MSG_TIMER: {
		angle += SPINNING_SPEED;
		if (angle >= 46080) {
			angle = 0;
			isRotateEnd = TRUE;
			KillTimer(hwnd, TIMER_ID);
		}
		InvalidateRect(hwnd, NULL, TRUE);
		break;
	}
	case MSG_PAINT: {
		HDC hdc = BeginPaint(hwnd);

		if (selectStatus == SELECT_STATUS_NO) {
			FillBoxWithBitmap(hdc, 11, 5, 0, 0, &rotateAddView->bmpBgMin);
			RotateBitmap(hdc, &rotateAddView->bmpAnim[0], 150, 35, angle);
		} else if (selectStatus == SELECT_STATUS_YES) {
			FillBoxWithBitmap(hdc, 0, 0, 0, 0, &rotateAddView->bmpBgBig);
			FillBoxWithBitmap(hdc, 148, 33, 0, 0, &rotateAddView->bmpAnim[1]);
		}

		SetBkMode(hdc, BM_TRANSPARENT);
		SetTextColor(hdc, COLOR_lightwhite);
		SelectFont(hdc, GetWindowFont(hwnd));

		spCaption = GetWindowCaption(hwnd);
		if (spCaption) {
			TextOut(hdc, 30, 30, spCaption);
		}

		EndPaint(hwnd, hdc);

		if (isRotateEnd && (GetWindowStyle(hwnd) & SS_NOTIFY)) {
			isRotateEnd = FALSE;
			NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED,
					(DWORD) selectStatus);
		}
		return 0;
	}
	case MSG_LBUTTONDOWN: {
		if (GetCapture() == hwnd)
			break;
		SetCapture(hwnd);
		if (selectStatus == SELECT_STATUS_YES) {
			selectStatus = SELECT_STATUS_NO;
		} else {
			selectStatus = SELECT_STATUS_YES;
		}
		InvalidateRect(hwnd, NULL, TRUE);
		doubleClick = FALSE;
		isRotateEnd = FALSE;
		break;
	}
	case MSG_LBUTTONDBLCLK:
		doubleClick = TRUE;
		break;
	case MSG_LBUTTONUP:
		if (!doubleClick) {
			if (GetCapture() == hwnd) {
				ReleaseCapture();
				if (selectStatus == SELECT_STATUS_YES) {
					selectStatus = SELECT_STATUS_NO;
					SetTimer(hwnd, TIMER_ID, 1);
				}
			}
		}
		break;
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterRotateAddView() {
	WNDCLASS MyClass;
	MyClass.spClassName = ROTATE_ADD_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = RotateAddViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterRotateAddView(void) {
	UnregisterWindowClass(ROTATE_ADD_VIEW);
}


