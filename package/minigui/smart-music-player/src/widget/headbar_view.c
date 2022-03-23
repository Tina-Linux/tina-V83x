/**
 * Project Name: smart-music-player
 * File Name: headbar_view.c
 * Date: 2019-08-19 19:57
 * Author: anruliu
 * Description: Custom control implementation
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "headbar_view.h"
#include "common.h"
#include "resource.h"
#include "datafactory.h"
#include "globalization.h"

#ifdef __SMART_MUSIC_PLAYER_240x320__
#define UPDATE_INTERVALS   10   /* 100ms slide once  */
#else
#define UPDATE_INTERVALS   2   /* 20ms slide once  */
#endif

static void my_notify_proc(HWND hwnd, int id, int nc, DWORD add_data) {

    sm_debug("headbar back button clike hwnd=%p id=%d\n", hwnd, id);

    if (nc == STN_CLICKED) {
        PostMessage(GetParent(GetParent(hwnd)), MSG_CLOSE, 0, 0);
    }
}

static int HeadBarViewProc(HWND hwnd, int message, WPARAM wParam, LPARAM lParam) {

    HeadbarData *headbarData = NULL;
    headbarData = (HeadbarData*) GetWindowAdditionalData(hwnd);

    static int musicPlayingIndex;

    switch (message) {
    case MSG_CREATE: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        if (rect.right > 240) {
            headbarData->buttonData.buttonWnd = CreateWindowEx(BUTTON_VIEW, "",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 12, 12, 48,
                    48, hwnd, (DWORD) &headbarData->buttonData);
        } else {
            headbarData->buttonData.buttonWnd = CreateWindowEx(BUTTON_VIEW, "",
            WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, 8, 8, 36,
                    36, hwnd, (DWORD) &headbarData->buttonData);
        }
        SetNotificationCallback(headbarData->buttonData.buttonWnd,
                my_notify_proc);
        musicPlayingIndex = 0;
        if (headbarData->flag) {
            SetTimer(hwnd, ID_TIMER_MUSIC_PLAYING, UPDATE_INTERVALS);
        }
        break;
    }
    case MSG_TIMER:
        musicPlayingIndex = (musicPlayingIndex < 3) ? ++musicPlayingIndex : 0;
        UpdateWindow(hwnd, FALSE);
        break;
    case MSG_LBUTTONUP: {
        RECT rect;
        GetClientRect(hwnd, &rect);
        /* Increase the back button click range */
        if (rect.right > 240) {
            if (LOWORD(lParam) < 72) {
                PostMessage(GetParent(hwnd), MSG_CLOSE, 0, 0);
            }
        } else {
            if (LOWORD(lParam) < 50) {
                PostMessage(GetParent(hwnd), MSG_CLOSE, 0, 0);
            }
        }
        return 0;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hwnd);
        RECT rect;
        GetClientRect(hwnd, &rect);

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, headbarData->titleLogfont);
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));
        if (rect.right > 240) {
            TextOut(hdc, 66, 10, getHeadbarDesText(headbarData->titleIndex));

            if (headbarData->flag) {
                FillBoxWithBitmap(hdc, 260, 14, 0, 0,
                        &headbarData->musicPlayingBmp[musicPlayingIndex]);
            }
        } else {
            TextOut(hdc, 46, 8, getHeadbarDesText(headbarData->titleIndex));

            if (headbarData->flag) {
                FillBoxWithBitmap(hdc, 196, 8, 0, 0,
                        &headbarData->musicPlayingBmp[musicPlayingIndex]);
            }
        }
        EndPaint(hwnd, hdc);
        return 0;
    }
    case MSG_DESTROY: {
        unloadBitMap(&headbarData->buttonData.bmpNormal);
        unloadBitMap(&headbarData->buttonData.bmpSelect);
        if (headbarData->flag) {
            KillTimer(hwnd, ID_TIMER_MUSIC_PLAYING);
            int i;
            for (i = 0; i < 4; i++) {
                unloadBitMap(&headbarData->musicPlayingBmp[i]);
            }
        }
        if (headbarData) {
            free(headbarData);
            headbarData = NULL;
        }
        break;
    }
    }
    return DefaultControlProc(hwnd, message, wParam, lParam);
}

/*
 * @param flag 0 is normal, 1 is music playing
 * @param index 0 ~ 7, title description
 */
HWND HeadbarViewInit(HWND hosting, Uint32 flag, Uint32 index) {
    smRect rect;
    HWND headbarHwnd = HWND_INVALID;
    HeadbarData *headbarData = NULL;
    headbarData = HeadbarDataInit(headbarData, flag, index);

    getResRect(ID_HEADBAR_BUTTON_BMP_AREA, &rect);

    headbarHwnd = CreateWindowEx(HEADBAR_VIEW, "",
    WS_CHILD | WS_VISIBLE | SS_NOTIFY, WS_EX_TRANSPARENT, 0, rect.x, rect.y,
            rect.w, rect.h, hosting, (DWORD) headbarData);
    return headbarHwnd;
}

BOOL RegisterHeadbarView() {
    WNDCLASS MyClass;
    MyClass.spClassName = HEADBAR_VIEW;
    MyClass.dwStyle = WS_NONE;
    MyClass.dwExStyle = WS_EX_NONE;
    MyClass.hCursor = GetSystemCursor(0);
    MyClass.iBkColor = PIXEL_black;
    MyClass.WinProc = HeadBarViewProc;

    return RegisterWindowClass(&MyClass);
}

void UnregisterHeadbarView(void) {
    UnregisterWindowClass(HEADBAR_VIEW);
}

