#include "double_text_view.h"
#include "common.h"

#define TIMER_ID 10
#ifdef ALLWINNERTECH_R6
#define PAGE_SWITCH_SPEED 12
#else
#define PAGE_SWITCH_SPEED 3
#endif

void ChangeDoubleTextPage(HWND hwnd) {
	SetTimer(hwnd, TIMER_ID, 1);
}

BOOL ChangeValue(HWND hwnd, char *value) {
	DoubleTextView *doubleTextData = NULL;
	doubleTextData = (DoubleTextView*) GetWindowAdditionalData(hwnd);
	if (doubleTextData) {
		switch (doubleTextData->currentPage) {
		case PAGE_ONE:
			doubleTextData->valueUnitDes[0] = value;
			break;
		case PAGE_TOW:
			doubleTextData->valueUnitDes[3] = value;
			break;
		default:
			doubleTextData->currentPage = PAGE_ONE;
			doubleTextData->valueUnitDes[0] = value;
			break;
		}
		InvalidateRect(hwnd, NULL, TRUE);
		return TRUE;
	} else {
		return FALSE;
	}
}

static BOOL IncludeChinese(char *str) {
	char c;
	c = str[0];
	while (c != '\0') {
		if ((c & 0x80) && (*str & 0x80)) {
			return TRUE;
		}
		c = *str++;
	}
	return FALSE;
}

static int DoubleTextViewProc(HWND hwnd, int message, WPARAM wParam,
		LPARAM lParam) {
	DoubleTextView *doubleTextData = NULL;
	doubleTextData = (DoubleTextView*) GetWindowAdditionalData(hwnd);
	int index1, index2, index3;

	switch (message) {
	case MSG_TIMER: {
		if (doubleTextData->currentPage == PAGE_ONE) {
			doubleTextData->slideDistance -= PAGE_SWITCH_SPEED;
			if (doubleTextData->slideDistance < -50) {
				doubleTextData->slideDistance = 0;
				doubleTextData->currentPage = PAGE_TOW;
				KillTimer(hwnd, TIMER_ID);
			}
			InvalidateRect(hwnd, NULL, TRUE);
		} else if (doubleTextData->currentPage == PAGE_TOW) {
			doubleTextData->slideDistance += PAGE_SWITCH_SPEED;
			if (doubleTextData->slideDistance > 50) {
				doubleTextData->slideDistance = 0;
				doubleTextData->currentPage = PAGE_ONE;
				KillTimer(hwnd, TIMER_ID);
			}
			InvalidateRect(hwnd, NULL, TRUE);
		}
		break;
	}
	case MSG_PAINT: {
		HDC hdc = BeginPaint(hwnd);
		RECT rect;
		SIZE size1, size2, size3;
		GetClientRect(hwnd, &rect);

		if (doubleTextData->bmpBg.bmBits)
			FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
					&doubleTextData->bmpBg);

		switch (doubleTextData->currentPage) {
		case PAGE_ONE:
			index1 = 0;
			index2 = 1;
			index3 = 2;
			break;
		case PAGE_TOW:
			index1 = 3;
			index2 = 4;
			index3 = 5;
			break;
		default:
			doubleTextData->currentPage = PAGE_ONE;
			index1 = 0;
			index2 = 1;
			index3 = 2;
			break;
		}
		SetBkMode(hdc, BM_TRANSPARENT);
		SelectFont(hdc, GetWindowFont(hwnd));
		GetTextExtent(hdc, doubleTextData->valueUnitDes[index1], -1, &size1);
		SelectFont(hdc, doubleTextData->logfontUnit);
		GetTextExtent(hdc, doubleTextData->valueUnitDes[index2], -1, &size2);
		SelectFont(hdc, doubleTextData->logfontDes);
		GetTextExtent(hdc, doubleTextData->valueUnitDes[index3], -1, &size3);

		if (!IncludeChinese(doubleTextData->valueUnitDes[index1])) {
			SelectFont(hdc, doubleTextData->logfontUnit);
			SetTextColor(hdc, COLOR_lightwhite);
			TextOut(hdc,
					rect.right / 2 + size1.cx / 2 - size2.cx / 2
							+ doubleTextData->slideDistance,
					rect.bottom / 2 + size1.cy / 2 - size3.cy / 2 - size2.cy,
					doubleTextData->valueUnitDes[index2]);
		} else {
			size2.cx = 0;
			size2.cy = 0;
		}
		SelectFont(hdc, GetWindowFont(hwnd));
		if (doubleTextData->enableClick[doubleTextData->currentPage]
				== ENABLE_CLICK_YES) {
			SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 179, 9));
		} else if (doubleTextData->enableClick[doubleTextData->currentPage]
				== ENABLE_CLICK_NO) {
			SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 144,144,144));
		}
		TextOut(hdc,
				rect.right / 2 - size1.cx / 2 - size2.cx / 2
						+ doubleTextData->slideDistance,
				rect.bottom / 2 - size1.cy / 2 - size3.cy / 2,
				doubleTextData->valueUnitDes[index1]);
		SelectFont(hdc, doubleTextData->logfontDes);
		SetTextColor(hdc, COLOR_lightwhite);
		TextOut(hdc,
				rect.right / 2 - size3.cx / 2 + doubleTextData->slideDistance,
				rect.bottom / 2 + size1.cy / 2 - size3.cy / 2,
				doubleTextData->valueUnitDes[index3]);

		EndPaint(hwnd, hdc);
		return 0;
	}
	case MSG_LBUTTONDOWN:
		if ((GetWindowStyle(hwnd) & SS_NOTIFY)
				&& doubleTextData->enableClick[doubleTextData->currentPage]
						== ENABLE_CLICK_YES)
			NotifyParent(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED);
		break;
	}
	return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterDoubleTextView() {
	WNDCLASS MyClass;
	MyClass.spClassName = Double_TEXT_VIEW;
	MyClass.dwStyle = WS_NONE;
	MyClass.dwExStyle = WS_EX_NONE;
	MyClass.hCursor = 0;
	MyClass.iBkColor = PIXEL_lightgray;
	MyClass.WinProc = DoubleTextViewProc;

	return RegisterWindowClass(&MyClass);
}

void UnregisterDoubleTextView(void) {
	UnregisterWindowClass(Double_TEXT_VIEW);
}
