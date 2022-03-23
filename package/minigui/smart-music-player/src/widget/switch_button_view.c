/**
 * Project Name: smart-music-player
 * File Name: switch_button_view.c
 * Date: 2019-08-23 10:13
 * Author: anruliu
 * Description: Custom control implementation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "switch_button_view.h"
#include "resource.h"

#ifdef __SMART_MUSIC_PLAYER_240x320__
#define SLIDING_STEP_VALUE  4   /* Decrease 4 each time */
#else
#define SLIDING_STEP_VALUE  8   /* Decrease 8 each time */
#endif

static int SwitchbuttonViewProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    SwitchButtonData *switchButtonData = NULL;
    switchButtonData = (SwitchButtonData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_CREATE: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (switchButtonData->buttonStatus) {
            /* Initial state If the button is selected, then slidingDistance should be on the right */
            switchButtonData->slidingDistance = rect.right
                    - switchButtonData->onBmp.bmWidth;
        } else {
            switchButtonData->slidingDistance = 0;
        }
        break;
    }
    case MSG_TIMER: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        /* Calculate the direction and distance of movement */
        if (switchButtonData->buttonStatus) {
            /* move to the left */
            switchButtonData->slidingDistance =
                    switchButtonData->slidingDistance - SLIDING_STEP_VALUE;
            if (switchButtonData->slidingDistance <= 0) {
                switchButtonData->slidingDistance = 0;
                switchButtonData->buttonStatus = 0;
                KillTimer(hwnd, ID_TIMER_SWITCH_BUTTON);
                /* Notify the parent window button status change */
                NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED,
                        (DWORD) switchButtonData->buttonStatus);
            }
        } else {
            /* move to the right */
            switchButtonData->slidingDistance =
                    switchButtonData->slidingDistance + SLIDING_STEP_VALUE;
            if (switchButtonData->slidingDistance
                    >= rect.right - switchButtonData->onBmp.bmWidth) {
                switchButtonData->slidingDistance = rect.right
                        - switchButtonData->onBmp.bmWidth;
                switchButtonData->buttonStatus = 1;
                KillTimer(hwnd, ID_TIMER_SWITCH_BUTTON);
                /* Notify the parent window button status change */
                NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED,
                        (DWORD) switchButtonData->buttonStatus);
            }
        }
        UpdateWindow(hwnd, FALSE);
        break;
    }
    case MSG_LBUTTONDOWN:
        if (GetCapture() == hwnd)
            break;
        SetCapture(hwnd);
        break;
    case MSG_LBUTTONUP:
        if (GetCapture() == hwnd)
            ReleaseCapture();
        else
            break;
        SetTimer(hwnd, ID_TIMER_SWITCH_BUTTON, 1);
        break;
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        RECT rect;
        GetClientRect(hwnd, &rect);

        /* Drawing background */
        FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
                &switchButtonData->bgBmp);

        /* Display images based on the state and distance of the button */
        if (switchButtonData->buttonStatus) {
            FillBoxWithBitmap(hdc, switchButtonData->slidingDistance, 0,
                    switchButtonData->onBmp.bmWidth,
                    switchButtonData->onBmp.bmHeight, &switchButtonData->onBmp);
        } else {
            FillBoxWithBitmap(hdc, switchButtonData->slidingDistance, 0,
                    switchButtonData->offBmp.bmWidth,
                    switchButtonData->offBmp.bmHeight,
                    &switchButtonData->offBmp);
        }
        EndPaint(hwnd, hdc);
        return 0;
    }
    case MSG_DESTROY: {
        if (IsTimerInstalled(hwnd, ID_TIMER_SWITCH_BUTTON)) {
            KillTimer(hwnd, ID_TIMER_SWITCH_BUTTON);
        }
        unloadBitMap(&switchButtonData->bgBmp);
        unloadBitMap(&switchButtonData->onBmp);
        unloadBitMap(&switchButtonData->offBmp);
        if (switchButtonData) {
            free(switchButtonData);
            switchButtonData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterSwitchButtonView() {
    WNDCLASS MyClass;
    MyClass.spClassName = SWITCH_BUTTON_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = SwitchbuttonViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterSwitchButtonView(void) {
    UnregisterWindowClass(SWITCH_BUTTON_VIEW);
}
