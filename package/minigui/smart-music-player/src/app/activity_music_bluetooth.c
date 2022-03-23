/**
 * Project Name: smart-music-player
 * File Name: activity_music_bluetooth.c
 * Date: 2019-08-21 09:12
 * Author: anruliu
 * Description: Play Bluetooth music window
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"
#include "globalization.h"

static int ActivityMusicBluetoothProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    switch (nMessage) {
    case MSG_CREATE: {
        HeadbarViewInit(hWnd, 0, 2);
        ControlButotnViewInit(hWnd, 1);
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hWnd);
        SIZE size;
        RECT rect;
        GetClientRect(hWnd, &rect);

        /* Draw blue background */
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 0, 30, 255));
        if (rect.right > 240) {
            FillBox(hdc, 0, 0, rect.right, 240);
        } else {
            FillBox(hdc, 0, 0, rect.right, 160);
        }

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, getLogFont(ID_FONT_TIMES_24));
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 255, 255, 255));
        char* str = "BGM0000";
        GetTextExtent(hdc, str, -1, &size);
        if (rect.right > 240) {
            TextOut(hdc, rect.right / 2 - size.cx / 2, 112, str);
        } else {
            TextOut(hdc, rect.right / 2 - size.cx / 2, 76, str);
        }

        SelectFont(hdc, getLogFont(ID_FONT_TIMES_18));
        GetTextExtent(hdc, getSettingDesText(9), -1, &size);
        if (rect.right > 240) {
            TextOut(hdc, rect.right / 2 - size.cx / 2, 158,
                    getSettingDesText(9));
        } else {
            TextOut(hdc, rect.right / 2 - size.cx / 2, 108,
                    getSettingDesText(9));
        }

        EndPaint(hWnd, hdc);
        return 0;
    }
    case MSG_DESTROY:
        sm_debug("MSG_DESTROY\n");
        DestroyAllControls(hWnd);
        return 0;
    case MSG_CLOSE:
        DestroyMainWindow(hWnd);
        MainWindowThreadCleanup(hWnd);
        return 0;
    }
    return DefaultMainWinProc(hWnd, nMessage, wParam, lParam);
}

HWND ActivityMusicBluetooth(HWND hosting) {

    MSG Msg;
    MAINWINCREATE CreateInfo;

#ifdef _MGRM_PROCESSES
    JoinLayer(NAME_DEF_LAYER , SMART_MUSIC_PLAYER , 0 , 0);
#endif

    smRect rect;
    getResRect(ID_SCREEN, &rect);

    CreateInfo.dwStyle = WS_NONE;
    CreateInfo.dwExStyle = WS_EX_NONE;
    CreateInfo.spCaption = "";
    CreateInfo.hMenu = 0;
    CreateInfo.hCursor = GetSystemCursor(0);
    CreateInfo.hIcon = 0;
    CreateInfo.MainWindowProc = ActivityMusicBluetoothProc;
    CreateInfo.lx = 0;
    CreateInfo.ty = 0;
    CreateInfo.rx = rect.w;
    CreateInfo.by = rect.h;
    CreateInfo.iBkColor = PIXEL_black;
    CreateInfo.dwAddData = 0;
    CreateInfo.hHosting = hosting;

    HWND hMainWnd = CreateMainWindow(&CreateInfo);

    if (hMainWnd == HWND_INVALID)
        return HWND_INVALID;

    ShowWindow(hMainWnd, SW_SHOWNORMAL);

    return hMainWnd;
}

#ifdef _MGRM_THREADS
#include <minigui/dti.c>
#endif
