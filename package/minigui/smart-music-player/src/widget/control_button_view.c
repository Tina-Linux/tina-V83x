/**
 * Project Name: smart-music-player
 * File Name: control_button_view.c
 * Date: 2019-08-21 09:51
 * Author: anruliu
 * Description: Custom control implementation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "control_button_view.h"
#include "common.h"
#include "resource.h"
#include "datafactory.h"

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("control button clike hwnd=%p id=%d\n", hwnd, id);

    if (nc == STN_CLICKED) {
        /* PostMessage(GetParent(GetParent(hwnd)), MSG_CLOSE, 0, 0); */
    }
}

static int ControlButtonViewProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    ControlButotnData *contrelButotnData = NULL;
    contrelButotnData = (ControlButotnData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_CREATE: {
        int margin;
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (rect.right > 240) {
            margin = 20;
            if (contrelButotnData->flag != 0)
                margin = 28;
        } else {
            margin = 16;
            if (contrelButotnData->flag != 0)
                margin = 22;
        }

        int bmpTotalWidth = contrelButotnData->buttonData[0].bmpNormal.bmWidth
                + contrelButotnData->buttonData[1].bmpNormal.bmWidth
                + contrelButotnData->buttonData[2].bmpNormal.bmWidth;
        int buttonInterval = (rect.right - bmpTotalWidth - margin * 2) / 2;

        contrelButotnData->buttonData[0].buttonWnd = CreateWindowEx(
        BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
        WS_EX_TRANSPARENT, 0, margin,
                rect.bottom / 2
                        - contrelButotnData->buttonData[0].bmpNormal.bmHeight
                                / 2,
                contrelButotnData->buttonData[0].bmpNormal.bmWidth,
                contrelButotnData->buttonData[0].bmpNormal.bmHeight, hwnd,
                (DWORD) &contrelButotnData->buttonData[0]);
        SetNotificationCallback(contrelButotnData->buttonData[0].buttonWnd,
                my_notify_proc);

        contrelButotnData->buttonData[1].buttonWnd = CreateWindowEx(
        BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
        WS_EX_TRANSPARENT, 1,
                margin + buttonInterval
                        + contrelButotnData->buttonData[0].bmpNormal.bmWidth, 0,
                contrelButotnData->buttonData[1].bmpNormal.bmWidth,
                contrelButotnData->buttonData[1].bmpNormal.bmHeight, hwnd,
                (DWORD) &contrelButotnData->buttonData[1]);
        SetNotificationCallback(contrelButotnData->buttonData[1].buttonWnd,
                my_notify_proc);

        contrelButotnData->buttonData[2].buttonWnd = CreateWindowEx(
        BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
        WS_EX_TRANSPARENT, 2,
                margin + buttonInterval
                        + contrelButotnData->buttonData[0].bmpNormal.bmWidth
                        + buttonInterval
                        + contrelButotnData->buttonData[1].bmpNormal.bmWidth,
                rect.bottom / 2
                        - contrelButotnData->buttonData[0].bmpNormal.bmHeight
                                / 2,
                contrelButotnData->buttonData[2].bmpNormal.bmWidth,
                contrelButotnData->buttonData[2].bmpNormal.bmHeight, hwnd,
                (DWORD) &contrelButotnData->buttonData[2]);
        SetNotificationCallback(contrelButotnData->buttonData[2].buttonWnd,
                my_notify_proc);
        break;
    }
    case MSG_DESTROY: {
        int i;
        for (i = 0; i < 3; i++) {
            unloadBitMap(&contrelButotnData->buttonData[i].bmpNormal);
            unloadBitMap(&contrelButotnData->buttonData[i].bmpSelect);
        }

        if (contrelButotnData) {
            free(contrelButotnData);
            contrelButotnData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

/*
 * @param flag 0 is music paly window, 1 is bluetooth music window, 2 is music aux window
 */
HWND ControlButotnViewInit(HWND hosting, Uint32 flag) {
    smRect rect;
    HWND controlButtonHwnd = HWND_INVALID;

    if (flag == 0) {
        ControlButotnData *controlButotnData = NULL;
        controlButotnData = ControlButotnDataInit(controlButotnData, 0);
        getResRect(ID_CONTROL_BUTTON_NEXT_BMP_AREA, &rect);
        controlButtonHwnd = CreateWindowEx(CONTROL_BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
                rect.w, rect.h, hosting, (DWORD) controlButotnData);
    } else if (flag == 1) {
        ControlButotnData *controlButotnData = NULL;
        ControlButotnData *controlButotnData1 = NULL;
        controlButotnData = ControlButotnDataInit(controlButotnData, 0);
        controlButotnData1 = ControlButotnDataInit(controlButotnData1, 2);
        getResRect(ID_CONTROL_BUTTON_NEXT_BMP_AREA, &rect);
        if (rect.w > 240) {
            rect.y = rect.y + 120;
        } else {
            rect.y = rect.y + 80;
        }
        controlButtonHwnd = CreateWindowEx(CONTROL_BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
                rect.w, rect.h, hosting, (DWORD) controlButotnData);
        getResRect(ID_CONTROL_BUTTON_BLUETOOTH_BMP_AREA, &rect);
        controlButtonHwnd = CreateWindowEx(CONTROL_BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
                rect.w, rect.h, hosting, (DWORD) controlButotnData1);
    } else if (flag == 2) {
        ControlButotnData *controlButotnData = NULL;
        controlButotnData = ControlButotnDataInit(controlButotnData, 1);
        getResRect(ID_CONTROL_BUTTON_VOLUME_BMP_AREA, &rect);
        controlButtonHwnd = CreateWindowEx(CONTROL_BUTTON_VIEW, "",
        WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
                rect.w, rect.h, hosting, (DWORD) controlButotnData);
    }

    return controlButtonHwnd;
}

BOOL RegisterControlButotnView() {
    WNDCLASS MyClass;
    MyClass.spClassName = CONTROL_BUTTON_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = ControlButtonViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterControlButotnView(void) {
    UnregisterWindowClass(CONTROL_BUTTON_VIEW);
}
