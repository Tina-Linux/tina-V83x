/**
 * Project Name: smart-music-player
 * File Name: list_item_select.c
 * Date: 2019-08-26 16:34
 * Author: anruliu
 * Description:
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "list_item_select.h"
#include "common.h"
#include <sys/time.h>
#include <time.h>

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("select button clike hwnd=%p id=%d add_data=%ld\n", hwnd, id,
            add_data);

    if (nc == STN_CLICKED) {
        NotifyParentEx(GetParent(hwnd), GetDlgCtrlID(GetParent(hwnd)),
        STN_CLICKED, add_data);
    }
}

static int ListItemSelectProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    ListItemSelectData *listItemSelectData = NULL;
    listItemSelectData = (ListItemSelectData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_CREATE: {
        HWND switchButtonHwnd;
        RECT rect;
        GetClientRect(hwnd, &rect);
        switchButtonHwnd = CreateWindowEx(SELECT_BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0,
                rect.right - listItemSelectData->selectButtonData->bgBmp.bmWidth
                        - 14,
                rect.bottom / 2
                        - listItemSelectData->selectButtonData->bgBmp.bmHeight
                                / 2,
                listItemSelectData->selectButtonData->bgBmp.bmWidth,
                listItemSelectData->selectButtonData->bgBmp.bmHeight, hwnd,
                (DWORD) listItemSelectData->selectButtonData);
        SetNotificationCallback(switchButtonHwnd, my_notify_proc);

        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        RECT rect;
        SIZE size;
        GetClientRect(hwnd, &rect);

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, listItemSelectData->desLogfont);
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));
        GetTextExtent(hdc, listItemSelectData->desText, -1, &size);
        TextOut(hdc, 14, rect.bottom / 2 - size.cy / 2,
                listItemSelectData->desText);

        /* Drawing dividing line */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 38, 38, 38));
        FillBox(hdc, 14, rect.bottom - 2, rect.right - 28, 2);

        EndPaint(hwnd, hdc);
        return 0;
    }
    case MSG_DESTROY: {
        if (listItemSelectData) {
            free(listItemSelectData);
            listItemSelectData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterListItemSelect() {
    WNDCLASS MyClass;
    MyClass.spClassName = LIST_ITEM_SELECT;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = ListItemSelectProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterListItemSelect(void) {
    UnregisterWindowClass(LIST_ITEM_SELECT);
}
