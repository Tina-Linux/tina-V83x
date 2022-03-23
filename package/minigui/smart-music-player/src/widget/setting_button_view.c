/**
 * Project Name: smart-music-player
 * File Name: setting_button_view.c
 * Date: 2019-08-22 10:15
 * Author: anruliu
 * Description: Custom control implementation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "setting_button_view.h"
#include "datafactory.h"
#include "resource.h"
#include <sys/time.h>
#include <time.h>

static int SettingButtonViewProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    SettingButtonData *settingButtonData = NULL;
    settingButtonData = (SettingButtonData*) GetWindowAdditionalData(hwnd);

    static int buttonStatus;
    static BOOL isUpClick;
    static BOOL doubleClick;
    static long oldtime;
    struct timeval tv;

    switch (message) {
    case MSG_CREATE:
        buttonStatus = 0;
        isUpClick = FALSE;
        doubleClick = FALSE;
        gettimeofday(&tv, NULL);
        oldtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        break;
    case MSG_LBUTTONDBLCLK:
        doubleClick = TRUE;
        break;
    case MSG_LBUTTONDOWN:
        if (GetCapture() == hwnd)
            break;
        SetCapture(hwnd);
        doubleClick = FALSE;
        isUpClick = FALSE;
        buttonStatus = 1;
        UpdateWindow(hwnd, FALSE);
        break;
    case MSG_LBUTTONUP:
        if (GetCapture() == hwnd)
            ReleaseCapture();
        gettimeofday(&tv, NULL);
        long newtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        int lentime = newtime - oldtime;
        if (lentime < 300)
            doubleClick = TRUE;
        oldtime = newtime;
        isUpClick = TRUE;
        buttonStatus = 0;
        UpdateWindow(hwnd, FALSE);
        break;
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        RECT rect;
        SIZE size;
        GetClientRect(hwnd, &rect);

        /* Draw a background based on the button state */
        if (buttonStatus == 1) {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 35, 35, 35));
        } else {
            SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 0, 0, 0));
        }
        FillBox(hdc, 0, 0, rect.right, rect.bottom);

        /* Draw the picture on the left */
        FillBoxWithBitmap(hdc, 14,
                rect.bottom / 2 - settingButtonData->settingBmp.bmHeight / 2, 0,
                0, &settingButtonData->settingBmp);

        /* Draw the text on the right */
        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, settingButtonData->settingLogfont);
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));
        GetTextExtent(hdc, settingButtonData->settingText, -1, &size);
        TextOut(hdc, 22 + settingButtonData->settingBmp.bmWidth,
                rect.bottom / 2 - size.cy / 2, settingButtonData->settingText);

        EndPaint(hwnd, hdc);

        /* Notify the parent window button to be clicked */
        if (isUpClick && (GetWindowStyle(hwnd) & SS_NOTIFY) && !doubleClick) {
            isUpClick = FALSE;
            NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED, 0);
        }
        return 0;
    }
    case MSG_DESTROY: {
        unloadBitMap(&settingButtonData->settingBmp);
        if (settingButtonData) {
            free(settingButtonData);
            settingButtonData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterSettingButtonView() {
    WNDCLASS MyClass;
    MyClass.spClassName = SETTING_BUTTON_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = SettingButtonViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterSettingButtonView(void) {
    UnregisterWindowClass(SETTING_BUTTON_VIEW);
}
