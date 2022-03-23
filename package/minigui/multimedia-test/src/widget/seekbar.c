#include "seekbar.h"
#include "BottomMenu.h"
#include "player_int.h"

extern BottomMenuDataType *bottom;
extern RecordWinDataType *rcd;
int seekto_lengthX = 0;
int i = 0;
static int SeektoBarProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

	SeektoBar *seektobar = NULL;
	seektobar = (SeektoBar *) GetWindowAdditionalData(hwnd);
	const char* spCaption;
	struct timeval tv;
	CvrRectType rect;
	HDC hdc;
	switch (message) {
	case MSG_CREATE:
		SetTimer(hwnd, _ID_SEEKBAR_TIMER, 50);
		return 0;
	case MSG_PAINT: {
		hdc = BeginPaint(hwnd);
		CvrRectType rect;
		GetClientRect(hwnd, &rect);
		spCaption = GetWindowCaption(hwnd);
		SetBkMode(hdc, BM_TRANSPARENT);
		SetTextColor(hdc, GetWindowBkColor(hwnd));
		int id = (int) wParam;
		tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
		if (seektobar->isDown == TRUE && seektobar->seektobarlen < rect.w) {

			FillBoxWithBitmap(hdc, rect.x, rect.y, seektobar->seektobarlen,
					rect.h, &seektobar->bmpSelect);
		} else if (seektobar->isDown == FALSE
				&& seektobar->seektobarlen < rect.w) {

			FillBoxWithBitmap(hdc, rect.x, rect.y, seektobar->seektobarlen,
					rect.h, &seektobar->bmpSelect);
			seektobar->seektobarlen = bottom->startMenuSize.w
					* rcd->rpData.videoCurTime / rcd->rpData.videoTotalTime;
			bottom->onceflash = FALSE;
		}
		EndPaint(hwnd, hdc);
		return 0;
	}
	case MSG_TIMER:
		tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
		if (bottom->isPlaying == FALSE && rcd->rpData.videoCurTime > 0
				&& rcd->rpData.videoCurTime < rcd->rpData.videoTotalTime) {
			InvalidateRect(hwnd, NULL, TRUE);
		} else if (bottom->isPlaying == TRUE) {
			if (bottom->onceflash == TRUE) {
				InvalidateRect(hwnd, NULL, TRUE);
			} else if (bottom->onceflash == FALSE) {
				break;
			}
		}

		break;
	case MSG_LBUTTONDOWN:
		if (bottom->isPlaying == TRUE) {
			break;
		} else {
			seekto_lengthX = LOWORD(lParam);
			seektobar->seektobarlen = seekto_lengthX;
			seektobar->isDown = TRUE;
			InvalidateRect(hwnd, NULL, TRUE);
		}
		break;
	case MSG_LBUTTONUP:
		if (bottom->isPlaying == TRUE) {
			break;
		} else {
			seektobar->isDown = FALSE;
			tplayer_getcurrentpos(&rcd->rpData.videoCurTime);
			NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED, 0);
		}
		break;
#if 0
		case MSG_ERASEBKGND: {
			HDC hdc = (HDC) wParam;
			BOOL fGetDC = FALSE;
			if (hdc == 0) {
				hdc = GetClientDC(hwnd);
				fGetDC = TRUE;
			}
			SetBrushColor(hdc, RGB2Pixel(hdc, 0xFF, 0xFF, 0xFF));
			FillBox(hdc, 0, 0, bottom->mainSize.w, bottom->mainSize.h);
			SetMemDCAlpha(hdc, MEMDC_FLAG_SRCALPHA, 50);

			/* SetMemDCColorKey(hdc, MEMDC_FLAG_SRCCOLORKEY, PIXEL_transparent); */
			if (fGetDC)
			ReleaseDC(hdc);
			return 0;
		}
#endif
	case MSG_CLOSE:
		DestroyMainWindow(hwnd);
		return 0;
	case MSG_DESTROY:
		if (NULL != seektobar) {
			free(seektobar);
			seektobar = NULL;
		}
		return 0;
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterSeektoBarView() {
	WNDCLASS MyClass;
	MyClass.spClassName = SEEKTO_BAR;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_transparent;
	MyClass.WinProc = SeektoBarProc;
	return RegisterWindowClass(&MyClass);
}

void UnregisterSeektoBarView(void) {
	UnregisterWindowClass(SEEKTO_BAR);
}
