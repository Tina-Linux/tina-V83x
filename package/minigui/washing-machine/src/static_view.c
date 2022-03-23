#include "static_view.h"

static int StaticViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

	BITMAP *bmp;
	static BOOL doubleClick = FALSE;

	switch (message) {
	case MSG_PAINT: {
		HDC hdc = BeginPaint(hwnd);
		bmp = (BITMAP*) GetWindowAdditionalData(hwnd);
		if (bmp->bmBits)
			FillBoxWithBitmap(hdc, 0, 0, 0, 0, bmp);
		EndPaint(hwnd, hdc);
		return 0;
	}
	case MSG_LBUTTONDOWN:
		doubleClick = FALSE;
		break;
	case MSG_LBUTTONDBLCLK:
		doubleClick = TRUE;
		break;
	case MSG_LBUTTONUP:
		if (!doubleClick && (GetWindowStyle(hwnd) & SS_NOTIFY)) {
			PostMessage(GetParent(hwnd), MSG_STATIC_CLICK, GetDlgCtrlID(hwnd),
					0);
		}
		break;
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterStaticView() {
	WNDCLASS MyClass;
	MyClass.spClassName = STATIC_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = StaticViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterStaticView(void) {
	UnregisterWindowClass(STATIC_VIEW);
}

