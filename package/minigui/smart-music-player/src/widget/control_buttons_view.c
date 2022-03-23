/**
 * Project Name: smart-music-player
 * File Name: control_button_view.c
 * Date: 2019-08-21 09:51
 * Author: anruliu
 * Description: Custom control implementation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "control_buttons_view.h"
#include "common.h"
#include "resource.h"
#include "datafactory.h"

/* extern HWND ActivityMusicList(HWND hosting); */

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("controls button clike hwnd=%p id=%d\n", hwnd, id);

    if (nc == STN_CLICKED) {
        if (id == 1) {
            /* ActivityMusicList(GetParent(hwnd)); */
        }
    }
}

static int ControlButtonsViewProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    ControlButotnsData *contrelButotnsData = NULL;
    contrelButotnsData = (ControlButotnsData*) GetWindowAdditionalData(hwnd);

    static int playMode;

    switch (message) {
    case MSG_CREATE: {
        int margin;
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (rect.right > 240) {
            margin = 20;
        } else {
            margin = 14;
        }

        int bmpTotalWidth = contrelButotnsData->buttonData[0].bmpNormal.bmWidth
                * 4;
        int buttonInterval = (rect.right - bmpTotalWidth - margin * 2) / 3;

        contrelButotnsData->buttonData[0].buttonWnd = CreateWindowEx(
        BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
        WS_EX_TRANSPARENT, 0, margin, 0,
                contrelButotnsData->buttonData[0].bmpNormal.bmWidth,
                contrelButotnsData->buttonData[0].bmpNormal.bmHeight, hwnd,
                (DWORD) &contrelButotnsData->buttonData[0]);
        SetNotificationCallback(contrelButotnsData->buttonData[0].buttonWnd,
                my_notify_proc);

        contrelButotnsData->buttonData[1].buttonWnd = CreateWindowEx(
        BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
        WS_EX_TRANSPARENT, 1,
                margin + buttonInterval * 2
                        + contrelButotnsData->buttonData[0].bmpNormal.bmWidth
                                * 2, 0,
                contrelButotnsData->buttonData[1].bmpNormal.bmWidth,
                contrelButotnsData->buttonData[1].bmpNormal.bmHeight, hwnd,
                (DWORD) &contrelButotnsData->buttonData[1]);
        SetNotificationCallback(contrelButotnsData->buttonData[1].buttonWnd,
                my_notify_proc);

        contrelButotnsData->buttonData[2].buttonWnd = CreateWindowEx(
        BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
        WS_EX_TRANSPARENT, 2,
                margin + buttonInterval * 3
                        + contrelButotnsData->buttonData[0].bmpNormal.bmWidth
                                * 3, 0,
                contrelButotnsData->buttonData[2].bmpNormal.bmWidth,
                contrelButotnsData->buttonData[2].bmpNormal.bmHeight, hwnd,
                (DWORD) &contrelButotnsData->buttonData[2]);
        SetNotificationCallback(contrelButotnsData->buttonData[2].buttonWnd,
                my_notify_proc);

        playMode = 0;
        break;
    }
    case MSG_LBUTTONUP: {
        int margin;
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (rect.right > 240) {
            margin = 20;
        } else {
            margin = 14;
        }
        int bmpTotalWidth = contrelButotnsData->buttonData[0].bmpNormal.bmWidth
                * 4;
        int buttonInterval = (rect.right - bmpTotalWidth - margin * 2) / 3;

        /* Click on the time control to jump to the main window */
        if (LOSWORD(lParam)
                > margin + buttonInterval
                        + contrelButotnsData->playModeBmp[0].bmWidth
                && LOSWORD(lParam)
                        < margin + buttonInterval
                                + contrelButotnsData->playModeBmp[0].bmWidth
                                        * 2) {
            playMode = (playMode < 3) ? ++playMode : 0;
            UpdateWindow(hwnd, TRUE);
        }
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        int margin;
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (rect.right > 240) {
            margin = 20;
        } else {
            margin = 14;
        }
        int bmpTotalWidth = contrelButotnsData->buttonData[0].bmpNormal.bmWidth
                * 4;
        int buttonInterval = (rect.right - bmpTotalWidth - margin * 2) / 3;
        FillBoxWithBitmap(hdc,
                margin + buttonInterval
                        + contrelButotnsData->playModeBmp[0].bmWidth, 0, 0, 0,
                &contrelButotnsData->playModeBmp[playMode]);
        EndPaint(hwnd, hdc);
        return 0;
    }
    case MSG_DESTROY: {
        int i;
        for (i = 0; i < 3; i++) {
            unloadBitMap(&contrelButotnsData->buttonData[i].bmpNormal);
            unloadBitMap(&contrelButotnsData->buttonData[i].bmpSelect);
        }
        for (i = 0; i < 4; i++) {
            unloadBitMap(&contrelButotnsData->playModeBmp[i]);
        }

        if (contrelButotnsData) {
            free(contrelButotnsData);
            contrelButotnsData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

HWND ControlButotnsViewInit(HWND hosting) {
    smRect rect;
    HWND controlButtonHwnd = HWND_INVALID;

    ControlButotnsData *controlButotnsData = NULL;
    controlButotnsData = ControlButotnsDataInit(controlButotnsData);
    getResRect(ID_CONTROL_BUTTONS_BMP_AREA, &rect);
    controlButtonHwnd = CreateWindowEx(CONTROL_BUTTONS_VIEW, "",
    WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
            rect.w, rect.h, hosting, (DWORD) controlButotnsData);

    return controlButtonHwnd;
}

BOOL RegisterControlButotnsView() {
    WNDCLASS MyClass;
    MyClass.spClassName = CONTROL_BUTTONS_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = ControlButtonsViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterControlButotnsView(void) {
    UnregisterWindowClass(CONTROL_BUTTONS_VIEW);
}
