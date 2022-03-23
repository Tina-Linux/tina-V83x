/**
 * Project Name: smart-music-player
 * File Name: list_item_switch.c
 * Date: 2019-08-23 14:45
 * Author: anruliu
 * Description: list view item by switch button
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "list_item_switch.h"
#include "common.h"
#include <sys/time.h>
#include <time.h>

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("switch button clike hwnd=%p id=%d add_data=%ld\n", hwnd, id,
            add_data);

    if (nc == STN_CLICKED) {
        UpdateWindow(GetParent(hwnd), FALSE);
    }
}

static int ListItemSwitchProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    ListItemSwitchData *listItemSwitchData = NULL;
    listItemSwitchData = (ListItemSwitchData*) GetWindowAdditionalData(hwnd);

    static int buttonStatus;
    static BOOL isUpClick;
    static BOOL doubleClick;
    static long oldtime;
    struct timeval tv;

    switch (message) {
    case MSG_CREATE: {
        HWND switchButtonHwnd;
        RECT rect;
        GetClientRect(hwnd, &rect);
        switchButtonHwnd = CreateWindowEx(SWITCH_BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0,
                rect.right - listItemSwitchData->switchButtonData->bgBmp.bmWidth
                        - 14,
                rect.bottom / 2
                        - listItemSwitchData->switchButtonData->bgBmp.bmHeight
                                / 2,
                listItemSwitchData->switchButtonData->bgBmp.bmWidth,
                listItemSwitchData->switchButtonData->bgBmp.bmHeight, hwnd,
                (DWORD) listItemSwitchData->switchButtonData);
        SetNotificationCallback(switchButtonHwnd, my_notify_proc);

        buttonStatus = 0;
        isUpClick = FALSE;
        doubleClick = FALSE;
        gettimeofday(&tv, NULL);
        oldtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        break;
    }
    case MSG_LBUTTONDBLCLK:
        if (listItemSwitchData->clickDisable)
            break;
        doubleClick = TRUE;
        break;
    case MSG_LBUTTONDOWN:
        if (listItemSwitchData->clickDisable)
            break;
        /* Click range of the switch button */
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (rect.right > 240) {
            if (LOWORD(lParam) > 234)
                break;
        } else {
            if (LOWORD(lParam) > 160)
                break;
        }
        if (GetCapture() == hwnd)
            break;
        SetCapture(hwnd);
        doubleClick = FALSE;
        isUpClick = FALSE;
        buttonStatus = 1;
        UpdateWindow(hwnd, FALSE);
        break;
    case MSG_LBUTTONUP:
        if (listItemSwitchData->clickDisable)
            break;
        if (GetCapture() == hwnd)
            ReleaseCapture();
        else
            break;
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

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, listItemSwitchData->desLogfont);
        if (listItemSwitchData->switchButtonData->buttonStatus) {
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 162, 0));
        } else {
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));
        }
        GetTextExtent(hdc, listItemSwitchData->desText, -1, &size);
        TextOut(hdc, 14, rect.bottom / 2 - size.cy / 2,
                listItemSwitchData->desText);

        /* Drawing dividing line */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 38, 38, 38));
        FillBox(hdc, 14, rect.bottom - 2, rect.right - 28, 2);

        EndPaint(hwnd, hdc);

        /* Notify the parent window button to be clicked */
        if (isUpClick && (GetWindowStyle(hwnd) & SS_NOTIFY) && !doubleClick) {
            isUpClick = FALSE;
            NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED, 0);
        }
        return 0;
    }
    case MSG_DESTROY: {
        if (listItemSwitchData) {
            free(listItemSwitchData);
            listItemSwitchData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterListItemSwitch() {
    WNDCLASS MyClass;
    MyClass.spClassName = LIST_ITEM_SWITCH;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = ListItemSwitchProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterListItemSwitch(void) {
    UnregisterWindowClass(LIST_ITEM_SWITCH);
}
