/**
 * Project Name: smart-musicplayer
 * File Name: activity_music_play.c
 * Date: 2019-08-19 19:01
 * Author: anruliu
 * Description: Music play window
 * Copyright (c) 2019, Allwinnertech All Rights Reserved.
 */

#include "resource.h"
#include "datafactory.h"

static int ActivityMusicPlayProc(HWND hWnd, int nMessage, WPARAM wParam,
        LPARAM lParam) {

    switch (nMessage) {
    case MSG_CREATE: {
        HeadbarViewInit(hWnd, 1, 0);
        ControlButotnViewInit(hWnd, 0);
        ControlButotnsViewInit(hWnd);
        ProgressViewInit(hWnd);
        break;
    }
    case MSG_PAINT: {
        HDC hdc = BeginPaint(hWnd);
        RECT rect;
        GetClientRect(hWnd, &rect);
        SetBrushColor(hdc, RGB2Pixel(HDC_SCREEN, 204, 40, 71));

        SetBkMode(hdc, BM_TRANSPARENT);
        SelectFont(hdc, getLogFont(ID_FONT_TIMES_14));
        SetTextColor(hdc, RGB2Pixel(HDC_SCREEN, 152, 152, 152));

        if (rect.right > 240) {
            FillBox(hdc, 0, 0, rect.right, 297);
            TextOut(hdc, 17, 440, "02:28");
            TextOut(hdc, 244, 440, "05:20");
        } else {
            FillBox(hdc, 0, 0, rect.right, 198);
            TextOut(hdc, 14, 293, "02:28");
            TextOut(hdc, 190, 293, "05:20");
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

HWND ActivityMusicPlay(HWND hosting) {

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
    CreateInfo.MainWindowProc = ActivityMusicPlayProc;
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

