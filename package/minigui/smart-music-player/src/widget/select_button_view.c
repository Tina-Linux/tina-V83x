/**
 * Project Name: smart-music-player
 * File Name: select_button_view.c
 * Date: 2019-08-26 11:01
 * Author: anruliu
 * Description: Custom control implementation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "select_button_view.h"
#include "common.h"
#include "resource.h"

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    SelectButtonData *selectButtonData = NULL;
    selectButtonData = (SelectButtonData*) GetWindowAdditionalData(
            GetParent(hwnd));

    sm_debug("button clike hwnd=%p id=%d\n", hwnd, id);

    if (nc == STN_CLICKED) {
        if (id == 0) {
            if (selectButtonData->curIndex < selectButtonData->listSize - 1) {
                selectButtonData->curIndex++;
                UpdateWindow(GetParent(hwnd), FALSE);

                /* Notify the parent window button to be clicked */
                NotifyParentEx(GetParent(hwnd), GetDlgCtrlID(GetParent(hwnd)),
                STN_CLICKED, (DWORD) selectButtonData->curIndex);
            }
        } else {
            if (selectButtonData->curIndex > 0) {
                selectButtonData->curIndex--;
                UpdateWindow(GetParent(hwnd), FALSE);

                /* Notify the parent window button to be clicked */
                NotifyParentEx(GetParent(hwnd), GetDlgCtrlID(GetParent(hwnd)),
                STN_CLICKED, (DWORD) selectButtonData->curIndex);
            }
        }
    }
}

static int SelectButtonViewProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    SelectButtonData *selectButtonData = NULL;
    selectButtonData = (SelectButtonData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_CREATE: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (selectButtonData->displayMode == SELECT_BUTTON_HORIZONTAL) {
            selectButtonData->buttonData[0].buttonWnd = CreateWindowEx(
            BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
            WS_EX_TRANSPARENT, 1, 0, 0,
                    selectButtonData->buttonData[0].bmpNormal.bmWidth,
                    selectButtonData->buttonData[0].bmpNormal.bmHeight, hwnd,
                    (DWORD) &selectButtonData->buttonData[0]);
            SetNotificationCallback(selectButtonData->buttonData[0].buttonWnd,
                    my_notify_proc);

            selectButtonData->buttonData[1].buttonWnd = CreateWindowEx(
            BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
            WS_EX_TRANSPARENT, 0,
                    rect.right
                            - selectButtonData->buttonData[1].bmpNormal.bmWidth,
                    0, selectButtonData->buttonData[1].bmpNormal.bmWidth,
                    selectButtonData->buttonData[1].bmpNormal.bmHeight, hwnd,
                    (DWORD) &selectButtonData->buttonData[1]);
            SetNotificationCallback(selectButtonData->buttonData[1].buttonWnd,
                    my_notify_proc);
        } else {
            selectButtonData->buttonData[0].buttonWnd = CreateWindowEx(
            BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
            WS_EX_TRANSPARENT, 0, 0, 0,
                    selectButtonData->buttonData[0].bmpNormal.bmHeight,
                    selectButtonData->buttonData[0].bmpNormal.bmWidth, hwnd,
                    (DWORD) &selectButtonData->buttonData[0]);
            SetNotificationCallback(selectButtonData->buttonData[0].buttonWnd,
                    my_notify_proc);

            selectButtonData->buttonData[1].buttonWnd =
                    CreateWindowEx(
                    BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
                    WS_EX_TRANSPARENT, 1, 0,
                            rect.bottom
                                    - selectButtonData->buttonData[1].bmpNormal.bmHeight,
                            selectButtonData->buttonData[1].bmpNormal.bmHeight,
                            selectButtonData->buttonData[1].bmpNormal.bmWidth,
                            hwnd, (DWORD) &selectButtonData->buttonData[1]);
            SetNotificationCallback(selectButtonData->buttonData[1].buttonWnd,
                    my_notify_proc);
        }
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        RECT rect;
        SIZE size;
        GetClientRect(hwnd, &rect);

        /* Drawing background */
        FillBoxWithBitmap(hdc, 0, 0, rect.right, rect.bottom,
                &selectButtonData->bgBmp);

        /* Draw the list text */
        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, selectButtonData->textLogfont);
        if (selectButtonData->displayMode == SELECT_BUTTON_HORIZONTAL) {
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 162, 0));
        } else {
            SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 116, 205, 0));
        }
        GetTextExtent(hdc,
                selectButtonData->listText[selectButtonData->curIndex], -1,
                &size);
        TextOut(hdc, rect.right / 2 - size.cx / 2,
                rect.bottom / 2 - size.cy / 2,
                selectButtonData->listText[selectButtonData->curIndex]);

        EndPaint(hwnd, hdc);
        return 0;
    }
    case MSG_DESTROY: {
        unloadBitMap(&selectButtonData->buttonData[0].bmpNormal);
        unloadBitMap(&selectButtonData->buttonData[0].bmpSelect);
        unloadBitMap(&selectButtonData->buttonData[1].bmpNormal);
        unloadBitMap(&selectButtonData->buttonData[1].bmpSelect);
        unloadBitMap(&selectButtonData->bgBmp);
        if (selectButtonData) {
            free(selectButtonData);
            selectButtonData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

BOOL RegisterSelectButtonView() {
    WNDCLASS MyClass;
    MyClass.spClassName = SELECT_BUTTON_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = SelectButtonViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterSelectButtonView(void) {
    UnregisterWindowClass(SELECT_BUTTON_VIEW);
}
