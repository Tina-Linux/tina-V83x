#include "swipe_view.h"

#define SWIPE_SLIDE_SPEED 50

static int slideDistance;
HWND laundryHwnd1, laundryHwnd2;

static int SwipeViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

    SwipeData *swipeData = NULL;
    swipeData = (SwipeData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_CREATE: {

        if (swipeData->laundryData[0]) {
            laundryHwnd1 =  CreateWindowEx(LAUNDRY_VIEW, "",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY,
            WS_EX_TRANSPARENT, 0, 0, 0, 486, 100, hwnd,
                    (DWORD) swipeData->laundryData[0]);
        }

        if (swipeData->laundryData[1]) {
            laundryHwnd2 = CreateWindowEx(LAUNDRY_VIEW, "",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY,
            WS_EX_TRANSPARENT, 0, 486, 0, 486, 100, hwnd,
                    (DWORD) swipeData->laundryData[1]);
        }

        slideDistance = 0;
        swipeData->currentPageIndex = 0;
        return 0;
    }
    case MSG_TIMER: {
        if (swipeData->currentPageIndex == 0) {
            slideDistance -= SWIPE_SLIDE_SPEED;
            MoveWindow(laundryHwnd1, slideDistance, 0, 486, 100, FALSE);
            MoveWindow(laundryHwnd2, 486 + slideDistance, 0, 486, 100, FALSE);
            if (slideDistance <= -480) {
                MoveWindow(laundryHwnd1, -486, 0, 486, 100, FALSE);
                MoveWindow(laundryHwnd2, 0, 0, 486, 100, FALSE);
                swipeData->currentPageIndex = 1;
                slideDistance = -486;
                KillTimer(hwnd, SWIPE_TIMER_ID);
            }
        } else {
            slideDistance += SWIPE_SLIDE_SPEED;
            MoveWindow(laundryHwnd1, slideDistance, 0, 486, 100, FALSE);
            MoveWindow(laundryHwnd2, 486 + slideDistance, 0, 486, 100, FALSE);
            if (slideDistance >= -6) {
                MoveWindow(laundryHwnd1, 0, 0, 486, 100, FALSE);
                MoveWindow(laundryHwnd2, 486, 0, 486, 100, FALSE);
                swipeData->currentPageIndex = 0;
                slideDistance = 0;
                KillTimer(hwnd, SWIPE_TIMER_ID);
            }
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterSwipeView() {
    WNDCLASS MyClass;
    MyClass.spClassName = SWIPE_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = 0;
    MyClass.iBkColor = PIXEL_lightgray;
    MyClass.WinProc = SwipeViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterSwipeView(void) {
    UnregisterWindowClass(SWIPE_VIEW);
}
