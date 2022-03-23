/**
 * Project Name: smart-music-player
 * File Name: desk_button_view.c
 * Date: 2019-08-17 12:25
 * Author: anruliu
 * Description: Custom control implementation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "desk_button_view.h"
#include "common.h"
#include "globalization.h"
#include "resource.h"
#include "datafactory.h"

extern HWND ActivityMusicPlay(HWND hosting);
extern HWND ActivityMusicList(HWND hosting);
extern HWND ActivityMusicBluetooth(HWND hosting);
extern HWND ActivityMusicAux(HWND hosting);
extern HWND ActivitySettingAlarm(HWND hosting);
extern HWND ActivitySettingSystem(HWND hosting);

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    DeskButotnData *deskButotnData = NULL;
    deskButotnData = (DeskButotnData*) GetWindowAdditionalData(GetParent(hwnd));

    sm_debug("desk button clike hwnd=%p id=%d\n", hwnd, id);

    if (nc == STN_CLICKED) {
        if (deskButotnData->flag == 0) {
            switch (id) {
            case 0:
                ActivityMusicPlay(GetParent(hwnd));
                break;
            case 1:
                ActivityMusicList(GetParent(hwnd));
                break;
            case 2:
                ActivityMusicBluetooth(GetParent(hwnd));
                break;
            case 3:
                ActivityMusicAux(GetParent(hwnd));
                break;
            case 4:
                ActivitySettingAlarm(GetParent(hwnd));
                break;
            case 5:
                ActivitySettingSystem(GetParent(hwnd));
                break;
            default:
                sm_error("Do not open any windows id is %d\n", id);
                break;
            }
        } else {
            sm_error("Do not open any windows id is %d\n", id);
        }
    }
}

static int DeskButtonViewProc(HWND hwnd, int message, WPARAM wParam,
        LPARAM lParam) {

    DeskButotnData *deskButotnData = NULL;
    deskButotnData = (DeskButotnData*) GetWindowAdditionalData(hwnd);

    switch (message) {
    case MSG_CREATE: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        /* Calculate control coordinates by variable i */
        int i;
        if (deskButotnData->flag == 0) {
            for (i = 0; i < 6; i++) {
                if (rect.right > 240) {
                    deskButotnData->buttonData[i].buttonWnd = CreateWindowEx(
                    BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
                    WS_EX_TRANSPARENT, i, 14 + 108 * (i - (i / 3) * 3),
                            (i / 3) * 154, 75, 75, hwnd,
                            (DWORD) &deskButotnData->buttonData[i]);
                    SetNotificationCallback(
                            deskButotnData->buttonData[i].buttonWnd,
                            my_notify_proc);
                } else {
                    deskButotnData->buttonData[i].buttonWnd = CreateWindowEx(
                    BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
                    WS_EX_TRANSPARENT, i, 14 + 78 * (i - (i / 3) * 3),
                            (i / 3) * 94, 56, 56, hwnd,
                            (DWORD) &deskButotnData->buttonData[i]);
                    SetNotificationCallback(
                            deskButotnData->buttonData[i].buttonWnd,
                            my_notify_proc);
                }
            }
        } else {
            for (i = 0; i < 4; i++) {
                if (rect.right > 240) {
                    deskButotnData->buttonData[i].buttonWnd = CreateWindowEx(
                    BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
                    WS_EX_TRANSPARENT, i, 32 + 166 * (i - (i / 2) * 2),
                            (i / 2) * 154, 75, 75, hwnd,
                            (DWORD) &deskButotnData->buttonData[i]);
                    SetNotificationCallback(
                            deskButotnData->buttonData[i].buttonWnd,
                            my_notify_proc);
                } else {
                    deskButotnData->buttonData[i].buttonWnd = CreateWindowEx(
                    BUTTON_VIEW, "", WS_CHILD | WS_VISIBLE | SS_NOTIFY,
                    WS_EX_TRANSPARENT, i, 32 + 120 * (i - (i / 2) * 2),
                            (i / 2) * 94, 56, 56, hwnd,
                            (DWORD) &deskButotnData->buttonData[i]);
                    SetNotificationCallback(
                            deskButotnData->buttonData[i].buttonWnd,
                            my_notify_proc);
                }
            }
        }
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        SIZE size;
        RECT rect;
        GetClientRect(hwnd, &rect);

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, deskButotnData->desLogfont);
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 152, 152, 152));

        /* Calculate the position of the text output through the variable i */
        int i;
        if (deskButotnData->flag == 0) {
            for (i = 0; i < 6; i++) {
                if (rect.right > 240) {
                    GetTextExtent(hdc,
                            getDeskButtonText(deskButotnData->flag, i), -1,
                            &size);
                    TextOut(hdc,
                            38 + 14 + 108 * (i - (i / 3) * 3) - size.cx / 2,
                            76 + (i / 3) * 154,
                            getDeskButtonText(deskButotnData->flag, i));
                } else {
                    GetTextExtent(hdc,
                            getDeskButtonText(deskButotnData->flag, i), -1,
                            &size);
                    TextOut(hdc, 28 + 14 + 78 * (i - (i / 3) * 3) - size.cx / 2,
                            62 + (i / 3) * 94,
                            getDeskButtonText(deskButotnData->flag, i));
                }
            }
        } else {
            for (i = 0; i < 4; i++) {
                if (rect.right > 240) {
                    GetTextExtent(hdc,
                            getDeskButtonText(deskButotnData->flag, i), -1,
                            &size);
                    TextOut(hdc,
                            38 + 32 + 166 * (i - (i / 2) * 2) - size.cx / 2,
                            76 + (i / 2) * 154,
                            getDeskButtonText(deskButotnData->flag, i));
                } else {
                    GetTextExtent(hdc,
                            getDeskButtonText(deskButotnData->flag, i), -1,
                            &size);
                    TextOut(hdc,
                            28 + 32 + 120 * (i - (i / 2) * 2) - size.cx / 2,
                            62 + (i / 2) * 94,
                            getDeskButtonText(deskButotnData->flag, i));
                }
            }
        }

        EndPaint(hwnd, hdc);
        return 0;
    }
    case MSG_DESTROY: {
        int i;
        if (deskButotnData->flag == 0) {
            for (i = 0; i < 6; i++) {
                unloadBitMap(&deskButotnData->buttonData[i].bmpNormal);
                unloadBitMap(&deskButotnData->buttonData[i].bmpSelect);
            }
        } else {
            for (i = 0; i < 4; i++) {
                unloadBitMap(&deskButotnData->buttonData[i].bmpNormal);
                unloadBitMap(&deskButotnData->buttonData[i].bmpSelect);
            }
        }

        if (deskButotnData) {
            free(deskButotnData);
            deskButotnData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

/*
 * @param flag 0 is smart music player window, 1 is scene window
 */
HWND DeskButotnViewInit(HWND hosting, Uint32 flag) {
    smRect rect;
    HWND deskButtonHwnd = HWND_INVALID;
    DeskButotnData *deskButotnData = NULL;
    deskButotnData = DeskButotnDataInit(deskButotnData, flag);

    getResRect(ID_DESK_BUTTON_AREA, &rect);

    deskButtonHwnd = CreateWindowEx(DESK_BUTTON_VIEW, "",
    WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
            rect.w, rect.h, hosting, (DWORD) deskButotnData);

    return deskButtonHwnd;
}

BOOL RegisterDeskButotnView() {
    WNDCLASS MyClass;
    MyClass.spClassName = DESK_BUTTON_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = DeskButtonViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterDeskButotnView(void) {
    UnregisterWindowClass(DESK_BUTTON_VIEW);
}
