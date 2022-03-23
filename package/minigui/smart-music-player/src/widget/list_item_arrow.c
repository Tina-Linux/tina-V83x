/**
 * Project Name: smart-music-player
 * File Name: list_item_arrow.c
 * Date: 2019-08-27 16:52
 * Author: anruliu
 * Description: List item containing arrow
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "list_item_arrow.h"
#include "resource.h"
#include <sys/time.h>
#include <time.h>

static int ListItemArrowProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    ListItemArrowData *listItemArrowData = NULL;
    listItemArrowData = (ListItemArrowData*) GetWindowAdditionalData(hwnd);

    static int buttonStatus;
    static BOOL isUpClick;
    static BOOL doubleClick;
    static long oldtime;
    struct timeval tv;

    switch (message) {
    case MSG_CREATE: {
        buttonStatus = 0;
        isUpClick = FALSE;
        doubleClick = FALSE;
        gettimeofday(&tv, NULL);
        oldtime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
        break;
    }
    case MSG_LBUTTONDBLCLK:
        if (listItemArrowData->clickDisable)
            break;
        doubleClick = TRUE;
        break;
    case MSG_LBUTTONDOWN:
        if (listItemArrowData->clickDisable)
            break;
        doubleClick = FALSE;
        isUpClick = FALSE;
        buttonStatus = 1;
        UpdateWindow(hwnd, FALSE);
        break;
    case MSG_LBUTTONUP:
        /* If the list is sliding, it won't be executed again because
         * it was intercepted by the list view.*/
        if (listItemArrowData->clickDisable)
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
    case MSG_MOUSEMOVE:
        if (buttonStatus) {
            /* When the list is slid, the item needs to be restored */
            buttonStatus = 0;
            UpdateWindow(hwnd, FALSE);
        }
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
        SelectFont(hdc, listItemArrowData->desLogfont);
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));
        GetTextExtent(hdc, listItemArrowData->desText, -1, &size);
        TextOut(hdc, 14, rect.bottom / 2 - size.cy / 2,
                listItemArrowData->desText);

        /* Drawing dividing line */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 38, 38, 38));
        FillBox(hdc, 14, rect.bottom - 2, rect.right - 28, 2);

        /* Drawing arrows */
        FillBoxWithBitmap(hdc,
                rect.right - listItemArrowData->arrowBmp.bmWidth - 14,
                rect.bottom / 2 - listItemArrowData->arrowBmp.bmHeight / 2, 0,
                0, &listItemArrowData->arrowBmp);

        EndPaint(hwnd, hdc);

        /* Notify the parent window button to be clicked */
        if (isUpClick && (GetWindowStyle(hwnd) & SS_NOTIFY) && !doubleClick) {
            isUpClick = FALSE;
            NotifyParentEx(hwnd, GetDlgCtrlID(hwnd), STN_CLICKED, 0);
        }
        return 0;
    }
    case MSG_DESTROY: {
        unloadBitMap(&listItemArrowData->arrowBmp);
        if (listItemArrowData) {
            free(listItemArrowData);
            listItemArrowData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterListItemArrow() {
    WNDCLASS MyClass;
    MyClass.spClassName = LIST_ITEM_ARROW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = ListItemArrowProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterListItemArrow(void) {
    UnregisterWindowClass(LIST_ITEM_ARROW);
}
