#include "laundry_part_view.h"

/* Determine if the text contains Chinese */
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

static int LaundryPartViewProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    LaundryPartData *laundryPartData = NULL;
    laundryPartData = (LaundryPartData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        RECT rect;
        SIZE size1, size2, size3;
        GetClientRect(hwnd, &rect);

        if (laundryPartData->bmpBg.bmBits)
            FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
                    &laundryPartData->bmpBg);

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, GetWindowFont(hwnd));
        GetTextExtent(hdc, laundryPartData->valueUnitDes[0], -1, &size1);
        SelectFont(hdc, laundryPartData->logfontUnit);
        GetTextExtent(hdc, laundryPartData->valueUnitDes[1], -1, &size2);
        SelectFont(hdc, laundryPartData->logfontDes);
        GetTextExtent(hdc, laundryPartData->valueUnitDes[2], -1, &size3);

        if (!IncludeChinese(laundryPartData->valueUnitDes[0])) {
            SelectFont(hdc, laundryPartData->logfontUnit);
            SetTextColor(hdc, COLOR_lightwhite);
            TextOut(hdc, rect.right / 2 + size1.cx / 2 - size2.cx / 2,
                    rect.bottom / 2 + size1.cy / 2 - size3.cy / 2 - size2.cy,
                    laundryPartData->valueUnitDes[1]);
        } else {
            size2.cx = 0;
            size2.cy = 0;
        }
        SelectFont(hdc, GetWindowFont(hwnd));
        if (laundryPartData->enableClick == LAUNDRY_PART_ENABLE_CLICK_YES) {
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 179, 9));
        } else if (laundryPartData->enableClick == LAUNDRY_PART_ENABLE_CLICK_NO) {
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 144,144,144));
        }
        TextOut(hdc, rect.right / 2 - size1.cx / 2 - size2.cx / 2,
                rect.bottom / 2 - size1.cy / 2 - size3.cy / 2,
                laundryPartData->valueUnitDes[0]);
        SelectFont(hdc, laundryPartData->logfontDes);
        SetTextColor(hdc, COLOR_lightwhite);
        TextOut(hdc, rect.right / 2 - size3.cx / 2,
                rect.bottom / 2 + size1.cy / 2 - size3.cy / 2,
                laundryPartData->valueUnitDes[2]);

        EndPaint(hwnd, hdc);
        return 0;
    }
    case MSG_LBUTTONDOWN:
        if ((GetWindowStyle(hwnd) & SS_NOTIFY)
                && laundryPartData->enableClick == LAUNDRY_PART_ENABLE_CLICK_YES)
            NotifyParent(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED);
        break;
    case MSG_LAUNDRY_PART_CHANGE_VALUE:
        laundryPartData->valueUnitDes[0] = (char *) wParam;
        InvalidateRect(hwnd, NULL, TRUE);
        break;
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterLaundryPartView() {
    WNDCLASS MyClass;
    MyClass.spClassName = LAUNDRY_PART_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = 0;
    MyClass.iBkColor = PIXEL_lightgray;
    MyClass.WinProc = LaundryPartViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterLaundryPartView(void) {
    UnregisterWindowClass(LAUNDRY_PART_VIEW);
}
